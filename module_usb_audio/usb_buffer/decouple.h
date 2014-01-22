#ifndef __DECOUPLE_H__
#define __DECOUPLE_H__


/** Manage the data transfer between the USB audio buffer and the
 *  Audio I/O driver.
 *
 * \param c_audio_out Channel connected to the audio() or mixer() threads
 * \param c_clk_int Optional chanend connected to the clockGen() thread if present
 */
void decouple(chanend c_audio_out,
              chanend ?c_clk_int
#ifdef CHAN_BUFF_CTRL
                , chanend c_buff_ctrl
#endif
);

#endif // __DECOUPLE_H__
