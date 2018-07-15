cmake_minimum_required (VERSION 3.0.2)
project (daobell)

option (BUILD_LIBRARY "Build project as library" ON)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory (src)
add_subdirectory (test)
