cmake_minimum_required(VERSION 4.0.0)

project(nodepp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(NODEPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Search for OpenSSL
find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL development files.")
endif()

# Search for ZLIB
find_package(ZLIB REQUIRED)
if(NOT ZLIB_FOUND)
    message(FATAL_ERROR "ZLIB not found. Please install ZLIB development files.")
endif()