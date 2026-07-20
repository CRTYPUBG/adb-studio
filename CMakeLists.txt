cmake_minimum_required(VERSION 3.25)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.crty" ADB_STUDIO_VERSION_JSON)
string(JSON ADB_STUDIO_VERSION GET "${ADB_STUDIO_VERSION_JSON}" version)
string(JSON ADB_STUDIO_BUILD GET "${ADB_STUDIO_VERSION_JSON}" build)
project(ADBStudio VERSION "${ADB_STUDIO_VERSION}" LANGUAGES CXX)

configure_file(
  cmake/version.hpp.in
  "${CMAKE_CURRENT_BINARY_DIR}/generated/adb_studio/version.hpp"
  @ONLY)
if(WIN32)
  set(ADB_STUDIO_ICON_RC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/app-icon-adb.ico")
  configure_file(
    cmake/version.rc.in
    "${CMAKE_CURRENT_BINARY_DIR}/generated/adb_studio/version.rc"
    @ONLY)
endif()

include(GNUInstallDirs)
include(cmake/ProjectOptions.cmake)
include(cmake/Architecture.cmake)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Test)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
qt_standard_project_setup(REQUIRES 6.8)

execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_resources.py"
          "${CMAKE_CURRENT_SOURCE_DIR}"
  RESULT_VARIABLE ADB_STUDIO_RESOURCE_VALIDATION_RESULT
)
if(NOT ADB_STUDIO_RESOURCE_VALIDATION_RESULT EQUAL 0)
  message(FATAL_ERROR "resources/res.crty validation failed")
endif()
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_version.py"
          "${CMAKE_CURRENT_SOURCE_DIR}"
  RESULT_VARIABLE ADB_STUDIO_VERSION_VALIDATION_RESULT
)
if(NOT ADB_STUDIO_VERSION_VALIDATION_RESULT EQUAL 0)
  message(FATAL_ERROR "version.crty validation failed")
endif()
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_architecture.py"
          "${CMAKE_CURRENT_SOURCE_DIR}"
  RESULT_VARIABLE ADB_STUDIO_ARCHITECTURE_VALIDATION_RESULT
)
if(NOT ADB_STUDIO_ARCHITECTURE_VALIDATION_RESULT EQUAL 0)
  message(FATAL_ERROR "architecture dependency validation failed")
endif()

add_subdirectory(core)
add_subdirectory(apps/studio)
if(ADB_STUDIO_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

add_custom_target(validate-resources
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_resources.py"
          "${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM)
add_custom_target(validate-version
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_version.py"
          "${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM)
add_custom_target(validate-architecture
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate_architecture.py"
          "${CMAKE_CURRENT_SOURCE_DIR}"
  VERBATIM)

install(FILES version.crty DESTINATION .)
install(FILES resources/res.crty DESTINATION resources)
