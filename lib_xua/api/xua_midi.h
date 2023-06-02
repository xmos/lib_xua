// Copyright 2011-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_MIDI_H_
#define _XUA_MIDI_H_

#include "xua.h"

#ifndef MIDI_SHIFT_TX
#define MIDI_SHIFT_TX      (0)
#endif

/** USB MIDI I/O task.
 *
 *  This function passes MIDI data between XUA_Buffer and MIDI UART I/O.
 *
 *  \param p_midi_in    1-bit input port for MIDI
 *  \param p_midi_out   1-bit output port for MIDI
 *  \param clk_midi     Clock block used for clockin the UART; should have
 *                      a rate of 100MHz
 *  \param c_midi       Chanend connected to the decouple() thread
 *  \param cable_number The cable number of the MIDI implementation.
 *                      This should be set to 0.
 **/
void usb_midi(
#if (MIDI_RX_PORT_WIDTH == 4)
    buffered in port:4 ?p_midi_in,
#else
    buffered in port:1 ?p_midi_in,
#endif
    port ?p_midi_out,
    clock ?clk_midi,
    chanend ?c_midi,
    unsigned cable_number
#ifdef IAP
    , chanend ?c_iap, chanend ?c_i2c, // iOS stuff
    port ?p_scl, port ?p_sda
#endif
);

#define MAX_USB_MIDI_PACKET_SIZE 1024
#define MIDI_USB_BUFFER_FROM_HOST_FIFO_SIZE (512+1024)
#define MIDI_USB_BUFFER_TO_HOST_SIZE (256)
#define MIDI_ACK 20
#define USB_MIDI_DEVICE_OUT_FIFO_SIZE (1024)

#ifdef __MIDI_IMPL
#define INLINE
#else
#define INLINE inline
#endif

#ifdef NO_INLINE_MIDI_SELECT_HANDLER
#pragma select handler
void midi_get_ack_or_data(chanend c, int &is_ack, unsigned int &datum);
#else
#pragma select handler
INLINE void midi_get_ack_or_data(chanend c, int &is_ack, unsigned int &datum) {
  if (testct(c)) {
    is_ack = 1;
    (void) inct(c); // read 1-bytes control token
    (void) inuchar(c);
    (void) inuchar(c);
    (void) inuchar(c);
  }
  else {
    is_ack = 0;
    datum = inuint(c);
  }
}
#endif

INLINE void midi_send_ack(chanend c) {
  outct(c, MIDI_ACK);
  outuchar(c, 0);
  outuchar(c, 0);
  outuchar(c, 0);
}
#define MIDI_RATE           (31250)
#define MIDI_BITTIME        (XS1_TIMER_MHZ * 1000000 / MIDI_RATE)
#define MIDI_BITTIME_2      (MIDI_BITTIME>>1)
#endif // _XUA_MIDI_H_
