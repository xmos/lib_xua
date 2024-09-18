// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.


// This file conatins the mic_array task

#include "xua_pdm_mic.h"
#include "mic_array.h"

#include <xcore/channel.h>


#define CLRSR(c)                asm volatile("clrsr %0" : : "n"(c));
#define CLEAR_KEDI()            CLRSR(XS1_SR_KEDI_MASK)

#include <print.h>

void mic_array_task(chanend_t c_mic_to_audio){

    ma_init();
    /*
     * ma_task() itself uses interrupts, and does re-enable them. However,
     * it appears to assume that KEDI is not set, therefore it is cleared here in
     * case this module is compiled with dual issue.
     */
    CLEAR_KEDI()

    /* Synchronise with consumer to ensure we start at same time and avoid ma bug */
    // chan_out_word(c_mic_to_audio, 22);

    /* Start endless loop */
    ma_task(c_mic_to_audio);
}
