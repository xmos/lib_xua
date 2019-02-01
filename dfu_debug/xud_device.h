// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __xud_device_h__
#define __xud_device_h__

#include <xccompat.h>

#define USB_BM_REQTYPE_DIRECTION_H2D    0
#define USB_BM_REQTYPE_DIRECTION_D2H    1

#define USB_BM_REQTYPE_TYPE_STANDARD    0
#define USB_BM_REQTYPE_TYPE_CLASS       1
#define USB_BM_REQTYPE_TYPE_VENDOR      2

#define USB_BM_REQTYPE_RECIP_DEV        0
#define USB_BM_REQTYPE_RECIP_INTER      1
#define USB_BM_REQTYPE_RECIP_EP         2
#define USB_BM_REQTYPE_RECIP_OTHER      3

#define USB_DESCTYPE_CONFIGURATION 2

#define DFU_PRODUCT_STR_INDEX 0
#define DFU_MANUFACTURER_STR_INDEX 0

typedef struct {
  unsigned char Recipient;
  unsigned char Type;
  unsigned char Direction;
} USB_BmRequestType_t;

typedef struct {
  USB_BmRequestType_t bmRequestType;
  unsigned char bRequest;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
} USB_SetupPacket_t;

typedef int XUD_ep;
typedef int XUD_Result_t;

XUD_Result_t XUD_GetBuffer(XUD_ep ep_out, unsigned char buffer[],
                           REFERENCE_PARAM(unsigned, length));

XUD_Result_t XUD_DoGetRequest(XUD_ep ep_out, XUD_ep ep_in,
                              unsigned char buffer[], unsigned length,
                              unsigned requested);

XUD_Result_t XUD_DoSetRequestStatus(XUD_ep ep_in);

void xud_prepare_set_data(unsigned char data[], int data_len);
int xud_read_set_status(void);
void xud_read_get_data(unsigned char data[], REFERENCE_PARAM(int, data_len));

#endif
