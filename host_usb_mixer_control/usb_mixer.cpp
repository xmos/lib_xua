// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "usb_mixer.h"

#if defined(__APPLE__)
    #include "libusb.h"
#elif defined(_WIN32)
    #include "global.h"
    //driver interface
    TUsbAudioApiDll gDrvApi;
#endif

// TODO we dont really need to store mixer input strings
// Currently res, max, min dont get populated

#define XMOS_VID 0x20b1
#define XMOS_DEBUG_PID 0xF7D1

#define USB_REQUEST_TO_DEV 0x21 /* D7 	Data direction: 0 (Host to device)
                                 * D6:5	Type: 01 (Class)
 				 * D4:0 Receipient: 1 (Interface) */
#define USB_REQUEST_FROM_DEV 0xa1

#define USB_CS_INTERFACE 0x24
#define USB_INPUT_TERM_TYPE 0x02
#define USB_MIXER_UNIT_TYPE 0x04
#define USB_FEATURE_UNIT_TYPE 0x06

#define INPUT_TERMINAL USB_INPUT_TERM_TYPE
#define EXTENSION_UNIT 0x9
#define CS_INTERFACE USB_CS_INTERFACE
#define FEATURE_UNIT 0x06

#define CS_XU_SEL 0x6
#define MU_MIXER_CONTROL 0x1

// Output from PC
#define USB_STREAMING 0x01
// Input to device
//#define MICROPHONE 0x02

#define ID_XU_OUT       51
#define ID_XU_IN        52

#define OFFSET_BLENGTH 0
#define OFFSET_BDESCRIPTORTYPE 1
#define OFFSET_BDESCRIPTORSUBTYPE 2
#define OFFSET_BUNITID 3

#define OFFSET_FU_BSOURCEID   4

#define OFFSET_XU_BNRINPINS   6
#define OFFSET_XU_BSOURCEID   7

#define OFFSET_IT_WTERMINALTYPE 5
#define OFFSET_IT_BNRCHANNELS 8
#define OFFSET_IT_ICHANNELNAMES 13

typedef struct
{
    double min;
    double max;
    double res;
    double weight;
} mixer_node;

typedef struct 
{
    unsigned int id;
    unsigned int num_inputs;
    char input_names[USB_MIXER_INPUTS][USB_MIXER_MAX_NAME_LEN]; /* Current mixer input names -
                                                                 * we dont really need to store these */
    int input_connections[USB_MIXER_INPUTS];
    unsigned int num_outputs;
    char output_names[USB_MIXER_INPUTS][USB_MIXER_MAX_NAME_LEN];
    unsigned int num_inPins;
    mixer_node nodes[USB_MIXER_INPUTS * USB_MIXER_OUTPUTS];
} usb_mixer_device;

typedef struct {
    int cur;
    int default_value;
    char name[USB_MIXER_MAX_NAME_LEN];
    enum usb_chan_type ctype;
}channel_map_node;

typedef struct {
    int numInputs;
    int numOutputs;
    channel_map_node map[USB_MAX_CHANNEL_MAP_SIZE];
}channel_mapp;

typedef struct
{
    unsigned int id;
    unsigned int numInputs;
    char inputStrings[USB_MIXER_INPUTS*4][USB_MIXER_MAX_NAME_LEN];   /* Complete list of all possible inputs */
    unsigned int numOutputs;
    unsigned int state[USB_MIXER_INPUTS];
} t_usb_mixSel;

typedef struct {
  unsigned int device_open;
  unsigned int num_usb_mixers;
  usb_mixer_device usb_mixer[USB_MIXERS];
  t_usb_mixSel usb_mixSel[USB_MIXERS];

  channel_mapp audChannelMap;
  channel_mapp usbChannelMap;


} usb_mixer_handle;

#if defined(__APPLE__)
typedef unsigned char   char_style;
#elif defined(_WIN32)
typedef char            char_style;
#endif

static usb_mixer_handle *usb_mixers = NULL;

#if defined(__APPLE__)
    static libusb_device_handle *devh = NULL;
#elif defined(_WIN32)
    static TUsbAudioHandle devh;
#endif

/* Issue a generic control/class GET request to a specific unit in the Audio Interface */
int usb_audio_class_get(unsigned char bRequest, unsigned char cs, unsigned char cn, unsigned short unitID, unsigned short wLength, unsigned char *data)
{
#if defined(__APPLE__)
    return libusb_control_transfer(devh, 
                            USB_REQUEST_FROM_DEV, 
                            bRequest, 
                            (cs<<8) | cn, /* wValue */
                            (unitID & 0xff) << 8 | 0x0,
                            data,
                            wLength,
                            0);
#elif defined(_WIN32)
    return gDrvApi.TUSBAUDIO_AudioControlRequestGet(devh,
                            unitID,
                            bRequest,
                            cs,
                            cn,
                            data,
                            wLength,
                            NULL,
                            1000);
#endif
}

