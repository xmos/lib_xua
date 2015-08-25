#ifndef __USB_BUFFER_H__
#define __USB_BUFFER_H__
/** USB Audio Buffering Thread.
 *
 *  This function buffers USB audio data between the XUD layer and the decouple
 *  thread. Most of the chanend parameters to the function should be connected to
 *  XUD_Manager()
 *
 *  \param c_aud_out Audio OUT endpoint channel connected to the XUD
 *  \param c_aud_in  Audio IN endpoint channel connected to the XUD
 *  \param c_aud_fb  Audio feedback endpoint channel connected to the XUD
 *  \param c_midi_from_host  MIDI OUT endpoint channel connected to the XUD
 *  \param c_midi_to_host  MIDI IN endpoint channel connected to the XUD
 *  \param c_int  Audio clocking interrupt endpoint channel connected to the XUD
 *  \param c_clk_int Optional chanend connected to the clockGen() thread if present
 *  \param c_sof  Start of frame channel connected to the XUD
 *  \param c_aud_ctl Audio control channel connected to  Endpoint0()
 *  \param p_off_mclk A port that is clocked of the MCLK input (not the MCLK input itself)
 */
#include "devicedefines.h"

void buffer(chanend c_aud_out,
            chanend c_aud_in,
#if (NUM_USB_CHAN_IN == 0) || defined (UAC_FORCE_FEEDBACK_EP)
            chanend c_aud_fb,
#endif
#ifdef MIDI
            chanend c_midi_from_host,
            chanend c_midi_to_host,
			chanend c_midi,
#endif
#ifdef IAP
            chanend c_iap_from_host,
            chanend c_iap_to_host,
#ifdef IAP_INT_EP
            chanend c_iap_to_host_int,
#endif
            chanend c_iap,
#ifdef IAP_EA_NATIVE_TRANS
            chanend c_iap_ea_native_out,
            chanend c_iap_ea_native_in,
            chanend c_iap_ea_native_ctrl,
            chanend c_iap_ea_native_data,
#endif
#endif
#if defined(SPDIF_RX) || defined(ADAT_RX)
            chanend ?c_int,
            chanend ?c_clk_int,
#endif
            chanend c_sof,
            chanend c_aud_ctl,
            in port p_off_mclk
#ifdef HID_CONTROLS
            , chanend c_hid
#endif
#ifdef CHAN_BUFF_CTRL
            , chanend c_buff_ctrl
#endif
        );
#endif
