// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/**
 * @brief   Implements endpoint zero for an USB Audio 1.0/2.0 device
 * @author  Ross Owen, XMOS Semiconductor
 */

#include <xs1.h>
#include <safestring.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <xclib.h>
#include <xassert.h>
#include "xua.h"

#if XUA_USB_EN
#include "xud_device.h"          /* Standard descriptor requests */
#include "dfu_types.h"
#include "usbaudio20.h"          /* Defines from USB Audio 2.0 spec */
#include "xua_ep0_descriptors.h" /* This devices descriptors */
#include "xua_commands.h"
#include "audiostream.h"
#include "hostactive.h"
#include "vendorrequests.h"
#include "xc_ptr.h"
#include "xua_ep0_uacreqs.h"

#if XUA_OR_STATIC_HID_ENABLED
#include "hid.h"
#include "xua_hid.h"
#include "xua_hid_report.h"
#endif

#if DSD_CHANS_DAC > 0
#include "dsd_support.h"
#endif

#define DEBUG_UNIT XUA_EP0

#ifndef DEBUG_PRINT_ENABLE_XUA_EP0
    #define DEBUG_PRINT_ENABLE_XUA_EP0 0
#endif // DEBUG_PRINT_ENABLE_XUA_EP0

#include "debug_print.h"
#include "xua_usb_params_funcs.h"

#ifndef __XC__
/* Support for xCORE  channels in C */
#define null 0
#define outuint(c, x)   asm ("out res[%0], %1" :: "r" (c), "r" (x))
#define chkct(c, x)     asm ("chkct res[%0], %1" :: "r" (c), "r" (x))
#endif

/* Some warnings.... */

/* Windows does not have a built in DFU driver (windows will prompt), so warn that DFU will not be functional in Audio 1.0 mode */
#ifndef FORCE_UAC1_DFU
#if ((AUDIO_CLASS == 1) || (AUDIO_CLASS_FALLBACK)) && defined(DFU)
#warning DFU will not be enabled in AUDIO 1.0 mode due to Windows requesting driver
#endif
#endif // FORCE_UAC1_DFU

/* MIDI not supported in Audio 1.0 mode */
#if ((AUDIO_CLASS == 1) || (AUDIO_CLASS_FALLBACK)) && defined(MIDI)
#warning MIDI is currently not supported and will not be enabled in AUDIO 1.0 mode
#endif

/* If DFU_PID not defined, standard PID used.. this is probably what we want.. */
#ifndef DFU_PID
#warning DFU_PID not defined, Using PID_AUDIO_2. This is probably fine!
#endif

#if (XUA_DFU_EN == 1)
#include "xua_dfu.h"
#define DFU_IF_NUM INPUT_INTERFACES + OUTPUT_INTERFACES + MIDI_INTERFACES + 1
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

#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

unsigned int DFU_mode_active = 0;         // 0 - App active, 1 - DFU active

/* Global volume and mute tables */
int volsOut[NUM_USB_CHAN_OUT + 1];
unsigned int mutesOut[NUM_USB_CHAN_OUT + 1];

int volsIn[NUM_USB_CHAN_IN + 1];
unsigned int mutesIn[NUM_USB_CHAN_IN + 1];

#if (MIXER)
short mixer1Weights[MIX_INPUTS * MAX_MIX_COUNT];

//unsigned char channelMap[NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT];
/* Mapping of channels to output audio interfaces */
unsigned char channelMapAud[NUM_USB_CHAN_OUT];

/* Mapping of channels to USB host */
unsigned char channelMapUsb[NUM_USB_CHAN_IN];

/* Mapping of channels to Mixer(s) */
unsigned char mixSel[MAX_MIX_COUNT][MIX_INPUTS];
#endif

int min(int x, int y);

/* Global current device config var*/
extern unsigned char g_currentConfig;

/* Global endpoint status arrays - declared in usb_device.xc */
extern unsigned char g_interfaceAlt[];

/* We remember which streaming alt we were last using to avoid interrupting the I2S as best we can */
/* Note, we cannot simply use g_interfaceAlt[] since this also records using the zero bandwidth alt */
unsigned g_curStreamAlt_Out = 0;
unsigned g_curStreamAlt_In = 0;

/* Global variable for current USB bus speed (i.e. FS/HS) */
#if (AUDIO_CLASS == 2)
XUD_BusSpeed_t g_curUsbSpeed = XUD_SPEED_HS;
#else
XUD_BusSpeed_t g_curUsbSpeed = XUD_SPEED_FS;
#endif

/* Global variables for current USB Vendor and Product strings */
char g_vendor_str[XUA_MAX_STR_LEN] = VENDOR_STR;
#if (AUDIO_CLASS == 2)
char g_product_str[XUA_MAX_STR_LEN] = PRODUCT_STR_A2;
#else
char g_product_str[XUA_MAX_STR_LEN] = PRODUCT_STR_A1;
#endif

/* Global variable for current USB Serial Number strings */
char g_serial_str[XUA_MAX_STR_LEN] = SERIAL_STR;

/* Subslot */
const unsigned g_subSlot_Out_HS[OUTPUT_FORMAT_COUNT]    = {HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES,
#if(OUTPUT_FORMAT_COUNT > 1)
                                                            HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES,
#endif
#if(OUTPUT_FORMAT_COUNT > 2)
                                                            HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES
#endif
};

const unsigned g_subSlot_Out_FS[OUTPUT_FORMAT_COUNT]    = {FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES,
#if(OUTPUT_FORMAT_COUNT > 1)
                                                            FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES,
#endif
#if(OUTPUT_FORMAT_COUNT > 2)
                                                            FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES
#endif
};

const unsigned g_subSlot_In_HS[INPUT_FORMAT_COUNT]      = {HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES,
#if(INPUT_FORMAT_COUNT > 1)
                                                            HS_STREAM_FORMAT_INPUT_2_SUBSLOT_BYTES,
#endif
#if(INPUT_FORMAT_COUNT > 2)
                                                            HS_STREAM_FORMAT_INPUT_3_SUBSLOT_BYTES
#endif
};

