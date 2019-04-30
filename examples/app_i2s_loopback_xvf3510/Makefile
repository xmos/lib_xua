APP_NAME = app_xua_i2s_loopback

TARGET = XK_XVF3510_L71.xn

# The flags passed to xcc when building the application
XCC_FLAGS = -Os -report -save-temps -g -Wno-unused-function -DXUD_SERIES_SUPPORT=XUD_X200_SERIES -DUSB_TILE=tile[1]

# The USED_MODULES variable lists other module used by the application. These
# modules will extend the SOURCE_DIRS, INCLUDE_DIRS and LIB_DIRS variables.
# Modules are expected to be in the directory above the BASE_DIR directory.
USED_MODULES = lib_xua lib_xud lib_i2c


#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common

