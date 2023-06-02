// Copyright 2011-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __XUA_BUFFER_H__
#define __XUA_BUFFER_H__

#if __XC__

#include "xua_clocking.h" /* Required for pll_ref_if */

/** USB Audio Buffering Core(s).
 *
 *  This function buffers USB audio data between the XUD and the audio subsystem.
 *  Most of the chanend parameters to the function should be connected to
 *  XUD_Manager().  The uses two cores.
 *
 *  \param c_aud_out          Audio OUT endpoint channel connected to the XUD
 *  \param c_aud_in           Audio IN endpoint channel connected to the XUD
 *  \param c_aud_fb           Audio feedback endpoint channel connected to the XUD
 *  \param c_midi_from_host   MIDI OUT endpoint channel connected to the XUD
 *  \param c_midi_to_host     MIDI IN endpoint channel connected to the XUD
 *  \param c_midi             Channel connected to MIDI core
 *  \param c_int              Audio clocking interrupt endpoint channel connected to the XUD
 *  \param c_clk_int          Optional chanend connected to the clockGen() thread if present
 *  \param c_sof              Start of frame channel connected to the XUD
 *  \param c_aud_ctl          Audio control channel connected to  Endpoint0()
 *  \param p_off_mclk         A port that is clocked of the MCLK input (not the MCLK input itself)
 *  \param c_aud              Channel connected to XUA_AudioHub() core
 *  \param i_pll_ref          Interface to task that toggles reference pin to CS2100
 */
void XUA_Buffer(
            chanend c_aud_out,
#if (NUM_USB_CHAN_IN > 0) || defined(__DOXYGEN__)
            chanend c_aud_in,
#endif
#if (NUM_USB_CHAN_IN == 0) || defined (UAC_FORCE_FEEDBACK_EP)
            chanend c_aud_fb,
#endif
#if defined(MIDI) || defined(__DOXYGEN__)
            chanend c_midi_from_host,
            chanend c_midi_to_host,
			chanend c_midi,
#endif
#if XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN || defined(__DOXYGEN__)
            chanend ?c_int,
            chanend ?c_clk_int,
#endif
            chanend c_sof,
            chanend c_aud_ctl,
            in port p_off_mclk
#if (HID_CONTROLS)
            , chanend c_hid
#endif
            , chanend c_aud
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC) || defined(__DOXYGEN__)
            , client interface pll_ref_if i_pll_ref
#endif
        );

void XUA_Buffer_Ep(chanend c_aud_out,
#if (NUM_USB_CHAN_IN > 0) || defined(__DOXYGEN__)
            chanend c_aud_in,
#endif
#if (NUM_USB_CHAN_IN == 0) || defined (UAC_FORCE_FEEDBACK_EP)
            chanend c_aud_fb,
#endif
#ifdef MIDI
            chanend c_midi_from_host,
            chanend c_midi_to_host,
			chanend c_midi,
#endif
#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN)
            chanend ?c_int,
            chanend ?c_clk_int,
#endif
            chanend c_sof,
            chanend c_aud_ctl,
            in port p_off_mclk
#if (HID_CONTROLS)
            , chanend c_hid
#endif
#ifdef CHAN_BUFF_CTRL
            , chanend c_buff_ctrl
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC) || defined(__DOXYGEN__)
            , client interface pll_ref_if i_pll_ref
#endif
        );

/** Manage the data transfer between the USB audio buffer and the
 *  Audio I/O driver.
 *
 * \param c_audio_out Channel connected to the audio() or mixer() threads
 */
void XUA_Buffer_Decouple(chanend c_audio_out
#ifdef CHAN_BUFF_CTRL
     , chanend c_buff_ctrl
#endif
);
#endif
#endif
