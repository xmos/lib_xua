#ifndef _CODEC_H_
#define _CODEC_H_

/* These functions must be implemented for the CODEC/ADC/DAC arrangement of a specific design */

/* Any required clocking and CODEC initialisation - run once at start up */
void AudioHwInit(chanend ?c_codec);

/* Configure audio hardware (clocking, CODECs etc) for a specific mClk/Sample frquency - run on every sample frequency change */
void AudioHwConfig(unsigned samFreq, unsigned mClk, chanend ?c_codec, unsigned dsdMode,
        unsigned sampRes_DAC, unsigned sampRes_ADC);

#endif
