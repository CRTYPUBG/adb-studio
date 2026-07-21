#include "adb_studio/presentation/mirror_view_model.hpp"
#include "adb_studio/services/mirror_service.hpp"

#include <QSignalSpy>
#include <QtTest>

using adb_studio::MirrorViewModel;
using adb_studio::services::AdbService;
using adb_studio::services::DeviceManagerService;
using adb_studio::services::MirrorService;
using adb_studio::services::MirrorSettings;

class MirrorServiceIntegrationTest final : public QObject
{
    Q_OBJECT

  private slots:
    void verifiesBundledTools()
    {
        const AdbService adb;
        const DeviceManagerService devices(adb);
        MirrorService mirror(adb, devices);
        QVERIFY(adb.available());
        QVERIFY(mirror.available());
        QVERIFY(mirror.version().startsWith(QStringLiteral("scrcpy 4.0")));
    }

    void reportsChildProcessFailureDeterministically()
    {
        const AdbService adb;
        const DeviceManagerService devices(adb);
        MirrorService mirror(adb, devices, nullptr,
                             QString::fromUtf8(ADB_STUDIO_PROCESS_FAILURE_FIXTURE));
        QSignalSpy failureSpy(&mirror, &MirrorService::sessionFailed);
        QVERIFY(mirror.start(QStringLiteral("fixture-device"), MirrorSettings{}));
        QTRY_COMPARE_WITH_TIMEOUT(failureSpy.count(), 1, 1000);
        QVERIFY(!failureSpy.constFirst().at(1).toString().isEmpty());
        QVERIFY(!failureSpy.constFirst().at(2).toString().isEmpty());
        QVERIFY(failureSpy.constFirst().at(3).toString().contains(
            QStringLiteral("deterministic process failure")));
        QVERIFY(mirror.activeSerials().isEmpty());
    }

    void viewModelValidatesSettingsAndSelection()
    {
        const AdbService adb;
        DeviceManagerService devices(adb);
        MirrorService mirror(adb, devices);
        MirrorViewModel viewModel(devices, mirror);
        QVERIFY(viewModel.engineAvailable());

        viewModel.setMaxFps(0);
        QCOMPARE(viewModel.maxFps(), 60);
        viewModel.setCodec(QStringLiteral("invalid"));
        QCOMPARE(viewModel.codec(), QStringLiteral("h264"));
        viewModel.start();
        QVERIFY(!viewModel.errorCause().isEmpty());
        QVERIFY(!viewModel.errorSolution().isEmpty());
    }
};

QTEST_GUILESS_MAIN(MirrorServiceIntegrationTest)
#include "mirror_service_integration_test.moc"
