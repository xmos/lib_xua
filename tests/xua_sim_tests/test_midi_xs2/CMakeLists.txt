cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_midi_xs2)

set(APP_HW_TARGET XCORE-200-EXPLORER)

set(APP_INCLUDES src)

set(APP_COMPILER_FLAGS -O2 -g)

include(${CMAKE_CURRENT_LIST_DIR}/../../../deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

XMOS_REGISTER_APP()
