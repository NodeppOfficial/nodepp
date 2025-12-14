cmake_minimum_required(VERSION 3.0.0)
project(nodepp VERSION 1.3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(NODEPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Search for ZLIB
find_package(ZLIB REQUIRED)
if(NOT ZLIB_FOUND)
    message(FATAL_ERROR "ZLIB not found. Please install libz-dev or equivalent files.")
endif()

# Search for OpenSSL
find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found. Please install libopenssl-dev or equivalent files.")
endif()

# interface-target
add_library(nodepp INTERFACE)
target_include_directories(nodepp INTERFACE ${NODEPP_INCLUDE_DIR})
target_link_libraries(nodepp INTERFACE -lssl -lcrypto -lz $<$<OR:$<BOOL:${MSVC}>,$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>>:ws2_32> )