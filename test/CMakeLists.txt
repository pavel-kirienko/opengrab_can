#
# Copyright (c) 2014 Pavel Kirienko <pavel.kirienko@gmail.com>
#

cmake_minimum_required(VERSION 2.8)

project(zubax_gps_test)

find_library(UAVCAN_LIB uavcan REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -pedantic -std=c++0x -g3")  # GCC or Clang

add_executable(test test.cpp)
target_link_libraries(test ${UAVCAN_LIB} rt)
