cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_xua)

set(APP_HW_TARGET xk-audio-316-mc.xn)

set(APP_INCLUDES src)

set(APP_COMPILER_FLAGS -O3
                       -report
                       -lquadflash
                       -g
                       -Wno-unused-function
                       -DXUD_CORE_CLOCK=600
                       -fxscope
                       -DUAC_FORCE_FEEDBACK_EP=0
                       -DBOARD_SUPPORT_BOARD=XK_AUDIO_316_MC_AB)

include(${CMAKE_CURRENT_LIST_DIR}/../deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

XMOS_REGISTER_APP()