const unsigned g_subSlot_In_FS[INPUT_FORMAT_COUNT]      = {FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES,
#if(INPUT_FORMAT_COUNT > 1)
                                                            FS_STREAM_FORMAT_INPUT_2_SUBSLOT_BYTES,
#endif
#if(INPUT_FORMAT_COUNT > 2)
                                                            FS_STREAM_FORMAT_INPUT_3_SUBSLOT_BYTES
#endif
};

/* Sample Resolution */
const unsigned g_sampRes_Out_HS[OUTPUT_FORMAT_COUNT]    = {HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS,
#if(OUTPUT_FORMAT_COUNT > 1)
                                                            HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS,
#endif
#if(OUTPUT_FORMAT_COUNT > 2)
                                                            HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
#endif
};

const unsigned g_sampRes_Out_FS[OUTPUT_FORMAT_COUNT]    = {FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS,
#if(OUTPUT_FORMAT_COUNT > 1)
                                                            FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS,
#endif
#if(OUTPUT_FORMAT_COUNT > 2)
                                                            FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
#endif
};

const unsigned g_sampRes_In_HS[INPUT_FORMAT_COUNT]     = {HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS,
#if(INPUT_FORMAT_COUNT > 1)
                                                            HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS,
#endif
#if(INPUT_FORMAT_COUNT > 2)
                                                            HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
#endif
};

const unsigned g_sampRes_In_FS[INPUT_FORMAT_COUNT]     = {FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS,
#if(INPUT_FORMAT_COUNT > 1)
                                                            FS_STREAM_FORMAT_INPUT_2_RESOLUTION_BITS,
#endif
#if(INPUT_FORMAT_COUNT > 2)
                                                            FS_STREAM_FORMAT_INPUT_3_RESOLUTION_BITS
#endif
};

/* Data Format (Note, this is shared over HS and FS */
const unsigned g_dataFormat_Out[OUTPUT_FORMAT_COUNT]    = {STREAM_FORMAT_OUTPUT_1_DATAFORMAT,
#if(OUTPUT_FORMAT_COUNT > 1)
                                                            STREAM_FORMAT_OUTPUT_2_DATAFORMAT,
#endif
#if(OUTPUT_FORMAT_COUNT > 2)
                                                            STREAM_FORMAT_OUTPUT_3_DATAFORMAT
#endif
};

const unsigned g_dataFormat_In[INPUT_FORMAT_COUNT]      = {STREAM_FORMAT_INPUT_1_DATAFORMAT,
#if(INPUT_FORMAT_COUNT > 1)
                                                            STREAM_FORMAT_INPUT_2_DATAFORMAT,
#endif
#if(INPUT_FORMAT_COUNT > 2)
                                                            STREAM_FORMAT_INPUT_3_DATAFORMAT
#endif
};

/* Channel count */
/* Note, currently only input changes.. */
const unsigned g_chanCount_In_HS[INPUT_FORMAT_COUNT]       = {HS_STREAM_FORMAT_INPUT_1_CHAN_COUNT,
#if(INPUT_FORMAT_COUNT > 1)
                                                            HS_STREAM_FORMAT_INPUT_2_CHAN_COUNT,
#endif
#if(INPUT_FORMAT_COUNT > 2)
                                                            HS_STREAM_FORMAT_INPUT_3_CHAN_COUNT
#endif
};

XUD_ep ep0_out;
XUD_ep ep0_in;

void XUA_Endpoint0_setVendorId(unsigned short vid) {
#if (AUDIO_CLASS == 1)
    devDesc_Audio1.idVendor = vid;
#else
    devDesc_Audio2.idVendor = vid;
#endif // AUDIO_CLASS == 1}
}

#if (MIXER)
void InitLocalMixerState()
{
    for (int i = 0; i < MIX_INPUTS * MAX_MIX_COUNT; i++)
    {
        mixer1Weights[i] = 0x8001; //-inf
    }

    /* Configure default connections */
    for (int i = 0; i < MAX_MIX_COUNT; i++)
    {
        mixer1Weights[(i * MAX_MIX_COUNT) + i] = 0;
    }

#if NUM_USB_CHAN_OUT > 0
    /* Setup up audio output channel mapping */
    for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
    {
       channelMapAud[i] = i;
    }
#endif

#if NUM_USB_CHAN_IN > 0
    for(int i = 0; i < NUM_USB_CHAN_IN; i++)
    {
       channelMapUsb[i] = i + NUM_USB_CHAN_OUT;
    }
#endif

    /* Init mixer inputs */
    for(int j = 0; j < MAX_MIX_COUNT; j++)
        for(int i = 0; i < MIX_INPUTS; i++)
        {
            mixSel[j][i] = i;
        }
}
#endif

void concatenateAndCopyStrings(char* string1, char* string2, char* string_buffer) {
    debug_printf("concatenateAndCopyStrings() for \"%s\" and \"%s\"\n", string1, string2);

    memset(string_buffer, '\0', strlen(string_buffer));

    uint32_t remaining_buffer_size = MIN(strlen(string1), XUA_MAX_STR_LEN-1);
    strncpy(string_buffer, string1, remaining_buffer_size);
    uint32_t total_string_size = MIN(strlen(string1)+strlen(string2), XUA_MAX_STR_LEN-1);
    if (total_string_size==XUA_MAX_STR_LEN-1) {
        remaining_buffer_size =  XUA_MAX_STR_LEN-1-strlen(string1);
    } else {
        remaining_buffer_size = strlen(string2);
    }

    strncat(string_buffer, string2, remaining_buffer_size);
    debug_printf("concatenateAndCopyStrings() creates \"%s\"\n", string_buffer);
}