/* Issue a generic control/class SET request to a specific unit in the Audio Interface */
int usb_audio_class_set(unsigned char bRequest, unsigned char cs, unsigned char cn, unsigned short unitID, unsigned short wLength, unsigned char *data)
{
#if defined(__APPLE__)
    return libusb_control_transfer(devh,
                            USB_REQUEST_TO_DEV,
                            bRequest, 
                            (cs<<8) | cn,            /* wValue */
                            (unitID & 0xff) << 8 | 0x0,
                            data,
                            wLength,
                            0);
#elif defined(_WIN32)
    return gDrvApi.TUSBAUDIO_AudioControlRequestSet(devh,
                            unitID,
                            bRequest,
                            cs,
                            cn,
                            data,
                            wLength,
                            NULL,
                            1000);
#endif
}

/* Note, this never get cached in an object since it can change on the device side */
int usb_mixer_mem_get(unsigned int mixer, unsigned offset, unsigned char *data)
{
#if defined(__APPLE__)
    return libusb_control_transfer(devh,
                            USB_REQUEST_FROM_DEV,    /* nRequest */
                            MEM,                     /* bRequest */
                            offset,                  /* wValue */
                            (usb_mixers->usb_mixer[mixer].id & 0xff) << 8 | 0x0, /* wIndex */
                            data, 64, 0);
#elif defined(_WIN32)
    return gDrvApi.TUSBAUDIO_AudioControlRequestGet(devh,
                            usb_mixers->usb_mixer[mixer].id,
                            MEM,
                            0, // cs
                            offset, //was cn
                            &data,
                            64,
                            NULL,
                            1000);
#endif
}

static const unsigned char *findUnit(const unsigned char *descs, int length, int id)
{
    const unsigned char *interface_data = descs;
    while (length) 
    {
        const unsigned char *interface_len = interface_data;
        int bDescriptorType = *(interface_len + 1);
        if (bDescriptorType == CS_INTERFACE) 
        {
            int unit_id = *(interface_len + 3);
            if (unit_id == id) 
            {
                return interface_len;
            }
        }
        interface_data+=*interface_len;
        length -= *interface_len;
    }
    return NULL;
}

static int get_num_mixer_units(const unsigned char *data, int length) {
  const unsigned char *interface_data = data;
  int interface_len = length;
  int num_mixer_units_found = 0;

  while (interface_len) {
    const unsigned char *interfaces = interface_data;
    int interface_type = *(interfaces + 1);
    int unit_type = *(interfaces + 2);
    if (interface_type == USB_CS_INTERFACE && unit_type == USB_MIXER_UNIT_TYPE) {
      num_mixer_units_found++;
    }
    interface_data+=*interfaces;
    interface_len -= *interfaces;
  }

  return num_mixer_units_found;
}

static double dev_get_mixer_value(unsigned int mixer, unsigned int nodeId)
{
    short data;
    unsigned char cs = 0; /* Device doesnt use CS for getting/setting mixer weights */
    usb_audio_class_get(CUR, cs,  nodeId,  usb_mixers->usb_mixer[mixer].id, 2,(unsigned char *) &data);
    return ((double) data / 256);
}

/* Populates min, max, res */
static unsigned short dev_get_mixer_range(unsigned int mixer, unsigned int channel, 
         double *min, double *max, double *res)
{
    short data[64];

    short min2, max2, res2;

    usb_audio_class_get(RANGE, MU_MIXER_CONTROL, channel, usb_mixers->usb_mixer[mixer].id, 8, (unsigned char *) data); 

    min2 = data[1];
    max2 = data[2];
    res2 = data[3];
    //printf("%f, %f, %f\n", (double)min2/256, (double)max2/256, (double) res2/256);
    *min = (double)min2/256;
    *max = (double)max2/256;
    *res = (double)res2/256;

   return 0;
}

int dev_get_channel_map(int channel, int unitId)
{
    short data;
    usb_audio_class_get(CUR, 0,  channel,  unitId, 2,(unsigned char *) &data);
    return data;
}

static int dev_set_channel_map(int channel, int val, int unitId)
{
    short value = val;
    usb_audio_class_set(CUR, 0, channel, unitId, 1, (unsigned char *)&value);
    return 0;
}

static int mixer_update_all_nodes(unsigned int mixer_index) 
{
    int i = 0;
    int j = 0;
    double min, max, res;
 
    for (i = 0; i < usb_mixers->usb_mixer[mixer_index].num_inputs; i++) 
    {
        for (j = 0; j < usb_mixers->usb_mixer[mixer_index].num_outputs; j++) 
        {
            dev_get_mixer_range(mixer_index, (i*usb_mixers->usb_mixer[mixer_index].num_outputs)+j, &min, &max, &res);
           
            usb_mixers->usb_mixer[mixer_index].nodes[(i*usb_mixers->usb_mixer[mixer_index].num_outputs)+j].min = min; 
            usb_mixers->usb_mixer[mixer_index].nodes[(i*usb_mixers->usb_mixer[mixer_index].num_outputs)+j].max = max; 
            usb_mixers->usb_mixer[mixer_index].nodes[(i*usb_mixers->usb_mixer[mixer_index].num_outputs)+j].res = res; 
            //printf("%f, %f, %f\n", min, max, res);

            usb_mixers->usb_mixer[mixer_index].nodes[(i*usb_mixers->usb_mixer[mixer_index].num_outputs)+j].weight = 
                dev_get_mixer_value(mixer_index, (i*usb_mixers->usb_mixer[mixer_index].num_outputs)+j);
        }
    }
    return 0;
}


