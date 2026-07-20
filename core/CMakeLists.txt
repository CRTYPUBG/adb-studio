add_library(adb_studio_core STATIC)
qt_add_qml_module(adb_studio_core
  URI ADBStudio.Presentation
  VERSION 1.0
  RESOURCE_PREFIX /qt/qml
  OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ADBStudio/Presentation"
  SOURCES
  presentation/include/adb_studio/presentation/theme_service.hpp
  presentation/include/adb_studio/presentation/device_dashboard_view_model.hpp
  presentation/src/theme_service.cpp
)
target_include_directories(adb_studio_core
  PUBLIC presentation/include
  PRIVATE presentation/include/adb_studio/presentation)
target_link_libraries(adb_studio_core PUBLIC Qt6::Core Qt6::Gui Qt6::Qml)
adb_studio_declare_layer(adb_studio_core "viewmodel")
adb_studio_enable_strict(adb_studio_core)
