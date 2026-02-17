// Copyright 2011-2026 XMOS LIMITED.
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
#include <stdio.h>
#include "xua.h"

#if XUA_USB_EN
#include "msos_descriptors.h"
#include "msos_helpers.h"
#include "simple_ep0_msos_descriptors.h"
#include "xud_device.h"          /* Standard descriptor requests */
#include "dfu_types.h"
#include "usbaudio20.h"          /* Defines from USB Audio 2.0 spec */
#include "xua_ep0_descriptors.h" /* This devices descriptors */
#include "xua_ep0_msos_descriptors.h" /* Composite MSOS descriptors */
#include "xua_commands.h"
#include "audiostream.h"
#include "hostactive.h"
#include "suspend.h"
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

#ifdef __xua_user_ep0_decl_h_exists__
#include "xua_user_ep0_decl.h"
#endif

#define DEBUG_UNIT XUA_EP0

#ifndef DEBUG_PRINT_ENABLE_XUA_EP0
    #define DEBUG_PRINT_ENABLE_XUA_EP0 0
#endif // DEBUG_PRINT_ENABLE_XUA_EP0

#include "debug_print.h"
#include "xua_usb_params_funcs.h"

/* Support for xCORE  channels in C */
#define null 0
#define outuint(c, x)   asm ("out res[%0], %1" :: "r" (c), "r" (x))
#define outct(c, x)     asm ("outct res[%0], %1" :: "r" (c), "r" (x))
#define chkct(c, x)     asm ("chkct res[%0], %1" :: "r" (c), "r" (x))


/* If DFU_PID not defined, standard PID used.. this is probably what we want.. */
#ifndef DFU_PID
#warning DFU_PID not defined, Using PID_AUDIO_2. This is probably fine!
#endif

#include "xua_dfu_descs.h"
#if XUA_DFU_EN
#include "dfu_usb_requests.h"
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
XUD_BusSpeed_t g_curUsbSpeed = XUA_USB_BUS_SPEED;

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
const unsigned g_chanCount_In_HS[INPUT_FORMAT_COUNT]       = {HS_STREAM_FORMAT_INPUT_1_CHAN_COUNT,
#if(INPUT_FORMAT_COUNT > 1)
                                                            HS_STREAM_FORMAT_INPUT_2_CHAN_COUNT,
#endif
#if(INPUT_FORMAT_COUNT > 2)
                                                            HS_STREAM_FORMAT_INPUT_3_CHAN_COUNT
#endif
};

const unsigned g_chanCount_Out_HS[OUTPUT_FORMAT_COUNT]       = {HS_STREAM_FORMAT_OUTPUT_1_CHAN_COUNT,
#if(OUTPUT_FORMAT_COUNT > 1)
                                                            HS_STREAM_FORMAT_OUTPUT_2_CHAN_COUNT,
#endif
#if(OUTPUT_FORMAT_COUNT > 2)
                                                            HS_STREAM_FORMAT_OUTPUT_3_CHAN_COUNT
#endif
};

XUD_ep ep0_out;
XUD_ep ep0_in;

