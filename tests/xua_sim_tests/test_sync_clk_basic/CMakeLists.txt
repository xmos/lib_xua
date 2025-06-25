cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(test_sync_clk_basic)

set(APP_HW_TARGET test_xs3_600.xn)

set(APP_INCLUDES src)

set(COMMON_FLAGS -O3 -g -DXUD_CORE_CLOCK=600
                 -DLOCAL_CLOCK_INCREMENT=10000
                 -DLOCAL_CLOCK_MARGIN=100
                 -DXUA_USE_SW_PLL=0)

set(APP_COMPILER_FLAGS_HS ${COMMON_FLAGS} -DBUS_SPEED=2)

set(APP_COMPILER_FLAGS_FS ${COMMON_FLAGS} -DBUS_SPEED=1)

include(${CMAKE_CURRENT_LIST_DIR}/../../../examples/deps.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../test_deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

XMOS_REGISTER_APP()
