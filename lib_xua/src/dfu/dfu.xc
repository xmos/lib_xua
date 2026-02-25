// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"
#if defined(XUA_DFU_EN) && (XUA_DFU_EN == 1)
#include <xs1.h>
#include <platform.h>
#include <string.h>

#if defined(XUA_USB_EN) && (XUA_USB_EN == 1)
#include "dfu_types.h"
#include "xua_flash_interface.h"
#include "dfu_interface.h"
#include "dfu_reboot.h"

static int DFU_status = DFU_OK;
static timer DFUTimer;
static int DFU_flash_connected = 0;

static unsigned int subPagesLeft = 0;
static int flash_cmd_start_write_image_in_progress = 1;

extern void DFUCustomFlashEnable();
extern void DFUCustomFlashDisable();

static unsigned char save_blk0_request_data[DFU_TRANSFER_SIZE_BYTES];

/* Similarly to the delay before reboot to DFU mode, this delay is meant to
 * avoid shocking the Windows software stack. Suggest revisiting to establish
 * if 50 or 500 is needed.
 */
#define DELAY_BEFORE_REBOOT_FROM_DFU_MS   50

/* Return non-zero on error */
static int DFU_OpenFlash()
{
	if (!DFU_flash_connected)
	{
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

static int DFU_CloseFlash()
{
    if (DFU_flash_connected)
    {
        DFUCustomFlashDisable();
        flash_cmd_deinit();
        DFU_flash_connected = 0;
    }
    return 0;
}

static int DFU_Dnload(unsigned int request_len, unsigned int block_num, const unsigned char request_data[DFU_TRANSFER_SIZE_BYTES], int32_t &return_data_len, unsigned &DFU_state)
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
        if (subPagesLeft)
        {
            unsigned char subPagePad[DFU_TRANSFER_SIZE_BYTES] = {0};
            for (unsigned i = 0; i < subPagesLeft; i++)
            {
                flash_cmd_write_page_data(subPagePad);
            }
        }
        flash_cmd_end_write_image();
        DFU_state = STATE_DFU_MANIFEST_SYNC;
    }
    else
    {
        DFU_state = STATE_DFU_DOWNLOAD_SYNC; //from the spec. dfuDNLOAD-SYNC = Device has received a block and is waiting for the host to
        // solicit the status via DFU_GETSTATUS. So if the host were to do a GetState right after this, it should see the device state as STATE_DFU_DOWNLOAD_SYNC.
        // That is why, even when flash_cmd_start_write_image() returns not complete, we don't transition to STATE_DFU_DOWNLOAD_BUSY at this point but do it only
        // from DFU_GetStatus()
        if (!(block_num % _NUM_DFU_PAGES_PER_FLASH_PAGE)) // Every 4th block
        {
            flash_cmd_reset_subpage_index();
            subPagesLeft = _NUM_DFU_PAGES_PER_FLASH_PAGE;
            if (fromDfuIdle) // Only relevant for block 0 which is when fromDfuIdle is also true
            {
                // Erase flash on block 0
                flash_cmd_erase_all();

                flash_cmd_start_write_image_in_progress = flash_cmd_start_write_image();

                if(flash_cmd_start_write_image_in_progress) // flash_cmd_start_write_image() still in progress
                {
                    memcpy(save_blk0_request_data, request_data, DFU_TRANSFER_SIZE_BYTES); // save block 0 request data to be written to flash once flash_cmd_start_write_image() is complete
                    return 0; // return from here. We only write block 0 to flash once flash_cmd_start_write_image() completes.
                    //Further checks for flash_cmd_start_write_image() completion and subsequent writing of block 0 to flash happen in DFU_GetStatus()
                }
            }
        }

        unsigned char cmd_data[DFU_TRANSFER_SIZE_BYTES];
        memcpy(cmd_data, request_data, DFU_TRANSFER_SIZE_BYTES);
        flash_cmd_write_page_data(cmd_data);
        subPagesLeft--;
    }

    return 0;
}


