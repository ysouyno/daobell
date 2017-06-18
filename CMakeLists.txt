cmake_minimum_required(VERSION 3.0.2)
project(daobell)

add_compile_options(--std=c++11)
add_compile_options(-Wall)
add_compile_options(-g)

find_package(Boost REQUIRED COMPONENTS regex)
if(NOT Boost_FOUND)
  message("Boost_FOUND is false")
endif()

aux_source_directory(./src srcs)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(daobell ${srcs})
target_link_libraries(daobell ${Boost_LIBRARIES})