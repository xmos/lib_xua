// Copyright 2011-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_AUDIOHUB_H_
#define _XUA_AUDIOHUB_H_

#if defined(__XC__) || defined(__DOXYGEN__)

#include "xua.h"

#if XUA_USB_EN
#include "dfu_interface.h"
#endif

#if CODEC_MASTER
    #define i2s_clk_port_type in_buffered_port_32_t
#else
    #define i2s_clk_port_type out_buffered_port_32_t
#endif

/** The audio driver thread.
 *
 *  This function drives I2S ports and handles samples to/from other digital
 *  I/O threads.
 *
 *  \param c_aud                Audio sample channel connected to the mixer() thread or the
 *                              decouple() thread
 *
 *  \param clk_audio_mclk       Nullable clockblock to be clocked from master clock
 *
 *  \param clk_audio_bclk       Nullable clockblock to be clocked from i2s bit clock
 *
 *  \param p_mclk_in            Master clock inport port (must be 1-bit). Use null when xcore is slave
 *
 *  \param p_lrclk              Nullable port for I2S sample clock
 *
 *  \param p_bclk               Nullable port for I2S bit clock
 *
 *  \param p_i2s_dac            Nullable array of ports for I2S data output lines
 *
 *  \param p_i2s_adc            Nullable array of ports for I2S data input lines
 *
 *  \param i_SoftPll            Interface to software PLL task
 *
 *  \param c_spdif_tx           Channel connected to S/PDIF transmitter core from lib_spdif
 *
 *  \param c_dig                Channel connected to the clockGen() thread for
 *                              receiving/transmitting samples
 *
 *  \param c_audio_rate_change  Channel notifying ep_buffer of an mclk frequency change and sync for stable clock
 *
 *  \param dfuInterface         Interface supporting DFU methods
 *
 *  \param c_pdm_in             Channel for receiving decimated PDM samples
 */
void XUA_AudioHub(
    NULLABLE_RESOURCE(chanend, c_aud),
    NULLABLE_RESOURCE(clock, clk_audio_mclk),
    NULLABLE_RESOURCE(clock, clk_audio_bclk), NULLABLE_RESOURCE(in_port_t, p_mclk_in)
    , NULLABLE_RESOURCE(i2s_clk_port_type, p_lrclk)
    , NULLABLE_RESOURCE(i2s_clk_port_type, p_bclk)
    , NULLABLE_ARRAY_OF_SIZE(out_buffered_port_32_t, p_i2s_dac, I2S_WIRES_DAC)
    , NULLABLE_ARRAY_OF_SIZE(in_buffered_port_32_t, p_i2s_adc, I2S_WIRES_ADC)
#if (XUA_SPDIF_TX_EN) || defined(__DOXYGEN__)
    , chanend c_spdif_tx
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN || defined(__DOXYGEN__))
    , chanend c_dig
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN || defined(__DOXYGEN__))
    , chanend c_audio_rate_change
#endif
#if (((XUD_TILE != 0) && (AUDIO_IO_TILE == 0) && (XUA_DFU_EN == 1)) || defined(__DOXYGEN__))
   , NULLABLE_SERVER_INTERFACE(i_dfu, dfuInterface)
#endif
#if (XUA_NUM_PDM_MICS > 0 || defined(__DOXYGEN__))
    , chanend c_pdm_in
#endif
);

void SpdifTxWrapper(chanend c_spdif_tx);

#endif // __XC__

/**
 * @brief   User buffer management code
 *
 * This function is called at the sample rate of the USB Audio stack (e.g,. 48 kHz) and between the two parameter arrays
 * contain a full multi-channel audio-frame. The first array carries all the data that has been received from the USB host
 * and is to be presented to the audio interfaces. The second array carries all the data received from the interfaces and
 * is to be presented to the USB host. The user can chose to intercept and overwrite the samples stored in these arrays.
 *
 * \param sampsFromUsbToAudio    Samples received from USB host and to be presented to audio interfaces
 *
 * \param sampsFromAudioToUsb    Samples received from the audio interfaces and to be presented to the USB host
*/
void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[]);

/**
 * @brief   User buffer managment init code
 *
 * This function is called once, before the first call to UserBufferManagement(), and can be used to initialise any
 * related user state
 *
 * \param sampFreq               The initial sample frequency
 *
 */
void UserBufferManagementInit(unsigned sampFreq);

#endif // _XUA_AUDIOHUB_H_
