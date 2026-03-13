cmake_minimum_required(VERSION 3.16)
project( 2D_SVG_RENDERER CXX) 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

file(GLOB ALL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(SOURCE_FILE ${ALL_SOURCES})

    get_filename_component(EXE_NAME ${SOURCE_FILE} NAME_WE)
    
    add_executable(${EXE_NAME} ${SOURCE_FILE})
    
    target_compile_options(${EXE_NAME} PRIVATE -fpermissive)
    target_include_directories(${EXE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include" )
    target_link_libraries(${EXE_NAME} PRIVATE 
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        ZLIB::ZLIB
    )

endforeach()
