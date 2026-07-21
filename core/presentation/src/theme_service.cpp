#include "adb_studio/presentation/theme_service.hpp"

#include <QGuiApplication>
#include <QPalette>
#include <QSettings>
#include <QStyleHints>

namespace adb_studio
{

ThemeService::ThemeService(QObject* parent) : QObject(parent)
{
    const auto* hints = QGuiApplication::styleHints();
    connect(hints, &QStyleHints::colorSchemeChanged, this, &ThemeService::refreshSystemState);
    const int storedMode = QSettings().value(QStringLiteral("ui/themeMode"), 0).toInt();
    if (storedMode >= static_cast<int>(ThemeMode::System) &&
        storedMode <= static_cast<int>(ThemeMode::Dark))
    {
        setThemeMode(static_cast<ThemeMode>(storedMode));
    }
    refreshSystemState();
}

void ThemeService::setThemeMode(ThemeMode mode)
{
    if (m_themeMode == mode)
    {
        return;
    }
    m_themeMode = mode;
    QSettings().setValue(QStringLiteral("ui/themeMode"), static_cast<int>(mode));
    switch (mode)
    {
    case ThemeMode::System:
        QGuiApplication::styleHints()->unsetColorScheme();
        break;
    case ThemeMode::Light:
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
        break;
    case ThemeMode::Dark:
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
        break;
    }
    emit themeModeChanged();
    refreshDarkState();
}

void ThemeService::setHighContrast(bool enabled)
{
    if (m_highContrast == enabled)
    {
        return;
    }
    m_highContrast = enabled;
    emit highContrastChanged();
}

void ThemeService::setReducedMotion(bool enabled)
{
    if (m_reducedMotion == enabled)
    {
        return;
    }
    m_reducedMotion = enabled;
    emit reducedMotionChanged();
}

void ThemeService::refreshSystemState()
{
    refreshDarkState();
    refreshAccent();
}

void ThemeService::refreshDarkState()
{
    const auto systemScheme = QGuiApplication::styleHints()->colorScheme();
    const bool dark = m_themeMode == ThemeMode::Dark ||
                      (m_themeMode == ThemeMode::System && systemScheme == Qt::ColorScheme::Dark);
    if (m_dark != dark)
    {
        m_dark = dark;
        emit darkChanged();
    }
}

void ThemeService::refreshAccent()
{
    const QColor systemAccent = QGuiApplication::palette().accent().color();
    const QColor accent = systemAccent.isValid() ? systemAccent : QColor(QStringLiteral("#2563EB"));
    if (m_accent != accent)
    {
        m_accent = accent;
        emit accentChanged();
    }
}

} // namespace adb_studio
