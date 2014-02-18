
include(compiler_checks)

SET(UTF8_HEADERS
	"${UTF8_INCLUDE_DIR}/utf8.h"
	"${UTF8_INCLUDE_DIR}/utf8/checked.h"
	"${UTF8_INCLUDE_DIR}/utf8/core.h"
	"${UTF8_INCLUDE_DIR}/utf8/unchecked.h"
)

SET(VFS_HEADERS
	"${VSFPP_INCLUDE_DIR}/VFSPP/core.hpp"
	"${VSFPP_INCLUDE_DIR}/VFSPP/system.hpp"
	"${VSFPP_INCLUDE_DIR}/VFSPP/merged.hpp"
	"${VSFPP_INCLUDE_DIR}/VFSPP/memory.hpp"
	"${VSFPP_INCLUDE_DIR}/VFSPP/util.hpp"
	"${VSFPP_INCLUDE_DIR}/VFSPP/defines.hpp"
	"${CMAKE_CURRENT_BINARY_DIR}/vfspp_export.h"
	"${CMAKE_CURRENT_BINARY_DIR}/compiler.h"
	${UTF8_HEADERS}
)

SET(VFS_SRC
	VFSPP.cpp
	system/PhysicalEntry.cpp
	system/PhysicalFileSystem.cpp
	merged/MergedEntry.cpp
	merged/MergedFileSystem.cpp
	memory/MemoryFileSystem.cpp
	memory/MemoryFileEntry.cpp
)

source_group(System REGULAR_EXPRESSION system/.*)

source_group(Merged REGULAR_EXPRESSION merged/.*)

source_group(Memory REGULAR_EXPRESSION memory/.*)

source_group(External\\UTF8 FILES ${UTF8_HEADERS})

if(VFSPP_7ZIP_SUPPORT)
	SET(VFS_HEADERS
		${VFS_HEADERS}
		"${VSFPP_INCLUDE_DIR}/VFSPP/7zip.hpp"
	)

	SET(VFS_SRC
		${VFS_SRC}
		${7Z_SOURCES}
		7zip/SevenZipFileSystem.cpp
		7zip/SevenZipFileEntry.cpp
	)

	source_group(7zip REGULAR_EXPRESSION 7zip/.*)
	source_group(External\\7ZIP FILES ${7Z_SOURCES})
endif(VFSPP_7ZIP_SUPPORT)

SET(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS filesystem system iostreams REQUIRED)

include(GenerateExportHeader)

add_compiler_export_flags()

if(VFSPP_BUILD_SHARED)
	set(LIB_TYPE SHARED)
else(VFSPP_BUILD_SHARED)
	set(LIB_TYPE STATIC)
endif(VFSPP_BUILD_SHARED)

ADD_LIBRARY(VFSPP ${LIB_TYPE} ${VFS_SRC} ${VFS_HEADERS})

generate_export_header(VFSPP)

SET(INCLUDE_DIRS ${VSFPP_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS})
set_target_properties(VFSPP
	PROPERTIES
		INCLUDE_DIRECTORIES "${INCLUDE_DIRS};${UTF8_INCLUDE_DIR}"
		INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIRS}"
)

SET(COMPILE_DEFS BOOST_ALL_NO_LIB)
IF(NOT VFSPP_BUILD_SHARED)
	SET(COMPILE_DEFS ${COMPILE_DEFS} VFSPP_STATIC_DEFINE)
ENDIF(NOT VFSPP_BUILD_SHARED)

IF(MSVC)
	# Stupid MSVC with your unfixable warnings...
	SET(COMPILE_DEFS ${COMPILE_DEFS} _SCL_SECURE_NO_WARNINGS)
ENDIF(MSVC)

set_target_properties(VFSPP
	PROPERTIES
		COMPILE_DEFINITIONS "${COMPILE_DEFS}"
		INTERFACE_COMPILE_DEFINITIONS "${COMPILE_DEFS}"
)

target_link_libraries(VFSPP ${Boost_LIBRARIES})

INSTALL(TARGETS VFSPP
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin)

install(DIRECTORY "${VSFPP_INCLUDE_DIR}" DESTINATION include)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vfspp_export.h" DESTINATION include)
