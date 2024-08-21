
DEBUG ?= 0

ifeq ($(DEBUG),1)
TEST_DEBUG_FLAGS = -g -DDEBUG_PRINT_ENABLE=1
else
TEST_DEBUG_FLAGS =
endif

TEST_FLAGS = -DTEST_SEED=$(TEST_SEED) $(TEST_DEBUG_FLAGS) -DXUD_WEAK_API=1

XCC_FLAGS = -O3 $(TEST_FLAGS)

TARGET = test_xs3_600.xn

USED_MODULES = lib_xua lib_logging lib_random

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
