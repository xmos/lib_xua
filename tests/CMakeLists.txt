cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(tests)

add_subdirectory(xua_hw_tests)
add_subdirectory(xua_sim_tests)
add_subdirectory(xua_unit_tests)
