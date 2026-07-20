#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace adb_studio
{

class DeviceDashboardViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("DeviceDashboardViewModel is provided by the application")
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString manufacturer READ manufacturer NOTIFY deviceChanged)
    Q_PROPERTY(QString model READ model NOTIFY deviceChanged)
    Q_PROPERTY(QString androidVersion READ androidVersion NOTIFY deviceChanged)
    Q_PROPERTY(int apiLevel READ apiLevel NOTIFY deviceChanged)
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY metricsChanged)
    Q_PROPERTY(QString storage READ storage NOTIFY metricsChanged)
    Q_PROPERTY(QString cpu READ cpu NOTIFY metricsChanged)
    Q_PROPERTY(QString memory READ memory NOTIFY metricsChanged)
  public:
    explicit DeviceDashboardViewModel(QObject* parent = nullptr)
        : QObject(parent), m_state(tr("No devices connected"))
    {
    }

    bool connected() const noexcept
    {
        return m_connected;
    }
    QString state() const
    {
        return m_state;
    }
    QString manufacturer() const
    {
        return m_manufacturer;
    }
    QString model() const
    {
        return m_model;
    }
    QString androidVersion() const
    {
        return m_androidVersion;
    }
    int apiLevel() const noexcept
    {
        return m_apiLevel;
    }
    int batteryPercent() const noexcept
    {
        return m_batteryPercent;
    }
    QString storage() const
    {
        return m_storage;
    }
    QString cpu() const
    {
        return m_cpu;
    }
    QString memory() const
    {
        return m_memory;
    }

    Q_INVOKABLE void refresh()
    {
        emit refreshRequested();
    }

  signals:
    void connectedChanged();
    void stateChanged();
    void deviceChanged();
    void metricsChanged();
    void refreshRequested();

  private:
    bool m_connected = false;
    QString m_state;
    QString m_manufacturer;
    QString m_model;
    QString m_androidVersion;
    int m_apiLevel = 0;
    int m_batteryPercent = 0;
    QString m_storage;
    QString m_cpu;
    QString m_memory;
};

} // namespace adb_studio
