cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

if (TARGET logy)
	return()
endif ()

project(logy
	VERSION 1.0.0
	DESCRIPTION "Yet another logging library"
	HOMEPAGE_URL "https://github.com/cfnptr/logy"
	LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_subdirectory(libraries/mpio)

set(MPMT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/mpmt)

configure_file(
	cmake/defines.h.in
	logy/defines.h)

add_library(logy STATIC
	source/logger.c)
target_link_libraries(logy PUBLIC
	mpio
	mpmt)
target_include_directories(logy PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)
