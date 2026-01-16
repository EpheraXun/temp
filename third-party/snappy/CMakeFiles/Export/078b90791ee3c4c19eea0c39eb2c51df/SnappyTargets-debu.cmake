#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debu".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Snappy::snappy" for configuration "Debu"
set_property(TARGET Snappy::snappy APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBU)
set_target_properties(Snappy::snappy PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBU "CXX"
  IMPORTED_LOCATION_DEBU "${_IMPORT_PREFIX}/lib/libsnappy.a"
  )

list(APPEND _cmake_import_check_targets Snappy::snappy )
list(APPEND _cmake_import_check_files_for_Snappy::snappy "${_IMPORT_PREFIX}/lib/libsnappy.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
