// Copyright 2013-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _DSD_H_
#define _DSD_H_

#define DSD_MODE_OFF          0
#define DSD_MODE_DOP          1
#define DSD_MODE_NATIVE       2

/* DoP defines */
#define DSD_MARKER_1          0xFA
#define DSD_MARKER_2          0x05
#define DSD_MARKER_XOR        0xFF
#define DSD_EN_THRESH         32          /* Number of consecutive DSD markers before switching to DSD mode */
#define DSD_MASK(x)           ((x >> 24) & 0xff)

#endif
