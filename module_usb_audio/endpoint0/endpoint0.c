/**
 * g
 * @file    endpoint0.xc
 * @brief   Implements endpoint zero for an USB Audio 1.0/2.0 device
 * @author  Ross Owen, XMOS Semiconductor
 */

#include <xs1.h>
#include <safestring.h>
#include <stddef.h>

#include "xud.h"                 /* XUD user defines and functions */
#include "usb_std_requests.h"
#include "usbaudio20.h"          /* Defines from USB Audio 2.0 spec */

#include "devicedefines.h"
#include "usb_device.h"          /* Standard descriptor requests */
#include "descriptors.h"       /* This devices descriptors */
#include "commands.h"
#include "audiostream.h"
#include "hostactive.h"
#include "vendorrequests.h"
#include "dfu_types.h"
#include "xc_ptr.h"
#include "audiorequests.h"
#ifdef HID_CONTROLS
#include "hid.h"
#endif
#if DSD_CHANS_DAC > 0
#include "dsd_support.h"
#endif

#ifndef __XC__
/* Support for C */
#define null 0
#define outuint(c, x)   asm ("out res[%0], %1" :: "r" (c), "r" (x))
#define chkct(c, x)     asm ("chkct res[%0], %1" :: "r" (c), "r" (x))
#endif

/* Some warnings.... */

/* Windows does not have a built in DFU driver (windows will prompt), so warn that DFU will not be functional in Audio 1.0 mode */
#if ((AUDIO_CLASS==1) || defined(AUDIO_CLASS_FALLBACK)) && defined(DFU)
#warning DFU will not be enabled in AUDIO 1.0 mode due to Windows requesting driver
#endif

/* MIDI not supported in Audio 1.0 mode */
#if ((AUDIO_CLASS==1) || defined(AUDIO_CLASS_FALLBACK)) && defined(MIDI)
#warning MIDI is currently not supported and will not be enabled in AUDIO 1.0 mode
#endif

/* If PID_DFU not defined, standard PID used.. this is probably what we want.. */
#ifndef PID_DFU
#warning PID_DFU not defined, Using PID_AUDIO_2. This is probably fine!
#endif

#ifdef DFU
#include "dfu.h"
#define DFU_IF_NUM INPUT_INTERFACES + OUTPUT_INTERFACES + MIDI_INTERFACES + 1

unsigned int DFU_mode_active = 0;         // 0 - App active, 1 - DFU active
extern void device_reboot(chanend);
#endif

/* Global volume and mute tables */
int volsOut[NUM_USB_CHAN_OUT + 1];
unsigned int mutesOut[NUM_USB_CHAN_OUT + 1];
//unsigned int multOut[NUM_USB_CHAN_OUT + 1];

int volsIn[NUM_USB_CHAN_IN + 1];
unsigned int mutesIn[NUM_USB_CHAN_IN + 1];
//unsigned int multIn[NUM_USB_CHAN_IN + 1];

#ifdef MIXER
unsigned char mixer1Crossbar[18];
short mixer1Weights[18*8];

unsigned char channelMap[NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT];
#if NUM_USB_CHAN_OUT > 0
unsigned char channelMapAud[NUM_USB_CHAN_OUT];
#endif
#if NUM_USB_CHAN_IN > 0
unsigned char channelMapUsb[NUM_USB_CHAN_IN];
#endif
unsigned char mixSel[MIX_INPUTS];
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
XUD_BusSpeed_t g_curUsbSpeed = 0;

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

const unsigned g_subSlot_In_HS[INPUT_FORMAT_COUNT]      = {HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES};

const unsigned g_subSlot_In_FS[INPUT_FORMAT_COUNT]      = {FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES};

const unsigned g_sampRes_Out_HS[OUTPUT_FORMAT_COUNT]    = {HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS,
                                                            HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS,
                                                            HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS};

const unsigned g_sampRes_Out_FS[OUTPUT_FORMAT_COUNT]    = {FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS,
                                                            FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS,
                                                            FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS};

const unsigned g_sampRes_In_HS[OUTPUT_FORMAT_COUNT]     = {HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS};

