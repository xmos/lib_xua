// Copyright 2011-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _CLOCKING_H_
#define _CLOCKING_H_

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
void clockGen (streaming chanend ?c_spdif_rx, chanend ?c_adat_rx, out port p, chanend c_audio, chanend c_clk_ctl, chanend c_clk_int);

interface sync_if
{
    void toggle();
};

[[combinable]]
void PllRefPinTask(server interface sync_if i_sync, out port p_sync);
#endif

