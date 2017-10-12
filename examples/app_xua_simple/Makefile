APP_NAME = app_xua_simple

TARGET = xk-audio-216-mc.xn

# The flags passed to xcc when building the application
XCC_FLAGS     = -fcomment-asm -Xmapper --map -Xmapper MAPFILE -O3 -report -fsubword-select -save-temps -g -Wno-unused-function -Wno-timing -DXUD_SERIES_SUPPORT=XUD_X200_SERIES -DUSB_TILE=tile[1] 

# The USED_MODULES variable lists other module used by the application. These
# modules will extend the SOURCE_DIRS, INCLUDE_DIRS and LIB_DIRS variables.
# Modules are expected to be in the directory above the BASE_DIR directory.
USED_MODULES = lib_i2c lib_mic_array lib_logging lib_xua lib_device_control lib_src lib_dsp lib_spi lib_xud

#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common

