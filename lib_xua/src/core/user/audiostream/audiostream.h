// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _AUDIOSTREAM_H_
#define _AUDIOSTREAM_H_

/* Functions that handle functions that must occur on stream start/stop e.g. DAC mute/un-mute
 *
 * THESE NEED IMPLEMENTING FOR A SPECIFIC DESIGN
 *
 * */

/* Any actions required for stream start e.g. DAC un-mute - run every stream start */
void UserAudioStreamStart(void);

/* Any actions required on stream stop e.g. DAC mute - run every steam stop  */
void UserAudioStreamStop(void);

/* Any actions required on input stream start */
void UserAudioInputStreamStart(void);

/* Any actions required on input stream stop */
void UserAudioInputStreamStop(void);

/* Any actions required on output stream start */
void UserAudioOutputStreamStart(void);

/* Any actions required on output stream stop */
void UserAudioOutputStreamStop(void);

#endif

