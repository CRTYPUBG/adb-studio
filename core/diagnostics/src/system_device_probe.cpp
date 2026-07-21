#include "adb_studio/diagnostics/system_device_probe.hpp"

#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

#include <algorithm>

namespace adb_studio::diagnostics
{
namespace
{
// Qt value members default-initialize themselves.
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct ProcessResult
{
    int exitCode = -1;
    QString standardOutput;
    QString standardError;
    bool finished = false;
};

// Program, argument list and timeout have distinct semantics at every call site.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto runProcess(const QString& program, const QStringList& arguments,
                int timeoutMs = 5000) -> ProcessResult
{
    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(program, arguments, QIODevice::ReadOnly);
    if (!process.waitForStarted(timeoutMs))
    {
        return {};
    }
    const bool finished = process.waitForFinished(timeoutMs);
    if (!finished)
    {
        process.kill();
        process.waitForFinished(1000);
    }
    return {process.exitCode(), QString::fromUtf8(process.readAllStandardOutput()),
            QString::fromUtf8(process.readAllStandardError()), finished};
}

// The two QString arguments represent distinct executable and transport identifiers.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto adbShell(const QString& adb, const QString& serial, const QStringList& command,
              int timeoutMs = 4000) -> ProcessResult
{
    QStringList arguments = {QStringLiteral("-s"), serial, QStringLiteral("shell")};
    arguments.append(command);
    return runProcess(adb, arguments, timeoutMs);
}

// Measures only bytes delivered by an already-authorized ADB transport. It does not infer a
// physical USB link; callers must verify that evidence before invoking this bounded probe.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto measureTransportMbps(const QString& adb, const QString& serial) -> int
{
    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(adb,
                  {QStringLiteral("-s"), serial, QStringLiteral("exec-out"), QStringLiteral("dd"),
                   QStringLiteral("if=/dev/zero"), QStringLiteral("bs=262144"),
                   QStringLiteral("count=16")},
                  QIODevice::ReadOnly);
    if (!process.waitForStarted(2000))
    {
        return -1;
    }
    QElapsedTimer timer;
    timer.start();
    if (!process.waitForFinished(8000))
    {
        process.kill();
        process.waitForFinished(1000);
        return -1;
    }
    const QByteArray payload = process.readAllStandardOutput();
    const qint64 elapsedMs = timer.elapsed();
    if (process.exitCode() != 0 || payload.isEmpty() || elapsedMs <= 0)
    {
        return -1;
    }
    return static_cast<int>((payload.size() * 8LL) / (elapsedMs * 1000LL));
}

// The source document and property name are deliberately adjacent for readable call sites.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto propertyValue(const QString& properties, const QString& key) -> QString
{
    const QRegularExpression expression(
        QStringLiteral("^\\[%1\\]: \\[(.*)\\]$").arg(QRegularExpression::escape(key)),
        QRegularExpression::MultilineOption);
    const auto match = expression.match(properties);
    return match.hasMatch() ? match.captured(1).trimmed() : QString{};
}

