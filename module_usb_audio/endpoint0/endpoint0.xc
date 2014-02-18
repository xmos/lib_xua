/**
 * @file    endpoint0.xc
 * @brief   Implements endpoint zero for an USB Audio 1.0/2.0 device
 * @author  Ross Owen, XMOS Semiconductor
 */

#include <xs1.h>
#include <safestring.h>

#include "xud.h"                 /* XUD user defines and functions */
#include "usb.h"                 /* Defines from USB 2.0 Spec */
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
#ifdef HID_CONTROLS
#include "hid.h"
#endif
#if DSD_CHANS_DAC > 0
#include "dsd_support.h"
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

/* Handles Audio Class requests */
int AudioClassRequests_2(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl);
int AudioClassRequests_1(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl);
int AudioEndpointRequests_1(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl);



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
//#define MAX_MIX_COUNT 8

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

/* Global variable for current USB bus speed (i.e. FS/HS) */
unsigned g_curUsbSpeed = 0;


/* Global used for signalling reset to decouple */
#ifdef IAP
extern unsigned g_iap_reset;
#endif

#ifdef NATIVE_DSD
/* We remember if we are in DSD mode to avoid Configuring the DAC too often - thus avoiding pops and clicks */
unsigned g_dsdMode = 0;
#endif

void VendorAudioRequestsInit(chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl);

/* Endpoint 0 function.  Handles all requests to the device */
void Endpoint0( chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend ?c_mix_ctl, chanend ?c_clk_ctl, chanend ?c_usb_test)
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

    /* Build up channel string table - By default all channels are marked as analogue
     * TODO We really want to do this an build time... */
#if defined(SPDIF_RX) && (SPDIF_RX_INDEX != 0)
    safestrcpy(strDescs[SPDIF_RX_INDEX + INPUT_INTERFACE_STRING_INDEX], "S/PDIF 1");
    safestrcpy(strDescs[SPDIF_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 1], "S/PDIF 2");
#endif
#if defined(ADAT_RX) && (ADAT_RX_INDEX != 0)
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX], "ADAT 1");
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 1], "ADAT 2");
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 2], "ADAT 3");
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 3], "ADAT 4");
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 4], "ADAT 5");
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 5], "ADAT 6");
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 6], "ADAT 7");
    safestrcpy(strDescs[ADAT_RX_INDEX + INPUT_INTERFACE_STRING_INDEX + 7], "ADAT 8");
#endif

#if defined(SPDIF) && (SPDIF_TX_INDEX != 0)     /* "Analogue naming gets priority */
    safestrcpy(strDescs[SPDIF_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX], "S/PDIF 1");
    safestrcpy(strDescs[SPDIF_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 1], "S/PDIF 2");
#endif
#if defined(ADAT_TX) && (ADAT_TX_INDEX != 0)
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX], "ADAT 1");
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 1], "ADAT 2");
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 2], "ADAT 3");
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 3], "ADAT 4");
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 4], "ADAT 5");
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 5], "ADAT 6");
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 6], "ADAT 7");
    safestrcpy(strDescs[ADAT_TX_INDEX + OUTPUT_INTERFACE_STRING_INDEX + 7], "ADAT 8");
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
        //chkct(c_audioControl, XS1_CT_END);
        DFU_mode_active = 1;
    }
