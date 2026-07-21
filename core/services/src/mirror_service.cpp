#include "adb_studio/services/mirror_service.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>

#include <algorithm>
#include <utility>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace adb_studio::services
{
namespace
{
// Program, argument vector, and timeout are distinct process-launch concepts.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto runProcess(const QString& program, const QStringList& arguments,
                int timeoutMs) -> ProcessResult
{
    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(program, arguments, QIODevice::ReadOnly);
    if (!process.waitForStarted(2000))
    {
        return {};
    }
    const bool finished = process.waitForFinished(timeoutMs);
    if (!finished)
    {
        process.kill();
        process.waitForFinished(1000);
    }
    return {.exitCode = process.exitCode(),
            .output = process.readAllStandardOutput(),
            .error = process.readAllStandardError(),
            .finished = finished};
}

auto toolCandidates(const QString& fileName) -> QStringList
{
    const QString applicationDirectory = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        applicationDirectory + QStringLiteral("/tools/scrcpy/") + fileName,
        QDir::currentPath() + QStringLiteral("/scrcpy/") + fileName,
    };
    QDir directory(applicationDirectory);
    for (int depth = 0; depth < 6 && directory.cdUp(); ++depth)
    {
        candidates.append(directory.filePath(QStringLiteral("scrcpy/") + fileName));
    }
    return candidates;
}

auto technicalText(const ProcessResult& result) -> QString
{
    return QString::fromUtf8(result.error + result.output).trimmed();
}
} // namespace

AdbService::AdbService() : m_executable(locateTool(QStringLiteral("adb.exe")))
{
}

bool AdbService::available() const
{
    return !m_executable.isEmpty();
}
QString AdbService::executable() const
{
    return m_executable;
}

ProcessResult AdbService::execute(const QStringList& arguments, int timeoutMs) const
{
    if (!available())
    {
        return {};
    }
    return runProcess(m_executable, arguments, timeoutMs);
}

// Serial and destination are different protocol concepts at every call site.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool AdbService::saveScreenshot(const QString& serial, const QString& path,
                                QString* technicalError) const
{
    const ProcessResult result = execute({QStringLiteral("-s"), serial, QStringLiteral("exec-out"),
                                          QStringLiteral("screencap"), QStringLiteral("-p")},
                                         10000);
    if (!result.finished || result.exitCode != 0 || !result.output.startsWith("\x89PNG"))
    {
        if (technicalError != nullptr)
        {
            *technicalError = technicalText(result);
        }
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(result.output) != result.output.size() ||
        !file.commit())
    {
        if (technicalError != nullptr)
        {
            *technicalError = file.errorString();
        }
        return false;
    }
    return true;
}

