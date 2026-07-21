#include "adb_studio/diagnostics/system_device_probe.hpp"

#include <QElapsedTimer>
#include <QtTest>

class SystemDeviceProbeIntegrationTest final : public QObject
{
    Q_OBJECT

  private slots:
    void probeCompletesWithExplicitAdbEvidence()
    {
        QElapsedTimer timer;
        timer.start();
        const auto facts = adb_studio::diagnostics::SystemDeviceProbe::probe();
        QVERIFY(timer.elapsed() < 15000);
        QVERIFY(facts.adbInstalled != adb_studio::diagnostics::EvidenceState::Unknown);
        QVERIFY(facts.deviceCount >= 0);
    }
};

QTEST_GUILESS_MAIN(SystemDeviceProbeIntegrationTest)
#include "system_device_probe_integration_test.moc"
