#ifndef __audio_h__
#define __audio_h__

#include "devicedefines.h"
#include "dfu_interface.h"
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
void audio(chanend c_in,
#if defined(SPDIF_TX) && (SPDIF_TX_TILE != AUDIO_IO_TILE)
    chanend c_spdif_tx,
#endif
#if(defined(SPDIF_RX) || defined(ADAT_RX))
    chanend c_dig,
#endif
    chanend ?c_config, chanend ?c_adc
#if (XUD_TILE != 0)
   , server interface i_dfu dfuInterface
#endif
#if (NUM_PDM_MICS > 0)
    , chanend c_pdm_in
#endif
);

void SpdifTxWrapper(chanend c_spdif_tx);

#endif // __audio_h__
