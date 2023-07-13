
TEST_FLAGS ?=

XCC_FLAGS_HS = -O3 -g -DXUD_CORE_CLOCK=600 -save-temps -DUSB_TILE=tile[0] -DLOCAL_CLOCK_INCREMENT=10000 -DLOCAL_CLOCK_MARGIN=100 \
							-DBUS_SPEED=2 \
							-DXUA_USE_APP_PLL=0 \
							$(TEST_FLAGS)

XCC_FLAGS_FS = -O3 -g -DXUD_CORE_CLOCK=600 -save-temps -DUSB_TILE=tile[0] -DLOCAL_CLOCK_INCREMENT=10000 -DLOCAL_CLOCK_MARGIN=100 \
							-DBUS_SPEED=1 \
							-DXUA_USE_APP_PLL=0 \
							$(TEST_FLAGS)

TARGET = test_xs3_600.xn

USED_MODULES = lib_xua

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
