// Copyright 2011-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _CLOCKING_H_
#define _CLOCKING_H_

#include <xs1.h>

interface pll_ref_if
{
    void toggle();
    void init();
    void toggle_timed(int relative);
};

[[distributable]]
void PllRefPinTask(server interface pll_ref_if i_pll_ref, out port p_sync);

/** Clock generation and digital audio I/O handling.
 *
 *  \param c_spdif_rx channel connected to S/PDIF receive thread
 *  \param c_adat_rx channel connect to ADAT receive thread
 *  \param i_pll_ref interface to taslk that outputs clock signal to drive external frequency synthesizer
 *  \param c_audio channel connected to the audio() thread
 *  \param c_clk_ctl channel connected to Endpoint0() for configuration of the
 *                   clock
 *  \param c_clk_int channel connected to the decouple() thread for clock
 *                  interrupts
 *  \param p_for_mclk_count_aud port used for counting mclk and providing a timestamp
 * 
 *  \param c_mclk_change channel to notify of master clock change
 */
void clockGen(  streaming chanend ?c_spdif_rx,
                chanend ?c_adat_rx,
                client interface pll_ref_if i_pll_ref,
                chanend c_audio,
                chanend c_clk_ctl,
                chanend c_clk_int,
                port ?p_for_mclk_count_aud,
                chanend c_mclk_change);

#if (XUA_USE_APP_PLL)

interface SoftPll_if
{
    void init(int mclk_hz);
};

#if (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
[[distributable]]
#endif
void XUA_SoftPll(tileref tile, server interface SoftPll_if i_softPll, chanend c_update);

#endif
#endif

