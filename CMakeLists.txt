
cmake_minimum_required(VERSION 2.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set a few policies
cmake_policy(SET CMP0051 OLD)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0054 NEW)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

PROJECT(VFSPP)

option(VFSPP_BUILD_SHARED "Build the library as a shared library" OFF)

option(VFSPP_BUILD_TESTS "Build the tests of the library" OFF)

option(VFSPP_7ZIP_SUPPORT "Enable support for 7zip archives" ON)

option(VFSPP_USECXX11 "If available use features of c++11" ON)

SET(VSFPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

include(toolchain)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(src)

IF(VFSPP_BUILD_TESTS)
	ADD_SUBDIRECTORY(test)
ENDIF(VFSPP_BUILD_TESTS)
