# SPDX-License-Identifier: MIT
# Copyright (C) 2025 Chloe Eather

cmake_minimum_required(VERSION 3.3...4.0)

project(skyfauna VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (SKYFAUNA_EXTERNAL_SPDLOG)
	add_subdirectory(${SKYFAUNA_EXTERNAL_SPDLOG})
else()
	find_package(spdlog REQUIRED)
endif()

option(SKYFAUNA_BUILD_TESTS "Build tests" OFF)
option(SKYFAUNA_FIND_GOOGLETEST "Use find_package to find googletest. Alternatively set SKYFAUNA_GOOGLETEST_DIR" ON)

option(SKYFAUNA_ENABLE_ASSERTIONS "Allow assertions at runtime" OFF)

add_subdirectory(skyfauna-common)
add_subdirectory(skyfauna)
add_subdirectory(skyfauna-cli)

if (SKYFAUNA_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

install(TARGETS skyfauna-common skyfauna skyfauna-cli
	EXPORT skyfaunaTargets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/skyfaunaConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/skyfaunaConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/skyfaunaConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/skyfauna"
)

install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/skyfaunaConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/skyfaunaConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIB}/cmake/skyfauna"
)

export(EXPORT skyfaunaTargets
	NAMESPACE skyfauna::
	FILE "${CMAKE_CURRENT_BINARY_DIR}/skyfaunaTargets.cmake"
)

