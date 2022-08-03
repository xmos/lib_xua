
TEST_FLAGS ?=

XCC_FLAGS = -O3 -g -DXUD_CORE_CLOCK=600 -save-temps -DUSB_TILE=tile[0] -DLOCAL_CLOCK_INCREMENT=10000 -DLOCAL_CLOCK_MARGIN=100 $(TEST_FLAGS)

TARGET = test_xs3_600.xn

USED_MODULES = lib_xua

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
