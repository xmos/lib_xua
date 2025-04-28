cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)

set(APP_INCLUDES src)
set(COMMON_FLAGS -O2 -g -report -DSIMULATION=1 -DXUA_LOW_POWER_NON_STREAMING=1)

include(${CMAKE_CURRENT_LIST_DIR}/../../../examples/deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

project(test_audio_stop_start)
set(APP_HW_TARGET XK-EVK-XU316)
set(APP_COMPILER_FLAGS ${COMMON_FLAGS})
XMOS_REGISTER_APP()
