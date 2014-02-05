if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	include(toolchain-gcc)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	include(toolchain-clang)
ENDIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	