add_executable(adb_studio_dashboard_tests unit/device_dashboard_view_model_test.cpp)
target_link_libraries(adb_studio_dashboard_tests PRIVATE adb_studio_core Qt6::Test)
adb_studio_declare_layer(adb_studio_dashboard_tests "tests")
adb_studio_enable_strict(adb_studio_dashboard_tests)
add_test(NAME dashboard-view-model COMMAND adb_studio_dashboard_tests)
set_tests_properties(dashboard-view-model PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

add_executable(adb_studio_theme_tests unit/theme_service_test.cpp)
target_link_libraries(adb_studio_theme_tests PRIVATE adb_studio_core Qt6::Test Qt6::Gui)
adb_studio_declare_layer(adb_studio_theme_tests "tests")
adb_studio_enable_strict(adb_studio_theme_tests)
add_test(NAME theme-service COMMAND adb_studio_theme_tests)
set_tests_properties(theme-service PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
