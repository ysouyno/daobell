add_compile_options (--std=c++11)
add_compile_options (-Wall)
add_compile_options (-g)

find_package (Boost REQUIRED COMPONENTS regex)
if (NOT Boost_FOUND)
  message("Boost_FOUND is false")
endif ()

find_package (Threads REQUIRED)
if (THREAD_HAVE_PTHREAD_ARG)
#   target_compile_options(PUBLIC daobell_main "-pthread")
  target_compile_options(PUBLIC daobell_lib "-pthread")
endif ()

# find package cppunit
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
find_package (CPPUNIT REQUIRED)

aux_source_directory (. src)
include_directories (${Boost_INCLUDE_DIRS})

# add_executable (daobell_main ${src})
# target_link_libraries (daobell_main ${Boost_LIBRARIES})

add_library (daobell_lib SHARED ${src})
target_link_libraries (daobell_lib ${Boost_LIBRARIES})

if (CMAKE_THREAD_LIBS_INIT)
#   target_link_libraries(daobell_main "${CMAKE_THREAD_LIBS_INIT}")
  target_link_libraries(daobell_lib "${CMAKE_THREAD_LIBS_INIT}")
endif ()
