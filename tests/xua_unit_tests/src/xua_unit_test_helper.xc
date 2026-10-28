// Copyright 2021-2026 XMOS LIMITED.
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

/////////////////////// Harness for sw_pll_rate_window_update()

#include "sw_pll_wrapper.h"

#if XUA_USE_SW_PLL
/* Never a valid error word: errors sent to the SW PLL are within (-32768, 32768) */
#define RATE_WINDOW_HARNESS_DONE (0x80000000)

/* Runs the real sw_pll_rate_window_update() with its channel connected to a receiver standing in
 * for sw_pll_task(). Returns its status; *sent is 1 and *word holds the word received if it sent
 * one. A marker word after the call tells the receiver whether anything was sent. */
int sw_pll_rate_window_update_c_wrapper(xua_rate_window_t * unsafe window,
                                        unsigned short mclk_time_stamp,
                                        unsigned reference_timestamp,
                                        unsigned mclks_per_count,
                                        unsigned * unsafe sent,
                                        unsigned * unsafe word)
{
    chan c;
    int status = 0;
    unsigned received = 0;
    unsigned value = 0;
    unsafe {
        par {
            {
                status = sw_pll_rate_window_update(*window, mclk_time_stamp, reference_timestamp,
                                                   mclks_per_count, c);
                outuint(c, RATE_WINDOW_HARNESS_DONE);
                outct(c, XS1_CT_END);
            }
            {
                unsigned x = inuint(c);
                inct(c);
                if (x != RATE_WINDOW_HARNESS_DONE)
                {
                    received = 1;
                    value = x;
                    (void)inuint(c);
                    inct(c);
                }
            }
        }
        *sent = received;
        *word = value;
    }
    return status;
}
#endif

