#ifndef __decouple_h__
#define __decouple_h__


/** Manage the data transfer between the USB audio buffer and the 
 *  Audio I/O driver.
 *  
 * \param c_audio_out Channel connected to the audio() or mixer() threads
 * \param c_led Optional chanend connected to an led driver thread for
 *              debugging purposes
 * \param c_midi Optional chanend connect to usb_midi() thread if present
 * \param c_clk_int Optional chanend connected to the clockGen() thread if present
 */
void decouple(chanend c_audio_out,
             // chanend ?c_midi, 
              chanend ?c_clk_int
//#ifdef IAP
//, chanend ?c_iap
//#endif
);

#endif // __decouple_h__
