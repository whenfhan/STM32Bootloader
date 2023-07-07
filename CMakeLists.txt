cmake_minimum_required(VERSION 3.10)
project(MAIN)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

link_directories(${PROJECT_SOURCE_DIR}/lib)

if ("${CMAKE_GENERATOR}" MATCHES "MinGW")
    add_subdirectory(src)
endif()
add_subdirectory(test)