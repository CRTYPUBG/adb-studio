#include "adb_studio/presentation/device_dashboard_view_model.hpp"

#include "adb_studio/diagnostics/system_device_probe.hpp"

#include <QtConcurrentRun>

namespace adb_studio
{

DeviceDashboardViewModel::DeviceDashboardViewModel(QObject* parent)
    : QObject(parent), m_state(tr("No devices connected")),
      m_explanation(tr("Connect a device or start a scan. No cause has been assumed.")),
      m_recommendedAction(tr("Select Scan devices to begin verified detection.")),
      m_currentDetectionStep(tr("Waiting to scan")), m_usbStatus(tr("Not verified")),
      m_wirelessStatus(tr("Not verified"))
{
    connect(&m_scanWatcher, &QFutureWatcher<diagnostics::ConnectionReport>::finished, this,
            [this]() {
                applyReport(m_scanWatcher.result());
                setScanning(false);
            });
}

bool DeviceDashboardViewModel::connected() const noexcept
{
    return m_connected;
}

bool DeviceDashboardViewModel::scanning() const noexcept
{
    return m_scanning;
}

QString DeviceDashboardViewModel::state() const
{
    return m_state;
}

QString DeviceDashboardViewModel::connectionStateKey() const
{
    return m_connectionStateKey;
}

QString DeviceDashboardViewModel::explanation() const
{
    return m_explanation;
}

QString DeviceDashboardViewModel::recommendedAction() const
{
    return m_recommendedAction;
}

QString DeviceDashboardViewModel::currentDetectionStep() const
{
    return m_currentDetectionStep;
}

int DeviceDashboardViewModel::healthScore() const noexcept
{
    return m_healthScore;
}

int DeviceDashboardViewModel::healthConfidence() const noexcept
{
    return m_healthConfidence;
}

QVariantList DeviceDashboardViewModel::diagnostics() const
{
    return m_diagnostics;
}

bool DeviceDashboardViewModel::oemDetected() const noexcept
{
    return m_oemDetected;
}

QString DeviceDashboardViewModel::oemGuideTitle() const
{
    return m_oemGuideTitle;
}

QStringList DeviceDashboardViewModel::oemGuideSteps() const
{
    return m_oemGuideSteps;
}

QStringList DeviceDashboardViewModel::oemGuideNotes() const
{
    return m_oemGuideNotes;
}

int DeviceDashboardViewModel::oemGuideMinutes() const noexcept
{
    return m_oemGuideMinutes;
}

QString DeviceDashboardViewModel::manufacturer() const
{
    return m_manufacturer;
}

QString DeviceDashboardViewModel::brand() const
{
    return m_brand;
}

QString DeviceDashboardViewModel::model() const
{
    return m_model;
}

QString DeviceDashboardViewModel::androidVersion() const
{
    return m_androidVersion;
}

QString DeviceDashboardViewModel::buildNumber() const
{
    return m_buildNumber;
}

QString DeviceDashboardViewModel::oemSkin() const
{
    return m_oemSkin;
}

int DeviceDashboardViewModel::apiLevel() const noexcept
{
    return m_apiLevel;
}

int DeviceDashboardViewModel::batteryPercent() const noexcept
{
    return m_batteryPercent;
}

QString DeviceDashboardViewModel::storage() const
{
    return m_storage;
}

QString DeviceDashboardViewModel::cpu() const
{
    return m_cpu;
}

QString DeviceDashboardViewModel::memory() const
{
    return m_memory;
}

QString DeviceDashboardViewModel::usbStatus() const
{
    return m_usbStatus;
}

QString DeviceDashboardViewModel::wirelessStatus() const
{
    return m_wirelessStatus;
}

void DeviceDashboardViewModel::refresh()
{
    emit refreshRequested();
}

void DeviceDashboardViewModel::startScan()
{
    launchScan(false);
}

void DeviceDashboardViewModel::recoverConnection()
{
    launchScan(true);
}

void DeviceDashboardViewModel::openOemGuide()
{
    if (m_oemDetected && m_oemGuideUrl.isValid() &&
        m_oemGuideUrl.scheme() == QStringLiteral("https"))
    {
        emit openUrlRequested(m_oemGuideUrl);
    }
}

void DeviceDashboardViewModel::openWirelessSetup()
{
    emit openUrlRequested(QUrl(
        QStringLiteral("https://developer.android.com/tools/adb#connect-to-a-device-over-wi-fi")));
}

void DeviceDashboardViewModel::openDriverGuide()
{
    if (m_oemGuideUrl.isValid() && m_oemGuideUrl.scheme() == QStringLiteral("https"))
    {
        emit openUrlRequested(m_oemGuideUrl);
    }
}

void DeviceDashboardViewModel::retranslate()
{
    m_state = tr("Refreshing device state");
    m_explanation = tr("Device evidence is being refreshed in the selected language.");
    m_recommendedAction = tr("Wait for the verified device scan to complete.");
    m_currentDetectionStep = tr("Refreshing localized device diagnostics");
    emit stateChanged();
    emit diagnosticsChanged();
    if (!m_scanning)
    {
        launchScan(false);
    }
}

void DeviceDashboardViewModel::launchScan(bool forceRecovery)
{
    if (m_scanning)
    {
        return;
    }
    setScanning(true);
    m_currentDetectionStep = forceRecovery ? tr("Restarting ADB and rescanning")
                                           : tr("Checking ADB, transports, and device facts");
    emit diagnosticsChanged();
    m_scanWatcher.setFuture(QtConcurrent::run([]() {
        const diagnostics::DeviceFacts facts = diagnostics::SystemDeviceProbe::probeWithRecovery();
        return diagnostics::ConnectionDiagnosticsEngine::analyze(facts);
    }));
}

void DeviceDashboardViewModel::applyReport(const diagnostics::ConnectionReport& report)
{
    const bool connected = report.state == diagnostics::ConnectionState::Connected ||
                           report.state == diagnostics::ConnectionState::WirelessConnected;
    if (m_connected != connected)
    {
        m_connected = connected;
        emit connectedChanged();
    }
    m_state = report.title;
    m_connectionStateKey = report.stateKey;
    m_explanation = report.explanation;
    m_recommendedAction = report.recommendedAction;
    m_currentDetectionStep = report.currentStep;
    m_healthScore = report.health.total;
    m_healthConfidence = report.health.confidence;
    m_manufacturer = report.facts.manufacturer;
    m_brand = report.facts.brand;
    m_model = report.facts.model;
    m_androidVersion = report.facts.androidVersion;
    m_buildNumber = report.facts.buildNumber;
    m_oemSkin = report.facts.oemSkin;
    m_apiLevel = report.facts.apiLevel;
    m_oemDetected = !m_manufacturer.isEmpty() || !m_brand.isEmpty();
    m_oemGuideTitle = report.oemGuide.title;
    m_oemGuideSteps = report.oemGuide.steps;
    m_oemGuideNotes = report.oemGuide.notes;
    m_oemGuideMinutes = report.oemGuide.estimatedMinutes;
    m_oemGuideUrl = QUrl(report.oemGuide.officialDocumentationUrl);
    const auto evidenceText = [this](diagnostics::EvidenceState state) {
        if (state == diagnostics::EvidenceState::Yes)
        {
            return tr("Connected");
        }
        if (state == diagnostics::EvidenceState::No)
        {
            return tr("Disconnected");
        }
        return tr("Not verified");
    };
    m_usbStatus = evidenceText(report.facts.usbDevicePresent);
    m_wirelessStatus = evidenceText(report.facts.wirelessConnected);
    m_diagnostics.clear();
    for (const auto& diagnostic : report.diagnostics)
    {
        m_diagnostics.append(
            QVariantMap{{QStringLiteral("id"), diagnostic.id},
                        {QStringLiteral("status"),
                         diagnostics::ConnectionDiagnosticsEngine::statusKey(diagnostic.status)},
                        {QStringLiteral("title"), diagnostic.title},
                        {QStringLiteral("summary"), diagnostic.summary},
                        {QStringLiteral("why"), diagnostic.whyItMatters},
                        {QStringLiteral("action"), diagnostic.action},
                        {QStringLiteral("difficulty"), diagnostic.difficulty},
                        {QStringLiteral("minutes"), diagnostic.estimatedMinutes},
                        {QStringLiteral("automaticFix"), diagnostic.automaticFixAvailable}});
    }
    emit stateChanged();
    emit deviceChanged();
    emit diagnosticsChanged();
}

void DeviceDashboardViewModel::setScanning(bool scanning)
{
    if (m_scanning == scanning)
    {
        return;
    }
    m_scanning = scanning;
    emit scanningChanged();
}

} // namespace adb_studio