/* Start at unit %id, find it in descs, keep recursively parsing up path(s) until get to Input Term and add strings */
int addStrings(const unsigned char *data, int length, int mixer_index, int id, int chanCount)
{
    const unsigned char *currentUnitPtr = NULL;

    /* Find this unit in the descs */
    currentUnitPtr = findUnit(data, length, id);
#if defined(_WIN32)
    TUsbAudioStatus st;
#endif
                
    if(currentUnitPtr != NULL)
    {
        /* Check if unit is a Input term */
        if(*(currentUnitPtr+OFFSET_BDESCRIPTORSUBTYPE) == INPUT_TERMINAL)
        {

             /* Get channel names */
#ifdef DEBUG
            printf("Input terminal found on path (ID: %d): %d channels, total: %d\n",*(currentUnitPtr+OFFSET_BUNITID),  
                    *(currentUnitPtr+OFFSET_IT_BNRCHANNELS), chanCount);
#endif

            int iChannelNames = *(currentUnitPtr+OFFSET_IT_ICHANNELNAMES);
            int wTerminalType = *(currentUnitPtr+OFFSET_IT_WTERMINALTYPE);

#ifdef DEBUG
            printf("iChannelNames: %d wTerminalType: %d\n", iChannelNames, wTerminalType);

            printf("Channels found:\n");
            
#endif
            for(int i = 0; i < *(currentUnitPtr+OFFSET_IT_BNRCHANNELS); i++)
            {
                char_style mixer_input_name[USB_MIXER_MAX_NAME_LEN];
#if defined(__APPLE__)
                memset(mixer_input_name, 0 ,USB_MIXER_MAX_NAME_LEN);
#elif defined(_WIN32)
                WCHAR mixer_input_name_wchar[USB_MIXER_MAX_NAME_LEN];
#endif
                if (wTerminalType == 1) 
                {
                    strcpy(usb_mixers->usb_mixSel[mixer_index].inputStrings[chanCount], "DAW - ");

                    //usb_mixers->channel_maps[usb_mixers->num_channel_maps].ctype = (enum usb_chan_type) USB_CHAN_OUT;
                    
                    usb_mixers->audChannelMap.numOutputs = usb_mixers->audChannelMap.numOutputs +1;

                    usb_mixers->audChannelMap.map[usb_mixers->audChannelMap.numInputs].ctype = (enum usb_chan_type) USB_CHAN_OUT;
                    usb_mixers->usbChannelMap.map[usb_mixers->usbChannelMap.numInputs].ctype = (enum usb_chan_type) USB_CHAN_OUT;
                    
                }
                else  
                {
                    strcpy(usb_mixers->usb_mixSel[mixer_index].inputStrings[chanCount], "AUD - ");
                    
                    //usb_mixers->channel_maps[usb_mixers->num_channel_maps].ctype = (enum usb_chan_type) USB_CHAN_IN;

                    usb_mixers->audChannelMap.map[usb_mixers->audChannelMap.numInputs].ctype = (enum usb_chan_type) USB_CHAN_IN;
                    usb_mixers->usbChannelMap.map[usb_mixers->usbChannelMap.numInputs].ctype = (enum usb_chan_type) USB_CHAN_IN;


                    usb_mixers->usbChannelMap.numOutputs = usb_mixers->usbChannelMap.numOutputs +1;
                } 
                /* Get relevant string descriptor */
#if defined(__APPLE__)
                libusb_get_string_descriptor_ascii(devh, iChannelNames+i, mixer_input_name, 
                    USB_MIXER_MAX_NAME_LEN - strlen(usb_mixers->usb_mixSel[mixer_index].inputStrings[chanCount]));
#elif defined(_WIN32)
                st = gDrvApi.TUSBAUDIO_GetUsbStringDescriptorString(devh, iChannelNames + i, 0, mixer_input_name_wchar, USB_MIXER_MAX_NAME_LEN - strlen(usb_mixers->usb_mixSel[mixer_index].inputStrings[chanCount]));
                if (TSTATUS_SUCCESS != st) {
                    return USB_MIXER_FAILURE;
                }
                wcstombs(mixer_input_name, mixer_input_name_wchar, USB_MIXER_MAX_NAME_LEN);
#endif
                strcat(usb_mixers->usb_mixSel[mixer_index].inputStrings[chanCount], (char *)mixer_input_name);

                /* Add to channel mappers also */
                //strcat(usb_mixers->channel_maps[usb_mixers->num_channel_maps].name, (char *)mixer_input_name);
                strcat(usb_mixers->audChannelMap.map[usb_mixers->audChannelMap.numInputs ].name, (char *)mixer_input_name);
                strcat(usb_mixers->usbChannelMap.map[usb_mixers->audChannelMap.numInputs].name, (char *)mixer_input_name);
                
                usb_mixers->audChannelMap.numInputs = usb_mixers->audChannelMap.numInputs +1;
                usb_mixers->usbChannelMap.numInputs = usb_mixers->usbChannelMap.numInputs +1;

                //usb_mixers->num_channel_maps = usb_mixers->num_channel_maps+1;
                chanCount++;
            }

#ifdef DEBUG
            int meh = chanCount - *(currentUnitPtr+OFFSET_IT_BNRCHANNELS);
            for(int i = 0; i < *(currentUnitPtr+OFFSET_IT_BNRCHANNELS); i++)
            {
                printf("%d: %s\n", i,usb_mixers->usb_mixSel[mixer_index].inputStrings[meh+i]);
            }

            printf("\n\n");
#endif
        }
        else 
        {
            /* Unit not a input terminal, keep going... */
            if(*(currentUnitPtr+OFFSET_BDESCRIPTORSUBTYPE) == FEATURE_UNIT)
            {
                chanCount = addStrings(data, length, mixer_index, *(currentUnitPtr+OFFSET_FU_BSOURCEID), chanCount);
            }
            else if(*(currentUnitPtr+OFFSET_BDESCRIPTORSUBTYPE) == EXTENSION_UNIT)
            {
                /* Multiple inputs for Extention units */
                for (int i = 0; i < *(currentUnitPtr+OFFSET_XU_BNRINPINS); i++)
                {
                    chanCount = addStrings(data, length, mixer_index, *(currentUnitPtr+OFFSET_XU_BSOURCEID+i), chanCount);
                }
            }
            else
            {
                fprintf(stderr,"ERROR: Currently don't support this unit: %d\n", 
                    *(currentUnitPtr+OFFSET_BDESCRIPTORSUBTYPE));
                exit(1); 
            }
        } 
    }
    else
    {
        fprintf(stderr,"ERROR: Couldn't find unit %d in descs\n", id );
        exit(1);
    }

    return chanCount;
}

