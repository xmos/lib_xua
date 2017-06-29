#ifndef NO_USB

#include "xud.h"
#include "vendorrequests.h"

int VendorAudioRequests(XUD_ep ep0_out, XUD_ep ep0_in, unsigned char bRequest, unsigned char cs, unsigned char cn,
    unsigned short unitId, unsigned char direction, chanend c_audioControl,
    NULLABLE_RESOURCE(chanend, c_mix_ctl),
    NULLABLE_RESOURCE(chanend, c_clk_ctL)) __attribute__ ((weak));

int VendorAudioRequests(XUD_ep ep0_out, XUD_ep ep0_in, unsigned char bRequest, unsigned char cs, unsigned char cn,
    unsigned short unitId, unsigned char direction, chanend c_audioControl,
    NULLABLE_RESOURCE(chanend, c_mix_ctl),
    NULLABLE_RESOURCE(chanend, c_clk_ctL))
{

    return XUD_RES_ERR;
}

int VendorRequests(XUD_ep ep0_out, XUD_ep ep0_in,  REFERENCE_PARAM(USB_SetupPacket_t, sp) VENDOR_REQUESTS_PARAMS_DEC_) __attribute__ ((weak));

int VendorRequests(XUD_ep ep0_out, XUD_ep ep0_in,  REFERENCE_PARAM(USB_SetupPacket_t, sp) VENDOR_REQUESTS_PARAMS_DEC_)
{
    return XUD_RES_ERR;
}

void VendorRequests_Init(VENDOR_REQUESTS_PARAMS_DEC) __attribute__ ((weak));

void VendorRequests_Init(VENDOR_REQUESTS_PARAMS_DEC)
{

}

#endif /* NO_USB */
