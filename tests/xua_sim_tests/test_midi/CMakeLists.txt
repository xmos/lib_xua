cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)

set(APP_INCLUDES src)
set(COMMON_FLAGS -O2 -g)

include(${CMAKE_CURRENT_LIST_DIR}/../../../examples/deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

project(test_midi_xs3)
set(APP_HW_TARGET XK-EVK-XU316)
set(APP_COMPILER_FLAGS ${COMMON_FLAGS})
XMOS_REGISTER_APP()

project(test_midi_loopback)
set(APP_HW_TARGET XK-EVK-XU316)
set(APP_COMPILER_FLAGS ${COMMON_FLAGS} -DMIDI_LOOPBACK=1)
XMOS_REGISTER_APP()

project(test_midi_xs2)
set(APP_HW_TARGET XCORE-200-EXPLORER)
set(APP_COMPILER_FLAGS ${COMMON_FLAGS})
XMOS_REGISTER_APP()
