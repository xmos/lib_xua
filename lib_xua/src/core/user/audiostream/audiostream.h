// Copyright 2011-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _AUDIOSTREAM_H_
#define _AUDIOSTREAM_H_

/* Functions that handle functionality that occur on stream start/stop e.g. DAC mute/un-mute.
 * They should be implemented for the external audio hardware arrangement of a specific design.
 */

/**
 * @brief   User stream start code
 *
 * User code to perform any actions required at every stream start - either input or output
 */
void UserAudioStreamStart(void);

/**
 * @brief   User stream stop code
 *
 * User code to perform any actions required on every stream stop - either input or output*/
void UserAudioStreamStop(void);

/**
 * @brief   User input stream stop code
 *
 * User code to perform any actions required on input stream start i.e. device to host
 */
void UserAudioInputStreamStart(void);

/**
 * @brief   User input stream stop code
 *
 * User code to perform any actions required on input stream stop i.e. device to host
 */
void UserAudioInputStreamStop(void);

/**
 * @brief   User output stream start code
 *
 * User code to perform any actions required on output stream start i.e. host to device
 */
void UserAudioOutputStreamStart(void);

/**
 * @brief   User output stream stop code
 *
 * User code to perfrom any actions required on output stream stop i.e. host to device
 */
void UserAudioOutputStreamStop(void);

#endif

