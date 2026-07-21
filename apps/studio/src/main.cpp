#include "adb_studio/presentation/device_dashboard_view_model.hpp"
#include "adb_studio/presentation/language_service.hpp"
#include "adb_studio/presentation/mirror_view_model.hpp"
#include "adb_studio/presentation/theme_service.hpp"
#include "adb_studio/presentation/workspace_view_model.hpp"
#include "adb_studio/services/mirror_service.hpp"
#include "adb_studio/version.hpp"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QResource>
#include <QSaveFile>
#include <QTimer>
#include <QUrl>
#include <QVariant>

auto main(int argc, char* argv[]) -> int
{
    QElapsedTimer startupTimer;
    startupTimer.start();
    QQuickStyle::setStyle(QStringLiteral("FluentWinUI3"));
    QGuiApplication application(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("ADB Studio"));
    QGuiApplication::setOrganizationName(QStringLiteral("CRTY"));
    QGuiApplication::setApplicationVersion(QStringLiteral(ADB_STUDIO_VERSION));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/app-icon-adb.png")));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("ADB Studio Android device management"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption screenshotOption(
        QStringLiteral("render-screenshot"),
        QStringLiteral("Render the application shell to an image and exit."),
        QStringLiteral("path"));
    parser.addOption(screenshotOption);
    const QCommandLineOption startupOption(
        QStringLiteral("report-startup"),
        QStringLiteral("Write time to first QML root object to the diagnostic log."));
    parser.addOption(startupOption);
    const QCommandLineOption workspaceOption(
        QStringLiteral("workspace"), QStringLiteral("Open a registered workspace at startup."),
        QStringLiteral("key"), QStringLiteral("dashboard"));
    parser.addOption(workspaceOption);
    const QCommandLineOption languageOption(
        QStringLiteral("language"), QStringLiteral("Select the application language (en or tr)."),
        QStringLiteral("code"));
    parser.addOption(languageOption);
    parser.process(application);

    // Qt's resource initialization macro intentionally expands to a guarded do-while statement.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    Q_INIT_RESOURCE(adb_studio_translations);
    adb_studio::ThemeService themeService;
    adb_studio::LanguageService languageService;
    if (parser.isSet(languageOption))
    {
        languageService.setLanguageCode(parser.value(languageOption));
    }
    adb_studio::DeviceDashboardViewModel dashboard;
    adb_studio::WorkspaceViewModel workspace;
    adb_studio::services::AdbService adbService;
    adb_studio::services::DeviceManagerService deviceManager(adbService);
    adb_studio::services::MirrorService mirrorService(adbService, deviceManager);
    adb_studio::MirrorViewModel mirrorViewModel(deviceManager, mirrorService);
    QObject::connect(&dashboard, &adb_studio::DeviceDashboardViewModel::refreshRequested,
                     &dashboard, &adb_studio::DeviceDashboardViewModel::startScan);
    QObject::connect(&dashboard, &adb_studio::DeviceDashboardViewModel::openUrlRequested,
                     &application, [](const QUrl& url) { QDesktopServices::openUrl(url); });
    QObject::connect(&mirrorViewModel, &adb_studio::MirrorViewModel::openUrlRequested, &application,
                     [](const QUrl& url) { QDesktopServices::openUrl(url); });
    QObject::connect(&workspace, &adb_studio::WorkspaceViewModel::commandRequested, &application,
                     [&dashboard](const QString& command) {
                         if (command == QStringLiteral("scan-devices"))
                         {
                             dashboard.startScan();
                         }
                         else if (command == QStringLiteral("restart-adb"))
                         {
                             dashboard.recoverConnection();
                         }
                         else if (command == QStringLiteral("oem-guide"))
                         {
                             dashboard.openOemGuide();
                         }
                         else if (command == QStringLiteral("wireless-guide"))
                         {
                             dashboard.openWirelessSetup();
                         }
                     });
    workspace.selectModule(parser.value(workspaceOption));

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {QStringLiteral("themeService"), QVariant::fromValue(&themeService)},
        {QStringLiteral("languageService"), QVariant::fromValue(&languageService)},
        {QStringLiteral("dashboardViewModel"), QVariant::fromValue(&dashboard)},
        {QStringLiteral("workspaceViewModel"), QVariant::fromValue(&workspace)},
        {QStringLiteral("mirrorViewModel"), QVariant::fromValue(&mirrorViewModel)},
        {QStringLiteral("applicationVersion"), QGuiApplication::applicationVersion()},
    });
    QObject::connect(&languageService, &adb_studio::LanguageService::retranslateRequested, &engine,
                     [&engine, &workspace, &mirrorViewModel, &dashboard]() {
                         engine.retranslate();
                         workspace.retranslate();
                         mirrorViewModel.retranslate();
                         dashboard.retranslate();
                     });
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ADBStudio/UI/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
    {
        return 1;
    }
    if (!parser.isSet(screenshotOption))
    {
        dashboard.startScan();
    }
    if (parser.isSet(startupOption))
    {
        const qint64 startupMilliseconds = startupTimer.elapsed();
        qInfo().nospace() << "ADB_STUDIO_STARTUP_MS=" << startupMilliseconds;
        const QString reportPath = qEnvironmentVariable("ADB_STUDIO_STARTUP_REPORT");
        if (!reportPath.isEmpty())
        {
            QSaveFile report(reportPath);
            if (!report.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                return 3;
            }
            report.write(QByteArray::number(startupMilliseconds));
            report.write("\n");
            if (!report.commit())
            {
                return 3;
            }
        }
    }
    if (parser.isSet(screenshotOption))
    {
        const QString screenshotPath = parser.value(screenshotOption);
        QTimer::singleShot(1000, &application, [&application, &engine, screenshotPath]() {
            auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());
            if (window == nullptr || screenshotPath.isEmpty() ||
                !window->grabWindow().save(screenshotPath))
            {
                QGuiApplication::exit(2);
                return;
            }
            QGuiApplication::quit();
        });
    }
    return QGuiApplication::exec();
}