#endif

    while(1)
    {
        /* Returns 0 for success, -1 for bus reset */
        XUD_Result_t result = USB_GetSetupPacket(ep0_out, ep0_in, sp);

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
                        if(sp.wIndex == 1)  // Ouput interface
                        {
                            switch(sp.wValue)
                            {
                                case 0:
                                    /* Stream stop */
#if defined(NATIVE_DSD) && defined(DEFAULT_TO_PCM)
                                    /* Default to PCM mode */
                                    if(g_dsdMode)
                                    {
                                        outuint(c_audioControl, SET_DSD_MODE);
                                        outuint(c_audioControl, DSD_MODE_OFF);

                                        /* Handshake */
							            chkct(c_audioControl, XS1_CT_END);
                                        g_dsdMode = 0;
                                    }
#endif
                                    break;
                                case 1:
                                    /* Stream active + 0 chans */
                                    /* NOTE there could be a difference between HS/UAC1 and FS/UAC1 channel count */
                                    /* Also note, currently we assume with won't be doing ADAT in FS/UAC1...*/
                                    if(g_curUsbSpeed == XUD_SPEED_HS)
                                    {
                                        outuint(c_audioControl, SET_CHAN_COUNT_OUT);
                                        outuint(c_audioControl, NUM_USB_CHAN_OUT);
                                    }
                                    else
                                    {
                                        outuint(c_audioControl, SET_CHAN_COUNT_OUT);
                                        outuint(c_audioControl, NUM_USB_CHAN_OUT_FS);
                                    }
#ifdef NATIVE_DSD
                                    if(g_dsdMode)
                                    {
                                        outuint(c_audioControl, SET_DSD_MODE);
                                        outuint(c_audioControl, DSD_MODE_OFF);

                                        // Handshake
							            chkct(c_audioControl, XS1_CT_END);
                                        g_dsdMode = 0;
                                    }
#endif /* NATIVE_DSD */
                                    break;
#ifdef NATIVE_DSD
                                case 2:

                                    if(!g_dsdMode)
                                    {
                                        outuint(c_audioControl, SET_DSD_MODE);
                                        outuint(c_audioControl, DSD_MODE_NATIVE);
							            chkct(c_audioControl, XS1_CT_END);
                                        g_dsdMode = 1;
                                    }
                                    break;
#endif /* NATIVE_DSD */
                            }
                        }
                        else if(sp.wIndex == 2) // Input interface
                        {
                            switch(sp.wValue)
                            {
                                case 0:
                                    break;
                                case 1:
                                    /* Stream active + 0 chans */
                                    /* NOTE there could be a difference between HS/UAC1 and FS/UAC1 channel count */
                                    /* Also note, currently we assume with won't be doing ADAT in FS/UAC1...*/
                                    if(g_curUsbSpeed == XUD_SPEED_HS)
                                    {
                                        outuint(c_audioControl, SET_CHAN_COUNT_IN);
                                        outuint(c_audioControl, NUM_USB_CHAN_IN);
                                    }
                                    else
                                    {
                                        outuint(c_audioControl, SET_CHAN_COUNT_IN);
                                        outuint(c_audioControl, NUM_USB_CHAN_IN_FS);
                                    }
#ifdef ADAT_RX
                                    outuint(c_clk_ctl, SET_SMUX);
                                    outuint(c_clk_ctl, 0);
                                    outct(c_clk_ctl, XS1_CT_END);
#endif
                                    break;

#ifdef ADAT_RX
                                case 2:
                                    /* Stream active + 8 chans */
                                    outuint(c_audioControl, SET_CHAN_COUNT_IN);
                                    outuint(c_audioControl, NUM_USB_CHAN_IN-4);
                                    outuint(c_clk_ctl, SET_SMUX);
                                    outuint(c_clk_ctl, 1);
                                    outct(c_clk_ctl, XS1_CT_END);
                                    break;

                                case 3:
                                    outuint(c_audioControl, SET_CHAN_COUNT_IN);
                                    outuint(c_audioControl, NUM_USB_CHAN_IN-6);
                                    outuint(c_clk_ctl, SET_SMUX);
                                    outuint(c_clk_ctl, 1);
                                    outct(c_clk_ctl, XS1_CT_END);
                                    break;
#endif
                            }
                        }
#if 1
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
#endif
                        /* Record interface change */
                        if(sp.wIndex < NUM_INTERFACES)
                            g_interfaceAlt[sp.wIndex] = sp.wValue;

                        /* No data stage for this request, just do data stage */
                        result = XUD_DoSetRequestStatus(ep0_in);

                    } /* if(sp.bRequest == SET_INTERFACE) */

                    break; /* BMREQ_H2D_STANDARD_INT */

                case USB_BMREQ_D2H_STANDARD_INT:

                    switch(sp.bRequest)
                    {

#ifdef HID_CONTROLS
                        case USB_GET_DESCRIPTOR:

                            /* Check what inteface request is for */
                            if(sp.wIndex == INTERFACE_NUM_HID)
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

                            //g_currentConfig = sp.wValue;
                            //if(g_current_config == 1)
                            {
                                /* Consider host active with valid driver at this point */
                                UserHostActive(1);
                            }
#ifdef IAP
                            {
                               int iap_reset = 1;
                               SET_SHARED_GLOBAL(g_iap_reset, iap_reset);
                            }
#endif
                            ///* No data stage for this request, just do status stage */
                            //result = XUD_DoSetRequestStatus(ep0_in);

                           // /* We want to run USB_StandardsRequests() implementation also */
                            //if(result == 0)
                              //  result = 1;
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
                                result = AudioEndpointRequests_1(ep0_out, ep0_in, sp, c_audioControl, c_mix_ctl, c_clk_ctl);
                            }
