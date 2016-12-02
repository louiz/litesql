# - Find SQLite3
# Find the SQLite3 library
#
# This module defines the following variables:
#   SQLITE3_FOUND  -  True if library and include directory are found
# If set to TRUE, the following are also defined:
#   SQLITE3_INCLUDE_DIRS  -  The directory where to find the header file
#   SQLITE3_LIBRARIES  -  Where to find the library file
#   SQLITE3_GENERATE_CPP - A function, to be used like this:
#
# For conveniance, these variables are also set. They have the same values
# than the variables above.  The user can thus choose his/her prefered way
# to write them.
#   SQLITE3_INCLUDE_DIR
#   SQLITE3_LIBRARY
#
# This file is in the public domain

include(FindPkgConfig)
pkg_check_modules(SQLITE3 sqlite3)

if(NOT SQLITE3_FOUND)
  find_path(SQLITE3_INCLUDE_DIRS NAMES sqlite3.h
    DOC "The SQLite3 include directory")

  find_library(SQLITE3_LIBRARIES NAMES sqlite3
    DOC "The SQLite3 library")

  # Use some standard module to handle the QUIETLY and REQUIRED arguments, and
  # set SQLITE3_FOUND to TRUE if these two variables are set.
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SQLITE3 REQUIRED_VARS SQLITE3_LIBRARIES SQLITE3_INCLUDE_DIRS)

  # Compatibility for all the ways of writing these variables
  if(SQLITE3_FOUND)
    set(SQLITE3_INCLUDE_DIR ${SQLITE3_INCLUDE_DIRS})
    set(SQLITE3_LIBRARY ${SQLITE3_LIBRARIES})
  endif()
endif()

mark_as_advanced(SQLITE3_INCLUDE_DIRS SQLITE3_LIBRARIES)
