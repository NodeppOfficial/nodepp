cmake_minimum_required(VERSION 3.10)
project(Nodepp_Examples)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# 1. Discover all examples
file(GLOB EXAMPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")

# 2. EXCLUDE the failing 3-Conio.cpp example
# This removes any entry from the list that contains "3-Conio"
list(FILTER EXAMPLE_FILES EXCLUDE REGEX "3-Conio|WSS|WS-load-test|57-mutex")

# 3. Process the remaining examples
foreach(SOURCE_FILE ${EXAMPLE_FILES})
    get_filename_component(EXE_NAME ${SOURCE_FILE} NAME_WE)
    
    add_executable(${EXE_NAME} ${SOURCE_FILE})
    
    target_compile_options(${EXE_NAME} PRIVATE -fpermissive)
    
    target_link_libraries(${EXE_NAME} PRIVATE 
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        ZLIB::ZLIB
    )
endforeach()