#elif (AUDIO_CLASS==1)
                            result = AudioEndpointRequests_1(ep0_out, ep0_in, sp, c_audioControl, c_mix_ctl, c_clk_ctl);
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
                        unsigned DFU_IF = DFU_IF_NUM;

                        /* DFU interface number changes based on which mode we are currently running in */
                        if (DFU_mode_active)
                        {
                            DFU_IF = 0;
                        }

                        if (interfaceNum == DFU_IF)
                        {
                            /* If running in application mode stop audio */
                            /* Don't interupt audio for save and restore cmds */
                            if ((DFU_IF == DFU_IF_NUM) && (sp.bRequest != XMOS_DFU_SAVESTATE) &&
                                (sp.bRequest != XMOS_DFU_RESTORESTATE))
                            {
                                // Stop audio
                                outuint(c_audioControl, SET_SAMPLE_FREQ);
                                outuint(c_audioControl, AUDIO_STOP_FOR_DFU);
                                // Handshake
							    chkct(c_audioControl, XS1_CT_END);
                            }

                            /* This will return 1 if reset requested */
                            if (DFUDeviceRequests(ep0_out, ep0_in, sp, null, g_interfaceAlt[sp.wIndex], 1))
                            {
                                timer tmr;
                                unsigned s;
                                tmr :> s;
                                tmr when timerafter(s + 50000000) :> s;
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
                                result = AudioClassRequests_2(ep0_out, ep0_in, sp, c_audioControl, c_mix_ctl, c_clk_ctl);
                            }
                            else
                            {
                                result = AudioClassRequests_1(ep0_out, ep0_in, sp, c_audioControl, c_mix_ctl, c_clk_ctl);
                            }
#elif (AUDIO_CLASS==2)
                            result = AudioClassRequests_2(ep0_out, ep0_in, sp, c_audioControl, c_mix_ctl, c_clk_ctl);
#else
                            result = AudioClassRequests_1(ep0_out, ep0_in, sp, c_audioControl, c_mix_ctl, c_clk_ctl);
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
                    devDesc_Audio2, sizeof(devDesc_Audio2),
                    cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                    devDesc_Audio1, sizeof(devDesc_Audio1),
                    cfgDesc_Audio1, sizeof(cfgDesc_Audio1),
                    strDescs, sizeof(strDescs)/sizeof(strDescs[0]),
                    sp, c_usb_test, g_curUsbSpeed);
#elif FULL_SPEED_AUDIO_2
                /* Return Audio 2.0 Descriptors for high_speed and full-speed */

                /* Unfortunately we need to munge the descriptors a bit between full and high-speed */
                if(g_curUsbSpeed == XUD_SPEED_HS)
                {
#if (NUM_USB_CHAN_OUT > 0)
                    /* Output interface - Interface 1 */
                    /* Mod bSlotSize */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+20] = SAMPLE_SUBSLOT_SIZE_HS;

                    /* Mod bBitResolution */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+21] = SAMPLE_BIT_RESOLUTION_HS;

                    /* wMaxPacketSize */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+26] = MAX_PACKET_SIZE_OUT_HS&0xff;
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+27] = (MAX_PACKET_SIZE_OUT_HS&0xff00)>>8;

                    /* bNrChannels */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+10] = NUM_USB_CHAN_OUT;
