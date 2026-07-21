#include "adb_studio/presentation/device_dashboard_view_model.hpp"

#include <QSignalSpy>
#include <QtTest>

class DeviceDashboardViewModelTest final : public QObject
{
    Q_OBJECT
  private slots:
    void startsDisconnected()
    {
        const adb_studio::DeviceDashboardViewModel viewModel;
        QVERIFY(!viewModel.connected());
        QVERIFY(!viewModel.state().isEmpty());
        QVERIFY(!viewModel.usbStatus().isEmpty());
        QVERIFY(!viewModel.wirelessStatus().isEmpty());
    }

    void requestsRefresh()
    {
        adb_studio::DeviceDashboardViewModel viewModel;
        const QSignalSpy spy(&viewModel, &adb_studio::DeviceDashboardViewModel::refreshRequested);
        viewModel.refresh();
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_GUILESS_MAIN(DeviceDashboardViewModelTest)
#include "device_dashboard_view_model_test.moc"
