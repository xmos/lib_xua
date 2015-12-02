/**
 * @brief   Implements relevant requests from the USB Audio 2.0 Specification
 * @author  Ross Owen, XMOS Semiconductor
 */

#include <xs1.h>
#include "xud.h"
#include "usb_std_requests.h"
#include "usbaudio20.h"
#include "usbaudio10.h"
#include "dbcalc.h"
#include "devicedefines.h"
#include "commands.h"
#include "xc_ptr.h"
#ifdef MIXER
#include "mixer.h"
#endif

#define CS_XU_MIXSEL (0x06)

extern unsigned int multOut[NUM_USB_CHAN_OUT + 1];
extern unsigned int multIn[NUM_USB_CHAN_IN + 1];

extern int interfaceAlt[];

/* Global volume and mute tables */
extern int volsOut[];
extern unsigned int mutesOut[];

extern int volsIn[];
extern unsigned int mutesIn[];

/* Mixer settings */
#ifdef MIXER
extern unsigned char mixer1Crossbar[];
extern short mixer1Weights[];

/* Device channel mapping */
extern unsigned char channelMapAud[NUM_USB_CHAN_OUT];
extern unsigned char channelMapUsb[NUM_USB_CHAN_IN];

/* Mixer input mapping */
extern unsigned char mixSel[MAX_MIX_COUNT][MIX_INPUTS];
#endif

/* Global var for current frequency, set to default freq */
unsigned int g_curSamFreq = DEFAULT_FREQ;
//unsigned int g_curSamFreq48000Family = DEFAULT_FREQ % 48000 == 0;

#if 0
/* Original feedback implementation */
long long g_curSamFreqMultiplier = (DEFAULT_FREQ * 512 * 4) / (DEFAULT_MCLK_FREQ);
#endif

/* Store an int into a char array: Note this allows non-word aligned access unlike reinerpret cast */
static void storeInt(unsigned char buffer[], int index, int val)
{
    buffer[index+3] = val>>24;
    buffer[index+2] = val>>16;
    buffer[index+1] = val>>8;
    buffer[index]  =  val;
}

/* Store an short into a char array: Note this allows non-word aligned access unlike reinerpret cast */
static void storeShort(unsigned char buffer[], int index, short val)
{
    buffer[index+1] = val>>8;
    buffer[index]  =  val;
}

static void storeFreq(unsigned char buffer[], int &i, int freq)
{
    storeInt(buffer, i, freq);
    i+=4;
    storeInt(buffer, i, freq);
    i+=4;
    storeInt(buffer, i, 0);
    i+=4;
    return;
}

/* Delay based on USB speed. Feedback takes longer to stabilise at FS */
void FeedbackStabilityDelay()
{
    unsigned usbSpeed;
    timer t;
    unsigned time;
    unsigned delay;

    asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usbSpeed) :);

    if (usbSpeed == XUD_SPEED_HS)
    {
        delay = FEEDBACK_STABILITY_DELAY_HS;
    }
    else
    {
        delay = FEEDBACK_STABILITY_DELAY_FS;
    }

    t :> time;
    t when timerafter(time + delay):> void;
}



static unsigned longMul(unsigned a, unsigned b, int prec)
{
    unsigned x,y;
    unsigned ret;

    //    {x, y} = lmul(a, b, 0, 0);
    asm("lmul %0, %1, %2, %3, %4, %5":"=r"(x),"=r"(y):"r"(a),"r"(b),"r"(0),"r"(0));


    ret = (x << (32-prec) | (y >> prec));
    return ret;
}

#if 0
/* Original feedback implementation */
unsafe
{
    unsigned * unsafe curSamFreqMultiplier = &g_curSamFreqMultiplier;

static void setG_curSamFreqMultiplier(unsigned x)
{
   // asm(" stw %0, dp[g_curSamFreqMultiplier]" :: "r"(x));
    *curSamFreqMultiplier = x;
}
}
#endif