// PnP evidence is kept together so mutually dependent interface/driver facts are applied
// atomically. NOLINTNEXTLINE(readability-function-cognitive-complexity)
void applyWindowsUsbEvidence(DeviceFacts& facts)
{
#ifdef Q_OS_WIN
    const QString powershell = QStandardPaths::findExecutable(QStringLiteral("powershell"));
    if (powershell.isEmpty())
    {
        return;
    }
    const QString command = QStringLiteral(
        "Get-CimInstance Win32_PnPEntity | Select-Object Name,PNPDeviceID,Service,Status,"
        "ConfigManagerErrorCode | ConvertTo-Json -Compress");
    const ProcessResult result =
        runProcess(powershell,
                   {QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"),
                    QStringLiteral("-Command"), command},
                   3000);
    if (!result.finished || result.exitCode != 0)
    {
        return;
    }
    const QJsonDocument document = QJsonDocument::fromJson(result.standardOutput.toUtf8());
    QJsonArray devices;
    if (document.isArray())
    {
        devices = document.array();
    }
    else if (document.isObject())
    {
        devices.append(document.object());
    }
    for (const auto& value : devices)
    {
        const QJsonObject device = value.toObject();
        const QString identifier = device.value(QStringLiteral("PNPDeviceID")).toString();
        const QString name = device.value(QStringLiteral("Name")).toString();
        const QString service = device.value(QStringLiteral("Service")).toString();
        if (!identifier.startsWith(QStringLiteral("USB"), Qt::CaseInsensitive))
        {
            continue;
        }
        const QString searchable = (name + QLatin1Char(' ') + service).toLower();
        const bool androidInterface = searchable.contains(QStringLiteral("android")) ||
                                      searchable.contains(QStringLiteral("adb"));
        const bool mtpInterface = searchable.contains(QStringLiteral("mtp"));
        if (!androidInterface && !mtpInterface)
        {
            continue;
        }
        facts.usbDevicePresent = EvidenceState::Yes;
        const bool healthy =
            device.value(QStringLiteral("Status")).toString() == QStringLiteral("OK") &&
            device.value(QStringLiteral("ConfigManagerErrorCode")).toInt(-1) == 0;
        if (androidInterface)
        {
            facts.usbDriverPresent = healthy ? EvidenceState::Yes : EvidenceState::No;
            facts.windowsDriverPresent = healthy ? EvidenceState::Yes : EvidenceState::No;
        }
        if (mtpInterface)
        {
            facts.mtpDriverPresent = healthy ? EvidenceState::Yes : EvidenceState::No;
            facts.usbMode = healthy ? QStringLiteral("mtp") : facts.usbMode;
        }
    }
#else
    Q_UNUSED(facts)
#endif
}
} // namespace

auto SystemDeviceProbe::parseAdbDevices(const QString& output) -> DeviceFacts
{
    DeviceFacts facts;
    const QStringList lines =
        output.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    for (const QString& rawLine : lines)
    {
        const QString line = rawLine.trimmed();
        if (line.startsWith(QStringLiteral("List of devices")) || line.startsWith(QLatin1Char('*')))
        {
            continue;
        }
        const QStringList fields =
            line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (fields.size() < 2)
        {
            continue;
        }
        ++facts.deviceCount;
        if (facts.serial.isEmpty())
        {
            facts.serial = fields.at(0);
            facts.adbDeviceState = fields.at(1);
            const bool wireless = facts.serial.contains(QLatin1Char(':'));
            facts.wirelessConnected = wireless ? EvidenceState::Yes : EvidenceState::No;
            if (line.contains(QStringLiteral("usb:")))
            {
                facts.usbDevicePresent = EvidenceState::Yes;
                facts.usbDriverPresent = EvidenceState::Yes;
            }
        }
    }
    return facts;
}

