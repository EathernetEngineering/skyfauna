# SPDX-License-Identifier: MIT
# Copyright (C) 2025 Chloe Eather

cmake_minimum_required(VERSION 4.0)

option(SKYFAUNA_BUILD_SHARED "Build tests" OFF)

set(SKYFAUNA_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/lex.cpp
)

if (SKYFAUNA_BUILD_SHARED)
	add_library(skyfauna SHARED ${SKYFAUNA_SOURCES})
	set_target_properties(skyfauna PROPERTIES
		VERSION ${skyfauna_VERSION}
		SOVERSION ${skyfauna_VERSION_MAJOR}
	)
else ()
	add_library(skyfauna STATIC ${SKYFAUNA_SOURCES})
endif ()

target_include_directories(skyfauna PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(skyfauna PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_sources(skyfauna PUBLIC
	FILE_SET public_headers
		TYPE HEADERS
		BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}/include"
			"${CMAKE_CURRENT_BINARY_DIR}/include"
		FILES
			"${CMAKE_CURRENT_SOURCE_DIR}/include/skyfauna/lex.h"
			"${CMAKE_CURRENT_SOURCE_DIR}/include/skyfauna/solver.h"
			"${CMAKE_CURRENT_BINARY_DIR}/include/skyfauna/config.h"
)
add_library(skyfauna::skyfauna ALIAS skyfauna)
set_target_properties(skyfauna PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/skyfauna/config.h"
	@ONLY
)

