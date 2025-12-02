// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _XUD_CONF_H_
#define _XUD_CONF_H_

#include "xua_conf_full.h"
#include "packet_sizes.h"

/* Link lib_xua's XUA_XUD_TILE_NUM to lib_xud's USB_TILE */
#define USB_TILE tile[XUA_XUD_TILE_NUM]

/* Update XUD_USB_ISO_EP_MAX_TXN_SIZE if HiBW enabled */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define ROUND_UP_TO_MULTIPLE_OF_4(x)  (((x) + 3) & ~0x3)
#define DIVIDE_AND_ROUND_UP(x, n)       (((x) + (n-1)) / (n))

/* If DFU is enabled, assign to "...GUID_CONTROL" to assign GUID to `simple` MSOS descriptors */
#if XUA_DFU_EN
#define XUD_WINUSB_DEVICE_INTERFACE_GUID_CONTROL    XUA_WINUSB_DEVICE_INTERFACE_GUID_DFU
#else
#define XUD_WINUSB_DEVICE_INTERFACE_GUID_CONTROL    XUA_WINUSB_DEVICE_INTERFACE_GUID_CONTROL
#endif

#define XUD_REQUEST_GET_MSOS_DESCRIPTOR             XUA_REQUEST_GET_MSOS_DESCRIPTOR

#if (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME > 1) // HiBW enabled
// For HiBW enabled, recalculate XUD_USB_ISO_EP_MAX_TXN_SIZE to uniformly divide MAX_HS_STREAM_PACKETSIZE across transactions
#define MAX_HS_STREAM_PACKETSIZE XUA_MAX(MAX_PACKETSIZE_OUT_HS, MAX_PACKETSIZE_IN_HS)

#if (MAX_HS_STREAM_PACKETSIZE > (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME * 1024))
    /* If the max transfer size cannot be supported in XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME * 1024, flag error */
    #error "MAX_HS_STREAM_PACKETSIZE > (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME * 1024). Compile with a lower MAX_FREQ or reduce the number of USB channels."

#elif MAX_HS_STREAM_PACKETSIZE > (1024)
    /* If max transfer size is bigger than the 1024, we need to split transfer into multiple transactions.
    Redefine XUD_USB_ISO_EP_MAX_TXN_SIZE to split transfer into uniformly sized transactions*/

    #undef XUD_USB_ISO_EP_MAX_TXN_SIZE
    #define XUD_USB_ISO_EP_MAX_TXN_SIZE   (ROUND_UP_TO_MULTIPLE_OF_4(DIVIDE_AND_ROUND_UP((MAX_HS_STREAM_PACKETSIZE), (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME))))
    /* Note: Rounding XUD_USB_ISO_EP_MAX_TXN_SIZE to be a multiple of 4 to ensure that the buffer start for the 2nd txn in the transfer is at a word aligned address */

#endif // #if (MAX_HS_STREAM_PACKETSIZE > (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME * 1024))
#endif // #if (XUD_USB_ISO_MAX_TXNS_PER_MICROFRAME > 1)

#endif // #ifndef _XUD_CONF_H_
