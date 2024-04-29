// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "xua_unit_tests.h"
#include "../../../lib_xua/src/midi/queue.h"

#define DEBUG       0

#if     DEBUG
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif


#define RANDOM_SEED                     55378008
#define USB_MIDI_DEVICE_OUT_FIFO_SIZE   1024
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

unsigned rndm = RANDOM_SEED;


void test_midi_queue_init(void) {
    queue_t symbol_fifo;
    unsigned symbol_fifo_storage[USB_MIDI_DEVICE_OUT_FIFO_SIZE];
    memset(symbol_fifo_storage, USB_MIDI_DEVICE_OUT_FIFO_SIZE * sizeof(unsigned), 0xdb); // Non zero

    queue_init_c_wrapper(&symbol_fifo, ARRAY_SIZE(symbol_fifo_storage));

    int empty = queue_is_empty_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_INT32(1, empty);
   
    int full = queue_is_full_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_INT32(0, full);

    unsigned items = queue_items_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_UINT32(0, items);

    unsigned space = queue_space_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_UINT32(USB_MIDI_DEVICE_OUT_FIFO_SIZE, space);

    // Pop empty queue
    unsigned entry = queue_pop_word_c_wrapper(&symbol_fifo, symbol_fifo_storage);
    TEST_ASSERT_EQUAL_UINT32(MIDI_OUT_NULL_MESSAGE, entry);

    space = queue_space_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_UINT32(USB_MIDI_DEVICE_OUT_FIFO_SIZE, space);
}

void test_midi_queue_full(void) {
    queue_t symbol_fifo;
    unsigned symbol_fifo_storage[USB_MIDI_DEVICE_OUT_FIFO_SIZE];
    queue_init_c_wrapper(&symbol_fifo, ARRAY_SIZE(symbol_fifo_storage));

    for(unsigned i = 0; i < USB_MIDI_DEVICE_OUT_FIFO_SIZE; i++){
        queue_push_word_c_wrapper(&symbol_fifo, symbol_fifo_storage, 1111);
    }

    int empty = queue_is_empty_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_INT32(0, empty);
   
    int full = queue_is_full_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_INT32(1, full);

    unsigned items = queue_items_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_UINT32(USB_MIDI_DEVICE_OUT_FIFO_SIZE, items);

    unsigned space = queue_space_c_wrapper(&symbol_fifo);
    TEST_ASSERT_EQUAL_UINT32(0, space);

    // We want no exception here and this to be ignored
    queue_push_word_c_wrapper(&symbol_fifo, symbol_fifo_storage, 2222);

    unsigned entry = queue_pop_word_c_wrapper(&symbol_fifo, symbol_fifo_storage);
    TEST_ASSERT_EQUAL_UINT32(1111, entry);
}

void test_midi_queue_push_pop(void) {
    queue_t symbol_fifo;
    unsigned symbol_fifo_storage[USB_MIDI_DEVICE_OUT_FIFO_SIZE];
    queue_init_c_wrapper(&symbol_fifo, ARRAY_SIZE(symbol_fifo_storage));

    for(unsigned i = 0; i < USB_MIDI_DEVICE_OUT_FIFO_SIZE; i++){
        int items = queue_items_c_wrapper(&symbol_fifo);
        dprintf("Pre i: %u items: %d\n", i, items);
        TEST_ASSERT_EQUAL_UINT32(i, items);

        unsigned entry = i + 1000;
        queue_push_word_c_wrapper(&symbol_fifo, symbol_fifo_storage, entry);
        dprintf("pushed: %u\n", entry);

        items = queue_items_c_wrapper(&symbol_fifo);
        TEST_ASSERT_EQUAL_UINT32(i + 1, items);

        dprintf("Post items: %d\n", items);
    }

    unsigned counter = 0;
    for(int i = USB_MIDI_DEVICE_OUT_FIFO_SIZE; i > 0; i--){
        int items = queue_items_c_wrapper(&symbol_fifo);
        dprintf("i: %u items: %d\n", i, items);
        TEST_ASSERT_EQUAL_UINT32(i, items);

        unsigned entry = queue_pop_word_c_wrapper(&symbol_fifo, symbol_fifo_storage);
        unsigned expected = 1000 + counter;

        dprintf("expected: %u got: %d\n", expected, entry);
        TEST_ASSERT_EQUAL_UINT32(expected, entry);

        counter++;
    }
}