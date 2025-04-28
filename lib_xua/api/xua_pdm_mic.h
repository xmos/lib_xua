// Copyright 2015-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUA_PDM_MIC_H
#define XUA_PDM_MIC_H

#include <xccompat.h>
#include <stdint.h>

#include "mic_array_conf.h"
#include "mic_array.h"

#ifdef __cplusplus
extern "C" {
#endif
void ma_init(unsigned mic_samp_rate);
void ma_task(chanend c_mic_to_audio);
#ifdef __cplusplus
}
#endif

/** USB PDM Mic task.
 *
 *  This task runs the PDM rx and decimators and passes PCM samples to XUA.
 *  It runs forever and currently supports a single sample rate of 
 *  48 kHz, 32 kHz or 16 kHz
 *
 *  \param c_mic_to_audio    1-bit input port for MIDI
 * 
 **/
void mic_array_task(chanend c_mic_to_audio);

/** User pre-PDM mic function callback (optional).
 *  Use to initialise any PDM related hardware.
 *
 **/
void user_pdm_init();

/** USB PDM Mic PCM sample post processing callback (optional).
 *
 *  This is called after the raw PCM samples are received from mic_array.
 *  It can be used to modify the samples (gain, filter etc.) before sending
 *  to XUA audiohub. Please note this is called from Audiohub (I2S) and
 *  so any processing must take significantly less than on half of a sample
 *  period else I2S will break timing.
 *
 *  \param mic_audio    Array of samples for in-place processing
 * 
 **/void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT]);

#endif

