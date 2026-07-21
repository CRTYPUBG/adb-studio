#include "adb_studio/diagnostics/device_diagnostics.hpp"

#include <QCoreApplication>
#include <QHash>

#include <algorithm>

namespace adb_studio::diagnostics
{
namespace
{
auto tr(const char* text) -> QString
{
    return QCoreApplication::translate("DeviceDiagnostics", text);
}

// These internal factories keep diagnostic declarations compact and named at their call sites.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto makeDiagnostic(const QString& id, const QString& category, DiagnosticStatus status,
                    const QString& title, const QString& summary, const QString& action,
                    bool automatic = false, int minutes = 3,
                    const QString& difficulty = tr("Easy")) -> DiagnosticResult
{
    return {id,      category,
            status,  title,
            summary, tr("This check affects connection reliability, compatibility, or security."),
            action,  difficulty,
            minutes, automatic};
}

auto evidenceDiagnostic(const QString& id, const QString& category, EvidenceState evidence,
                        const QString& title, const QString& passSummary,
                        const QString& failSummary, const QString& action,
                        bool automatic = false) -> DiagnosticResult
{
    switch (evidence)
    {
    case EvidenceState::Yes:
        return makeDiagnostic(id, category, DiagnosticStatus::Pass, title, passSummary, {});
    case EvidenceState::No:
        return makeDiagnostic(id, category, DiagnosticStatus::Fail, title, failSummary, action,
                              automatic);
    case EvidenceState::Unknown:
        return makeDiagnostic(id, category, DiagnosticStatus::Unknown, title,
                              tr("There is not enough verified information for this check."),
                              action);
    }
    return {};
}

auto capabilityDiagnostic(const QString& id, EvidenceState evidence, const QString& title,
                          const QString& passSummary, const QString& unsupportedSummary,
                          const QString& action) -> DiagnosticResult
{
    if (evidence == EvidenceState::Yes)
    {
        return makeDiagnostic(id, QStringLiteral("Compatibility"), DiagnosticStatus::Pass, title,
                              passSummary, {});
    }
    if (evidence == EvidenceState::No)
    {
        return makeDiagnostic(id, QStringLiteral("Compatibility"), DiagnosticStatus::Unsupported,
                              title, unsupportedSummary, action);
    }
    return makeDiagnostic(id, QStringLiteral("Compatibility"), DiagnosticStatus::Unknown, title,
                          tr("There is not enough verified capability information."), action);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

auto scoreForStatus(DiagnosticStatus status) -> int
{
    switch (status)
    {
    case DiagnosticStatus::Pass:
        return 100;
    case DiagnosticStatus::Warning:
        return 60;
    case DiagnosticStatus::Fail:
        return 0;
    case DiagnosticStatus::Unsupported:
    case DiagnosticStatus::Unknown:
        return -1;
    }
    return -1;
}

auto normalized(const QString& value) -> QString
{
    return value.trimmed().toLower();
}
} // namespace

auto ConnectionDiagnosticsEngine::classify(const DeviceFacts& facts) -> ConnectionState
{
    if (facts.fastbootMode == EvidenceState::Yes)
    {
        return ConnectionState::FastbootMode;
    }
    if (facts.deviceCount > 1)
    {
        return ConnectionState::MultipleDevices;
    }
    if (facts.adbInstalled == EvidenceState::No || facts.adbServerRunning == EvidenceState::No)
    {
        return ConnectionState::AdbServerNotRunning;
    }
    if (facts.adbVersionCompatible == EvidenceState::No)
    {
        return ConnectionState::AdbVersionMismatch;
    }
    if (facts.deviceBusy == EvidenceState::Yes)
    {
        return ConnectionState::DeviceBusy;
    }
    if (facts.supportedDevice == EvidenceState::No)
    {
        return ConnectionState::UnsupportedDevice;
    }
    if (facts.authorizationPending == EvidenceState::Yes)
    {
        return ConnectionState::AuthorizationPending;
    }
    const QString adbState = normalized(facts.adbDeviceState);
    if (adbState == QStringLiteral("unauthorized"))
    {
        return ConnectionState::Unauthorized;
    }
    if (adbState == QStringLiteral("offline"))
    {
        return ConnectionState::Offline;
    }
    if (adbState == QStringLiteral("recovery") || facts.recoveryMode == EvidenceState::Yes)
    {
        return ConnectionState::RecoveryMode;
    }
    if (adbState == QStringLiteral("device"))
    {
        return facts.wirelessConnected == EvidenceState::Yes ? ConnectionState::WirelessConnected
                                                             : ConnectionState::Connected;
    }
    if (facts.usbDriverPresent == EvidenceState::No)
    {
        return ConnectionState::DriverMissing;
    }
    if (facts.mtpDriverPresent == EvidenceState::No && facts.usbDevicePresent == EvidenceState::Yes)
    {
        return ConnectionState::MtpDriverMissing;
    }
    if (facts.usbDevicePresent == EvidenceState::Yes &&
        facts.usbDebuggingEnabled == EvidenceState::No)
    {
        return ConnectionState::UsbDebuggingDisabled;
    }
    const QString usbMode = normalized(facts.usbMode);
    if (usbMode == QStringLiteral("charging"))
    {
        return ConnectionState::UsbChargingOnly;
    }
    if (usbMode == QStringLiteral("file-transfer") || usbMode == QStringLiteral("mtp"))
    {
        return ConnectionState::UsbFileTransfer;
    }
    if (facts.usbDevicePresent == EvidenceState::No)
    {
        return ConnectionState::NoUsbDevice;
    }
    if (facts.wirelessAvailable == EvidenceState::Yes)
    {
        return ConnectionState::WirelessAvailable;
    }
    return ConnectionState::Unknown;
}

auto ConnectionDiagnosticsEngine::diagnosticsFor(const DeviceFacts& facts)
    -> QList<DiagnosticResult>
{
    QList<DiagnosticResult> results;
    results.append(evidenceDiagnostic(
        QStringLiteral("adb-installed"), QStringLiteral("ADB"), facts.adbInstalled,
        tr("ADB installed"), tr("ADB is available."), tr("ADB was not found."),
        tr("Install Android platform-tools from the official source.")));
    results.append(evidenceDiagnostic(
        QStringLiteral("adb-running"), QStringLiteral("ADB"), facts.adbServerRunning,
        tr("ADB server"), tr("The ADB server responded."), tr("The ADB server did not respond."),
        tr("Restart the ADB server and scan again."), true));
    results.append(evidenceDiagnostic(QStringLiteral("adb-version"), QStringLiteral("ADB"),
                                      facts.adbVersionCompatible, tr("ADB version compatibility"),
                                      tr("No client/server mismatch was detected."),
                                      tr("The ADB client and server versions do not match."),
                                      tr("Use one current platform-tools installation."), true));
    results.append(evidenceDiagnostic(QStringLiteral("usb-driver"), QStringLiteral("USB"),
                                      facts.usbDriverPresent, tr("USB driver"),
                                      tr("A compatible USB driver was verified."),
                                      tr("A compatible USB driver was not detected."),
                                      tr("Open the detected manufacturer driver guide.")));
    results.append(evidenceDiagnostic(
        QStringLiteral("windows-driver"), QStringLiteral("USB"), facts.windowsDriverPresent,
        tr("Windows driver"), tr("Windows reported a healthy Android ADB interface."),
        tr("Windows reported a missing or unhealthy Android ADB interface."),
        tr("Repair or install the manufacturer driver, then rescan devices.")));
    results.append(evidenceDiagnostic(
        QStringLiteral("mtp-driver"), QStringLiteral("USB"), facts.mtpDriverPresent,
        tr("MTP driver"), tr("The MTP driver is available."),
        tr("The MTP driver is missing or unavailable."),
        tr("Reconnect in File Transfer mode and repair the Windows driver.")));
    results.append(
        makeDiagnostic(QStringLiteral("usb-speed"), QStringLiteral("USB"),
                       facts.usbSpeedMbps >= 0 ? DiagnosticStatus::Pass : DiagnosticStatus::Unknown,
                       tr("USB speed"),
                       facts.usbSpeedMbps >= 0 ? tr("USB throughput was measured.")
                                               : tr("USB speed has not been measured."),
                       tr("Run the USB throughput benchmark after authorization.")));
    results.append(
        makeDiagnostic(QStringLiteral("usb-mode"), QStringLiteral("USB"),
                       facts.usbMode.isEmpty() ? DiagnosticStatus::Unknown : DiagnosticStatus::Pass,
                       tr("USB mode"),
                       facts.usbMode.isEmpty() ? tr("The active USB mode is not verified.")
                                               : tr("The active USB mode was measured."),
                       tr("Select the OEM-recommended data mode when a connection requires it.")));
    DiagnosticStatus cableStatus = DiagnosticStatus::Unknown;
    QString cableSummary =
        tr("Transport stability has not been measured; physical cable quality is unknown.");
    if (facts.usbCableStable == EvidenceState::Yes)
    {
        cableStatus = DiagnosticStatus::Pass;
        cableSummary = tr("Two USB transfer samples were stable within the approved tolerance.");
    }
    else if (facts.usbCableStable == EvidenceState::No)
    {
        cableStatus = DiagnosticStatus::Warning;
        cableSummary = tr("Repeated USB transfer samples varied beyond the stability tolerance.");
    }
    results.append(makeDiagnostic(
        QStringLiteral("usb-cable"), QStringLiteral("USB"), cableStatus,
        tr("USB transport stability"), cableSummary,
        tr("This is a transfer-stability proxy, not proof of physical cable quality. Retest with "
           "a known data cable and another port.")));
    const bool authorized = normalized(facts.adbDeviceState) == QStringLiteral("device");
    DiagnosticStatus authorizationStatus = DiagnosticStatus::Unknown;
    if (authorized)
    {
        authorizationStatus = DiagnosticStatus::Pass;
    }
    else if (normalized(facts.adbDeviceState) == QStringLiteral("unauthorized"))
    {
        authorizationStatus = DiagnosticStatus::Fail;
    }
    results.append(makeDiagnostic(
        QStringLiteral("authorization"), QStringLiteral("Connection"), authorizationStatus,
        tr("ADB authorization"),
        authorized ? tr("The device authorized this computer.")
                   : tr("ADB authorization is not verified."),
        tr("Unlock the device, review the RSA fingerprint, and approve the prompt."), false, 2));
    results.append(evidenceDiagnostic(QStringLiteral("wireless"), QStringLiteral("Connection"),
                                      facts.wirelessAvailable, tr("Wireless debugging"),
                                      tr("Wireless debugging is available."),
                                      tr("Wireless debugging is unavailable."),
                                      tr("Open Wireless debugging in Developer options.")));
    results.append(capabilityDiagnostic(
        QStringLiteral("audio"), facts.audioSupported, tr("Audio forwarding"),
        tr("Audio forwarding support was verified."), tr("Audio forwarding is unsupported."),
        tr("Check Android version and mirror backend compatibility.")));
    results.append(capabilityDiagnostic(
        QStringLiteral("recording"), facts.screenRecordingSupported, tr("Screen recording"),
        tr("Screen recording support was verified."), tr("Screen recording is unsupported."),
        tr("Check the device media projection and codec capabilities.")));
    results.append(capabilityDiagnostic(
        QStringLiteral("codec"), facts.codecSupported, tr("Video codec"),
        tr("A compatible video codec was verified."), tr("No compatible video codec was verified."),
        tr("Run the codec capability probe.")));
    DiagnosticResult refresh = capabilityDiagnostic(
        QStringLiteral("high-refresh"), facts.highRefreshSupported, tr("High refresh rate"),
        tr("High refresh support was verified."), tr("High refresh is unsupported."),
        tr("Use a supported display mode and codec."));
    refresh.category = QStringLiteral("Performance");
    results.append(refresh);
    DiagnosticStatus rootStatus = DiagnosticStatus::Unknown;
    QString rootSummary = tr("Root status is not verified.");
    if (facts.rooted == EvidenceState::Yes)
    {
        rootStatus = DiagnosticStatus::Warning;
        rootSummary = tr("Root access was detected; review the security impact.");
    }
    else if (facts.rooted == EvidenceState::No)
    {
        rootStatus = DiagnosticStatus::Pass;
        rootSummary = tr("No root access was detected.");
    }
    results.append(makeDiagnostic(
        QStringLiteral("root"), QStringLiteral("Security"), rootStatus, tr("Root status"),
        rootSummary, tr("Use least privilege and do not grant untrusted tools root access.")));
    EvidenceState normalMode = EvidenceState::Unknown;
    if (facts.fastbootMode == EvidenceState::Yes)
    {
        normalMode = EvidenceState::No;
    }
    else if (facts.fastbootMode == EvidenceState::No)
    {
        normalMode = EvidenceState::Yes;
    }
    results.append(
        evidenceDiagnostic(QStringLiteral("fastboot"), QStringLiteral("Connection"), normalMode,
                           tr("Normal Android mode"), tr("Fastboot mode is not active."),
                           tr("The device is in Fastboot mode."),
                           tr("Boot Android normally unless Fastboot work is intentional.")));
    EvidenceState normalRecoveryMode = EvidenceState::Unknown;
    if (facts.recoveryMode == EvidenceState::Yes)
    {
        normalRecoveryMode = EvidenceState::No;
    }
    else if (facts.recoveryMode == EvidenceState::No)
    {
        normalRecoveryMode = EvidenceState::Yes;
    }
    results.append(evidenceDiagnostic(
        QStringLiteral("recovery"), QStringLiteral("Connection"), normalRecoveryMode,
        tr("Normal boot mode"), tr("Android Recovery mode is not active."),
        tr("The device is running Android Recovery."),
        tr("Restart into Android unless Recovery work is intentional.")));
    results.append(makeDiagnostic(
        QStringLiteral("battery-optimization"), QStringLiteral("Recommended Settings"),
        DiagnosticStatus::Unknown, tr("Battery optimization"),
        tr("Battery optimization restrictions have not been measured."),
        tr("Open the OEM guide before changing power-management settings.")));
    return results;
}

auto ConnectionDiagnosticsEngine::calculateHealth(const QList<DiagnosticResult>& diagnostics)
    -> HealthScore
{
    const QStringList categories = {QStringLiteral("Connection"),
                                    QStringLiteral("ADB"),
                                    QStringLiteral("USB"),
                                    QStringLiteral("Performance"),
                                    QStringLiteral("Compatibility"),
                                    QStringLiteral("Security"),
                                    QStringLiteral("Recommended Settings")};
    QHash<QString, int> scores;
    int known = 0;
    for (const QString& category : categories)
    {
        int total = 0;
        int count = 0;
        for (const auto& diagnostic : diagnostics)
        {
            if (diagnostic.category != category)
            {
                continue;
            }
            const int score = scoreForStatus(diagnostic.status);
            if (score >= 0)
            {
                total += score;
                ++count;
                ++known;
            }
        }
        scores.insert(category, count > 0 ? total / count : 50);
    }
    HealthScore result;
    result.connection = scores.value(QStringLiteral("Connection"));
    result.adb = scores.value(QStringLiteral("ADB"));
    result.usb = scores.value(QStringLiteral("USB"));
    result.performance = scores.value(QStringLiteral("Performance"));
    result.compatibility = scores.value(QStringLiteral("Compatibility"));
    result.security = scores.value(QStringLiteral("Security"));
    result.recommendedSettings = scores.value(QStringLiteral("Recommended Settings"));
    result.total =
        (result.connection * 25 + result.adb * 20 + result.usb * 15 + result.performance * 10 +
         result.compatibility * 10 + result.security * 10 + result.recommendedSettings * 10) /
        100;
    result.confidence =
        diagnostics.isEmpty()
            ? 0
            : static_cast<int>((static_cast<qsizetype>(known) * 100) / diagnostics.size());
    return result;
}

auto ConnectionDiagnosticsEngine::statusKey(DiagnosticStatus status) -> QString
{
    switch (status)
    {
    case DiagnosticStatus::Pass:
        return QStringLiteral("PASS");
    case DiagnosticStatus::Warning:
        return QStringLiteral("WARNING");
    case DiagnosticStatus::Fail:
        return QStringLiteral("FAIL");
    case DiagnosticStatus::Unsupported:
        return QStringLiteral("UNSUPPORTED");
    case DiagnosticStatus::Unknown:
        return QStringLiteral("UNKNOWN");
    }
    return QStringLiteral("UNKNOWN");
}

auto ConnectionDiagnosticsEngine::analyze(const DeviceFacts& facts) -> ConnectionReport
{
    ConnectionReport report;
    report.facts = facts;
    report.state = classify(facts);
    report.currentStep = facts.recoveryAttempted
                             ? tr("Automatic recovery completed; connection analysis updated")
                             : tr("Connection analysis complete");
    switch (report.state)
    {
    case ConnectionState::NoUsbDevice:
        report.stateKey = QStringLiteral("NO_USB_DEVICE");
        report.title = tr("No USB device detected");
        report.explanation =
            tr("The hardware probe verified that no Android USB device is present.");
        report.recommendedAction = tr("Connect an unlocked device with a data-capable cable.");
        break;
    case ConnectionState::UsbChargingOnly:
        report.stateKey = QStringLiteral("USB_CHARGING_ONLY");
        report.title = tr("USB is set to charging only");
        report.explanation = tr("The detected USB mode does not expose a data connection.");
        report.recommendedAction =
            tr("Choose File Transfer or the recommended USB mode on the device.");
        break;
    case ConnectionState::UsbFileTransfer:
        report.stateKey = QStringLiteral("USB_FILE_TRANSFER");
        report.title = tr("USB file transfer detected");
        report.explanation =
            tr("The USB data path is available, but ADB authorization is not verified.");
        report.recommendedAction = tr("Enable USB debugging and approve the computer fingerprint.");
        break;
    case ConnectionState::UsbDebuggingDisabled:
        report.stateKey = QStringLiteral("USB_DEBUGGING_DISABLED");
        report.title = tr("USB debugging is disabled");
        report.explanation =
            tr("The device USB configuration was measured and does not include ADB.");
        report.recommendedAction = tr("Enable USB debugging in Developer options, then reconnect.");
        break;
    case ConnectionState::AuthorizationPending:
        report.stateKey = QStringLiteral("AUTHORIZATION_PENDING");
        report.title = tr("Authorization prompt pending");
        report.explanation = tr("The device is waiting for an on-device authorization decision.");
        report.recommendedAction =
            tr("Unlock the device and review the displayed RSA fingerprint.");
        break;
    case ConnectionState::Unauthorized:
        report.stateKey = QStringLiteral("UNAUTHORIZED");
        report.title = tr("Authorization required");
        report.explanation =
            tr("ADB sees the device, but the device has not authorized this computer.");
        report.recommendedAction = tr("Unlock the device and approve the RSA fingerprint prompt.");
        break;
    case ConnectionState::Offline:
        report.stateKey = QStringLiteral("OFFLINE");
        report.title = tr("Device is offline");
        report.explanation = tr("ADB reported the transport but could not communicate with it.");
        report.recommendedAction = tr("Reconnect the cable, then restart ADB and scan again.");
        break;
    case ConnectionState::AdbServerNotRunning:
        report.stateKey = QStringLiteral("ADB_SERVER_NOT_RUNNING");
        report.title = tr("ADB is unavailable");
        report.explanation = facts.adbInstalled == EvidenceState::No
                                 ? tr("Android platform-tools were not found on this computer.")
                                 : tr("The ADB server did not respond successfully.");
        report.recommendedAction = facts.adbInstalled == EvidenceState::No
                                       ? tr("Install official Android platform-tools.")
                                       : tr("Restart the ADB server and scan again.");
        break;
    case ConnectionState::AdbVersionMismatch:
        report.stateKey = QStringLiteral("ADB_VERSION_MISMATCH");
        report.title = tr("ADB version mismatch");
        report.explanation = tr("The ADB client and server reported incompatible versions.");
        report.recommendedAction =
            tr("Remove stale ADB instances and use one platform-tools version.");
        break;
    case ConnectionState::DeviceBusy:
        report.stateKey = QStringLiteral("DEVICE_BUSY");
        report.title = tr("Device is busy");
        report.explanation =
            tr("A measured device operation is preventing the requested connection action.");
        report.recommendedAction = tr("Wait for the active operation to finish, then scan again.");
        break;
    case ConnectionState::DriverMissing:
        report.stateKey = QStringLiteral("DRIVER_MISSING");
        report.title = tr("USB driver missing");
        report.explanation =
            tr("The hardware probe found the device but not a compatible ADB driver.");
        report.recommendedAction = tr("Open the manufacturer driver instructions.");
        break;
    case ConnectionState::MtpDriverMissing:
        report.stateKey = QStringLiteral("MTP_DRIVER_MISSING");
        report.title = tr("MTP driver missing");
        report.explanation =
            tr("The USB device is present, but the File Transfer driver is unavailable.");
        report.recommendedAction = tr("Repair the Windows MTP driver, then reconnect the device.");
        break;
    case ConnectionState::FastbootMode:
        report.stateKey = QStringLiteral("FASTBOOT_MODE");
        report.title = tr("Fastboot mode detected");
        report.explanation = tr("The device was positively identified by the Fastboot tool.");
        report.recommendedAction = tr("Boot Android normally unless Fastboot work is intentional.");
        break;
    case ConnectionState::RecoveryMode:
        report.stateKey = QStringLiteral("RECOVERY_MODE");
        report.title = tr("Recovery mode detected");
        report.explanation = tr("ADB reported that the device is running Android Recovery.");
        report.recommendedAction = tr("Restart into Android for normal device management.");
        break;
    case ConnectionState::WirelessAvailable:
        report.stateKey = QStringLiteral("WIRELESS_AVAILABLE");
        report.title = tr("Wireless debugging is available");
        report.explanation =
            tr("The device supports wireless setup, but no wireless session is connected.");
        report.recommendedAction =
            tr("Pair from Wireless debugging using the displayed pairing details.");
        break;
    case ConnectionState::WirelessConnected:
        report.stateKey = QStringLiteral("WIRELESS_CONNECTED");
        report.title = tr("Connected wirelessly");
        report.explanation = tr("ADB verified an active wireless device transport.");
        report.recommendedAction = tr("Run diagnostics to confirm latency and throughput.");
        break;
    case ConnectionState::MultipleDevices:
        report.stateKey = QStringLiteral("MULTIPLE_DEVICES");
        report.title = tr("Multiple devices detected");
        report.explanation = tr("ADB returned more than one device transport.");
        report.recommendedAction =
            tr("Select a device explicitly before running a device command.");
        break;
    case ConnectionState::UnsupportedDevice:
        report.stateKey = QStringLiteral("UNSUPPORTED_DEVICE");
        report.title = tr("Unsupported device");
        report.explanation = tr("A verified capability required by ADB Studio is unavailable.");
        report.recommendedAction =
            tr("Review the compatibility diagnostics and supported platform matrix.");
        break;
    case ConnectionState::Connected:
        report.stateKey = QStringLiteral("CONNECTED");
        report.title = tr("Device connected");
        report.explanation = tr("ADB verified and authorized the device transport.");
        report.recommendedAction = tr("Review diagnostics and recommended settings.");
        break;
    case ConnectionState::Unknown:
        report.stateKey = QStringLiteral("UNKNOWN");
        report.title = tr("Connection state needs verification");
        report.explanation = tr("Available evidence is insufficient to identify one exact cause.");
        report.recommendedAction =
            tr("Follow the measured diagnostic results; do not change settings speculatively.");
        break;
    }
    report.diagnostics = diagnosticsFor(facts);
    report.health = calculateHealth(report.diagnostics);
    report.oemGuide = OemGuideCatalog::detect(facts);
    return report;
}

auto OemGuideCatalog::detect(const DeviceFacts& facts) -> OemGuide
{
    const QString combined =
        normalized(facts.manufacturer + QLatin1Char(' ') + facts.brand + QLatin1Char(' ') +
                   facts.model + QLatin1Char(' ') + facts.oemSkin);
    struct Match
    {
        const char* key;
        const char* display;
        const char* documentation;
        QStringList tokens;
    };
    const QList<Match> matches = {
        {"samsung",
         "Samsung",
         "https://developer.samsung.com/android-usb-driver",
         {QStringLiteral("samsung")}},
        {"google",
         "Google Pixel",
         "https://developer.android.com/studio/debug/dev-options",
         {QStringLiteral("google"), QStringLiteral("pixel")}},
        {"xiaomi",
         "Xiaomi / Redmi / POCO",
         "https://developer.android.com/studio/debug/dev-options",
         {QStringLiteral("xiaomi"), QStringLiteral("redmi"), QStringLiteral("poco"),
          QStringLiteral("miui"), QStringLiteral("hyperos")}},
        {"huawei",
         "Huawei",
         "https://consumer.huawei.com/support/",
         {QStringLiteral("huawei"), QStringLiteral("harmonyos"), QStringLiteral("emui")}},
        {"honor",
         "Honor",
         "https://www.honor.com/support/",
         {QStringLiteral("honor"), QStringLiteral("magicos")}},
        {"oneplus",
         "OnePlus",
         "https://service.oneplus.com/",
         {QStringLiteral("oneplus"), QStringLiteral("oxygenos")}},
        {"oppo",
         "OPPO",
         "https://support.oppo.com/",
         {QStringLiteral("oppo"), QStringLiteral("coloros")}},
        {"realme",
         "Realme",
         "https://www.realme.com/support",
         {QStringLiteral("realme"), QStringLiteral("realme ui")}},
        {"vivo",
         "Vivo",
         "https://www.vivo.com/en/support",
         {QStringLiteral("vivo"), QStringLiteral("funtouch")}},
        {"motorola",
         "Motorola",
         "https://en-us.support.motorola.com/",
         {QStringLiteral("motorola"), QStringLiteral("moto")}},
        {"sony",
         "Sony",
         "https://developer.sony.com/",
         {QStringLiteral("sony"), QStringLiteral("xperia")}},
        {"nokia",
         "Nokia",
         "https://www.hmd.com/support",
         {QStringLiteral("nokia"), QStringLiteral("hmd")}},
        {"asus",
         "ASUS",
         "https://www.asus.com/support/",
         {QStringLiteral("asus"), QStringLiteral("rog phone"), QStringLiteral("zenfone")}},
        {"nothing", "Nothing", "https://support.nothing.tech/", {QStringLiteral("nothing")}},
        {"lenovo", "Lenovo", "https://support.lenovo.com/", {QStringLiteral("lenovo")}},
        {"zte",
         "ZTE",
         "https://support.ztedevices.com/",
         {QStringLiteral("zte"), QStringLiteral("nubia")}},
        {"meizu",
         "Meizu",
         "https://www.meizu.com/en/support",
         {QStringLiteral("meizu"), QStringLiteral("flyme")}},
        {"transsion",
         "Tecno / Infinix",
         "https://www.tecno-mobile.com/support/",
         {QStringLiteral("tecno"), QStringLiteral("infinix"), QStringLiteral("itel")}},
    };
    const Match* selected = nullptr;
    for (const auto& candidate : matches)
    {
        if (std::any_of(candidate.tokens.cbegin(), candidate.tokens.cend(),
                        [&combined](const QString& token) { return combined.contains(token); }))
        {
            selected = &candidate;
            break;
        }
    }
    OemGuide guide;
    guide.key = selected != nullptr ? QString::fromLatin1(selected->key) : QStringLiteral("other");
    guide.displayName =
        selected != nullptr ? QString::fromLatin1(selected->display) : tr("Other manufacturer");
    guide.title = tr("%1 connection setup").arg(guide.displayName);
    guide.officialDocumentationUrl =
        selected != nullptr
            ? QString::fromLatin1(selected->documentation)
            : QStringLiteral("https://developer.android.com/studio/debug/dev-options");
    guide.estimatedMinutes = 5;
    guide.steps = {tr("Open Settings and verify the exact device model and software version."),
                   tr("Enable Developer options by following the manufacturer menu labels."),
                   tr("Enable USB debugging, then reconnect with a data-capable cable."),
                   tr("Select File Transfer when the device asks for a USB mode."),
                   tr("Unlock the device and approve the displayed RSA fingerprint."),
                   tr("Return to ADB Studio and run the connection scan again.")};
    if (guide.key == QStringLiteral("samsung"))
    {
        guide.steps.insert(
            3, tr("Review Samsung USB restrictions and Knox policy before changing them."));
        guide.notes = {
            tr("Game Booster and Knox policies can restrict background or USB behavior."),
            tr("Use the official Samsung USB driver when Windows does not bind an ADB interface.")};
        guide.estimatedMinutes = 7;
    }
    else if (guide.key == QStringLiteral("xiaomi"))
    {
        guide.steps.insert(
            3, tr("Review USB debugging (Security), Install via USB, and Default USB mode."));
        guide.notes = {tr("MIUI or HyperOS may require a signed-in account for security-sensitive "
                          "debugging options."),
                       tr("Do not disable optimization unless a measured problem and official "
                          "guidance require it.")};
        guide.estimatedMinutes = 8;
    }
    else if (guide.key == QStringLiteral("huawei"))
    {
        guide.steps.insert(3, tr("Enable HDB only when the detected Huawei software requires it."));
        guide.notes = {tr("HiSuite and HarmonyOS can use different authorization paths."),
                       tr("Verify the on-device authorization prompt after reconnecting.")};
    }
    else if (guide.key == QStringLiteral("oppo") || guide.key == QStringLiteral("realme"))
    {
        guide.steps.insert(3,
                           tr("Review Permission monitoring and the OEM debugging restrictions."));
        guide.notes = {
            tr("ColorOS-family power and permission controls can stop background sessions.")};
    }
    else if (guide.key == QStringLiteral("vivo"))
    {
        guide.steps.insert(3, tr("Review Vivo authorization and power-management settings."));
        guide.notes = {
            tr("Change power settings only when diagnostics show a background-session failure.")};
    }
    else if (guide.key == QStringLiteral("google"))
    {
        guide.steps.insert(3,
                           tr("Choose USB Preferences and enable Wireless debugging when needed."));
        guide.notes = {tr("Pixel devices use the standard Android Developer options workflow.")};
    }
    return guide;
}

} // namespace adb_studio::diagnostics