static int DFU_Upload(unsigned int request_len, unsigned int block_num, unsigned char data_out[DFU_TRANSFER_SIZE_BYTES], unsigned &DFU_state)
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
        subPagesLeft = _NUM_DFU_PAGES_PER_FLASH_PAGE;

        // If address out of range, terminate!
        if (cmd_data[0] == 1)
        {
            subPagesLeft = 0;
            // Back to idle state, upload complete
            DFU_state = STATE_DFU_IDLE;
            return 0;
        }
    }

    // Get _DFU_TRANSFER_SIZE_BYTES bytes of page data from memory
    flash_cmd_read_page_data(data_out);

    subPagesLeft--;

    DFU_state = STATE_DFU_UPLOAD_IDLE;

    return _DFU_TRANSFER_SIZE_BYTES;
}

#define GET_STATUS_POLL_TIMEOUT_MS     (400)    // Erasing 512*1024 bytes of flash requires about 26 instances of the device returning STATE_DFU_DOWNLOAD_BUSY
static unsigned transition_dfu_download_state()
{
    if(!flash_cmd_start_write_image_in_progress) // If flash_cmd_start_write_image() is done, transition to IDLE since the actual flash writes (flash_cmd_write_page_data) are synchronous
    {
        return STATE_DFU_DOWNLOAD_IDLE;
    }
    else
    {
        timer tmr;
        unsigned time;
        tmr :> time;
        unsigned end_time = time + (XS1_TIMER_KHZ * GET_STATUS_POLL_TIMEOUT_MS);

        while(timeafter(end_time, time)) // Erase as many sectors as we can in GET_STATUS_POLL_TIMEOUT_MS time duration
        {
            if(!flash_cmd_start_write_image_in_progress)
            {
                break;
            }
            flash_cmd_start_write_image_in_progress = flash_cmd_start_write_image();
            tmr :> time;
        }

        if(!flash_cmd_start_write_image_in_progress)
        {
            // Write block 0 to flash
            flash_cmd_write_page_data(save_blk0_request_data);
            subPagesLeft--;
            return STATE_DFU_DOWNLOAD_IDLE;
        }
        else // Continue to wait for flash_cmd_start_write_image() to complete
        {
            return STATE_DFU_DOWNLOAD_BUSY;
        }

    }

}

