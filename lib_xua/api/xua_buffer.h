// Copyright 2011-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_BUFFER_H_
#define _XUA_BUFFER_H_

#if defined(__XC__) || defined(__DOXYGEN__)

#include "xccompat.h"
#include "xua_clocking.h" /* Required for pll_ref_if */

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

void XUA_Buffer(
#if (NUM_USB_CHAN_OUT > 0) || defined(__DOXYGEN__)
            chanend c_aud_out,
#endif
#if (NUM_USB_CHAN_IN > 0) || defined(__DOXYGEN__)
            chanend c_aud_in,
#endif
#if ((NUM_USB_CHAN_OUT > 0) && ((NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP))) || defined(__DOXYGEN__)
            chanend c_aud_fb,
#endif
#if defined(MIDI) || defined(__DOXYGEN__)
            chanend c_midi_from_host,
            chanend c_midi_to_host,
            chanend c_midi,
#endif
#if XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN || defined(__DOXYGEN__)
            NULLABLE_RESOURCE(chanend, c_int),
            NULLABLE_RESOURCE(chanend, c_clk_int),
#endif
            chanend c_sof,
            chanend c_aud_ctl,
            NULLABLE_RESOURCE(in_port_t, p_off_mclk),
#if (HID_CONTROLS) || defined(__DOXYGEN__)
            chanend c_hid,
#endif
            chanend c_aud
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC) || defined(__DOYXGEN__)
            , chanend c_audio_rate_change
    #if (!XUA_USE_SW_PLL) || defined(__DOXYGEN__)
            , CLIENT_INTERFACE(pll_ref_if, i_pll_ref)
    #endif
    #if (XUA_USE_SW_PLL) || defined(__DOXYGEN__)
            , chanend c_swpll_update
    #endif
#endif
        );


void XUA_Buffer_Ep(

#if (NUM_USB_CHAN_OUT > 0)
            chanend c_aud_out,
#endif
#if (NUM_USB_CHAN_IN > 0) || defined(__DOXYGEN__)
            chanend c_aud_in,
#endif
#if ((NUM_USB_CHAN_OUT > 0) && ((NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP))) || defined(__DOXYGEN__)
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
            in port ?p_off_mclk
#if (HID_CONTROLS)
            , chanend c_hid
#endif
#ifdef XUA_CHAN_BUFF_CTRL
            , chanend c_buff_ctrl
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC) || defined(__DOYXGEN__)
            , chanend c_audio_rate_change
    #if (!XUA_USE_SW_PLL) || defined(__DOXYGEN__)
            , client interface pll_ref_if i_pll_ref
    #endif
    #if (XUA_USE_SW_PLL) || defined(__DOXYGEN__)
            , chanend c_swpll_update
    #endif
#endif
    );


/** Manage the data transfer between the USB audio buffer and the
 *  Audio I/O driver.
 *
 * \param c_audio_out Channel connected to the audio() or mixer() threads
 */
void XUA_Buffer_Decouple(chanend c_audio_out
#ifdef XUA_CHAN_BUFF_CTRL
     , chanend c_buff_ctrl
#endif
);
#endif

#define XUA_MAX(x,y) ((x)>(y) ? (x) : (y))

/* TODO use SLOTSIZE to potentially save memory */
/* Note we could improve on this, for one subslot is set to 4 */
/* The *4 is conversion to bytes, note we're assuming a slotsize of 4 here whic is potentially as waste */
#define MAX_DEVICE_AUD_PACKET_SIZE_MULT_HS  ((MAX_FREQ/8000+1)*4)
#define MAX_DEVICE_AUD_PACKET_SIZE_MULT_FS  ((MAX_FREQ_FS/1000+1)*4)

/*** IN PACKET SIZES ***/
/* Max packet sizes in bytes. Note the +4 is because we store packet lengths in the buffer */
#define MAX_DEVICE_AUD_PACKET_SIZE_IN_HS  (MAX_DEVICE_AUD_PACKET_SIZE_MULT_HS * NUM_USB_CHAN_IN + 4)
#define MAX_DEVICE_AUD_PACKET_SIZE_IN_FS  (MAX_DEVICE_AUD_PACKET_SIZE_MULT_FS * NUM_USB_CHAN_IN_FS + 4)

#define MAX_DEVICE_AUD_PACKET_SIZE_IN (XUA_MAX(MAX_DEVICE_AUD_PACKET_SIZE_IN_FS, MAX_DEVICE_AUD_PACKET_SIZE_IN_HS))

/*** OUT PACKET SIZES ***/
#define MAX_DEVICE_AUD_PACKET_SIZE_OUT_HS  (MAX_DEVICE_AUD_PACKET_SIZE_MULT_HS * NUM_USB_CHAN_OUT + 4)
#define MAX_DEVICE_AUD_PACKET_SIZE_OUT_FS  (MAX_DEVICE_AUD_PACKET_SIZE_MULT_FS * NUM_USB_CHAN_OUT_FS + 4)

#define MAX_DEVICE_AUD_PACKET_SIZE_OUT (XUA_MAX(MAX_DEVICE_AUD_PACKET_SIZE_OUT_FS, MAX_DEVICE_AUD_PACKET_SIZE_OUT_HS))

/*** BUFFER SIZES ***/
/* How many packets too allow for in buffer - minimum is 5.
2 for having in the aud_to_host buffer when it comes out of underflow, space available for 2 more for to accomodate cases when
2 pkts from audio hub get written into the aud_to_host buffer within 1 SOF period, and space for 1 extra packet to ensure that
when the 4th packet gets written to the buffer, there's space to accomodate the next packet, otherwise handle_audio_request() will
drop packets after writing the 4th packet in the buffer
*/
#define BUFFER_PACKET_COUNT (5)

#define BUFF_SIZE_OUT_HS    (MAX_DEVICE_AUD_PACKET_SIZE_OUT_HS * BUFFER_PACKET_COUNT)
#define BUFF_SIZE_OUT_FS    (MAX_DEVICE_AUD_PACKET_SIZE_OUT_FS * BUFFER_PACKET_COUNT)

#define BUFF_SIZE_IN_HS     (MAX_DEVICE_AUD_PACKET_SIZE_IN_HS * BUFFER_PACKET_COUNT)
#define BUFF_SIZE_IN_FS     (MAX_DEVICE_AUD_PACKET_SIZE_IN_FS * BUFFER_PACKET_COUNT)

#define BUFF_SIZE_OUT       XUA_MAX(BUFF_SIZE_OUT_HS, BUFF_SIZE_OUT_FS)
#define BUFF_SIZE_IN        XUA_MAX(BUFF_SIZE_IN_HS, BUFF_SIZE_IN_FS)

#define OUT_BUFFER_PREFILL  (XUA_MAX(MAX_DEVICE_AUD_PACKET_SIZE_OUT_HS, MAX_DEVICE_AUD_PACKET_SIZE_OUT_FS))

#endif