/* Returns the source of an mix sel output */
static unsigned char get_mixsel_value(unsigned int mixer, unsigned int channel)
{
    
    unsigned char bRequest = CUR;
    //unsigned cs = CS_XU_SEL;
    unsigned char cs = 1; /* Note, currently the host app configures all mix sel's indentically, so if we get one they all should match */
    unsigned char cn = channel;
    unsigned short unitId = usb_mixers->usb_mixSel[mixer].id;
    unsigned short wLength = 1;
    unsigned char data[64];
    
    usb_audio_class_get(CUR, cs, cn, unitId, wLength, (unsigned char *)data);
    
    return data[0];
}

static int get_mixer_info(const unsigned char *data, int length, unsigned int mixer_index
#if defined(__APPLE__)
                                                ,libusb_config_descriptor *config_desc
#endif
) 
{
    const unsigned char *interface_data = data;
    int interface_len = length;
    int num_mixer_units_found = 0;
    //const unsigned char *current_input_term_unit_ptr = NULL;
    //int current_input_term_unit_index = 0;
    //const unsigned char *current_feature_unit_ptr = NULL;
    int devChanInputCount = 0;

    while (interface_len) 
    {
        const unsigned char *interfaces = interface_data;
        int interface_type = *(interfaces + 1);          /* bDescriptorType */
        int unit_type = *(interfaces + 2);               /* bDescriptorSubType */

        /* Look for a mixer unit */
        if (interface_type == USB_CS_INTERFACE && unit_type == USB_MIXER_UNIT_TYPE) 
        {
            int unit_id = *(interfaces + 3);    /* bUnitId */
            int bNrInPins = *(interfaces + 4);
            int num_in = *(interfaces + 4);     /* bNrInPins - NOTE This is pins NOT channels!! */
                                                /* Total number of inputs is the sum of the channel counts in the input 
                                                 * clusters.  We need to inspect the sources to gain channel counts */
            int chansIn = 0;
#ifdef DEBUG 
            printf("Found Mixer Unit %d with %d inputs\n", unit_id, bNrInPins);
            printf("Inspecting mixer inputs... \n\n");
#endif 
            /* For each input pin need to find out inspect its output cluster */
            for (int i = 1; i <= bNrInPins; i++)
            {
                int sourceId = *(interfaces+4+i);
#ifdef DEBUG 
                printf("baSourceID(%d): %d\n", i, sourceId);
#endif

                /* Find the unit in the config desc */
                int descsLength = length;
                const unsigned char *descsData = data;
                int found = 0;
                int bDescriptorSubType;
                int bDescriptorType;
                int bUnitId;

                while(descsLength)
                {
                    int currentLength = *descsData;
                    bDescriptorSubType = *(descsData + 2);
                    bDescriptorType = *(descsData + 1);

                    if(bDescriptorType == USB_CS_INTERFACE) 
                    {
                        if(bDescriptorSubType == USB_FEATURE_UNIT_TYPE || 
                            bDescriptorSubType == USB_INPUT_TERM_TYPE ||
                            bDescriptorSubType == EXTENSION_UNIT )
                        {
                            bUnitId = *(descsData+3);
                            if(bUnitId == sourceId) 
                            {
                                found = 1;
                                break;
                            }
                        }
                    }
                    
                    descsData+=currentLength;
                    descsLength -= currentLength;
                }

                if(found)
                {
                    int bNrChannelsOffset = 0;
                    int bNrChannels;

                    /* We found the unit in the descs.  Now inspect channel cluster */
#ifdef DEBUG
                    printf("Found unit %d, type %d\n", bUnitId, bDescriptorSubType);
#endif
                    /* We are looking for bNrChannels in the descs, this is in a different location in desc depending 
                     * on unit type */
                    switch(bDescriptorSubType)
                    {
                        case USB_INPUT_TERM_TYPE:
                            bNrChannelsOffset = 8;
                            bNrChannels = *(descsData+bNrChannelsOffset);
                            break; 
                        case EXTENSION_UNIT:
                            bNrChannelsOffset = 7 + *(descsData+6);
                            bNrChannels = *(descsData+bNrChannelsOffset);

                            break; 
                        default:
                            printf("ERR\n");
                            exit(1);
                            break;               
                    }
#ifdef DEBUG
                    printf("Output chan count: %d\n\n", bNrChannels);
#endif
                    chansIn += bNrChannels;

                }
                else
                {          
                    fprintf(stderr,"ERROR: Mixer input connected to something we dont understand...\n");
                    exit(1);
                }
            }
            
            /* get number of output channels straight from mixer unit descriptor: bNrChannels */
            int num_out = *(interfaces + 5 + num_in); 
#ifdef DEBUG 
            printf("Mixer Unit parse complete: bUnitId: %d, Total Input Chans: %d, Output Chans: %d\n\n", unit_id, chansIn, num_out);
#endif
            usb_mixers->usb_mixer[mixer_index].id = unit_id;
            usb_mixers->usb_mixer[mixer_index].num_inputs = chansIn;
            usb_mixers->usb_mixer[mixer_index].num_inPins = bNrInPins;
            usb_mixers->usb_mixer[mixer_index].num_outputs = num_out;

            /* Go through all input pins */
            const unsigned char *in_unit_start_ptr = interfaces + 5;
            // const unsigned char *currentUnitPtr = NULL;
            // int current_input_term_unit_id = 0;
           
            /* We expect this to be a single input from an XU, but we'll keep it slightly generic here */ 
            for (int num = 0; num < usb_mixers->usb_mixer[mixer_index].num_inPins; num++) 
            {
                /* Save source ID */
                usb_mixers->usb_mixer[mixer_index].input_connections[num] = *(in_unit_start_ptr+num);
#ifdef DEBUG
                printf("Inspecting for Input Terms from mixer unit input pin %d (id: %d)\n",
                        num,usb_mixers->usb_mixer[mixer_index].input_connections[num]); 
#endif

                devChanInputCount = addStrings(data, length, mixer_index, 
                        usb_mixers->usb_mixer[mixer_index].input_connections[num], devChanInputCount); 
               
            }
           
            /* The the first input pin at the mix select for the moment. 
             * probably should be checking if its an XU here */ 
            usb_mixers->usb_mixSel[mixer_index].id = usb_mixers->usb_mixer[mixer_index].input_connections[0];
            usb_mixers->usb_mixSel[mixer_index].numInputs = devChanInputCount;
            usb_mixers->usb_mixSel[mixer_index].numOutputs = chansIn;

            /* Set up mixer output names */
            char_style mixer_output_name[USB_MIXER_MAX_NAME_LEN];
            unsigned int iChannelNames =  *(interfaces + 10 + bNrInPins);
#if defined(_WIN32)
            WCHAR mixer_output_name_wchar[USB_MIXER_MAX_NAME_LEN];
#endif

            //OSX     
            for (int i = 0; i < usb_mixers->usb_mixer[mixer_index].num_outputs; i++)
            {
                /* Get relevant string descriptor */
                strcpy(usb_mixers->usb_mixer[mixer_index].output_names[i], "MIX - ");
#if defined(__APPLE__)
                libusb_get_string_descriptor_ascii(devh, iChannelNames+i, mixer_output_name, 
                  USB_MIXER_MAX_NAME_LEN - strlen(usb_mixers->usb_mixSel[mixer_index].inputStrings[i]));
#elif defined(_WIN32)
                TUsbAudioStatus st = gDrvApi.TUSBAUDIO_GetUsbStringDescriptorString(devh, iChannelNames + i, 0, mixer_output_name_wchar, USB_MIXER_MAX_NAME_LEN - strlen(usb_mixers->usb_mixSel[mixer_index].inputStrings[i]));
                if (TSTATUS_SUCCESS != st) {
                    return USB_MIXER_FAILURE;
                }
                wcstombs(mixer_output_name, mixer_output_name_wchar, USB_MIXER_MAX_NAME_LEN);
#endif
                strcat(usb_mixers->usb_mixer[mixer_index].output_names[i], (char *)mixer_output_name);
            }
        }

        interface_data+=*interfaces;
        interface_len -= *interfaces;
    }

    return num_mixer_units_found;
}
//OSX
static int find_xmos_device(unsigned int id) 
{
    int i = 0;
    int found = -1;
#if defined(__APPLE__)
    libusb_device *dev;
    libusb_device **devs;

    libusb_get_device_list(NULL, &devs);
    while ((dev = devs[i]) != NULL) 
    {
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(dev, &desc);
        // printf("VID = 0x%x, PID = 0x%x\n", desc.idVendor, desc.idProduct);
        if (desc.idVendor == XMOS_VID && desc.idProduct < XMOS_DEBUG_PID)
        {
            found++;
            if (found == id)
            {
                id = i;
            }
        }
        i++;
    }
    if (found == -1) {
        fprintf(stderr, "ERROR :: No device detected\n");
        return USB_MIXER_FAILURE;
    }
    else if (found > 0) {
        fprintf(stderr, "ERROR :: Multiple devices detected\n");
        return USB_MIXER_FAILURE;
    }

#elif defined(_WIN32)
    TUsbAudioStatus st;

    unsigned int devcnt = gDrvApi.TUSBAUDIO_GetDeviceCount();
    if (devcnt == 0) {
        fprintf(stderr, "ERROR :: No device detected\n");
        return USB_MIXER_FAILURE;
    }
    else if (devcnt > 1) {
        fprintf(stderr, "ERROR :: Multiple devices detected\n");
        return USB_MIXER_FAILURE;
    }

    st = gDrvApi.TUSBAUDIO_OpenDeviceByIndex(0, &devh);
#endif
    if (
#if defined(__APPLE__)
    libusb_open(devs[id], &devh) < 0
#elif defined(_WIN32)
    TSTATUS_SUCCESS != st
#endif
    )
    {
        devh = 0;
        // skip
    }
    else
    {
#if defined(__APPLE__)
        libusb_config_descriptor *config_desc = NULL;
        libusb_get_active_config_descriptor(devs[id], &config_desc);
        if (config_desc != NULL) 
        {
            //unsigned int num_mixers_found = 0;

            usb_mixers->device_open = 1;
            usb_mixers->num_usb_mixers = 0;

            for (int j = 0; j < config_desc->bNumInterfaces; j++) 
            {
                const libusb_interface_descriptor *inter_desc = 
                    ((libusb_interface *)&config_desc->interface[j])->altsetting;
    
                usb_mixers->num_usb_mixers += get_num_mixer_units(inter_desc->extra, inter_desc->extra_length);
            }

            for (int j = 0; j < config_desc->bNumInterfaces; j++) 
            {
                const libusb_interface_descriptor *inter_desc = 
                    ((libusb_interface *)&config_desc->interface[j])->altsetting;
                get_mixer_info(inter_desc->extra, inter_desc->extra_length, j, config_desc);
            }
        }
#elif defined(_WIN32)
        unsigned int numBytes = 0;
        unsigned char descBuffer[64 * 1024];

        st = gDrvApi.TUSBAUDIO_GetUsbConfigDescriptor(devh, descBuffer, 64 * 1024, &numBytes);
        if (TSTATUS_SUCCESS != st) {
            return USB_MIXER_FAILURE;
        }

        usb_mixers->num_usb_mixers = get_num_mixer_units(descBuffer, numBytes);
        get_mixer_info(descBuffer, numBytes, 0);
#endif
        /* Init channel maps from device */
        for(int i = 0; i < usb_mixers->audChannelMap.numOutputs; i++)
        {
        usb_mixers->audChannelMap.map[i].cur = dev_get_channel_map(i, ID_XU_OUT);
        
        }

        for(int i = 0; i < usb_mixers->usbChannelMap.numOutputs; i++)
        {
        usb_mixers->usbChannelMap.map[i].cur = dev_get_channel_map(i, ID_XU_IN);
        //printf("%d\n", usb_mixers->usbChannelMap.map[i].cur);
        }
        
        /* Now add the mix outputs */
        for(int i = 0; i < usb_mixers->num_usb_mixers; i++)
        {
            for(int j = 0; j < usb_mixers->usb_mixer[i].num_outputs;j++)
            {   
                //strcpy(usb_mixers->channel_maps[usb_mixers->num_channel_maps].name, usb_mixers->usb_mixer[i].output_names[j]);
                //usb_mixers->channel_maps[usb_mixers->num_channel_maps].ctype = (enum usb_chan_type) USB_CHAN_MIXER;
                //usb_mixers->num_channel_maps = usb_mixers->num_channel_maps+1;

                usb_mixers->audChannelMap.map[usb_mixers->audChannelMap.numInputs].ctype = (enum usb_chan_type) USB_CHAN_MIXER;
                strcpy(usb_mixers->audChannelMap.map[usb_mixers->audChannelMap.numInputs].name, usb_mixers->usb_mixer[i].output_names[j]);
                usb_mixers->audChannelMap.numInputs = usb_mixers->audChannelMap.numInputs +1;

                usb_mixers->usbChannelMap.map[usb_mixers->usbChannelMap.numInputs].ctype = (enum usb_chan_type) USB_CHAN_MIXER;
                strcpy(usb_mixers->usbChannelMap.map[usb_mixers->usbChannelMap.numInputs].name, usb_mixers->usb_mixer[i].output_names[j]);
                usb_mixers->usbChannelMap.numInputs = usb_mixers->usbChannelMap.numInputs +1;
            }
        }

        if(devh)
        {
            /* Populate mixer input strings */
            for(int i = 0; i < usb_mixers->num_usb_mixers; i++)
            {
                mixer_update_all_nodes(i);

                /* Get current each mixer input and populate channel number state and strings from device */
                for (int j = 0; j < usb_mixers->usb_mixSel[i].numOutputs; j++)
                {
                    /* Get value from device */
                    int inputChan = get_mixsel_value(i, j);

                    usb_mixers->usb_mixSel[i].state[j] = inputChan;

                    /* Look up channel string */
                    strcpy(usb_mixers->usb_mixer[i].input_names[j], usb_mixers->usb_mixSel[i].inputStrings[inputChan]);
                }
            }
        }
    }
#if defined(__APPLE__)
    libusb_free_device_list(devs, 1);
#endif
    return devh ? 0 : -1;
}

