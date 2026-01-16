// Copyright 2015-2026 XMOS LIMITED.
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
 * \param channel_map
 *        Array mapping the logical microphone indices to PDM input channels.
 *        The i<sup>th</sup> entry is the pdm-data port pin that is routed to
 *        microphone output channel *i*
 */
void mic_array_task(chanend c_mic_to_audio, unsigned channel_map[MIC_ARRAY_CONFIG_MIC_COUNT]);

/**
 * @brief User pre-PDM mic function callback (optional).
 *
 * This function is called before mic-array initialisation—both the first time
 * and every time the mic array restarts.
 * It can be used to update the ``channel_map`` in cases where the mapping
 * between PDM input port pins and microphone output channels is not 1:1.
 * It may also be used to initialise any PDM related hardware.
 *
 * @note This function is called on the same tile as the mic-array task
 *       (``XUA_MIC_PDM_TILE_NUM``).
 *       ``channel_map`` is a global array whose default initialisation occurs on
 *       that tile before ``mic_array_task()`` runs.
 *       ``xua_user_pdm_init()`` is invoked by ``mic_array_task()`` itself, before
 *       starting the mic-array hardware thread.
 *
 **/
void xua_user_pdm_init(unsigned channel_map[MIC_ARRAY_CONFIG_MIC_COUNT]);

/**
 * @brief USB PDM Mic PCM sample post processing callback (optional).
 *
 *  This is called after a PCM sample is received from mic_array.
 *  It can be used to modify the samples (gain, filter etc.) before sending
 *  to XUA audiohub.
 *
 * @warning This function is called from Audiohub (I2S) and
 *  so any processing must take significantly less than half of a sample
 *  period else I2S will break timing.
 *
 *  \param mic_audio    Array of samples for in-place processing
 *
 **/
void xua_user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT]);

#endif

