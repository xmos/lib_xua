// Copyright 2022-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua_conf.h"
#if (XUA_NUM_PDM_MICS > 0)


#include "xua_pdm_mic.h"
#include "mic_array.h"

#include <xcore/chanend.h>


#define CLRSR(c)                asm volatile("clrsr %0" : : "n"(c));
#define CLEAR_KEDI()            CLRSR(XS1_SR_KEDI_MASK)

#include <print.h>

void mic_array_task(chanend_t c_mic_to_audio){
    unsigned mic_samp_rate = chanend_in_word(c_mic_to_audio);
    ma_init(mic_samp_rate);
    /*
     * ma_task() itself uses interrupts, and does re-enable them. However,
     * it appears to assume that KEDI is not set, therefore it is cleared here in
     * case this module is compiled with dual issue.
     */
    CLEAR_KEDI()

    /* Start endless loop */
    ma_task(c_mic_to_audio);
}

void ma_frame_rx_custom(
    int32_t frame[],
    const chanend_t c_frame_in,
    const unsigned channel_count,
    const unsigned sample_count)
{
    for(int i = 0; i < channel_count * sample_count; i++){
        frame[i] = chanend_in_word(c_frame_in);
    }
}

#endif // #if (XUA_NUM_PDM_MICS > 0)