// End of libusb interface functions

int usb_mixer_connect() 
{
    // Allocate internal storage
    usb_mixers = (usb_mixer_handle *)malloc(sizeof(usb_mixer_handle));
    memset(usb_mixers, 0, sizeof(usb_mixer_handle));

#if defined(_WIN32)
    gDrvApi.LoadByGUID(_T("{E5A2658B-817D-4A02-A1DE-B628A93DDF5D}"));
    TUsbAudioStatus st = gDrvApi.TUSBAUDIO_EnumerateDevices();
#endif

    if (
#if defined(__APPLE__)
        libusb_init(NULL) < 0
#elif defined(_WIN32)
        TSTATUS_SUCCESS != st
#endif
        ) 
    {
        // printf("libusb_init failure\n");
        return USB_MIXER_FAILURE;
    }

    if (find_xmos_device(0) < 0) 
    {
        // printf("find_xmos_device failure\n");
        return USB_MIXER_FAILURE;
    }

    return USB_MIXER_SUCCESS;
}

int usb_mixer_disconnect() {
#if defined(__APPLE__)
    libusb_close(devh);
    libusb_exit(NULL);
#elif(_WIN32)
    if (devh != 0)
    {
        gDrvApi.TUSBAUDIO_CloseDevice(devh);
    }
#endif

    free(usb_mixers);

    return USB_MIXER_SUCCESS;
}

