// Copyright 2016-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xs1.h>
#include <stdio.h>

void AudioHwInit(void)
{
    printf("AudioHwInit\n");
}

void AudioHwShutdown(void)
{
    printf("AudioHwShutdown\n");
}

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    printf("AudioHwConfig %u\n", samFreq);
}
