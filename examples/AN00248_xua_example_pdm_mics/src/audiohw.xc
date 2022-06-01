// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <assert.h>
#include <platform.h>

#include "xua.h"

/* 0: DAC reset */
/* 1: Ethernet Phy reset */
on tile[1] : out port p_gpio = XS1_PORT_4F;

void AudioHwInit()
{
    /* DAC in reset */
    p_gpio <: 0;

    return;
}

/* Configures the external audio hardware for the required sample frequency */
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode,
    unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    /* Note, without any config the Cirrus 2100 will output it's 24.576MHz ref clock
       to the Aux output - which we will use for mclk */

    return;
}
//:
