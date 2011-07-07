
#ifndef _CLOCKING_H_
#define _CLOCKING_H_

/* Functions that handle master clock generation.  These need modifying for an existing design */

/* Any initialisation required for master clock generation - run once at start up */
void ClockingInit(void);

/* Configuration for a specific master clock frequency - run every sample frequency change */
void ClockingConfig(unsigned mClkFreq);


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
void clockGen (streaming chanend c_spdif_rx, chanend c_adat_rx, out port p, chanend c_audio, chanend c_clk_ctl, chanend c_clk_int);
#endif

