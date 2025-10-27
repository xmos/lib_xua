// Copyright 2022-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua_conf.h"
#if (XUA_NUM_PDM_MICS > 0)


#include "xua_pdm_mic.h"
#include "mic_array.h"

#include <xcore/channel.h>

void mic_array_task(chanend_t c_mic_to_audio){
    while(1) {
        /* This channel transaction serves to synchronise the start of
        * audiohub with mic_array so we always consume samples */
        unsigned mic_samp_rate = chan_in_word(c_mic_to_audio);
        unsigned mClk = MCLK_48, pdmClk = 3072000;

    #if (XUA_PDM_MIC_USE_DDR)
        pdm_rx_resources_t pdm_res = PDM_RX_RESOURCES_DDR(
            PORT_MCLK_IN,
            PORT_PDM_CLK,
            PORT_PDM_DATA,
            mClk,
            pdmClk,
            XS1_CLKBLK_1,
            XS1_CLKBLK_2);
    #else
        pdm_rx_resources_t pdm_res = PDM_RX_RESOURCES_SDR(
            PORT_MCLK_IN,
            PORT_PDM_CLK,
            PORT_PDM_DATA,
            mClk,
            pdmClk,
            XS1_CLKBLK_1);
    #endif
        mic_array_init(&pdm_res, NULL, mic_samp_rate);

        /* Start endless loop */
        mic_array_start(c_mic_to_audio);
    }
}

#endif // #if (XUA_NUM_PDM_MICS > 0)
