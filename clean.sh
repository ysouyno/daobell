#!/bin/sh

# clean cmake files
find . -name "CMakeCache.txt" | xargs rm -rf
find . -name "cmake_install.cmake" | xargs rm -rf
find . -name "Makefile" | xargs rm -rf
find . -name "CMakeFiles" | xargs rm -rf

# clean eclipse project files
find . -name ".cproject" | xargs rm -rf
find . -name ".project" | xargs rm -rf

# clean executable file
find . -name "daobell_main" | xargs rm -rf
find . -name "daobell_test" | xargs rm -rf
find . -name "libdaobell_lib.a" | xargs rm -rf
find . -name "libdaobell_lib.so" | xargs rm -rf
