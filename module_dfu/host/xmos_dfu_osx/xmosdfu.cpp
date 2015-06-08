#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libusb.h"

/* the device's vendor and product id */
#define XMOS_VID 0x20b1

#define XMOS_XCORE_AUDIO_AUDIO2_PID 0x3066
#define XMOS_L1_AUDIO2_PID 0x0002
#define XMOS_L1_AUDIO1_PID 0x0003
#define XMOS_L2_AUDIO2_PID 0x0004
#define XMOS_SU1_AUDIO2_PID 0x0008
#define XMOS_U8_MFA_AUDIO2_PID 0x000A

unsigned short pidList[] = {XMOS_XCORE_AUDIO_AUDIO2_PID, 
                            XMOS_L1_AUDIO2_PID,
                            XMOS_L1_AUDIO1_PID,
                            XMOS_L2_AUDIO2_PID,
                            XMOS_SU1_AUDIO2_PID, 
                            XMOS_U8_MFA_AUDIO2_PID}; 

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
#define XMOS_DFU_RESETDEVICE 		0xf0
#define XMOS_DFU_REVERTFACTORY 		0xf1
#define XMOS_DFU_RESETINTODFU	 	0xf2
#define XMOS_DFU_RESETFROMDFU 		0xf3
#define XMOS_DFU_SAVESTATE   		0xf5
#define XMOS_DFU_RESTORESTATE   	0xf6

static libusb_device_handle *devh = NULL;

static int find_xmos_device(unsigned int id, unsigned int list) 
{
    libusb_device *dev;
    libusb_device **devs;
    int i = 0;
    int found = 0;
  
    libusb_get_device_list(NULL, &devs);

    while ((dev = devs[i++]) != NULL) 
    {
        int foundDev = 0;
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(dev, &desc); 
        printf("VID = 0x%x, PID = 0x%x, BCDDevice: 0x%x\n", desc.idVendor, desc.idProduct, desc.bcdDevice);

        if(desc.idVendor == XMOS_VID)
        {
            for(int j = 0; j < sizeof(pidList)/sizeof(unsigned short); j++)
            {
                if(desc.idProduct == pidList[j] && !list)
                {
                    foundDev = 1;
                    break;
                }
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
                    libusb_config_descriptor *config_desc = NULL;
                    libusb_get_active_config_descriptor(dev, &config_desc); 
                    if (config_desc != NULL) 
                    {
                        for (int j = 0; j < config_desc->bNumInterfaces; j++) 
                        {
                            const libusb_interface_descriptor *inter_desc = ((libusb_interface *)&config_desc->interface[j])->altsetting;
                            if (inter_desc->bInterfaceClass == 0xFE && inter_desc->bInterfaceSubClass == 0x1) 
                            {
                                XMOS_DFU_IF = j;
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

int xmos_dfu_resetdevice(void) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESETDEVICE, 0, 0, NULL, 0, 0);
}

int xmos_dfu_revertfactory(void) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_REVERTFACTORY, 0, 0, NULL, 0, 0);
}

int xmos_dfu_resetintodfu(unsigned int interface) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESETINTODFU, 0, interface, NULL, 0, 0);
}

int xmos_dfu_resetfromdfu(unsigned int interface) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESETFROMDFU, 0, interface, NULL, 0, 0);
}

int dfu_detach(unsigned int interface, unsigned int timeout) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_DETACH, timeout, interface, NULL, 0, 0);
  return 0;
}

int dfu_getState(unsigned int interface, unsigned char *state) {
  libusb_control_transfer(devh, DFU_REQUEST_FROM_DEV, DFU_GETSTATE, 0, interface, state, 1, 0);
  return 0;
}

int dfu_getStatus(unsigned int interface, unsigned char *state, unsigned int *timeout,
                  unsigned char *nextState, unsigned char *strIndex) {
  unsigned int data[2];
  libusb_control_transfer(devh, DFU_REQUEST_FROM_DEV, DFU_GETSTATUS, 0, interface, (unsigned char *)data, 6, 0);
  
  *state = data[0] & 0xff;
  *timeout = (data[0] >> 8) & 0xffffff;
  *nextState = data[1] & 0xff;
  *strIndex = (data[1] >> 8) & 0xff;
  return 0;
}

int dfu_clrStatus(unsigned int interface) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_CLRSTATUS, 0, interface, NULL, 0, 0);
  return 0;
}

int dfu_abort(unsigned int interface) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_ABORT, 0, interface, NULL, 0, 0);
  return 0;
}


int xmos_dfu_save_state(unsigned int interface) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_SAVESTATE, 0, interface, NULL, 0, 0);
  printf("Save state command sent\n");
  return 0;
}

int xmos_dfu_restore_state(unsigned int interface) {
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, XMOS_DFU_RESTORESTATE, 0, interface, NULL, 0, 0);
  printf("Restore state command sent\n");
  return 0;
}

int dfu_download(unsigned int interface, unsigned int block_num, unsigned int size, unsigned char *data) {
  //printf("... Downloading block number %d size %d\r", block_num, size);
  libusb_control_transfer(devh, DFU_REQUEST_TO_DEV, DFU_DNLOAD, block_num, interface, data, size, 0);
  return 0;
}

int dfu_upload(unsigned int interface, unsigned int block_num, unsigned int size, unsigned char*data) {
  unsigned int numBytes = 0;
  numBytes = libusb_control_transfer(devh, DFU_REQUEST_FROM_DEV, DFU_UPLOAD, block_num, interface, (unsigned char *)data, size, 0);
  return numBytes;
}

