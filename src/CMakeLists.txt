add_compile_options (--std=c++11)
add_compile_options (-Wall)
add_compile_options (-g)

find_package (Boost REQUIRED COMPONENTS regex)
if (NOT Boost_FOUND)
  message ("No boost regex found")
endif ()

find_package (Threads REQUIRED)
if (THREAD_HAVE_PTHREAD_ARG)
  target_compile_options (PUBLIC daobell "-pthread")
endif ()

# find package cppunit
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
find_package (CPPUNIT REQUIRED)

aux_source_directory (. src)
include_directories (${Boost_INCLUDE_DIRS})

if (BUILD_LIBRARY)
  message (STATUS "Build project as library")
  add_definitions (-DBUILD_LIBRARY)
  add_library (daobell SHARED ${src})
else (BUILD_LIBRARY)
  message (STATUS "Build project as executable")
  add_executable (daobell ${src})
endif (BUILD_LIBRARY)

target_link_libraries (daobell ${Boost_LIBRARIES})

if (CMAKE_THREAD_LIBS_INIT)
  target_link_libraries (daobell "${CMAKE_THREAD_LIBS_INIT}" "rt")
endif ()
