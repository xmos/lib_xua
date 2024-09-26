cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_xua_pdm_mics)

set(APP_HW_TARGET XCORE-AI-EXPLORER)

set(APP_INCLUDES src)

set(APP_COMPILER_FLAGS -O3
                       -report
                       -g
                       -Wno-unused-function
                       -DXUD_CORE_CLOCK=600
                       -DUSB_TILE=tile[0]
                       -mcmodel=large
                       -DBOARD_SUPPORT_BOARD=XK_EVK_XU316)

include(${CMAKE_CURRENT_LIST_DIR}/../deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

XMOS_REGISTER_APP()
