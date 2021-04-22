// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
// Default Command requests (from Spec)
#define DFU_DETACH 0
#define DFU_DNLOAD 1
#define DFU_UPLOAD 2
#define DFU_GETSTATUS 3
#define DFU_CLRSTATUS 4
#define DFU_GETSTATE 5
#define DFU_ABORT 6

// XMOS Alternate Setting Command Requests
#define XMOS_DFU_RESETDEVICE   0xf0
#define XMOS_DFU_REVERTFACTORY 0xf1
#define XMOS_DFU_RESETINTODFU  0xf2
#define XMOS_DFU_RESETFROMDFU  0xf3
#define XMOS_DFU_SELECTIMAGE   0xf4
#define XMOS_DFU_SAVESTATE     0xf5
#define XMOS_DFU_RESTORESTATE  0xf6

// DFU States
#define STATE_APP_IDLE                  0x00
#define STATE_APP_DETACH                0x01
#define STATE_DFU_IDLE                  0x02
#define STATE_DFU_DOWNLOAD_SYNC         0x03
#define STATE_DFU_DOWNLOAD_BUSY         0x04
#define STATE_DFU_DOWNLOAD_IDLE         0x05
#define STATE_DFU_MANIFEST_SYNC         0x06
#define STATE_DFU_MANIFEST              0x07
#define STATE_DFU_MANIFEST_WAIT_RESET   0x08
#define STATE_DFU_UPLOAD_IDLE           0x09
#define STATE_DFU_ERROR                 0x0a

// DFU error conditions
#define DFU_OK              0x00 // No error condition is present.
#define DFU_errTARGET       0x01 // File is not targeted for use by this device.
#define DFU_errFILE         0x02 // File is for this device but fails some vendor-specific verification test.
#define DFU_errWRITE        0x03 // Device is unable to write memory.
#define DFU_errERASE        0x04 // Memory erase function failed.
#define DFU_errCHECK_ERASED 0x05 // Memory erase check failed.
#define DFU_errPROG         0x06 // Program memory function failed.
#define DFU_errVERIFY       0x07 // Programmed memory failed verification.
#define DFU_errADDRESS      0x08 // Cannot program memory due to received address that is out of range.
#define DFU_errNOTDONE      0x09 // Received DFU_DNLOAD with wLength = 0, but device does not think it has all of the data yet.
#define DFU_errFIRMWARE     0x0A // Devices firmware is corrupt. It cannot return to run-time (non-DFU) operations
#define DFU_errVENDOR       0x0B // iString indicates a vendor-specific error.
#define DFU_errUSBR         0x0C // Device detected unexpected USB reset signaling.
#define DFU_errPOR          0x0D // Device detected unexpected power on reset.
#define DFU_errUNKNOWN      0x0E // Something went wrong, but the device does not know what it was
#define DFU_errSTALLEDPKT   0x0F // Device stalled an unexpected request.
