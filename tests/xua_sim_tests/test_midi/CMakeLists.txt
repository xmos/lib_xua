cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_midi)

set(APP_HW_TARGET XCORE-AI-EXPLORER)

set(APP_INCLUDES src)

set(APP_COMPILER_FLAGS_xs3 -O2 -g)

set(APP_COMPILER_FLAGS_LOOPBACK -O2 -g -DMIDI_LOOPBACK=1)

include(${CMAKE_CURRENT_LIST_DIR}/../../../deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

XMOS_REGISTER_APP()
