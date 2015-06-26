
#ifndef __DFU_INTERFACE_H__
#define __DFU_INTERFACE_H__

#include "usb_std_requests.h"

interface i_dfu
{
    {unsigned, int, int, int, unsigned} HandleDfuRequest(USB_SetupPacket_t &sp, unsigned data_buffer[], unsigned data_buffer_length, unsigned dfuState);
    void finish();
};
#endif


