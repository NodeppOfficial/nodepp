cmake_minimum_required(VERSION 3.16)
project( 2D_SVG_RENDERER CXX) 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build")

add_executable( main ./main.cpp )
target_include_directories( main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" )
