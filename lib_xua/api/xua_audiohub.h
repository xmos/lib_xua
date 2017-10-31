#ifndef __XUA_AUDIOHUB_H__
#define __XUA_AUDIOHUB_H__

#if __XC__

#include "xccompat.h"

#ifndef NO_USB
#include "dfu_interface.h"
#endif

/** The audio driver thread.
 *
 *  This function drives I2S ports and handles samples to/from other digital
 *  I/O threads.
 *
 *  \param c_aud Audio sample channel connected to the mixer() thread or the
 *              decouple() thread
 *  \param c_dig channel connected to the clockGen() thread for
 *               receiving/transmitting samples
 *  \param c_config An optional channel that will be passed on to the
 *                  CODEC configuration functions.
 */
void XUA_AudioHub(chanend ?c_aud
#if defined(SPDIF_TX) && (SPDIF_TX_TILE != AUDIO_IO_TILE)
    , chanend c_spdif_tx
#endif
#if(defined(SPDIF_RX) || defined(ADAT_RX))
    , chanend c_dig
#endif
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0) && (XUA_DFU_EN == 1)
   , server interface i_dfu ?dfuInterface
#endif
#if (NUM_PDM_MICS > 0)
    , chanend c_pdm_in
#endif
);

void SpdifTxWrapper(chanend c_spdif_tx);

/* These functions must be implemented for the CODEC/ADC/DAC arrangement of a specific design */

/* Any required clocking and CODEC initialisation - run once at start up */
/* TODO Provide default implementation of this */
void AudioHwInit();

/* Configure audio hardware (clocking, CODECs etc) for a specific mClk/Sample frquency - run on every sample frequency change */
/* TODO Provide default implementation of this */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode,
        unsigned sampRes_DAC, unsigned sampRes_ADC);

#endif // __XC__

void UserBufferManagementInit();

void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[]);

#endif // __XUA_AUDIOHUB_H__