/* Update master volume i.e. i.e update weights for all channels */
static void updateMasterVol( int unitID, chanend ?c_mix_ctl)
{
    int x;
#ifndef OUT_VOLUME_IN_MIXER
    xc_ptr p_multOut = array_to_xc_ptr(multOut);
#endif
#ifndef IN_VOLUME_IN_MIXER
    xc_ptr p_multIn = array_to_xc_ptr(multIn);
#endif
    switch( unitID)
    {
        case FU_USBOUT:
            {
                unsigned master_vol = volsOut[0] == 0x8000 ? 0 : db_to_mult(volsOut[0], 8, 29);

                for (int i = 1; i < (NUM_USB_CHAN_OUT + 1); i++)
                {
                    /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
                    /* 0x8000 is a special value representing -inf (i.e. mute) */
                    unsigned vol = volsOut[i] == 0x8000 ? 0 : db_to_mult(volsOut[i], 8, 29);

                    x = longMul(master_vol, vol, 29) * !mutesOut[0] * !mutesOut[i];

#ifdef OUT_VOLUME_IN_MIXER
                    if (!isnull(c_mix_ctl))
                    {
                        outuint(c_mix_ctl, SET_MIX_OUT_VOL);
                        outuint(c_mix_ctl, i-1);
                        outuint(c_mix_ctl, x);
                        outct(c_mix_ctl, XS1_CT_END);
                    }
#else
                    asm("stw %0, %1[%2]"::"r"(x),"r"(p_multOut),"r"(i-1));
#endif
                }
            }
            break;

        case FU_USBIN:
            {
                unsigned master_vol = volsIn[0] == 0x8000 ? 0 : db_to_mult(volsIn[0], 8, 29);
                for (int i = 1; i < (NUM_USB_CHAN_IN + 1); i++)
                {
                    /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
                    /* 0x8000 is a special value representing -inf (i.e. mute) */
                    unsigned vol = volsIn[i] == 0x8000 ? 0 : db_to_mult(volsIn[i], 8, 29);

                    x = longMul(master_vol, vol, 29) * !mutesIn[0] * !mutesIn[i];

#ifdef IN_VOLUME_IN_MIXER
                    if (!isnull(c_mix_ctl))
                    {
                        outuint(c_mix_ctl, SET_MIX_IN_VOL);
                        outuint(c_mix_ctl, i-1);
                        outuint(c_mix_ctl, x);
                        outct(c_mix_ctl, XS1_CT_END);
                    }
#else
                    asm("stw %0, %1[%2]"::"r"(x),"r"(p_multIn),"r"(i-1));
#endif
                }
            }
            break;

        default:
            break;
    }
}

static void updateVol(int unitID, int channel, chanend ?c_mix_ctl)
{
    int x;
#ifndef OUT_VOLUME_IN_MIXER
    xc_ptr p_multOut = array_to_xc_ptr(multOut);
#endif
#ifndef IN_VOLUME_IN_MIXER
    xc_ptr p_multIn = array_to_xc_ptr(multIn);
#endif
    /* Check for master volume update */
    if (channel == 0)
    {
        updateMasterVol( unitID , c_mix_ctl);
    }
    else
    {
        switch( unitID )
        {
            case FU_USBOUT:
            {
                /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
                /* 0x8000 is a special value representing -inf (i.e. mute) */
                unsigned master_vol = volsOut[0] == 0x8000 ? 0 : db_to_mult(volsOut[0], 8, 29);
                unsigned vol = volsOut[channel] == 0x8000 ? 0 : db_to_mult(volsOut[channel], 8, 29);

                x = longMul(master_vol, vol, 29) * !mutesOut[0] * !mutesOut[channel];

#ifdef OUT_VOLUME_IN_MIXER
                if (!isnull(c_mix_ctl))
                {
                    outuint(c_mix_ctl, SET_MIX_OUT_VOL);
                    outuint(c_mix_ctl, channel-1);
                    outuint(c_mix_ctl, x);
                    outct(c_mix_ctl, XS1_CT_END);
                }
#else
                asm("stw %0, %1[%2]"::"r"(x),"r"(p_multOut),"r"(channel-1));
#endif
                break;
            }
           case FU_USBIN:
           {
                /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
                /* 0x8000 is a special value representing -inf (i.e. mute) */
                unsigned master_vol = volsIn[0] == 0x8000 ? 0 : db_to_mult(volsIn[0], 8, 29);
                 unsigned vol = volsIn[channel] == 0x8000 ? 0 : db_to_mult(volsIn[channel], 8, 29);

                x = longMul(master_vol, vol, 29) * !mutesIn[0] * !mutesIn[channel];

#ifdef IN_VOLUME_IN_MIXER
                if (!isnull(c_mix_ctl))
                {
                    outuint(c_mix_ctl, SET_MIX_IN_VOL);
                    outuint(c_mix_ctl, channel-1);
                    outuint(c_mix_ctl, x);
                    outct(c_mix_ctl, XS1_CT_END);
                }
#else
                asm("stw %0, %1[%2]"::"r"(x),"r"(p_multIn),"r"(channel-1));
#endif
                break;
            }
        }
    }
}

/* Handles the audio class specific requests
 * returns:     XUD_RES_OKAY if request dealt with successfully without error,
 *              XUD_RES_RST for device reset
 *              else XUD_RES_ERR
 */
