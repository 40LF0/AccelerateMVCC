#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Kuku::kuku" for configuration "Release"
set_property(TARGET Kuku::kuku APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Kuku::kuku PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/kuku-2.1.lib"
  )

list(APPEND _cmake_import_check_targets Kuku::kuku )
list(APPEND _cmake_import_check_files_for_Kuku::kuku "${_IMPORT_PREFIX}/lib/kuku-2.1.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
