// Copyright 2015-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUA_PDM_MIC_H
#define XUA_PDM_MIC_H

#include <xccompat.h>
#include <stdint.h>

#include "mic_array.h"

/**
 * @brief USB PDM microphone task.
 *
 * Starts the mic-array processing thread(s)
 *
 * Supported sample rates: 16 kHz, 32 kHz, and 48 kHz.
 *
 * The task runs in a continuous while(1) loop until `ma_shutdown()` is invoked.
 * When `ma_shutdown()` is called, the internal mic thread terminates
 * (`mic_array_start()` returns). After that, a new sampling-rate value may be
 * received on the same channel, and the mic-array thread is then started again
 * at the new rate.
 *
 * c_mic_to_audio channel usage:
 * - While the mic thread is running, decimated PCM frames are sent from the
 *   mic array to the application over this channel.
 * - Before the mic thread is started, the PCM sampling rate is received over
 *   this channel.
 *
 * \param c_mic_to_audio
 *        Channel over which decimated PCM frames are produced by the mic array
 *        and delivered to the application.
 */
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
 **/
void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT]);

#endif

