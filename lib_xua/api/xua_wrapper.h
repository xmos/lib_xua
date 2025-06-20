// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_WRAPPER_H_
#define _XUA_WRAPPER_H_


#include <stdint.h>
#include "xccompat.h"

/** USB Audio Buffering Core(s).
 *
 *  This function buffers USB audio data between the XUD and the audio subsystem.
 *  Most of the chanend parameters to the function should be connected to
 *  XUD_Manager().  The uses two cores.
 *
 *  \param c_aud_out            Audio OUT endpoint channel connected to the XUD
 *  \param c_aud_in             Audio IN endpoint channel connected to the XUD
 *  \param c_aud_fb             Audio feedback endpoint channel connected to the XUD
 *  \param c_midi_from_host     MIDI OUT endpoint channel connected to the XUD
 *  \param c_midi_to_host       MIDI IN endpoint channel connected to the XUD
 *  \param c_midi               Channel connected to MIDI thread
 *  \param c_int                Audio clocking interrupt endpoint channel connected to the XUD
 *  \param c_clk_int            Optional chanend connected to the clockGen() thread if present
 *  \param c_sof                Start of frame channel connected to the XUD
 *  \param c_aud_ctl            Audio control channel connected to  Endpoint0()
 *  \param p_off_mclk           A port that is clocked of the MCLK input (not the MCLK input itself)
 *  \param c_hid                Channel connected to the HID handler thread
 *  \param c_aud                Channel connected to XUA_AudioHub() thread
 *  \param c_audio_rate_change  Channel to notify and synchronise on audio rate change
 *  \param i_pll_ref            Interface to task that toggles reference pin to CS2100
 *  \param c_swpll_update       Channel connected to software PLL task. Expects master clock counts based on USB frames.
 */

void XUA_wrapper_task(chanend c_aud);

/** USB Audio Buffering Core(s).
 *
 *  This function buffers USB audio data between the XUD and the audio subsystem.
 *  Most of the chanend parameters to the function should be connected to
 *  XUD_Manager().  The uses two cores.
 *
 *  \param c_aud_out            Audio OUT endpoint channel connected to the XUD
 *  \param c_aud_in             Audio IN endpoint channel connected to the XUD
 *  \param c_aud_fb             Audio feedback endpoint channel connected to the XUD
 *  \param c_midi_from_host     MIDI OUT endpoint channel connected to the XUD
 *  \param c_midi_to_host       MIDI IN endpoint channel connected to the XUD
 *  \param c_midi               Channel connected to MIDI thread
 *  \param c_int                Audio clocking interrupt endpoint channel connected to the XUD
 *  \param c_clk_int            Optional chanend connected to the clockGen() thread if present
 *  \param c_sof                Start of frame channel connected to the XUD
 *  \param c_aud_ctl            Audio control channel connected to  Endpoint0()
 *  \param p_off_mclk           A port that is clocked of the MCLK input (not the MCLK input itself)
 *  \param c_hid                Channel connected to the HID handler thread
 *  \param c_aud                Channel connected to XUA_AudioHub() thread
 *  \param c_audio_rate_change  Channel to notify and synchronise on audio rate change
 *  \param i_pll_ref            Interface to task that toggles reference pin to CS2100
 *  \param c_swpll_update       Channel connected to software PLL task. Expects master clock counts based on USB frames.
 */
void XUA_wrapper_exchange_samples(chanend c_aud, int32_t samples_to_host[NUM_USB_CHAN_IN], int32_t samples_from_host[NUM_USB_CHAN_OUT]);
#endif
