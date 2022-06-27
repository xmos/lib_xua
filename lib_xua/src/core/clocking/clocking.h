// Copyright 2011-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _CLOCKING_H_
#define _CLOCKING_H_



interface sync_if
{
    void toggle();
    void init();
    void toggle_timed(int relative);
};

[[combinable]]
void PllRefPinTask(server interface sync_if i_sync, out port p_sync);

/** Clock generation and digital audio I/O handling.
 *
 *  \param c_spdif_rx channel connected to S/PDIF receive thread
 *  \param c_adat_rx channel connect to ADAT receive thread
 *  \param p port to output clock signal to drive external frequency synthesizer
 *  \param c_audio channel connected to the audio() thread
 *  \param c_clk_ctl channel connected to Endpoint0() for configuration of the
 *                   clock
 *  \param c_clk_int channel connected to the decouple() thread for clock
                     interrupts
 */
#if (AUDIO_IO_TILE == PLL_REF_TILE)
void clockGen(streaming chanend ?c_spdif_rx, chanend ?c_adat_rx, client interface out port p_pll_ref, chanend c_audio, chanend c_clk_ctl, chanend c_clk_int);
#else
void clockGen(streaming chanend ?c_spdif_rx, chanend ?c_adat_rx, client interface sync_if i_sync, chanend c_audio, chanend c_clk_ctl, chanend c_clk_int);
#endif
#endif

