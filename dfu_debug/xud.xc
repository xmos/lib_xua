// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <string.h>
#include <assert.h>
#include "xud_device.h"

static unsigned char set_data[64];
static int set_data_len = 0;

static unsigned char get_data[64];
static int get_data_len = 0;

static int set_status = 0;

XUD_Result_t XUD_GetBuffer(XUD_ep ep_out, unsigned char buffer[],
                           unsigned &length)
{
  assert(set_data_len > 0);
  memcpy(buffer, set_data, set_data_len);
  length = set_data_len;
  set_data_len = 0;
  return 0;
}

XUD_Result_t XUD_DoGetRequest(XUD_ep ep_out, XUD_ep ep_in,
                              unsigned char buffer[], unsigned length,
                              unsigned requested)
{
  assert(length <= sizeof(get_data));
  assert(get_data_len == 0);
  memcpy(get_data, buffer, length);
  get_data_len = length;
  return 0;
}

XUD_Result_t XUD_DoSetRequestStatus(XUD_ep ep_in)
{
  assert(set_status == 0);
  set_status = 1;
  return 0;
}

void xud_prepare_set_data(unsigned char data[], int data_len)
{
  assert(data_len <= sizeof(set_data));
  assert(set_data_len == 0);
  memcpy(set_data, data, data_len);
  set_data_len = data_len;
}

int xud_read_set_status(void)
{
  if (set_status) {
    set_status = 0;
    return 1;
  }
  else {
    return 0;
  }
}

void xud_read_get_data(unsigned char data[], int &data_len)
{
  if (get_data_len > 0) {
    memcpy(data, get_data, get_data_len);
    data_len = get_data_len;
    get_data_len = 0;
  }
  else {
    data_len = 0;
  }
}
