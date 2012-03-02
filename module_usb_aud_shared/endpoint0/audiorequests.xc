/**
 * @file    AudioRequests.xc
 * @brief   Implements relevant requests from the USB Audio 2.0 Specification
 * @author  Ross Owen, XMOS Semiconductor
 * @version 1.4
 */

#include <xs1.h>
//#include <print.h>

#include "xud.h"
#include "usb.h"
#include "usbaudio20.h"
#include "dbcalc.h"
#include "devicedefines.h"
#include "common.h"
#include "clockcmds.h"
#ifdef MIXER
#include "mixer.h"
#endif
#include <print.h>

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
#if NUM_USB_CHAN_OUT > 0
extern unsigned char channelMapAud[NUM_USB_CHAN_OUT];
#endif
#if NUM_USB_CHAN_IN > 0
extern unsigned char channelMapUsb[NUM_USB_CHAN_IN];
#endif

/* Mixer input mapping */
extern unsigned char mixSel[MIX_INPUTS];
#endif

/* Global var for current frequency */
extern unsigned int g_curSamFreq;
extern unsigned int g_curSamFreq48000Family;
extern unsigned int g_curSamFreqMultiplier;

/* Store an int into a char array: Note this allows non-word aligned access unlike reinerpret cast */
void storeInt(unsigned char buffer[], int index, int val)
{
    buffer[index+3] = val>>24;
    buffer[index+2] = val>>16;
    buffer[index+1] = val>>8;
    buffer[index]  =  val;
}

/* Store an short into a char array: Note this allows non-word aligned access unlike reinerpret cast */
void storeShort(unsigned char buffer[], int index, short val)
{
    buffer[index+1] = val>>8;
    buffer[index]  =  val;
}

void storeFreq(unsigned char buffer[], int &i, int freq)
{
  storeInt(buffer, i, freq);
  i+=4;
  storeInt(buffer, i, freq);
  i+=4;
  storeInt(buffer, i, 0);
  i+=4;
  return;
}


unsigned longMul(unsigned a, unsigned b, int prec) 
{
    unsigned x,y;
    unsigned ret;

    //    {x, y} = lmul(a, b, 0, 0);
    asm("lmul %0, %1, %2, %3, %4, %5":"=r"(x),"=r"(y):"r"(a),"r"(b),"r"(0),"r"(0));


    ret = (x << (32-prec) | (y >> prec));
    return ret;
}

void setG_curSamFreqMultiplier(int x) {
    asm(" stw %0, dp[g_curSamFreqMultiplier]" :: "r"(x));
}

/* Update master volume i.e. i.e update weights for all channels */
void updateMasterVol( int unitID, chanend ?c_mix_ctl)
{
    int x;
    switch( unitID)
    {
        case FU_USBOUT:
            for (int i = 1; i < (NUM_USB_CHAN_OUT + 1); i++)
            {
              /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
              /* 0x8000 is a special value representing -inf (i.e. mute) */
              unsigned master_vol = volsOut[0] == 0x8000 ? 0 : db_to_mult(volsOut[0], 8, 29);
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
              asm("stw %0, %1[%2]"::"r"(x),"r"(multOut),"r"(i-1));
#endif

            }
            break;

        case FU_USBIN:
            for (int i = 1; i < (NUM_USB_CHAN_IN + 1); i++) 
            {
              /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
              /* 0x8000 is a special value representing -inf (i.e. mute) */
              unsigned master_vol = volsIn[0] == 0x8000 ? 0 : db_to_mult(volsIn[0], 8, 29);
              unsigned vol = volsIn[i] == 0x8000 ? 0 : db_to_mult(volsIn[i], 8, 29);

              x = longMul(master_vol, vol, 29) * !mutesIn[0] * !mutesIn[i];

#ifdef IN_VOLUME_IN_MIXER
              if (!isnull(c_mix_ctl))
               {
                //master 
                //{
                  //c_mix_ctl <: SET_MIX_IN_VOL;
                  //c_mix_ctl <: i-1;
                  //c_mix_ctl <: x;
                //}
                  outuint(c_mix_ctl, SET_MIX_IN_VOL); 
                outuint(c_mix_ctl, i-1);
                outuint(c_mix_ctl, x);
                outct(c_mix_ctl, XS1_CT_END);



              }
#else
                asm("stw %0, %1[%2]"::"r"(x),"r"(multIn),"r"(i-1));
#endif
            }
            break;

        default:
            XUD_Error_hex("MVol: No such unit: ", unitID);
            break;
    }
}  

