// Copyright 2012-2024 XMOS LIMITED.
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


typedef struct device_pid_t
{
    const char *device_name;
    unsigned int pid;
} device_pid_t;


unsigned int XMOS_DFU_IF = 0;
static int dfu_timeout = 5000; // 5s

#define USB_BMREQ_H2D_CLASS_INT (LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE)
#define USB_BMREQ_D2H_CLASS_INT (LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE)

#define USB_BMREQ_H2D_VENDOR_INT (LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE)

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

#define bInterfaceProtocol_RUNTIME (1)
#define bInterfaceProtocol_DFU (2)


enum dfu_state {
	DFU_STATE_appIDLE		= 0,
	DFU_STATE_appDETACH		= 1,
	DFU_STATE_dfuIDLE		= 2,
	DFU_STATE_dfuDNLOAD_SYNC	= 3,
	DFU_STATE_dfuDNBUSY		= 4,
	DFU_STATE_dfuDNLOAD_IDLE	= 5,
	DFU_STATE_dfuMANIFEST_SYNC	= 6,
	DFU_STATE_dfuMANIFEST		= 7,
	DFU_STATE_dfuMANIFEST_WAIT_RST	= 8,
	DFU_STATE_dfuUPLOAD_IDLE	= 9,
	DFU_STATE_dfuERROR		= 10
};

static libusb_device_handle *devh = NULL;
static int device_bInterfaceProtocol;
static int match_vendor = -1;
static int match_product = -1;
static int match_vendor_dfu = -1;
static int match_product_dfu = -1;