int AudioClassRequests_2(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl
)
{
    unsigned char buffer[512];
    int unitID;
    XUD_Result_t result;
    unsigned datalength;

    /* Inspect request, NOTE: these are class specific requests */
    switch( sp.bRequest )
    {

        /* CUR Request*/
        case CUR:
        {
            /* Extract unitID from wIndex */
            unitID = sp.wIndex >> 8;

            switch( unitID )
            {
                /* Clock Unit(s) */
                case ID_CLKSRC_INT:
                case ID_CLKSRC_SPDIF:
                case ID_CLKSRC_ADAT:
                {
                    /* Check Control selector (CS) */
                    switch( sp.wValue >> 8 )
                    {
                        /* Sample Frequency control */
                        case CS_SAM_FREQ_CONTROL:
                        {
                            /* Direction: Host-to-device */
                            if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D)
                            {
                                /* Get OUT data with Sample Rate into buffer*/
                                if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                                {
                                    return result;
                                }

                                if(datalength == 4)
                                {
                                    /* Re-construct Sample Freq */
                                    int newSampleRate = buffer[0] | (buffer[1] << 8) | buffer[2] << 16 | buffer[3] << 24;

                                    /* Instruct audio thread to change sample freq (if change required) */
                                    if(newSampleRate != g_curSamFreq)
                                    {
                                        int newMasterClock;

                                        g_curSamFreq = newSampleRate;
#if 0
                                        /* Original feedback implementation */
                                        g_curSamFreq48000Family = ((MCLK_48 % g_curSamFreq) == 0);

                                        if(g_curSamFreq48000Family)
                                        {
                                            newMasterClock = MCLK_48;
                                        }
                                        else
                                        {
                                            newMasterClock = MCLK_441;
                                        }

                                        setG_curSamFreqMultiplier(g_curSamFreq/(newMasterClock/512));
#endif
#ifdef ADAT_RX
                                        /* Configure ADAT SMUX based on sample rate */
                                        outuint(c_clk_ctl, SET_SMUX);
                                        if(g_curSamFreq < 88200)
                                        {
                                            /* No SMUX */
                                            outuint(c_clk_ctl, 0);
                                        }
                                        else if(g_curSamFreq < 176400)
                                        {
                                            /* SMUX */
                                            outuint(c_clk_ctl, 1);
                                        }
                                        else
                                        {
                                            /* SMUX II */
                                            outuint(c_clk_ctl, 2);
                                        }
                                        outct(c_clk_ctl, XS1_CT_END);
#endif
                                        outuint(c_audioControl, SET_SAMPLE_FREQ);
                                        outuint(c_audioControl, g_curSamFreq);

                                        /* Wait for handshake back - i.e. PLL locked and clocks okay */
                                        chkct(c_audioControl, XS1_CT_END);

                                    }

                                    /* Allow time for our feedback to stabilise*/
                                    FeedbackStabilityDelay();
                                }

                                /* Send 0 Length as status stage */
                                XUD_DoSetRequestStatus(ep0_in);
                            }
                            /* Direction: Device-to-host: Send Current Sample Freq */
                            else
                            {
                                switch(unitID)
                                {
                                    case ID_CLKSRC_SPDIF:
                                    case ID_CLKSRC_ADAT:
#ifdef REPORT_SPDIF_FREQ
                                        /* Interogate clockgen thread for SPDIF freq */
                                        if (!isnull(c_clk_ctl))
                                        {
                                            outuint(c_clk_ctl, GET_FREQ);
                                            outuint(c_clk_ctl, CLOCK_SPDIF_INDEX);
                                            outct(c_clk_ctl, XS1_CT_END);
                                            (buffer, unsigned[])[0] = inuint(c_clk_ctl);
                                            chkct(c_clk_ctl, XS1_CT_END);
                                            return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 4, sp.wLength );
                                        }
                                        else
                                        {
                                            (buffer, unsigned[])[0] = g_curSamFreq;
                                            return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 4, sp.wLength );
                                        }

                                        break;
#endif
                                    case ID_CLKSRC_INT:
                                        /* Always report our current operating frequency */
                                        (buffer, unsigned[])[0] = g_curSamFreq;
                                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 4, sp.wLength );
                                        break;

                                    default:
                                        /* Unknown Unit ID in Sample Frequency Control Request: unitID */
                                        break;
                                }

                            }
                            break;
                        }

                        /* Clock Valid Control */
                        case CS_CLOCK_VALID_CONTROL:
                        {
                            switch(unitID)
                            {
                                case ID_CLKSRC_INT:

                                    /* Internal clock always valid */
                                    buffer[0] = 1;
                                    return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 1, sp.wLength);
                                    break;

                                case ID_CLKSRC_SPDIF:

                                    /* Interogate clockgen thread for validity */
                                    if (!isnull(c_clk_ctl))
                                    {
                                        outuint(c_clk_ctl, GET_VALID);
                                        outuint(c_clk_ctl, CLOCK_SPDIF_INDEX);
                                        outct(c_clk_ctl, XS1_CT_END);
                                        buffer[0] = inuint(c_clk_ctl);
                                        chkct(c_clk_ctl, XS1_CT_END);
                                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 1, sp.wLength);
                                    }

                                    break;

                                 case ID_CLKSRC_ADAT:

                                    if (!isnull(c_clk_ctl))
                                    {
                                        outuint(c_clk_ctl, GET_VALID);
                                        outuint(c_clk_ctl, CLOCK_ADAT_INDEX);
                                        outct(c_clk_ctl, XS1_CT_END);
                                        buffer[0] = inuint(c_clk_ctl);
                                        chkct(c_clk_ctl, XS1_CT_END);
                                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 1, sp.wLength);
                                    }
                                    break;

                                default:
                                    //Unknown Unit ID in Clock Valid Control Request
                                    break;
                            }
                            break;
                        }

                        default:
                            //Unknown Control Selector for Clock Unit: sp.wValue >> 8
                            break;

                    }
                    break; /* Clock Unit IDs */
                }

                /* Clock Selector Unit(s) */
                case ID_CLKSEL:
                {
                    if ((sp.wValue >> 8) == CX_CLOCK_SELECTOR_CONTROL)
                    {
                        /* Direction: Host-to-device */
                        if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D )
                        {
                            if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                            {
                                return result;
                            }

                            /* Check for correct datalength for clock sel */
                            if(datalength == 1)
                            {
                                if (!isnull(c_clk_ctl))
                                {
                                    outuint(c_clk_ctl, SET_SEL);
                                    outuint(c_clk_ctl, buffer[0]);
                                    outct(c_clk_ctl, XS1_CT_END);
                                }
                                /* Send 0 Length as status stage */
                                return XUD_DoSetRequestStatus(ep0_in);
                            }

                        }
                        else
                        {
                            /* Direction: Device-to-host: Send Current Selection */
                            buffer[0] = 1;
                            if (!isnull(c_clk_ctl))
                            {
                                outuint(c_clk_ctl, GET_SEL);
                                outct(c_clk_ctl, XS1_CT_END);
                                buffer[0] = inuint(c_clk_ctl);
                                chkct(c_clk_ctl, XS1_CT_END);
                            }
                            return XUD_DoGetRequest( ep0_out, ep0_in, buffer, 1, sp.wLength );

                        }
                    }
                    break;
                }

                /* Feature Units */
                case FU_USBOUT:
                case FU_USBIN:

                    /* Inspect Control Selector (CS) */
                    switch(sp.wValue >> 8)
                    {
                        case FU_VOLUME_CONTROL:

                            if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D)
                            {
                                /* Expect OUT here (with volume) */
                                if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                                {
                                    return result;
                                }

                                if(unitID == FU_USBOUT)
                                {
                                    if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT)
                                    {
                                        volsOut[ sp.wValue&0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                        return XUD_DoSetRequestStatus(ep0_in);
                                    }
                                }
                                else
                                {
                                    if ((sp.wValue & 0xff) <= NUM_USB_CHAN_IN)
                                    {
                                        volsIn[ sp.wValue&0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                        return XUD_DoSetRequestStatus(ep0_in);
                                    }
                                }
                            }
                            else /* Direction: Device-to-host */
                            {
                                if(unitID == FU_USBOUT)
                                {
                                    if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT)
                                    {
                                        buffer[0] = volsOut[ sp.wValue&0xff ];
                                        buffer[1] = volsOut[ sp.wValue&0xff ] >> 8;
                                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 2,  sp.wLength);
                                    }
                                }
                                else
                                {
                                    if ((sp.wValue & 0xff) <= NUM_USB_CHAN_IN)
                                    {
                                        buffer[0] = volsIn[ sp.wValue&0xff ];
                                        buffer[1] = volsIn[ sp.wValue&0xff ] >> 8;
                                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 2,  sp.wLength);
                                    }
                                }
                            }
                            break; /* FU_VOLUME_CONTROL */

                        case FU_MUTE_CONTROL:

                            if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D)
                            {
                                /* Expect OUT here with mute */
                                if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                                {
                                    return result;
                                }

                                if (unitID == FU_USBOUT)
                                {
                                    if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT)
                                    {
                                        mutesOut[sp.wValue & 0xff] = buffer[0];
                                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl);
                                        return XUD_DoSetRequestStatus(ep0_in);
                                    }
                                }
                                else
                                {
                                    if((sp.wValue & 0xff) <= NUM_USB_CHAN_IN)
                                    {
                                        mutesIn[ sp.wValue&0xff ] = buffer[0];
                                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl);
                                        return XUD_DoSetRequestStatus(ep0_in);
                                    }
                                }
                            }
                            else // Direction: Device-to-host
                            {
                                if(unitID == FU_USBOUT)
                                {
                                    if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT)
                                    {
                                        buffer[0] = mutesOut[sp.wValue&0xff];
                                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength, sp.wLength);
                                    }
                                }
                                else
                                {
                                    if((sp.wValue & 0xff) <= NUM_USB_CHAN_IN)
                                    {
                                        buffer[0] = mutesIn[ sp.wValue&0xff ];
                                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength, sp.wLength);
                                    }
                                }
                            }
                            break;

                        default:
                            // Unknown Control Selector for FU
                            break;
                    }

                    break; /* FU_USBIN */

