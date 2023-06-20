#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Kuku::kuku" for configuration "RelWithDebInfo"
set_property(TARGET Kuku::kuku APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Kuku::kuku PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C;CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/kuku-2.1.lib"
  )

list(APPEND _cmake_import_check_targets Kuku::kuku )
list(APPEND _cmake_import_check_files_for_Kuku::kuku "${_IMPORT_PREFIX}/lib/kuku-2.1.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
