#include "adb_studio/presentation/device_dashboard_view_model.hpp"
#include "adb_studio/presentation/theme_service.hpp"
#include "adb_studio/version.hpp"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>
#include <QVariant>

int main(int argc, char* argv[])
{
    QElapsedTimer startupTimer;
    startupTimer.start();
    QQuickStyle::setStyle(QStringLiteral("FluentWinUI3"));
    QGuiApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("ADB Studio"));
    application.setOrganizationName(QStringLiteral("ADB Studio Community"));
    application.setApplicationVersion(QStringLiteral(ADB_STUDIO_VERSION));
    application.setWindowIcon(QIcon(QStringLiteral(":/assets/app-icon-adb.png")));

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
    parser.process(application);

    adb_studio::ThemeService themeService;
    adb_studio::DeviceDashboardViewModel dashboard;

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {QStringLiteral("themeService"), QVariant::fromValue(&themeService)},
        {QStringLiteral("dashboardViewModel"), QVariant::fromValue(&dashboard)},
        {QStringLiteral("applicationVersion"), application.applicationVersion()},
    });
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ADBStudio/UI/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
    {
        return 1;
    }
    if (parser.isSet(startupOption))
    {
        qInfo().nospace() << "ADB_STUDIO_STARTUP_MS=" << startupTimer.elapsed();
    }
    if (parser.isSet(screenshotOption))
    {
        const QString screenshotPath = parser.value(screenshotOption);
        QTimer::singleShot(1000, &application, [&application, &engine, screenshotPath]() {
            auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst());
            if (window == nullptr || screenshotPath.isEmpty() ||
                !window->grabWindow().save(screenshotPath))
            {
                application.exit(2);
                return;
            }
            application.quit();
        });
    }
    return application.exec();
}
