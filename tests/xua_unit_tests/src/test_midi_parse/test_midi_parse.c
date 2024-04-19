// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>

#include "xua_unit_tests.h"
#include "../../../lib_xua/src/midi/midiinparse.h"

#define NUM_CHANS       16
#define NOTE_OFF        128
#define NOTE_ON         144
#define PRESSURE        160
#define CONTROL         176
#define PROGRAM         192
#define PRESSURE_VAL    208
#define RANGE           224
#define MANUFACTURE_ID  240

#define DATA_RANGE      128
#define DATA_MASK       (DATA_RANGE - 1)

#define NUM_TESTS_PER_TEST  30

#define CABLE_NUM       2
#define RANDOM_SEED     6031769

unsigned mini_in_parse_ut(unsigned midi[3]){
    // printf("Composing data: 0x%x 0x%x 0x%x\n", midi[0], midi[1], midi[2]);

    struct midi_in_parse_state m_state;
    void * mips = &m_state;
    reset_midi_state_c_wrapper(mips);

    unsigned valid = 0;
    unsigned packed = 0;

    midi_in_parse_c_wrapper(mips, CABLE_NUM, midi[0], &valid, &packed);
    // printf("Valid: %d data: %u\n", valid, packed);
    if(valid){
        return packed;
    }
    midi_in_parse_c_wrapper(mips, CABLE_NUM, midi[1], &valid, &packed);
    // printf("Valid: %d data: %u\n", valid, packed);
    if(valid){
        return packed;
    }
    midi_in_parse_c_wrapper(mips, CABLE_NUM, midi[2], &valid, &packed);
    // printf("Valid: %d data: %u\n", valid, packed);
    if(valid){
        return packed;
    }

    return 0;
}

unsigned rndm = RANDOM_SEED;


void test_midi_note(void) {
    for(int cmd = NOTE_OFF; cmd < NOTE_ON + NUM_CHANS; cmd++){
        for(int test = 0; test < NUM_TESTS_PER_TEST; test++){
            unsigned midi_ref[3] = {cmd, random(&rndm) & DATA_MASK, random(&rndm) & DATA_MASK};
            unsigned packed = mini_in_parse_ut(midi_ref);
            unsigned midi_dut[3] = {0};
            unsigned size = 0;
            midi_out_parse_c_wrapper(packed, midi_dut, &size);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_ref[0], midi_ref[1], midi_ref[2]);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_dut[0], midi_dut[1], midi_dut[2]);
            //TEST_ASSERT_EQUAL_UINT32_ARRAY not working!?
            for(int i = 0; i < size; i++){
                TEST_ASSERT_EQUAL_UINT32(midi_ref[i], midi_dut[i]);
            }
        }
    }
}

void test_midi_pressure(void) {
    for(int cmd = PRESSURE; cmd < PRESSURE + NUM_CHANS; cmd++){
        for(int test = 0; test < NUM_TESTS_PER_TEST; test++){
            unsigned midi_ref[3] = {cmd, random(&rndm) & DATA_MASK, random(&rndm) & DATA_MASK};
            unsigned packed = mini_in_parse_ut(midi_ref);
            unsigned midi_dut[3] = {0};
            unsigned size = 0;
            midi_out_parse_c_wrapper(packed, midi_dut, &size);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_ref[0], midi_ref[1], midi_ref[2]);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_dut[0], midi_dut[1], midi_dut[2]);
            //TEST_ASSERT_EQUAL_UINT32_ARRAY not working!?
            for(int i = 0; i < size; i++){
                TEST_ASSERT_EQUAL_UINT32(midi_ref[i], midi_dut[i]);
            }
        }
    }
}

void test_midi_control(void) {
    for(int cmd = CONTROL; cmd < CONTROL + NUM_CHANS; cmd++){
        for(int test = 0; test < NUM_TESTS_PER_TEST; test++){
            unsigned midi_ref[3] = {cmd, random(&rndm) & DATA_MASK, random(&rndm) & DATA_MASK};
            unsigned packed = mini_in_parse_ut(midi_ref);
            unsigned midi_dut[3] = {0};
            unsigned size = 0;
            midi_out_parse_c_wrapper(packed, midi_dut, &size);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_ref[0], midi_ref[1], midi_ref[2]);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_dut[0], midi_dut[1], midi_dut[2]);
            //TEST_ASSERT_EQUAL_UINT32_ARRAY not working!?
            for(int i = 0; i < size; i++){
                TEST_ASSERT_EQUAL_UINT32(midi_ref[i], midi_dut[i]);
            }
        }
    }
}

