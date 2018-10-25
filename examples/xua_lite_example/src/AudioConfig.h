#ifndef _AUDIO_CONFIG_
#define _AUDIO_CONFIG_

void ConfigAudioPorts(unsigned divide);

void AudioHwInit();

void PLL_Init(void);

/* Configures master clock and codc for passed sample freq */
void AudioHwConfig(unsigned samFreq);

void ConfigCodec24576(unsigned samFeq);

#endif
