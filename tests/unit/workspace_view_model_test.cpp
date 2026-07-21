#include "adb_studio/presentation/workspace_view_model.hpp"

#include <QSignalSpy>
#include <QtTest>

using adb_studio::WorkspaceViewModel;

class WorkspaceViewModelTest final : public QObject
{
    Q_OBJECT

  private slots:
    void exposesEverySidebarModule()
    {
        const WorkspaceViewModel viewModel;
        QCOMPARE(viewModel.modules().size(), 4);
        QCOMPARE(viewModel.currentModuleKey(), QStringLiteral("dashboard"));
        QVERIFY(!viewModel.features().isEmpty());
    }

    void navigationSearchAndUndoAreStateful()
    {
        WorkspaceViewModel viewModel;
        viewModel.selectModule(QStringLiteral("devices"));
        QCOMPARE(viewModel.currentModuleKey(), QStringLiteral("devices"));
        QVERIFY(viewModel.canUndo());

        viewModel.setSearchText(QStringLiteral("Fastboot"));
        QCOMPARE(viewModel.features().size(), 1);
        viewModel.undo();
        QCOMPARE(viewModel.currentModuleKey(), QStringLiteral("dashboard"));
    }

    void onlyVerifiedCommandsAreDispatched()
    {
        WorkspaceViewModel viewModel;
        QSignalSpy commandSpy(&viewModel, &WorkspaceViewModel::commandRequested);
        viewModel.selectModule(QStringLiteral("devices"));
        viewModel.activateFeature(QStringLiteral("scan"));
        QCOMPARE(commandSpy.count(), 1);
        QCOMPARE(commandSpy.constFirst().constFirst().toString(), QStringLiteral("scan-devices"));

        viewModel.activateFeature(QStringLiteral("unregistered"));
        QCOMPARE(commandSpy.count(), 1);
        QVERIFY(!viewModel.errorMessage().isEmpty());
    }

    void refreshUsesLoadingAndCompletionStates()
    {
        WorkspaceViewModel viewModel;
        viewModel.refresh();
        QVERIFY(viewModel.busy());
        QTRY_VERIFY_WITH_TIMEOUT(!viewModel.busy(), 1000);
        QVERIFY(!viewModel.notification().isEmpty());
    }
};

QTEST_GUILESS_MAIN(WorkspaceViewModelTest)
#include "workspace_view_model_test.moc"
