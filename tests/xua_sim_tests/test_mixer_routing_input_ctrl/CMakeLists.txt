cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_mixer_routing_input_ctrl)

set(APP_HW_TARGET test_xs3_600.xn)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

set(APP_INCLUDES src
	         ${CMAKE_CURRENT_LIST_DIR}/../shared)

set(APP_COMPILER_FLAGS -O3
                       -DXUD_WEAK_API=1)

include(${CMAKE_CURRENT_LIST_DIR}/../../../deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

XMOS_REGISTER_APP()