/* Getter for num_usb_mixers */
int usb_mixer_get_num_mixers() 
{
  return usb_mixers->num_usb_mixers;
}

int usb_mixer_get_num_outputs(unsigned int mixer)
{
  return usb_mixers->usb_mixer[mixer].num_outputs;
}

int usb_mixer_get_num_inputs(unsigned int mixer)
{
  return usb_mixers->usb_mixer[mixer].num_inputs;
}

int usb_mixer_get_layout(unsigned int mixer, unsigned int *inputs, unsigned int *outputs) {
  *inputs = usb_mixers->usb_mixer[mixer].num_inputs;
  *outputs = usb_mixers->usb_mixer[mixer].num_outputs;
  return 0;
}

/* MixSel getters and setters */
char *usb_mixsel_get_input_string(unsigned int mixer, unsigned int input) 
{
    return usb_mixers->usb_mixSel[mixer].inputStrings[input];
}

int usb_mixsel_get_input_count(unsigned int mixer)
{
    return usb_mixers->usb_mixSel[mixer].numInputs;
}

int usb_mixsel_get_output_count(unsigned int mixer)
{
    return usb_mixers->usb_mixSel[mixer].numOutputs;
}

char *usb_mixer_get_input_name(unsigned int mixer, unsigned int input) 
{
    return usb_mixers->usb_mixer[mixer].input_names[input];
}

