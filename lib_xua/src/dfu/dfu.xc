// Copyright 2011-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"
#if defined(XUA_DFU_EN) && (XUA_DFU_EN == 1)
#include <xs1.h>
#include <platform.h>

#if defined(XUA_USB_EN) && (XUA_USB_EN == 1)
#include "dfu_types.h"
#include "xua_flash_interface.h"
#include "dfu_interface.h"

static int DFU_status = DFU_OK;
static timer DFUTimer;
static int DFU_flash_connected = 0;

static unsigned int subPagesLeft = 0;
static int flash_cmd_start_write_image_in_progress = 1;

extern void DFUCustomFlashEnable();
extern void DFUCustomFlashDisable();

static unsigned int save_blk0_request_data[_DFU_TRANSFER_SIZE_WORDS];

/* Return non-zero on error */
static int DFU_OpenFlash()
{
	if (!DFU_flash_connected)
	{
        // unsigned int cmd_data[_DFU_TRANSFER_SIZE_WORDS];
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
        // unsigned int cmd_data[_DFU_TRANSFER_SIZE_WORDS];
        DFUCustomFlashDisable();
        flash_cmd_deinit();
        DFU_flash_connected = 0;
    }
    return 0;
}

static int DFU_Dnload(unsigned int request_len, unsigned int block_num, const unsigned request_data[_DFU_TRANSFER_SIZE_WORDS], int &return_data_len, unsigned &DFU_state)
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
        return 0;
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
            unsigned int subPagePad[_DFU_TRANSFER_SIZE_WORDS] = {0};
            for (unsigned i = 0; i < subPagesLeft; i++)
            {
                flash_cmd_write_page_data((subPagePad, unsigned char[_DFU_TRANSFER_SIZE_BYTES]));
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
                    for (unsigned i = 0; i < _DFU_TRANSFER_SIZE_WORDS; i++)
                    {
                        save_blk0_request_data[i] = request_data[i]; // save block 0 request data to be written to flash once flash_cmd_start_write_image() is complete
                    }
                    return 0; // return from here. We only write block 0 to flash once flash_cmd_start_write_image() completes.
                    //Further checks for flash_cmd_start_write_image() completion and subsequent writing of block 0 to flash happen in DFU_GetStatus()
                }
            }
        }

        unsigned int cmd_data[_DFU_TRANSFER_SIZE_WORDS];
        for (unsigned i = 0; i < _DFU_TRANSFER_SIZE_WORDS; i++)
        {
            cmd_data[i] = request_data[i];
        }
        flash_cmd_write_page_data((cmd_data, unsigned char[_DFU_TRANSFER_SIZE_BYTES]));
        subPagesLeft--;
    }

    return 0;
}


static int DFU_Upload(unsigned int request_len, unsigned int block_num, unsigned data_out[_DFU_TRANSFER_SIZE_WORDS], unsigned &DFU_state)
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
    flash_cmd_read_page_data((data_out, unsigned char[_DFU_TRANSFER_SIZE_BYTES]));

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
            flash_cmd_write_page_data((save_blk0_request_data, unsigned char[_DFU_TRANSFER_SIZE_BYTES]));
            subPagesLeft--;
            return STATE_DFU_DOWNLOAD_IDLE;
        }
        else // Continue to wait for flash_cmd_start_write_image() to complete
        {
            return STATE_DFU_DOWNLOAD_BUSY;
        }

    }

}