void test_midi_program(void) {
    for(int cmd = PROGRAM; cmd < PROGRAM + NUM_CHANS; cmd++){
        for(int test = 0; test < NUM_TESTS_PER_TEST; test++){
            unsigned midi_ref[3] = {cmd, random(&rndm) & DATA_MASK, random(&rndm) & DATA_MASK};
            unsigned packed = mini_in_parse_ut(midi_ref);
            unsigned midi_dut[3] = {0};
            unsigned size = 0;
            midi_out_parse_c_wrapper(packed, midi_dut, &size);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_ref[0], midi_ref[1], midi_ref[2]);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_dut[0], midi_dut[1], midi_dut[2]);
            //TEST_ASSERT_EQUAL_UINT32_ARRAY not working!?
            for(int i = 0; i < size; i++){
                TEST_ASSERT_EQUAL_UINT32(midi_ref[i], midi_dut[i]);
            }
        }
    }
}

void test_midi_pressure_val(void) {
    for(int cmd = PRESSURE_VAL; cmd < PRESSURE_VAL + NUM_CHANS; cmd++){
        for(int test = 0; test < NUM_TESTS_PER_TEST; test++){
            unsigned midi_ref[3] = {cmd, random(&rndm) & DATA_MASK, random(&rndm) & DATA_MASK};
            unsigned packed = mini_in_parse_ut(midi_ref);
            unsigned midi_dut[3] = {0};
            unsigned size = 0;
            midi_out_parse_c_wrapper(packed, midi_dut, &size);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_ref[0], midi_ref[1], midi_ref[2]);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_dut[0], midi_dut[1], midi_dut[2]);
            //TEST_ASSERT_EQUAL_UINT32_ARRAY not working!?
            for(int i = 0; i < size; i++){
                TEST_ASSERT_EQUAL_UINT32(midi_ref[i], midi_dut[i]);
            }
        }
    }
}

void test_midi_range(void) {
    for(int cmd = RANGE; cmd < RANGE + NUM_CHANS; cmd++){
        for(int test = 0; test < NUM_TESTS_PER_TEST; test++){
            unsigned midi_ref[3] = {cmd, random(&rndm) & DATA_MASK, random(&rndm) & DATA_MASK};
            unsigned packed = mini_in_parse_ut(midi_ref);
            unsigned midi_dut[3] = {0};
            unsigned size = 0;
            midi_out_parse_c_wrapper(packed, midi_dut, &size);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_ref[0], midi_ref[1], midi_ref[2]);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_dut[0], midi_dut[1], midi_dut[2]);
            //TEST_ASSERT_EQUAL_UINT32_ARRAY not working!?
            for(int i = 0; i < size; i++){
                TEST_ASSERT_EQUAL_UINT32(midi_ref[i], midi_dut[i]);
            }
        }
    }
}

void test_midi_manufacturer_id(void) {
    for(int cmd = MANUFACTURE_ID; cmd < MANUFACTURE_ID + NUM_CHANS; cmd++){
        for(int test = 0; test < NUM_TESTS_PER_TEST; test++){
            unsigned midi_ref[3] = {cmd, random(&rndm) & DATA_MASK, random(&rndm) & DATA_MASK};
            unsigned packed = mini_in_parse_ut(midi_ref);
            unsigned midi_dut[3] = {0};
            unsigned size = 0;
            midi_out_parse_c_wrapper(packed, midi_dut, &size);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_ref[0], midi_ref[1], midi_ref[2]);
            // printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi_dut[0], midi_dut[1], midi_dut[2]);
            //TEST_ASSERT_EQUAL_UINT32_ARRAY not working!?
            for(int i = 0; i < size; i++){
                TEST_ASSERT_EQUAL_UINT32(midi_ref[i], midi_dut[i]);
            }
        }
    }
}