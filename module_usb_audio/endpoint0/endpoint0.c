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

/* If DFU_PID not defined, standard PID used.. this is probably what we want.. */
#ifndef DFU_PID
#warning DFU_PID not defined, Using PID_AUDIO_2. This is probably fine!
#endif

#ifdef DFU
#include "dfu.h"
#define DFU_IF_NUM INPUT_INTERFACES + OUTPUT_INTERFACES + MIDI_INTERFACES + 1
extern void device_reboot(chanend);
#endif

unsigned int DFU_mode_active = 0;         // 0 - App active, 1 - DFU active

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
unsigned char channelMapAud[NUM_USB_CHAN_OUT];
unsigned char channelMapUsb[NUM_USB_CHAN_IN];
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
XUD_BusSpeed_t g_curUsbSpeed = 0;


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

/* Endpoint 0 function.  Handles all requests to the device */
void Endpoint0(chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend c_mix_ctl, chanend c_clk_ctl, chanend c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, dfuInterface))
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
    for(int j = 0; j < MAX_MIX_COUNT; j++)
    for(int i = 0; i < MIX_INPUTS; i++)
    {
        mixSel[j][i] = i;
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
        XUD_Result_t result = USB_GetSetupPacket(ep0_out, ep0_in, &sp);

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
                                break;
#endif

#if (NUM_USB_CHAN_IN > 0)
                            case INTERFACE_NUMBER_AUDIO_INPUT:
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
                        if ((sp.wIndex == INTERFACE_NUMBER_AUDIO_OUTPUT) || (sp.wIndex == INTERFACE_NUMBER_AUDIO_INPUT))
                        {
                            /* Check for stream start stop on output and input audio interfaces */
                            if(sp.wValue && !g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT] && !g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT])
                            {
                                /* If start and input AND output not currently running */
                                UserAudioStreamStart();
                            }
                            else if(((sp.wIndex == 1) && (!sp.wValue)) && g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT] && (!g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT]))
                            {
                                /* if output stop and output running and input not running */
                                UserAudioStreamStop();
                            }
                            else if(((sp.wIndex == 2) && (!sp.wValue)) && g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT] && (!g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT]))
                            {
                                /* if input stop and input running and output not running */
                                UserAudioStreamStop();
                            }
                        }
#elif (NUM_USB_CHAN_OUT > 0)
                        if(sp.wIndex == INTERFACE_NUMBER_AUDIO_OUTPUT)
                        {
                            if(sp.wValue && (!g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT]))
                            {
                                /* if start and not currently running */
                                UserAudioStreamStart();
                            }
                            else if (!sp.wValue && g_interfaceAlt[INTERFACE_NUMBER_AUDIO_OUTPUT])
                            {
                                /* if stop and currently running */
                                UserAudioStreamStop();
                            }
                        }
#elif (NUM_USB_CHAN_IN > 0)
                        if(sp.wIndex == INTERFACE_NUMBER_AUDIO_INPUT)
                        {
                            if(sp.wValue && (!g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT]))
                            {
                                /* if start and not currently running */
                                UserAudioStreamStart();
                            }
                            else if (!sp.wValue && g_interfaceAlt[INTERFACE_NUMBER_AUDIO_INPUT])
                            {
                                /* if stop and currently running */
                                UserAudioStreamStop();
                            }
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
                                    case HID_HID:
                                        /* Return HID Descriptor */
                                         result = XUD_DoGetRequest(ep0_out, ep0_in, hidDescriptor,
                                            sizeof(hidDescriptor), sp.wLength);
                                        break;
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

                        if ((epNum == ENDPOINT_ADDRESS_OUT_AUDIO) || (epNum == ENDPOINT_ADDRESS_IN_AUDIO))
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
                            int reset = 0;

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

                            /* This will return 1 if reset requested */
                            result = DFUDeviceRequests(ep0_out, &ep0_in, &sp, null, g_interfaceAlt[sp.wIndex], dfuInterface, &reset);

                            if(reset)
                            {
                                DFUDelay(50000000);
                                device_reboot(c_audioControl);
                            }
                        }
#endif
                        /* Check for:   - Audio CONTROL interface request - always 0, note we check for DFU first
                         *              - Audio STREAMING interface request  (In or Out)
                         *              - Audio endpoint request (Audio 1.0 Sampling freq requests are sent to the endpoint)
                         */
                        if(((interfaceNum == 0) || (interfaceNum == 1) || (interfaceNum == 2))
#ifdef DFU
                                && !DFU_mode_active
#endif
                            )
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