#if defined(MIXER) && (MAX_MIX_COUNT > 0)
                case ID_XU_OUT:
                {
                    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D) /* Direction: Host-to-device */
                    {
                        unsigned volume = 0;
                        int c = sp.wValue & 0xff;


                        if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                        {
                            return result;
                        }

                        channelMapAud[c] = buffer[0] | buffer[1] << 8;

                        if (!isnull(c_mix_ctl))
                        {
                            if (c < NUM_USB_CHAN_OUT)
                            {
                                outuint(c_mix_ctl, SET_SAMPLES_TO_DEVICE_MAP);
                                outuint(c_mix_ctl, c);
                                outuint(c_mix_ctl, channelMapAud[c]);
                                outct(c_mix_ctl, XS1_CT_END);
                                /* Send 0 Length as status stage */
                                return XUD_DoSetRequestStatus(ep0_in);
                            }
                        }

                    }
                    else
                    {
                        buffer[0] = channelMapAud[sp.wValue & 0xff];
                        buffer[1] = 0;

                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength,  sp.wLength);
                    }

                }
                    break;

                case ID_XU_IN:
                    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D) /* Direction: Host-to-device */
                    {
                        unsigned volume = 0;
                        int c = sp.wValue & 0xff;

                        if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                        {
                            return result;
                        }

                        channelMapUsb[c] = buffer[0] | buffer[1] << 8;

                        if (c < NUM_USB_CHAN_IN)
                        {
                            if (!isnull(c_mix_ctl))
                            {
                                outuint(c_mix_ctl, SET_SAMPLES_TO_HOST_MAP);
                                outuint(c_mix_ctl, c);
                                outuint(c_mix_ctl, channelMapUsb[c]);
                                outct(c_mix_ctl, XS1_CT_END);
                                return XUD_DoSetRequestStatus(ep0_in);
                            }
                        }
                    }
                    else
                    {
                        /* Direction: Device-to-host */
                        buffer[0] = channelMapUsb[sp.wValue & 0xff];
                        buffer[1] = 0;
                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength,  sp.wLength);
                    }
                    break;

                case ID_XU_MIXSEL:
                {
                    int cs = sp.wValue >> 8;    /* Control Selector */
                    int cn = sp.wValue & 0xff;  /* Channel number */

                    /* Check for Get or Set */
                    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D)
                    {
                        /* Direction: Host-to-device */ /* Host-to-device */
                        if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                        {
                            return result;
                        }

                        if(datalength > 0)
                        {
                            /* cn bounds check for safety..*/
                            if(cn < MIX_INPUTS)
                            {
                                //if(cs == CS_XU_MIXSEL)
                                /* cs now contains mix number */
                                if(cs < (MAX_MIX_COUNT + 1))
                                {
                                    /* Check for "off" - update local state */
                                    if(buffer[0] == 0xFF)
                                    {
                                        mixSel[cs][cn] = (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT);
                                    }
                                    else
                                    {
                                        mixSel[cs][cn] = buffer[0];
                                    }

                                    if(cs == 0)
                                    {
                                        /* Update all mix maps */
                                        for (int i = 0; i < MAX_MIX_COUNT; i++)
                                        {
                                            outuint(c_mix_ctl, SET_MIX_MAP);
                                            outuint(c_mix_ctl, i);                  /* Mix bus */
                                            outuint(c_mix_ctl, cn);                 /* Mixer input */
                                            outuint(c_mix_ctl, (int) mixSel[cn]);   /* Source */
                                            outct(c_mix_ctl, XS1_CT_END);
                                        }
                                    }
                                    else
                                    {
                                        /* Update relevant mix map */
                                        outuint(c_mix_ctl, SET_MIX_MAP);          /* Command */
                                        outuint(c_mix_ctl, (cs-1));               /* Mix bus */
                                        outuint(c_mix_ctl, cn);                   /* Mixer input */
                                        outuint(c_mix_ctl, (int) mixSel[cs][cn]); /* Source */
                                        outct(c_mix_ctl, XS1_CT_END);             /* Wait for handshake back */
                                    }

                                    return XUD_DoSetRequestStatus(ep0_in);
                                }
                            }
                        }
                    }
                    else
                    {
                        /* Direction: Device-to-Host (GET) */
                        buffer[0] = 0;

                        /* Channel Number bounds check for safety */
                        if(cn < MIX_INPUTS)
                        {
                            /* Inspect control selector */
                            /* TODO ideally have a return for cs = 0. I.e all mix maps */
                            if((cs > 0) && (cs < (MAX_MIX_COUNT+1)))
                            {
                                buffer[0] = mixSel[cs-1][cn];
                                return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 1, 1 );
                            }
                        }
                    }
                    break;
                }

                case ID_MIXER_1:

                    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D) /* Direction: Host-to-device */
                    {
                        unsigned volume = 0;

                        /* Expect OUT here with mute */
                        if((result = XUD_GetBuffer(ep0_out, buffer, datalength)) != XUD_RES_OKAY)
                        {
                            return result;
                        }

                        mixer1Weights[sp.wValue & 0xff] = buffer[0] | buffer[1] << 8;

                        if (mixer1Weights[sp.wValue & 0xff] == 0x8000)
                        {
                            volume = 0;
                        }
                        else
                        {
                            volume = db_to_mult(mixer1Weights[sp.wValue & 0xff], 8, 25);
                        }
                        if (!isnull(c_mix_ctl))
                        {
                             outuint(c_mix_ctl, SET_MIX_MULT);
                             outuint(c_mix_ctl, (sp.wValue & 0xff) % 8);
                             outuint(c_mix_ctl, (sp.wValue & 0xff) / 8);
                             outuint(c_mix_ctl, volume);
                             outct(c_mix_ctl, XS1_CT_END);
                        }

                        /* Send 0 Length as status stage */
                        return XUD_DoSetRequestStatus(ep0_in);
                    }
                    else
                    {
                        short weight = mixer1Weights[sp.wValue & 0xff];
                        buffer[0] = weight & 0xff;
                        buffer[1] = (weight >> 8) & 0xff;

                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength,  sp.wLength);
                    }
                    break;