QString AdbService::locateTool(const QString& fileName)
{
    const QString configuredDirectory = qEnvironmentVariable("ADB_STUDIO_SCRCPY_DIR");
    if (!configuredDirectory.isEmpty())
    {
        const QString configured = QDir(configuredDirectory).filePath(fileName);
        if (QFileInfo::exists(configured))
        {
            return QFileInfo(configured).absoluteFilePath();
        }
    }
    for (const QString& candidate : toolCandidates(fileName))
    {
        if (QFileInfo::exists(candidate))
        {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }
    return QStandardPaths::findExecutable(fileName);
}

DeviceManagerService::DeviceManagerService(const AdbService& adbService) : m_adbService(adbService)
{
}

QList<DeviceInfo> DeviceManagerService::devices() const
{
    QList<DeviceInfo> result;
    const ProcessResult process =
        m_adbService.execute({QStringLiteral("devices"), QStringLiteral("-l")}, 5000);
    if (!process.finished || process.exitCode != 0)
    {
        return result;
    }
    const QStringList lines =
        QString::fromUtf8(process.output)
            .split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
    const QRegularExpression modelExpression(QStringLiteral("(?:^|\\s)model:([^\\s]+)"));
    for (const QString& line : lines)
    {
        if (line.startsWith(QStringLiteral("List of devices")))
        {
            continue;
        }
        const QStringList fields =
            line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (fields.size() < 2)
        {
            continue;
        }
        const auto modelMatch = modelExpression.match(line);
        result.append({.serial = fields.at(0),
                       .state = fields.at(1),
                       .model = modelMatch.hasMatch() ? modelMatch.captured(1).replace(
                                                            QLatin1Char('_'), QLatin1Char(' '))
                                                      : QString{},
                       .wireless = fields.at(0).contains(QLatin1Char(':'))});
    }
    return result;
}

bool DeviceManagerService::restartAdb(QString* technicalError) const
{
    const ProcessResult killed = m_adbService.execute({QStringLiteral("kill-server")}, 5000);
    const ProcessResult started = m_adbService.execute({QStringLiteral("start-server")}, 5000);
    if (!killed.finished || !started.finished || started.exitCode != 0)
    {
        if (technicalError != nullptr)
        {
            *technicalError = technicalText(started);
        }
        return false;
    }
    return true;
}

bool DeviceManagerService::installApk(const QString& serial, const QString& path,
                                      QString* technicalError) const
{
    if (!QFileInfo(path).isFile() ||
        QFileInfo(path).suffix().compare(QStringLiteral("apk"), Qt::CaseInsensitive) != 0)
    {
        if (technicalError != nullptr)
        {
            *technicalError = QObject::tr("The selected path is not a readable APK file.");
        }
        return false;
    }
    const ProcessResult result =
        m_adbService.execute({QStringLiteral("-s"), serial, QStringLiteral("install"),
                              QStringLiteral("-r"), QFileInfo(path).absoluteFilePath()},
                             120000);
    if (!result.finished || result.exitCode != 0 || !result.output.contains("Success"))
    {
        if (technicalError != nullptr)
        {
            *technicalError = technicalText(result);
        }
        return false;
    }
    return true;
}

int DeviceManagerService::latencyMilliseconds(const QString& serial) const
{
    QElapsedTimer timer;
    timer.start();
    const ProcessResult result =
        m_adbService.execute({QStringLiteral("-s"), serial, QStringLiteral("shell"),
                              QStringLiteral("echo"), QStringLiteral("adb-studio")},
                             3000);
    return result.finished && result.exitCode == 0 && result.output.contains("adb-studio")
               ? static_cast<int>(timer.elapsed())
               : -1;
}

MirrorService::MirrorService(const AdbService& adbService,
                             const DeviceManagerService& deviceManager, QObject* parent,
                             QString scrcpyExecutable)
    : QObject(parent), m_adbService(adbService), m_deviceManager(deviceManager),
      m_scrcpyExecutable(scrcpyExecutable.isEmpty() ? locateScrcpy() : std::move(scrcpyExecutable))
{
}

MirrorService::~MirrorService()
{
    const QList<QProcess*> processes = m_sessions.values();
    m_sessions.clear();
    for (QProcess* process : processes)
    {
        process->disconnect(this);
        process->kill();
        process->waitForFinished(1000);
    }
}
bool MirrorService::available() const
{
    return !m_scrcpyExecutable.isEmpty();
}

QString MirrorService::version() const
{
    if (!available())
    {
        return {};
    }
    const ProcessResult result =
        runProcess(m_scrcpyExecutable, {QStringLiteral("--version")}, 3000);
    return result.finished && result.exitCode == 0
               ? QString::fromUtf8(result.output).section(QLatin1Char('\n'), 0, 0).trimmed()
               : QString{};
}

QStringList MirrorService::activeSerials() const
{
    return m_sessions.keys();
}
bool MirrorService::isActive(const QString& serial) const
{
    return m_sessions.contains(serial);
}

bool MirrorService::isPaused(const QString& serial) const
{
    return m_pausedSessions.contains(serial);
}

bool MirrorService::start(const QString& serial, const MirrorSettings& settings)
{
    if (!available() || serial.isEmpty() || isActive(serial))
    {
        return false;
    }
    // QObject parent ownership releases the process after the asynchronous session completes.
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* process = new QProcess(this);
    process->setProgram(m_scrcpyExecutable);
    process->setArguments(argumentsFor(serial, settings));
    process->setWorkingDirectory(QFileInfo(m_scrcpyExecutable).absolutePath());
    process->setProcessChannelMode(QProcess::SeparateChannels);
    m_sessions.insert(serial, process);
    connect(process, &QProcess::started, this, [this, serial]() { emit sessionStarted(serial); });
    connect(process, &QProcess::readyReadStandardOutput, this,
            [this, serial, process]() { consumeOutput(serial, process); });
    connect(process, &QProcess::readyReadStandardError, this,
            [this, serial, process]() { consumeOutput(serial, process); });
    connect(process, &QProcess::errorOccurred, this,
            [this, serial, process](QProcess::ProcessError error) {
                if (error != QProcess::FailedToStart || m_sessions.value(serial) != process)
                {
                    return;
                }
                m_sessions.remove(serial);
                m_pausedSessions.remove(serial);
                emit sessionFailed(serial, tr("The scrcpy process could not be started."),
                                   tr("Verify the bundled runtime and retry."),
                                   process->errorString());
                process->deleteLater();
            });
    connect(process, &QProcess::finished, this,
            [this, serial, process](int exitCode, QProcess::ExitStatus exitStatus) {
                if (m_sessions.value(serial) != process)
                {
                    return;
                }
                consumeOutput(serial, process);
                m_sessions.remove(serial);
                m_pausedSessions.remove(serial);
                process->deleteLater();
                const QString details = m_sessionOutput.take(serial).trimmed();
                if (m_intentionalStops.remove(serial))
                {
                    emit sessionStopped(serial);
                    return;
                }
                const bool crashed = exitStatus == QProcess::CrashExit || exitCode != 0;
                if (!crashed)
                {
                    emit sessionStopped(serial);
                    return;
                }
                emit sessionFailed(
                    serial, tr("scrcpy stopped before the mirror session completed."),
                    tr("Run repair to restart ADB, verify authorization, and reconnect."), details);
            });
    process->start();
    return true;
}

void MirrorService::stop(const QString& serial)
{
    QProcess* process = m_sessions.value(serial, nullptr);
    if (process == nullptr)
    {
        return;
    }
    m_intentionalStops.insert(serial);
    process->terminate();
    QTimer::singleShot(2000, process, [process]() {
        if (process->state() != QProcess::NotRunning)
        {
            process->kill();
        }
    });
}

void MirrorService::stopAll()
{
    const QStringList serials = m_sessions.keys();
    for (const QString& serial : serials)
    {
        stop(serial);
    }
}

bool MirrorService::setPaused(const QString& serial, bool paused)
{
    QProcess* process = m_sessions.value(serial, nullptr);
    if (process == nullptr || m_pausedSessions.contains(serial) == paused)
    {
        return process != nullptr;
    }
#ifdef Q_OS_WIN
    using NtProcessFunction = LONG(WINAPI*)(HANDLE);
    const HMODULE module = GetModuleHandleW(L"ntdll.dll");
    const char* functionName = paused ? "NtSuspendProcess" : "NtResumeProcess";
    // Win32 exposes the native suspend/resume entry points as FARPROC.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto function =
        module != nullptr
            ? reinterpret_cast<NtProcessFunction>(GetProcAddress(module, functionName))
            : nullptr;
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    const HANDLE handle =
        OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, static_cast<DWORD>(process->processId()));
    if (function == nullptr || handle == nullptr)
    {
        if (handle != nullptr)
        {
            CloseHandle(handle);
        }
        return false;
    }
    const LONG result = function(handle);
    CloseHandle(handle);
    if (result != 0)
    {
        return false;
    }
#else
    Q_UNUSED(paused)
    return false;
#endif
    if (paused)
    {
        m_pausedSessions.insert(serial);
    }
    else
    {
        m_pausedSessions.remove(serial);
    }
    emit sessionPausedChanged(serial, paused);
    return true;
}

