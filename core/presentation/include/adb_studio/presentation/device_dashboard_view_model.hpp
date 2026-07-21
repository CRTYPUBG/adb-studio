#pragma once

#include "adb_studio/diagnostics/device_diagnostics.hpp"

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace adb_studio
{

class DeviceDashboardViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("DeviceDashboardViewModel is provided by the application")
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString connectionStateKey READ connectionStateKey NOTIFY diagnosticsChanged)
    Q_PROPERTY(QString explanation READ explanation NOTIFY diagnosticsChanged)
    Q_PROPERTY(QString recommendedAction READ recommendedAction NOTIFY diagnosticsChanged)
    Q_PROPERTY(QString currentDetectionStep READ currentDetectionStep NOTIFY diagnosticsChanged)
    Q_PROPERTY(int healthScore READ healthScore NOTIFY diagnosticsChanged)
    Q_PROPERTY(int healthConfidence READ healthConfidence NOTIFY diagnosticsChanged)
    Q_PROPERTY(QVariantList diagnostics READ diagnostics NOTIFY diagnosticsChanged)
    Q_PROPERTY(bool oemDetected READ oemDetected NOTIFY diagnosticsChanged)
    Q_PROPERTY(QString oemGuideTitle READ oemGuideTitle NOTIFY diagnosticsChanged)
    Q_PROPERTY(QStringList oemGuideSteps READ oemGuideSteps NOTIFY diagnosticsChanged)
    Q_PROPERTY(QStringList oemGuideNotes READ oemGuideNotes NOTIFY diagnosticsChanged)
    Q_PROPERTY(int oemGuideMinutes READ oemGuideMinutes NOTIFY diagnosticsChanged)
    Q_PROPERTY(QString manufacturer READ manufacturer NOTIFY deviceChanged)
    Q_PROPERTY(QString brand READ brand NOTIFY deviceChanged)
    Q_PROPERTY(QString model READ model NOTIFY deviceChanged)
    Q_PROPERTY(QString androidVersion READ androidVersion NOTIFY deviceChanged)
    Q_PROPERTY(QString buildNumber READ buildNumber NOTIFY deviceChanged)
    Q_PROPERTY(QString oemSkin READ oemSkin NOTIFY deviceChanged)
    Q_PROPERTY(int apiLevel READ apiLevel NOTIFY deviceChanged)
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY metricsChanged)
    Q_PROPERTY(QString storage READ storage NOTIFY metricsChanged)
    Q_PROPERTY(QString cpu READ cpu NOTIFY metricsChanged)
    Q_PROPERTY(QString memory READ memory NOTIFY metricsChanged)
    Q_PROPERTY(QString usbStatus READ usbStatus NOTIFY diagnosticsChanged)
    Q_PROPERTY(QString wirelessStatus READ wirelessStatus NOTIFY diagnosticsChanged)

  public:
    explicit DeviceDashboardViewModel(QObject* parent = nullptr);

    [[nodiscard]] bool connected() const noexcept;
    [[nodiscard]] bool scanning() const noexcept;
    [[nodiscard]] QString state() const;
    [[nodiscard]] QString connectionStateKey() const;
    [[nodiscard]] QString explanation() const;
    [[nodiscard]] QString recommendedAction() const;
    [[nodiscard]] QString currentDetectionStep() const;
    [[nodiscard]] int healthScore() const noexcept;
    [[nodiscard]] int healthConfidence() const noexcept;
    [[nodiscard]] QVariantList diagnostics() const;
    [[nodiscard]] bool oemDetected() const noexcept;
    [[nodiscard]] QString oemGuideTitle() const;
    [[nodiscard]] QStringList oemGuideSteps() const;
    [[nodiscard]] QStringList oemGuideNotes() const;
    [[nodiscard]] int oemGuideMinutes() const noexcept;
    [[nodiscard]] QString manufacturer() const;
    [[nodiscard]] QString brand() const;
    [[nodiscard]] QString model() const;
    [[nodiscard]] QString androidVersion() const;
    [[nodiscard]] QString buildNumber() const;
    [[nodiscard]] QString oemSkin() const;
    [[nodiscard]] int apiLevel() const noexcept;
    [[nodiscard]] int batteryPercent() const noexcept;
    [[nodiscard]] QString storage() const;
    [[nodiscard]] QString cpu() const;
    [[nodiscard]] QString memory() const;
    [[nodiscard]] QString usbStatus() const;
    [[nodiscard]] QString wirelessStatus() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void recoverConnection();
    Q_INVOKABLE void openOemGuide();
    Q_INVOKABLE void openWirelessSetup();
    Q_INVOKABLE void openDriverGuide();
    Q_INVOKABLE void retranslate();

  public slots:
    void startScan();

  signals:
    void connectedChanged();
    void scanningChanged();
    void stateChanged();
    void deviceChanged();
    void metricsChanged();
    void diagnosticsChanged();
    void refreshRequested();
    void openUrlRequested(const QUrl& url);

  private:
    void launchScan(bool forceRecovery);
    void applyReport(const diagnostics::ConnectionReport& report);
    void setScanning(bool scanning);

    bool m_connected = false;
    bool m_scanning = false;
    QString m_state;
    QString m_connectionStateKey = QStringLiteral("UNKNOWN");
    QString m_explanation;
    QString m_recommendedAction;
    QString m_currentDetectionStep;
    int m_healthScore = 0;
    int m_healthConfidence = 0;
    QVariantList m_diagnostics;
    bool m_oemDetected = false;
    QString m_oemGuideTitle;
    QStringList m_oemGuideSteps;
    QStringList m_oemGuideNotes;
    int m_oemGuideMinutes = 0;
    QUrl m_oemGuideUrl;
    QString m_manufacturer;
    QString m_brand;
    QString m_model;
    QString m_androidVersion;
    QString m_buildNumber;
    QString m_oemSkin;
    int m_apiLevel = 0;
    int m_batteryPercent = 0;
    QString m_storage;
    QString m_cpu;
    QString m_memory;
    QString m_usbStatus;
    QString m_wirelessStatus;
    QFutureWatcher<diagnostics::ConnectionReport> m_scanWatcher;
};

} // namespace adb_studio