void XUA_Endpoint0_setStrTable() {

    // update Vendor strings
    concatenateAndCopyStrings(g_vendor_str, "", g_strTable.vendorStr);
#if (AUDIO_CLASS == 2)
    concatenateAndCopyStrings(g_vendor_str, " Clock Selector", g_strTable.clockSelectorStr);
    concatenateAndCopyStrings(g_vendor_str, " Internal Clock", g_strTable.internalClockSourceStr);
#endif
#if (XUA_SPDIF_RX_EN)
    concatenateAndCopyStrings(g_vendor_str, " S/PDIF Clock", g_strTable.spdifClockSourceStr);
#endif
#if (XUA_ADAT_RX_EN)
    concatenateAndCopyStrings(g_vendor_str, " ADAT Clock", g_strTable.adatClockSourceStr);
#endif
#if (XUA_DFU_EN == 1)
    concatenateAndCopyStrings(g_vendor_str, " DFU", g_strTable.dfuStr);
#endif
#ifdef USB_CONTROL_DESCS
    concatenateAndCopyStrings(g_vendor_str, " Control", g_strTable.ctrlStr);
#endif
#ifdef MIDI
    concatenateAndCopyStrings(g_vendor_str, " MIDI Out", g_strTable.midiOutStr);
    concatenateAndCopyStrings(g_vendor_str, " MIDI In", g_strTable.midiInStr);
#endif
    // update product strings
#if (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)
    concatenateAndCopyStrings(g_product_str, "", g_strTable.productStr_Audio1);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.outputInterfaceStr_Audio1);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.inputInterfaceStr_Audio1);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.usbInputTermStr_Audio1);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.usbOutputTermStr_Audio1);
#elif (AUDIO_CLASS == 2)
    concatenateAndCopyStrings(g_product_str, "", g_strTable.productStr_Audio2);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.outputInterfaceStr_Audio2);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.inputInterfaceStr_Audio2);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.usbInputTermStr_Audio2);
    concatenateAndCopyStrings(g_product_str, "", g_strTable.usbOutputTermStr_Audio2);
#endif

    // update Serial strings
    concatenateAndCopyStrings(g_serial_str, "", g_strTable.serialStr);
}

void XUA_Endpoint0_setVendorStr(char* vendor_str) {
    debug_printf("XUA_Endpoint0_setVendorStr() with string %s\n", vendor_str);
    concatenateAndCopyStrings(vendor_str, "", g_vendor_str);
}

void XUA_Endpoint0_setProductStr(char* product_str) {
    debug_printf("XUA_Endpoint0_setProductStr() with string %s\n", product_str);
    concatenateAndCopyStrings(product_str, "", g_product_str);
}

void XUA_Endpoint0_setSerialStr(char* serial_str) {
    debug_printf("XUA_Endpoint0_setSerialStr() with string %s\n", serial_str);
    concatenateAndCopyStrings(serial_str, "", g_serial_str);
}

char* XUA_Endpoint0_getVendorStr() {
    return g_strTable.vendorStr;
}

char* XUA_Endpoint0_getProductStr() {
    #if (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS == 1)
    return g_strTable.productStr_Audio1;
    #elif (AUDIO_CLASS == 2)
    return g_strTable.productStr_Audio2;
    #endif
}

char* XUA_Endpoint0_getSerialStr() {
    return g_strTable.serialStr;
}

void XUA_Endpoint0_setProductId(unsigned short pid) {
#if (AUDIO_CLASS == 1)
    devDesc_Audio1.idProduct = pid;
#else
    devDesc_Audio2.idProduct = pid;
#endif // AUDIO_CLASS == 1}
}

unsigned short XUA_Endpoint0_getVendorId() {
    unsigned short vid;
#if (AUDIO_CLASS == 1)
    vid = devDesc_Audio1.idVendor;
#else
    vid = devDesc_Audio2.idVendor;
#endif // AUDIO_CLASS == 1}
    return vid;
}

unsigned short XUA_Endpoint0_getProductId() {
    unsigned short pid;
#if (AUDIO_CLASS == 1)
    pid = devDesc_Audio1.idProduct;
#else
    pid = devDesc_Audio2.idProduct;
#endif // AUDIO_CLASS == 1}
    return pid;
}

unsigned short XUA_Endpoint0_getBcdDevice() {
    unsigned short bcd;
#if (AUDIO_CLASS == 1)
    bcd = devDesc_Audio1.bcdDevice;
#else
    bcd = devDesc_Audio2.bcdDevice;
#endif // AUDIO_CLASS == 1}
    return bcd;
}

void XUA_Endpoint0_setBcdDevice(unsigned short bcd) {
#if (AUDIO_CLASS == 1)
    devDesc_Audio1.bcdDevice = bcd;
#else
    devDesc_Audio2.bcdDevice = bcd;
#endif // AUDIO_CLASS == 1}
}

#if defined(__static_hid_report_h_exists__)
#define hidReportDescriptorLength (sizeof(hidReportDescriptorPtr))
static unsigned char hidReportDescriptorPtr[] = {
#include "static_hid_report.h"
};
#endif