#endif
                default:
                    /* We dont have a unit with this ID! */
                    break;

            }  /* switch(sp.wIndex >> 8)   i.e Unit ID */
            break;
        }

        case RANGE:
        {
            unitID = sp.wIndex >> 8;

            switch( unitID )
            {
                /* Clock Source Units */
                case ID_CLKSRC_SPDIF:
                case ID_CLKSRC_ADAT:
                case ID_CLKSRC_INT:

                    /* Control Selector (CS) */
                    switch( sp.wValue >> 8 )
                    {
                        case CS_SAM_FREQ_CONTROL:

                            /* Currently always return all freqs for all clocks */
                            {
                                int num_freqs = 0;
                                int i = 2;

#ifndef SAMPLE_RATE_LIST
                                int currentFreq44 = 11025;  //MIN_FREQ_44;
                                int currentFreq48 = 8000;   //MIN_FREQ_48;
                                unsigned maxFreq = MAX_FREQ;

#if defined (FULL_SPEED_AUDIO_2)
                                unsigned usbSpeed;
                                asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usbSpeed) :);

                                if (usbSpeed == XUD_SPEED_FS)
                                {
                                    maxFreq = MAX_FREQ_FS;
                                }
#endif
                                /* Special case for some low sample rates */
                                unsigned lowSampleRateList[] = {8000, 11025, 12000, 16000, 32000};

                                for (int k = 0; k < sizeof(lowSampleRateList)/sizeof(unsigned); k++)
                                {
                                    if((lowSampleRateList[k] >= MIN_FREQ) && (lowSampleRateList[k] <= MAX_FREQ))
                                    {
                                        storeFreq(buffer, i, lowSampleRateList[k]);
                                        num_freqs++;
                                    }
                                }

                                /* Just keep doubling for standard freqs >= 44.1/48kHz */
                                currentFreq44 = 44100;
                                currentFreq48 = 48000;
                                while(1)
                                {
                                    if((currentFreq44 <= maxFreq) && (currentFreq44 >= MIN_FREQ))
                                    {
                                        storeFreq(buffer, i, currentFreq44);
                                        num_freqs++;
                                        currentFreq44*=2;
                                    }

                                    if((currentFreq48 <= maxFreq))
                                    {
                                        /* Note i passed byref here */
                                        storeFreq(buffer, i, currentFreq48);
                                        num_freqs++;
                                        currentFreq48*=2;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
#else
                                unsigned srList[] = {SAMPLE_RATE_LIST};
                                for(int j = 0; j < sizeof(srList)/(sizeof(srList[0])); j++)
                                {
                                    storeFreq(buffer, i, srList[j]);
                                    num_freqs++;
                                }
#endif

                                storeShort(buffer, 0, num_freqs);

                                return XUD_DoGetRequest(ep0_out, ep0_in, buffer, i, sp.wLength);
                            }
                            break;

                        default:
                            //Unknown Control Selector in Clock Source Range Request
                            break;
                    }

                    break;

                /* Feature Units */
                case FU_USBIN:      /* USB Data into Device */
                case FU_USBOUT:     /* USB Data from Device */

                    /* Control Selector (CS) */
                    switch( sp.wValue >> 8 )
                    {
                        /* Volume control, send back same range for all channels (i.e. ignore CN) */
                        case FU_VOLUME_CONTROL:

                            storeShort(buffer, 0, 1);
                            storeShort(buffer, 2, MIN_VOLUME);
                            storeShort(buffer, 4, MAX_VOLUME);
                            storeShort(buffer, 6, VOLUME_RES);
                            return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength, sp.wLength);
                            break;

                        default:
                            /* Unknown control selector for FU */
                            break;

                    }
                    break;

#ifdef MIXER
                /* Mixer Unit */
                case ID_MIXER_1:
                    storeShort(buffer, 0, 1);
                    storeShort(buffer, 2, MIN_MIXER_VOLUME);
                    storeShort(buffer, 4, MAX_MIXER_VOLUME);
                    storeShort(buffer, 6, VOLUME_RES_MIXER);
                    return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength, sp.wLength);
                    break;
