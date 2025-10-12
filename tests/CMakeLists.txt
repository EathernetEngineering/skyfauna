# SPDX-License-Identifier: MIT
# Copyright (C) 2025 Chloe Eather

cmake_minimum_required(VERSION 3.3...4.0)

if (TARGET gtest_main)
	message(STATUS "Found Google Test, building tests.")
elseif (SKYFAUNA_GOOGLETEST_DIR)
	add_subdirectory(SKYFAUNA_GOOGLETEST_DIR)
	if (NOT TARGET gtest_main)
		message(SEND_ERROR "Could not add googletest."
			"Are you sure ${SKYFAUNA_GOOGLETEST_DIR} is correct?")
	endif()
elseif (SKYFAUNA_FIND_GOOGLETEST)
	find_package(PkgConfig REQUIRED)
	find_package(GTest REQUIRED)
else()
	message(SEND_ERROR "Did not find googletest, tests will not be built."
		"If you have provided an external googletest set CEE_GOOGLETEST_DIR.")
endif()

# Use:
#
# skyfauna_add_unit_test(
# 	TARGET <name>
# 	SRCS <file1> [<file2> ...]
# 	[LIBS <lib1> [<lib2> ...]]
# 	[DEFINES <DEF_1> [DEF2 ...]]
# 	[INCLUDE_DIRS <dir1> [<dir2> ...]]
# 	[ENVIRONMENT <ENV_1> [<ENV_2> ...]]
# )

function(skyfauna_add_unit_test)
	set(one_value_args TARGET)
	set(multi_value_args SRCS LIBS ENVIRONMENT DEFINES INCLUDE_DIRS)
	cmake_parse_arguments(
		ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})
	set(target test_${ARG_TARGET})
	set(SRC_COPY ${ARG_SRCS})
	add_executable(${target} ${SRC_COPY})
	target_compile_definitions(${target} PUBLIC ${ARG_DEFINES})
	target_include_directories(${target} PRIVATE
		${gtest_SOURCE_DIR}/include
		${ARG_INCLUDE_DIRS}
	)
	target_link_libraries(${target} PRIVATE ${ARG_LIBS})
	if (TARGET gtest_main)
		target_link_libraries(${target} PRIVATE gtest_main)
	elseif (SKYFAUNA_FIND_GOOGLETEST)
		target_link_libraries(${target} PRIVATE GTest::gtest_main)
	endif()
	set_target_properties(${target}
		PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON)
	add_test(NAME skyfauna_${target} COMMAND ${target})
	if (DEFINED ARG_ENVIRNOMENT)
		set_test_properties(skyfauna_${target} PROPERTIES ENVIRONMENT ${ARG_ENVRIONMENT})
	endif()
	set_property(TARGET ${target} PROPERTY FOLDER "skyfauna tests")
endfunction()

skyfauna_add_unit_test(
	TARGET lex
	SRCS lex.cpp
	LIBS skyfauna
)

