
cmake_minimum_required(VERSION 2.8)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(set_policy policy value)
	if (POLICY ${policy})
		cmake_policy(SET ${policy} ${value})
	endif ()
endmacro(set_policy)

# Set a few policies
set_policy(CMP0051 OLD)
set_policy(CMP0053 NEW)
set_policy(CMP0054 NEW)

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
