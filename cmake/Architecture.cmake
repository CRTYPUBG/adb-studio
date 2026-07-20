function(adb_studio_declare_layer target layer)
  set_property(TARGET "${target}" PROPERTY ADB_STUDIO_LAYER "${layer}")
endfunction()

function(adb_studio_enable_strict target)
  target_compile_features("${target}" PRIVATE cxx_std_20)
  set_property(TARGET "${target}" PROPERTY CXX_EXTENSIONS OFF)
endfunction()
