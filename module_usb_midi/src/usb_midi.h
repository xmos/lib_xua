#ifndef __usb_midi_h__
#define __usb_midi_h__

#include "devicedefines.h"

/** USB MIDI I/O thread.
 *
 *  This function passes MIDI data from USB to UART I/O.
 *
 *  \param p_midi_in 1-bit input port for MIDI
 *  \param p_midi_out 1-bit output port for MIDI
 *  \param clk_midi clock block used for clockin the UART; should have 
 *                  a rate of 100MHz
 *  \param c_midi chanend connected to the decouple() thread
 *  \param cable_number the cable number of the MIDI implementation.
 *                      This should be set to 0.
 **/
void usb_midi(port ?p_midi_in, port ?p_midi_out, 
              clock ?clk_midi,
              chanend ?c_midi,
              unsigned cable_number,
chanend ?c_iap, chanend ?c_i2c, // iOS stuff
port ?p_scl, port ?p_sda
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
#endif // __usb_midi_h__
