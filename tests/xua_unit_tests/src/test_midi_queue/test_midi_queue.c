// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>

#include "xua_unit_tests.h"
#include "../../../lib_xua/src/midi/queue.h"


#define RANDOM_SEED                     55378008
#define USB_MIDI_DEVICE_OUT_FIFO_SIZE   1024
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

unsigned rndm = RANDOM_SEED;


void test_midi_queue(void) {
    queue_t symbol_fifo;
    unsigned symbol_fifo_storage[USB_MIDI_DEVICE_OUT_FIFO_SIZE];
    queue_init_wrap(&symbol_fifo, ARRAY_SIZE(symbol_fifo_storage));

    int empty = queue_is_empty_wrap(&symbol_fifo);
    TEST_ASSERT_EQUAL_UINT32(1, empty);
   
}