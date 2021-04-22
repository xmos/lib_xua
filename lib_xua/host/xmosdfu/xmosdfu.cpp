// Copyright 2012-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>

// Used for checking if a file is a directory
#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif
// Aliases for Windows
#define stat _stat
#define fstat _fstat
#define fileno _fileno

#else
#include <unistd.h>

void Sleep(unsigned milliseconds) {
    usleep(milliseconds * 1000);
}
#endif

#include "libusb.h"

/* the device's vendor and product id */
#define XMOS_VID 0x20b1

typedef struct device_pid_t
{
    const char *device_name;
    unsigned int pid;
} device_pid_t;

device_pid_t pidList[] = {
    { "XMOS_XCORE_AUDIO_AUDIO2_PID", 0x3066},
    { "XMOS_L1_AUDIO2_PID",          0x0002},
    { "XMOS_L1_AUDIO1_PID",          0x0003},
    { "XMOS_L2_AUDIO2_PID",          0x0004},
    { "XMOS_SU1_AUDIO2_PID",         0x0008},
    { "XMOS_U8_MFA_AUDIO2_PID",      0x000A}
};

unsigned int XMOS_DFU_IF = 0;

#define DFU_REQUEST_TO_DEV 0x21
#define DFU_REQUEST_FROM_DEV 0xa1

// Standard DFU requests
#define DFU_DETACH 0
#define DFU_DNLOAD 1
#define DFU_UPLOAD 2
#define DFU_GETSTATUS 3
#define DFU_CLRSTATUS 4
#define DFU_GETSTATE 5
#define DFU_ABORT 6

// XMOS alternate setting requests
#define XMOS_DFU_RESETDEVICE          0xf0
#define XMOS_DFU_REVERTFACTORY        0xf1
#define XMOS_DFU_RESETINTODFU         0xf2
#define XMOS_DFU_RESETFROMDFU         0xf3
#define XMOS_DFU_SAVESTATE            0xf5
#define XMOS_DFU_RESTORESTATE         0xf6

static libusb_device_handle *devh = NULL;

static int find_xmos_device(unsigned int id, unsigned int pid, unsigned int list)
{
    libusb_device *dev;
    libusb_device **devs;
    int i = 0;
    unsigned int found = 0;

    size_t count = libusb_get_device_list(NULL, &devs);
    if ((int)count < 0)
    {
        printf("ERROR: get_device_list returned %d\n", (int)count);
        exit(1);
    }

    while ((dev = devs[i++]) != NULL)
    {
        int foundDev = 0;
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(dev, &desc);
        printf("VID = 0x%x, PID = 0x%x, BCDDevice: 0x%x\n", desc.idVendor, desc.idProduct, desc.bcdDevice);

        if(desc.idVendor == XMOS_VID)
        {
            if (desc.idProduct == pid && !list)
            {
                foundDev = 1;
            }
        }

        if (foundDev)
        {
            if (found == id)
            {
                if (libusb_open(dev, &devh) < 0)
                {
                    return -1;
                }
                else
                {
                    struct libusb_config_descriptor *config_desc = NULL;
                    int ret = libusb_get_active_config_descriptor(dev, &config_desc);
                    if (ret != 0) {
                      return -1;
                    }
                    if (config_desc != NULL)
                    {
                        //printf("bNumInterfaces: %d\n", config_desc->bNumInterfaces);
                        for (int j = 0; j < config_desc->bNumInterfaces; j++)
                        {
                            //printf("%d\n", j);
                            const struct libusb_interface_descriptor *inter_desc = ((struct libusb_interface *)&config_desc->interface[j])->altsetting;
                            if (inter_desc->bInterfaceClass == 0xFE && inter_desc->bInterfaceSubClass == 0x1)
                            {
                                XMOS_DFU_IF = inter_desc->bInterfaceNumber;
                            }
                        }
                    }
                    else
                    {
                        XMOS_DFU_IF = 0;
                    }
                }
                break;
            }
            found++;
        }
    }

    libusb_free_device_list(devs, 1);

    return devh ? 0 : -1;
}

int xmos_dfu_resetdevice(void)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESETDEVICE, 0, 0, NULL, 0, 0);
    return 0;
}

int xmos_dfu_revertfactory(void)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_REVERTFACTORY, 0, 0, NULL, 0, 0);
    return 0;
}