void updateVol(int unitID, int channel, chanend ?c_mix_ctl)
{   
    int x;

    /* Check for master volume update */
    if (channel == 0)
    {
        updateMasterVol( unitID , c_mix_ctl);
    }
    else
    {
        switch( unitID )
        {
            case FU_USBOUT: {
              /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
              /* 0x8000 is a special value representing -inf (i.e. mute) */
              unsigned master_vol = volsOut[0] == 0x8000 ? 0 : db_to_mult(volsOut[0], 8, 29);
              unsigned vol = volsOut[channel] == 0x8000 ? 0 : db_to_mult(volsOut[channel], 8, 29);

              x = longMul(master_vol, vol, 29) * !mutesOut[0] * !mutesOut[channel];

#ifdef OUT_VOLUME_IN_MIXER
              if (!isnull(c_mix_ctl))
                {
                //master {
                 // c_mix_ctl <: SET_MIX_OUT_VOL;
                 // c_mix_ctl <: channel-1;
                 // /c_mix_ctl <: x;
                //}
                outuint(c_mix_ctl, SET_MIX_OUT_VOL); 
                outuint(c_mix_ctl, channel-1);
                outuint(c_mix_ctl, x);
                outct(c_mix_ctl, XS1_CT_END);

              }



#else
                asm("stw %0, %1[%2]"::"r"(x),"r"(multOut),"r"(channel-1));  
#endif


                break;
            }
           case FU_USBIN: {
              /* Calc multipliers with 29 fractional bits from a db value with 8 fractional bits */
              /* 0x8000 is a special value representing -inf (i.e. mute) */
              unsigned master_vol = volsIn[0] == 0x8000 ? 0 : db_to_mult(volsIn[0], 8, 29);
              unsigned vol = volsIn[channel] == 0x8000 ? 0 : db_to_mult(volsIn[channel], 8, 29);

              x = longMul(master_vol, vol, 29) * !mutesIn[0] * !mutesIn[channel];

#ifdef IN_VOLUME_IN_MIXER
              if (!isnull(c_mix_ctl)) {
                //master {
                 // c_mix_ctl <: SET_MIX_IN_VOL;
                 // c_mix_ctl <: channel-1;
                 // c_mix_ctl <: x;
                //}
                outuint(c_mix_ctl, SET_MIX_IN_VOL); 
                outuint(c_mix_ctl, channel-1);
                outuint(c_mix_ctl, x);
                outct(c_mix_ctl, XS1_CT_END);


              }
#else
                asm("stw %0, %1[%2]"::"r"(x),"r"(multIn),"r"(channel-1));  
#endif            
            break;
            }
            default: 
                /* Don't hit */
                 XUD_Error_hex("Vol: No such unit: ",  unitID);
                break;
        }
    }
}

#ifdef EP0_THREAD_COMBINED_WITH_SPI
void spi(chanend c_spi, chanend c_spi_ss);
#endif

/* Handles the audio class specific requests 
 * returns:     0   if request delt with successfully without error, 
 *              <0  for device reset suspend 
 *              else 1 
 */