static int DFU_GetStatus(unsigned int request_len, unsigned data_buffer[_DFU_TRANSFER_SIZE_WORDS], unsigned &DFU_state)
{
    unsigned int timeout = 0;

    data_buffer[0] = (timeout << 8) | (unsigned char)DFU_status;

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

static int DFU_GetState(unsigned int request_len, unsigned int request_data[_DFU_TRANSFER_SIZE_WORDS], unsigned &DFU_state)
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

static int XMOS_DFU_RevertFactory()
{
    unsigned s = 0;

    DFU_OpenFlash();

    flash_cmd_erase_all();

    DFUTimer :> s;
    DFUTimer when timerafter(s + 25000000) :> s; // Wait for flash erase

    return 0;
}

static int XMOS_DFU_SelectImage(unsigned int index)
{
    // Select the image index for firmware update
    // Currently not used or implemented
    return 0;
}

[[distributable]]
void DFUHandler(server interface i_dfu i)
{
    while(1)
    {
        select
        {
            case i.HandleDfuRequest(uint16_t request, uint16_t value, uint16_t index, uint16_t length, unsigned data_buffer[], unsigned data_buffer_length, unsigned dfuState)
                -> {unsigned reset_device_after_ack, int return_data_len, int dfu_reset_override, int returnVal, unsigned newDfuState}:

                reset_device_after_ack = 0;
                return_data_len = 0;
                dfu_reset_override = 0;
                unsigned tmpDfuState = dfuState;
                returnVal = 0;
                // Map Standard DFU commands onto device level firmware upgrade mechanism
                switch (request)
                {
                    case DFU_DETACH:
                        if(dfuState == STATE_APP_IDLE)
                        {
                            dfu_reset_override = 1; // Reboot in DFU mode
                        }
                        else
                        {
                            // We expect to come here only in the STATE_DFU_IDLE state but to be safe,
                            // in every state other than APP_IDLE, reboot in APP mode.
                            dfu_reset_override = 0;
                        }
                        reset_device_after_ack = 1;
                        return_data_len = 0;
                        break;

                    case DFU_DNLOAD:
                        unsigned data[_DFU_TRANSFER_SIZE_WORDS];
                        for(int i = 0; i < _DFU_TRANSFER_SIZE_WORDS; i++)
                            data[i] = data_buffer[i];
                        returnVal = DFU_Dnload(length, value, data, return_data_len, tmpDfuState);
                        break;

                    case DFU_UPLOAD:
                        unsigned data_out[_DFU_TRANSFER_SIZE_WORDS];
                        return_data_len = DFU_Upload(length, value, data_out, tmpDfuState);
                        for(int i = 0; i < _DFU_TRANSFER_SIZE_WORDS; i++)
                            data_buffer[i] = data_out[i];
                        break;

                    case DFU_GETSTATUS:
                        unsigned data_out[_DFU_TRANSFER_SIZE_WORDS];
                        return_data_len = DFU_GetStatus(length, data_out, tmpDfuState);
                        for(int i = 0; i < _DFU_TRANSFER_SIZE_WORDS; i++)
                            data_buffer[i] = data_out[i];
                        break;

                    case DFU_CLRSTATUS:
                        return_data_len = DFU_ClrStatus(tmpDfuState);
                        break;

                    case DFU_GETSTATE:
                        unsigned data_out[_DFU_TRANSFER_SIZE_WORDS];
                        return_data_len = DFU_GetState(length, data_out, tmpDfuState);
                        for(int i = 0; i < _DFU_TRANSFER_SIZE_WORDS; i++)
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
                        return_data_len = XMOS_DFU_RevertFactory();
                        break;

                    case XMOS_DFU_RESETINTODFU:
                        reset_device_after_ack = 1;
                        dfu_reset_override = 1;
                        return_data_len = 0;
                        break;

                    case XMOS_DFU_RESETFROMDFU:
                        reset_device_after_ack = 1;
                        dfu_reset_override = 0;
                        return_data_len = 0;
                        break;

                    case XMOS_DFU_SELECTIMAGE:
                        return_data_len = XMOS_DFU_SelectImage(value);
                        break;

                    default:
                        returnVal = 1; // Unrecognised request
                        break;
                }
				newDfuState = tmpDfuState;
                break;

           case i.finish():
                return;
        }
    }
}
#endif /* XUA_USB_EN */

#endif
