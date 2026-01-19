cmake_minimum_required(VERSION 3.10)
project(nodepp VERSION 1.3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(NODEPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Search for Dependencies
find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

# interface-target
add_library(nodepp INTERFACE)
target_include_directories(nodepp INTERFACE ${NODEPP_INCLUDE_DIR})
target_link_libraries(nodepp INTERFACE 
Threads::Threads
OpenSSL::Crypto
OpenSSL::SSL
ZLIB::ZLIB
$<$<OR:$<BOOL:${MSVC}>,$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>>:ws2_32> )