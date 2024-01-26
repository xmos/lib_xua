// Copyright 2023-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _AUDIO_HW_H_
#define _AUDIO_HW_H_

/* The functions below should be implemented for the external audio hardware arrangement of a specific design.
 * Note, default (empty) implementations of these are provided in audiohub_user.c
 */

/**
 * @brief   User audio hardware initialisation code
 *
 * This function is called when the device starts up and should contain user code to perform any required audio hardware initialisation
 */
void AudioHwInit(void);

/**
 * @brief   User audio hardware configuration code
 *
 * This function is called when on sample rate change and should contain user code to configure audio hardware
 *  (clocking, CODECs etc) for a specific mClk/Sample frequency
 *
 * \param samFreq       The new sample frequency (in Hz)
 *
 * \param mClk          The new master clock frequency (in Hz)
 *
 * \param dsdMode       DSD mode, DSD_MODE_NATIVE, DSD_MODE_DOP or DSD_MODE_OFF
 *
 * \param sampRes_DAC   Playback sample resolution (in bits)
 *
 * \param sampRes_ADC   Record sample resolution (in bits)
 */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC);

/**
 * @brief   User code mute audio hardware
 *
 * This function is called before AudioHwConfig() and should contain user code to mute audio hardware before a
 * sample rate change in order to reduced audible pops/clicks
 *
 *  Note, if using the application PLL of a xcore.ai device this function will be called before the master-clock is
 *  changed
 */
void AudioHwConfig_Mute(void);

/**
 * @brief   User code to un-mute audio hardware
 *
 * This function is called after AudioHwConfig() and should contain user code to un-mute audio hardware after a
 *  sample rate change
 */
void AudioHwConfig_UnMute(void);

#endif
