// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"
#if (XUA_DFU_EN== 1)
#include <xs1.h>
#include <platform.h>

#if XUA_USB_EN
#include "xud_device.h"
#include "dfu_types.h"
#include "flash_interface.h"
#include "dfu_interface.h"

#if defined(__XS2A__)
/* Note range 0x7FFC8 - 0x7FFFF guarenteed to be untouched by tools */
#define FLAG_ADDRESS 0x7ffcc
#else
/* Note range 0xFFFC8 - 0xFFFFF guarenteed to be untouched by tools */
#define FLAG_ADDRESS 0xfffcc
#endif

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

static int g_DFU_state = STATE_APP_IDLE;
static int DFU_status = DFU_OK;
static timer DFUTimer;
static unsigned int DFUTimerStart = 0;
static unsigned int DFUResetTimeout = 100000000; // 1 second default
static int DFU_flash_connected = 0;

static unsigned int subPagesLeft = 0;

extern void DFUCustomFlashEnable();
extern void DFUCustomFlashDisable();

void DFUDelay(unsigned d)
{
    timer tmr;
    unsigned s;
    tmr :> s;
    tmr when timerafter(s + d) :> void;
}

/* Return non-zero on error */
static int DFU_OpenFlash()
{
	if (!DFU_flash_connected)
	{
    	unsigned int cmd_data[16];
        DFUCustomFlashEnable();
        int error = flash_cmd_init();
        if(error)
        {
            return error;
        }

    	DFU_flash_connected = 1;
  	}

  	return 0;
}

static int DFU_CloseFlash(chanend ?c_user_cmd)
{
    if (DFU_flash_connected)
    {
        unsigned int cmd_data[16];
        DFUCustomFlashDisable();
        flash_cmd_deinit();
        DFU_flash_connected = 0;
    }
    return 0;
}

static int DFU_Detach(unsigned int timeout, chanend ?c_user_cmd, unsigned &DFU_state)
{
    if (DFU_state == STATE_APP_IDLE)
    {
        DFU_state = STATE_APP_DETACH;

        DFU_OpenFlash();

        // Setup DFU timeout value
        DFUResetTimeout = timeout * 100000;

        // Start DFU reset timer
        DFUTimer :> DFUTimerStart;
    }
    else
    {
        DFU_state = STATE_DFU_ERROR;
    }
    return 0;
}

static int DFU_Dnload(unsigned int request_len, unsigned int block_num, const unsigned request_data[16], chanend ?c_user_cmd, int &return_data_len, unsigned &DFU_state)
{
    unsigned int fromDfuIdle = 0;
    return_data_len = 0;
    int error;
    // Get DFU packets here, sequence is
    // DFU_DOWNLOAD -> DFU_DOWNLOAD_SYNC
    // GET_STATUS -> DFU_DOWNLOAD_SYNC (flash busy) || DFU_DOWNLOAD_IDLE
    // REPEAT UNTIL DFU_DOWNLOAD with 0 length -> DFU_MANIFEST_SYNC

    if((error = DFU_OpenFlash()))
    {
        return error;
    }

    switch (DFU_state)
    {
        case STATE_DFU_IDLE:
        case STATE_DFU_DOWNLOAD_IDLE:
            break;
        default:
            DFU_state = STATE_DFU_ERROR;
            return 1;
    }

    if ((DFU_state == STATE_DFU_IDLE) && (request_len == 0))
    {
        DFU_state = STATE_DFU_ERROR;
        return 1;
    }
    else if (DFU_state == STATE_DFU_IDLE)
    {
        fromDfuIdle = 1;
    }
    else
    {
        fromDfuIdle = 0;
    }

    if (request_len == 0)
    {
        // Host signalling complete download
        int i = 0;
        unsigned int cmd_data[16];
        if (subPagesLeft)
        {
            unsigned int subPagePad[16] = {0};
            for (i = 0; i < subPagesLeft; i++)
            {
                flash_cmd_write_page_data((subPagePad, unsigned char[64]));
            }
        }

        cmd_data[0] = 2; // Terminate write
        flash_cmd_write_page((cmd_data, unsigned char[]));

        DFU_state = STATE_DFU_MANIFEST_SYNC;
    }
    else
    {
        unsigned int i = 0;
        unsigned int flash_cmd = 0;
        unsigned int flash_page_index = 0;
        unsigned int cmd_data[16];

        if (fromDfuIdle)
        {
            unsigned s = 0;

            // Erase flash on first block
            flash_cmd_erase_all();
        }

        // Program firmware, STATE_DFU_DOWNLOAD_BUSY not currently used
        if (!(block_num % 4))
        {
            cmd_data[0] = !fromDfuIdle; // 0 for first page, 1 for other pages.
            flash_cmd_write_page((cmd_data, unsigned char[64]));
            subPagesLeft = 4;
        }

        for (i = 0; i < 16; i++)
        {
            cmd_data[i] = request_data[i];
        }

        flash_cmd_write_page_data((cmd_data, unsigned char[64]));
        subPagesLeft--;

        DFU_state = STATE_DFU_DOWNLOAD_SYNC;
    }

    return 0;
}


