#ifndef __audio_h__
#define __audio_h__

#if __XC__

#ifndef NO_USB
#include "dfu_interface.h"
#endif

typedef interface audManage_if
{
    [[guarded]]
    void transfer_buffers(int * unsafe in_aud_buf, int * unsafe in_usb_buf,
                            int * unsafe out_usb_buf, int * unsafe out_aud_buf);

    [[guarded]]
    void transfer_samples(int in_mic_buf[], int in_spk_buf[], int out_mic_buf[], int out_spk_buf[]);

} audManage_if;


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
void audio(chanend ?c_in,
#if defined(SPDIF_TX) && (SPDIF_TX_TILE != AUDIO_IO_TILE)
    chanend c_spdif_tx,
#endif
#if(defined(SPDIF_RX) || defined(ADAT_RX))
    chanend c_dig,
#endif
    chanend ?c_config, chanend ?c_adc
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0)
   , server interface i_dfu ?dfuInterface
#endif
#if (NUM_PDM_MICS > 0)
    , chanend c_pdm_in
#endif
    , client audManage_if i_audMan
);

void SpdifTxWrapper(chanend c_spdif_tx);

#endif // __XC__

#endif // __audio_h__
