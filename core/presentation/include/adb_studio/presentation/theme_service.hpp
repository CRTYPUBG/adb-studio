#pragma once

#include <QColor>
#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace adb_studio
{

class ThemeService : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("ThemeService is provided by the application")
    Q_PROPERTY(ThemeMode themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(bool dark READ dark NOTIFY darkChanged)
    Q_PROPERTY(bool highContrast READ highContrast NOTIFY highContrastChanged)
    Q_PROPERTY(bool reducedMotion READ reducedMotion NOTIFY reducedMotionChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY accentChanged)
    Q_PROPERTY(QString success READ success CONSTANT)
    Q_PROPERTY(QString warning READ warning CONSTANT)
    Q_PROPERTY(QString error READ error CONSTANT)
    Q_PROPERTY(QString secondary READ secondary CONSTANT)
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
  public:
    enum class ThemeMode
    {
        System,
        Light,
        Dark
    };
    Q_ENUM(ThemeMode)

    explicit ThemeService(QObject* parent = nullptr);

    ThemeMode themeMode() const noexcept
    {
        return m_themeMode;
    }
    bool dark() const noexcept
    {
        return m_dark;
    }
    bool highContrast() const noexcept
    {
        return m_highContrast;
    }
    bool reducedMotion() const noexcept
    {
        return m_reducedMotion;
    }
    QColor accent() const
    {
        return m_accent;
    }
    QString success() const
    {
        return QStringLiteral("#22C55E");
    }
    QString warning() const
    {
        return QStringLiteral("#F59E0B");
    }
    QString error() const
    {
        return QStringLiteral("#DC2626");
    }
    QString secondary() const
    {
        return QStringLiteral("#06B6D4");
    }
    QString fontFamily() const
    {
        return QStringLiteral("Segoe UI Variable, Inter, Noto Sans");
    }

  public slots:
    void setThemeMode(ThemeMode mode);
    void setHighContrast(bool enabled);
    void setReducedMotion(bool enabled);

  signals:
    void themeModeChanged();
    void darkChanged();
    void highContrastChanged();
    void reducedMotionChanged();
    void accentChanged();

  private slots:
    void refreshSystemState();

  private:
    void refreshDarkState();
    void refreshAccent();

    ThemeMode m_themeMode = ThemeMode::System;
    bool m_dark = false;
    bool m_highContrast = false;
    bool m_reducedMotion = false;
    QColor m_accent;
};

} // namespace adb_studio
