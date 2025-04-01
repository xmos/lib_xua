cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(dfu_test)

set(XMOS_SANDBOX_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../..)

set(APP_HW_TARGET xk-audio-316-mc.xn)
set(APP_DEPENDENT_MODULES "lib_xua"
                          "lib_board_support(1.1.0)")
set(APP_INCLUDES src)

set(COMMON_FLAGS -O3
                 -lquadflash
                 -DXUA_QUAD_SPI_FLASH=1
                 -DBOARD_SUPPORT_BOARD=XK_AUDIO_316_MC_AB)

# Define app-under-test configs here

set(APP_COMPILER_FLAGS_i8o8 ${COMMON_FLAGS}
                            -DBCD_DEVICE=0x0100
                            -DNUM_USB_CHAN_IN=8
                            -DNUM_USB_CHAN_OUT=8
                            -DI2S_CHANS_ADC=8
                            -DI2S_CHANS_DAC=8)

set(APP_COMPILER_FLAGS_i2s_only ${COMMON_FLAGS}
                                -DBCD_DEVICE=0x0101
                                -DNUM_USB_CHAN_IN=0
                                -DNUM_USB_CHAN_OUT=0
                                -DI2S_CHANS_ADC=8
                                -DI2S_CHANS_DAC=8
                                -DCODEC_MASTER=1)

set(APP_COMPILER_FLAGS_hid ${COMMON_FLAGS}
                           -DBCD_DEVICE=0x0102
                           -DNUM_USB_CHAN_IN=8
                           -DNUM_USB_CHAN_OUT=8
                           -DI2S_CHANS_ADC=8
                           -DI2S_CHANS_DAC=8
                           -DHID_CONTROLS=1)

set(APP_COMPILER_FLAGS_control ${COMMON_FLAGS}
                           -DBCD_DEVICE=0x0102
                           -DNUM_USB_CHAN_IN=8
                           -DNUM_USB_CHAN_OUT=8
                           -DI2S_CHANS_ADC=8
                           -DI2S_CHANS_DAC=8
                           -DUSB_CONTROL_DESCS=1)


set(APP_COMPILER_FLAGS_control_not_winusb ${COMMON_FLAGS}
                           -DBCD_DEVICE=0x0102
                           -DNUM_USB_CHAN_IN=8
                           -DNUM_USB_CHAN_OUT=8
                           -DI2S_CHANS_ADC=8
                           -DI2S_CHANS_DAC=8
                           -DUSB_CONTROL_DESCS=1
                           -DENUMERATE_CONTROL_INTF_AS_WINUSB=0)

# baseline configs for the testcase

set(BASE_TEST_FLAGS -DNUM_USB_CHAN_IN=2
                    -DNUM_USB_CHAN_OUT=2
                    -DI2S_CHANS_ADC=2
                    -DI2S_CHANS_DAC=2)

set(APP_COMPILER_FLAGS_factory ${COMMON_FLAGS}
                               ${BASE_TEST_FLAGS}
                               -DBCD_DEVICE=0x0001)

set(APP_COMPILER_FLAGS_upgrade ${COMMON_FLAGS}
                               ${BASE_TEST_FLAGS}
                               -DBCD_DEVICE=0x9999)

XMOS_REGISTER_APP()
