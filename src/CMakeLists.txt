cmake_minimum_required(VERSION 3.0.2)
project(daobell)

add_compile_options(--std=c++11)
add_compile_options(-Wall)
add_compile_options(-g)

find_package(Boost REQUIRED COMPONENTS regex)
if(NOT Boost_FOUND)
  message("Boost_FOUND is false")
endif()

find_package(Threads REQUIRED)
if(THREAD_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC daobell_main "-pthread")
endif()

aux_source_directory(. src)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(daobell_main ${src})
target_link_libraries(daobell_main ${Boost_LIBRARIES})

if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(daobell_main "${CMAKE_THREAD_LIBS_INIT}")
endif()
