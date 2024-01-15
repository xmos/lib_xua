// Copyright 2023-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Default implementations of AudioHwInit(), AudioHwConfig(), AudioHwConfig_Mute() and AudioHwConfig_UnMute() */

void AudioHwInit() __attribute__ ((weak));
void AudioHwInit()
{
    return;
}

void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC) __attribute__ ((weak));
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
    return;
}

void AudioHwConfig_Mute() __attribute__ ((weak));
void AudioHwConfig_Mute()
{
    return;
}

void AudioHwConfig_UnMute() __attribute__ ((weak));
void AudioHwConfig_UnMute()
{
    return;
}