static int DFU_GetStatus(unsigned int request_len, unsigned char data_buffer[DFU_TRANSFER_SIZE_BYTES], unsigned &DFU_state)
{
    unsigned int timeout = 0;

    data_buffer[DFU_GETSTATUS_STATUS_INDEX] = (unsigned char)DFU_status;
    data_buffer[DFU_GETSTATUS_POLL_TIMEOUT_INDEX + 0] = (unsigned char)(timeout & 0xff);
    data_buffer[DFU_GETSTATUS_POLL_TIMEOUT_INDEX + 1] = (unsigned char)(timeout >> 8);
    data_buffer[DFU_GETSTATUS_POLL_TIMEOUT_INDEX + 2] = (unsigned char)(timeout >> 16);

    switch (DFU_state)
    {
        case STATE_DFU_MANIFEST:
        case STATE_DFU_MANIFEST_WAIT_RESET:
            DFU_state = STATE_DFU_ERROR;
            break;
        case STATE_DFU_DOWNLOAD_BUSY:
        case STATE_DFU_DOWNLOAD_SYNC:
            DFU_state = transition_dfu_download_state();
            break;
        case STATE_DFU_MANIFEST_SYNC:
            // Check if complete here
            DFU_state = STATE_DFU_IDLE;
            break;
        default:
            break;
    }

    data_buffer[DFU_GETSTATUS_STATE_INDEX] = (unsigned char)DFU_state;

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

static int DFU_GetState(unsigned int request_len, unsigned char request_data[DFU_TRANSFER_SIZE_BYTES], unsigned &DFU_state)
{
    request_data[DFU_GETSTATE_INDEX] = (unsigned char)DFU_state;

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

static int XMOS_DFU_RevertFactory()
{
    unsigned s = 0;

    DFU_OpenFlash();

    flash_cmd_erase_all();

    DFUTimer :> s;
    DFUTimer when timerafter(s + 25000000) :> s; // Wait for flash erase

    return 0;
}

static int m_DFU_state = STATE_APP_IDLE;

[[distributable]]
void DFUHandler(server interface i_dfu i)
{
    while(1)
    {
        select
        {
            case i.HandleDfuRequest(struct dfu_request_params request, unsigned data_buffer[], unsigned data_buffer_length)
                -> struct dfu_cmd_response dfu:

                unsigned char data_local[DFU_TRANSFER_SIZE_BYTES];

                dfu.deferred_request = 0;
                dfu.return_data_len = 0;
                unsigned tmpDfuState = m_DFU_state;
                dfu.status = DFU_API_SUCCESS;
                // Map Standard DFU commands onto device level firmware upgrade mechanism
                switch (request.request)
                {
                    case DFU_DETACH:
                        if(tmpDfuState == STATE_APP_IDLE)
                        {
                            dfu.deferred_request = DFU_DEFERRED_ACTION_REBOOT_TO_DFU;
                        }
                        else
                        {
                            // TODO - ignore detach request if not in app idle state.

                            // We expect to come here only in the STATE_DFU_IDLE state but to be safe,
                            // in every state other than APP_IDLE, reboot in APP mode.
                            dfu.deferred_request = DFU_DEFERRED_ACTION_REBOOT;
                        }
                        dfu.return_data_len = 0;
                        break;

                    case DFU_DNLOAD:
                        memcpy(data_local, data_buffer, DFU_TRANSFER_SIZE_BYTES);
                        dfu.status = DFU_Dnload(request.length, request.value, data_local, dfu.return_data_len, tmpDfuState);
                        break;

                    case DFU_UPLOAD:
                        dfu.return_data_len = DFU_Upload(request.length, request.value, data_local, tmpDfuState);
                        memcpy(data_buffer, data_local, DFU_TRANSFER_SIZE_BYTES);
                        break;

                    case DFU_GETSTATUS:
                        dfu.return_data_len = DFU_GetStatus(request.length, data_local, tmpDfuState);
                        memcpy(data_buffer, data_local, DFU_TRANSFER_SIZE_BYTES);
                        break;

                    case DFU_CLRSTATUS:
                        dfu.return_data_len = DFU_ClrStatus(tmpDfuState);
                        break;

                    case DFU_GETSTATE:
                        dfu.return_data_len = DFU_GetState(request.length, data_local, tmpDfuState);
                        memcpy(data_buffer, data_local, DFU_TRANSFER_SIZE_BYTES);
                        break;

                    case DFU_ABORT:
                        dfu.return_data_len = DFU_Abort(tmpDfuState);
                        break;

                    case XMOS_DFU_REVERTFACTORY:
                        dfu.return_data_len = XMOS_DFU_RevertFactory();
                        break;

                    case XMOS_DFU_BUS_RESET:
                        // value is 1 when bus reset is entering DFU mode and 0 when bus reset is from APP mode
                        if (request.value)
                        {
                            tmpDfuState = STATE_DFU_IDLE;
                        }
                        else
                        {
                            DFU_CloseFlash();
                            if (tmpDfuState != STATE_APP_IDLE)
                            {
                                // When host triggers a bus reset from DFU mode, transition to APP_IDLE.
                                tmpDfuState = STATE_APP_IDLE;
                                /* Send reboot command */
                                timer tmr;
                                unsigned now;
                                tmr :> now;
                                tmr when timerafter(now + (DELAY_BEFORE_REBOOT_FROM_DFU_MS * XS1_TIMER_KHZ)) :> void;
                                device_reboot();
                            }
                            tmpDfuState = STATE_APP_IDLE;
                        }
                        // Non-zero return value means DFU mode.
                        dfu.status = request.value;
                        break;

                    default:
                        dfu.status = DFU_API_ERROR; // Unrecognised request
                        break;
                }
				m_DFU_state = tmpDfuState;
                break;

           case i.finish():
                return;
        }
    }
}
#endif /* XUA_USB_EN */

#endif
