#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Zopfli::libzopfli" for configuration "Release"
set_property(TARGET Zopfli::libzopfli APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Zopfli::libzopfli PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "m"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libzopfli.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Zopfli::libzopfli )
list(APPEND _IMPORT_CHECK_FILES_FOR_Zopfli::libzopfli "${_IMPORT_PREFIX}/lib/libzopfli.a" )

# Import target "Zopfli::libzopflipng" for configuration "Release"
set_property(TARGET Zopfli::libzopflipng APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Zopfli::libzopflipng PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "Zopfli::libzopfli"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libzopflipng.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Zopfli::libzopflipng )
list(APPEND _IMPORT_CHECK_FILES_FOR_Zopfli::libzopflipng "${_IMPORT_PREFIX}/lib/libzopflipng.a" )

# Import target "Zopfli::zopfli" for configuration "Release"
set_property(TARGET Zopfli::zopfli APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Zopfli::zopfli PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/zopfli"
  )

list(APPEND _IMPORT_CHECK_TARGETS Zopfli::zopfli )
list(APPEND _IMPORT_CHECK_FILES_FOR_Zopfli::zopfli "${_IMPORT_PREFIX}/bin/zopfli" )

# Import target "Zopfli::zopflipng" for configuration "Release"
set_property(TARGET Zopfli::zopflipng APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Zopfli::zopflipng PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/zopflipng"
  )

list(APPEND _IMPORT_CHECK_TARGETS Zopfli::zopflipng )
list(APPEND _IMPORT_CHECK_FILES_FOR_Zopfli::zopflipng "${_IMPORT_PREFIX}/bin/zopflipng" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
