// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"
#if XUA_USB_EN
#include "xud.h"

extern XUD_BusSpeed_t g_curUsbSpeed;

/* Returns the max and min packet sizes to send back to host for a given sample frequency
 * See page 13 of USB Audio Device Class Definitions for Audio Data Formats Spec (v2.0)
 *
 * Audio samples per frame = INT(sampFreq/frametime); Variation allowed is + 1;
 *
 * For HS frame time = 8 * 1000
 *
 * so n = INT(SampFreq/8000) | INT (SampFreq/8000) + 1
 *
 * In the case where INT(SampFreq/8000) == SampFreq/8000) n may vary between
 *
 * INT(SamFreq/8000) - 1 | INT(SampFreq/8000) | INT (SampFreq/8000) + 1
 *
 * Note: Assumes HS (i.e. 8 frames per 1ms)
 *
 * Examples:
 * 44100:  min: 5  max: 6
 * 48000:  min: 5  max: 7
 * 96000:  min: 11 max: 13
 * 88200:  min: 11 max: 12
 * 176400: min: 22 max: 23
 * 192000: min: 23 max: 25
 *
 */
void GetADCCounts(unsigned samFreq, int *min, int *mid, int *max)
{
    unsigned frameTime;
    XUD_BusSpeed_t usb_speed;
    usb_speed = g_curUsbSpeed;
    if (usb_speed == XUD_SPEED_HS)
      frameTime = 8000;
    else
      frameTime = 1000;

    *min = samFreq / frameTime;
    *max = *min + 1;

    *mid = *min;

    /* Check for INT(SampFreq/8000) == SampFreq/8000 */
    if((samFreq % frameTime) == 0)
    {
        *min -= 1;
    }

}
#endif /* XUA_USB_EN */

