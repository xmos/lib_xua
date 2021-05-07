// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <xccompat.h>
#include "xud_device.h"
#include "dfu_interface.h"
#include "dfu_types.h"
#include "xua.h"
#include "xua_dfu.h"
#include "requests.h"

#define DFU_BLOCK 64

void get_status(CLIENT_INTERFACE(i_dfu, i))
{
  printf("+ get_status\n");

  USB_SetupPacket_t sp;

  sp.bmRequestType.Recipient = USB_BM_REQTYPE_RECIP_INTER;
  sp.bmRequestType.Type = USB_BM_REQTYPE_TYPE_CLASS;
  sp.bmRequestType.Direction = USB_BM_REQTYPE_DIRECTION_D2H;
  sp.bRequest = DFU_GETSTATUS;
  sp.wValue = 0;
  sp.wIndex = 0;
  sp.wLength = 1;

  XUD_ep epout = 0, epin = 0;
  int reset = 0;

  DFUDeviceRequests(epout, &epin, &sp, (chanend)NULL, 0, i, &reset);

  int data_len = 0;
  unsigned char data[64];

  xud_read_get_data(data, &data_len);

  printf("- get_data %d state=%d timeout=%d next_state=%d\n",
         data_len, data[0], data[1], data[4]);
}

void download_block(CLIENT_INTERFACE(i_dfu, i), int block_num,
                    unsigned char data[], int data_length)
{
  if (data_length == 0) {
    printf("+ download_block %d 0 NULL\n", block_num);
  }
  else {
    printf("+ download_block %d %d 0x%x 0x%x\n", block_num, data_length,
           data[0], data[data_length - 1]);
  }

  USB_SetupPacket_t sp;

  sp.bmRequestType.Recipient = USB_BM_REQTYPE_RECIP_INTER;
  sp.bmRequestType.Type = USB_BM_REQTYPE_TYPE_CLASS;
  sp.bmRequestType.Direction = USB_BM_REQTYPE_DIRECTION_H2D;
  sp.bRequest = DFU_DNLOAD;
  sp.wValue = block_num;
  sp.wIndex = 0;
  sp.wLength = data_length;

  if (data_length > 0)
    xud_prepare_set_data(data, data_length);

  XUD_ep epout = 0, epin = 0;
  int reset = 0;

  DFUDeviceRequests(epout, &epin, &sp, (chanend)NULL, 0, i, &reset);

  int set_status = xud_read_set_status();

  printf("- set_status %d\n", set_status);
}

void download(CLIENT_INTERFACE(i_dfu, i), const char file_name[])
{
  FILE *f = fopen(file_name, "rb" );
  assert(f != NULL);
  fseek(f, 0, SEEK_END);
  int file_size = (int)ftell(f);
  fseek(f, 0, SEEK_SET);
  int num_blocks = file_size / DFU_BLOCK;
  int remainder = file_size - num_blocks * DFU_BLOCK;
  unsigned char block[DFU_BLOCK];
  int block_count = 0;

  for (int j = 0; j < num_blocks; j++) {
    memset(block, 0, DFU_BLOCK);
    fread(block, 1, DFU_BLOCK, f);
    download_block(i, block_count, block, DFU_BLOCK);
    get_status(i);
    block_count++;
  }

  if (remainder > 0) {
    memset(block, 0, DFU_BLOCK);
    fread(block, 1, remainder, f);
    download_block(i, block_count, block, DFU_BLOCK);
    get_status(i);
  }

  // zero length to terminate
  download_block(i, 0, block, 0);
  get_status(i);
}

void revertfactory(CLIENT_INTERFACE(i_dfu, i))
{
  printf("+ revertfactory\n");

  USB_SetupPacket_t sp;

  sp.bmRequestType.Recipient = USB_BM_REQTYPE_RECIP_INTER;
  sp.bmRequestType.Type = USB_BM_REQTYPE_TYPE_CLASS;
  sp.bmRequestType.Direction = USB_BM_REQTYPE_DIRECTION_H2D;
  sp.bRequest = XMOS_DFU_REVERTFACTORY;
  sp.wValue = 0;
  sp.wIndex = 0;
  sp.wLength = 0;

  XUD_ep epout = 0, epin = 0;
  int reset = 0;

  DFUDeviceRequests(epout, &epin, &sp, (chanend)NULL, 0, i, &reset);

  int set_status = xud_read_set_status();

  printf("- set_status %d\n", set_status);
}

void initialise(void)
{
  DFUReportResetState((chanend)NULL);
}
