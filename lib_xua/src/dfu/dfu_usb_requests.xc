// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "dfu_usb_requests.h"

#include "xua.h"
#if defined(XUA_USB_EN) && XUA_USB_EN

#include <timer.h>
#include <xs1.h>
#include <xassert.h>
#include <xccompat.h>

#include "descriptor_defs.h"
#include "xud_device.h"
#include "dfu_types.h"

#if defined(__XS2A__)
/* Note range 0x7FFC8 - 0x7FFFF guaranteed to be untouched by tools */
#define FLAG_ADDRESS 0x7ffcc
#elif defined(__XS3A__)
/* Note range 0xFFFC8 - 0xFFFFF guaranteed to be untouched by tools */
#define FLAG_ADDRESS 0xfffcc
#else
#error DFU code requires __XS2A__ or __XS3A__ to be defined
#endif

#define _BOOT_DFU_MODE_FLAG (0x11042011)

/* Store Flag to fixed address */
static void SetDFUFlag(unsigned x)
{
    asm volatile("stw %0, %1[0]" :: "r"(x), "r"(FLAG_ADDRESS));
}

/* Load flag from fixed address */
static unsigned GetDFUFlag()
{
    unsigned x;
    asm volatile("ldw %0, %1[0]" : "=r"(x) : "r"(FLAG_ADDRESS));
    return x;
}

#define USB_BMREQ_H2D_VENDOR_INT    ((USB_BM_REQTYPE_DIRECTION_H2D << 7) | \
                                    (USB_BM_REQTYPE_TYPE_VENDOR << 5) | \
                                    (USB_BM_REQTYPE_RECIP_INTER))

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

static int DFU_mode_active = 0;
static int g_DFU_state = STATE_APP_IDLE;

int DFUModeIsActive(void)
{
    return DFU_mode_active;
}

void DFUSetModeActive()
{
    DFU_mode_active = 1;
}

void DFUSetModeInactive()
{
    DFU_mode_active = 0;
}

void DFUDelay(unsigned d)
{
    timer tmr;
    unsigned s;
    tmr :> s;
    tmr when timerafter(s + d) :> void;
}

// Tell the DFU state machine that a USB reset has occurred
/* USB bus reset
 * Input: USB bus reset event.
 * Output: 1 if in DFU mode, 0 if not in DFU mode
 * State handling: On USB bus reset signalling
 * - APP_IDLE on normal boot with "valid" firmware (only ever valid).
 * - DFU_IDLE after request to reset from application to DFU mode.
 * 
 * Note: DFU_IDLE on normal boot with "invalid" firmware not possible with flashed firmware.
 */
int DFUReportResetState()
{
    unsigned int inDFU = 0;
    // unsigned int currentTime = 0;

    unsigned flag;
    flag = GetDFUFlag();

//#define START_IN_DFU 1
#ifdef START_IN_DFU
    flag = _BOOT_DFU_MODE_FLAG;
#endif

    if (flag == _BOOT_DFU_MODE_FLAG)
    {
        inDFU = 1;
        g_DFU_state = STATE_DFU_IDLE;
        return inDFU;
    }

    switch(g_DFU_state)
    {
        case STATE_APP_DETACH:
        case STATE_DFU_IDLE:
            g_DFU_state = STATE_DFU_IDLE;

            // DFUTimer :> currentTime;
            // if (currentTime - DFUTimerStart > DFUResetTimeout)
            // {
            //     g_DFU_state = STATE_APP_IDLE;
            //     inDFU = 0;
            // }
            // else
            {
                inDFU = 1;
            }
            break;
        case STATE_APP_IDLE:
        case STATE_DFU_DOWNLOAD_SYNC:
        case STATE_DFU_DOWNLOAD_BUSY:
        case STATE_DFU_DOWNLOAD_IDLE:
        case STATE_DFU_MANIFEST_SYNC:
        case STATE_DFU_MANIFEST:
        case STATE_DFU_MANIFEST_WAIT_RESET:
        case STATE_DFU_UPLOAD_IDLE:
        case STATE_DFU_ERROR:
            inDFU = 0;
            g_DFU_state = STATE_APP_IDLE;
            break;
        default:
            // Not a valid state transition - to error - on USB bus reset event.
            // On USB bus reset event the only valid states are APP_IDLE or DFU_IDLE.
            g_DFU_state = STATE_DFU_ERROR;
            inDFU = 1;
        break;
    }

    if (!inDFU)
    {
        // TODO - ARGH! - this is possibly called from tile1...
        // DFU_CloseFlash();
    }

    return inDFU;
}

