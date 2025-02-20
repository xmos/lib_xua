// Copyright 2017-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <platform.h>
#include <string.h>

#include "xua.h"
#include "xud_device.h"

/* Copy mic samples inbound to the host to the DAC so they can be monitored there */
#pragma unsafe arrays
void UserBufferManagement(unsigned sampsFromUsbToAudio[], unsigned sampsFromAudioToUsb[])
{

    for(int i = 0; i < I2S_CHANS_DAC; i++)
    {
        sampsFromUsbToAudio[i] = sampsFromAudioToUsb[i];
    }
}

/* Apply some gain so we can hear the mics easily (non-saturating - will overflow) */
#pragma unsafe arrays
void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT])
{
    for(int i = 0; i < XUA_NUM_PDM_MICS; i++)
    {
        mic_audio[i] = mic_audio[i] << 6; /* x64 */
    }
}

