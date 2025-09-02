// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUA_WRAPPER_H_
#define _XUA_WRAPPER_H_


#include <stdint.h>
#include "xccompat.h"

/** USB Audio Wrapper subsystem.
 *
 *  This task starts four threads and provides a complete USB audio subsystem which may be
 *  treated as simple audio source/sink. It starts four logical cores (threads) for XUD, Endpoint 0 and
 *  the two buffering tasks. Basic host audio input/output only is supported; I2S or any other
 *  digital interfaces must be started and managed by the user.
 * 
 *  It is necessary to have a valid xua_conf.h and the user must connect the XUA internally
 *  declared MCLK count port `p_for_mclk_count` to a clock block which is being clocked at 
 *  the MCLK rate.
 * 
 *  Once this task has been started then the user need only call XUA_wrapper_exchange_samples()
 *  at the sample rate (note this must be locked to the master clock). At startup or after a
 *  host initiated sample or stream format change, the function XUA_wrapper_get_stream_format()
 *  may be called to determine the host-selected audio stream format.
 *
 *  \param c_aud                Channel connected to the block which exchanges audio with USB
 */
void XUA_wrapper_task(chanend c_aud);

/** Exchange samples with XUA_wrapper_task()
 *
 *  This function exchanges samples which should be sent or received from USB audio. If this is not
 *  called at the correct rate it will result in overflow or underflow in the USB audio subsystem.
 *
 *  \param c_aud                Channel connected to XUA_wrapper_task()
 *  \param samples_to_host      An array of samples which are to be passed to the host (input)
 *  \param samples_from_host    An array of samples which have been provided by the host (output)
 * 
 *  \returns                    Zero if samples have been exchanged normally, 1 if a stream format
 *                              change has been requested by the host
 */
int XUA_wrapper_exchange_samples(chanend c_aud, int32_t samples_to_host[NUM_USB_CHAN_IN], int32_t samples_from_host[NUM_USB_CHAN_OUT]);

/** Get the latest stream format requested by host to XUA_wrapper_task()
 *
 *  This function provides a way of querying the current host audio streaming settings. This can be
 *  called after XUA_wrapper_exchange_samples() returns 1, indicating a sample rate or sample format change.
 *
 *  \param curSamFreq       Pointer to sample rate in Hertz to be updated
 *  \param mClk             Pointer to Master clock rate in Hertz to be updated
 *  \param curSamRes_DAC    Pointer to output stream bit resolution to be updated
 *  \param curSamRes_ADC    Pointer Input stream bit resolution to be updated
 * 
 */
void XUA_wrapper_get_stream_format(unsigned *curSamFreq, unsigned *mClk, unsigned *curSamRes_DAC, unsigned *curSamRes_ADC);


#endif
