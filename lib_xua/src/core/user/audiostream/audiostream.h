// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _AUDIOSTREAM_H_
#define _AUDIOSTREAM_H_

/* Functions that handle functionality that occur on stream start/stop e.g. DAC mute/un-mute.
 * They should be implemented for the external audio hardware arrangement of a specific design.

 * Note that these are called from the EP0 code which always resides on XUA_XUD_TILE_NUM, i.e. the
 * tile where the USB device code is executed.
 */

/**
 * @brief   User stream start code
 *
 * User code to perform any actions required at every stream start - either input or output.
 *
 * /param inputActive	An input stream is active if 1, else inactive if 0
 * /param OutputActive	An output stream is active if 1, else inactive if 0
 */

void UserAudioStreamState(int inputActive, int outputActive);

#endif

