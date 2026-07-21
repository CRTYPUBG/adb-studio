#pragma once

#include "adb_studio/diagnostics/device_diagnostics.hpp"

#include <QString>

namespace adb_studio::diagnostics
{

class SystemDeviceProbe final
{
  public:
    [[nodiscard]] static DeviceFacts probe();
    [[nodiscard]] static DeviceFacts probeWithRecovery();
    [[nodiscard]] static DeviceFacts parseAdbDevices(const QString& output);

  private:
    static bool restartAdbServer();
};

} // namespace adb_studio::diagnostics
