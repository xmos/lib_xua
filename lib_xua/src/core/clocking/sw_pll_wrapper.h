// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _SW_PLL_WRAPPPER_H_
#define _SW_PLL_WRAPPPER_H_

#include "xua.h"

#if XUA_USE_SW_PLL
extern "C"
{
    #include "sw_pll.h"
}

/* Special control value to disable SDM. Outside of normal range which is less than 16b.*/
#define DISABLE_SDM     0x10000000


/** Task that receives an error term, passes it through a PI controller and periodically
 *  calclulates a sigma delta output value and sends it to the PLL fractional register.
 *
 *  \param c_sw_pll                 Channel connected to the clocking thread to pass raw error terms.
 */
void sw_pll_task(chanend c_sw_pll);

/** Helper function that sends a special restart command. It causes the SDM task
 *  to quit and restart using the new mclk.
 *
 *  \param c_sw_pll                 Channel connected to the clocking thread to pass raw error terms.
 *  \param mclk_Rate                The mclk frequency in Hz.
 */
void restart_sigma_delta(chanend c_sw_pll, unsigned mclk_rate);

/** Performs a frequency comparsion between the incoming digital Rx stream and the local mclk.
 *
 *  \param mclk_time_stamp  The captured mclk count (using port timer) at the time of sample Rx.
 *  \param mclks_per_sample The nominal number of mclks per audio sample.
 *  \param c_sw_pll         Channel connected to the sigma delta and controller thread.
 *  \param receivedSamples  The number of received samples since tha last call to this function.
 *  \param reset_sw_pll_pfd Reference to a flag which will be used to signal reset of this function's state.
 */
void do_sw_pll_phase_frequency_detector_dig_rx( unsigned short mclk_time_stamp,
                                                unsigned mclks_per_sample,
                                                chanend c_sw_pll,
                                                int receivedSamples,
                                                int &reset_sw_pll_pfd);

/** Initilaises the software PLL both hardware and state. Sets the mclk frequency to a nominal point.
 *
 *  \param sw_pll   Reference to a software pll state struct to be initialised.
 *  \param mClk     The current nominal mClk frequency.
 *
 *  returns         The SDM update interval in ticks and the initial DCO setting for nominal frequency */
{unsigned, unsigned} InitSWPLL(sw_pll_state_t &sw_pll, unsigned mClk);

#endif /* XUA_USE_SW_PLL */
#endif /* _SW_PLL_WRAPPPER_H_ */