void XUA_Endpoint0_init(chanend c_ep0_out, chanend c_ep0_in, NULLABLE_RESOURCE(chanend, c_audioControl),
    chanend c_mix_ctl, chanend c_clk_ctl, chanend c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
{
    ep0_out = XUD_InitEp(c_ep0_out);
    ep0_in  = XUD_InitEp(c_ep0_in);

    XUA_Endpoint0_setStrTable();

    VendorRequests_Init(VENDOR_REQUESTS_PARAMS);

#if (MIXER)
    /* Set up mixer default state */
    InitLocalMixerState();
#endif

#ifdef VENDOR_AUDIO_REQS
    VendorAudioRequestsInit(c_audioControl, c_mix_ctl, c_clk_ctl);
#endif

#if (XUA_DFU_EN == 1)
    /* Check if device has started in DFU mode */
    if (DFUReportResetState(null))
    {
        assert(((unsigned)c_audioControl != 0) && msg("DFU not supported when c_audioControl is null"));

        /* Stop audio */
        outuint(c_audioControl, SET_SAMPLE_FREQ);
        outuint(c_audioControl, AUDIO_STOP_FOR_DFU);
        /* No Handshake */
        DFU_mode_active = 1;
    }
#endif

#ifdef XUA_USB_DESCRIPTOR_OVERWRITE_RATE_RES //change USB descriptor frequencies and bit resolution values here

    const int num_of_usb_descriptor_freq = 3; //This should be =3 according to the comments "using a value of <=2 or > 7 for num_freqs_a1 causes enumeration issues on Windows" in xua_ep0_descriptors.h

#if( 0 < NUM_USB_CHAN_IN )

    cfgDesc_Audio1[USB_AS_IN_INTERFACE_DESCRIPTOR_OFFSET_SUB_FRAME] = get_device_to_usb_bit_res() >> 3; //sub frame rate = bit rate /8
    cfgDesc_Audio1[USB_AS_IN_INTERFACE_DESCRIPTOR_OFFSET_SUB_FRAME + 1] = (get_device_to_usb_bit_res() & 0xff); //bit resolution

    for(int i=0;i<num_of_usb_descriptor_freq;i++)
    {
        cfgDesc_Audio1[USB_AS_IN_INTERFACE_DESCRIPTOR_OFFSET_FREQ + 3*i] = get_device_to_usb_rate() & 0xff;
        cfgDesc_Audio1[USB_AS_IN_INTERFACE_DESCRIPTOR_OFFSET_FREQ + 3*i + 1] = (get_device_to_usb_rate() & 0xff00)>> 8;
        cfgDesc_Audio1[USB_AS_IN_INTERFACE_DESCRIPTOR_OFFSET_FREQ + 3*i + 2] = (get_device_to_usb_rate() & 0xff0000)>> 16;
    }
    cfgDesc_Audio1[USB_AS_IN_EP_DESCRIPTOR_OFFSET_MAXPACKETSIZE] = ((get_device_to_usb_bit_res() >> 3) * MAX_PACKET_SIZE_MULT_IN_FS) & 0xff; //max packet size
    cfgDesc_Audio1[USB_AS_IN_EP_DESCRIPTOR_OFFSET_MAXPACKETSIZE + 1] = (((get_device_to_usb_bit_res() >> 3)  * MAX_PACKET_SIZE_MULT_IN_FS) & 0xff00) >> 8; //max packet size

#endif // NUM_USB_CHAN_IN

#if( 0 < NUM_USB_CHAN_OUT )

    cfgDesc_Audio1[USB_AS_OUT_INTERFACE_DESCRIPTOR_OFFSET_SUB_FRAME] = get_usb_to_device_bit_res() >> 3; //sub frame rate = bit rate /8
    cfgDesc_Audio1[USB_AS_OUT_INTERFACE_DESCRIPTOR_OFFSET_SUB_FRAME + 1] = (get_usb_to_device_bit_res() & 0xff); //bit resolution

    for(int i=0;i<num_of_usb_descriptor_freq;i++)
    {
        cfgDesc_Audio1[USB_AS_OUT_INTERFACE_DESCRIPTOR_OFFSET_FREQ + 3*i] = get_usb_to_device_rate() & 0xff;
        cfgDesc_Audio1[USB_AS_OUT_INTERFACE_DESCRIPTOR_OFFSET_FREQ + 3*i + 1] = (get_usb_to_device_rate() & 0xff00)>> 8;
        cfgDesc_Audio1[USB_AS_OUT_INTERFACE_DESCRIPTOR_OFFSET_FREQ + 3*i + 2] = (get_usb_to_device_rate() & 0xff0000)>> 16;
    }

    cfgDesc_Audio1[USB_AS_OUT_EP_DESCRIPTOR_OFFSET_MAXPACKETSIZE] = ((get_usb_to_device_bit_res() >> 3) * MAX_PACKET_SIZE_MULT_OUT_FS) & 0xff; //max packet size
    cfgDesc_Audio1[USB_AS_OUT_EP_DESCRIPTOR_OFFSET_MAXPACKETSIZE + 1] = (((get_usb_to_device_bit_res() >> 3)  * MAX_PACKET_SIZE_MULT_OUT_FS) & 0xff00) >> 8; //max packet size
#endif // NUM_USB_CHAN_OUT

#endif // XUA_USB_DESCRIPTOR_OVERWRITE_RATE_RES

#if XUA_OR_STATIC_HID_ENABLED
#if XUA_HID_ENABLED
    hidReportInit();
    hidPrepareReportDescriptor();

    size_t hidReportDescriptorLength = hidGetReportDescriptorLength();
#endif
    unsigned char hidReportDescriptorLengthLo =  hidReportDescriptorLength & 0xFF;
    unsigned char hidReportDescriptorLengthHi = (hidReportDescriptorLength & 0xFF00) >> 8;

#if( AUDIO_CLASS == 1 )
    cfgDesc_Audio1[USB_HID_DESCRIPTOR_OFFSET + HID_DESCRIPTOR_LENGTH_FIELD_OFFSET    ] = hidReportDescriptorLengthLo;
    cfgDesc_Audio1[USB_HID_DESCRIPTOR_OFFSET + HID_DESCRIPTOR_LENGTH_FIELD_OFFSET + 1] = hidReportDescriptorLengthHi;
#endif

    hidDescriptor[HID_DESCRIPTOR_LENGTH_FIELD_OFFSET    ] = hidReportDescriptorLengthLo;
    hidDescriptor[HID_DESCRIPTOR_LENGTH_FIELD_OFFSET + 1] = hidReportDescriptorLengthHi;

#endif // 0 < HID_CONTROLS

}

void XUA_Endpoint0_loop(XUD_Result_t result, USB_SetupPacket_t sp, chanend c_ep0_out, chanend c_ep0_in, NULLABLE_RESOURCE(chanend, c_audioControl),
    chanend c_mix_ctl, chanend c_clk_ctl, chanend c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
{
 if (result == XUD_RES_OKAY)
    {
        result = XUD_RES_ERR;

        /* Inspect Request type and Receipient and direction */
        switch( (sp.bmRequestType.Direction << 7) | (sp.bmRequestType.Recipient ) | (sp.bmRequestType.Type << 5) )
        {
            case USB_BMREQ_H2D_STANDARD_INT:

                /* Over-riding USB_StandardRequests implementation */
                if(sp.bRequest == USB_SET_INTERFACE)
                {
                    switch (sp.wIndex)
                    {
                        /* Check for audio stream from host start/stop */
#if (NUM_USB_CHAN_OUT > 0) && (AUDIO_CLASS == 2)
                        case INTERFACE_NUMBER_AUDIO_OUTPUT:
                            /* Check the alt is in range */
                            if(sp.wValue <= OUTPUT_FORMAT_COUNT)
                            {
                                /* Alt 0 is stream stop */
                                /* Only send change if we need to */
                                if((sp.wValue > 0) && (g_curStreamAlt_Out != sp.wValue))
                                {
                                    assert((c_audioControl != null) && msg("Format change not supported when c_audioControl is null"));
                                    g_curStreamAlt_Out = sp.wValue;

                                    /* Send format of data onto buffering */
                                    outuint(c_audioControl, SET_STREAM_FORMAT_OUT);
                                    outuint(c_audioControl, g_dataFormat_Out[sp.wValue-1]);        /* Data format (PCM/DSD) */

                                    if(g_curUsbSpeed == XUD_SPEED_HS)
                                    {
                                        outuint(c_audioControl, NUM_USB_CHAN_OUT);                 /* Channel count */
                                        outuint(c_audioControl, g_subSlot_Out_HS[sp.wValue-1]);    /* Subslot */
                                        outuint(c_audioControl, g_sampRes_Out_HS[sp.wValue-1]);    /* Resolution */
                                    }
                                    else
                                    {
                                        outuint(c_audioControl, NUM_USB_CHAN_OUT_FS);              /* Channel count */
                                        outuint(c_audioControl, g_subSlot_Out_FS[sp.wValue-1]);    /* Subslot */
                                        outuint(c_audioControl, g_sampRes_Out_FS[sp.wValue-1]);    /* Resolution */
                                    }

                                    /* Handshake */
                                    chkct(c_audioControl, XS1_CT_END);
                                }
                            }
                            break;
#endif

#if (NUM_USB_CHAN_IN > 0) && (AUDIO_CLASS == 2)
                        case INTERFACE_NUMBER_AUDIO_INPUT:
                            /* Check the alt is in range */
                            if(sp.wValue <= INPUT_FORMAT_COUNT)
                            {
                                /* Alt 0 is stream stop */
                                /* Only send change if we need to */
                                if((sp.wValue > 0) && (g_curStreamAlt_In != sp.wValue))
                                {
                                    assert((c_audioControl != null) && msg("Format change not supported when c_audioControl is null"));
                                    g_curStreamAlt_In = sp.wValue;

                                    /* Send format of data onto buffering */
                                    outuint(c_audioControl, SET_STREAM_FORMAT_IN);
                                    outuint(c_audioControl, g_dataFormat_In[sp.wValue-1]);        /* Data format (PCM/DSD) */

                                    if(g_curUsbSpeed == XUD_SPEED_HS)
                                    {
                                        outuint(c_audioControl, g_chanCount_In_HS[sp.wValue-1]);  /* Channel count */
                                        outuint(c_audioControl, g_subSlot_In_HS[sp.wValue-1]);    /* Subslot */
                                        outuint(c_audioControl, g_sampRes_In_HS[sp.wValue-1]);    /* Resolution */
                                    }
                                    else
                                    {
                                        outuint(c_audioControl, NUM_USB_CHAN_IN_FS);               /* Channel count */
                                        outuint(c_audioControl, g_subSlot_In_FS[sp.wValue-1]);     /* Subslot */
                                        outuint(c_audioControl, g_sampRes_In_FS[sp.wValue-1]);     /* Resolution */
                                    }

                                    /* Wait for handshake */
                                    chkct(c_audioControl, XS1_CT_END);
                                }
                            }
                            break;
#endif

#ifdef IAP_EA_NATIVE_TRANS
                        case INTERFACE_NUMBER_IAP_EA_NATIVE_TRANS:
                            /* Check the alt is in range */
                            if (sp.wValue <= IAP_EA_NATIVE_TRANS_ALT_COUNT)
                            {
                                /* Reset all state of endpoints associated with this interface
                                 * when changing an alternative setting. See USB 2.0 Spec 9.1.1.5 */
                                XUD_ResetEpStateByAddr(ENDPOINT_ADDRESS_IN_IAP_EA_NATIVE_TRANS);
                                XUD_ResetEpStateByAddr(ENDPOINT_ADDRESS_OUT_IAP_EA_NATIVE_TRANS);

                                /* Send selected Alt interface number onto EA Native EP manager */
                                outuint(c_EANativeTransport_ctrl, (unsigned)sp.wValue);

                                /* Wait for handshake */
                                chkct(c_EANativeTransport_ctrl, XS1_CT_END);
                            }
                            break;
#endif
                        default:
                            /* Unhandled interface */
                            break;
                    }

#if (NUM_USB_CHAN_OUT > 0) && (NUM_USB_CHAN_IN > 0)
                    unsigned num_input_interfaces = g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT];
                    unsigned num_output_interfaces = g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT];
                    if (sp.wIndex == INTERFACE_NUMBER_AUDIO_INPUT)
                    {
                        // in: 0 -> 1
                        if (sp.wValue && !num_input_interfaces)
                        {
                            UserAudioInputStreamStart();
                            if (!num_output_interfaces)
                            {
                                UserAudioStreamStart();
                            }
                        }
                        // in: 1 -> 0
                        else if (!sp.wValue && num_input_interfaces)
                        {
                            UserAudioInputStreamStop();
                            if (!num_output_interfaces)
                            {
                                UserAudioStreamStop();
                            }
                        }
                    }
                    else if (sp.wIndex == INTERFACE_NUMBER_AUDIO_OUTPUT)
                    {
                        // out: 0 -> 1
                        if (sp.wValue && !num_output_interfaces)
                        {
                            UserAudioOutputStreamStart();
                            if (!num_input_interfaces)
                            {
                                UserAudioStreamStart();
                            }
                        }
                        // out: 1 -> 0
                        else if (!sp.wValue && num_output_interfaces)
                        {
                            UserAudioOutputStreamStop();
                            if (!num_input_interfaces)
                            {
                                UserAudioStreamStop();
                            }
                        }
                    }
#elif (NUM_USB_CHAN_OUT > 0)
                    if(sp.wIndex == INTERFACE_NUMBER_AUDIO_OUTPUT)
                    {
                        if(sp.wValue && (!g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT]))
                        {
                            /* if start and not currently running */
                            UserAudioStreamStart();
                            UserAudioOutputStreamStart();
                        }
                        else if (!sp.wValue && g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT])
                        {
                            /* if stop and currently running */
                            UserAudioStreamStop();
                            UserAudioOutputStreamStop();
                        }
                    }
#elif (NUM_USB_CHAN_IN > 0)
                    if(sp.wIndex == INTERFACE_NUMBER_AUDIO_INPUT)
                    {
                        if(sp.wValue && (!g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT]))
                        {
                            /* if start and not currently running */
                            UserAudioStreamStart();
                            UserAudioInputStreamStart();
                        }
                        else if (!sp.wValue && g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT])
                        {
                            /* if stop and currently running */
                            UserAudioStreamStop();
                            UserAudioInputStreamStop();
                        }
                    }