static int DFU_Upload(unsigned int request_len, unsigned int block_num, unsigned data_out[16], unsigned &DFU_state)
{
    unsigned int cmd_data[1];
    unsigned int firstRead = 0;

    // Start at flash address 0
    // Keep reading flash pages until read_page returns 1 (address out of range)
    // Return terminating upload packet at this point
    DFU_OpenFlash();

    switch (DFU_state)
    {
        case STATE_DFU_IDLE:
        case STATE_DFU_UPLOAD_IDLE:
            break;
        default:
            DFU_state = STATE_DFU_ERROR;
            return 0;
    }

    if ((DFU_state == STATE_DFU_IDLE) && (request_len == 0))
    {
        DFU_state = STATE_DFU_ERROR;
        return 0;
    }
    else if (DFU_state == STATE_DFU_IDLE)
    {
        firstRead = 1;
        subPagesLeft = 0;
    }

    if (!subPagesLeft)
    {
        cmd_data[0] = !firstRead;

        // Read whole (256bytes) page from the image on the flash into a memory buffer
        flash_cmd_read_page((cmd_data, unsigned char[1]));
        subPagesLeft = 4;

        // If address out of range, terminate!
        if (cmd_data[0] == 1)
        {
            subPagesLeft = 0;
            // Back to idle state, upload complete
            DFU_state = STATE_DFU_IDLE;
            return 0;
        }
    }

    // Get 64 bytes of page data from memory
    flash_cmd_read_page_data((data_out, unsigned char[64]));

    subPagesLeft--;

    DFU_state = STATE_DFU_UPLOAD_IDLE;

    return 64;
}

static int DFU_GetStatus(unsigned int request_len, unsigned data_buffer[16], chanend ?c_user_cmd, unsigned &DFU_state)
{
    unsigned int timeout = 0;

    data_buffer[0] = timeout << 8 | (unsigned char)DFU_status;

    switch (DFU_state)
    {
        case STATE_DFU_MANIFEST:
        case STATE_DFU_MANIFEST_WAIT_RESET:
            DFU_state = STATE_DFU_ERROR;
            break;
        case STATE_DFU_DOWNLOAD_BUSY:
            // If download completes -> DFU_DOWNLOAD_SYNC
            // Currently all transactions are synchronous so no busy state
            break;
        case STATE_DFU_DOWNLOAD_SYNC:
            DFU_state = STATE_DFU_DOWNLOAD_IDLE;
            break;
        case STATE_DFU_MANIFEST_SYNC:
            // Check if complete here
            DFU_state = STATE_DFU_IDLE;
            break;
        default:
            break;
    }

    data_buffer[1] = DFU_state;

    return 6;

}

