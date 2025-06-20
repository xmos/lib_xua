// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua.h"                          /* Device specific defines */
#include "xua_commands.h"
#include <string.h>

#define XUA_MAX(x,y) ((x)>(y) ? (x) : (y))


const int readBuffNo = 0;
unsigned command = XUA_AUDCTL_NO_COMMAND;
unsigned underflowWord = 0;
extern unsigned samplesOut[XUA_MAX(NUM_USB_CHAN_OUT, I2S_CHANS_DAC)];
extern unsigned samplesIn[2][XUA_MAX(NUM_USB_CHAN_IN, I2S_CHANS_DAC)];
extern void receive_command(unsigned command, chanend c_aud, unsigned &curSamFreq, unsigned &dsdMode, unsigned &curSamRes_DAC, unsigned &audioActive);

#include "xua_audiohub_st.h"

void XUA_wrapper_exchange_samples(chanend c_aud, int32_t samples_to_host[NUM_USB_CHAN_IN], int32_t samples_from_host[NUM_USB_CHAN_OUT])
{
	StartSampleTransfer(c_aud, underflowWord);                  /* Send first token to fire ISR in decouple */
	memcpy(samplesIn[readBuffNo], samples_to_host, sizeof(samples_to_host));
	CompleteSampleTransferUsbChans(c_aud, readBuffNo, command); /* Check for command & transfer the samples & UBM */
	memcpy(samples_from_host, samplesOut, sizeof(samples_from_host));
	if(command != XUA_AUDCTL_NO_COMMAND)
	{
	    /* Just consume the command, ignore it + keep on looping forever */
	    unsigned dummy1, dummy2, dummy3, dummy4;
	    receive_command(command, c_aud, dummy1, dummy2, dummy3, dummy4);
	    outct(c_aud, XS1_CT_END);
	}
    
}