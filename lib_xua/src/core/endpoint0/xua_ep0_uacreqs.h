// Copyright 2014-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef _AUDIOREQUESTS_H_
#define _AUDIOREQUESTS_H_

#include <xccompat.h>

int AudioClassRequests_2(XUD_ep ep0_out, XUD_ep ep0_in, REFERENCE_PARAM(USB_SetupPacket_t, sp), NULLABLE_RESOURCE(chanend, c_audioControl),
    NULLABLE_RESOURCE(chanend, c_mix_ctl), NULLABLE_RESOURCE(chanend, c_clk_ctl));

XUD_Result_t AudioClassRequests_1(XUD_ep ep0_out, XUD_ep ep0_in, REFERENCE_PARAM(USB_SetupPacket_t, sp), NULLABLE_RESOURCE(chanend, c_audioControl),
    NULLABLE_RESOURCE(chanend, c_mix_ctl), NULLABLE_RESOURCE(chanend, c_clk_ctl));

int AudioEndpointRequests_1(XUD_ep ep0_out, XUD_ep ep0_in, REFERENCE_PARAM(USB_SetupPacket_t, sp), NULLABLE_RESOURCE(chanend, c_audioControl),
    NULLABLE_RESOURCE(chanend, c_mix_ctl), NULLABLE_RESOURCE(chanend, c_clk_ctl));


void VendorAudioRequestsInit(chanend c_audioControl, NULLABLE_RESOURCE(chanend, c_mix_ctl), NULLABLE_RESOURCE(chanend, c_clk_ctl));

#endif
