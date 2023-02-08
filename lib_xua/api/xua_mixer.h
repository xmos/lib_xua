// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_MIXER_H_
#define _XUA_MIXER_H_

#include "xua.h"

enum mix_ctl_cmd {
  SET_SAMPLES_TO_HOST_MAP,
  SET_SAMPLES_TO_DEVICE_MAP,
  SET_MIX_MULT,
  SET_MIX_MAP,
  SET_MIX_IN_VOL,
  SET_MIX_OUT_VOL,
  GET_INPUT_LEVELS,
  GET_STREAM_LEVELS,
  GET_OUTPUT_LEVELS
};


/** Digital sample mixer.
 *
 *  This thread mixes audio streams between the decouple() thread and
 *  the audio() thread.
 *
 *  \param c_to_host a chanend connected to the decouple() thread for
 *                   receiving/transmitting samples
 *  \param c_to_audio a chanend connected to the audio() thread for
 *                    receiving/transmitting samples
 *  \param c_mix_ctl a chanend connected to the Endpoint0() thread for
 *                   receiving control commands
 *
 */
void mixer(chanend c_to_host, chanend c_to_audio, chanend c_mix_ctl);

#define XUA_MIXER_OFFSET_OUT        (0)
#define XUA_MIXER_OFFSET_IN         (NUM_USB_CHAN_OUT)
#define XUA_MIXER_OFFSET_MIX        (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN)
#define XUA_MIXER_OFFSET_OFF        (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT)

/* Defines uses for DB to actual muliplier conversion */
#define XUA_MIXER_MULT_FRAC_BITS    (25)
#define XUA_MIXER_DB_FRAC_BITS      (8)
#define XUA_MIXER_MAX_MULT          (1<<XUA_MIXER_MULT_FRAC_BITS) /* i.e. multiply by 0 */

#endif
