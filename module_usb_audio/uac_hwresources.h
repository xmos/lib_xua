
#ifndef _UAC_HWRESOURCES_H_
#define _UAC_HWRESOURCES_H_

#include "xud.h"                 /* XMOS USB Device Layer defines and functions */

#if ((XUD_SERIES_SUPPORT != XUD_U_SERIES) && (XUD_SERIES_SUPPORT != XUD_X200_SERIES))

/* XUD_L_SERIES and XUD_G_SERIES */

#if (AUDIO_IO_TILE == XUD_TILE)
/* Note: L series ref clocked clocked from USB clock when USB enabled - use another clockblock for MIDI
 * if MIDI and XUD on same tile. See XUD documentation.
 *
 * This is a clash with S/PDIF Tx but simultaneous S/PDIF and MIDI not currently supported on single tile device
 *
 */
#define CLKBLK_MIDI        XS1_CLKBLK_1;
#else
#define CLKBLK_MIDI        XS1_CLKBLK_REF;
#endif

#define CLKBLK_SPDIF_TX    XS1_CLKBLK_1
#define CLKBLK_SPDIF_RX    XS1_CLKBLK_1
#define CLKBLK_MCLK        XS1_CLKBLK_2 /* Note, potentially used twice */
#define CLKBLK_ADAT_RX     XS1_CLKBLK_3
#define CLKBLK_USB_RST     XS1_CLKBLK_4 /* Clock block passed into L/G series XUD */
#define CLKBLK_FLASHLIB    XS1_CLKBLK_5 /* Clock block for use by flash lib */

#define CLKBLK_I2S_BIT     XS1_CLKBLK_3

#else

/* XUD_U_SERIES, XUD_X200_SERIES */
/* Note, U-series XUD uses clock blocks 4 and 5 - see XUD_Ports.xc */
#define CLKBLK_MIDI        XS1_CLKBLK_REF;
#define CLKBLK_SPDIF_TX    XS1_CLKBLK_1
#define CLKBLK_SPDIF_RX    XS1_CLKBLK_1
#define CLKBLK_MCLK        XS1_CLKBLK_2   /* Note, potentially used twice */
#define CLKBLK_FLASHLIB    XS1_CLKBLK_3   /* Clock block for use by flash lib */
#define CLKBLK_ADAT_RX     XS1_CLKBLK_REF /* Use REF for ADAT_RX on U/x200 series */
#define CLKBLK_I2S_BIT     XS1_CLKBLK_3
#endif

#endif /* _UAC_HWRESOURCES_H_ */
