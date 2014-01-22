#ifndef __mixer_h__
#define __mixer_h__

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

#endif
