include_directories (${CMAKE_SOURCE_DIR}/src)

add_compile_options (--std=c++11)
add_compile_options (-Wall)
add_compile_options (-g)

# add_definitions (-DDAOBELL_TEST_DIR=\"$(top_srcdir)/test\")
add_definitions (-DDAOBELL_TEST_DIR="${CMAKE_SOURCE_DIR}/test")

aux_source_directory (. test)
add_executable (daobell_test ${test})

# post-build
add_custom_command (TARGET daobell_test POST_BUILD COMMAND
  "${CMAKE_BINARY_DIR}/daobell_test")

add_custom_target (tests "${CMAKE_BINARY_DIR}/daobell_test"
  DEPENDS daobell_test COMMENT "Running CPPUNIT tests...")

target_link_libraries (daobell_test cppunit daobell_lib)
