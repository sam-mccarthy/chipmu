cmake_minimum_required(VERSION 3.7)
project(chipmu)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2mixer_INCLUDE_DIRS})

set(SOURCE_FILES main.cpp System.cpp System.h)
add_executable(chipmu ${SOURCE_FILES})

target_link_libraries(chipmu ${SDL2_LIBRARIES} ${SDL2mixer_LIBRARIES})
