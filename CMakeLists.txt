# 1. Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# 2. Project Name and Language (CXX = C++)
project(TradeEngine CXX)

# 3. Force Modern C++20 standard for low latency features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 4. Find the Operating System's Threading Library (Required for Logger)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# 5. Tell the compiler where to find your .h files
# When you say #include "Logger.h", it knows to look inside core/include/
include_directories(core/include)

# 6. Build the executable from main.cpp
add_executable(TradeEngine app/main.cpp)

# 7. Link the threading library to your executable
target_link_libraries(TradeEngine PRIVATE Threads::Threads)

# 8. (Windows Specific) Force static linking to avoid missing DLL errors
if(WIN32)
    target_link_options(TradeEngine PRIVATE "-static")
endif()