// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _UAC_HWRESOURCES_H_
#define _UAC_HWRESOURCES_H_

/* XUD_U_SERIES, XUD_X200_SERIES */
/* Note, U-series XUD uses clock blocks 4 and 5 - see XUD_Ports.xc */
#define CLKBLK_MIDI        XS1_CLKBLK_REF;
#define CLKBLK_SPDIF_TX    XS1_CLKBLK_1
#define CLKBLK_SPDIF_RX    XS1_CLKBLK_1
#define CLKBLK_MCLK        XS1_CLKBLK_2
#define CLKBLK_FLASHLIB    XS1_CLKBLK_3   /* Clock block for use by flash lib */
#define CLKBLK_ADAT_RX     XS1_CLKBLK_REF /* Use REF for ADAT_RX on U/x200 series */
#define CLKBLK_I2S_BIT     XS1_CLKBLK_3

#endif /* _UAC_HWRESOURCES_H_ */
