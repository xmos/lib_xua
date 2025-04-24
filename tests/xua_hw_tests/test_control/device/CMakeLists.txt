cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(control_test)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../..)

set(APP_HW_TARGET xk-audio-316-mc.xn)
set(APP_DEPENDENT_MODULES "lib_xua"
                          "lib_board_support(1.1.0)")

set(COMMON_FLAGS -O3
                 -lquadflash
                 -DXUA_QUAD_SPI_FLASH=1
                 -DBOARD_SUPPORT_BOARD=XK_AUDIO_316_MC_AB)

# audio + control + dfu
set(APP_COMPILER_FLAGS_audio_control_dfu ${COMMON_FLAGS}
                           -DBCD_DEVICE=0x0102
                           -DNUM_USB_CHAN_IN=8
                           -DNUM_USB_CHAN_OUT=8
                           -DI2S_CHANS_ADC=8
                           -DI2S_CHANS_DAC=8
                           -DXUA_USB_CONTROL_DESCS=1
                           -DXUA_DFU_EN=1)
# audio + control
set(APP_COMPILER_FLAGS_audio_control ${COMMON_FLAGS}
                           -DBCD_DEVICE=0x0102
                           -DNUM_USB_CHAN_IN=8
                           -DNUM_USB_CHAN_OUT=8
                           -DI2S_CHANS_ADC=8
                           -DI2S_CHANS_DAC=8
                           -DXUA_USB_CONTROL_DESCS=1
                           -DXUA_DFU_EN=0)

# Enumerate with only a single interface for Vendor specific control
set(APP_COMPILER_FLAGS_control_only ${COMMON_FLAGS}
                           -DBCD_DEVICE=0x0103
                           -DNUM_USB_CHAN_IN=0
                           -DNUM_USB_CHAN_OUT=0
                           -DI2S_CHANS_ADC=8
                           -DI2S_CHANS_DAC=8
                           -DXUA_USB_CONTROL_DESCS=1
                           -DXUA_DFU_EN=0)

set(APP_INCLUDES src ../../test_dfu/src)
# Same sources as test_dfu
file(GLOB COMMON_XC_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../../test_dfu/src/*.xc)
file(GLOB COMMON_C_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../../test_dfu/src/*.c)

set(APP_XC_SRCS ${COMMON_XC_SOURCES})
set(APP_C_SRCS ${COMMON_C_SOURCES})

XMOS_REGISTER_APP()
