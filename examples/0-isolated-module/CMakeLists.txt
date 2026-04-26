cmake_minimum_required(VERSION 3.16)
project(Isolated_Modules_System CXX) 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB    REQUIRED)

set(NODEPP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

add_library(Modules_Lib SHARED 
    module1.cpp 
    module2.cpp
)

target_include_directories(Modules_Lib PRIVATE 
    ${NODEPP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE 
    Modules_Lib
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

target_include_directories(main PRIVATE 
    ${NODEPP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(main PRIVATE -fpermissive)
target_compile_options(Modules_Lib PRIVATE -fpermissive)