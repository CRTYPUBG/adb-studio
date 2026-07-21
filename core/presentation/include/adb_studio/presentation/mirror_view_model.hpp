#pragma once

#include "adb_studio/services/mirror_service.hpp"

#include <QFutureWatcher>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace adb_studio
{

class MirrorViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("MirrorViewModel is provided by the application")
    Q_PROPERTY(bool engineAvailable READ engineAvailable CONSTANT)
    Q_PROPERTY(QString engineVersion READ engineVersion NOTIFY engineChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QString selectedSerial READ selectedSerial WRITE setSelectedSerial NOTIFY
                   selectedSerialChanged)
    Q_PROPERTY(QStringList activeSessions READ activeSessions NOTIFY sessionChanged)
    Q_PROPERTY(bool active READ active NOTIFY sessionChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY sessionChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorCause READ errorCause NOTIFY errorChanged)
    Q_PROPERTY(QString errorSolution READ errorSolution NOTIFY errorChanged)
    Q_PROPERTY(QString technicalDetails READ technicalDetails NOTIFY errorChanged)
    Q_PROPERTY(QStringList logs READ logs NOTIFY logsChanged)
    Q_PROPERTY(double fps READ fps NOTIFY statisticsChanged)
    Q_PROPERTY(int latencyMs READ latencyMs NOTIFY statisticsChanged)
    Q_PROPERTY(int maxSize READ maxSize WRITE setMaxSize NOTIFY settingsChanged)
    Q_PROPERTY(int maxFps READ maxFps WRITE setMaxFps NOTIFY settingsChanged)
    Q_PROPERTY(int bitRateMbps READ bitRateMbps WRITE setBitRateMbps NOTIFY settingsChanged)
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY settingsChanged)
    Q_PROPERTY(QString codec READ codec WRITE setCodec NOTIFY settingsChanged)
    Q_PROPERTY(bool audio READ audio WRITE setAudio NOTIFY settingsChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen WRITE setFullscreen NOTIFY settingsChanged)
    Q_PROPERTY(bool clipboardSync READ clipboardSync WRITE setClipboardSync NOTIFY settingsChanged)
    Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect NOTIFY settingsChanged)
    Q_PROPERTY(
        QString recordingPath READ recordingPath WRITE setRecordingPath NOTIFY settingsChanged)

  public:
    MirrorViewModel(services::DeviceManagerService& deviceManager,
                    services::MirrorService& mirrorService, QObject* parent = nullptr);
    ~MirrorViewModel() override;

    [[nodiscard]] bool engineAvailable() const;
    [[nodiscard]] QString engineVersion() const;
    [[nodiscard]] QVariantList devices() const;
    [[nodiscard]] QString selectedSerial() const;
    [[nodiscard]] QStringList activeSessions() const;
    [[nodiscard]] bool active() const;
    [[nodiscard]] bool paused() const;
    [[nodiscard]] bool scanning() const noexcept;
    [[nodiscard]] QString status() const;
    [[nodiscard]] QString errorCause() const;
    [[nodiscard]] QString errorSolution() const;
    [[nodiscard]] QString technicalDetails() const;
    [[nodiscard]] QStringList logs() const;
    [[nodiscard]] double fps() const noexcept;
    [[nodiscard]] int latencyMs() const noexcept;
    [[nodiscard]] int maxSize() const noexcept;
    [[nodiscard]] int maxFps() const noexcept;
    [[nodiscard]] int bitRateMbps() const noexcept;
    [[nodiscard]] int orientation() const noexcept;
    [[nodiscard]] QString codec() const;
    [[nodiscard]] bool audio() const noexcept;
    [[nodiscard]] bool fullscreen() const noexcept;
    [[nodiscard]] bool clipboardSync() const noexcept;
    [[nodiscard]] bool autoReconnect() const noexcept;
    [[nodiscard]] QString recordingPath() const;

    Q_INVOKABLE void scanDevices();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void reconnect();
    Q_INVOKABLE void repairAndRetry();
    Q_INVOKABLE void takeScreenshot(const QString& path);
    Q_INVOKABLE void installApk(const QString& path);
    Q_INVOKABLE void copyError() const;
    Q_INVOKABLE void reportIssue();
    Q_INVOKABLE void clearError();
    Q_INVOKABLE void retranslate();

  public slots:
    void setSelectedSerial(const QString& serial);
    void setMaxSize(int value);
    void setMaxFps(int value);
    void setBitRateMbps(int value);
    void setOrientation(int value);
    void setCodec(const QString& value);
    void setAudio(bool value);
    void setFullscreen(bool value);
    void setClipboardSync(bool value);
    void setAutoReconnect(bool value);
    void setRecordingPath(const QString& value);

  signals:
    void devicesChanged();
    void selectedSerialChanged();
    void sessionChanged();
    void scanningChanged();
    void statusChanged();
    void errorChanged();
    void logsChanged();
    void statisticsChanged();
    void settingsChanged();
    void engineChanged();
    void openUrlRequested(const QUrl& url);

  private:
    [[nodiscard]] services::MirrorSettings settings() const;
    void setError(const QString& cause, const QString& solution, const QString& technical);
    void restartForSettingChange();
    void refreshLatency();

    services::DeviceManagerService& m_deviceManager;
    services::MirrorService& m_mirrorService;
    QFutureWatcher<QList<services::DeviceInfo>> m_deviceWatcher;
    QFutureWatcher<int> m_latencyWatcher;
    QFutureWatcher<QString> m_engineWatcher;
    QTimer m_latencyTimer;
    QVariantList m_devices;
    QString m_engineVersion;
    QString m_selectedSerial;
    QString m_status;
    QString m_errorCause;
    QString m_errorSolution;
    QString m_technicalDetails;
    QStringList m_logs;
    double m_fps = 0.0;
    int m_latencyMs = -1;
    int m_maxSize = 1080;
    int m_maxFps = 60;
    int m_bitRateMbps = 8;
    int m_orientation = 0;
    QString m_codec = QStringLiteral("h264");
    QString m_recordingPath;
    bool m_audio = true;
    bool m_fullscreen = false;
    bool m_clipboardSync = true;
    bool m_autoReconnect = true;
    bool m_scanning = false;
    bool m_restartingForSettings = false;
};

} // namespace adb_studio