static int DFU_ClrStatus(unsigned &DFU_state)
{
    if (DFU_state == STATE_DFU_ERROR)
    {
        DFU_state = STATE_DFU_IDLE;
    }
    else
    {
        DFU_state = STATE_DFU_ERROR;
    }
    return 0;
}

static int DFU_GetState(unsigned int request_len, unsigned int request_data[16], chanend ?c_user_cmd, unsigned &DFU_state)
{
    request_data[0] = DFU_state;

    switch (DFU_state)
    {
        case STATE_DFU_DOWNLOAD_BUSY:
        case STATE_DFU_MANIFEST:
        case STATE_DFU_MANIFEST_WAIT_RESET:
            DFU_state = STATE_DFU_ERROR;
            break;
        default:
        break;
    }

    return 1;
}

static int DFU_Abort(unsigned &DFU_state)
{
    DFU_state = STATE_DFU_IDLE;
    return 0;
}

// Tell the DFU state machine that a USB reset has occured
int DFUReportResetState(chanend ?c_user_cmd)
{
    unsigned int inDFU = 0;
    unsigned int currentTime = 0;

    unsigned flag;
    flag = GetDFUFlag();

//#define START_IN_DFU 1
#ifdef START_IN_DFU
    flag = 0x11042011;
#endif

    if (flag == 0x11042011)
    {
        unsigned int cmd_data[16];
        inDFU = 1;
        g_DFU_state = STATE_DFU_IDLE;
        return inDFU;
    }

    switch(g_DFU_state)
    {
        case STATE_APP_DETACH:
        case STATE_DFU_IDLE:
            g_DFU_state = STATE_DFU_IDLE;

            DFUTimer :> currentTime;
            if (currentTime - DFUTimerStart > DFUResetTimeout)
            {
                g_DFU_state = STATE_APP_IDLE;
                inDFU = 0;
            }
            else
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
            g_DFU_state = STATE_DFU_ERROR;
            inDFU = 1;
        break;
    }

    if (!inDFU)
    {
        DFU_CloseFlash(c_user_cmd);
    }

    return inDFU;
}

static int XMOS_DFU_RevertFactory(chanend ?c_user_cmd)
{
    unsigned s = 0;

    DFU_OpenFlash();

    flash_cmd_erase_all();

    DFUTimer :> s;
    DFUTimer when timerafter(s + 25000000) :> s; // Wait for flash erase

    return 0;
}

static int XMOS_DFU_SelectImage(unsigned int index, chanend ?c_user_cmd)
{
    // Select the image index for firmware update
    // Currently not used or implemented
    return 0;
}

static int XMOS_DFU_SaveState()
{
    return 0;
}

static int XMOS_DFU_LoadState()
{
    return 0;
}

