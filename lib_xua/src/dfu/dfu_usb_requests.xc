// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "dfu_usb_requests.h"

#include "xua.h"
#if defined(XUA_USB_EN) && XUA_USB_EN

#include <xs1.h>
#include <xassert.h>
#include <xccompat.h>

#include "descriptor_defs.h"
// #include "dfu_types.h"
#include "xud_device.h"
#include "xua_dfu_api.h"

// TODO Move to lib_xud
#define USB_BMREQ_H2D_VENDOR_INT          ((USB_BM_REQTYPE_DIRECTION_H2D << 7) | \
                                            (USB_BM_REQTYPE_TYPE_VENDOR << 5) | \
                                            (USB_BM_REQTYPE_RECIP_INTER))

// TODO - fix - circular 
extern void device_reboot(void);

/* Windows core USB/device driver stack may not like device coming off bus for
 * a very short period of less than 500ms. Enforce at least 500ms by stalling.
 * This may not have the desired effect depending on whether 'off the bus'
 * requires device terminations disabled (PHY off). In that case we would be
 * better off doing the reboot to DFU and then delaying PHY initialisation
 * instead. Suggest revisiting.
 */
#define DELAY_BEFORE_REBOOT_TO_DFU_MS     500

/* Similarly to the delay before reboot to DFU mode, this delay is meant to
 * avoid shocking the Windows software stack. Suggest revisiting to establish
 * if 50 or 500 is needed.
 */
#define DELAY_BEFORE_REBOOT_FROM_DFU_MS   50

// static int DFU_mode_active = 0;

int dfu_usb_vendor_requests(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, client interface i_dfu dfuInterface, int DFU_mode_active) {
    int result = XUD_RES_ERR;
    // From Theycon DFU driver v2.66.0 onwards, the XMOS_DFU_REVERTFACTORY request comes as a H2D vendor request and not as a class request addressed to the DFU interface
    unsigned bmRequestType = (sp.bmRequestType.Direction<<7) | (sp.bmRequestType.Type<<5) | (sp.bmRequestType.Recipient);
    if((bmRequestType == USB_BMREQ_H2D_VENDOR_INT) && (sp.bRequest == XMOS_DFU_REVERTFACTORY))
    {
        unsigned interface_num = sp.wIndex & 0xff;
        unsigned dfu_if = (DFU_mode_active) ? 0 : INTERFACE_NUMBER_DFU;

        if(interface_num == dfu_if)
        {
            int reset = 0;
            result = DFUDeviceRequests(ep0_out, ep0_in, sp, null, 0 /*this is unused in DFUDeviceRequests()??*/, dfuInterface, reset);
            if(reset)
            {
                DFUDelay(DELAY_BEFORE_REBOOT_TO_DFU_MS * 100000);
                device_reboot();
            }
        }
    }
    return result;
}

int dfu_usb_class_int_requests(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, client interface i_dfu dfuInterface, NULLABLE_RESOURCE(chanend, c_aud_ctl), int DFU_mode_active) {
    int result = XUD_RES_ERR;

    unsigned interfaceNum = sp.wIndex & 0xff;
    /* DFU interface number changes based on which mode we are currently running in */
    unsigned dfu_if = (DFU_mode_active) ? 0 : INTERFACE_NUMBER_DFU;

    if (interfaceNum == dfu_if)
    {
        /* If running in application mode stop audio */
        /* Don't interrupt audio for save and restore cmds */
        static unsigned int notify_audio_stop_for_DFU = 0;
        if (!DFU_mode_active && !notify_audio_stop_for_DFU)
        {
            DFUNotifyEntryCallback(c_aud_ctl);
            notify_audio_stop_for_DFU = 1;  // So we notify AUDIO_STOP_FOR_DFU only once
        }

        /* Reset will be set to 1 if reboot requested */
        // TODO - do we need to support alternative interface for DFU?
        // result = DFUDeviceRequests(ep0_out, &ep0_in, &sp, null, g_interfaceAlt[sp.wIndex], dfuInterface, &reset);
        int reset = 0;
        result = DFUDeviceRequests(ep0_out, ep0_in, sp, null, 0, dfuInterface, reset);

        if(reset)
        {
            DFUDelay(DELAY_BEFORE_REBOOT_TO_DFU_MS * 100000);
            device_reboot();
        }
    }
    return result;
}

#endif /* XUA_USB_EN */