#endif
#if (NUM_USB_CHAN_IN > 0)
                    /* Input interface - Interface 2 */
                    /* Mod bSlotSize */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+20] = SAMPLE_SUBSLOT_SIZE_HS;

                    /* Mod bBitResolution */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+21] = SAMPLE_BIT_RESOLUTION_HS;

                    /* wMaxPacketSize */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+26] = MAX_PACKET_SIZE_IN_HS&0xff;
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+27] = (MAX_PACKET_SIZE_IN_HS&0xff00)>>8;

                    /* bNrChannels */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+10] = NUM_USB_CHAN_IN;
#endif
                }
                else
                {
#if (NUM_USB_CHAN_OUT > 0)
                    /* Output interface - Interface 1 */
                    /* Mod bSlotSize */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+20] = SAMPLE_SUBSLOT_SIZE_FS;

                    /* Mod bBitResolution */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+21] = SAMPLE_BIT_RESOLUTION_FS;

                    /* wMaxPacketSize */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+26] = MAX_PACKET_SIZE_OUT_FS&0xff;
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+27] = (MAX_PACKET_SIZE_OUT_FS&0xff00)>>8;

                    /* bNrChannels */
                    cfgDesc_Audio2[STREAMING_OUTPUT_ALT1_OFFSET+10] = NUM_USB_CHAN_OUT_FS;
#endif
#if (NUM_USB_CHAN_IN > 0)
                    /* Input interface - Interface 2 */
                    /* Mod bSlotSize */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+20] = SAMPLE_SUBSLOT_SIZE_FS;

                    /* Mod bBitResolution */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+21] = SAMPLE_BIT_RESOLUTION_FS;

                    /* wMaxPacketSize */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+26] = MAX_PACKET_SIZE_IN_FS&0xff;
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+27] = (MAX_PACKET_SIZE_IN_FS&0xff00)>>8;

                    /* bNrChannels */
                    cfgDesc_Audio2[STREAMING_INPUT_ALT1_OFFSET+10] = NUM_USB_CHAN_IN_FS;
#endif
                }

                result = USB_StandardRequests(ep0_out, ep0_in,
                    devDesc_Audio2, sizeof(devDesc_Audio2),
                    cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                    null, 0,
                    null, 0,
                    strDescs, sizeof(strDescs)/sizeof(strDescs[0]), sp, c_usb_test, g_curUsbSpeed);
#elif (AUDIO_CLASS == 1)
                /* Return Audio 1.0 Descriptors in FS, should never be in HS! */
                 result = USB_StandardRequests(ep0_out, ep0_in,
                    null, 0,
                    null, 0,
                    devDesc_Audio1, sizeof(devDesc_Audio1),
                    cfgDesc_Audio1, sizeof(cfgDesc_Audio1),
                    strDescs, sizeof(strDescs)/sizeof(strDescs[0]), sp, c_usb_test, g_curUsbSpeed);
#else
                /* Return Audio 2.0 Descriptors with Null device as fallback */
                result = USB_StandardRequests(ep0_out, ep0_in,
                    devDesc_Audio2, sizeof(devDesc_Audio2),
                    cfgDesc_Audio2, sizeof(cfgDesc_Audio2),
                    devDesc_Null, sizeof(devDesc_Null),
                    cfgDesc_Null, sizeof(cfgDesc_Null),
                    strDescs, sizeof(strDescs)/sizeof(strDescs[0]), sp, c_usb_test, g_curUsbSpeed);
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
                    strDescs, sizeof(strDescs), sp, c_usb_test, g_curUsbSpeed);
            }
#endif
        }

        if (result == XUD_RES_RST)
        {
            g_curUsbSpeed = XUD_ResetEndpoint(ep0_out, ep0_in);

            g_currentConfig = 0;

#ifdef DFU
            if (DFUReportResetState(null))
            {
                if (!DFU_mode_active)
                {
                    timer tmr;
                    unsigned s;
                    DFU_mode_active = 1;
                }
            }
            else
            {
                if (DFU_mode_active)
                {
                    timer tmr;
                    unsigned s;
                    DFU_mode_active = 0;

                    /* Send reboot command */
                    tmr :> s;
                    tmr when timerafter(s + 5000000) :> s;
                    device_reboot(c_audioControl);
                }
            }
#endif
        }

    }
}
