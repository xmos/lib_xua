# This file provides overrides to the Doxyfile configuration

PROJECT_NAME           = lib_xua
PROJECT_BRIEF          = "XUA library"

INPUT                  = ../lib_xua/api ../lib_xua/src/core/user/audiostream ../lib_xua/src/core/user/hostactive ../lib_xua/src/core/user/hid ../lib_xua/src/core/user/audiohw

PREDEFINED             = C_API= EXTERN_C= UNSAFE= __DOXYGEN__=1 XUA_USB_EN=1