void XUA_Endpoint0_setVendorId(unsigned short vid) {
#if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
    devDesc_Audio2.idVendor = vid;
#endif
#if (XUA_AUDIO_CLASS_FS == 1)
    devDesc_Audio1.idVendor = vid;
#endif
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

void concatenateAndCopyStrings(char* string1, char* string2, char* destBuffer, size_t destLen)
{
    debug_printf("concatenateAndCopyStrings() for \"%s\" and \"%s\"\n", string1, string2);

    memset(destBuffer, '\0', strlen(destBuffer));

    /* Copy string1 into the destination buffer */
    strncat(destBuffer, string1, destLen);

    /* Cat what we can of string2 onto the destination buffer */
    strncat(destBuffer, string2, destLen - strlen(string1));

    debug_printf("concatenateAndCopyStrings() creates \"%s\"\n", destBuffer);
}

void XUA_Endpoint0_setVendorStr(char* vendorStr)
{
    debug_printf("XUA_Endpoint0_setVendorStr() with string %s\n", vendorStr);
    concatenateAndCopyStrings(vendorStr, "", g_strTable.vendorStr, sizeof(XUA_VENDOR_DEFAULT_STRING));

    /* Update other strings with the new vendorStr */
#if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
    concatenateAndCopyStrings(vendorStr, " Clock Selector", g_strTable.clockSelectorStr, sizeof(XUA_CLOCK_SELECTOR_DEFAULT_STRING));
    concatenateAndCopyStrings(vendorStr, " Internal Clock", g_strTable.internalClockSourceStr, sizeof(XUA_INTERNAL_CLOCK_SOURCE_DEFAULT_STRING));
#endif
#if (XUA_SPDIF_RX_EN)
    concatenateAndCopyStrings(vendorStr, " S/PDIF Clock", g_strTable.spdifClockSourceStr, sizeof(XUA_SPDIF_CLOCK_SOURCE_DEFAULT_STRING));
#endif
#if (XUA_ADAT_RX_EN)
    concatenateAndCopyStrings(vendorStr, " ADAT Clock", g_strTable.adatClockSourceStr, sizeof(XUA_ADAT_CLOCK_SOURCE_DEFAULT_STRING));
#endif
#if XUA_DFU_EN
    concatenateAndCopyStrings(vendorStr, " DFU", g_strTable.dfuStr, sizeof(XUA_DFU_DEFAULT_STRING));
#endif
#if XUA_USB_CONTROL_DESCS
    concatenateAndCopyStrings(vendorStr, " Control", g_strTable.ctrlStr, sizeof(XUA_CTRL_DEFAULT_STRING));
#endif
#ifdef MIDI
    concatenateAndCopyStrings(vendorStr, " MIDI Out", g_strTable.midiOutStr, sizeof(XUA_MIDI_OUT_DEFAULT_STRING));
    concatenateAndCopyStrings(vendorStr, " MIDI In", g_strTable.midiInStr, sizeof(XUA_MIDI_IN_DEFAULT_STRING));
#endif
}

/* Note, this writes both UAC1 and UAC2 product strings */
void XUA_Endpoint0_setProductStr(char* productStr)
{
    debug_printf("XUA_Endpoint0_setProductStr() with string %s\n", productStr);

#if (XUA_AUDIO_CLASS_FS == 1)
    concatenateAndCopyStrings(productStr, "", g_strTable.productStr_Audio1, sizeof(XUA_PRODUCT_A1_DEFAULT_STRING));
#endif
#if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
    concatenateAndCopyStrings(productStr, "", g_strTable.productStr_Audio2, sizeof(XUA_PRODUCT_A2_DEFAULT_STRING));
#endif
    /* Note, don't update outputInterfaceStr_Audio2, inputInterfaceStr_Audio2 etc as they are all pointing to the same string */
}

void XUA_Endpoint0_setSerialStr(char* serialStr)
{
    debug_printf("XUA_Endpoint0_setSerialStr() with string %s\n", serialStr);
    concatenateAndCopyStrings(serialStr, "", g_strTable.serialStr, sizeof(XUA_SERIAL_DEFAULT_STRING));
}

char* XUA_Endpoint0_getVendorStr()
{
    return g_strTable.vendorStr;
}

char* XUA_Endpoint0_getProductStr()
{
#if (XUA_AUDIO_CLASS_HS == 2)
    if (g_curUsbSpeed == XUD_SPEED_HS)
    {
        /* We only run UAC2 at HS */
        return g_strTable.productStr_Audio2;
    }
    else
#endif
    {
        /* In FS we could be running in UAC1 or UAC2 mode */
#if (XUA_AUDIO_CLASS_FS == 2)
        return g_strTable.productStr_Audio2;
#else
        return g_strTable.productStr_Audio1;
#endif
    }
}

char* XUA_Endpoint0_getSerialStr()
{
    return g_strTable.serialStr;
}


/* Note, this sets the same Product ID for both Audio Class 1.0 and Audio Class 2.0.
 * This is unlikely to be desirable.
 */
void XUA_Endpoint0_setProductId(unsigned short pid)
{
#if (XUA_AUDIO_CLASS_FS == 1)
    devDesc_Audio1.idProduct = pid;
#endif
#if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
    devDesc_Audio2.idProduct = pid;
#endif
}

/* FIXME this implementation is incorrect - needs to be based on bus-speed */
unsigned short XUA_Endpoint0_getVendorId() {
    unsigned short vid;
#if (AUDIO_CLASS == 1)
    vid = devDesc_Audio1.idVendor;
#else
    vid = devDesc_Audio2.idVendor;
#endif
    return vid;
}

/* FIXME this implementation is incorrect - needs to be based on current bus-speed */
unsigned short XUA_Endpoint0_getProductId() {
    unsigned short pid;
#if (AUDIO_CLASS == 1)
    pid = devDesc_Audio1.idProduct;
#else
    pid = devDesc_Audio2.idProduct;
#endif
    return pid;
}

/* FIXME this implementation is incorrect - needs to be based on current bus-speed */
unsigned short XUA_Endpoint0_getBcdDevice() {
    unsigned short bcd;
#if (AUDIO_CLASS == 1)
    bcd = devDesc_Audio1.bcdDevice;
#else
    bcd = devDesc_Audio2.bcdDevice;
#endif
    return bcd;
}

void XUA_Endpoint0_setBcdDevice(unsigned short bcd)
{
#if (XUA_AUDIO_CLASS_FS == 1)
    devDesc_Audio1.bcdDevice = bcd;
#endif
#if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
    devDesc_Audio2.bcdDevice = bcd;
#endif
}

#if defined(__static_hid_report_h_exists__)
#define hidReportDescriptorLength (sizeof(hidReportDescriptorPtr))
static unsigned char hidReportDescriptorPtr[] = {
#include "static_hid_report.h"
};
#endif

unsigned char __attribute__((aligned (4))) hid_desc_word_aligned[sizeof(USB_HID_Descriptor_t)];
void XUA_Endpoint0_init(chanend c_ep0_out, chanend c_ep0_in, NULLABLE_RESOURCE(chanend, c_aud_ctl),
    chanend c_mix_ctl, chanend c_clk_ctl, CLIENT_INTERFACE(i_dfu, dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
{
    ep0_out = XUD_InitEp(c_ep0_out);
    ep0_in  = XUD_InitEp(c_ep0_in);

    VendorRequests_Init(VENDOR_REQUESTS_PARAMS);
#if _XUA_ENABLE_BOS_DESC
    Xua_Init_Ep0_Msos_Descriptors();
#endif

    if(strcmp(g_strTable.serialStr, ""))
    {
        // If serialStr is not empty then point the serial number in the device descriptor to the
        // string table. iserialNumber is is set to none (0) by default */
#if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
        devDesc_Audio2.iSerialNumber = offsetof(StringDescTable_t, serialStr)/sizeof(char *);
#endif
#if (XUA_AUDIO_CLASS_FS == 1)
        devDesc_Audio1.iSerialNumber = offsetof(StringDescTable_t, serialStr)/sizeof(char *);
#endif
    }

#if (MIXER)
    /* Set up mixer default state */
    InitLocalMixerState();
#endif

#ifdef VENDOR_AUDIO_REQS
    VendorAudioRequestsInit(c_aud_ctl, c_mix_ctl, c_clk_ctl);
#endif

#if XUA_DFU_EN
    if(strcmp(g_strTable.serialStr, "")) // If serialStr is not empty
    {
        DFUdevDesc.iSerialNumber = offsetof(StringDescTable_t, serialStr)/sizeof(char *); /* Same as the run-time mode device descriptor */
    }
    /* Check if device has started in DFU mode */
    if (DFUReportResetState())
    {
        assert(((unsigned)c_aud_ctl != 0) && msg("DFU not supported when c_aud_ctl is null"));

        /* Stop audio */
        outct(c_aud_ctl, XUA_AUDCTL_SET_SAMPLE_FREQ);
        outuint(c_aud_ctl, AUDIO_STOP_FOR_DFU);
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

#if(XUA_AUDIO_CLASS_FS == 1)
    cfgDesc_Audio1[USB_HID_DESCRIPTOR_OFFSET + HID_DESCRIPTOR_LENGTH_FIELD_OFFSET    ] = hidReportDescriptorLengthLo;
    cfgDesc_Audio1[USB_HID_DESCRIPTOR_OFFSET + HID_DESCRIPTOR_LENGTH_FIELD_OFFSET + 1] = hidReportDescriptorLengthHi;
#endif
    hidDescriptor[HID_DESCRIPTOR_LENGTH_FIELD_OFFSET    ] = hidReportDescriptorLengthLo;
    hidDescriptor[HID_DESCRIPTOR_LENGTH_FIELD_OFFSET + 1] = hidReportDescriptorLengthHi;

    memcpy(hid_desc_word_aligned, hidDescriptor, hidDescriptor[0]); // Copy to a word-aligned address that can be safely sent to XUD_DoGetRequest()
#endif // 0 < HID_CONTROLS
}

void XUA_Endpoint0_loop(XUD_Result_t result, USB_SetupPacket_t sp, chanend c_ep0_out, chanend c_ep0_in, NULLABLE_RESOURCE(chanend, c_aud_ctl),
    chanend c_mix_ctl, chanend c_clk_ctl, CLIENT_INTERFACE(i_dfu, dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
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
#if (NUM_USB_CHAN_OUT > 0)
                        case INTERFACE_NUMBER_AUDIO_OUTPUT:
                            {
                                (void) sp.wIndex; /* Compiler bug - this dummy statement avoids an error when int declared after case */
                                int newStreamAlt_Out = sp.wValue;
                                /* Check the alt is in range */
                                if(newStreamAlt_Out <= OUTPUT_FORMAT_COUNT)
                                {
                                    /* Alt 0 is stream stop */
                                    /* Only send change if we need to */
                                    if(g_curStreamAlt_Out != newStreamAlt_Out)
                                    {
                                        assert((c_aud_ctl != null) && msg("Format change not supported when c_aud_ctl is null"));
                                        g_curStreamAlt_Out = newStreamAlt_Out;

                                        /* Send format of data onto buffering */
                                        if(g_curStreamAlt_Out > 0)
                                        {
                                            outct(c_aud_ctl, XUA_AUDCTL_SET_STREAM_OUTPUT_START);
                                            outuint(c_aud_ctl, g_dataFormat_Out[newStreamAlt_Out-1]);        /* Data format (PCM/DSD) */

                                            if(g_curUsbSpeed == XUD_SPEED_HS)
                                            {
                                                outuint(c_aud_ctl, g_chanCount_Out_HS[newStreamAlt_Out-1]);  /* Channel count */
                                                outuint(c_aud_ctl, g_subSlot_Out_HS[newStreamAlt_Out-1]);    /* Subslot */
                                                outuint(c_aud_ctl, g_sampRes_Out_HS[newStreamAlt_Out-1]);    /* Resolution */
                                            }
                                            else
                                            {
                                                outuint(c_aud_ctl, NUM_USB_CHAN_OUT_FS);                     /* Channel count */
                                                outuint(c_aud_ctl, g_subSlot_Out_FS[newStreamAlt_Out-1]);    /* Subslot */
                                                outuint(c_aud_ctl, g_sampRes_Out_FS[newStreamAlt_Out-1]);    /* Resolution */
                                            }
                                        }
                                        else
                                        {
                                            outct(c_aud_ctl, XUA_AUDCTL_SET_STREAM_OUTPUT_STOP);
                                        }
                                        /* Handshake */
                                        chkct(c_aud_ctl, XS1_CT_END);

                                        UserAudioStreamState(g_curStreamAlt_In > 0, g_curStreamAlt_Out > 0);
                                    } /* (g_curStreamAlt_Out != newStreamAlt_Out) */
                            } /* (newStreamAlt_Out <= OUTPUT_FORMAT_COUNT) */
                            break;
                        }
#endif

#if (NUM_USB_CHAN_IN > 0)
                        case INTERFACE_NUMBER_AUDIO_INPUT:
                            {
                                /* Check the alt is in range */
                                int newStreamAlt_In = sp.wValue;
                                if(newStreamAlt_In <= INPUT_FORMAT_COUNT)
                                {
                                    /* Alt 0 is stream stop */
                                    /* Only send change if we need to */
                                    if(g_curStreamAlt_In != newStreamAlt_In)
                                    {
                                        assert((c_aud_ctl != null) && msg("Format change not supported when c_aud_ctl is null"));
                                        g_curStreamAlt_In = newStreamAlt_In;

                                        /* Send format of data onto buffering */
                                        if(g_curStreamAlt_In > 0)
                                        {
                                            outct(c_aud_ctl, XUA_AUDCTL_SET_STREAM_INPUT_START);
                                            outuint(c_aud_ctl, g_dataFormat_In[newStreamAlt_In-1]);        /* Data format (PCM/DSD) */

                                            if(g_curUsbSpeed == XUD_SPEED_HS)
                                            {
                                                outuint(c_aud_ctl, g_chanCount_In_HS[newStreamAlt_In-1]);  /* Channel count */
                                                outuint(c_aud_ctl, g_subSlot_In_HS[newStreamAlt_In-1]);    /* Subslot */
                                                outuint(c_aud_ctl, g_sampRes_In_HS[newStreamAlt_In-1]);    /* Resolution */
                                            }
                                            else
                                            {
                                                outuint(c_aud_ctl, NUM_USB_CHAN_IN_FS);                     /* Channel count */
                                                outuint(c_aud_ctl, g_subSlot_In_FS[newStreamAlt_In-1]);     /* Subslot */
                                                outuint(c_aud_ctl, g_sampRes_In_FS[newStreamAlt_In-1]);     /* Resolution */
                                            }
                                        }
                                        else
                                        {
                                            outct(c_aud_ctl, XUA_AUDCTL_SET_STREAM_INPUT_STOP);
                                        }
                                        /* Wait for handshake */
                                        chkct(c_aud_ctl, XS1_CT_END);

                                        UserAudioStreamState(g_curStreamAlt_In > 0, g_curStreamAlt_Out > 0);
                                    } /* (g_curStreamAlt_In != newStreamAlt_Out) */
                                } /* (newStreamAlt_In <= INPUT_FORMAT_COUNT) */
                            break;
                        }
#endif
                        default:
                            /* Unhandled interface */
                            break;
                    } /* switch(sp.wIndex) */
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
                                        result = XUD_DoGetRequest(ep0_out, ep0_in, hid_desc_word_aligned,
                                            hid_desc_word_aligned[0], sp.wLength);
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
                switch(sp.bRequest)
                {
                    /* Standard request: SetConfiguration */
                    /* Overriding implementation in USB_StandardRequests */
                    case USB_SET_CONFIGURATION:

                        /* Check if moving into a configured state */
                        if((g_currentConfig == 0) && (sp.wValue == 1))
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
#if (NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0)
                    unsigned epNum = sp.wIndex & 0xff;
#endif

// Ensure we only check for AUDIO EPs if enabled
#if (NUM_USB_CHAN_IN != 0 && NUM_USB_CHAN_OUT == 0)
                    if (epNum == ENDPOINT_ADDRESS_IN_AUDIO)
#elif (NUM_USB_CHAN_IN == 0 && NUM_USB_CHAN_OUT != 0)
                    if (epNum == ENDPOINT_ADDRESS_OUT_AUDIO)
#elif (NUM_USB_CHAN_IN != 0 && NUM_USB_CHAN_OUT != 0)
                    if ((epNum == ENDPOINT_ADDRESS_IN_AUDIO) || (epNum == ENDPOINT_ADDRESS_OUT_AUDIO))
#endif
                    {
#if (XUA_AUDIO_CLASS_FS == 1)
                        /* Since we only support Audio Class 1.0 in FS don't need to check bus speed */
                        result = AudioEndpointRequests_1(ep0_out, ep0_in, &sp, c_aud_ctl, c_mix_ctl, c_clk_ctl);
#endif
                    }

                }
                break;

            case USB_BMREQ_H2D_CLASS_INT:
            case USB_BMREQ_D2H_CLASS_INT:
                {
#if XUA_DFU_EN
                    result = dfu_usb_class_int_requests(ep0_out, ep0_in, &sp, dfuInterface, c_aud_ctl, DFU_mode_active);
#endif
#if XUA_HID_ENABLED
                    if (interfaceNum == INTERFACE_NUMBER_HID)
                    {
                        result = HidInterfaceClassRequests(ep0_out, ep0_in, &sp);
                    }
#endif
                    unsigned interfaceNum = sp.wIndex & 0xff;
                    /* Check for:   - Audio CONTROL interface request - always 0, note we check for DFU first
                     *              - Audio STREAMING interface request  (In or Out)
                     *              - Audio endpoint request (Audio 1.0 Sampling freq requests are sent to the endpoint)
                     */
                    if(((interfaceNum == 0) || (interfaceNum == 1) || (interfaceNum == 2))
#if XUA_DFU_EN
                            && !DFU_mode_active
#endif
                        )
                    {
#if (XUA_AUDIO_CLASS_HS == 2)
                        if(g_curUsbSpeed == XUD_SPEED_HS)
                        {
                            result = AudioClassRequests_2(ep0_out, ep0_in, &sp, c_aud_ctl, c_mix_ctl, c_clk_ctl);
                        }
                        else
#endif
                        {
#if (XUA_AUDIO_CLASS_FS == 1)
                            result = AudioClassRequests_1(ep0_out, ep0_in, &sp, c_aud_ctl, c_mix_ctl, c_clk_ctl);
#else
                            result = AudioClassRequests_2(ep0_out, ep0_in, &sp, c_aud_ctl, c_mix_ctl, c_clk_ctl);
#endif
                        }

#ifdef VENDOR_AUDIO_REQS
                        /* If result is ERR at this point, then request to audio interface not handled - handle vendor audio reqs */
                        if(result == XUD_RES_ERR)
                        {
                            result = VendorAudioRequests(ep0_out, ep0_in, sp.bRequest,
                                sp.wValue >> 8, sp.wValue & 0xff,
                                sp.wIndex >> 8, sp.bmRequestType.Direction,
                                c_aud_ctl, c_mix_ctl, c_clk_ctl);
                        }
#endif
                    }

                // Additional endpoint 0 handling code if declared
#ifdef __xua_user_ep0_handler_h_exists__
                #include "xua_user_ep0_handler.h"
#endif

                }
                break;

            default:
                break;
        }

    } /* if(result == XUD_RES_OKAY) */

    {
#if _XUA_ENABLE_BOS_DESC
        if(result == XUD_RES_ERR)
        {
            unsigned bmRequestType = (sp.bmRequestType.Direction<<7) | (sp.bmRequestType.Type<<5) | (sp.bmRequestType.Recipient);
            if(bmRequestType == USB_BMREQ_D2H_VENDOR_DEV)
            {
                if((sp.bRequest == XUD_REQUEST_GET_MSOS_DESCRIPTOR) &&
                    sp.wIndex == MS_OS_20_DESCRIPTOR_INDEX)
                {
                    int num_interfaces;
                    if(DFU_mode_active) {
                        num_interfaces = DFUcfgDesc.Config.bNumInterfaces;
                    }
                    else
                    {
                        /* TODO this needs to account for USB bus-speed and potentially falling back to UAC1 in FS mode */
                        #if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
                        num_interfaces = cfgDesc_Audio2.Config.bNumInterfaces;
                        #else
                        num_interfaces = NUM_INTERFACES_A1;
                        #endif
                    }

                    if (num_interfaces == 1) {
                        // Use simple MSOS descriptor
                        result = XUD_GetMsosDescriptor(ep0_out, ep0_in, &sp);
                    }
                    else {
                        // Use composite MSOS descriptor
                        result = Xua_GetMsosDescriptor(ep0_out, ep0_in, &sp);
                    }
                }
            }

        }
#endif

#if XUA_DFU_EN
        if(result == XUD_RES_ERR)
        {
            // Handle XMOS_DFU_REVERTFACTORY request in both DFU mode and application mode
            result = dfu_usb_vendor_requests(ep0_out, ep0_in, &sp, dfuInterface, DFU_mode_active);
        }
#endif
        if(result == XUD_RES_ERR)
        {
            /* Run vendor defined parsing/processing */
            /* Note, an interface might seem ideal here but this *must* be executed on the same
             * core sure to shared memory dependency */
            result = VendorRequests(ep0_out, ep0_in, &sp VENDOR_REQUESTS_PARAMS_);
        }
#if _XUA_ENABLE_BOS_DESC
        // Check for BOS descriptor request
        if(result == XUD_RES_ERR)
        {
            unsigned bmRequestType = (sp.bmRequestType.Direction<<7) | (sp.bmRequestType.Type<<5) | (sp.bmRequestType.Recipient);
            switch(bmRequestType)
            {
                case USB_BMREQ_D2H_STANDARD_DEV:
                    switch(sp.bRequest)
                    {
                        case USB_GET_DESCRIPTOR:
                            switch(sp.wValue & 0xff00)
                            {
                                case (USB_DESCTYPE_BOS << 8):
                                {
                                    int num_interfaces;
                                    if(DFU_mode_active) {
                                        num_interfaces = DFUcfgDesc.Config.bNumInterfaces;
                                    }
                                    else {
                                        /* TODO this needs to account for USB bus-speed and potentially falling back to UAC1 in FS mode */
                                        #if (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
                                        num_interfaces = cfgDesc_Audio2.Config.bNumInterfaces;
                                        #else
                                        num_interfaces = NUM_INTERFACES_A1;
                                        #endif
                                    }

                                    if(num_interfaces == 1) {
                                        // Use simple MSOS descriptor
                                        result = XUD_GetBosDescriptor(ep0_out, ep0_in, &sp);
                                    }
                                    else {
                                        // Use composite MSOS descriptor
                                        result = Xua_GetBosDescriptor(ep0_out, ep0_in, &sp);
                                    }
                                }
                                break;
                            }
                        break;
                    }
                break;
            }
        }
#endif
    }

    if(result == XUD_RES_ERR)
    {
#if XUA_DFU_EN
        if (!DFU_mode_active)
        {
#endif
#if (XUA_AUDIO_CLASS_HS == 2) && (XUA_AUDIO_CLASS_FS == 1)
            /* Return Audio 2.0 Descriptors with Audio 1.0 as fallback */
            result = USB_StandardRequests(ep0_out, ep0_in,
                (unsigned char*)&devDesc_Audio2, sizeof(devDesc_Audio2),
                (unsigned char*)&cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                (unsigned char*)&devDesc_Audio1, sizeof(devDesc_Audio1),
                cfgDesc_Audio1, sizeof(cfgDesc_Audio1),
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *),
                &sp, g_curUsbSpeed);
#elif (XUA_AUDIO_CLASS_HS == 2) || (XUA_AUDIO_CLASS_FS == 2)
            /* Return Audio 2.0 Descriptors for high-speed and full-speed */
            /* Unfortunately we need to munge the descriptors a bit between full and high-speed */
            if(g_curUsbSpeed == XUD_SPEED_HS)
            {
                /* Modify Audio Class 2.0 Config descriptor for High-speed operation */
#if (NUM_USB_CHAN_OUT > 0)
                cfgDesc_Audio2.Audio_CS_Control_Int.Audio_Out_InputTerminal.bNrChannels = NUM_USB_CHAN_OUT;
#if (NUM_USB_CHAN_OUT > 0)
                cfgDesc_Audio2.Audio_Out_Format.bSubslotSize = HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format.bBitResolution = HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint.wMaxPacketSize = HS_STREAM_FORMAT_OUTPUT_1_EP_DESC_WMAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface.bNrChannels = HS_STREAM_FORMAT_OUTPUT_1_CHAN_COUNT;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
                /* Async feedback endpoint descriptor change between FS and HS */
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint.wMaxPacketSize = FEEDBACK_MAX_PACKET_SIZE_HS;
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint.bInterval = FEEDBACK_INTERVAL_HS;
#endif
#endif // (NUM_USB_CHAN_OUT > 0)
#if (OUTPUT_FORMAT_COUNT > 1)
                cfgDesc_Audio2.Audio_Out_Format_2.bSubslotSize = HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_2.bBitResolution = HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_2.wMaxPacketSize = HS_STREAM_FORMAT_OUTPUT_2_EP_DESC_WMAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_2.bNrChannels = HS_STREAM_FORMAT_OUTPUT_2_CHAN_COUNT;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
                /* Async feedback endpoint descriptor change between FS and HS */
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_2.wMaxPacketSize = FEEDBACK_MAX_PACKET_SIZE_HS;
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_2.bInterval = FEEDBACK_INTERVAL_HS;
#endif
#endif // (OUTPUT_FORMAT_COUNT > 1)
#if (OUTPUT_FORMAT_COUNT > 2)
                cfgDesc_Audio2.Audio_Out_Format_3.bSubslotSize = HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_3.bBitResolution = HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_3.wMaxPacketSize = HS_STREAM_FORMAT_OUTPUT_3_EP_DESC_WMAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_3.bNrChannels = HS_STREAM_FORMAT_OUTPUT_3_CHAN_COUNT;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
                /* Async feedback endpoint descriptor change between FS and HS */
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_3.wMaxPacketSize = FEEDBACK_MAX_PACKET_SIZE_HS;
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_3.bInterval = FEEDBACK_INTERVAL_HS;
#endif
#endif // (OUTPUT_FORMAT_COUNT > 2)
#endif // (NUM_USB_CHAN_OUT > 0)

#if (NUM_USB_CHAN_IN > 0)
                cfgDesc_Audio2.Audio_CS_Control_Int.Audio_In_InputTerminal.bNrChannels = NUM_USB_CHAN_IN;
                cfgDesc_Audio2.Audio_In_Format.bSubslotSize = HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_In_Format.bBitResolution = HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_In_Endpoint.wMaxPacketSize = HS_STREAM_FORMAT_INPUT_1_EP_DESC_WMAXPACKETSIZE;
                cfgDesc_Audio2.Audio_In_ClassStreamInterface.bNrChannels = NUM_USB_CHAN_IN;
#endif
#if MIDI
                /* MIDI endpoint max packet size must be 512 in HS */
                cfgDesc_Audio2.MIDI_Descriptors.MIDI_Standard_Bulk_OUT_Endpoint.wMaxPacketSize = 512;
                cfgDesc_Audio2.MIDI_Descriptors.MIDI_Standard_Bulk_IN_Endpoint.wMaxPacketSize = 512;
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
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
                /* Async feedback endpoint descriptor change between FS and HS */
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint.wMaxPacketSize = FEEDBACK_MAX_PACKET_SIZE_FS;
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint.bInterval = FEEDBACK_INTERVAL_FS;
#endif
#endif // (NUM_USB_CHAN_OUT > 0)
#if (OUTPUT_FORMAT_COUNT > 1)
                cfgDesc_Audio2.Audio_Out_Format_2.bSubslotSize = FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_2.bBitResolution = FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_2.wMaxPacketSize = FS_STREAM_FORMAT_OUTPUT_2_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_2.bNrChannels = NUM_USB_CHAN_OUT_FS;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
                /* Async feedback endpoint descriptor change between FS and HS */
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_2.wMaxPacketSize = FEEDBACK_MAX_PACKET_SIZE_FS;
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_2.bInterval = FEEDBACK_INTERVAL_FS;
#endif // (OUTPUT_FORMAT_COUNT > 1)
#endif
#if (OUTPUT_FORMAT_COUNT > 2)
                cfgDesc_Audio2.Audio_Out_Format_3.bSubslotSize = FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_Out_Format_3.bBitResolution = FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_Out_Endpoint_3.wMaxPacketSize = FS_STREAM_FORMAT_OUTPUT_3_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_Out_ClassStreamInterface_3.bNrChannels = NUM_USB_CHAN_OUT_FS;
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
                /* Async feedback endpoint descriptor change between FS and HS */
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_3.wMaxPacketSize = FEEDBACK_MAX_PACKET_SIZE_FS;
                cfgDesc_Audio2.Audio_Out_Fb_Endpoint_3.bInterval = FEEDBACK_INTERVAL_FS;
#endif
#endif // (OUTPUT_FORMAT_COUNT > 2)
#endif // (NUM_USB_CHAN_OUT > 0)

#if (NUM_USB_CHAN_IN > 0)
                cfgDesc_Audio2.Audio_CS_Control_Int.Audio_In_InputTerminal.bNrChannels = NUM_USB_CHAN_IN_FS;
                cfgDesc_Audio2.Audio_In_Format.bSubslotSize = FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;
                cfgDesc_Audio2.Audio_In_Format.bBitResolution = FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS;
                cfgDesc_Audio2.Audio_In_Endpoint.wMaxPacketSize = FS_STREAM_FORMAT_INPUT_1_MAXPACKETSIZE;
                cfgDesc_Audio2.Audio_In_ClassStreamInterface.bNrChannels = NUM_USB_CHAN_IN_FS;
#endif
#if MIDI
                /* MIDI endpoint max packet size must be 64 bytes in FS */
                cfgDesc_Audio2.MIDI_Descriptors.MIDI_Standard_Bulk_OUT_Endpoint.wMaxPacketSize = 64;
                cfgDesc_Audio2.MIDI_Descriptors.MIDI_Standard_Bulk_IN_Endpoint.wMaxPacketSize = 64;
#endif
            }

            result = USB_StandardRequests(ep0_out, ep0_in,
                (unsigned char*)&devDesc_Audio2, sizeof(devDesc_Audio2),
                (unsigned char*)&cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                null, 0,
                null, 0,
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, g_curUsbSpeed);
#elif (XUA_AUDIO_CLASS_FS == 1)
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
#if XUA_DFU_EN
        }
        else
        {
            /* Running in DFU mode - always return same descs for DFU whether HS or FS */
            result = USB_StandardRequests(ep0_out, ep0_in,
                (unsigned char*)&DFUdevDesc, sizeof(DFUdevDesc),
                (unsigned char*)&DFUcfgDesc, sizeof(DFUcfgDesc),
                null, 0, /* Used same descriptors for full and high-speed */
                null, 0,
                (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, g_curUsbSpeed);
        }
#endif
    }

    if(result == XUD_RES_UPDATE)
    {
        XUD_BusState_t busState = XUD_GetBusState(ep0_out, &ep0_in);

        if(busState == XUD_BUS_RESET)
        {
            g_curUsbSpeed = XUD_ResetEndpoint(ep0_out, &ep0_in);

            if(g_curStreamAlt_Out || g_curStreamAlt_In)
            {
                UserAudioStreamState(0, 0);
                g_curStreamAlt_Out = 0;
                g_curStreamAlt_In = 0;
            }

            if(g_currentConfig)
            {
                UserHostActive(0);
                g_currentConfig = 0;
            }

#if XUA_DFU_EN
            if (DFUReportResetState())
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
        else
        {
            if (busState == XUD_BUS_SUSPEND)
            {
                /* Ensure all streams have stopped (in case this came in as unplug during streaming) */
                /* Note the logic in decouple contains state about current stream state and so it
                   will not pass this on to audio if already stopped */
                if(NUM_USB_CHAN_IN > 0){
                    outct(c_aud_ctl, XUA_AUDCTL_SET_STREAM_INPUT_STOP);
                    chkct(c_aud_ctl, XS1_CT_END);
                }
                if(NUM_USB_CHAN_OUT > 0){
                    outct(c_aud_ctl, XUA_AUDCTL_SET_STREAM_OUTPUT_STOP);
                    chkct(c_aud_ctl, XS1_CT_END);
                }

                /* Device moving from CONFIGURED to SUSPENDED state */
                if(g_currentConfig)
                {
                    UserHostActive(0);
                }

                // Perform user-defined suspend behaviour
                XUA_UserSuspendPowerDown();
            }
            else // XUD_BUS_RESUME
            {
                // Peform user-defined resume behaviour
                XUA_UserSuspendPowerUp();

                /* If audio interfaces still active call user call back */
                if(g_curStreamAlt_Out || g_curStreamAlt_In)
                    UserAudioStreamState(g_curStreamAlt_Out > 0, g_curStreamAlt_In > 0);

                /* Device moving from SUSPENDED to CONFIGURED state - call user call back */
                if(g_currentConfig == 1)
                    UserHostActive(1);
            }
            /* Acknowledge back to XUD letting it know we've handled suspend/resume */
            XUD_AckBusState(ep0_out, &ep0_in); // This should set ep_info[i]
        }
    }
}

/* Endpoint 0 function.  Handles all requests to the device */
void XUA_Endpoint0(chanend c_ep0_out, chanend c_ep0_in, NULLABLE_RESOURCE(chanend, c_aud_ctl),
    chanend c_mix_ctl, chanend c_clk_ctl, CLIENT_INTERFACE(i_dfu, dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_)
{
    USB_SetupPacket_t sp;
    XUA_Endpoint0_init(c_ep0_out, c_ep0_in, c_aud_ctl, c_mix_ctl, c_clk_ctl, dfuInterface VENDOR_REQUESTS_PARAMS_);

    while(1)
    {
        /* Returns XUD_RES_OKAY for success, XUD_RES_UPDATE for bus status update */
        XUD_Result_t result = USB_GetSetupPacket(ep0_out, ep0_in, &sp);
        XUA_Endpoint0_loop(result, sp, c_ep0_out, c_ep0_in, c_aud_ctl, c_mix_ctl, c_clk_ctl, dfuInterface VENDOR_REQUESTS_PARAMS_);
    }
}

void DFUNotifyEntry(NULLABLE_RESOURCE(chanend, c_aud_ctl))
{
    /* Send STOP_AUDIO_FOR_DFU command. This will either pass through
        * buffering system (i.e. ep_buffer/decouple) if the device has USB audio
        * channels. Otherwise this directly interacts with AudioHub
        * This command needs to be sent such that AudioHub runs the DFUHandler()
        * task - in the case where AudioHub is running on tile[0] i.e the
        * flash tile and the USB code (i.e this task) are running on separate
        * tiles. It also means that Flash pins can be shared with "audio" pins.
        */
    assert((c_aud_ctl != null) && msg("DFU not supported when c_aud_ctl is null"));
    // Stop audio
    outct(c_aud_ctl, XUA_AUDCTL_SET_SAMPLE_FREQ);
    outuint(c_aud_ctl, AUDIO_STOP_FOR_DFU);
    // Handshake
    chkct(c_aud_ctl, XS1_CT_END);
}

#endif /* XUA_USB_EN*/
