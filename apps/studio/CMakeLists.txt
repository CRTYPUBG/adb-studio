qt_add_executable(adb-studio WIN32 src/main.cpp)
if(WIN32)
  target_sources(adb-studio PRIVATE "${CMAKE_BINARY_DIR}/generated/adb_studio/version.rc")
endif()
target_link_libraries(adb-studio PRIVATE adb_studio_core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)
target_include_directories(adb-studio PRIVATE "${CMAKE_BINARY_DIR}/generated")
adb_studio_declare_layer(adb-studio "presentation")
adb_studio_enable_strict(adb-studio)

qt_add_qml_module(adb-studio
  URI ADBStudio.UI
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/components/PrimaryButton.qml
    qml/components/StatusBadge.qml
    qml/components/InfoCard.qml
    qml/components/DeviceCard.qml
    qml/components/LoadingView.qml
    qml/pages/DashboardPage.qml
)

set(ADB_STUDIO_BRANDING_ASSETS
  "${CMAKE_SOURCE_DIR}/assets/app-icon-adb.png"
  "${CMAKE_SOURCE_DIR}/assets/logo-adb.png"
  "${CMAKE_SOURCE_DIR}/assets/tray-adb.png")
set_source_files_properties("${CMAKE_SOURCE_DIR}/assets/app-icon-adb.png"
  PROPERTIES QT_RESOURCE_ALIAS app-icon-adb.png)
set_source_files_properties("${CMAKE_SOURCE_DIR}/assets/logo-adb.png"
  PROPERTIES QT_RESOURCE_ALIAS logo-adb.png)
set_source_files_properties("${CMAKE_SOURCE_DIR}/assets/tray-adb.png"
  PROPERTIES QT_RESOURCE_ALIAS tray-adb.png)
qt_add_resources(adb-studio branding
  PREFIX /assets
  FILES ${ADB_STUDIO_BRANDING_ASSETS})

qt_import_qml_plugins(adb-studio INCLUDE adb_studio_coreplugin)
target_link_libraries(adb-studio PRIVATE adb_studio_coreplugin)

install(TARGETS adb-studio RUNTIME DESTINATION bin)
