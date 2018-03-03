include_directories (${CMAKE_SOURCE_DIR}/src)
add_executable (run_tests cppunit_demo.cpp)

# post-build
# add_custom_command (TARGET run_tests POST_BUILD COMMAND
#   "${CMAKE_SOURCE_DIR}/test/run_tests")

add_custom_target (tests "${CMAKE_SOURCE_DIR}/test/run_tests"
  DEPENDS run_tests COMMENT "Running CPPUNIT tests...")

target_link_libraries (run_tests cppunit)