int xmos_dfu_resetintodfu(unsigned int interface)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESETINTODFU, 0, interface, NULL, 0, 0);
    return 0;
}

int xmos_dfu_resetfromdfu(unsigned int interface)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESETFROMDFU, 0, interface, NULL, 0, 0);
    return 0;
}

int dfu_detach(unsigned int interface, unsigned int timeout)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_DETACH, timeout, interface, NULL, 0, 0);
    return 0;
}

int dfu_getState(unsigned int interface, unsigned char *state)
{
    libusb_control_transfer(devh, DFU_REQUEST_FROM_DEV, DFU_GETSTATE, 0, interface, state, 1, 0);
    return 0;
}

int dfu_getStatus(unsigned int interface, unsigned char *state, unsigned int *timeout,
                  unsigned char *nextState, unsigned char *strIndex)
{
    unsigned int data[2];
    libusb_control_transfer(devh, DFU_REQUEST_FROM_DEV, DFU_GETSTATUS, 0, interface, (unsigned char *)data, 6, 0);

    *state = data[0] & 0xff;
    *timeout = (data[0] >> 8) & 0xffffff;
    *nextState = data[1] & 0xff;
    *strIndex = (data[1] >> 8) & 0xff;
    return 0;
}

int dfu_clrStatus(unsigned int interface)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_CLRSTATUS, 0, interface, NULL, 0, 0);
    return 0;
}

int dfu_abort(unsigned int interface)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_ABORT, 0, interface, NULL, 0, 0);
    return 0;
}


int xmos_dfu_save_state(unsigned int interface)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_SAVESTATE, 0, interface, NULL, 0, 0);
    printf("Save state command sent\n");
    return 0;
}

int xmos_dfu_restore_state(unsigned int interface)
{
    libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESTORESTATE, 0, interface, NULL, 0, 0);
    printf("Restore state command sent\n");
    return 0;
}

unsigned int dfu_download(unsigned int interface, unsigned int block_num, unsigned int size, unsigned char *data)
{
    //printf("... Downloading block number %d size %d\r", block_num, size);
    /* Returns actual data size transferred */
    unsigned int transfered = libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_DNLOAD, block_num, interface, data, size, 0);
    return transfered;
}

int dfu_upload(unsigned int interface, unsigned int block_num, unsigned int size, unsigned char*data)
{
    unsigned int numBytes = 0;
    numBytes = libusb_control_transfer(devh, DFU_REQUEST_FROM_DEV, DFU_UPLOAD, block_num, interface, (unsigned char *)data, size, 0);
    return numBytes;
}

int write_dfu_image(char *file)
{
    unsigned int i = 0;
    FILE* inFile = NULL;
    int image_size = 0;
    unsigned int num_blocks = 0;
    unsigned int block_size = 64;
    unsigned int remainder = 0;
    unsigned char block_data[256];

    unsigned char dfuState = 0;
    unsigned char nextDfuState = 0;
    unsigned int timeout = 0;
    unsigned char strIndex = 0;
    unsigned int dfuBlockCount = 0;
    struct stat statbuf;

    inFile = fopen( file, "rb" );
    if( inFile == NULL )
    {
        fprintf(stderr,"Error: Failed to open input data file.\n");
        return -1;
    }

    /* Check if file is a directory */
    int status = fstat(fileno(inFile), &statbuf);
    if (status != 0)
    {
        fprintf(stderr,"Error: Failed to get info on file.\n");
        return -1;
    }
    if ( S_ISDIR(statbuf.st_mode) )
    {
        fprintf(stderr,"Error: Specified path is a directory.\n");
        return -1;
    }

    /* Discover the size of the image. */
    if( 0 != fseek( inFile, 0, SEEK_END ) )
    {
        fprintf(stderr,"Error: Failed to discover input data file size.\n");
        return -1;
    }

    image_size = (int)ftell( inFile );

    if( 0 != fseek( inFile, 0, SEEK_SET ) )
    {
        fprintf(stderr,"Error: Failed to input file pointer.\n");
        return -1;
    }

    num_blocks = image_size/block_size;
    remainder = image_size - (num_blocks * block_size);

    printf("... Downloading image (%s) to device\n", file);

    dfuBlockCount = 0;

    for (i = 0; i < num_blocks; i++)
    {
        memset(block_data, 0x0, block_size);
        fread(block_data, 1, block_size, inFile);
        unsigned int transferred = dfu_download(0, dfuBlockCount, block_size, block_data);
        if(transferred != block_size)
        {
            /* Error */
            printf("ERROR: %d\n", transferred);
            return -1;

        }
        dfu_getStatus(0, &dfuState, &timeout, &nextDfuState, &strIndex);
        dfuBlockCount++;
    }

    if (remainder)
    {
        memset(block_data, 0x0, block_size);
        fread(block_data, 1, remainder, inFile);
        dfu_download(0, dfuBlockCount, block_size, block_data);
        dfu_getStatus(0, &dfuState, &timeout, &nextDfuState, &strIndex);
    }

    // 0 length download terminates
    dfu_download(0, 0, 0, NULL);
    dfu_getStatus(0, &dfuState, &timeout, &nextDfuState, &strIndex);

    printf("... Download complete\n");

    return 0;
    }

