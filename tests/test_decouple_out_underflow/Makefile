TARGET = xk-audio-316-mc.xn

USED_MODULES = lib_xua lib_logging lib_random

XCC_FLAGS = 	$(TEST_BUILD_FLAGS) \
				-O3 -g -lflash -DXUD_CORE_CLOCK=600 -fxscope -save-temps -DXUD_WEAK_API=1 \
				-DDEBUG_PRINT_ENABLE=0 \
				-DXASSERT_ENABLE_ASSERTIONS_MAIN=1

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
