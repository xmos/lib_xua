cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_xua_simple_lib_i2s)

set(APP_HW_TARGET xk-audio-316-mc.xn)

set(APP_INCLUDES src)

set(APP_DEPENDENT_MODULES "lib_xua(develop)"
                          "lib_i2s(6.0.1)"
                          "lib_board_support(develop)")

set(COMPILER_FLAGS_COMMON       -O3
                                -report
                                -g
                                -Wno-unused-function
                                -DXUD_CORE_CLOCK=600
                                -DUSB_TILE=tile[0]
                                -DXUD_TILE=0
                                -DAUDIO_IO_TILE=1
                                -fxscope
                                -DUAC_FORCE_FEEDBACK_EP=0
                                -DDEBUG_PRINT_ENABLE=1
                                -DBOARD_SUPPORT_BOARD=XK_AUDIO_316_MC_AB)

set(APP_COMPILER_FLAGS_master   ${COMPILER_FLAGS_COMMON}
                                -DI2S_MASTER=1)

set(APP_COMPILER_FLAGS_slave    ${COMPILER_FLAGS_COMMON}
                                -DI2S_SLAVE=1)


set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

XMOS_REGISTER_APP()
