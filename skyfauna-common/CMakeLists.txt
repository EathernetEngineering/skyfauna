# SPDX-License-Identifier: MIT
# Copyright (C) 2025 Chloe Eather

cmake_minimum_required(VERSION 4.0)

find_package(spdlog REQUIRED)

add_library(skyfauna-common STATIC log.cpp util.cpp)
target_include_directories(skyfauna-common PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(skyfauna-common PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(skyfauna-common PUBLIC spdlog::spdlog)
set_target_properties(skyfauna-common PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/skyfauna/common/config.h"
	@ONLY
)

target_sources(skyfauna-common PUBLIC
	FILE_SET public_headers
		TYPE HEADERS
		BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}/include"
			"${CMAKE_CURRENT_BINARY_DIR}/include"
		FILES
			"${CMAKE_CURRENT_BINARY_DIR}/include/skyfauna/common/config.h"
			"${CMAKE_CURRENT_SOURCE_DIR}/include/skyfauna/common/log.h"
			"${CMAKE_CURRENT_SOURCE_DIR}/include/skyfauna/common/util.h"
)

add_library(skyfauna::skyfauna-common ALIAS skyfauna-common)

