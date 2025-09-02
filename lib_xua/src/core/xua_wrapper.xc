// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua.h"
#include "xua_commands.h"
#include <string.h>

const int readBuffNo = 0; 	// Note we don't need to double buffer here due to copy
unsigned underflowWord = 0;	// Always assumuing PCM

// Variables used by audiohub for sample transfer 
/* Two buffers for ADC data to allow for DAC and ADC I2S ports being offset */
#define IN_CHAN_COUNT (I2S_CHANS_ADC + XUA_NUM_PDM_MICS + (8*XUA_ADAT_RX_EN) + (2*XUA_SPDIF_RX_EN))
extern unsigned samplesOut[XUA_MAX(NUM_USB_CHAN_OUT, I2S_CHANS_DAC)];
extern unsigned samplesIn[2][XUA_MAX(NUM_USB_CHAN_IN, IN_CHAN_COUNT)];

/* Sample transfer functions use on the audiohub side */
#include "xua_audiohub_st.h"

extern void receive_command(unsigned command, chanend c_aud, unsigned &curSamFreq, unsigned &dsdMode, unsigned &curSamRes_DAC, unsigned &audioActive);
extern void check_and_enter_dfu(unsigned curSamFreq, chanend c_aud, server interface i_dfu ?dfuInterface);


// Setup to initial defaults
unsigned command = XUA_AUDCTL_NO_COMMAND;
unsigned xua_wrapper_sample_rate = DEFAULT_FREQ; 							// MIN_FREQ
unsigned xua_wrapper_mclk_rate = DEFAULT_MCLK;   							// Choose correct MCLK for DEFAULT_FREQ
unsigned xua_wrapper_dsd_mode = 0;											// Not in DSD mode, PCM only supported for XUA_WRAPPER
unsigned xua_wrapper_dac_res = STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS;
unsigned xua_wrapper_adc_res = STREAM_FORMAT_INPUT_1_RESOLUTION_BITS;

int XUA_wrapper_exchange_samples(chanend c_aud, int32_t samples_to_host[NUM_USB_CHAN_IN], int32_t samples_from_host[NUM_USB_CHAN_OUT])
{
	StartSampleTransfer(c_aud, underflowWord);                  // Send first token to fire ISR in decouple
	if(NUM_USB_CHAN_IN > 0)
	{
		memcpy(samplesIn[readBuffNo], samples_to_host, sizeof(samples_to_host));
	}
	CompleteSampleTransferUsbChans(c_aud, readBuffNo, command); // Check for command & transfer the samples & UBM
	if(NUM_USB_CHAN_OUT > 0)
	{
		memcpy(samples_from_host, samplesOut, sizeof(samples_from_host));
	}
	if(command != XUA_AUDCTL_NO_COMMAND)
	{
	    /* Just consume the command, grab copies of vars then ignore it & keep on looping forever, unless DFU time */
	    unsigned audioActive = 1;
	    receive_command(command, c_aud, xua_wrapper_sample_rate, xua_wrapper_dsd_mode, xua_wrapper_dac_res, audioActive);
#if XUA_DFU_EN
		check_and_enter_dfu(xua_wrapper_sample_rate, c_aud, null);
#endif

	    /* Calculate what master clock we should be using */
        if (((MCLK_441) % xua_wrapper_sample_rate) == 0)
        {
            xua_wrapper_mclk_rate = MCLK_441;
        }
        else if (((MCLK_48) % xua_wrapper_sample_rate	) == 0)
        {
            xua_wrapper_mclk_rate = MCLK_48;
        }
	    outct(c_aud, XS1_CT_END);

	    return 1;
	}
    return 0;
}

void XUA_wrapper_get_stream_format(unsigned *curSamFreq, unsigned *mClk, unsigned *curSamRes_DAC, unsigned *curSamRes_ADC)
{
	*curSamFreq = xua_wrapper_sample_rate;
	*mClk = xua_wrapper_mclk_rate;
	*curSamRes_DAC = xua_wrapper_dac_res;
	*curSamRes_ADC = xua_wrapper_adc_res;
}

// See also void XUA_wrapper_task(chanend c_aud) in main.xc