#endif
                } /* if(sp.bRequest == SET_INTERFACE) */

                break; /* BMREQ_H2D_STANDARD_INT */

            case USB_BMREQ_D2H_STANDARD_INT:

                switch(sp.bRequest)
                {
#if XUA_OR_STATIC_HID_ENABLED
                    case USB_GET_DESCRIPTOR:

                        /* Check what inteface request is for */
                        if(sp.wIndex == INTERFACE_NUMBER_HID)
                        {
                            /* High byte of wValue is descriptor type */
                            unsigned descriptorType = sp.wValue & 0xff00;

                            switch (descriptorType)
                            {
                                case HID_HID:
                                    {
                                        /* Return HID Descriptor */
                                         result = XUD_DoGetRequest(ep0_out, ep0_in, hidDescriptor,
                                            hidDescriptor[0], sp.wLength);
                                    }
                                    break;
                                case HID_REPORT:
                                    {
                                        /* Return HID report descriptor */
#if XUA_HID_ENABLED
                                        unsigned char* hidReportDescriptorPtr;
                                        hidReportDescriptorPtr = hidGetReportDescriptor();
                                        size_t hidReportDescriptorLength = hidGetReportDescriptorLength();
#endif
                                        result = XUD_DoGetRequest(ep0_out, ep0_in, hidReportDescriptorPtr,
                                            hidReportDescriptorLength, sp.wLength);
                                    }
                                    break;
                            }
                        }
                        break;
#endif
                    default:
                        break;
               }
               break;

            /* Recipient: Device */
            case USB_BMREQ_H2D_STANDARD_DEV:

                /* Inspect for actual request */
                switch( sp.bRequest )
                {
                    /* Standard request: SetConfiguration */
                    /* Overriding implementation in USB_StandardRequests */
                    case USB_SET_CONFIGURATION:

                        //if(g_current_config == 1)
                        {
                            /* Consider host active with valid driver at this point */
                            UserHostActive(1);
                        }

                        /* We want to run USB_StandardsRequests() implementation also. Don't modify result
                         * and don't call XUD_DoSetRequestStatus() */
                        break;

                    default:
                        //Unknown device request"
                        break;
                }
                break;

            /* Audio Class 1.0 Sampling Freqency Requests go to Endpoint */
            case USB_BMREQ_H2D_CLASS_EP:
            case USB_BMREQ_D2H_CLASS_EP:
                {
                    unsigned epNum = sp.wIndex & 0xff;

                    if ((epNum == ENDPOINT_ADDRESS_OUT_AUDIO) || (epNum == ENDPOINT_ADDRESS_IN_AUDIO))
                    {
#if (AUDIO_CLASS == 2) && (AUDIO_CLASS_FALLBACK)
                        if(g_curUsbSpeed == XUD_SPEED_FS)
                        {
                            result = AudioEndpointRequests_1(ep0_out, ep0_in, &sp, c_audioControl, c_mix_ctl, c_clk_ctl);
                        }
#elif (AUDIO_CLASS==1)
                        result = AudioEndpointRequests_1(ep0_out, ep0_in, &sp, c_audioControl, c_mix_ctl, c_clk_ctl);
#endif
                    }

                }
                break;

            case USB_BMREQ_H2D_CLASS_INT:
            case USB_BMREQ_D2H_CLASS_INT:
                {
                    unsigned interfaceNum = sp.wIndex & 0xff;
                    //unsigned request = (sp.bmRequestType.Recipient ) | (sp.bmRequestType.Type << 5);

                    /* TODO Check on return value retval =  */
#if (XUA_DFU_EN == 1)
                    unsigned DFU_IF = INTERFACE_NUMBER_DFU;

                    /* DFU interface number changes based on which mode we are currently running in */
                    if (DFU_mode_active)
                    {
                        DFU_IF = 0;
                    }

                    if (interfaceNum == DFU_IF)
                    {
                        int reset = 0;

                        /* If running in application mode stop audio */
                        /* Don't interupt audio for save and restore cmds */
                        if ((DFU_IF == INTERFACE_NUMBER_DFU) && (sp.bRequest != XMOS_DFU_SAVESTATE) &&
                            (sp.bRequest != XMOS_DFU_RESTORESTATE))
                        {
                            assert((c_audioControl != null) && msg("DFU not supported when c_audioControl is null"));
                            // Stop audio
                            outuint(c_audioControl, SET_SAMPLE_FREQ);
                            outuint(c_audioControl, AUDIO_STOP_FOR_DFU);
                            // Handshake
                            chkct(c_audioControl, XS1_CT_END);
                        }

                        /* This will return 1 if reset requested */
                        result = DFUDeviceRequests(ep0_out, &ep0_in, &sp, null, g_interfaceAlt[sp.wIndex], dfuInterface, &reset);

                        if(reset)
                        {
                            DFUDelay(DELAY_BEFORE_REBOOT_TO_DFU_MS * 100000);
                            device_reboot();
                        }
                    }
#endif
#if XUA_HID_ENABLED
                    if (interfaceNum == INTERFACE_NUMBER_HID)
                    {
                        result = HidInterfaceClassRequests(ep0_out, ep0_in, &sp);
                    }
#endif
                    /* Check for:   - Audio CONTROL interface request - always 0, note we check for DFU first
                     *              - Audio STREAMING interface request  (In or Out)
                     *              - Audio endpoint request (Audio 1.0 Sampling freq requests are sent to the endpoint)
                     */
                    if(((interfaceNum == 0) || (interfaceNum == 1) || (interfaceNum == 2))
#if (XUA_DFU_EN == 1)
                            && !DFU_mode_active
#endif
                        )
                    {
#if (AUDIO_CLASS == 2) && (AUDIO_CLASS_FALLBACK)
                        if(g_curUsbSpeed == XUD_SPEED_HS)
                        {
                            result = AudioClassRequests_2(ep0_out, ep0_in, &sp, c_audioControl, c_mix_ctl, c_clk_ctl);
                        }
                        else
                        {
                            result = AudioClassRequests_1(ep0_out, ep0_in, &sp, c_audioControl, c_mix_ctl, c_clk_ctl);
                        }
#elif (AUDIO_CLASS==2)
                        result = AudioClassRequests_2(ep0_out, ep0_in, &sp, c_audioControl, c_mix_ctl, c_clk_ctl);
#else
                        result = AudioClassRequests_1(ep0_out, ep0_in, &sp, c_audioControl, c_mix_ctl, c_clk_ctl);
#endif

#ifdef VENDOR_AUDIO_REQS
                        /* If result is ERR at this point, then request to audio interface not handled - handle vendor audio reqs */
                        if(result == XUD_RES_ERR)
                        {
                            result = VendorAudioRequests(ep0_out, ep0_in, sp.bRequest,
                                sp.wValue >> 8, sp.wValue & 0xff,
                                sp.wIndex >> 8, sp.bmRequestType.Direction,
                                c_audioControl, c_mix_ctl, c_clk_ctl);
                        }
#endif
                    }
                }
                break;

            default:
                break;
        }

    } /* if(result == XUD_RES_OKAY) */

    {
        if(result == XUD_RES_ERR)
        {
            /* Run vendor defined parsing/processing */
            /* Note, an interface might seem ideal here but this *must* be executed on the same
             * core sure to shared memory depandancy */
            result = VendorRequests(ep0_out, ep0_in, &sp VENDOR_REQUESTS_PARAMS_);
        }
    }

    if(result == XUD_RES_ERR)
    {
#if (XUA_DFU_EN == 1)
        if (!DFU_mode_active)
        {
#endif
#if (AUDIO_CLASS_FALLBACK) && (AUDIO_CLASS != 1)
            /* Return Audio 2.0 Descriptors with Audio 1.0 as fallback */
            result = USB_StandardRequests(ep0_out, ep0_in,
                (unsigned char*)&devDesc_Audio2, sizeof(devDesc_Audio2),
                (unsigned char*)&cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                (unsigned char*)&devDesc_Audio1, sizeof(devDesc_Audio1),
                cfgDesc_Audio1, sizeof(cfgDesc_Audio1),
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *),
                &sp, g_curUsbSpeed);
#elif FULL_SPEED_AUDIO_2
            /* Return Audio 2.0 Descriptors for high_speed and full-speed */

            /* Unfortunately we need to munge the descriptors a bit between full and high-speed */
            if(g_curUsbSpeed == XUD_SPEED_HS)
            {
                /* Modify Audio Class 2.0 Config descriptor for High-speed operation */
#if (NUM_USB_CHAN_OUT > 0)
                cfgDesc_Audio2.Audio_CS_Control_Int.Audio_Out_InputTerminal.bNrChannels = NUM_USB_CHAN_OUT;
#if (NUM_USB_CHAN_OUT > 0)
                cfgDesc_Audio2.Audio_Out_Format.bSubslotSize = HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format.bBitResolution = HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint.wMaxPacketSize = HS_STREAM_FORMAT_OUTPUT_1_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface.bNrChannels = NUM_USB_CHAN_OUT;
#endif
#if (OUTPUT_FORMAT_COUNT > 1)
                cfgDesc_Audio2.Audio_Out_Format_2.bSubslotSize = HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_2.bBitResolution = HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_2.wMaxPacketSize = HS_STREAM_FORMAT_OUTPUT_2_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_2.bNrChannels = NUM_USB_CHAN_OUT;
#endif

#if (OUTPUT_FORMAT_COUNT > 2)
                cfgDesc_Audio2.Audio_Out_Format_3.bSubslotSize = HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_3.bBitResolution = HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_3.wMaxPacketSize = HS_STREAM_FORMAT_OUTPUT_3_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_3.bNrChannels = NUM_USB_CHAN_OUT;
#endif
#endif
#if (NUM_USB_CHAN_IN > 0)
                cfgDesc_Audio2.Audio_CS_Control_Int.Audio_In_InputTerminal.bNrChannels = NUM_USB_CHAN_IN;
                cfgDesc_Audio2.Audio_In_Format.bSubslotSize = HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_In_Format.bBitResolution = HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_In_Endpoint.wMaxPacketSize = HS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_In_ClassStreamInterface.bNrChannels = NUM_USB_CHAN_IN;
#endif
            }
            else
            {
                /* Modify Audio Class 2.0 Config descriptor for Full-speed operation */
#if (NUM_USB_CHAN_OUT > 0)
                cfgDesc_Audio2.Audio_CS_Control_Int.Audio_Out_InputTerminal.bNrChannels = NUM_USB_CHAN_OUT_FS;
#if (NUM_USB_CHAN_OUT > 0)
                cfgDesc_Audio2.Audio_Out_Format.bSubslotSize = FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format.bBitResolution = FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint.wMaxPacketSize = FS_STREAM_FORMAT_OUTPUT_1_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface.bNrChannels = NUM_USB_CHAN_OUT_FS;
#endif
#if (OUTPUT_FORMAT_COUNT > 1)
                cfgDesc_Audio2.Audio_Out_Format_2.bSubslotSize = FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_2.bBitResolution = FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_2.wMaxPacketSize = FS_STREAM_FORMAT_OUTPUT_2_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_2.bNrChannels = NUM_USB_CHAN_OUT_FS;
#endif

#if (OUTPUT_FORMAT_COUNT > 2)
                cfgDesc_Audio2.Audio_Out_Format_3.bSubslotSize = FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_3.bBitResolution = FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_3.wMaxPacketSize = FS_STREAM_FORMAT_OUTPUT_3_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_3.bNrChannels = NUM_USB_CHAN_OUT_FS;
#endif
#endif
#if (NUM_USB_CHAN_IN > 0)
                cfgDesc_Audio2.Audio_CS_Control_Int.Audio_In_InputTerminal.bNrChannels = NUM_USB_CHAN_IN_FS;
                cfgDesc_Audio2.Audio_In_Format.bSubslotSize = FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_In_Format.bBitResolution = FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_In_Endpoint.wMaxPacketSize = FS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_In_ClassStreamInterface.bNrChannels = NUM_USB_CHAN_IN_FS;
#endif
            }

            result = USB_StandardRequests(ep0_out, ep0_in,
                (unsigned char*)&devDesc_Audio2, sizeof(devDesc_Audio2),
                (unsigned char*)&cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                null, 0,
                null, 0,
#ifdef __XC__
                g_strTable, sizeof(g_strTable), sp, null, g_curUsbSpeed);
#else
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, g_curUsbSpeed);
#endif
#elif (AUDIO_CLASS == 1)
            /* Return Audio 1.0 Descriptors in FS, should never be in HS! */
             result = USB_StandardRequests(ep0_out, ep0_in,
                null, 0,
                null, 0,
                (unsigned char*)&devDesc_Audio1, sizeof(devDesc_Audio1),
                cfgDesc_Audio1, sizeof(cfgDesc_Audio1),
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, g_curUsbSpeed);
#else
            /* Return Audio 2.0 Descriptors with Null device as fallback */
            result = USB_StandardRequests(ep0_out, ep0_in,
                (unsigned char*)&devDesc_Audio2, sizeof(devDesc_Audio2),
                (unsigned char*)&cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                devDesc_Null, sizeof(devDesc_Null),
                cfgDesc_Null, sizeof(cfgDesc_Null),
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, g_curUsbSpeed);
#endif
#if (XUA_DFU_EN == 1)
        }

        else
        {
            /* Running in DFU mode - always return same descs for DFU whether HS or FS */
            result = USB_StandardRequests(ep0_out, ep0_in,
                DFUdevDesc, sizeof(DFUdevDesc),
                DFUcfgDesc, sizeof(DFUcfgDesc),
                null, 0, /* Used same descriptors for full and high-speed */
                null, 0,
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, g_curUsbSpeed);
        }