int AudioClassRequests_2(XUD_ep ep0_out, XUD_ep ep0_in, SetupPacket &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl
#ifdef EP0_THREAD_COMBINED_WITH_SPI
  , chanend c_spi, chanend c_spi_ss
#endif
)
{
    unsigned char buffer[128];
    int i_tmp;
    int unitID;
    int loop = 1;
    int datalength;

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
                case ID_CLKSRC_EXT: 
                case ID_CLKSRC_ADAT:
                {
                    /* Check Control selector (CS) */
                    switch( sp.wValue >> 8 )
                    {
                        /* Sample Frequency control */
                        case CS_SAM_FREQ_CONTROL: 
                        {
                            /* Direction: Host-to-device */
                            if( sp.bmRequestType.Direction == 0 ) 
                            {
                                /* Get OUT data with Sample Rate into buffer*/
                                datalength = XUD_GetBuffer(ep0_out, buffer);
                                                      
                                /* Check for reset/suspend */
                                if(datalength < 0)
                                {
                                    return datalength;
                                }
                                                       
                                if(datalength == 4)
                                {
                                                        
                                    /* Re-construct Sample Freq */
                                    i_tmp = buffer[0] | (buffer[1] << 8) | buffer[2] << 16 | buffer[3] << 24; 

                                    /* Instruct audio thread to change sample freq */
                                    g_curSamFreq = i_tmp;
                                    g_curSamFreq48000Family = g_curSamFreq % 48000 == 0;

                                    if(g_curSamFreq48000Family)
                                    {
                                        i_tmp = MCLK_48;
                                    }
                                    else
                                    {
                                        i_tmp = MCLK_441;
                                    }

                                    setG_curSamFreqMultiplier(g_curSamFreq/(i_tmp/512));

                                    outuint(c_audioControl, SET_SAMPLE_FREQ);
                                    outuint(c_audioControl, g_curSamFreq); 

#ifdef EP0_THREAD_COMBINED_WITH_SPI
				                    spi(c_spi, c_spi_ss);  /* CodecConfig */
#endif

                                    /* Wait for handshake back - i.e. pll locked and clocks okay */
                                    chkct(c_audioControl, XS1_CT_END);

                                    /* Allow time for our feedback to stabalise*/
                                    {
                                        timer t;
                                        unsigned time;         
                                        t :> time;
                                        t when timerafter(time+5000000):> void;
                                    }
                                }

                                                        
                                /* Send 0 Length as status stage */
                                return XUD_SetBuffer_ResetPid(ep0_in,   buffer, 0, PIDn_DATA1);
                                
                            }
                            /* Direction: Device-to-host: Send Current Sample Freq */
                            else
                            {
                                switch(unitID) 
                                {
                                    case ID_CLKSRC_EXT:
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
                                        }
                                        else
                                        {

                                            (buffer, unsigned[])[0] = g_curSamFreq;
                                        }
                                       
                                        break; 
#endif
                                    case ID_CLKSRC_INT:

                                        /* Always report our current operating frequency */
                                        (buffer, unsigned[])[0] = g_curSamFreq;
                                        break;
                                                        
                                    default:
                                        XUD_Error_hex("Unknown Unit ID in Sample Frequency Control Request", unitID); break;
                                }
                                
                                return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength, sp.wLength );
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
                                    break;
                                    
                                case ID_CLKSRC_EXT:

                                    /* Interogate clockgen thread for validity */
                                    if (!isnull(c_clk_ctl)) 
                                    {
                                        outuint(c_clk_ctl, GET_VALID);
                                        outuint(c_clk_ctl, CLOCK_SPDIF_INDEX);
                                        outct(c_clk_ctl, XS1_CT_END);
                                        buffer[0] = inuint(c_clk_ctl);
                                        chkct(c_clk_ctl, XS1_CT_END);
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
                                    }


                                    break;
                                
                                default:
                                    XUD_Error_hex("Unknown Unit ID in Clock Valid Control Request: ", unitID); 
                                    break;
                            }
                            
                            return XUD_DoGetRequest( ep0_out, ep0_in, buffer, sp.wLength, sp.wLength );

                            break;
                        }

                        default:
                            XUD_Error_hex("Unknown Control Selector for Clock Unit: ", sp.wValue >> 8 );
                            break;
                                                
                    }
                    break; /* Clock Unit IDs */
                }

                /* Clock Selector Unit(s) */
                case ID_CLKSEL:  
                {
                    if ((sp.wValue >> 8) == CX_CLOCK_SELECTOR_CONTROL) 
                    {
                        if( sp.bmRequestType.Direction == 0 ) 
                        { 
                            /* Direction: Host-to-device */
                            datalength = XUD_GetBuffer(ep0_out, buffer);
                            
                            if(datalength < 0)
                                return datalength;

                            /* Check for correct datalength for clock sel */
                            if(datalength == 1) 
                            {
                              
                                if (!isnull(c_clk_ctl)) 
                                {
                                    outuint(c_clk_ctl, SET_SEL);
                                    outuint(c_clk_ctl, buffer[0]);
                                    outct(c_clk_ctl, XS1_CT_END);
                                }
                            }
                                                                            
                            /* Send 0 Length as status stage */
                            return XUD_DoSetRequestStatus(ep0_in, 0);
                        } 
                        else 
                        {  
                            buffer[0] = 1; 
                            
                            /* Direction: Device-to-host: Send Current Selection */
                            
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
                    else 
                    {
                        XUD_Error_hex("Unknown control on clock selector", sp.wValue);
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
                            
                            if(sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_OUT) /* Direction: Host-to-device */
                            {
                                /* Expect OUT here (with v2yyolume) */
                                loop = XUD_GetBuffer(ep0_out, buffer);
                               
                                /* Check for rst/suspend */ 
                                if(loop < 0)
                                return loop;
                                
                                if(unitID == FU_USBOUT)
                                {    
                                  if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT) {   
                                    volsOut[ sp.wValue&0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                                    updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                  }
                                }
                                else
                                {
                                  if ((sp.wValue & 0xff) <= NUM_USB_CHAN_IN) {   
                                    volsIn[ sp.wValue&0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                                    updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                                  }
                                }

                                /* Send 0 Length as status stage */
                                return XUD_DoSetRequestStatus(ep0_in, 0);
                            }
                            else /* Direction: Device-to-host */
                            {
                                if(unitID == FU_USBOUT)
                                {
                                  if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT) {   
                                    buffer[0] = volsOut[ sp.wValue&0xff ];
                                    buffer[1] = volsOut[ sp.wValue&0xff ] >> 8;
                                  }
                                  else {
                                    buffer[0] = buffer[1] = 0;
                                  }
                                }
                                else
                                {
                                  if ((sp.wValue & 0xff) <= NUM_USB_CHAN_IN) {   
                                    buffer[0] = volsIn[ sp.wValue&0xff ];
                                    buffer[1] = volsIn[ sp.wValue&0xff ] >> 8;
                                  }
                                  else {
                                    buffer[0] = buffer[1] = 0;
                                  }
                                }
                                return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength,  sp.wLength); 
                            }
                            break; /* FU_VOLUME_CONTROL */
                                                
                        case FU_MUTE_CONTROL: 
                                                    
                            if(sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_OUT) // Direction: Host-to-device
                            {
                                /* Expect OUT here with mute */
                                loop = XUD_GetBuffer(ep0_out, buffer);

                                if(loop < 0)
                                    return loop;

                                if (unitID == FU_USBOUT)
                                {
                                  if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT) {   
                                    mutesOut[sp.wValue & 0xff] = buffer[0];
                                    updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl);
                                  }
                                }
                                else
                                {
                                    mutesIn[ sp.wValue&0xff ] = buffer[0];
                                    updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl);
                                }

                                /* Send 0 Length as status stage */
                                return XUD_DoSetRequestStatus(ep0_in, 0);
                            }
                            else // Direction: Device-to-host
                            {
                                if(unitID == FU_USBOUT)
                                {
                                  if ((sp.wValue & 0xff) <= NUM_USB_CHAN_OUT) {   
                                    buffer[0] = mutesOut[sp.wValue&0xff];
                                  }
                                  else {
                                    buffer[0] = 0;
                                  }
                                }
                                else
                                {
                                    buffer[0] = mutesIn[ sp.wValue&0xff ];
                                }
                                return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength, sp.wLength); 
                            }
                            break;
                        
                        //default:
                          //  XUD_Error("Unknown Control Selector for FU");
                            //break;
                    }
                     
                    break; /* FU_USBIN */
      
