#include "adb_studio/presentation/theme_service.hpp"

#include <QtTest>

class ThemeServiceTest final : public QObject
{
    Q_OBJECT
  private slots:
    void supportsExplicitSchemes()
    {
        adb_studio::ThemeService service;
        service.setThemeMode(adb_studio::ThemeService::ThemeMode::Dark);
        QVERIFY(service.dark());
        service.setThemeMode(adb_studio::ThemeService::ThemeMode::Light);
        QVERIFY(!service.dark());
    }

    void exposesValidTokens()
    {
        const adb_studio::ThemeService service;
        QVERIFY(service.accent().isValid());
        QVERIFY(!service.success().isEmpty());
        QVERIFY(!service.fontFamily().isEmpty());
    }
};

QTEST_MAIN(ThemeServiceTest)
#include "theme_service_test.moc"