#endif
    }

    if (result == XUD_RES_RST)
    {
#ifdef __XC__
        g_curUsbSpeed = XUD_ResetEndpoint(ep0_out, ep0_in);
#else
        g_curUsbSpeed = XUD_ResetEndpoint(ep0_out, &ep0_in);
#endif
        g_currentConfig = 0;
        g_curStreamAlt_Out = 0;
        g_curStreamAlt_In = 0;

#if (XUA_DFU_EN == 1)
        if (DFUReportResetState(null))
        {
            if (!DFU_mode_active)
            {
                DFU_mode_active = 1;
            }
        }
        else
        {
            if (DFU_mode_active)
            {
                DFU_mode_active = 0;

                /* Send reboot command */
                DFUDelay(DELAY_BEFORE_REBOOT_FROM_DFU_MS * 100000);
                device_reboot();
            }
        }
#endif
    }
}

/* Endpoint 0 function.  Handles all requests to the device */
void XUA_Endpoint0(chanend c_ep0_out, chanend c_ep0_in, NULLABLE_RESOURCE(chanend, c_audioControl),
    chanend c_mix_ctl, chanend c_clk_ctl, chanend c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
{
    USB_SetupPacket_t sp;
    XUA_Endpoint0_init(c_ep0_out, c_ep0_in, c_audioControl, c_mix_ctl, c_clk_ctl, c_EANativeTransport_ctrl, dfuInterface VENDOR_REQUESTS_PARAMS_);

    while(1)
    {
        /* Returns XUD_RES_OKAY for success, XUD_RES_RST for bus reset */
        XUD_Result_t result = USB_GetSetupPacket(ep0_out, ep0_in, &sp);
        XUA_Endpoint0_loop(result, sp, c_ep0_out, c_ep0_in, c_audioControl, c_mix_ctl, c_clk_ctl, c_EANativeTransport_ctrl, dfuInterface VENDOR_REQUESTS_PARAMS_);
    }
}
#endif /* XUA_USB_EN*/
