# SPDX-License-Identifier: MIT
# Copyright (C) 2025 Chloe Eather

cmake_minimum_required(VERSION 4.0)

set(SKYFAUNA_CLI_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_executable(skyfauna-cli ${SKYFAUNA_CLI_SOURCES})
target_include_directories(skyfauna-cli PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(skyfauna-cli PRIVATE skyfauna)
set_target_properties(skyfauna-cli PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	OUTPUT_NAME "sfc"
)

add_executable(skyfauna::cli ALIAS skyfauna-cli)

