APP_NAME = 

TARGET = RPI_HAT_60QFN.xn

# The flags passed to xcc when building the application
XCC_FLAGS	= -fcomment-asm -Xmapper --map -Xmapper MAPFILE -Os -report \
			  -g -Wno-unused-function -Wno-timing -DXUD_SERIES_SUPPORT=XUD_X200_SERIES -DUSB_TILE=tile[1] \
			  -D MIC_ARRAY_CH0=PIN0 -D MIC_ARRAY_CH1=PIN4
			  
# The USED_MODULES variable lists other module used by the application. These
# modules will extend the SOURCE_DIRS, INCLUDE_DIRS and LIB_DIRS variables.
# Modules are expected to be in the directory above the BASE_DIR directory.
USED_MODULES = lib_xua lib_i2s lib_xud lib_i2c lib_mic_array_ddr

#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common