// This function is an evidence orchestrator; individual process execution/parsing stays in helpers.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto SystemDeviceProbe::probe() -> DeviceFacts
{
    DeviceFacts facts;
    applyWindowsUsbEvidence(facts);
    const QString adb = QStandardPaths::findExecutable(QStringLiteral("adb"));
    if (adb.isEmpty())
    {
        facts.adbInstalled = EvidenceState::No;
        facts.adbServerRunning = EvidenceState::No;
    }
    else
    {
        facts.adbInstalled = EvidenceState::Yes;
        const ProcessResult version = runProcess(adb, {QStringLiteral("version")}, 2000);
        const QRegularExpression versionExpression(
            QStringLiteral("Android Debug Bridge version ([0-9.]+)"));
        const auto versionMatch = versionExpression.match(version.standardOutput);
        if (versionMatch.hasMatch())
        {
            facts.adbVersion = versionMatch.captured(1);
        }
        const ProcessResult devices =
            runProcess(adb, {QStringLiteral("devices"), QStringLiteral("-l")}, 4000);
        facts.adbServerRunning =
            devices.finished && devices.exitCode == 0 ? EvidenceState::Yes : EvidenceState::No;
        const QString errors = (devices.standardError + version.standardError).toLower();
        facts.adbVersionCompatible = EvidenceState::Unknown;
        if (errors.contains(QStringLiteral("version mismatch")) ||
            errors.contains(QStringLiteral("server version")))
        {
            facts.adbVersionCompatible = EvidenceState::No;
        }
        else if (version.finished)
        {
            facts.adbVersionCompatible = EvidenceState::Yes;
        }
        DeviceFacts devicesFacts = parseAdbDevices(devices.standardOutput);
        facts.deviceCount = devicesFacts.deviceCount;
        facts.serial = devicesFacts.serial;
        facts.adbDeviceState = devicesFacts.adbDeviceState;
        facts.wirelessConnected = devicesFacts.wirelessConnected;
        if (devicesFacts.usbDevicePresent != EvidenceState::Unknown)
        {
            facts.usbDevicePresent = devicesFacts.usbDevicePresent;
        }
        if (devicesFacts.usbDriverPresent != EvidenceState::Unknown)
        {
            facts.usbDriverPresent = devicesFacts.usbDriverPresent;
        }

        if (facts.deviceCount == 1 && facts.adbDeviceState == QStringLiteral("device"))
        {
            const ProcessResult properties =
                adbShell(adb, facts.serial, {QStringLiteral("getprop")}, 3000);
            if (properties.finished && properties.exitCode == 0)
            {
                facts.manufacturer = propertyValue(properties.standardOutput,
                                                   QStringLiteral("ro.product.manufacturer"));
                facts.brand =
                    propertyValue(properties.standardOutput, QStringLiteral("ro.product.brand"));
                facts.model =
                    propertyValue(properties.standardOutput, QStringLiteral("ro.product.model"));
                facts.androidVersion = propertyValue(properties.standardOutput,
                                                     QStringLiteral("ro.build.version.release"));
                facts.buildNumber =
                    propertyValue(properties.standardOutput, QStringLiteral("ro.build.display.id"));
                const QStringList skinProperties = {QStringLiteral("ro.miui.ui.version.name"),
                                                    QStringLiteral("ro.build.version.emui"),
                                                    QStringLiteral("ro.build.version.opporom"),
                                                    QStringLiteral("ro.vivo.os.version"),
                                                    QStringLiteral("ro.oxygen.version")};
                for (const QString& skinProperty : skinProperties)
                {
                    const QString detectedSkin =
                        propertyValue(properties.standardOutput, skinProperty);
                    if (!detectedSkin.isEmpty())
                    {
                        facts.oemSkin = detectedSkin;
                        break;
                    }
                }
                facts.apiLevel =
                    propertyValue(properties.standardOutput, QStringLiteral("ro.build.version.sdk"))
                        .toInt();
                const QString usbConfig = propertyValue(properties.standardOutput,
                                                        QStringLiteral("persist.sys.usb.config"))
                                              .toLower();
                if (!usbConfig.isEmpty())
                {
                    facts.usbDebuggingEnabled = usbConfig.contains(QStringLiteral("adb"))
                                                    ? EvidenceState::Yes
                                                    : EvidenceState::No;
                    if (usbConfig.contains(QStringLiteral("mtp")))
                    {
                        facts.usbMode = QStringLiteral("mtp");
                    }
                    else if (usbConfig.contains(QStringLiteral("none")))
                    {
                        facts.usbMode = QStringLiteral("charging");
                    }
                }
            }
            const ProcessResult root = adbShell(adb, facts.serial, {QStringLiteral("id")}, 2000);
            if (root.finished && root.standardOutput.contains(QStringLiteral("uid=")))
            {
                facts.rooted = root.standardOutput.contains(QStringLiteral("uid=0("))
                                   ? EvidenceState::Yes
                                   : EvidenceState::No;
            }
            const ProcessResult recorder = adbShell(
                adb, facts.serial,
                {QStringLiteral("command"), QStringLiteral("-v"), QStringLiteral("screenrecord")},
                2000);
            if (recorder.finished && recorder.exitCode == 0)
            {
                facts.screenRecordingSupported = recorder.standardOutput.trimmed().isEmpty()
                                                     ? EvidenceState::No
                                                     : EvidenceState::Yes;
            }
            const ProcessResult wireless =
                adbShell(adb, facts.serial,
                         {QStringLiteral("settings"), QStringLiteral("get"),
                          QStringLiteral("global"), QStringLiteral("adb_wifi_enabled")},
                         2000);
            if (wireless.finished && wireless.exitCode == 0)
            {
                const QString value = wireless.standardOutput.trimmed();
                if (value == QStringLiteral("1") || value == QStringLiteral("0"))
                {
                    facts.wirelessAvailable =
                        value == QStringLiteral("1") ? EvidenceState::Yes : EvidenceState::No;
                }
            }
            const ProcessResult display = adbShell(
                adb, facts.serial, {QStringLiteral("dumpsys"), QStringLiteral("display")}, 4000);
            if (display.finished && display.exitCode == 0)
            {
                const QRegularExpression refreshExpression(
                    QStringLiteral("(?:refreshRate|fps)[^0-9]*([0-9]+(?:\\.[0-9]+)?)"),
                    QRegularExpression::CaseInsensitiveOption);
                const auto refreshMatch = refreshExpression.match(display.standardOutput);
                if (refreshMatch.hasMatch())
                {
                    facts.highRefreshSupported = refreshMatch.captured(1).toDouble() > 60.0
                                                     ? EvidenceState::Yes
                                                     : EvidenceState::No;
                }
            }
            const ProcessResult codecs =
                adbShell(adb, facts.serial,
                         {QStringLiteral("dumpsys"), QStringLiteral("media.codec")}, 4000);
            if (codecs.finished && codecs.exitCode == 0)
            {
                facts.codecSupported =
                    codecs.standardOutput.contains(QStringLiteral("video/avc"), Qt::CaseInsensitive)
                        ? EvidenceState::Yes
                        : EvidenceState::No;
            }
            const bool verifiedUsbTransport = facts.usbDevicePresent == EvidenceState::Yes &&
                                              facts.wirelessConnected != EvidenceState::Yes;
            if (verifiedUsbTransport)
            {
                const int firstSample = measureTransportMbps(adb, facts.serial);
                const int secondSample = measureTransportMbps(adb, facts.serial);
                if (firstSample >= 0 && secondSample >= 0)
                {
                    const int lowerSample = std::min(firstSample, secondSample);
                    const int upperSample = std::max(firstSample, secondSample);
                    facts.usbSpeedMbps = (firstSample + secondSample) / 2;
                    facts.usbCableStable = upperSample == 0 || lowerSample * 100 >= upperSample * 70
                                               ? EvidenceState::Yes
                                               : EvidenceState::No;
                }
            }
        }
        if (facts.adbDeviceState == QStringLiteral("recovery"))
        {
            facts.recoveryMode = EvidenceState::Yes;
        }
        else if (facts.adbDeviceState == QStringLiteral("device"))
        {
            facts.recoveryMode = EvidenceState::No;
        }
    }

    const QString fastboot = QStandardPaths::findExecutable(QStringLiteral("fastboot"));
    if (!fastboot.isEmpty())
    {
        const ProcessResult result = runProcess(fastboot, {QStringLiteral("devices")}, 2000);
        if (result.finished && result.exitCode == 0)
        {
            facts.fastbootMode =
                result.standardOutput.trimmed().isEmpty() ? EvidenceState::No : EvidenceState::Yes;
        }
    }
    return facts;
}

