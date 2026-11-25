// Copyright 2024-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _SW_PLL_WRAPPPER_H_
#define _SW_PLL_WRAPPPER_H_

#include "xua.h"

#if XUA_USE_SW_PLL
#include "xua_rate_window.h"

extern "C"
{
    #include "sw_pll.h"
}

/* Special control value to disable SDM. Outside of normal range which is less than 16b.*/
#define DISABLE_SDM     0x10000000

#define SET_DCO_TO_NOMINAL 0x10000001


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

/** Measures MCLK against a sample stream and passes the error to the SW PLL.
 *  Call at each measurement point, after adding the samples received since the last one to
 *  window.received_samples. Once per complete window the error is checked for plausibility,
 *  sent to the SW PLL if valid, and the next window started.
 *
 *  \param window              Measurement window, set up with xua_rate_window_init() and
 *                             reset with xua_rate_window_reset() when the stream is interrupted.
 *  \param mclk_time_stamp     The captured mclk count (using port timer) at this point.
 *  \param reference_timestamp The reference timer at this point; ignored if the window has no
 *                             interval timing bounds.
 *  \param mclks_per_count     The nominal number of mclks per unit added to received_samples.
 *  \param c_sw_pll            Channel connected to the sigma delta and controller thread.
 *
 *  \returns 0 if the window is not yet complete, 1 if an error was sent, -1 if the window
 *           was discarded as implausible.
 */
int sw_pll_rate_window_update(xua_rate_window_t &window,
                              unsigned short mclk_time_stamp,
                              unsigned reference_timestamp,
                              unsigned mclks_per_count,
                              chanend c_sw_pll);

/** Initilaises the software PLL both hardware and state. Sets the mclk frequency to a nominal point.
 *
 *  \param sw_pll   Reference to a software pll state struct to be initialised.
 *  \param mClk     The current nominal mClk frequency.
 *
 *  returns         The SDM update interval in ticks and the initial DCO setting for nominal frequency */
{unsigned, unsigned} InitSWPLL(sw_pll_state_t &sw_pll, unsigned mClk);

/** Helper function that sends a command to reset dco setting to the midpoint. It causes the SDM task
 *  to set the dco setting to SW_PLL_SDM_CTRL_MID_24 or SW_PLL_SDM_CTRL_MID_22 depending on the mclk
 *  rate.
 *  \param c_sw_pll                 Channel connected to the clocking thread to pass raw error terms.
 *  \param selected_mclk_rate       Current mclk frequency in Hz.
 */
void sw_pll_set_pll_to_nominal(chanend c_sw_pll, unsigned selected_mclk_rate);

#endif /* XUA_USE_SW_PLL */
#endif /* _SW_PLL_WRAPPPER_H_ */
