cmake_minimum_required(VERSION 3.0)

project(skyfauna VERSION 0.1.0 LANGUAGES CXX)

configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY)

add_executable(skyfauna main.cpp lex.cpp)

set_property(TARGET skyfauna PROPERTY CXX_STANDARD 20)

install(TARGETS skyfauna RUNTIME DESTINATION bin)