char *usb_mixer_get_output_name(unsigned int mixer, unsigned int output) 
{
    return usb_mixers->usb_mixer[mixer].output_names[output];
}

unsigned char usb_mixsel_get_state(unsigned int mixer, unsigned int channel)
{
    return usb_mixers->usb_mixSel[mixer].state[channel];
}

void usb_mixsel_set_state(unsigned int mixer, unsigned int dst, unsigned int src)
{
    // Write to device
    // Note, we are updating inputs to all mixers here with a hard-coded 0, though the device allows
    // for separate input mapping per mixer
    unsigned bRequest = CUR; 
    unsigned cs = 0;
    unsigned cn = usb_mixers->usb_mixSel[mixer].id;
    unsigned wLength = 1;
    usb_audio_class_set(CUR, 0, dst, usb_mixers->usb_mixSel[mixer].id, wLength, (unsigned char *)&src);

    // Update object state
    usb_mixers->usb_mixSel[mixer].state[dst] = src;

    // Update local object strings 
    // TODO we don't really need to store strings since we can look them up...*/
    strcpy(usb_mixers->usb_mixer[mixer].input_names[dst], usb_mixers->usb_mixSel[mixer].inputStrings[src]);
}

double usb_mixer_get_value(unsigned int mixer, unsigned int nodeId) 
{
    return (double)usb_mixers->usb_mixer[mixer].nodes[nodeId].weight;
}

