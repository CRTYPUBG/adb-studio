#include "adb_studio/diagnostics/device_diagnostics.hpp"
#include "adb_studio/diagnostics/system_device_probe.hpp"

#include <QtTest>

#include <algorithm>

using adb_studio::diagnostics::ConnectionDiagnosticsEngine;
using adb_studio::diagnostics::ConnectionState;
using adb_studio::diagnostics::DeviceFacts;
using adb_studio::diagnostics::EvidenceState;
using adb_studio::diagnostics::OemGuideCatalog;
using adb_studio::diagnostics::SystemDeviceProbe;

class DeviceDiagnosticsTest final : public QObject
{
    Q_OBJECT

  private slots:
    void parsesAdbDeviceFacts()
    {
        const DeviceFacts facts = SystemDeviceProbe::parseAdbDevices(QStringLiteral(
            "List of devices attached\nABC123 device product:x model:Pixel usb:1-2\n"));
        QCOMPARE(facts.deviceCount, 1);
        QCOMPARE(facts.serial, QStringLiteral("ABC123"));
        QCOMPARE(facts.adbDeviceState, QStringLiteral("device"));
        QCOMPARE(facts.usbDevicePresent, EvidenceState::Yes);
        QCOMPARE(facts.wirelessConnected, EvidenceState::No);
    }

    void detectsMultipleDevices()
    {
        DeviceFacts facts;
        facts.adbInstalled = EvidenceState::Yes;
        facts.adbServerRunning = EvidenceState::Yes;
        facts.adbVersionCompatible = EvidenceState::Yes;
        facts.deviceCount = 2;
        const auto report = ConnectionDiagnosticsEngine::analyze(facts);
        QCOMPARE(report.state, ConnectionState::MultipleDevices);
        QCOMPARE(report.stateKey, QStringLiteral("MULTIPLE_DEVICES"));
    }

    void doesNotGuessUnknownUsbCause()
    {
        DeviceFacts facts;
        facts.adbInstalled = EvidenceState::Yes;
        facts.adbServerRunning = EvidenceState::Yes;
        facts.adbVersionCompatible = EvidenceState::Yes;
        const auto report = ConnectionDiagnosticsEngine::analyze(facts);
        QCOMPARE(report.state, ConnectionState::Unknown);
        QCOMPARE(report.stateKey, QStringLiteral("UNKNOWN"));
    }

    void identifiesUnauthorizedTransport()
    {
        DeviceFacts facts;
        facts.adbInstalled = EvidenceState::Yes;
        facts.adbServerRunning = EvidenceState::Yes;
        facts.adbVersionCompatible = EvidenceState::Yes;
        facts.deviceCount = 1;
        facts.adbDeviceState = QStringLiteral("unauthorized");
        const auto report = ConnectionDiagnosticsEngine::analyze(facts);
        QCOMPARE(report.state, ConnectionState::Unauthorized);
        QVERIFY(!report.recommendedAction.isEmpty());
    }

    void selectsManufacturerSpecificGuide()
    {
        DeviceFacts facts;
        facts.manufacturer = QStringLiteral("Xiaomi");
        facts.brand = QStringLiteral("POCO");
        facts.oemSkin = QStringLiteral("HyperOS");
        const auto guide = OemGuideCatalog::detect(facts);
        QCOMPARE(guide.key, QStringLiteral("xiaomi"));
        QVERIFY(guide.steps.size() >= 6);
        QVERIFY(guide.officialDocumentationUrl.startsWith(QStringLiteral("https://")));
    }

    void healthScoreIsBoundedAndHasConfidence()
    {
        DeviceFacts facts;
        facts.adbInstalled = EvidenceState::Yes;
        facts.adbServerRunning = EvidenceState::Yes;
        facts.adbVersionCompatible = EvidenceState::Yes;
        facts.adbDeviceState = QStringLiteral("device");
        facts.deviceCount = 1;
        facts.rooted = EvidenceState::No;
        const auto report = ConnectionDiagnosticsEngine::analyze(facts);
        QVERIFY(report.health.total >= 0);
        QVERIFY(report.health.total <= 100);
        QVERIFY(report.health.confidence > 0);
    }

    void reportsMeasuredUsbStabilityAndRequiredPlatformChecks()
    {
        DeviceFacts facts;
        facts.usbSpeedMbps = 240;
        facts.usbCableStable = EvidenceState::No;
        facts.windowsDriverPresent = EvidenceState::Yes;
        facts.recoveryMode = EvidenceState::No;
        const auto diagnostics = ConnectionDiagnosticsEngine::analyze(facts).diagnostics;
        const auto findById = [&diagnostics](const QString& id) {
            return std::find_if(diagnostics.cbegin(), diagnostics.cend(),
                                [&id](const auto& item) { return item.id == id; });
        };

        const auto cable = findById(QStringLiteral("usb-cable"));
        QVERIFY(cable != diagnostics.cend());
        QCOMPARE(cable->status, adb_studio::diagnostics::DiagnosticStatus::Warning);
        const auto windowsDriver = findById(QStringLiteral("windows-driver"));
        QVERIFY(windowsDriver != diagnostics.cend());
        QCOMPARE(windowsDriver->status, adb_studio::diagnostics::DiagnosticStatus::Pass);
        const auto recovery = findById(QStringLiteral("recovery"));
        QVERIFY(recovery != diagnostics.cend());
        QCOMPARE(recovery->status, adb_studio::diagnostics::DiagnosticStatus::Pass);
    }
};

QTEST_GUILESS_MAIN(DeviceDiagnosticsTest)
#include "device_diagnostics_test.moc"
