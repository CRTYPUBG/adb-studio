#include "adb_studio/presentation/mirror_view_model.hpp"

#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QSettings>
#include <QUrl>
#include <QtConcurrentRun>

#include <algorithm>

namespace adb_studio
{
namespace
{
auto localPath(const QString& value) -> QString
{
    const QUrl url(value);
    return url.isLocalFile() ? url.toLocalFile() : value;
}
} // namespace

// Constructor wiring is intentionally centralized so every asynchronous service signal has one
// lifetime-bound connection owned by this ViewModel.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
MirrorViewModel::MirrorViewModel(services::DeviceManagerService& deviceManager,
                                 services::MirrorService& mirrorService, QObject* parent)
    : QObject(parent), m_deviceManager(deviceManager), m_mirrorService(mirrorService),
      m_status(tr("Ready to scan for authorized devices"))
{
    QSettings settingsStore;
    const int storedSize = settingsStore.value(QStringLiteral("mirror/maxSize"), 1080).toInt();
    m_maxSize = storedSize == 0 || storedSize == 720 || storedSize == 1080 || storedSize == 1440
                    ? storedSize
                    : 1080;
    m_maxFps = qBound(15, settingsStore.value(QStringLiteral("mirror/maxFps"), 60).toInt(), 240);
    m_bitRateMbps =
        qBound(1, settingsStore.value(QStringLiteral("mirror/bitRateMbps"), 8).toInt(), 100);
    const int storedOrientation =
        settingsStore.value(QStringLiteral("mirror/orientation"), 0).toInt();
    m_orientation = storedOrientation == 0 || storedOrientation == 90 || storedOrientation == 180 ||
                            storedOrientation == 270
                        ? storedOrientation
                        : 0;
    const QString storedCodec =
        settingsStore.value(QStringLiteral("mirror/codec"), QStringLiteral("h264"))
            .toString()
            .toLower();
    m_codec = storedCodec == QStringLiteral("h264") || storedCodec == QStringLiteral("h265") ||
                      storedCodec == QStringLiteral("av1")
                  ? storedCodec
                  : QStringLiteral("h264");
    m_audio = settingsStore.value(QStringLiteral("mirror/audio"), true).toBool();
    m_fullscreen = settingsStore.value(QStringLiteral("mirror/fullscreen"), false).toBool();
    m_clipboardSync = settingsStore.value(QStringLiteral("mirror/clipboardSync"), true).toBool();
    m_autoReconnect = settingsStore.value(QStringLiteral("mirror/autoReconnect"), true).toBool();
    m_recordingPath = settingsStore.value(QStringLiteral("mirror/recordingPath")).toString();
    connect(&m_deviceWatcher, &QFutureWatcher<QList<services::DeviceInfo>>::finished, this,
            [this]() {
                m_devices.clear();
                for (const auto& device : m_deviceWatcher.result())
                {
                    if (device.state != QStringLiteral("device"))
                    {
                        continue;
                    }
                    m_devices.append(QVariantMap{
                        {QStringLiteral("serial"), device.serial},
                        {QStringLiteral("label"),
                         device.model.isEmpty() ? device.serial
                                                : tr("%1 — %2").arg(device.model, device.serial)},
                        {QStringLiteral("wireless"), device.wireless}});
                }
                if (m_selectedSerial.isEmpty() && !m_devices.isEmpty())
                {
                    m_selectedSerial =
                        m_devices.constFirst().toMap().value(QStringLiteral("serial")).toString();
                    emit selectedSerialChanged();
                }
                m_scanning = false;
                m_status = m_devices.isEmpty() ? tr("No authorized device is available")
                                               : tr("Authorized devices detected");
                emit devicesChanged();
                emit scanningChanged();
                emit statusChanged();
            });
    connect(&m_mirrorService, &services::MirrorService::sessionStarted, this,
            [this](const QString& serial) {
                m_status = tr("Mirroring %1").arg(serial);
                clearError();
                emit statusChanged();
                emit sessionChanged();
                if (serial == m_selectedSerial)
                {
                    m_latencyTimer.start();
                }
            });
    connect(&m_mirrorService, &services::MirrorService::sessionStopped, this,
            [this](const QString& serial) {
                emit sessionChanged();
                if (serial == m_selectedSerial)
                {
                    m_latencyTimer.stop();
                    m_fps = 0.0;
                    m_latencyMs = -1;
                    emit statisticsChanged();
                }
                if (m_restartingForSettings && serial == m_selectedSerial)
                {
                    m_restartingForSettings = false;
                    QTimer::singleShot(100, this, &MirrorViewModel::start);
                    return;
                }
                m_status = tr("Mirror session stopped");
                emit statusChanged();
            });
    connect(&m_mirrorService, &services::MirrorService::sessionPausedChanged, this,
            [this](const QString& serial, bool isPaused) {
                if (serial == m_selectedSerial)
                {
                    m_status =
                        isPaused ? tr("Mirror session paused") : tr("Mirroring %1").arg(serial);
                    emit statusChanged();
                    emit sessionChanged();
                }
            });
    // Signal argument order is defined by MirrorService and cannot be replaced with strong types.
    // NOLINTBEGIN(bugprone-easily-swappable-parameters)
    connect(&m_mirrorService, &services::MirrorService::sessionFailed, this,
            [this](const QString& serial, const QString& cause, const QString& solution,
                   const QString& technical) {
                emit sessionChanged();
                setError(cause, solution, technical);
                m_status = tr("Mirror session failed");
                emit statusChanged();
                if (m_autoReconnect && serial == m_selectedSerial)
                {
                    QTimer::singleShot(2000, this, [this]() {
                        if (!active())
                        {
                            start();
                        }
                    });
                }
            });
    // NOLINTEND(bugprone-easily-swappable-parameters)
    connect(&m_mirrorService, &services::MirrorService::logReceived, this,
            [this](const QString& serial, const QString& line) {
                m_logs.append(tr("[%1] %2").arg(serial, line));
                while (m_logs.size() > 200)
                {
                    m_logs.removeFirst();
                }
                emit logsChanged();
            });
    connect(&m_mirrorService, &services::MirrorService::fpsChanged, this,
            [this](const QString& serial, double value) {
                if (serial == m_selectedSerial)
                {
                    m_fps = value;
                    emit statisticsChanged();
                }
            });
    m_latencyTimer.setInterval(1500);
    connect(&m_latencyTimer, &QTimer::timeout, this, &MirrorViewModel::refreshLatency);
    connect(&m_latencyWatcher, &QFutureWatcher<int>::finished, this, [this]() {
        m_latencyMs = m_latencyWatcher.result();
        emit statisticsChanged();
    });
    connect(&m_engineWatcher, &QFutureWatcher<QString>::finished, this, [this]() {
        m_engineVersion = m_engineWatcher.result();
        emit engineChanged();
    });
    m_engineWatcher.setFuture(QtConcurrent::run([this]() { return m_mirrorService.version(); }));
}

MirrorViewModel::~MirrorViewModel()
{
    m_latencyTimer.stop();
    for (QFutureWatcherBase* watcher : {static_cast<QFutureWatcherBase*>(&m_deviceWatcher),
                                        static_cast<QFutureWatcherBase*>(&m_latencyWatcher),
                                        static_cast<QFutureWatcherBase*>(&m_engineWatcher)})
    {
        watcher->cancel();
        watcher->waitForFinished();
    }
}

bool MirrorViewModel::engineAvailable() const
{
    return m_mirrorService.available();
}
QString MirrorViewModel::engineVersion() const
{
    return m_engineVersion;
}
QVariantList MirrorViewModel::devices() const
{
    return m_devices;
}
QString MirrorViewModel::selectedSerial() const
{
    return m_selectedSerial;
}
QStringList MirrorViewModel::activeSessions() const
{
    return m_mirrorService.activeSerials();
}
bool MirrorViewModel::active() const
{
    return m_mirrorService.isActive(m_selectedSerial);
}
bool MirrorViewModel::paused() const
{
    return m_mirrorService.isPaused(m_selectedSerial);
}
bool MirrorViewModel::scanning() const noexcept
{
    return m_scanning;
}
QString MirrorViewModel::status() const
{
    return m_status;
}
QString MirrorViewModel::errorCause() const
{
    return m_errorCause;
}
QString MirrorViewModel::errorSolution() const
{
    return m_errorSolution;
}
QString MirrorViewModel::technicalDetails() const
{
    return m_technicalDetails;
}
QStringList MirrorViewModel::logs() const
{
    return m_logs;
}
double MirrorViewModel::fps() const noexcept
{
    return m_fps;
}
int MirrorViewModel::latencyMs() const noexcept
{
    return m_latencyMs;
}
int MirrorViewModel::maxSize() const noexcept
{
    return m_maxSize;
}
int MirrorViewModel::maxFps() const noexcept
{
    return m_maxFps;
}
int MirrorViewModel::bitRateMbps() const noexcept
{
    return m_bitRateMbps;
}
int MirrorViewModel::orientation() const noexcept
{
    return m_orientation;
}
QString MirrorViewModel::codec() const
{
    return m_codec;
}
bool MirrorViewModel::audio() const noexcept
{
    return m_audio;
}
bool MirrorViewModel::fullscreen() const noexcept
{
    return m_fullscreen;
}
bool MirrorViewModel::clipboardSync() const noexcept
{
    return m_clipboardSync;
}
bool MirrorViewModel::autoReconnect() const noexcept
{
    return m_autoReconnect;
}
QString MirrorViewModel::recordingPath() const
{
    return m_recordingPath;
}

void MirrorViewModel::scanDevices()
{
    if (m_scanning)
    {
        return;
    }
    m_scanning = true;
    m_status = tr("Scanning authorized ADB devices");
    emit scanningChanged();
    emit statusChanged();
    m_deviceWatcher.setFuture(QtConcurrent::run([this]() { return m_deviceManager.devices(); }));
}

void MirrorViewModel::start()
{
    if (!engineAvailable())
    {
        setError(tr("The verified scrcpy executable is unavailable."),
                 tr("Restore the bundled scrcpy runtime and validate the installation."), {});
        return;
    }
    if (m_selectedSerial.isEmpty())
    {
        setError(tr("No authorized device is selected."),
                 tr("Connect and authorize a device, then select Scan devices."), {});
        return;
    }
    const bool verifiedSelection =
        std::any_of(m_devices.cbegin(), m_devices.cend(), [this](const QVariant& item) {
            return item.toMap().value(QStringLiteral("serial")).toString() == m_selectedSerial;
        });
    if (!verifiedSelection)
    {
        setError(tr("The selected device is no longer in the verified device list."),
                 tr("Scan devices again before starting mirroring."), {});
        return;
    }
    if (!m_mirrorService.start(m_selectedSerial, settings()) && !active())
    {
        setError(tr("The mirror session could not be started."),
                 tr("Verify the device connection or run the repair action."), {});
    }
}

void MirrorViewModel::stop()
{
    m_mirrorService.stop(m_selectedSerial);
}

void MirrorViewModel::pause()
{
    if (active() && !paused() && !m_mirrorService.setPaused(m_selectedSerial, true))
    {
        setError(tr("The mirror process could not be paused."),
                 tr("Retry or reconnect the mirror session."), {});
    }
}

void MirrorViewModel::resume()
{
    if (active() && paused() && !m_mirrorService.setPaused(m_selectedSerial, false))
    {
        setError(tr("The mirror process could not be resumed."),
                 tr("Reconnect the mirror session."), {});
    }
}

void MirrorViewModel::reconnect()
{
    if (active())
    {
        m_restartingForSettings = true;
        stop();
    }
    else
    {
        start();
    }
}

void MirrorViewModel::repairAndRetry()
{
    m_status = tr("Restarting ADB before retrying the mirror session");
    emit statusChanged();
    // QObject parent ownership releases the one-shot watcher after completion.
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* watcher = new QFutureWatcher<QPair<bool, QString>>(this);
    connect(watcher, &QFutureWatcher<QPair<bool, QString>>::finished, this, [this, watcher]() {
        const auto result = watcher->result();
        watcher->deleteLater();
        if (!result.first)
        {
            setError(tr("ADB recovery did not complete successfully."),
                     tr("Open diagnostics and verify the ADB installation."), result.second);
            return;
        }
        scanDevices();
        QTimer::singleShot(500, this, &MirrorViewModel::start);
    });
    watcher->setFuture(QtConcurrent::run([this]() {
        QString error;
        return qMakePair(m_deviceManager.restartAdb(&error), error);
    }));
}

void MirrorViewModel::takeScreenshot(const QString& path)
{
    const QString destination = localPath(path);
    // QObject parent ownership releases the one-shot watcher after completion.
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* watcher = new QFutureWatcher<QPair<bool, QString>>(this);
    connect(watcher, &QFutureWatcher<QPair<bool, QString>>::finished, this,
            [this, watcher, destination]() {
                const auto result = watcher->result();
                watcher->deleteLater();
                if (!result.first)
                {
                    setError(tr("The device screenshot could not be saved."),
                             tr("Verify storage permissions and retry."), result.second);
                    return;
                }
                m_status = tr("Screenshot saved to %1").arg(destination);
                emit statusChanged();
            });
    watcher->setFuture(QtConcurrent::run([this, destination]() {
        QString error;
        return qMakePair(m_mirrorService.screenshot(m_selectedSerial, destination, &error), error);
    }));
}

void MirrorViewModel::installApk(const QString& path)
{
    const QString source = localPath(path);
    // QObject parent ownership releases the one-shot watcher after completion.
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* watcher = new QFutureWatcher<QPair<bool, QString>>(this);
    connect(watcher, &QFutureWatcher<QPair<bool, QString>>::finished, this,
            [this, watcher, source]() {
                const auto result = watcher->result();
                watcher->deleteLater();
                if (!result.first)
                {
                    setError(tr("The APK installation failed."),
                             tr("Verify the APK, device authorization and installation policy."),
                             result.second);
                    return;
                }
                m_status = tr("Installed %1").arg(QFileInfo(source).fileName());
                emit statusChanged();
            });
    watcher->setFuture(QtConcurrent::run([this, source]() {
        QString error;
        return qMakePair(m_mirrorService.installApk(m_selectedSerial, source, &error), error);
    }));
}

void MirrorViewModel::copyError() const
{
    QGuiApplication::clipboard()->setText(
        tr("Cause: %1\nSolution: %2\nTechnical details: %3")
            .arg(m_errorCause, m_errorSolution, m_technicalDetails));
}

void MirrorViewModel::reportIssue()
{
    emit openUrlRequested(
        QUrl(QStringLiteral("https://github.com/CRTYPUBG/adb-studio/issues/new")));
}

void MirrorViewModel::clearError()
{
    if (m_errorCause.isEmpty() && m_errorSolution.isEmpty() && m_technicalDetails.isEmpty())
    {
        return;
    }
    m_errorCause.clear();
    m_errorSolution.clear();
    m_technicalDetails.clear();
    emit errorChanged();
}

void MirrorViewModel::retranslate()
{
    clearError();
    if (paused())
    {
        m_status = tr("Mirror session paused");
    }
    else if (active())
    {
        m_status = tr("Mirroring %1").arg(m_selectedSerial);
    }
    else if (m_scanning)
    {
        m_status = tr("Scanning authorized ADB devices");
    }
    else
    {
        m_status = m_devices.isEmpty() ? tr("No authorized device is available")
                                       : tr("Authorized devices detected");
    }
    emit statusChanged();
}

void MirrorViewModel::setSelectedSerial(const QString& serial)
{
    if (m_selectedSerial == serial)
    {
        return;
    }
    m_selectedSerial = serial;
    m_fps = 0.0;
    m_latencyMs = -1;
    emit selectedSerialChanged();
    emit sessionChanged();
    emit statisticsChanged();
}

void MirrorViewModel::setMaxSize(int value)
{
    if ((value != 0 && value != 720 && value != 1080 && value != 1440) || m_maxSize == value)
    {
        return;
    }
    m_maxSize = value;
    QSettings().setValue(QStringLiteral("mirror/maxSize"), value);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setMaxFps(int value)
{
    if (value < 15 || value > 240 || m_maxFps == value)
    {
        return;
    }
    m_maxFps = value;
    QSettings().setValue(QStringLiteral("mirror/maxFps"), value);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setBitRateMbps(int value)
{
    if (value < 1 || value > 100 || m_bitRateMbps == value)
    {
        return;
    }
    m_bitRateMbps = value;
    QSettings().setValue(QStringLiteral("mirror/bitRateMbps"), value);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setOrientation(int value)
{
    if ((value != 0 && value != 90 && value != 180 && value != 270) || m_orientation == value)
    {
        return;
    }
    m_orientation = value;
    QSettings().setValue(QStringLiteral("mirror/orientation"), value);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setCodec(const QString& value)
{
    const QString normalized = value.toLower();
    if ((normalized != QStringLiteral("h264") && normalized != QStringLiteral("h265") &&
         normalized != QStringLiteral("av1")) ||
        m_codec == normalized)
    {
        return;
    }
    m_codec = normalized;
    QSettings().setValue(QStringLiteral("mirror/codec"), normalized);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setAudio(bool value)
{
    if (m_audio == value)
    {
        return;
    }
    m_audio = value;
    QSettings().setValue(QStringLiteral("mirror/audio"), value);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setFullscreen(bool value)
{
    if (m_fullscreen == value)
    {
        return;
    }
    m_fullscreen = value;
    QSettings().setValue(QStringLiteral("mirror/fullscreen"), value);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setClipboardSync(bool value)
{
    if (m_clipboardSync == value)
    {
        return;
    }
    m_clipboardSync = value;
    QSettings().setValue(QStringLiteral("mirror/clipboardSync"), value);
    emit settingsChanged();
    restartForSettingChange();
}

void MirrorViewModel::setAutoReconnect(bool value)
{
    if (m_autoReconnect == value)
    {
        return;
    }
    m_autoReconnect = value;
    QSettings().setValue(QStringLiteral("mirror/autoReconnect"), value);
    emit settingsChanged();
}

void MirrorViewModel::setRecordingPath(const QString& value)
{
    const QString path = localPath(value);
    if (m_recordingPath == path)
    {
        return;
    }
    m_recordingPath = path;
    QSettings().setValue(QStringLiteral("mirror/recordingPath"), path);
    emit settingsChanged();
    restartForSettingChange();
}

services::MirrorSettings MirrorViewModel::settings() const
{
    return {m_maxSize,       m_maxFps, m_bitRateMbps, m_orientation,  m_codec,
            m_recordingPath, m_audio,  m_fullscreen,  m_clipboardSync};
}

void MirrorViewModel::setError(const QString& cause, const QString& solution,
                               const QString& technical)
{
    m_errorCause = cause;
    m_errorSolution = solution;
    m_technicalDetails = technical;
    emit errorChanged();
}

void MirrorViewModel::restartForSettingChange()
{
    if (!active())
    {
        return;
    }
    m_restartingForSettings = true;
    stop();
}

void MirrorViewModel::refreshLatency()
{
    if (!active() || m_latencyWatcher.isRunning())
    {
        return;
    }
    const QString serial = m_selectedSerial;
    m_latencyWatcher.setFuture(QtConcurrent::run(
        [this, serial]() { return m_mirrorService.latencyMilliseconds(serial); }));
}

} // namespace adb_studio
