add_library(adb_studio_core STATIC)
qt_add_qml_module(adb_studio_core
  URI ADBStudio.Presentation
  VERSION 1.0
  RESOURCE_PREFIX /qt/qml
  OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ADBStudio/Presentation"
  SOURCES
  diagnostics/include/adb_studio/diagnostics/device_diagnostics.hpp
  diagnostics/include/adb_studio/diagnostics/system_device_probe.hpp
  diagnostics/src/device_diagnostics.cpp
  diagnostics/src/system_device_probe.cpp
  services/include/adb_studio/services/mirror_service.hpp
  services/src/mirror_service.cpp
  presentation/include/adb_studio/presentation/theme_service.hpp
  presentation/include/adb_studio/presentation/language_service.hpp
  presentation/include/adb_studio/presentation/device_dashboard_view_model.hpp
  presentation/include/adb_studio/presentation/workspace_view_model.hpp
  presentation/include/adb_studio/presentation/mirror_view_model.hpp
  presentation/src/device_dashboard_view_model.cpp
  presentation/src/workspace_view_model.cpp
  presentation/src/mirror_view_model.cpp
  presentation/src/theme_service.cpp
  presentation/src/language_service.cpp
)
target_include_directories(adb_studio_core
  PUBLIC diagnostics/include presentation/include services/include
  PRIVATE presentation/include/adb_studio/presentation)
target_link_libraries(adb_studio_core PUBLIC Qt6::Concurrent Qt6::Core Qt6::Gui Qt6::Qml)
adb_studio_declare_layer(adb_studio_core "viewmodel")
adb_studio_enable_strict(adb_studio_core)
