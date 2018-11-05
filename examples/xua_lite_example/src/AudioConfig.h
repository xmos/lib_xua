#ifndef _AUDIO_CONFIG_
#define _AUDIO_CONFIG_

void AudioHwConfigure(unsigned samFreq, client i2c_master_if i_i2c);
void pll_nudge(int nudge);


#endif