bool MirrorService::screenshot(const QString& serial, const QString& path,
                               QString* technicalError) const
{
    return m_adbService.saveScreenshot(serial, path, technicalError);
}

bool MirrorService::installApk(const QString& serial, const QString& path,
                               QString* technicalError) const
{
    return m_deviceManager.installApk(serial, path, technicalError);
}

int MirrorService::latencyMilliseconds(const QString& serial) const
{
    return m_deviceManager.latencyMilliseconds(serial);
}

QString MirrorService::locateScrcpy()
{
    const QString configuredDirectory = qEnvironmentVariable("ADB_STUDIO_SCRCPY_DIR");
    if (!configuredDirectory.isEmpty())
    {
        const QString configured = QDir(configuredDirectory).filePath(QStringLiteral("scrcpy.exe"));
        if (QFileInfo::exists(configured))
        {
            return QFileInfo(configured).absoluteFilePath();
        }
    }
    for (const QString& candidate : toolCandidates(QStringLiteral("scrcpy.exe")))
    {
        if (QFileInfo::exists(candidate))
        {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }
    return QStandardPaths::findExecutable(QStringLiteral("scrcpy"));
}

QStringList MirrorService::argumentsFor(const QString& serial, const MirrorSettings& settings)
{
    QStringList arguments = {QStringLiteral("--serial=%1").arg(serial),
                             QStringLiteral("--window-title=ADB Studio — %1").arg(serial),
                             QStringLiteral("--max-fps=%1").arg(settings.maxFps),
                             QStringLiteral("--video-bit-rate=%1M").arg(settings.bitRateMbps),
                             QStringLiteral("--video-codec=%1").arg(settings.codec),
                             QStringLiteral("--print-fps"),
                             QStringLiteral("--orientation=%1").arg(settings.orientation),
                             QStringLiteral("--stay-awake")};
    if (settings.maxSize > 0)
    {
        arguments.append(QStringLiteral("--max-size=%1").arg(settings.maxSize));
    }
    if (!settings.audio)
    {
        arguments.append(QStringLiteral("--no-audio"));
    }
    if (settings.fullscreen)
    {
        arguments.append(QStringLiteral("--fullscreen"));
    }
    if (!settings.clipboardSync)
    {
        arguments.append(QStringLiteral("--no-clipboard-autosync"));
    }
    if (!settings.recordingPath.isEmpty())
    {
        arguments.append(QStringLiteral("--record=%1").arg(settings.recordingPath));
    }
    return arguments;
}

void MirrorService::consumeOutput(const QString& serial, QProcess* process)
{
    const QString text =
        QString::fromUtf8(process->readAllStandardOutput() + process->readAllStandardError());
    if (!text.isEmpty())
    {
        m_sessionOutput[serial].append(text);
    }
    const QStringList lines =
        text.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
    const QRegularExpression fpsExpression(QStringLiteral("([0-9]+(?:\\.[0-9]+)?) fps"),
                                           QRegularExpression::CaseInsensitiveOption);
    for (const QString& line : lines)
    {
        emit logReceived(serial, line);
        const auto match = fpsExpression.match(line);
        if (match.hasMatch())
        {
            emit fpsChanged(serial, match.captured(1).toDouble());
        }
    }
}

} // namespace adb_studio::services
