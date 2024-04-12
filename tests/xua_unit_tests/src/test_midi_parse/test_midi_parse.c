// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stddef.h>
#include <stdio.h>

#include "xua_unit_tests.h"
#include "../../../lib_xua/src/midi/midiinparse.h"


#define CABLE_NUM   0

#define NOTE_ON     0x90
#define PITCH       60
#define VELOCITY    80

void something(void){

    struct midi_in_parse_state m_state;
    void * mips = &m_state;
    reset_midi_state_wrap(mips);

    unsigned valid = 0;
    unsigned packed = 0;
    midi_in_parse_wrap(mips, CABLE_NUM, NOTE_ON, &valid, &packed);
    printf("Valid: %d data: %u\n", valid, packed);
    midi_in_parse_wrap(mips, CABLE_NUM, PITCH, &valid, &packed);
    printf("Valid: %d data: %u\n", valid, packed);
    midi_in_parse_wrap(mips, CABLE_NUM, VELOCITY, &valid, &packed);
    printf("Valid: %d data: %u\n", valid, packed);


    unsigned midi[3];
    unsigned size = 0;
    
    midi_out_parse_wrap(packed, midi, &size);
    printf("size: %d data: 0x%x 0x%x 0x%x\n", size, midi[0], midi[1], midi[2]);
}

void test_midi_tx(void) {
    TEST_ASSERT_EQUAL_UINT(3, 3);
    something();
}