#endif

                default:
                    /* Unknown Unit ID in Range Request selector for FU */
                    break;

            }

            break; /* case: RANGE */
        }

#if defined (MIXER) && (MAX_MIX_COUNT > 0)
        case MEM:   /* Memory Requests (5.2.7.1) */

            unitID = sp.wIndex >> 8;

            switch( unitID )
            {
                case ID_MIXER_1:

                    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_D2H)
                    {
                        int length = 0;

                        /* Device-to-Host (GET) */
                        switch(sp.wValue) /* offset */
                        {
                            case 0: /* Input levels */
                                length = (NUM_USB_CHAN_IN + NUM_USB_CHAN_OUT) * 2; /* 2 bytes per chan */

                                for(int i = 0; i < (NUM_USB_CHAN_IN + NUM_USB_CHAN_OUT); i++)
                                {
                                	/* Get the level and truncate to 16-bit */
                                	if(i < NUM_USB_CHAN_OUT)
                                    {
                                        if (!isnull(c_mix_ctl))
                                        {
                                            outuint(c_mix_ctl, GET_STREAM_LEVELS);
                                            outuint(c_mix_ctl, i);
                                            outct(c_mix_ctl, XS1_CT_END);
                                            storeShort(buffer, i*2, (inuint(c_mix_ctl) >> 15));
                                            chkct(c_mix_ctl, XS1_CT_END);
										}
                                        else
                                        {
                                            storeShort(buffer, i*2, 0);
										}
                                	}
                                    else
                                    {
                                        if (!isnull(c_mix_ctl))
                                        {
                                            outuint(c_mix_ctl, GET_INPUT_LEVELS);
                                            outuint(c_mix_ctl, (i - NUM_USB_CHAN_OUT));
                                            outct(c_mix_ctl, XS1_CT_END);
                                            storeShort(buffer, i*2, (inuint(c_mix_ctl) >> 15));
                                            chkct(c_mix_ctl, XS1_CT_END);
										}
                                        else
                                        {
                                            storeShort(buffer, i*2, 0);
										}
                                	}
                                }

                                break;

                            case 1: /* Mixer Output levels */
                                length = MAX_MIX_COUNT * 2; /* 2 bytes per chan */

                                for(int i = 0; i < MAX_MIX_COUNT; i++)
                                {
                                    if (!isnull(c_mix_ctl))
                                    {
                                        outuint(c_mix_ctl, GET_OUTPUT_LEVELS);
                                        outuint(c_mix_ctl, i);
                                        outct(c_mix_ctl, XS1_CT_END);
                                        storeShort(buffer, i*2, (inuint(c_mix_ctl) >> 15));
                                        chkct(c_mix_ctl, XS1_CT_END);
									}
                                    else
                                    {
                                         storeShort(buffer, i*2, 0);
									}
                                }

                                break;
                        }
                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, length, sp.wLength);
                    }
                    break;
            }
            break;
