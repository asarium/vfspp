
if(MSVC)
	set(gtest_force_shared_crt ON)
endif(MSVC)

add_subdirectory(gtest-1.7.0)

# This keeps boost from automatically including the filesystem libs
add_definitions(-DBOOST_ALL_NO_LIB)

SET(TEST_WRITE_DIR "${CMAKE_CURRENT_BINARY_DIR}/test/resources")
FILE(MAKE_DIRECTORY "${TEST_WRITE_DIR}")

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_definitions(-DTEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")
add_definitions(-DTEST_WRITE_DIR="${TEST_WRITE_DIR}")

SET(TEST_SRCS
	globals.hpp
	globals.cpp
	core/core.cpp
	system/directory.cpp
	system/file.cpp
	merged/merged.cpp
	memory/memory.cpp
)

source_group(System REGULAR_EXPRESSION system/.*)

source_group(Core REGULAR_EXPRESSION core/.*)

source_group(Merged REGULAR_EXPRESSION merged/.*)

source_group(Memory REGULAR_EXPRESSION memory/.*)

if(VFSPP_7ZIP_SUPPORT)
	SET(TEST_SRCS
		${TEST_SRCS}
		7zip/7zip.cpp
	)
	
	source_group(7zip REGULAR_EXPRESSION 7zip/.*)
endif(VFSPP_7ZIP_SUPPORT)

add_executable(gtests ${TEST_SRCS})
target_link_libraries(gtests gtest_main gtest VFSPP)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(test gtests COMMENT "Running tests")

set_target_properties(gtests
	PROPERTIES
		FOLDER "test"
)
set_target_properties(test
	PROPERTIES
		FOLDER "test"
)
