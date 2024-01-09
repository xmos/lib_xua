// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* By default we use SW_PLL for Digital Rx configs running on XCORE-AI */
/* Note: Not yet implemented for Synchronous mode */
#ifdef __XS3A__
#ifndef USE_SW_PLL
#define USE_SW_PLL  1
#endif /* USE_SW_PLL */
#else
#define USE_SW_PLL  0
#endif /* __XS3A__ */


extern "C"
{
    #include "sw_pll.h"
}

/* Special control value to disable SDM. Outside of normal range which is less than 16b.*/
#define DISABLE_SDM     0x1000000 


/** Task that receives an error term, passes it through a PI controller and periodically
 *  calclulates a sigma delta output value and sends it to the PLL fractional register.
 *
 *  \param c_sigma_delta Channel connected to the clocking thread to pass raw error terms.
 *  \param sdm_interval  Unisgned value containing the sigma delta period in timer ticks.
 */
void SigmaDeltaTask(chanend c_sigma_delta, unsigned sdm_interval);

/** Helper function that sends a special disable command and waits for ACK. This is used
 *  to help prevemt simultaenous access to the PLL register from two different threads,
 *
 *  \param c_sigma_delta Channel connected to the clocking thread to pass raw error terms.
 */
void disable_sigma_delta(chanend c_sigma_delta);

/** Performs a frequency comparsion between the incoming digital Rx stream and the local mclk.
 *
 *  \param mclk_time_stamp  The captured mclk count (using port timer) at the time of sample Rx.
 *  \param mclks_per_sample The nominal number of mclks per audio sample.
 *  \param c_sigma_delta    Channel connected to the sigma delta and controller thread.
 *  \param receivedSamples  The number of received samples since tha last call to this function.
 *  \param reset_sw_pll_pfd Reference to a flag which will be used to signal reset of this function's state.
 */
void do_sw_pll_phase_frequency_detector_dig_rx( unsigned short mclk_time_stamp,
                                                unsigned mclks_per_sample,
                                                chanend c_sigma_delta,
                                                int receivedSamples,
                                                int &reset_sw_pll_pfd);

/** Initilaises the software PLL both hardware and state. Sets the mclk frequency to a nominal point. 
 *
 *  \param sw_pll   Reference to a software pll state struct to be initialised.
 *  \param mClk     The current nominal mClk frequency.
 */
unsigned InitSWPLL(sw_pll_state_t &sw_pll, unsigned mClk);