#endif
    }

    /* Didn't deal with request, return XUD_RES_ERR */
    return XUD_RES_ERR;

}

#if defined (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS==1)

int AudioEndpointRequests_1(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl)
{
    /* At this point we know:
     * bmRequestType.Recipient = Endpoint
     * bmRequestType.Type = Class
     * endpoint (wIndex & 0xff) is 0x01 or 0x82
     */

    XUD_Result_t result;
    unsigned char buffer[1024];
    unsigned length;

    /* Host to Device */
    if(sp.bmRequestType.Direction == USB_BM_REQTYPE_DIRECTION_H2D)
    {
        /* Inspect for request */
        switch(sp.bRequest)
        {
            case UAC_B_REQ_SET_CUR:
            {
                /* Check Control Selector */
                unsigned short controlSelector = sp.wValue>>8;

                if((result != XUD_GetBuffer(ep0_out, buffer, length)) != XUD_RES_OKAY)
                {
                    return result;
                }

                if(controlSelector == SAMPLING_FREQ_CONTROL)
                {
                    /* Expect length 3 for sample rate */
                    if((sp.wLength == 3) && (length == 3))
                    {
                        /* Recontruct sample-freq */
                        int newSampleRate = buffer[0] | (buffer [1] << 8) | (buffer[2] << 16);

                        if(newSampleRate != g_curSamFreq)
                        {
                            int curSamFreq44100Family;
                            int curSamFreq48000Family;

                            /* Windows Audio Class driver has a nice habbit of sending invalid SF's (e.g. 48001Hz)
                             * when under stress.  Lets double check it here and ignore if not valid. */
                            curSamFreq48000Family = MCLK_48 % newSampleRate  == 0;
                            curSamFreq44100Family = MCLK_441 % newSampleRate == 0;

                            if(curSamFreq48000Family || curSamFreq44100Family)
                            {
                                g_curSamFreq = newSampleRate;
#if 0
                                /* Original feedback implementation */

                                int newMasterClock;

                                if(g_curSamFreq48000Family)
                                {
                                    newMasterClock = MCLK_48;
                                }
                                else
                                {
                                    newMasterClock = MCLK_441;
                                }

                                setG_curSamFreqMultiplier((g_curSamFreq*512*4)/newMasterClock);
#endif

                                /* Instruct audio thread to change sample freq */
                                outuint(c_audioControl, SET_SAMPLE_FREQ);
                                outuint(c_audioControl, g_curSamFreq);

                                /* Wait for handshake back - i.e. pll locked and clocks okay */
                                chkct(c_audioControl, XS1_CT_END);

                                /* Allow time for the change - feedback to stabilise */
                                FeedbackStabilityDelay();
                                                            }
                        }
                        return XUD_SetBuffer(ep0_in, buffer, 0);
                    }
                }
            }
            break;
        }
    }
    else // sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_D2H
    {
        switch(sp.bRequest)
        {
            case UAC_B_REQ_GET_CUR:
                (buffer, unsigned[])[0] = g_curSamFreq;
                return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 3, sp.wLength);
                break;
        }
    }

    /* Return 1 for not handled */
    return 1;
}