int read_dfu_image(char *file)
{
    FILE *outFile = NULL;
    unsigned int block_count = 0;
    unsigned int block_size = 64;
    unsigned char block_data[64];

    outFile = fopen( file, "wb" );
    if( outFile == NULL )
    {
        fprintf(stderr,"Error: Failed to open output data file.\n");
        return -1;
    }

    printf("... Uploading image (%s) from device\n", file);

    while (1)
    {
        unsigned int numBytes = 0;
        numBytes = dfu_upload(0, block_count, 64, block_data);
        /* Upload is completed when dfu_upload() returns an empty block */
        if (numBytes == 0)
        {
            /* If upload is complete, but no block has been read,
               issue a warning about the upgrade image
            */
            if (block_count==0) {
                printf("... WARNING: Upgrade image size is 0: check if image is present in the flash\n");
            }
            break;
        }
        else if (numBytes < 0)
        {
            fprintf(stderr,"dfu_upload error (%d)\n", numBytes);
            break;
        }
        fwrite(block_data, 1, block_size, outFile);
        block_count++;
    }

    fclose(outFile);
    return 0;
}

static void print_device_list(FILE *file, const char *indent)
{
    for (long unsigned int i = 0; i < sizeof(pidList)/sizeof(pidList[0]); i++)
    {
        fprintf(file, "%s%-30s (0x%0x)\n", indent, pidList[i].device_name, pidList[i].pid);
    }
}

static void print_usage(const char *program_name, const char *error_msg)
{
    fprintf(stderr, "ERROR: %s\n\n", error_msg);
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "     %s --listdevices\n", program_name);
    fprintf(stderr, "     %s DEVICE_PID COMMAND\n", program_name);

    fprintf(stderr, "    Where DEVICE_PID can be a hex value or a name from:\n");
    print_device_list(stderr, "      ");

    fprintf(stderr, "    And COMMAND is one of:\n");
    fprintf(stderr, "       --download <firmware> : write an upgrade image\n");
    fprintf(stderr, "       --upload <firmware>   : read the upgrade image\n");
    fprintf(stderr, "       --revertfactory       : revert to the factory image\n");
    fprintf(stderr, "       --savecustomstate     : \n");
    fprintf(stderr, "       --restorecustomstate  : \n");

    exit(1);
}

static unsigned int select_pid(char *device_pid)
{
    // Try interpreting the name as a hex value
    char * endptr = device_pid;
    int pid = strtol(device_pid, &endptr, 16);
    if (endptr != device_pid && *endptr == '\0')
    {
        return pid;
    }

    // Otherwise do a lookup of names
    for (long unsigned int i = 0; i < sizeof(pidList)/sizeof(pidList[0]); i++)
    {
        if (strcmp(device_pid, pidList[i].device_name) == 0)
        {
            return pidList[i].pid;
        }
    }

    fprintf(stderr, "Failed to find device '%s', should have been one of:\n", device_pid);
    print_device_list(stderr, "  ");
    return 0;
}

