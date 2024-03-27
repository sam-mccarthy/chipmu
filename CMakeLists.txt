cmake_minimum_required(VERSION 3.7)
project(chipmu)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
find_package(SDL2_mixer REQUIRED)
message("test")
set(SOURCE_FILES main.cpp System.cpp System.h)

add_executable(chipmu ${SOURCE_FILES})
target_link_libraries(chipmu SDL2::SDL2main SDL2::SDL2)
target_link_libraries(chipmu SDL2_mixer::SDL2_mixer)