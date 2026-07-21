#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace adb_studio::diagnostics
{

enum class EvidenceState : unsigned char
{
    Unknown,
    No,
    Yes
};

enum class ConnectionState : unsigned char
{
    NoUsbDevice,
    UsbChargingOnly,
    UsbFileTransfer,
    UsbDebuggingDisabled,
    AuthorizationPending,
    Unauthorized,
    Offline,
    AdbServerNotRunning,
    AdbVersionMismatch,
    DeviceBusy,
    DriverMissing,
    MtpDriverMissing,
    FastbootMode,
    RecoveryMode,
    UnsupportedDevice,
    WirelessAvailable,
    WirelessConnected,
    MultipleDevices,
    Connected,
    Unknown
};

enum class DiagnosticStatus : unsigned char
{
    Pass,
    Warning,
    Fail,
    Unsupported,
    Unknown
};

struct DeviceFacts
{
    EvidenceState adbInstalled = EvidenceState::Unknown;
    EvidenceState adbServerRunning = EvidenceState::Unknown;
    EvidenceState adbVersionCompatible = EvidenceState::Unknown;
    EvidenceState usbDevicePresent = EvidenceState::Unknown;
    EvidenceState usbDriverPresent = EvidenceState::Unknown;
    EvidenceState windowsDriverPresent = EvidenceState::Unknown;
    EvidenceState mtpDriverPresent = EvidenceState::Unknown;
    EvidenceState usbCableStable = EvidenceState::Unknown;
    EvidenceState usbDebuggingEnabled = EvidenceState::Unknown;
    EvidenceState authorizationPending = EvidenceState::Unknown;
    EvidenceState deviceBusy = EvidenceState::Unknown;
    EvidenceState supportedDevice = EvidenceState::Unknown;
    EvidenceState wirelessAvailable = EvidenceState::Unknown;
    EvidenceState wirelessConnected = EvidenceState::Unknown;
    EvidenceState audioSupported = EvidenceState::Unknown;
    EvidenceState screenRecordingSupported = EvidenceState::Unknown;
    EvidenceState codecSupported = EvidenceState::Unknown;
    EvidenceState highRefreshSupported = EvidenceState::Unknown;
    EvidenceState rooted = EvidenceState::Unknown;
    EvidenceState recoveryMode = EvidenceState::Unknown;
    EvidenceState fastbootMode = EvidenceState::Unknown;
    QString adbVersion;
    QString adbDeviceState;
    QString usbMode;
    QString serial;
    QString manufacturer;
    QString brand;
    QString model;
    QString androidVersion;
    QString buildNumber;
    QString oemSkin;
    int apiLevel = 0;
    int deviceCount = 0;
    int usbSpeedMbps = -1;
    bool recoveryAttempted = false;
};

struct DiagnosticResult
{
    QString id;
    QString category;
    DiagnosticStatus status = DiagnosticStatus::Unknown;
    QString title;
    QString summary;
    QString whyItMatters;
    QString action;
    QString difficulty;
    int estimatedMinutes = 0;
    bool automaticFixAvailable = false;
};

struct OemGuide
{
    QString key;
    QString displayName;
    QString title;
    QStringList steps;
    QStringList notes;
    QString officialDocumentationUrl;
    int estimatedMinutes = 0;
};

struct HealthScore
{
    int total = 0;
    int confidence = 0;
    int connection = 0;
    int adb = 0;
    int usb = 0;
    int performance = 0;
    int compatibility = 0;
    int security = 0;
    int recommendedSettings = 0;
};

struct ConnectionReport
{
    ConnectionState state = ConnectionState::Unknown;
    QString stateKey;
    QString title;
    QString explanation;
    QString recommendedAction;
    QString currentStep;
    DeviceFacts facts;
    OemGuide oemGuide;
    QList<DiagnosticResult> diagnostics;
    HealthScore health;
};

class ConnectionDiagnosticsEngine final
{
  public:
    [[nodiscard]] static ConnectionReport analyze(const DeviceFacts& facts);
    [[nodiscard]] static QString statusKey(DiagnosticStatus status);

  private:
    [[nodiscard]] static ConnectionState classify(const DeviceFacts& facts);
    [[nodiscard]] static QList<DiagnosticResult> diagnosticsFor(const DeviceFacts& facts);
    [[nodiscard]] static HealthScore calculateHealth(const QList<DiagnosticResult>& diagnostics);
};

class OemGuideCatalog final
{
  public:
    [[nodiscard]] static OemGuide detect(const DeviceFacts& facts);
};

} // namespace adb_studio::diagnostics
