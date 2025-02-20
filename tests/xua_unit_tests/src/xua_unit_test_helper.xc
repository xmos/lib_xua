// Copyright 2021-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifdef __XC__

#include <xs1.h>
#include <platform.h>
#include <xclib.h>
#include "../../../lib_xua/src/midi/midiinparse.h"
#include "../../../lib_xua/src/midi/midioutparse.h"
#include "../../../lib_xua/src/midi/queue.h"

#endif // __XC__


in port p_mclk_in                   = XS1_PORT_1D;

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_1;   /* Bit clock */
clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_2;   /* Master clock */

// Supply missing but unused function
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    ; // nothing
}

// Supply missing but unused function
void AudioHwInit()
{
    ; // nothing
}

unsigned random(unsigned &x){
    crc32(x, -1, 0xEB31D82E);
    return x;
}

////////////////////// Wrappers for midi parse because C doesn't support return tuples
void midi_in_parse_c_wrapper(void * unsafe mips, unsigned cable_number, unsigned char b, unsigned * unsafe valid, unsigned *unsafe packed){
    unsafe{
        struct midi_in_parse_state * unsafe ptr = mips;
        {*valid, *packed} = midi_in_parse(*ptr, cable_number, b);
    }
}

void midi_out_parse_c_wrapper(unsigned tx_data, unsigned midi[3], unsigned * unsafe size){
    unsafe{
        {midi[0], midi[1], midi[2], *size} = midi_out_parse(tx_data);
    }
}

void reset_midi_state_c_wrapper(void * unsafe mips){
    unsafe{
        struct midi_in_parse_state * unsafe ptr = mips;
        reset_midi_state(*ptr);
    }
}

/////////////////////// Wrappers for queue test


void queue_init_c_wrapper(queue_t *q, unsigned size) {
    unsafe{
        queue_init(*q, size);
    }
}

int queue_is_empty_c_wrapper(queue_t *unsafe q) {
    unsafe{
        return queue_is_empty(*q);
    }
}

int queue_is_full_c_wrapper(queue_t *unsafe q) {
    unsafe{
        return queue_is_full(*q);
    }
}

void queue_push_word_c_wrapper(queue_t *q, unsigned array[], unsigned data){
    unsafe{
        queue_push_word(*q, array, data);
    }
}

unsigned queue_pop_word_c_wrapper(queue_t *q, unsigned array[]){
    unsafe{
        return queue_pop_word(*q, array);
    }
}

unsigned queue_items_c_wrapper(const queue_t *q){
    unsafe{
        return queue_items(*q);
    }
}

unsigned queue_space_c_wrapper(const queue_t *q){
    unsafe{
        return queue_space(*q);
    }
}

