#pragma once

#include <QHash>
#include <QObject>
#include <QProcess>
#include <QSet>
#include <QString>
#include <QStringList>

namespace adb_studio::services
{

struct ProcessResult
{
    int exitCode = -1;
    QByteArray output;
    QByteArray error;
    bool finished = false;
};

struct DeviceInfo
{
    QString serial;
    QString state;
    QString model;
    bool wireless = false;
};

class AdbService final
{
  public:
    AdbService();

    [[nodiscard]] bool available() const;
    [[nodiscard]] QString executable() const;
    [[nodiscard]] ProcessResult execute(const QStringList& arguments, int timeoutMs = 5000) const;
    [[nodiscard]] bool saveScreenshot(const QString& serial, const QString& path,
                                      QString* technicalError) const;

  private:
    [[nodiscard]] static QString locateTool(const QString& fileName);
    QString m_executable;
};

class DeviceManagerService final
{
  public:
    explicit DeviceManagerService(const AdbService& adbService);

    [[nodiscard]] QList<DeviceInfo> devices() const;
    [[nodiscard]] bool restartAdb(QString* technicalError) const;
    [[nodiscard]] bool installApk(const QString& serial, const QString& path,
                                  QString* technicalError) const;
    [[nodiscard]] int latencyMilliseconds(const QString& serial) const;

  private:
    const AdbService& m_adbService;
};

struct MirrorSettings
{
    int maxSize = 1080;
    int maxFps = 60;
    int bitRateMbps = 8;
    int orientation = 0;
    QString codec = QStringLiteral("h264");
    QString recordingPath;
    bool audio = true;
    bool fullscreen = false;
    bool clipboardSync = true;
};

class MirrorService final : public QObject
{
    Q_OBJECT

  public:
    MirrorService(const AdbService& adbService, const DeviceManagerService& deviceManager,
                  QObject* parent = nullptr, QString scrcpyExecutable = {});
    ~MirrorService() override;

    [[nodiscard]] bool available() const;
    [[nodiscard]] QString version() const;
    [[nodiscard]] QStringList activeSerials() const;
    [[nodiscard]] bool isActive(const QString& serial) const;
    [[nodiscard]] bool isPaused(const QString& serial) const;
    [[nodiscard]] bool start(const QString& serial, const MirrorSettings& settings);
    void stop(const QString& serial);
    void stopAll();
    [[nodiscard]] bool setPaused(const QString& serial, bool paused);
    [[nodiscard]] bool screenshot(const QString& serial, const QString& path,
                                  QString* technicalError) const;
    [[nodiscard]] bool installApk(const QString& serial, const QString& path,
                                  QString* technicalError) const;
    [[nodiscard]] int latencyMilliseconds(const QString& serial) const;

  signals:
    void sessionStarted(const QString& serial);
    void sessionStopped(const QString& serial);
    void sessionPausedChanged(const QString& serial, bool paused);
    void sessionFailed(const QString& serial, const QString& cause, const QString& solution,
                       const QString& technicalDetails);
    void logReceived(const QString& serial, const QString& line);
    void fpsChanged(const QString& serial, double fps);

  private:
    [[nodiscard]] static QString locateScrcpy();
    [[nodiscard]] static QStringList argumentsFor(const QString& serial,
                                                  const MirrorSettings& settings);
    void consumeOutput(const QString& serial, QProcess* process);

    const AdbService& m_adbService;
    const DeviceManagerService& m_deviceManager;
    QString m_scrcpyExecutable;
    QHash<QString, QProcess*> m_sessions;
    QHash<QString, QString> m_sessionOutput;
    QSet<QString> m_intentionalStops;
    QSet<QString> m_pausedSessions;
};

} // namespace adb_studio::services