/* Handles the Audio Class 1.0 specific requests */
int AudioClassRequests_1(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl
)
{
    unsigned char buffer[1024];
    unsigned unitID;
    XUD_Result_t result;

    /* Inspect request */
    /* Note we could check sp.bmRequestType.Direction if we wanted to be really careful */
    switch(sp.bRequest)
    {
        case UAC_B_REQ_SET_CUR:
        {
            unsigned datalength;

            if((result = XUD_GetBuffer(ep0_out, buffer,  datalength)) != XUD_RES_OKAY)
            {
                return result;
            }

            unitID = sp.wIndex >> 8;

            switch ((sp.wValue>>8) & 0xff)
            {
                case FU_VOLUME_CONTROL:

                    if(datalength == 2)
                    {
                        switch(unitID)
                        {
                            case FU_USBOUT:
                                volsOut[ sp.wValue & 0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                                updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                return XUD_DoSetRequestStatus(ep0_in);

                            case FU_USBIN:
                                volsIn[ sp.wValue & 0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                                updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                return XUD_DoSetRequestStatus(ep0_in);
                        }
                    }
                    break;

                case FU_MUTE_CONTROL:

                    if(datalength == 1)
                    {
                        switch(unitID)
                        {
                            case FU_USBOUT:
                                mutesOut[ sp.wValue & 0xff ] = buffer[0];
                                updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                return XUD_DoSetRequestStatus(ep0_in);

                            case FU_USBIN:
                                mutesIn[ sp.wValue & 0xff ] = buffer[0];
                                updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                return XUD_DoSetRequestStatus(ep0_in);
                        }
                    }
                    break;
            }
            break;
        }
        case UAC_B_REQ_GET_CUR:
        {
            unitID = sp.wIndex >> 8;
            if (unitID == FU_USBOUT)
            {
                switch ((sp.wValue>>8) & 0xff)
                {
                    case FU_VOLUME_CONTROL:
                    {
                        buffer[0] = volsOut[ sp.wValue&0xff ];
                        buffer[1] = volsOut[ sp.wValue&0xff ] >> 8;
                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 2, sp.wLength);
                        break;
                    }
                    case FU_MUTE_CONTROL:
                    {
                        buffer[0] = mutesOut[ sp.wValue & 0xff ];
                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 1, sp.wLength);
                        break;
                    }
                }
            }
            else if (unitID == FU_USBIN)
            {
                switch ((sp.wValue>>8) & 0xff)
                {
                    case FU_VOLUME_CONTROL:
                    {
                        buffer[0] = volsIn[ sp.wValue&0xff ];
                        buffer[1] = volsIn[ sp.wValue&0xff ] >> 8;
                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 2, sp.wLength);
                    }
                    case FU_MUTE_CONTROL:
                    {
                        buffer[0] = mutesIn[ sp.wValue & 0xff ];
                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 1, sp.wLength);
                    }
                }
            }
            break;
        }
        case UAC_B_REQ_GET_MIN:
            buffer[0] = (MIN_MIXER_VOLUME & 0xff);
            buffer[1] = (MIN_MIXER_VOLUME >> 8);
            return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 2, sp.wLength);

        case UAC_B_REQ_GET_MAX:
            buffer[0] = (MAX_MIXER_VOLUME & 0xff);
            buffer[1] = (MAX_MIXER_VOLUME >> 8);
            return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 2, sp.wLength);

        case UAC_B_REQ_GET_RES:
            buffer[0] = (VOLUME_RES_MIXER & 0xff);
            buffer[1] = (VOLUME_RES_MIXER >> 8);
            return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 2, sp.wLength);
    }

    return 1;
}
#endif