auto SystemDeviceProbe::restartAdbServer() -> bool
{
#ifdef Q_OS_WIN
    const QString pnputil = QStandardPaths::findExecutable(QStringLiteral("pnputil"));
    if (!pnputil.isEmpty())
    {
        runProcess(pnputil, {QStringLiteral("/scan-devices")}, 4000);
    }
#endif
    const QString adb = QStandardPaths::findExecutable(QStringLiteral("adb"));
    if (adb.isEmpty())
    {
        return false;
    }
    const ProcessResult killed = runProcess(adb, {QStringLiteral("kill-server")});
    const ProcessResult started = runProcess(adb, {QStringLiteral("start-server")}, 4000);
    if (!killed.finished || !started.finished || started.exitCode != 0)
    {
        return false;
    }
    const ProcessResult offline =
        runProcess(adb, {QStringLiteral("reconnect"), QStringLiteral("offline")}, 3000);
    const ProcessResult device =
        runProcess(adb, {QStringLiteral("reconnect"), QStringLiteral("device")}, 3000);
    return offline.finished && device.finished;
}

auto SystemDeviceProbe::probeWithRecovery() -> DeviceFacts
{
    DeviceFacts initial = probe();
    const bool failedTransport =
        initial.adbInstalled == EvidenceState::Yes && initial.fastbootMode != EvidenceState::Yes &&
        (initial.deviceCount == 0 || initial.adbDeviceState == QStringLiteral("offline") ||
         initial.adbServerRunning == EvidenceState::No);
    if (!failedTransport)
    {
        return initial;
    }
    if (!restartAdbServer())
    {
        initial.recoveryAttempted = true;
        return initial;
    }
    DeviceFacts recovered = probe();
    recovered.recoveryAttempted = true;
    return recovered;
}

} // namespace adb_studio::diagnostics
