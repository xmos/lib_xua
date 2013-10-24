#ifndef _CODEC_H_
#define _CODEC_H_

#include "dsd.h"

/* These functions must be implemented for the CODEC/ADC/DAC arrangement of a specific design */

/* TODO Are the channel args required? */

/* Any required clocking and CODEC initialisation - run once at start up */
void AudioHwInit(chanend ?c_codec);

/* Configure audio hardware (clocking, CODECs etc) for a specific mClk/Sample frquency - run on every sample frequency change */
void AudioHwConfig(unsigned samFreq, unsigned mClk, chanend ?c_codec, DsdMode dsdMode);

#endif
