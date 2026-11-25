cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_adapt_clk_basic)

# XUD itself is not run, so the core clock is lowered to 200 MHz to shorten the simulation
set(APP_HW_TARGET test_xs3_200.xn)

set(APP_INCLUDES src)

# XUD_CORE_CLOCK only satisfies the lib_xud build check
set(COMMON_FLAGS -O3 -g -DXUD_CORE_CLOCK=600)

set(APP_COMPILER_FLAGS_HS ${COMMON_FLAGS} -DBUS_SPEED=2)

set(APP_COMPILER_FLAGS_FS ${COMMON_FLAGS} -DBUS_SPEED=1)

include(${CMAKE_CURRENT_LIST_DIR}/../../../examples/deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

XMOS_REGISTER_APP()
