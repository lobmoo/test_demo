#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "iceoryx_platform::iceoryx_platform" for configuration "Release"
set_property(TARGET iceoryx_platform::iceoryx_platform APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(iceoryx_platform::iceoryx_platform PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libiceoryx_platform.a"
  )

list(APPEND _cmake_import_check_targets iceoryx_platform::iceoryx_platform )
list(APPEND _cmake_import_check_files_for_iceoryx_platform::iceoryx_platform "${_IMPORT_PREFIX}/lib/libiceoryx_platform.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