int write_dfu_image(char *file) {
  int i = 0;
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

  inFile = fopen( file, "rb" );
  if( inFile == NULL ) {
    fprintf(stderr,"Error: Failed to open input data file.\n");
    return -1;
  }

  /* Discover the size of the image. */
  if( 0 != fseek( inFile, 0, SEEK_END ) ) {
    fprintf(stderr,"Error: Failed to discover input data file size.\n");
    return -1;
  }

  image_size = (int)ftell( inFile );

  if( 0 != fseek( inFile, 0, SEEK_SET ) ) {
    fprintf(stderr,"Error: Failed to input file pointer.\n");
   return -1; 
  }

  num_blocks = image_size/block_size;
  remainder = image_size - (num_blocks * block_size);

  printf("... Downloading image (%s) to device\n", file);
 
  dfuBlockCount = 0; 

  for (i = 0; i < num_blocks; i++) {
    memset(block_data, 0x0, block_size);
    fread(block_data, 1, block_size, inFile);
    dfu_download(0, dfuBlockCount, block_size, block_data);
    dfu_getStatus(0, &dfuState, &timeout, &nextDfuState, &strIndex);
    dfuBlockCount++;
  }

  if (remainder) {
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

int read_dfu_image(char *file) {
  FILE *outFile = NULL;
  unsigned int block_count = 0;
  unsigned int block_size = 64;
  unsigned char block_data[64];

  outFile = fopen( file, "wb" );
  if( outFile == NULL ) {
    fprintf(stderr,"Error: Failed to open output data file.\n");
    return -1;
  }

  printf("... Uploading image (%s) from device\n", file);

  while (1) {
    unsigned int numBytes = 0;
    numBytes = dfu_upload(0, block_count, 64, block_data);
    if (numBytes == 0) 
      break;
    fwrite(block_data, 1, block_size, outFile);
    block_count++;
  }

  fclose(outFile);
}

int main(int argc, char **argv) {
  int r = 1;
  unsigned char dfuState = 0;
  unsigned char nextDfuState = 0;
  unsigned int timeout = 0;
  unsigned char strIndex = 0;

  unsigned int download = 0;
  unsigned int upload = 0;
  unsigned int revert = 0;
  unsigned int save = 0;
  unsigned int restore = 0;
  unsigned int listdev = 0;

  char *firmware_filename = NULL;

  if (argc < 2) {
    fprintf(stderr, "No options passed to dfu application\n");
    return -1;
  }

  if (strcmp(argv[1], "--download") == 0) {
    if (argv[2]) {
      firmware_filename = argv[2]; 
    } else {
      fprintf(stderr, "No filename specified for download option\n");
      return -1;
    }
    download = 1;
  } else if (strcmp(argv[1], "--upload") == 0) {
    if (argv[2]) {
      firmware_filename = argv[2]; 
    } else {
      fprintf(stderr, "No filename specified for upload option\n");
      return -1;
    }
    upload = 1;
  } else if (strcmp(argv[1], "--revertfactory") == 0) {
    revert = 1;
  } 
  else if(strcmp(argv[1], "--savecustomstate") == 0)
  {
    save = 1;
  }
  else if(strcmp(argv[1], "--restorecustomstate") == 0)
  {
    restore = 1;
  }
  else if(strcmp(argv[1], "--listdevices") == 0)
  {
    listdev = 1;
  }
  else {
    fprintf(stderr, "Invalid option passed to dfu application\n");
    return -1;
  }

 

  r = libusb_init(NULL);
  if (r < 0) {
    fprintf(stderr, "failed to initialise libusb\n");
    return -1;
  }

  r = find_xmos_device(0, listdev);
  if (r < 0) 
  {
      if(!listdev)
      {
        fprintf(stderr, "Could not find/open device\n");
        return -1;
      }
      return 0;
  }

  r = libusb_claim_interface(devh, XMOS_DFU_IF);
  if (r < 0) {
    fprintf(stderr, "Error claiming interface %d %d\n", XMOS_DFU_IF, r);
    return -1;
  }
  printf("XMOS DFU application started - Interface %d claimed\n", XMOS_DFU_IF);

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

    printf("Detaching device from application mode.\n");
    xmos_dfu_resetintodfu(XMOS_DFU_IF);

    libusb_release_interface(devh, XMOS_DFU_IF);
    libusb_close(devh);


    printf("Waiting for device to restart and enter DFU mode...\n");

    // Wait for device to enter dfu mode and restart
    system("sleep 20");

    // NOW IN DFU APPLICATION MODE

    r = find_xmos_device(0, 0);
    if (r < 0) {
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
                printf("The requested interface does not exist'n");
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

  if (download) {
    write_dfu_image(firmware_filename);
    xmos_dfu_resetfromdfu(XMOS_DFU_IF);
  } else if (upload) {
    read_dfu_image(firmware_filename);
    xmos_dfu_resetfromdfu(XMOS_DFU_IF);
  } else if (revert) {
    printf("... Reverting device to factory image\n");
    xmos_dfu_revertfactory(); 
    // Give device time to revert firmware
    system("sleep 2");
    xmos_dfu_resetfromdfu(XMOS_DFU_IF);
  } 
  else{
    xmos_dfu_resetfromdfu(XMOS_DFU_IF);
  }

  printf("... Returning device to application mode\n");
}
// END OF DFU APPLICATION MODE

  libusb_release_interface(devh, 0);
  libusb_close(devh);
  libusb_exit(NULL);

  return true;
}
