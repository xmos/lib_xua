// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_AUDIOHUB_H_
#define _XUA_AUDIOHUB_H_

#ifdef __XC__

#include "xccompat.h"
#include "xs1.h"

#if XUA_USB_EN
#include "dfu_interface.h"
#endif

/** The audio driver thread.
 *
 *  This function drives I2S ports and handles samples to/from other digital
 *  I/O threads.
 *
 *  \param c_aud            Audio sample channel connected to the mixer() thread or the
 *                          decouple() thread
 *
 *  \param clk_audio_mclk   Nullable clockblock to be clocked from master clock
 *
 *  \param clk_audio_bclk   Nullable clockblock to be clocked from i2s bit clock
 *
 *  \param p_mclk_in        Master clock inport port (must be 1-bit)
 *
 *  \param p_lrclk          Nullable port for I2S sample clock
 *
 *  \param p_bclk           Nullable port for I2S bit
 *
 *  \param p_i2s_dac        Nullable array of ports for I2S data output lines
 *
 *  \param p_i2s_adc        Nullable array of ports for I2S data input lines
 *
 *  \param c_spdif_tx       Channel connected to S/PDIF transmiter core from lib_spdif
 *
 *  \param c_dig            Channel connected to the clockGen() thread for
 *                          receiving/transmitting samples
 */
void XUA_AudioHub(chanend ?c_aud,
    clock ?clk_audio_mclk,
    clock ?clk_audio_bclk,
    in port p_mclk_in,
    buffered _XUA_CLK_DIR port:32 ?p_lrclk,
    buffered _XUA_CLK_DIR port:32 ?p_bclk,
    buffered out port:32 (&?p_i2s_dac)[I2S_WIRES_DAC],
    buffered in port:32  (&?p_i2s_adc)[I2S_WIRES_ADC]
#if (XUA_SPDIF_TX_EN) || defined(__DOXYGEN__)
    , chanend c_spdif_tx
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN || defined(__DOXYGEN__))
    , chanend c_dig
#endif
#if (XUD_TILE != 0) && (AUDIO_IO_TILE == 0) && (XUA_DFU_EN == 1)
   , server interface i_dfu ?dfuInterface
#endif
#if (XUA_NUM_PDM_MICS > 0)
    , chanend c_pdm_in
#endif
);

void SpdifTxWrapper(chanend c_spdif_tx);

/* The 4 functions below should implemented for the external audio haardware arrangement of a specific design.
 * Note, default (empty) implementations of these are provided in audiohub_user.c
 */

/** User code for any required audio hardwarte initialisation - run once at start up */
void AudioHwInit(void);

/** User code to mute audio hardware before a sample rate change - run every sample frequency change */
void AudioHwConfig_Mute(void);

/** User code to un-mute audio hardware after a sample rate change - run every sample frequency change */
void AudioHwConfig_UnMute(void);

/** User code Configure audio hardware (clocking, CODECs etc) for a specific mClk/Sample frquency - run on every sample frequency change
 *
 * \param samFreq       The new sample frequency (in Hz)
 *
 * \param mclk          The new master clock frequency (in Hz)
 *
 * \param dsdMode       DSD mode, DSD_MODE_NATIVE, DSD_MODE_DOP or DSD_MODE_OFF
 *
 * \param sampRes_DAC   Playback sample resolution (in bits)
 *
 * \param sampRes_ADC   Record sample resolution (in bits)
 */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC);

#endif // __XC__

void UserBufferManagementInit();

void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[]);

#endif // _XUA_AUDIOHUB_H_