static int probe_configuration(libusb_device *dev, struct libusb_device_descriptor *desc, unsigned int list)
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
            const struct libusb_interface_descriptor *inter_desc = ((struct libusb_interface *)&config_desc->interface[j])->altsetting;
            if (inter_desc->bInterfaceClass == 0xFE && inter_desc->bInterfaceSubClass == 0x1)
            {
                XMOS_DFU_IF = inter_desc->bInterfaceNumber;
                struct libusb_device_descriptor desc;
                libusb_get_device_descriptor(dev, &desc);

                if(inter_desc->bInterfaceProtocol == bInterfaceProtocol_RUNTIME)
                {
                    printf("Found Runtime: [%04x:%04x] ver=%04x\n", desc.idVendor, desc.idProduct, desc.bcdDevice);
                    if(!list)
                    {
                        if((desc.idVendor != match_vendor) || (desc.idProduct != match_product))
                        {
                            continue;
                        }
                        else
                        {
                            printf("Opening DFU capable USB device, [%04x:%04x], Runtime mode.\n", desc.idVendor, desc.idProduct);
                            device_bInterfaceProtocol = inter_desc->bInterfaceProtocol;
                            return 0;
                        }
                    }
                }
                else if(inter_desc->bInterfaceProtocol == bInterfaceProtocol_DFU)
                {
                    printf("Found DFU: [%04x:%04x] ver=%04x\n", desc.idVendor, desc.idProduct, desc.bcdDevice);
                    if(!list)
                    {
                        if((match_vendor_dfu >= 0 && desc.idVendor != match_vendor_dfu) ||
                            (match_product_dfu >= 0 && desc.idProduct != match_product_dfu))
                        {
                            continue;
                        }
                        else
                        {
                            printf("Opening DFU capable USB device, [%04x:%04x], DFU mode.\n", desc.idVendor, desc.idProduct);
                            device_bInterfaceProtocol = inter_desc->bInterfaceProtocol;
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return -1;
}

static int find_xmos_device(unsigned int list)
{
    libusb_device **devices;
	ssize_t num_devs;
	ssize_t i;

	num_devs = libusb_get_device_list(NULL, &devices);
    for (i = 0; i < num_devs; ++i)
    {
        struct libusb_device_descriptor desc;
		struct libusb_device *dev = devices[i];
        if (libusb_get_device_descriptor(dev, &desc))
			continue;
        int ret = probe_configuration(dev, &desc, list);
        if((list == 0) && (ret == 0))
        {
            if (libusb_open(dev, &devh) < 0)
            {
                libusb_free_device_list(devices, 1);
                return -1;
            }
            break;
        }
    }
    libusb_free_device_list(devices, 1);
    return devh ? 0 : -1;
}


int xmos_dfu_resetdevice(void)
{
    libusb_control_transfer(devh, USB_BMREQ_H2D_CLASS_INT, XMOS_DFU_RESETDEVICE, 0, 0, NULL, 0, 0);
    return 0;
}

int xmos_dfu_revertfactory(void)
{
    libusb_control_transfer(devh, USB_BMREQ_H2D_VENDOR_INT, XMOS_DFU_REVERTFACTORY, 0, 0, NULL, 0, 0);
    return 0;
}

int xmos_dfu_resetintodfu(unsigned int interface)
{
    libusb_control_transfer(devh, USB_BMREQ_H2D_CLASS_INT, XMOS_DFU_RESETINTODFU, 0, interface, NULL, 0, 0);
    return 0;
}

int xmos_dfu_resetfromdfu(unsigned int interface)
{
    libusb_control_transfer(devh, USB_BMREQ_H2D_CLASS_INT, XMOS_DFU_RESETFROMDFU, 0, interface, NULL, 0, 0);
    return 0;
}

int dfu_detach(unsigned int interface, unsigned int timeout)
{
    return libusb_control_transfer(devh, USB_BMREQ_H2D_CLASS_INT, DFU_DETACH, timeout, interface, NULL, 0, dfu_timeout);
}

int dfu_getState(unsigned int interface, unsigned char *state)
{
    libusb_control_transfer(devh, USB_BMREQ_D2H_CLASS_INT, DFU_GETSTATE, 0, interface, state, 1, 0);
    return 0;
}

int dfu_getStatus(unsigned int interface, unsigned char *state, unsigned int *timeout,
                  unsigned char *nextState, unsigned char *strIndex)
{
    unsigned int data[2];
    libusb_control_transfer(devh, USB_BMREQ_D2H_CLASS_INT, DFU_GETSTATUS, 0, interface, (unsigned char *)data, 6, 0);

    *state = data[0] & 0xff;
    *timeout = (data[0] >> 8) & 0xffffff;
    *nextState = data[1] & 0xff;
    *strIndex = (data[1] >> 8) & 0xff;
    return 0;
}

int dfu_clrStatus(unsigned int interface)
{
    libusb_control_transfer(devh, USB_BMREQ_H2D_CLASS_INT, DFU_CLRSTATUS, 0, interface, NULL, 0, 0);
    return 0;
}

int dfu_abort(unsigned int interface)
{
    libusb_control_transfer(devh, USB_BMREQ_H2D_CLASS_INT, DFU_ABORT, 0, interface, NULL, 0, 0);
    return 0;
}

unsigned int dfu_download(unsigned int interface, unsigned int block_num, unsigned int size, unsigned char *data)
{
    //printf("... Downloading block number %d size %d\r", block_num, size);
    /* Returns actual data size transferred */
    unsigned int transfered = libusb_control_transfer(devh, USB_BMREQ_H2D_CLASS_INT, DFU_DNLOAD, block_num, interface, data, size, 0);
    return transfered;
}

int dfu_upload(unsigned int interface, unsigned int block_num, unsigned int size, unsigned char*data)
{
    unsigned int numBytes = 0;
    numBytes = libusb_control_transfer(devh, USB_BMREQ_D2H_CLASS_INT, DFU_UPLOAD, block_num, interface, (unsigned char *)data, size, 0);
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
        do {
            dfu_getStatus(0, &dfuState, &timeout, &nextDfuState, &strIndex);
            if(nextDfuState == DFU_STATE_dfuERROR)
            {
                fprintf(stderr,"Error: dfu_getStatus() returned state as DFU_STATE_dfuERROR.\n");
                return -1;
            }
            if(nextDfuState == DFU_STATE_dfuDNLOAD_IDLE)
            {
                dfuBlockCount++;
                break;
            }
            Sleep(timeout);
        }while(1);
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



static void print_usage(const char *program_name, const char *error_msg)
{
    fprintf(stderr, "ERROR: %s\n\n", error_msg);
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "     %s --listdevices\n", program_name);
    fprintf(stderr, "     %s <vendor>:<product>[,<vendor_dfu>:<product_dfu>] <COMMAND>\n", program_name);

    fprintf(stderr, "    And COMMAND is one of:\n");
    fprintf(stderr, "       --download <firmware> : write an upgrade image\n");
    fprintf(stderr, "       --upload <firmware>   : read the upgrade image\n");
    fprintf(stderr, "       --revertfactory       : revert to the factory image\n");
    fprintf(stderr, "       --savecustomstate     : \n");
    fprintf(stderr, "       --restorecustomstate  : \n");

    exit(1);
}


static void print_pid_usage_and_exit()
{
    fprintf(stderr, "Specify Vendor/Product IDs as hex values in the format \n<vendor_id_runtime>:<product_id_runtime>[,<vendor_id_dfu>:<product_id_dfu>]\n");
    exit(-1);
}


static int parse_match_value(const char *str)
{
    if(str == NULL)
    {
        fprintf(stderr, "NULL string passed to parse_match_value()\n");
        print_pid_usage_and_exit();
    }
	char *remainder;
	int value;
    value = strtoul(str, &remainder, 16);
    if (remainder == str) {
        fprintf(stderr, "Error converting %s string to integer\n", str);
        print_pid_usage_and_exit();
    }
    return value;
}


static void parse_device_vid_pid(const char *str)
{
    const char *comma;
    const char *colon;

	/* Default to match any DFU device in runtime or DFU mode */
	match_vendor = -1;
	match_product = -1;
	match_vendor_dfu = -1;
	match_product_dfu = -1;

    comma = strchr(str, ',');
    if(comma == str) // example ,0xc:0xd
    {
        fprintf(stderr, "Runtime mode Vendor and Product ID not specified\n");
        print_pid_usage_and_exit();
    }
    colon = strchr(str, ':'); // example 0xa
    if(colon == NULL)
    {
        fprintf(stderr, "Runtime mode Vendor or Product ID not specified\n");
        print_pid_usage_and_exit();
    }

    if ((comma != NULL) && (colon > comma)) // example 0xa,0xc:0xd
    {
        fprintf(stderr, "Runtime mode Vendor or Product ID not specified\n");
        print_pid_usage_and_exit();
    }

    if(colon == str) // example :0xb,0xc:0xd
    {
        fprintf(stderr, "Missing runtime mode Vendor ID\n");
        print_pid_usage_and_exit();
    }
    ++colon;
    if((strlen(colon) == 0) || (colon[0] == ',')) // example 0xa:,0xc:0xd
    {
        fprintf(stderr, "Missing runtime mode Product ID\n");
        print_pid_usage_and_exit();
    }

    // Both DFU and runtime mode VID and PID are available.
    match_vendor = parse_match_value(str);
    match_product = parse_match_value(colon);

    if (comma != NULL) { // Parse DFU mode Vendor and Product ID
		++comma;
		colon = strchr(comma, ':'); // example 0xa:0xb, or 0xa:0xb,0xc
        if (colon == NULL) {
            fprintf(stderr, "Missing DFU mode Vendor or Product ID\n");
            print_pid_usage_and_exit();
        }
        if(colon == comma) // example 0xa:0xb,:0xd
        {
            fprintf(stderr, "Missing DFU mode Vendor ID\n");
            print_pid_usage_and_exit();
        }

        ++colon;
        if(strlen(colon) == 0) // example 0xa:0xb,0xc:
        {
            fprintf(stderr, "Missing DFU mode Product ID\n");
            print_pid_usage_and_exit();
        }

        match_vendor_dfu = parse_match_value(comma);
		match_product_dfu = parse_match_value(colon);
	}

    printf("runtime vid 0x%04x, pid 0x%04x. DFU vid 0x%04x, pid 0x%04x\n", match_vendor, match_product, match_vendor_dfu, match_product_dfu);
}

int main(int argc, char **argv)
{
    unsigned int download = 0;
    unsigned int upload = 0;
    unsigned int revert = 0;

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
            find_xmos_device(1);
            return 0;
        }
        print_usage(program_name, "Not enough options passed to dfu application");
    }

    if (argc < 3)
    {
        print_usage(program_name, "Not enough options passed to dfu application");
    }

    char *device_pid = argv[1];
    parse_device_vid_pid(device_pid);

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
    else
    {
        print_usage(program_name,  "Invalid option passed to dfu application");
    }

    unsigned int pid = match_product;
    if (pid == 0)
    {
        return -1;
    }

    r = find_xmos_device(0);
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


    if(device_bInterfaceProtocol == bInterfaceProtocol_RUNTIME)
    {
        printf("Detaching device from application mode.\n");
        if(dfu_detach(XMOS_DFU_IF, 1000) < 0)
        {
            fprintf(stderr, "error detaching\n");
            return -1;
        }

        libusb_release_interface(devh, XMOS_DFU_IF);
        libusb_close(devh);

        printf("Waiting for device to restart and enter DFU mode...\n");

        // Wait for device to enter dfu mode and restart
        Sleep(20 * 1000);
        r = find_xmos_device(0);
        if (r < 0)
        {
            fprintf(stderr, "Could not find/open device\n");
            return -1;
        }

        r = libusb_claim_interface(devh, XMOS_DFU_IF);
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
    }

    printf("... DFU firmware upgrade device opened\n");

    if (download)
    {
        write_dfu_image(firmware_filename);
        if(dfu_detach(XMOS_DFU_IF, 1000) < 0)
        {
            fprintf(stderr, "error detaching\n");
            return -1;
        }
    }
    else if (upload)
    {
        read_dfu_image(firmware_filename);
        if(dfu_detach(XMOS_DFU_IF, 1000) < 0)
        {
            fprintf(stderr, "error detaching\n");
            return -1;
        }
    }
    else if (revert)
    {
        printf("... Reverting device to factory image\n");
        xmos_dfu_revertfactory();
        // Give device time to revert firmware
        Sleep(2 * 1000);
        if(dfu_detach(XMOS_DFU_IF, 1000) < 0)
        {
            fprintf(stderr, "error detaching\n");
            return -1;
        }
    }
    else
    {
        if(dfu_detach(XMOS_DFU_IF, 1000) < 0)
        {
            fprintf(stderr, "error detaching\n");
            return -1;
        }
    }

    printf("... Returning device to application mode\n");

    // END OF DFU APPLICATION MODE

    libusb_release_interface(devh, XMOS_DFU_IF);
    libusb_close(devh);
    libusb_exit(NULL);

    return 0;
}
