#!/bin/sh

# https://cmake.org/Wiki/Eclipse_CDT4_Generator
#echo 'Generating eclipse project files ...'

# out-of-source build begin
#mkdir build; cd build

# Note: The following command must be a directory, such as "../src"",
# do not use CMakeLists.txt instead, or the generated file and
# CMakeLists.txt in the same directory
#cmake -G "Eclipse CDT4 - Unix Makefiles" ../src
# out-of-source build end

# in-source build begin
cmake -G "Eclipse CDT4 - Unix Makefiles" .
# in-source build end
