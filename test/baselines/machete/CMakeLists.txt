cmake_minimum_required(VERSION 3.20)

project(Buff)

# Set C++ standard version
set(CMAKE_CXX_STANDARD 17)

# -O3 Optimization for release version
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Set parallel compilation level as 4
set(CMAKE_BUILD_PARALLEL_LEVEL 4)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE LIB_SRC *.cpp *.c)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(machete SHARED ${LIB_SRC})
