cmake_minimum_required(VERSION 3.20)

project(PerfTest)

add_subdirectory(baselines/alp)
add_subdirectory(baselines/deflate)
add_subdirectory(baselines/fpc)
add_subdirectory(baselines/lz4)
add_subdirectory(baselines/chimp128)
add_subdirectory(baselines/gorilla)
add_subdirectory(baselines/elf)
add_subdirectory(baselines/machete)
add_subdirectory(baselines/lz77)
add_subdirectory(baselines/sz2)
# add_subdirectory(baselines/buff)
add_subdirectory(baselines/snappy)
# add_subdirectory(baselines/zstd/build/cmake)
add_subdirectory(baselines/sim_piece)

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/baselines/alp/include)

message(STATUS "[SelfCheck] Loading GoogleTest module")

# Set GoogleTest version
set(GTEST_VER "1.14.0")

# Load FetchContent Module
include(FetchContent)

# Auto fetch GoogleTest source code
FetchContent_Declare(
        googletest
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/google/googletest/archive/refs/tags/v${GTEST_VER}.zip)
# For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

message(STATUS "[SelfCheck] Loaded GoogleTest module")

# Scan and collect all test file
file(GLOB_RECURSE TEST_SRC *_test.cc)

enable_testing()

include(GoogleTest)

add_executable(PerformanceProgram Perf.cc)
target_link_libraries(PerformanceProgram PRIVATE ALP chimp deflate elf fpc gorilla lz77 lz4 machete sz snappy sim_piece GTest::gtest_main)
gtest_discover_tests(PerformanceProgram)
