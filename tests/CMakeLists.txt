add_executable(adb_studio_dashboard_tests unit/device_dashboard_view_model_test.cpp)
target_link_libraries(adb_studio_dashboard_tests PRIVATE adb_studio_core Qt6::Test)
adb_studio_declare_layer(adb_studio_dashboard_tests "tests")
adb_studio_enable_strict(adb_studio_dashboard_tests)
add_test(NAME dashboard-view-model COMMAND adb_studio_dashboard_tests)
set_tests_properties(dashboard-view-model PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(adb_studio_workspace_tests unit/workspace_view_model_test.cpp)
target_link_libraries(adb_studio_workspace_tests PRIVATE adb_studio_core Qt6::Test)
adb_studio_declare_layer(adb_studio_workspace_tests "tests")
adb_studio_enable_strict(adb_studio_workspace_tests)
add_test(NAME workspace-view-model COMMAND adb_studio_workspace_tests)
set_tests_properties(workspace-view-model PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(adb_studio_theme_tests unit/theme_service_test.cpp)
target_link_libraries(adb_studio_theme_tests PRIVATE adb_studio_core Qt6::Test Qt6::Gui)
adb_studio_declare_layer(adb_studio_theme_tests "tests")
adb_studio_enable_strict(adb_studio_theme_tests)
add_test(NAME theme-service COMMAND adb_studio_theme_tests)
set_tests_properties(theme-service PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(adb_studio_diagnostics_tests unit/device_diagnostics_test.cpp)
target_link_libraries(adb_studio_diagnostics_tests PRIVATE adb_studio_core Qt6::Test)
adb_studio_declare_layer(adb_studio_diagnostics_tests "tests")
adb_studio_enable_strict(adb_studio_diagnostics_tests)
add_test(NAME device-diagnostics COMMAND adb_studio_diagnostics_tests)
set_tests_properties(device-diagnostics PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(adb_studio_probe_integration_tests
  integration/system_device_probe_integration_test.cpp)
target_link_libraries(adb_studio_probe_integration_tests PRIVATE adb_studio_core Qt6::Test)
adb_studio_declare_layer(adb_studio_probe_integration_tests "tests")
adb_studio_enable_strict(adb_studio_probe_integration_tests)
add_test(NAME system-device-probe-integration COMMAND adb_studio_probe_integration_tests)
set_tests_properties(system-device-probe-integration PROPERTIES
  TIMEOUT 20
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(adb_studio_mirror_integration_tests
  integration/mirror_service_integration_test.cpp)
add_executable(adb_studio_process_failure_fixture
  integration/process_failure_fixture.cpp)
adb_studio_declare_layer(adb_studio_process_failure_fixture "tests")
adb_studio_enable_strict(adb_studio_process_failure_fixture)
target_compile_definitions(adb_studio_mirror_integration_tests PRIVATE
  ADB_STUDIO_PROCESS_FAILURE_FIXTURE="$<TARGET_FILE:adb_studio_process_failure_fixture>")
add_dependencies(adb_studio_mirror_integration_tests adb_studio_process_failure_fixture)
target_link_libraries(adb_studio_mirror_integration_tests PRIVATE adb_studio_core Qt6::Test Qt6::Gui)
adb_studio_declare_layer(adb_studio_mirror_integration_tests "tests")
adb_studio_enable_strict(adb_studio_mirror_integration_tests)
add_test(NAME mirror-service-integration COMMAND adb_studio_mirror_integration_tests)
set_tests_properties(mirror-service-integration PROPERTIES
  TIMEOUT 20
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