double usb_mixer_get_res(unsigned int mixer, unsigned int nodeId) 
{
    return (double)usb_mixers->usb_mixer[mixer].nodes[nodeId].res;
}

double usb_mixer_get_min(unsigned int mixer, unsigned int nodeId) 
{
    return (double)usb_mixers->usb_mixer[mixer].nodes[nodeId].min;
}

double usb_mixer_get_max(unsigned int mixer, unsigned int nodeId) 
{
    return (double)usb_mixers->usb_mixer[mixer].nodes[nodeId].max;
}

int usb_mixer_set_value(unsigned int mixer, unsigned int nodeId, double val) 
{
    /* check if update required */
    if(usb_mixers->usb_mixer[mixer].nodes[nodeId].weight != val)
    {
        /* update local object */
        usb_mixers->usb_mixer[mixer].nodes[nodeId].weight= val;

        /* write to device */
        short value = (short) (val * 256);
       
        unsigned char cs = 0; /* Device doesnt use CS for setting/getting mixer nodes */ 
        unsigned char cn = nodeId & 0xff;
        usb_audio_class_set(CUR, cs, cn, usb_mixers->usb_mixer[mixer].id,  2, (unsigned char *)&value);
        
    }
    return 0;
}

int usb_mixer_get_range(unsigned int mixer, unsigned int mixer_unit, int *min, int *max, int *res) 
{
    // range 0x02
    return 0;
}

int usb_get_usb_channel_map(int channel)
{
    return usb_mixers->usbChannelMap.map[channel].cur;
}

int usb_get_aud_channel_map(int channel)
{
    return usb_mixers->audChannelMap.map[channel].cur;
}

int usb_set_aud_channel_map(int channel, int val)
{
    /* Check if update required */
    if(usb_mixers->audChannelMap.map[channel].cur != val)
    {
        /* Update local object */
        usb_mixers->audChannelMap.map[channel].cur = val;
    
        /* Write setting to dev */    
        dev_set_channel_map(channel, val, ID_XU_OUT);
    }
    return 0;
}

int usb_set_usb_channel_map(int channel, int val)
{
    /* Check if update required */
    if(usb_mixers->usbChannelMap.map[channel].cur != val)
    {
        /* Update local object */
        usb_mixers->usbChannelMap.map[channel].cur = val;
    
        /* Write setting to dev */    
        dev_set_channel_map(channel, val, ID_XU_IN);
    }
    return 0;
}

enum usb_chan_type usb_get_aud_channel_map_type(int channel)
{
  return usb_mixers->audChannelMap.map[channel].ctype;
}

enum usb_chan_type usb_get_usb_channel_map_type(int channel) 
{
  return usb_mixers->usbChannelMap.map[channel].ctype;
}

char *usb_get_aud_channel_map_name(int channel) 
{
    return usb_mixers->audChannelMap.map[channel].name;
}

char *usb_get_usb_channel_map_name(int channel) 
{
    return usb_mixers->usbChannelMap.map[channel].name;
}

int usb_get_aud_channel_map_num_outputs()
{
    return usb_mixers->audChannelMap.numOutputs;
}
int usb_get_usb_channel_map_num_outputs()
{
    return usb_mixers->usbChannelMap.numOutputs;
}

int usb_get_aud_channel_map_num_inputs()
{
    return usb_mixers->audChannelMap.numInputs;
}
int usb_get_usb_channel_map_num_inputs()
{
    return usb_mixers->usbChannelMap.numInputs;
}