static int DFUDeviceRequests(XUD_ep ep0_out, XUD_ep &?ep0_in, USB_SetupPacket_t &sp, unsigned int altInterface, client interface i_dfu i,int &reset)
{
    unsigned int data_buffer_len = 0;
    unsigned int data_buffer[17];

    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D)
    {
        // Host to device
        if (sp.wLength)
            XUD_GetBuffer(ep0_out, (data_buffer, unsigned char[]), data_buffer_len);
    }
    /* Swap to dfu_request_params to avoid having USB SP struct as dependency of DFU. */
    struct dfu_request_params request;
    request.request = sp.bRequest;
    request.value = sp.wValue;
    request.index = sp.wIndex;
    request.length = sp.wLength;
    request.dfuState = g_DFU_state;
    /* Interface used here such that the handler can be on another tile */
    struct dfu_request_result result = i.HandleDfuRequest(request, data_buffer, data_buffer_len);

    if (result.reset_type == DFU_RESET_TYPE_RESET_TO_DFU) {
        SetDFUFlag(_BOOT_DFU_MODE_FLAG);
    } else {
        SetDFUFlag(0);
    }

    /* Update our version of dfuState */
    g_DFU_state = result.dfuState;

    int returnVal = 0;
    /* Check if the request was handled */
    if(result.return_code == 0)
    {
        if (sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_D2H && sp.wLength != 0)
        {
            returnVal = XUD_DoGetRequest(ep0_out, ep0_in, (data_buffer, unsigned char[]), result.return_data_len, result.return_data_len);
        }
        else
        {
            returnVal = XUD_DoSetRequestStatus(ep0_in);
        }

  	    // If device reset requested, handle after command acknowledgement
  	    if (result.reset_type != DFU_RESET_TYPE_NONE)
  	    {
  	        reset = 1;
        }
    } else {
        returnVal = XUD_RES_ERR;
    }
  	return returnVal;
}

int dfu_usb_vendor_requests(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, client interface i_dfu dfuInterface, unsigned int xua_dfu_interface_num) {
    int result = XUD_RES_ERR;
    // From Theycon DFU driver v2.66.0 onwards, the XMOS_DFU_REVERTFACTORY request comes as a H2D vendor request and not as a class request addressed to the DFU interface
    unsigned bmRequestType = (sp.bmRequestType.Direction << 7) | (sp.bmRequestType.Type << 5) | (sp.bmRequestType.Recipient);
    if((bmRequestType == USB_BMREQ_H2D_VENDOR_INT) && (sp.bRequest == XMOS_DFU_REVERTFACTORY))
    {
        unsigned interface_num = sp.wIndex & 0xff;
        unsigned dfu_if = (DFU_mode_active) ? 0 : xua_dfu_interface_num;

        if(interface_num == dfu_if)
        {
            int reset = 0;
            result = DFUDeviceRequests(ep0_out, ep0_in, sp, 0 /* alternate is unused in DFUDeviceRequests()??*/, dfuInterface, reset);
            if(reset)
            {
                DFUDelay(DELAY_BEFORE_REBOOT_TO_DFU_MS * XS1_TIMER_KHZ);
                device_reboot();
            }
        }
    }
    return result;
}

int dfu_usb_class_int_requests(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, client interface i_dfu dfuInterface, NULLABLE_RESOURCE(chanend, c_aud_ctl), unsigned int xua_dfu_interface_num) {
    int result = XUD_RES_ERR;

    unsigned interfaceNum = sp.wIndex & 0xff;
    /* DFU interface number changes based on which mode we are currently running in */
    unsigned dfu_if = (DFU_mode_active) ? 0 : xua_dfu_interface_num;

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
        result = DFUDeviceRequests(ep0_out, ep0_in, sp, 0, dfuInterface, reset);

        if(reset)
        {
            DFUDelay(DELAY_BEFORE_REBOOT_TO_DFU_MS * XS1_TIMER_KHZ);
            device_reboot();
        }
    }
    return result;
}

#endif /* XUA_USB_EN */
