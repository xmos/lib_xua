#ifndef __audio_h__
#define __audio_h__

/** The audio driver thread.
 *
 *  This function drives I2S ports and handles samples to/from other digital
 *  I/O threads.
 *
 *  \param c_in Audio sample channel connected to the mixer() thread or the
 *              decouple() thread
 *  \param c_dig channel connected to the clockGen() thread for
 *               receiving/transmitting samples
 *  \param c_config An optional channel that will be passed on to the
 *                  CODEC configuration functions.
 */
void audio(chanend c_in, chanend ?c_dig, chanend ?c_config, chanend ?c_adc);

#endif // __audio_h__