int main(int argc, char **argv)
{
    unsigned int download = 0;
    unsigned int upload = 0;
    unsigned int revert = 0;
    unsigned int save = 0;
    unsigned int restore = 0;
    unsigned int listdev = 0;

    char *firmware_filename = NULL;

    const char *program_name = argv[0];

    int r = libusb_init(NULL);
    if (r < 0)
    {
        fprintf(stderr, "failed to initialise libusb\n");
        return -1;
    }

    if (argc == 2)
    {
        if (strcmp(argv[1], "--listdevices") == 0)
        {
            find_xmos_device(0, 0, 1);
            return 0;
        }
        print_usage(program_name, "Not enough options passed to dfu application");
    }

    if (argc < 3)
    {
        print_usage(program_name, "Not enough options passed to dfu application");
    }

    char *device_pid = argv[1];
    char *command = argv[2];

    if (strcmp(command, "--download") == 0)
    {
        if (argc < 4)
        {
            print_usage(program_name, "No filename specified for download option");
        }
        firmware_filename = argv[3];
        download = 1;
    }
    else if (strcmp(command, "--upload") == 0)
    {
        if (argc < 4)
        {
            print_usage(program_name, "No filename specified for upload option");
        }
        firmware_filename = argv[3];
        upload = 1;
    }
    else if (strcmp(command, "--revertfactory") == 0)
    {
        revert = 1;
    }
    else if(strcmp(command, "--savecustomstate") == 0)
    {
        save = 1;
    }
    else if(strcmp(command, "--restorecustomstate") == 0)
    {
        restore = 1;
    }
    else
    {
        print_usage(program_name,  "Invalid option passed to dfu application");
    }

    unsigned int pid = select_pid(device_pid);
    if (pid == 0)
    {
        return -1;
    }
//#define START_IN_DFU 1
#ifndef START_IN_DFU
    r = find_xmos_device(0, pid, 0);
    if (r < 0)
    {
        fprintf(stderr, "Could not find/open device\n");
        return -1;
    }

    r = libusb_claim_interface(devh, XMOS_DFU_IF);
    if (r < 0)
    {
        fprintf(stderr, "Error claiming interface %d %d\n", XMOS_DFU_IF, r);
        return -1;
    }
    printf("XMOS DFU application started - Interface %d claimed\n", XMOS_DFU_IF);
#endif

    /* Dont go into DFU mode for save/restore */
    if(save)
    {
        xmos_dfu_save_state(XMOS_DFU_IF);
    }
    else if(restore)
    {
        xmos_dfu_restore_state(XMOS_DFU_IF);
    }
    else if(!listdev)
    {
#ifndef START_IN_DFU
        printf("Detaching device from application mode.\n");
        xmos_dfu_resetintodfu(XMOS_DFU_IF);

        libusb_release_interface(devh, XMOS_DFU_IF);
        libusb_close(devh);

        printf("Waiting for device to restart and enter DFU mode...\n");

        // Wait for device to enter dfu mode and restart
        Sleep(20 * 1000);
#endif

        // NOW IN DFU APPLICATION MODE

        r = find_xmos_device(0, pid, 0);
        if (r < 0)
        {
            fprintf(stderr, "Could not find/open device\n");
            return -1;
        }

        r = libusb_claim_interface(devh, 0);
        if (r != 0)
        {
            fprintf(stderr, "Error claiming interface 0\n");

            switch(r)
            {
                case LIBUSB_ERROR_NOT_FOUND:
                    printf("The requested interface does not exist\n");
                    break;
                case LIBUSB_ERROR_BUSY:
                    printf("Another program or driver has claimed the interface\n");
                    break;
                case LIBUSB_ERROR_NO_DEVICE:
                    printf("The device has been disconnected\n");
                    break;
                case LIBUSB_ERROR_ACCESS:
                    printf("Access denied\n");
                    break;
                default:
                    printf("Unknown error code:  %d\n", r);
                    break;
            }
            return -1;
        }

        printf("... DFU firmware upgrade device opened\n");

        if (download)
        {
            write_dfu_image(firmware_filename);
            xmos_dfu_resetfromdfu(XMOS_DFU_IF);
        }
        else if (upload)
        {
            read_dfu_image(firmware_filename);
            xmos_dfu_resetfromdfu(XMOS_DFU_IF);
        }
        else if (revert)
        {
            printf("... Reverting device to factory image\n");
            xmos_dfu_revertfactory();
            // Give device time to revert firmware
            Sleep(2 * 1000);
            xmos_dfu_resetfromdfu(XMOS_DFU_IF);
        }
        else
        {
            xmos_dfu_resetfromdfu(XMOS_DFU_IF);
        }

        printf("... Returning device to application mode\n");
    }
    // END OF DFU APPLICATION MODE

    libusb_release_interface(devh, 0);
    libusb_close(devh);
    libusb_exit(NULL);

    return 0;
}
