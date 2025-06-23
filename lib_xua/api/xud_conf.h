// Copyright 2017-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _XUD_CONF_H_
#define _XUD_CONF_H_

#include "packet_sizes.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)


#define ROUND_UP_TO_MULTIPLE_OF_4(x)  (((x) + 3) & ~0x3)
#define DIVIDE_AND_ROUND_UP(x, n)       (((x) + (n-1)) / (n))


#if (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME > 1) // HiBW enabled
// For HiBW enabled, recalculate XUD_USB_ISO_EP_MAX_TXN_SIZE to uniformly divide MAX_HS_STREAM_PACKETSIZE across transactions
#define MAX_HS_STREAM_PACKETSIZE MAX(MAX_PACKETSIZE_OUT_HS, MAX_PACKETSIZE_IN_HS)

#if (MAX_HS_STREAM_PACKETSIZE > (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME * 1024))
    /* If the max transfer size cannot be supported in XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME * 1024, flag error */
    #error "MAX_HS_STREAM_PACKETSIZE > (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME * 1024). Compile with a lower MAX_FREQ or reduce the number of USB channels."

#elif MAX_HS_STREAM_PACKETSIZE > (1024)
    /* If max transfer size is bigger than the 1024, we need to split transfer into multiple transactions.
    Redefine XUD_USB_ISO_EP_MAX_TXN_SIZE to split transfer into uniformly sized transactions*/

    #undef XUD_USB_ISO_EP_MAX_TXN_SIZE
    #define XUD_USB_ISO_EP_MAX_TXN_SIZE   (ROUND_UP_TO_MULTIPLE_OF_4(DIVIDE_AND_ROUND_UP((MAX_HS_STREAM_PACKETSIZE), (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME))))

#endif


#endif

#endif