[[distributable]]
void DFUHandler(server interface i_dfu i, chanend ?c_user_cmd)
{
    while(1)
    {
        select
        {
            case i.HandleDfuRequest(USB_SetupPacket_t &sp, unsigned data_buffer[], unsigned data_buffer_length, unsigned dfuState)
                -> {unsigned reset_device_after_ack, int return_data_len, int dfu_reset_override, int returnVal, unsigned newDfuState}:

                reset_device_after_ack = 0;
                return_data_len = 0;
                dfu_reset_override = 0;
                unsigned tmpDfuState = dfuState;
                returnVal = 0;

                // Map Standard DFU commands onto device level firmware upgrade mechanism
                switch (sp.bRequest)
                {
                    case DFU_DETACH:
                        tmpDfuState = dfuState;
                        return_data_len = DFU_Detach(sp.wValue, c_user_cmd, tmpDfuState);
                        newDfuState = tmpDfuState;;
                        break;

                    case DFU_DNLOAD:
                        unsigned data[16];
                        for(int i = 0; i < 16; i++)
                            data[i] = data_buffer[i];
                        returnVal = DFU_Dnload(sp.wLength, sp.wValue, data, c_user_cmd, return_data_len, tmpDfuState);
                        break;

                    case DFU_UPLOAD:
                        unsigned data_out[16];
                        return_data_len = DFU_Upload(sp.wLength, sp.wValue, data_out, tmpDfuState);
                        for(int i = 0; i < 16; i++)
                            data_buffer[i] = data_out[i];
                        break;

                    case DFU_GETSTATUS:
                        unsigned data_out[16];
                        return_data_len = DFU_GetStatus(sp.wLength, data_out, c_user_cmd, tmpDfuState);
                        for(int i = 0; i < 16; i++)
                            data_buffer[i] = data_out[i];
                        break;

                    case DFU_CLRSTATUS:
                        return_data_len = DFU_ClrStatus(tmpDfuState);
                        break;

                    case DFU_GETSTATE:
                        unsigned data_out[16];
                        return_data_len = DFU_GetState(sp.wLength, data_out, c_user_cmd, tmpDfuState);
                        for(int i = 0; i < 16; i++)
                            data_buffer[i] = data_out[i];
                        break;

                    case DFU_ABORT:
                        return_data_len = DFU_Abort(tmpDfuState);
                        break;

                    /* XMOS Custom DFU requests */
                    case XMOS_DFU_RESETDEVICE:
                        reset_device_after_ack = 1;
                        return_data_len = 0;
                        break;

                    case XMOS_DFU_REVERTFACTORY:
                        return_data_len = XMOS_DFU_RevertFactory(c_user_cmd);
                        break;

                    case XMOS_DFU_RESETINTODFU:
                        reset_device_after_ack = 1;
                        dfu_reset_override = 0x11042011;
                        return_data_len = 0;
                        break;

                    case XMOS_DFU_RESETFROMDFU:
                        reset_device_after_ack = 1;
                        dfu_reset_override = 0;
                        return_data_len = 0;
                        break;

                    case XMOS_DFU_SELECTIMAGE:
                        return_data_len = XMOS_DFU_SelectImage(sp.wValue, c_user_cmd);
                        break;

                    case XMOS_DFU_SAVESTATE:
                        /* Save passed state to flash */
                        return_data_len = XMOS_DFU_SaveState();
                        break;

                    case XMOS_DFU_RESTORESTATE:
                         /* Restore saved state from flash */
                        return_data_len = XMOS_DFU_LoadState();
                        break;

                    default:
                        break;
                }
				newDfuState = tmpDfuState;
                break;

           case i.finish():
                return;
        }
    }
}

int DFUDeviceRequests(XUD_ep ep0_out, XUD_ep &?ep0_in, USB_SetupPacket_t &sp, chanend ?c_user_cmd, unsigned int altInterface, client interface i_dfu i,int &reset)
{
    unsigned int return_data_len = 0;
    unsigned int data_buffer_len = 0;
    unsigned int data_buffer[17];
    unsigned int reset_device_after_ack = 0;
    int returnVal = 0;
    unsigned int dfuState = g_DFU_state;
    int dfuResetOverride;

    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D)
    {
        // Host to device
        if (sp.wLength)
            XUD_GetBuffer(ep0_out, (data_buffer, unsigned char[]), data_buffer_len);
    }

    /* Interface used here such that the handler can be on another tile */
    {reset_device_after_ack, return_data_len, dfuResetOverride, returnVal, dfuState} = i.HandleDfuRequest(sp, data_buffer, data_buffer_len, g_DFU_state);

    SetDFUFlag(dfuResetOverride);

    /* Update our version of dfuState */
    g_DFU_state = dfuState;

    /* Check if the request was handled */
    if(returnVal == 0)
    {
        if (sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_D2H && sp.wLength != 0)
        {
            returnVal = XUD_DoGetRequest(ep0_out, ep0_in, (data_buffer, unsigned char[]), return_data_len, return_data_len);
        }
        else
        {
            returnVal = XUD_DoSetRequestStatus(ep0_in);
        }

  	    // If device reset requested, handle after command acknowledgement
  	    if (reset_device_after_ack)
  	    {
  	        reset = 1;
        }
    }
  	return returnVal;
}
#endif /* XUA_USB_EN */

#endif
