# Locate the glfw library
# This module defines the following variables:
# GLFW_LIBRARY, the name of the library;
# GLFW_INCLUDE_DIR, where to find glfw include files.
# GLFW_FOUND, true if both the GLFW_LIBRARY and GLFW_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you could define an environment variable called
# GLFW_ROOT which points to the root of the glfw library installation. This is pretty useful
# on a Windows platform.
#
#
# Usage example to compile an "executable" target to the glfw library:
#
# FIND_PACKAGE (glfw REQUIRED)
# INCLUDE_DIRECTORIES (${GLFW_INCLUDE_DIR})
# ADD_EXECUTABLE (executable ${EXECUTABLE_SRCS})
# TARGET_LINK_LIBRARIES (executable ${GLFW_LIBRARY})
#
# TODO:
# Allow the user to select to link to a shared library or to a static library.

#Search for the include file...
FIND_PATH(GLFW_INCLUDE_DIR GL/glfw.h DOC "Path to GLFW include directory."
  HINTS
  $ENV{GLFW_ROOT}
  PATH_SUFFIX include
  PATHS
  /usr/include/
  /usr/local/include/
  # By default headers are under GL subfolder
  /usr/include/GL
  /usr/local/include/GL
  ${GLFW_ROOT_DIR}/include/ # added by ptr
)

FIND_LIBRARY(GLFW_LIBRARY_TEMP DOC "Absolute path to GLFW library."
  NAMES glfw GLFW.lib
  HINTS
  $ENV{GLFW_ROOT}
  # In the expanded GLFW source archive. Should be uncommon, but whatever.
  PATH_SUFFIXES lib/win32 lib/cocoa lib/x11
  PATHS
  /usr/local/lib
  /usr/lib
  ${GLFW_ROOT_DIR}/lib-msvc100/release # added by ptr
)

SET(GLFW_FOUND "NO")
IF(GLFW_LIBRARY_TEMP AND GLFW_INCLUDE_DIR)
  SET(GLFW_FOUND "YES")
  message(STATUS "Found GLFW: ${GLFW_LIBRARY_TEMP}")

  # For MinGW library
  IF(MINGW)
    SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
    SET(GLFW_LIBRARY_TEMP ${MINGW32_LIBRARY} ${GLFW_LIBRARY_TEMP})
  ENDIF(MINGW)

  # OS X uses the Cocoa port so we need to link against Cocoa
  IF(APPLE)
    SET(GLFW_LIBRARY_TEMP ${GLFW_LIBRARY_TEMP} "-framework Cocoa -framework IOKit")
    SET(GLFW_LIBRARY ${GLFW_LIBRARY_TEMP})
  ENDIF(APPLE)

  # Set the final string here so the GUI reflects the final state.
  SET(GLFW_LIBRARY ${GLFW_LIBRARY_TEMP} CACHE STRING "Where the GLFW Library can be found")
  # Set the temp variable to INTERNAL so it is not seen in the CMake GUI
  SET(GLFW_LIBRARY_TEMP "" CACHE INTERNAL "")
ENDIF(GLFW_LIBRARY_TEMP AND GLFW_INCLUDE_DIR)

