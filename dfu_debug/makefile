# Copyright (c) 2019, XMOS Ltd, All rights reserved
XUA = ../lib_xua/src/dfu

all:
	xcc vfst_base_usb.xn \
-Os -lquadflash \
-D XUD_SERIES_SUPPORT=4 -D DFU_DEBUG=1 -D START_IN_DFU=1 \
-I . -I ${XUA} \
${XUA}/dfu.xc ${XUA}/flash_interface.c ${XUA}/flashlib_user.c \
config.xscope \
test.xc xud.xc requests.c
	xobjdump -d a.xe > objdump
