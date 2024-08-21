cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_xua_pdm_mics)

set(APP_HW_TARGET mic_array_ref.xn)

set(APP_INCLUDES src)

set(APP_COMPILER_FLAGS -fcomment-asm
                       -O3
                       -report
                       -g
                       -Wno-unused-function
                       -Wno-timing
                       -DXUD_SERIES_SUPPORT=XUD_X200_SERIES
                       -DXUD_CORE_CLOCK=600
                       -DUSB_TILE=tile[1]
                       -fxscope
                       -DUAC_FORCE_FEEDBACK_EP=0)

include(${CMAKE_CURRENT_LIST_DIR}/../../deps.cmake)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

XMOS_REGISTER_APP()
