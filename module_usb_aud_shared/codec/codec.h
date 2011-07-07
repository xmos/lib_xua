#ifndef _CODEC_H_
#define _CODEC_H_

/* These functions must be implemented for the CODEC/ADC/DAC arrangement of a specific design */

/* TODO Are the channel args required? */

/* Any required CODEC initialisation - run once at start up */
void CodecInit(chanend ?c_codec);

/* Configure condec for a specific mClk/Sample frquency - run on every sample frequency change */
void CodecConfig(unsigned samFreq, unsigned mClk, chanend ?c_codec);

#endif
