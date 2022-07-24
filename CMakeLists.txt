# Copyright 2021-2022 Nikita Fediuchin. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.16)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

if (TARGET logy-static)
	return()
endif ()

project(logy
	VERSION 1.1.0
	DESCRIPTION "Yet another logging library"
	HOMEPAGE_URL "https://github.com/cfnptr/logy"
	LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

option(LOGY_BUILD_SHARED "Build LOGY shared library" ON)

add_subdirectory(libraries/mpio)

set(MPMT_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MPMT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MPMT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/mpmt)

configure_file(
	cmake/defines.h.in
	include/logy/defines.h)

set(LOGY_SOURCES
	source/logger.c)
	
set(LOGY_LINK_LIBRARIES
	mpio-static mpmt-static)

add_library(logy-static STATIC ${LOGY_SOURCES})
target_link_libraries(logy-static PUBLIC
	${LOGY_LINK_LIBRARIES})
target_include_directories(logy-static PUBLIC
	${PROJECT_BINARY_DIR}/include
	${PROJECT_SOURCE_DIR}/include)

if (LOGY_BUILD_SHARED)
	add_library(logy-shared SHARED ${LOGY_SOURCES})
	set_target_properties(logy-shared PROPERTIES
		OUTPUT_NAME "logy"
		WINDOWS_EXPORT_ALL_SYMBOLS ON)
	target_link_libraries(logy-shared PUBLIC
		${LOGY_LINK_LIBRARIES})
	target_include_directories(logy-shared PUBLIC
		${PROJECT_BINARY_DIR}/include
		${PROJECT_SOURCE_DIR}/include)
endif ()
