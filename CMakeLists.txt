cmake_minimum_required(VERSION 3.0)

project(skyfauna)

add_executable(skyfauna main.cpp Solver.cpp Tokenizer.cpp)

install(TARGETS skyfauna RUNTIME DESTINATION bin)
