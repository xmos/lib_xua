# Copyright (c) 2018, XMOS Ltd, All rights reserved

#Note no xcommon included - we are going bareback here because xcommon not good
#at dealing excluding/including source directories out of normal structure.
#We want to have a test app here using some source files from sw_vocalfusion (delay estimator)


BUILD_FLAGS = -O3 -g 
XCC_FLAGS = $(BUILD_FLAGS)

INCLUDE_DIRS = \
	-I ../src/ \
	-I . \

COMMON = \
  -O2 -g -report \
  -target=XCORE-200-EXPLORER \

SOURCES = \
	./test_fifo.xc \

BINARY=bin/test_fifo.xe \


all: 
	mkdir -p bin; \
	mv test_fifoxc test_fifo.xc; \
	xcc $(SOURCES) $(COMMON) $(INCLUDE_DIRS) -D AUDIO_DELAY_SAMPLES=$$DELAY -o $(BINARY); \
	mv test_fifo.xc test_fifoxc; \

sim:
	xsim $(BINARY)

clean:
	rm -rf bin/*
