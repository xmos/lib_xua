
TEST_FLAGS ?= -DTEST_SEED=$(TEST_SEED)

XCC_FLAGS = -O3 -g -DDEBUG_PRINT_ENABLE_main=0 $(TEST_FLAGS)

TARGET = test_xs3_600.xn

USED_MODULES = lib_xua lib_logging lib_random

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