#ifdef MIXER
                case ID_XU_OUT:
                {
                    if(sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_OUT) /* Direction: Host-to-device */
                    {
                        unsigned volume = 0;
                        int c = sp.wValue & 0xff;

                        loop = XUD_GetBuffer(ep0_out, buffer);

                        if(loop < 0)
                            return loop;

                        channelMapAud[c] = buffer[0] | buffer[1] << 8;

                        if (!isnull(c_mix_ctl)) 
                        {
                            if (c < NUM_USB_CHAN_OUT) 
                            {
                                //master {
                                 //   c_mix_ctl <: SET_SAMPLES_TO_DEVICE_MAP;
                                  //  c_mix_ctl <: c;
                                   // c_mix_ctl <: (int) channelMapAud[c];
                                //}
                                outuint(c_mix_ctl, SET_SAMPLES_TO_DEVICE_MAP);
                                outuint(c_mix_ctl, c);
                                outuint(c_mix_ctl, channelMapAud[c]);
                                outct(c_mix_ctl, XS1_CT_END);
                                                                       
                            }
                        }

                        /* Send 0 Length as status stage */
                        return XUD_DoSetRequestStatus(ep0_in, 0);
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
                {
                    if(sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_OUT) /* Direction: Host-to-device */
                    {
                        unsigned volume = 0;
                        int c = sp.wValue & 0xff;

                        loop = XUD_GetBuffer(ep0_out, buffer);

                        if(loop < 0)
                            return loop;

                        channelMapUsb[c] = buffer[0] | buffer[1] << 8;

                        if (!isnull(c_mix_ctl)) 
                        {
                            if (c < NUM_USB_CHAN_IN) 
                            {
                                //master {
                                 //   c_mix_ctl <: SET_SAMPLES_TO_HOST_MAP;
                                 //   c_mix_ctl <: c;
                                  //  c_mix_ctl <: (int) channelMapUsb[c];
                                //} 
                                outuint(c_mix_ctl, SET_SAMPLES_TO_HOST_MAP);
                                outuint(c_mix_ctl, c);
                                outuint(c_mix_ctl, channelMapUsb[c]);
                                outct(c_mix_ctl, XS1_CT_END);
                                   
                            }
                        }

                        /* Send 0 Length as status stage */
                        return XUD_DoSetRequestStatus(ep0_in, 0);
                    }
                    else
                    {
                        buffer[0] = channelMapUsb[sp.wValue & 0xff];
                        buffer[1] = 0;

                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, sp.wLength,  sp.wLength);
                    }

                }
                    break;


                case ID_XU_MIXSEL:
                {
                    int cs = sp.wValue >> 8; /* Control Selector */
                    int cn = sp.wValue & 0xff; /* Channel number */

                    /* Check for Get or Set */
                    if(sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_OUT)                     
                    {
                        /* Direction: Host-to-device */ /* Host-to-device */   
                        datalength = XUD_GetBuffer(ep0_out, buffer);
                                                      
                        /* Check for reset/suspend */
                        if(datalength < 0)
                        {
                            return datalength;
                        }
    
                        if(datalength > 0)
                        {    
                            /* cn bounds check for safety..*/                    
                            if(cn < MIX_INPUTS)
                            {
                                if(cs == CS_XU_MIXSEL)
                                {
                                    /* Check for "off" - update local state */
                                    if(buffer[0] == 0xFF)
                                        mixSel[cn] = (NUM_USB_CHAN_OUT + NUM_USB_CHAN_IN + MAX_MIX_COUNT);
                                    else
                                        mixSel[cn] = buffer[0];

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
                            }

                        }

                        /* Send 0 Length as status stage */
                        return XUD_DoSetRequestStatus(ep0_in, 0);


                    }
                    else
                    {
                        /* Direction: Device-to-Host (GET) */
                        buffer[0] = 0; 
                        
                        /* Channel Number bounds check for safety */
                        if(cn < MIX_INPUTS)
                        {
                            /* Inspect control selector */
                            if(cs == CS_XU_MIXSEL)
                            {
                                buffer[0] = mixSel[cn];
                            }
                        }

                        return XUD_DoGetRequest(ep0_out, ep0_in, buffer, 1, 1 );


                    }
                    break;
                }
                
                case ID_MIXER_1:
                    
                    if(sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_OUT) /* Direction: Host-to-device */
                    {
                        unsigned volume = 0;
                        /* Expect OUT here with mute */
                        loop = XUD_GetBuffer(ep0_out, buffer);

                        if(loop < 0)
                            return loop;
                        
                        mixer1Weights[sp.wValue & 0xff] = buffer[0] | buffer[1] << 8;
 
                        if (mixer1Weights[sp.wValue & 0xff] == 0x8000) {
                          volume = 0;
                        }
                        else {
                          volume = db_to_mult(mixer1Weights[sp.wValue & 0xff], 8, 25);  
                        }
                        if (!isnull(c_mix_ctl)) 
                        {
                          //master {
                           // c_mix_ctl <: SET_MIX_MULT;
                            //c_mix_ctl <: (sp.wValue & 0xff) % 8;
                            //c_mix_ctl <: (sp.wValue & 0xff) / 8;
                            ///c_mix_ctl <: volume;
                          //}/
                             outuint(c_mix_ctl, SET_MIX_MULT);
                             outuint(c_mix_ctl, (sp.wValue & 0xff) % 8);
                             outuint(c_mix_ctl, (sp.wValue & 0xff) / 8);
                             outuint(c_mix_ctl, volume);
                             outct(c_mix_ctl, XS1_CT_END);  

                        }

                        /* Send 0 Length as status stage */
                        return XUD_DoSetRequestStatus(ep0_in, 0);
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
                //default:
                                         
                    ///* We dont have a unit with this ID! */
                    //XUD_Error_hex("ERR: Unknown control unit: ", sp.wIndex);
                    //break;
                                          
            }  /* switch(sp.wIndex >> 8)   i.e Unit ID */           
            break;
        }
                                
        case RANGE: 
        {
            unitID = sp.wIndex >> 8;
            
            switch( unitID )
            {
                /* Clock Source Units */
                case ID_CLKSRC_EXT:
                case ID_CLKSRC_ADAT:
                case ID_CLKSRC_INT:  

                    /* Control Selector (CS) */
                    switch( sp.wValue >> 8 )
                    {
                        case CS_SAM_FREQ_CONTROL: 
                                        
                            /* Currently always return all freqs for all clocks */
                            //switch(unitID) 
                            //{
                              //  case ID_CLKSRC_INT:
                               // case ID_CLKSRC_EXT:

                          {
                            int num_freqs = 0;
                            int i = 2;

                            #if MAX_FREQ >= 44100
                            storeFreq(buffer, i, 44100);
                            num_freqs++;
                            #endif
                            #if MAX_FREQ >= 48000
                            storeFreq(buffer, i, 48000);
                            num_freqs++;
                            #endif
                            #if MAX_FREQ >= 88200
                            storeFreq(buffer, i, 88200);
                            num_freqs++;
                            #endif
                            #if MAX_FREQ >= 96000
                            storeFreq(buffer, i, 96000);
                            num_freqs++;
                            #endif
                            #if MAX_FREQ >= 176400
                            storeFreq(buffer, i, 176400);
                            num_freqs++;
                            #endif
                            #if MAX_FREQ >= 192000
                            storeFreq(buffer, i, 192000);
                            num_freqs++;
                            #endif
                            #if MAX_FREQ >= 352800
                            storeFreq(buffer, i, 352800);
                            num_freqs++;
                            #endif
                            #if MAX_FREQ >= 384000
                            storeFreq(buffer, i, 384000);
                            num_freqs++;
                            #endif
                            storeShort(buffer, 0, num_freqs);
                                 //   break;
                            //}
                    
                            return XUD_DoGetRequest(ep0_out, ep0_in, buffer, i, sp.wLength); 
                          }
                            break;
                     
                        //default:
                          //  XUD_Error_hex("Unknown Control Selector in Clock Source Range Request: ", sp.wValue);
                            // break;
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
                                                 
                    //default:
                      //XUD_Error_hex("Unknown control selector for FU: ", sp.wValue);
                        //        break;

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


            //default:
              //XUD_Error_hex("Unknown Unit ID in Range Request selector for FU: ", sp.wIndex >> 8);
              //break;
                   
            }                
  
            break;
        }

#ifdef MIXER        
        case MEM:   /* Memory Requests (5.2.7.1) */

            unitID = sp.wIndex >> 8;
            
            switch( unitID )
            {
                case ID_MIXER_1:
                    
                    if(sp.bmRequestType.Direction == BM_REQTYPE_DIRECTION_IN) 
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
                                	if(i < NUM_USB_CHAN_IN) 
                                    {
                                        if (!isnull(c_mix_ctl)) 
                                        {
                                            outuint(c_mix_ctl, GET_INPUT_LEVELS);
                                            outuint(c_mix_ctl, (i - NUM_USB_CHAN_IN));
                                            outct(c_mix_ctl, XS1_CT_END);
                                            storeShort(buffer, i*2, (inuint(c_mix_ctl)>>15));
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
                                            outuint(c_mix_ctl, GET_STREAM_LEVELS);
                                            outuint(c_mix_ctl, (i - NUM_USB_CHAN_IN));
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
                                    if (!isnull(c_mix_ctl)) {
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
                    else
                    {
                        /* Host-to-device (SET) */  
                        /* Currently no action for set mem request for any offset */    
                        datalength = XUD_GetBuffer(ep0_out, buffer);
                                                      
                        /* Check for reset/suspend */
                        if(datalength < 0)
                        {
                            return datalength;
                        }

                        /* Send 0 Length as status stage */
                        return XUD_DoSetRequestStatus(ep0_in, 0);
                    
                    }
                    break;
            }

            break;
      
        
        
#endif
    
    }

    /* Didn't deal with request, return 1 */
    return 1;

}

#if defined (AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS==1)
/* Handles the Audio Class 1.0 specific requests */
int AudioClassRequests_1(XUD_ep c_ep0_out, XUD_ep c_ep0_in, SetupPacket &sp, chanend c_audioControl, chanend ?c_mix_ctl, chanend ?c_clk_ctl
#ifdef EP0_THREAD_COMBINED_WITH_SPI
  , chanend c_spi, chanend c_spi_ss
#endif
)
{
    unsigned char buffer[1024];
    int unitID;
    int loop = 1;
    int i_tmp;

    /* Inspect request, NOTE: these are class specific requests */
    switch( sp.bRequest )
    {
        case SET_INTERFACE: 
        {
            return XUD_SetBuffer_ResetPid(c_ep0_in, buffer, 0, PIDn_DATA1);

            break; 
        }
              
        case B_REQ_SET_CUR:
        {
            loop = XUD_GetBuffer(c_ep0_out, buffer);

            /* Inspect for rst/suspend */
            if(loop < 0)
                return loop;
            
            unitID = sp.wIndex >> 8;
  
            if (unitID == FU_USBOUT)
            {
                switch ((sp.wValue>>8) & 0xff)
                {
                    case FU_VOLUME_CONTROL:
                    {  
                      volsOut[ sp.wValue & 0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                        break;
                    }
                    case FU_MUTE_CONTROL:
                    {   
                        mutesOut[ sp.wValue & 0xff ] = buffer[0];
                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
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
                      volsIn[ sp.wValue & 0xff ] = buffer[0] | (((int) (signed char) buffer[1]) << 8);
                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                        break;
                    }
                    case FU_MUTE_CONTROL:
                    {   
                        mutesIn[ sp.wValue & 0xff ] = buffer[0];
                        updateVol( unitID, ( sp.wValue & 0xff ), c_mix_ctl );
                        break;
                    }
                }
            }
            else if (unitID == 0)  // sample freq
            {
                i_tmp = buffer[0] | (buffer [1] << 8) | (buffer[2] << 16);

                
                if(i_tmp != g_curSamFreq)
                {
                    int curSamFreq44100Family;
                    
                    /* Windows Audio Class driver has a nice habbit of sending invalid SF's (e.g. 48001Hz) 
                     * when under stress.  Lets double check it here and ignore if not valid. */
                    g_curSamFreq48000Family = i_tmp % 48000 == 0;
                    curSamFreq44100Family = i_tmp % 44100 == 0;

                    if(g_curSamFreq48000Family || curSamFreq44100Family)
                    {
                        g_curSamFreq = i_tmp;

                        if(g_curSamFreq48000Family)
                        {
                            i_tmp = MCLK_48;
                        }
                        else
                        {
                            i_tmp = MCLK_441;
                        }

                        setG_curSamFreqMultiplier(g_curSamFreq/(i_tmp/512));

                        /* Instruct audio thread to change sample freq */
                        outuint(c_audioControl, SET_SAMPLE_FREQ);
                        outuint(c_audioControl, g_curSamFreq); 

#ifdef EP0_THREAD_COMBINED_WITH_SPI
		                spi(c_spi, c_spi_ss);  /* CodecConfig */
#endif

                        /* Wait for handshake back - i.e. pll locked and clocks okay */
                        chkct(c_audioControl, XS1_CT_END);
                
                
                        /* Allow time for the change - feedback to stabalise */
                        {
                            timer t;
                            unsigned time;
                            t :> time;
                            t when timerafter(time+50000000):> void;
                        }
                    }
                }
            }

            return XUD_SetBuffer_ResetPid(c_ep0_in, buffer, 0, PIDn_DATA1);

            break;
        }
        case B_REQ_GET_CUR:
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
                        break;
                    }
                    case FU_MUTE_CONTROL:
                    {   
                        buffer[0] = mutesOut[ sp.wValue & 0xff ];
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
                        break;
                    }
                    case FU_MUTE_CONTROL:
                    {   
                        buffer[0] = mutesIn[ sp.wValue & 0xff ];
                        break;
                    }
                }                
            }     
            else if(unitID == 0)
            {
                printintln(unitID);   
            }     
            
            loop = XUD_SetBuffer_ResetPid(c_ep0_in,  buffer, sp.wLength, PIDn_DATA1);
            
            if(loop < 0)
                return loop;
            
             /* Status stage (0 length OUT) */
             return XUD_GetBuffer(c_ep0_out,buffer);
             break;
        }
        case B_REQ_GET_MIN:
        {
            buffer[0] = (MIN_MIXER_VOLUME & 0xff);
            buffer[1] = (MIN_MIXER_VOLUME >> 8);
            
            loop = XUD_SetBuffer_ResetPid(c_ep0_in, buffer, sp.wLength, PIDn_DATA1);
            
            if(loop < 0)
                return loop;
            
            // Status stage (0 length OUT)
            return XUD_GetBuffer(c_ep0_out, buffer);
            break;
        }
        case B_REQ_GET_MAX:
        {
            buffer[0] = (MAX_MIXER_VOLUME & 0xff);
            buffer[1] = (MAX_MIXER_VOLUME >> 8);
            
            loop = XUD_SetBuffer_ResetPid(c_ep0_in,  buffer, sp.wLength, PIDn_DATA1);
            
            if(loop < 0)
                return 0;

            // Status stage (0 length OUT)
            return XUD_GetBuffer(c_ep0_out, buffer);
            break;
        }
        case B_REQ_GET_RES:
        {
            buffer[0] = (VOLUME_RES_MIXER & 0xff);
            buffer[1] = (VOLUME_RES_MIXER >> 8);
            loop = XUD_SetBuffer_ResetPid(c_ep0_in,  buffer, sp.wLength, PIDn_DATA1);
            
            if(loop < 0)
                return loop;
            
            // Status stage (0 length OUT)
            return XUD_GetBuffer(c_ep0_out, buffer);
            break;
        }
    }

    return 1;

}
#endif

