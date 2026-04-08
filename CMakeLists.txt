cmake_minimum_required(VERSION 3.10)
project(HFT_MatchingEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Removed -flto because the bundled MinGW compiler doesn't support it
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math -pthread")

include_directories(include)

# Add executable for benchmark
add_executable(benchmark_engine benchmark_engine/benchmark_engine.cpp)