const unsigned g_sampRes_In_FS[OUTPUT_FORMAT_COUNT]     = {FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS};

const unsigned g_dataFormat_Out[OUTPUT_FORMAT_COUNT]    = {STREAM_FORMAT_OUTPUT_1_DATAFORMAT,
                                                            STREAM_FORMAT_OUTPUT_2_DATAFORMAT,
                                                            STREAM_FORMAT_OUTPUT_3_DATAFORMAT};

const unsigned g_dataFormat_In[INPUT_FORMAT_COUNT] = {STREAM_FORMAT_INPUT_1_DATAFORMAT};

/* Endpoint 0 function.  Handles all requests to the device */
void Endpoint0(chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    NULLABLE_RESOURCE(chanend, c_mix_ctl),
    NULLABLE_RESOURCE(chanend, c_clk_ctl),
    NULLABLE_RESOURCE(chanend, c_usb_test))
{
    USB_SetupPacket_t sp;
    XUD_ep ep0_out = XUD_InitEp(c_ep0_out);
    XUD_ep ep0_in  = XUD_InitEp(c_ep0_in);

    /* Init tables for volumes (+ 1 for master) */
    for(int i = 0; i < NUM_USB_CHAN_OUT + 1; i++)
    {
        volsOut[i] = 0;
        mutesOut[i] = 0;
    }

    for(int i = 0; i < NUM_USB_CHAN_IN + 1; i++)
    {
        volsIn[i] = 0;
        mutesIn[i] = 0;
    }

#ifdef MIXER
    /* Set up mixer default state */
    for (int i = 0; i < 18*8; i++)
    {
        mixer1Weights[i] = 0x8001; //-inf
    }

    /* Configure default connections */
    mixer1Weights[0] = 0;
    mixer1Weights[9] = 0;
    mixer1Weights[18] = 0;
    mixer1Weights[27] = 0;
    mixer1Weights[36] = 0;
    mixer1Weights[45] = 0;
    mixer1Weights[54] = 0;
    mixer1Weights[63] = 0;

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

    /* Set up channel mapping default */
    for (int i = 0; i < NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN; i++)
    {
        channelMap[i] = i;
    }

#if MAX_MIX_COUNT > 0
    /* Mixer outputs mapping defaults */
    for (int i = 0; i < MAX_MIX_COUNT; i++)
    {
        channelMap[NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + i] = i;
    }
#endif

    /* Init mixer inputs */
    for(int i = 0; i < MIX_INPUTS; i++)
    {
        mixSel[i] = i;
    }
#endif

#ifdef VENDOR_AUDIO_REQS
    VendorAudioRequestsInit(c_audioControl, c_mix_ctl, c_clk_ctl);
#endif

#ifdef DFU
    /* Check if device has started in DFU mode */
    if (DFUReportResetState(null))
    {
        /* Stop audio */
        outuint(c_audioControl, SET_SAMPLE_FREQ);
        outuint(c_audioControl, AUDIO_STOP_FOR_DFU);
        // No Handshake
        DFU_mode_active = 1;
    }
#endif

    while(1)
    {
        /* Returns XUD_RES_OKAY for success, XUD_RES_RST for bus reset */
#if defined(__XC__)
        XUD_Result_t result = USB_GetSetupPacket(ep0_out, ep0_in, sp);
#else
        XUD_Result_t result = USB_GetSetupPacket(ep0_out, ep0_in, &sp);
#endif

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
                        /* Check for audio stream from host start/stop */
                        if(sp.wIndex == 1)  /* Output interface */
                        {
                            /* Check the alt is in range */
                            if(sp.wValue <= OUTPUT_FORMAT_COUNT)
                            {
                                /* Alt 0 is stream stop */
                                /* Only send change if we need to */
                                if((sp.wValue > 0) && (g_curStreamAlt_Out != sp.wValue))
                                {
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
                        }
                        else if(sp.wIndex == 2) /* Input interface */
                        {
                            /* Check the alt is in range */
                            if(sp.wValue <= INPUT_FORMAT_COUNT)
                            {
                                /* Alt 0 is stream stop */
                                /* Only send change if we need to */
                                if((sp.wValue > 0) && (g_curStreamAlt_In != sp.wValue))
                                {
                                    g_curStreamAlt_In = sp.wValue;

                                    /* Send format of data onto buffering */
                                    outuint(c_audioControl, SET_STREAM_FORMAT_IN);
                                    outuint(c_audioControl, g_dataFormat_In[sp.wValue-1]);        /* Data format (PCM/DSD) */

                                    if(g_curUsbSpeed == XUD_SPEED_HS)
                                    {
                                        outuint(c_audioControl, NUM_USB_CHAN_IN);                 /* Channel count */
                                        outuint(c_audioControl, g_subSlot_In_HS[sp.wValue-1]);    /* Subslot */
                                        outuint(c_audioControl, g_sampRes_In_HS[sp.wValue-1]);    /* Resolution */
                                    }
                                    else
                                    {
                                        outuint(c_audioControl, NUM_USB_CHAN_IN_FS);               /* Channel count */
                                        outuint(c_audioControl, g_subSlot_In_FS[sp.wValue-1]);     /* Subslot */
                                        outuint(c_audioControl, g_sampRes_In_FS[sp.wValue-1]);     /* Resolution */
                                    }

                                    /* Handshake */
							        chkct(c_audioControl, XS1_CT_END);
                                }
                            }
                        }
#if defined(OUTPUT) && defined(INPUT)
                        /* Check for stream start stop on output and input audio interfaces */
                        if(sp.wValue && !g_interfaceAlt[1] && !g_interfaceAlt[2])
                        {
                            /* If start and input AND output not currently running */
                            UserAudioStreamStart();
                        }
                        else if(((sp.wIndex == 1) && (!sp.wValue)) && g_interfaceAlt[1] && (!g_interfaceAlt[2]))
                        {
                            /* if output stop and output running and input not running */
                            UserAudioStreamStop();
                        }
                        else if(((sp.wIndex == 2) && (!sp.wValue)) && g_interfaceAlt[2] && (!g_interfaceAlt[1]))
                        {
                            /* if input stop and input running and output not running */
                            UserAudioStreamStop();
                        }
#elif defined(OUTPUT) || defined(INPUT)
                        if(sp.wValue && (!g_interfaceAlt[1]))
                        {
                            /* if start and not currently running */
                            UserAudioStreamStart();
                        }
                        else if (!sp.wValue && g_interfaceAlt[1])
                        {
                            /* if stop and currently running */
                            UserAudioStreamStop();
                        }
#endif
                    } /* if(sp.bRequest == SET_INTERFACE) */

                    break; /* BMREQ_H2D_STANDARD_INT */

                case USB_BMREQ_D2H_STANDARD_INT:

                    switch(sp.bRequest)
                    {

#ifdef HID_CONTROLS
                        case USB_GET_DESCRIPTOR:

                            /* Check what inteface request is for */
                            if(sp.wIndex == INTERFACE_NUMBER_HID)
                            {
                                /* High byte of wValue is descriptor type */
                                unsigned descriptorType = sp.wValue & 0xff00;
                                switch (descriptorType)
                                {
                                    case HID_REPORT:
                                        /* Return HID report descriptor */
                                        result = XUD_DoGetRequest(ep0_out, ep0_in, hidReportDescriptor,
                                            sizeof(hidReportDescriptor), sp.wLength);
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

                        if ((epNum == 0x82) || (epNum == 0x01))
				        {
#if (AUDIO_CLASS == 2) && defined(AUDIO_CLASS_FALLBACK)
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
#ifdef DFU
                        unsigned DFU_IF = INTERFACE_NUMBER_DFU;

                        /* DFU interface number changes based on which mode we are currently running in */
                        if (DFU_mode_active)
                        {
                            DFU_IF = 0;
                        }

                        if (interfaceNum == DFU_IF)
                        {
                            /* If running in application mode stop audio */
                            /* Don't interupt audio for save and restore cmds */
                            if ((DFU_IF == INTERFACE_NUMBER_DFU) && (sp.bRequest != XMOS_DFU_SAVESTATE) &&
                                (sp.bRequest != XMOS_DFU_RESTORESTATE))
                            {
                                // Stop audio
                                outuint(c_audioControl, SET_SAMPLE_FREQ);
                                outuint(c_audioControl, AUDIO_STOP_FOR_DFU);
                                // Handshake
							    chkct(c_audioControl, XS1_CT_END);
                            }
#ifdef __XC__
                            /* This will return 1 if reset requested */
                            if (DFUDeviceRequests(ep0_out, ep0_in, sp, null, g_interfaceAlt[sp.wIndex], 1))
#else
                            /* This will return 1 if reset requested */
                            if (DFUDeviceRequests(ep0_out, &ep0_in, &sp, null, g_interfaceAlt[sp.wIndex], 1))
#endif
                            {
                                DFUDelay(50000000);
                                device_reboot(c_audioControl);
                            }

                            /* TODO we should not make the assumption that all DFU requests are handled */
                            result = 0;
                        }
#endif
                        /* Check for:   - Audio CONTROL interface request - always 0, note we check for DFU first
                         *              - Audio STREAMING interface request  (In or Out)
                         *              - Audio endpoint request (Audio 1.0 Sampling freq requests are sent to the endpoint)
                         */
                        if((interfaceNum == 0) || (interfaceNum == 1) || (interfaceNum == 2))
				        {
#if (AUDIO_CLASS == 2) && defined(AUDIO_CLASS_FALLBACK)
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
#error
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

        if(result == XUD_RES_ERR)
        {
#ifdef DFU
            if (!DFU_mode_active)
            {
#endif
#ifdef AUDIO_CLASS_FALLBACK
                /* Return Audio 2.0 Descriptors with Audio 1.0 as fallback */
                result = USB_StandardRequests(ep0_out, ep0_in,
                    (unsigned char*)&devDesc_Audio2, sizeof(devDesc_Audio2),
                    (unsigned char*)&cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                    (unsigned char*)&devDesc_Audio1, sizeof(devDesc_Audio1),
                    cfgDesc_Audio1, sizeof(cfgDesc_Audio1),
                    (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *),
                    &sp, c_usb_test, g_curUsbSpeed);
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
                    g_strTable, sizeof(g_strTable), sp, c_usb_test, g_curUsbSpeed);
#else
                    (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, c_usb_test, g_curUsbSpeed);
#endif
#elif (AUDIO_CLASS == 1)
                /* Return Audio 1.0 Descriptors in FS, should never be in HS! */
                 result = USB_StandardRequests(ep0_out, ep0_in,
                    null, 0,
                    null, 0,
                    (unsigned char*)&devDesc_Audio1, sizeof(devDesc_Audio1),
                    cfgDesc_Audio1, sizeof(cfgDesc_Audio1),
                    (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, c_usb_test, g_curUsbSpeed);
#else
                /* Return Audio 2.0 Descriptors with Null device as fallback */
                result = USB_StandardRequests(ep0_out, ep0_in,
                    (unsigned char*)&devDesc_Audio2, sizeof(devDesc_Audio2),
                    (unsigned char*)&cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                    devDesc_Null, sizeof(devDesc_Null),
                    cfgDesc_Null, sizeof(cfgDesc_Null),
                    (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, c_usb_test, g_curUsbSpeed);
#endif
#ifdef DFU
            }
            else
            {
                /* Running in DFU mode - always return same descs for DFU whether HS or FS */
                result = USB_StandardRequests(ep0_out, ep0_in,
                    DFUdevDesc, sizeof(DFUdevDesc),
                    DFUcfgDesc, sizeof(DFUcfgDesc),
                    null, 0, /* Used same descriptors for full and high-speed */
                    null, 0,
                    (char**)&g_strTable, sizeof(g_strTable)/sizeof(char *), &sp, c_usb_test, g_curUsbSpeed);
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

#ifdef DFU
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
                    DFUDelay(5000000);
                    device_reboot(c_audioControl);
                }
            }
#endif
        }
    }
}
