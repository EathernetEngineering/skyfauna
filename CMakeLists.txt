cmake_minimum_required(VERSION 3.0)

project(skyfauna VERSION 1.0.0 LANGUAGES CXX)

set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED 20)

configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY)

add_executable(skyfauna main.cpp solver.cpp parser.cpp)

install(TARGETS skyfauna RUNTIME DESTINATION bin)
