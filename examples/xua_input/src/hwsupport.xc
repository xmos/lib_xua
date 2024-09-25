// Copyright 2017-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <assert.h>
#include <platform.h>

#include "xua.h"
#include "xk_evk_xu316/board.h"


void AudioHwInit()
{
    xk_evk_xu316_config_t hw_config = {MCLK_48};
    xk_evk_xu316_AudioHwInit(hw_config);

    return;
}

/* Configures the external audio hardware for the required sample frequency */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode,
    unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    samFreq = 48000;
    xk_evk_xu316_AudioHwConfig(samFreq, mClk, dsdMode, sampRes_DAC, sampRes_ADC);

    return;
}
//:
