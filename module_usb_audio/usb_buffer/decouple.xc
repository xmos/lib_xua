/**
 * Module:  module_usb_aud_shared
 * Version: 2v3
 * Build:   920238b18f6b0967226369682640e1b063865f02
 * File:    decouple.xc
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2010
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   
#include <xs1.h>
#include "xc_ptr.h"
#define NO_INLINE_MIDI_SELECT_HANDLER 1
#include "usb_midi.h"
#define NO_INLINE_IAP_SELECT_HANDLER 1
#ifdef IAP
#include "iAP.h"
#endif
#include "devicedefines.h"
#include "interrupt.h"
#include "clockcmds.h"
#include "xud.h"
#include "usb.h"
#ifdef HID_CONTROLS
#include "vendor_hid.h"
#endif

#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define MAX_CLASS_ONE_FREQ 96000
#define MAX_CLASS_ONE_CHAN 2

#define CLASS_TWO_PACKET_SIZE ((((MAX_FREQ+7999)/8000))+3)
#define CLASS_ONE_PACKET_SIZE  ((((MAX_CLASS_ONE_FREQ+999)/1000))+3)

#define BUFF_SIZE_OUT MAX(4 * CLASS_TWO_PACKET_SIZE * NUM_USB_CHAN_OUT, 4 * CLASS_ONE_PACKET_SIZE * MAX_CLASS_ONE_CHAN)
#define BUFF_SIZE_IN  MAX(4 * CLASS_TWO_PACKET_SIZE * NUM_USB_CHAN_IN, 4 * CLASS_ONE_PACKET_SIZE * MAX_CLASS_ONE_CHAN)
#define MAX_USB_AUD_PACKET_SIZE 1028
#define OUT_BUFFER_PREFILL (MAX(MAX_CLASS_ONE_CHAN*CLASS_ONE_PACKET_SIZE*3+4,NUM_USB_CHAN_OUT*CLASS_TWO_PACKET_SIZE*4+4)*1)
#define IN_BUFFER_PREFILL (MAX(CLASS_ONE_PACKET_SIZE*3+4,CLASS_TWO_PACKET_SIZE*4+4)*2)

/* Volume and mute tables */ 
#ifndef OUT_VOLUME_IN_MIXER
unsigned int multOut[NUM_USB_CHAN_OUT + 1];
static xc_ptr p_multOut;
#endif
#ifndef IN_VOLUME_IN_MIXER
unsigned int multIn[NUM_USB_CHAN_IN + 1];
static xc_ptr p_multIn;
#endif

/* Number of channels to/from the USB bus */
unsigned g_numUsbChanOut = NUM_USB_CHAN_OUT;
unsigned g_numUsbChanIn = NUM_USB_CHAN_IN;

#define MAX_DEVICE_AUD_PACKET_SIZE_CLASS_TWO ((MAX_FREQ/8000+1)*NUM_USB_CHAN_IN*4)
#define MAX_DEVICE_AUD_PACKET_SIZE_CLASS_ONE (((MAX_CLASS_ONE_FREQ/1000+1)*MAX_CLASS_ONE_CHAN*3)+4)

#define MAX_DEVICE_AUD_PACKET_SIZE (MAX(MAX_DEVICE_AUD_PACKET_SIZE_CLASS_ONE, MAX_DEVICE_AUD_PACKET_SIZE_CLASS_TWO))

/* Circular audio buffers */
unsigned outAudioBuff[BUFF_SIZE_OUT + (MAX_USB_AUD_PACKET_SIZE>>2) + 4];
unsigned audioBuffIn[BUFF_SIZE_IN + (MAX_DEVICE_AUD_PACKET_SIZE>>2) + 4];

unsigned inZeroBuff[(MAX_DEVICE_AUD_PACKET_SIZE>>2)+4];
//
unsigned ledVal = 1;
unsigned dir = 0;

void GetADCCounts(unsigned samFreq, int &min, int &mid, int &max);

#ifdef IAP
static inline void swap(xc_ptr &a, xc_ptr &b) 
{
  xc_ptr tmp;
  tmp = a;
  a = b;
  b = tmp;
  return;
}

unsigned g_iap_reset = 1;
unsigned g_iap_from_host_flag = 0;
unsigned g_iap_to_host_flag = 0;
int iap_to_host_usb_ep = 0;
int iap_to_host_int_usb_ep = 0;
int iap_from_host_usb_ep = 0;
unsigned int g_iap_to_host_buffer_A[MAX_IAP_PACKET_SIZE/4+4];
unsigned int g_iap_to_host_buffer_B[MAX_IAP_PACKET_SIZE/4+4];
int g_iap_from_host_buffer[MAX_IAP_PACKET_SIZE/4+4];
unsigned g_zero_buffer[1];
unsigned char  gc_zero_buffer[4];
#endif

//#ifdef HID_CONTROLS
#if 0
extern in port p_but;
unsigned char g_hidData[16] = {0};
unsigned char g_hidFlag = 0;
unsigned g_ep_hid = 0;
#endif

int aud_from_host_usb_ep = 0;
int aud_to_host_usb_ep = 0;
int int_usb_ep = 0;


/* Shared global audio buffering variables */

unsigned g_aud_from_host_buffer;
unsigned g_aud_to_host_buffer;
unsigned g_aud_to_host_flag = 0;
int buffer_aud_ctl_chan = 0;
unsigned g_aud_from_host_flag = 0;
unsigned g_aud_from_host_info;
unsigned g_freqChange_flag = 0;
unsigned g_freqChange_sampFreq;
int speedRem = 0;

xc_ptr aud_from_host_fifo_start;
xc_ptr aud_from_host_fifo_end;
xc_ptr g_aud_from_host_wrptr;
xc_ptr g_aud_from_host_rdptr; 

xc_ptr aud_to_host_fifo_start;
xc_ptr aud_to_host_fifo_end;
xc_ptr g_aud_to_host_wrptr;
xc_ptr g_aud_to_host_dptr;
xc_ptr g_aud_to_host_rdptr; 
xc_ptr g_aud_to_host_zeros; 
int sampsToWrite = 0;
int totalSampsToWrite = 0;
int aud_data_remaining_to_device = 0;

/* Audio over/under flow flags */
unsigned outUnderflow = 1;
unsigned outOverflow = 0;
unsigned inUnderflow = 1;
unsigned inOverflow = 0;

int aud_req_in_count = 0;
int aud_req_out_count = 0;

unsigned unpackState = 0;
unsigned unpackData = 0;

unsigned packState = 0;
unsigned packData = 0;

#if (AUDIO_CLASS==2)
int slotSize = 4;    /* 4 bytes per ssample for Audio Class 2.0 */
#else
int slotSize = 3;    /* 3 bytes per sample for Audio Class 1.0 */
#endif

#pragma select handler
#pragma unsafe arrays
void handle_audio_request(chanend c_mix_out) 
{
    int outSamps;
    int space_left;
    int usb_speed;

    /* Input word that triggered interrupt and handshake back */
    (void) inuint(c_mix_out);
    outuint(c_mix_out, 0);

    asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usb_speed) :);

    /* slotSize different for Audio Class 1.0/2.0. */
#if defined(AUDIO_CLASS_FALLBACK)
    if (usb_speed == XUD_SPEED_HS)
    {
        slotSize = 4;   /* 4 bytes per sample */
    }
    else
    {
        slotSize = 3;   /* 3 bytes per sample */
    }
#endif
 
    /* If in overflow condition then receive samples and throw away */
    if(inOverflow || sampsToWrite == 0)
    {
#pragma loop unroll
        for(int i = 0; i < NUM_USB_CHAN_IN; i++) 
        {
            (void) inuint(c_mix_out);
        }
        
        /* Calculate how much space left in buffer */
        space_left = g_aud_to_host_rdptr - g_aud_to_host_wrptr;

        if (space_left <= 0)
        {
            space_left += BUFF_SIZE_IN*4;
        }

        /* Check if we can come out of overflow */
        if (space_left > (BUFF_SIZE_IN*4/2)) 
        {
            inOverflow = 0;
        }
    }
    else
    {
        /* Not in overflow, store samples from mixer into sample buffer */
        if (usb_speed == XUD_SPEED_HS)
        {
            unsigned ptr = g_aud_to_host_dptr;

            for(int i = 0; i < g_numUsbChanIn; i++) 
            {
                /* Receive sample */
                int sample = inuint(c_mix_out);
#if !defined(IN_VOLUME_IN_MIXER)
                /* Apply volume */
                int mult;
                int h;
                unsigned l;
                asm("ldw %0, %1[%2]":"=r"(mult):"r"(p_multIn),"r"(i));
                {h, l} = macs(mult, sample, 0, 0);
                sample = h << 3;
#elif defined(IN_VOLUME_IN_MIXER) && defined(IN_VOLUME_AFTER_MIX)
                sample = sample << 3;
#endif
                /* Write into fifo */
                write_via_xc_ptr(ptr, sample);              
                ptr+=4;
            }
            
            /* Update global pointer */
            g_aud_to_host_dptr = ptr;
        }
        else
        {
            for(int i = 0; i < g_numUsbChanIn; i++) 
            {
                /* Receive sample */
                int sample = inuint(c_mix_out);
#ifndef IN_VOLUME_IN_MIXER
                /* Apply volume */
                int mult;
                int h;
                unsigned l;
                asm("ldw %0, %1[%2]":"=r"(mult):"r"(p_multIn),"r"(i));
                {h, l} = macs(mult, sample, 0, 0);
                sample = h << 3;
#endif
                /* Pack 3 byte samples */
                switch (packState&0x3) 
                {
                    case 0:                  
                        packData = sample;
                        break;
                    case 1:
                        packData = packData >> 8 | ((sample & 0xff00)<<16);
                        write_via_xc_ptr(g_aud_to_host_dptr, packData);          
                        g_aud_to_host_dptr+=4;
                        write_via_xc_ptr(g_aud_to_host_dptr, sample>>16);          
                        packData = sample;
                        break;
                    case 2:
                        packData = (packData>>16) | ((sample & 0xffff00) << 8);
                        write_via_xc_ptr(g_aud_to_host_dptr, packData);          
                        g_aud_to_host_dptr+=4;
                        packData = sample;
                        break;
                    case 3:
                        packData = (packData >> 24) | (sample & 0xffffff00);
                        write_via_xc_ptr(g_aud_to_host_dptr, packData);          
                        g_aud_to_host_dptr+=4;                  
                        break;
                }
                packState++;
            }
        }
     
        /* Input any remaining channels - past this thread we always operate on max channel count */
        for(int i = 0; i < NUM_USB_CHAN_IN - g_numUsbChanIn; i++)
        {
            inuint(c_mix_out);
        }

        sampsToWrite--;      
    }

    if(outUnderflow)
    {      
#pragma xta endpoint "out_underflow"
        /* We're still pre-buffering, send out 0 samps */
        for(int i = 0; i < NUM_USB_CHAN_OUT; i++) 
        {
            outuint(c_mix_out, 0);
        }

        /* Calc how many samples left in buffer */
        outSamps = g_aud_from_host_wrptr - g_aud_from_host_rdptr;
        if (outSamps < 0)
        {
            outSamps += BUFF_SIZE_OUT*4;
        }
        
        /* If we have a decent number of samples, come out of underflow cond */
        if (outSamps >= (OUT_BUFFER_PREFILL)) 
        {
          outUnderflow = 0;
        }
    }
    else
    {
        if (usb_speed == XUD_SPEED_HS)
        {
            /* Buffering not underflow condition send out some samples...*/
            for(int i = 0; i < g_numUsbChanOut; i++) 
            {
#pragma xta endpoint "mixer_request"
                int sample;
                int mult;
                int h;
                unsigned l;
                
                read_via_xc_ptr(sample, g_aud_from_host_rdptr);
                g_aud_from_host_rdptr+=4;
                
#ifndef OUT_VOLUME_IN_MIXER
                asm("ldw %0, %1[%2]":"=r"(mult):"r"(p_multOut),"r"(i));
                {h, l} = macs(mult, sample, 0, 0);
                h <<= 3;
                outuint(c_mix_out, h);
#else
                outuint(c_mix_out, sample);
#endif
            }
        }
        else
        {
            /* Buffering not underflow condition send out some samples...*/
            for(int i = 0; i < g_numUsbChanOut; i++) 
            {
#pragma xta endpoint "mixer_request"
                int sample;
                int mult;
                int h;
                unsigned l;

                /* Unpack 3 byte samples */
                switch (unpackState&0x3) 
                {
                    case 0:
                        read_via_xc_ptr(unpackData, g_aud_from_host_rdptr);
                        g_aud_from_host_rdptr+=4;
                        sample = unpackData << 8;                               
                        break;
                    case 1:
                        sample = (unpackData >> 16);
                        read_via_xc_ptr(unpackData, g_aud_from_host_rdptr);
                        g_aud_from_host_rdptr+=4;
                        sample = sample | (unpackData << 16);
                        break;
                    case 2:
                        sample = (unpackData >> 8);
                        read_via_xc_ptr(unpackData, g_aud_from_host_rdptr);         
                        g_aud_from_host_rdptr+=4;
                        sample = sample | (unpackData<< 24);
                        break;
                    case 3:
                        sample = unpackData & 0xffffff00;
                        break;
                 }
                unpackState++;
        
#ifndef OUT_VOLUME_IN_MIXER
            asm("ldw %0, %1[%2]":"=r"(mult):"r"(p_multOut),"r"(i));
            {h, l} = macs(mult, sample, 0, 0);
            h <<= 3;
            outuint(c_mix_out, h);
#else
            outuint(c_mix_out, sample);

#endif
            }
        }

        /* Output remaining channels. Past this point we always operate on MAX chan count */
        for(int i = 0; i < NUM_USB_CHAN_OUT - g_numUsbChanOut; i++)
        {
            outuint(c_mix_out, 0);
        }
        
        /* 3/4 bytes per sample */                    
        aud_data_remaining_to_device -= (g_numUsbChanOut*slotSize);
    }
    
    if (!inOverflow) 
    {
        if (sampsToWrite == 0) 
        {
            int speed;

            if (totalSampsToWrite) 
            {
                if (usb_speed == XUD_SPEED_HS) 
                {
                    g_aud_to_host_wrptr += 4+totalSampsToWrite*4*g_numUsbChanIn;
                }
                else 
                {
                    unsigned int datasize = totalSampsToWrite*3*g_numUsbChanIn;
                    datasize = (datasize+3) & (~0x3); // round up to nearest word
                    g_aud_to_host_wrptr += 4+datasize;                
                }
                if (g_aud_to_host_wrptr >= aud_to_host_fifo_end)
                {
                    g_aud_to_host_wrptr = aud_to_host_fifo_start;
                }
            }

            /* Get feedback val - ideally this would be syncronised */
            asm("ldw   %0, dp[g_speed]" : "=r" (speed) :);
     
            /* Calc packet size to send back based on our fb */ 
            speedRem += speed;
            totalSampsToWrite = speedRem >> 16;
            speedRem &= 0xffff;

            if (usb_speed == XUD_SPEED_HS) 
            {
                if (totalSampsToWrite < 0 || totalSampsToWrite*4*g_numUsbChanIn > (MAX_DEVICE_AUD_PACKET_SIZE_CLASS_TWO)) 
                {
                    totalSampsToWrite = 0;
                }
            }
            else  
            {
                if (totalSampsToWrite < 0 || totalSampsToWrite*3*g_numUsbChanIn > (MAX_DEVICE_AUD_PACKET_SIZE_CLASS_ONE)) 
                {
                    totalSampsToWrite = 0;
                }
            }

            /* Calc slots left in fifo */      
            space_left = g_aud_to_host_rdptr - g_aud_to_host_wrptr;      
       
            /* Mod and special case */
            if (space_left <= 0 && g_aud_to_host_rdptr == aud_to_host_fifo_start)
            {
                space_left = aud_to_host_fifo_end - g_aud_to_host_wrptr;
            }

            if ((space_left <= 0) || (space_left > totalSampsToWrite*g_numUsbChanIn*4+4)) 
            {    
                /* Packet okay, write to fifo */
                if (totalSampsToWrite) 
                {
                    write_via_xc_ptr(g_aud_to_host_wrptr, totalSampsToWrite*slotSize*g_numUsbChanIn);
                    packState = 0;
                    g_aud_to_host_dptr = g_aud_to_host_wrptr + 4;
                }
            }
            else 
            {
                inOverflow = 1;
                totalSampsToWrite = 0;
            }
            sampsToWrite = totalSampsToWrite;                              
        }
    }
  
    if (!outUnderflow && (aud_data_remaining_to_device<(slotSize*g_numUsbChanOut))) 
    {
        /* Handle any tail - incase a bad driver sent us a datalength not a multiple of chan count */
        if (aud_data_remaining_to_device) 
        {
            /* Round up to nearest word */
            aud_data_remaining_to_device +=3;
            aud_data_remaining_to_device &= (~3);

            /* Skip the rest of this malformed packet */
            g_aud_from_host_rdptr += aud_data_remaining_to_device;

            aud_data_remaining_to_device = 0;
        }

        /* Wrap read pointer */
        if (g_aud_from_host_rdptr >= aud_from_host_fifo_end)
        {
            g_aud_from_host_rdptr = aud_from_host_fifo_start;
        }

        outUnderflow = (g_aud_from_host_rdptr == g_aud_from_host_wrptr);
    
        if (!outUnderflow) 
        {   
            read_via_xc_ptr(aud_data_remaining_to_device, g_aud_from_host_rdptr);
 
            
            unpackState = 0;
            
            g_aud_from_host_rdptr+=4;
        }
    }
}


unsigned g_intFlag = 0;

extern unsigned char g_intData[8];

void check_for_interrupt(chanend ?c_clk_int) {
    unsigned tmp;

    select
    {
        /* Clocking thread wants to produce an interrupt... */
        case inuint_byref(c_clk_int, tmp):
            chkct(c_clk_int, XS1_CT_END);

            /* Check if we have interrupt pending */
            /* TODO This means we can loose interrupts */
            if(!g_intFlag)
            {
                int x;

                g_intFlag = 1;

                g_intData[5] = tmp;

                /* Make request to send to XUD endpoint - response handled in usb_buffer */
                //XUD_SetReady(int_usb_ep, 0);

                //asm("ldaw %0, dp[g_intData]":"=r"(x));
                //XUD_SetReady_In(int_usb_ep, g_intData, 6); 
            }

            break;
        default:
            break;
    }
}

unsigned char tmpBuffer[1026]; 

#pragma unsafe arrays
void decouple(chanend c_mix_out,
              //chanend ?c_midi, 
              chanend ?c_clk_int
#ifdef IAP
, chanend ?c_iap
#endif
)
{   
    unsigned sampFreq = DEFAULT_FREQ;
#ifdef OUTPUT 
    int aud_from_host_flag=0;
    xc_ptr released_buffer;
#endif
#ifdef INPUT
    int aud_to_host_flag = 0;
#endif 

#ifdef IAP
    xc_ptr iap_from_host_rdptr;
    xc_ptr iap_from_host_buffer;
    xc_ptr iap_to_host_buffer_being_sent = array_to_xc_ptr(g_iap_to_host_buffer_A);
    xc_ptr iap_to_host_buffer_being_collected = array_to_xc_ptr(g_iap_to_host_buffer_B);
    xc_ptr zero_buffer = array_to_xc_ptr(g_zero_buffer);
    
    int is_ack_iap;
    int is_reset;
    int iap_reset;
    unsigned int datum_iap;
    int iap_data_remaining_to_device = 0;
    int iap_data_collected_from_device = 0;
    int iap_waiting_on_send_to_host = 0;
    int iap_to_host_flag = 0;
    int iap_from_host_flag = 0;
    int iap_expecting_length = 1;
    int iap_expecting_data_length = 0;
#endif

    int t = array_to_xc_ptr(outAudioBuff);
    int aud_in_ready = 0;

#ifndef OUT_VOLUME_IN_MIXER
    p_multOut = array_to_xc_ptr(multOut);
#endif
#ifndef IN_VOLUME_IN_MIXER
    p_multIn = array_to_xc_ptr(multIn);
#endif

    aud_from_host_fifo_start = t;
    aud_from_host_fifo_end = aud_from_host_fifo_start + BUFF_SIZE_OUT*4;
    g_aud_from_host_wrptr = aud_from_host_fifo_start;
    g_aud_from_host_rdptr = aud_from_host_fifo_start; 

    t = array_to_xc_ptr(audioBuffIn);

    aud_to_host_fifo_start = t;
    aud_to_host_fifo_end = aud_to_host_fifo_start + BUFF_SIZE_IN*4;
    g_aud_to_host_wrptr = aud_to_host_fifo_start;
    g_aud_to_host_rdptr = aud_to_host_fifo_start; 

    t = array_to_xc_ptr(inZeroBuff);
    g_aud_to_host_zeros = t;
    
    /* Init interrupt report */
    g_intData[0] = 0;    // Class-specific, caused by interface
    g_intData[1] = 1;    // attribute: CUR
    g_intData[2] = 0;    // CN/ MCN
    g_intData[3] = 0;    // CS
    g_intData[4] = 0;    // interface
    g_intData[5] = 0;    // ID of entity causing interrupt - this will get modified

    /* Init vol mult tables */
#ifndef OUT_VOLUME_IN_MIXER
    for (int i = 0; i < NUM_USB_CHAN_OUT + 1; i++)
    {
      asm("stw %0, %1[%2]"::"r"(MAX_VOL),"r"(p_multOut),"r"(i));
    }
#endif
    
#ifndef IN_VOLUME_IN_MIXER
    for (int i = 0; i < NUM_USB_CHAN_IN + 1; i++)
    {
      asm("stw %0, %1[%2]"::"r"(MAX_VOL),"r"(p_multIn),"r"(i));
    }
#endif


    { int c=0;
      while(!c) {
        asm("ldw %0, dp[buffer_aud_ctl_chan]":"=r"(c));
      }
    }


    set_interrupt_handler(handle_audio_request, 200, 1, c_mix_out, 0);

#ifdef IAP
    //asm("ldaw %0, dp[g_iap_to_host_buffer]":"=r"(iap_to_host_buffer));
    asm("ldaw %0, dp[g_iap_from_host_buffer]":"=r"(iap_from_host_buffer));

    // wait for usb_buffer to set up
    while(!iap_from_host_flag)
    {
        GET_SHARED_GLOBAL(iap_from_host_flag, g_iap_from_host_flag);
    }

    iap_from_host_flag = 0;
    SET_SHARED_GLOBAL(g_iap_from_host_flag, iap_from_host_flag);

    // send the current host -> device buffer out of the fifo
    //XUD_SetReady(iap_from_host_usb_ep, 1);
#endif

#ifdef OUTPUT
    // wait for usb_buffer to set up
    while(!aud_from_host_flag) 
    {
      GET_SHARED_GLOBAL(aud_from_host_flag, g_aud_from_host_flag);
    }
    
    aud_from_host_flag = 0;
    SET_SHARED_GLOBAL(g_aud_from_host_flag, aud_from_host_flag);

    // send the current host -> device buffer out of the fifo
    SET_SHARED_GLOBAL(g_aud_from_host_buffer, g_aud_from_host_wrptr);
    XUD_SetReady_OutPtr(aud_from_host_usb_ep, g_aud_from_host_wrptr+4);
#endif

#ifdef INPUT
    // Wait for usb_buffer to set up
    while(!aud_to_host_flag) 
    {
      GET_SHARED_GLOBAL(aud_to_host_flag, g_aud_to_host_flag);
    }
    
    aud_to_host_flag = 0;
    SET_SHARED_GLOBAL(g_aud_to_host_flag, aud_to_host_flag);

    // send the current host -> device buffer out of the fifo
    SET_SHARED_GLOBAL(g_aud_to_host_buffer, g_aud_to_host_zeros);
    {
        xc_ptr p;
        int len;

        GET_SHARED_GLOBAL(p, g_aud_to_host_buffer);
        read_via_xc_ptr(len, p)
        XUD_SetReady_InPtr(aud_to_host_usb_ep, g_aud_to_host_buffer, len); 
        aud_in_ready = 1;   
 
    }
#endif

    while(1)
    {
        int tmp;
        if (!isnull(c_clk_int)) 
        {
            check_for_interrupt(c_clk_int);
        }
           
        {
#if 0
//#ifdef HID_CONTROLS
            Vendor_ReadHIDButtons(g_hidData);
            
            if(g_hidFlag == 0)
            {    
                XUD_SetReady_In(g_ep_hid, g_hidData, 1);
                g_hidFlag = 1;
            }
#endif
            asm("#decouple-default");

            /* Check for freq change or other update */
       
            GET_SHARED_GLOBAL(tmp, g_freqChange_flag);
            if (tmp == SET_SAMPLE_FREQ) 
            {
                SET_SHARED_GLOBAL(g_freqChange_flag, 0);
                GET_SHARED_GLOBAL(sampFreq, g_freqChange_sampFreq);

                /* Pass on to mixer */
                DISABLE_INTERRUPTS(); 
                inuint(c_mix_out);
                outct(c_mix_out, 9);
                outuint(c_mix_out, sampFreq);

                inOverflow = 0;
                inUnderflow = 1;
                SET_SHARED_GLOBAL(g_aud_to_host_rdptr,
                                  aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(g_aud_to_host_wrptr,
                                  aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(sampsToWrite, 0);
                SET_SHARED_GLOBAL(totalSampsToWrite, 0);
                
                /* Set buffer to send back to zeros buffer */
                SET_SHARED_GLOBAL(g_aud_to_host_buffer,g_aud_to_host_zeros);

                /* Update size of zeros buffer */
                {
                    int min, mid, max, usb_speed;
                    GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);
                    GetADCCounts(sampFreq, min, mid, max);
                    if (usb_speed == XUD_SPEED_HS) 
                        mid*=NUM_USB_CHAN_IN*4;
                    else
                        mid*=NUM_USB_CHAN_IN*3;

                    asm("stw %0, %1[0]"::"r"(mid),"r"(g_aud_to_host_zeros));
                }


                /* Check if we have an IN packet ready to go */
                if (aud_in_ready)
                {
                    xc_ptr p;
                    int len;
                    
                    GET_SHARED_GLOBAL(p, g_aud_to_host_buffer);
                    read_via_xc_ptr(len, p);                   
                    
                    /* Update packet size */
                    XUD_SetReady_InPtr(aud_to_host_usb_ep, p+4, len);
                }

                /* Reset OUT buffer state */                
                outUnderflow = 1;
                SET_SHARED_GLOBAL(g_aud_from_host_rdptr, aud_from_host_fifo_start);              
                SET_SHARED_GLOBAL(g_aud_from_host_wrptr, aud_from_host_fifo_start);
                SET_SHARED_GLOBAL(aud_data_remaining_to_device, 0);

                if(outOverflow)
                {
                    /* If we were previously in overflow we wont have marked as ready */   
                    XUD_SetReady_OutPtr(aud_from_host_usb_ep, aud_from_host_fifo_start+4);
                    outOverflow = 0;
                }


                /* Wait for handshake back and pass back up */
                chkct(c_mix_out, XS1_CT_END);

                SET_SHARED_GLOBAL(g_freqChange, 0);
                asm("outct res[%0],%1"::"r"(buffer_aud_ctl_chan),"r"(XS1_CT_END));
              
                ENABLE_INTERRUPTS();

                speedRem = 0;
                continue;
            }
            else if(tmp == SET_CHAN_COUNT_IN)
            {
                /* Change in IN channel count */
                DISABLE_INTERRUPTS(); 
                SET_SHARED_GLOBAL(g_freqChange_flag, 0);
                GET_SHARED_GLOBAL(g_numUsbChanIn, g_freqChange_sampFreq);  /* Misuse of g_freqChange_sampFreq */
 
                /* Reset IN buffer state */             
                inOverflow = 0;
                inUnderflow = 1;
                SET_SHARED_GLOBAL(g_aud_to_host_rdptr, aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(g_aud_to_host_wrptr,aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(sampsToWrite, 0);
                SET_SHARED_GLOBAL(totalSampsToWrite, 0);
                SET_SHARED_GLOBAL(g_aud_to_host_buffer, g_aud_to_host_zeros);
              
                SET_SHARED_GLOBAL(g_freqChange, 0);
                ENABLE_INTERRUPTS();
            }
        }

#ifdef OUTPUT
        /* Check for OUT data flag from host - set by buffer() */
        GET_SHARED_GLOBAL(aud_from_host_flag, g_aud_from_host_flag);
        if (aud_from_host_flag)
        {
            /* The buffer thread has filled up a buffer */
            int datalength;
            int space_left;
            int aud_from_host_wrptr;
            int aud_from_host_rdptr;
            GET_SHARED_GLOBAL(aud_from_host_wrptr, g_aud_from_host_wrptr);
            GET_SHARED_GLOBAL(aud_from_host_rdptr, g_aud_from_host_rdptr);
                
            SET_SHARED_GLOBAL(g_aud_from_host_flag, 0);
            GET_SHARED_GLOBAL(released_buffer, g_aud_from_host_buffer);
              
            /* Read datalength from buffer */
            read_via_xc_ptr(datalength, released_buffer);

            /* Ignore bad small packets */             
            if ((datalength >= (g_numUsbChanOut * slotSize)) && (released_buffer == aud_from_host_wrptr))
            {
            
                /* Move the write pointer of the fifo on - round up to nearest word */
                aud_from_host_wrptr = aud_from_host_wrptr + ((datalength+3)&~0x3) + 4;

                /* Wrap pointer */
                if (aud_from_host_wrptr >= aud_from_host_fifo_end)
                {
                    aud_from_host_wrptr = aud_from_host_fifo_start;
                }
                SET_SHARED_GLOBAL(g_aud_from_host_wrptr, aud_from_host_wrptr);
            }
            
            /* if we have enough space left then send a new buffer pointer 
             * back to the buffer thread */
            space_left = aud_from_host_rdptr - aud_from_host_wrptr;
  
            /* Mod and special case */
            if(space_left <= 0 && g_aud_from_host_rdptr == aud_from_host_fifo_start)
            {
                space_left = aud_from_host_fifo_end - g_aud_from_host_wrptr;
            }
 
         
            if (space_left <= 0 || space_left >= MAX_USB_AUD_PACKET_SIZE) 
            {   
                SET_SHARED_GLOBAL(g_aud_from_host_buffer, aud_from_host_wrptr);
                XUD_SetReady_OutPtr(aud_from_host_usb_ep, aud_from_host_wrptr+4);
            }
            else 
            {  
                /* Enter OUT over flow state */                         
                outOverflow = 1; 
#ifdef DEBUG_LEDS               
                led(c_led);
#endif
            }              
            continue;
        }
        else if (outOverflow) 
        {
            int space_left;
            int aud_from_host_wrptr;
            int aud_from_host_rdptr;
            GET_SHARED_GLOBAL(aud_from_host_wrptr, g_aud_from_host_wrptr);
            GET_SHARED_GLOBAL(aud_from_host_rdptr, g_aud_from_host_rdptr);
            space_left = aud_from_host_rdptr - aud_from_host_wrptr;
            if (space_left <= 0) 
                space_left += BUFF_SIZE_OUT*4;
            if (space_left >= (BUFF_SIZE_OUT*4/2)) 
            {
                /* Come out of OUT overflow state */
                outOverflow = 0;
                SET_SHARED_GLOBAL(g_aud_from_host_buffer, aud_from_host_wrptr);
                XUD_SetReady_OutPtr(aud_from_host_usb_ep, aud_from_host_wrptr+4);
#ifdef DEBUG_LEDS
                  led(c_led);
#endif
            }
        }
#endif

          
#ifdef INPUT
        { 
            /* Check if buffer() has sent a packet to host - uses shared mem flag to save chanends */
            int tmp;
            GET_SHARED_GLOBAL(tmp, g_aud_to_host_flag);
            //case inuint_byref(c_buf_in, tmp):
            if (tmp) 
            {
                /* Signals that the IN endpoint has sent data from the passed buffer */              
                /* Reset flag */
                SET_SHARED_GLOBAL(g_aud_to_host_flag, 0);              
                aud_in_ready = 0;
              
                if (inUnderflow) 
                {
                    int aud_to_host_wrptr;
                    int aud_to_host_rdptr;
                    int fill_level;
                    GET_SHARED_GLOBAL(aud_to_host_wrptr, g_aud_to_host_wrptr);
                    GET_SHARED_GLOBAL(aud_to_host_rdptr, g_aud_to_host_rdptr);

                    /* Check if we have come out of underflow */
                    fill_level = aud_to_host_wrptr - aud_to_host_rdptr;

                    if (fill_level < 0)
                        fill_level += BUFF_SIZE_IN*4;

                    if (fill_level >= IN_BUFFER_PREFILL) 
                    {                  
                        inUnderflow = 0;
                        SET_SHARED_GLOBAL(g_aud_to_host_buffer, aud_to_host_rdptr);
                    }
                    else 
                    {
                        SET_SHARED_GLOBAL(g_aud_to_host_buffer, g_aud_to_host_zeros);                  
                    }

                } 
                else 
                {
                    /* Not in IN underflow state */
                    int datalength;
                    int aud_to_host_wrptr;
                    int aud_to_host_rdptr;
                    GET_SHARED_GLOBAL(aud_to_host_wrptr, g_aud_to_host_wrptr);
                    GET_SHARED_GLOBAL(aud_to_host_rdptr, g_aud_to_host_rdptr);

                    /* Read datalength and round to nearest word */
                    read_via_xc_ptr(datalength, aud_to_host_rdptr);
                    aud_to_host_rdptr = aud_to_host_rdptr + ((datalength+3)&~0x3) + 4;
                    if (aud_to_host_rdptr >= aud_to_host_fifo_end)
                    {
                        aud_to_host_rdptr = aud_to_host_fifo_start;
                    }
                    SET_SHARED_GLOBAL(g_aud_to_host_rdptr, aud_to_host_rdptr);

                    /* Check for read pointer hitting write pointer - underflow */
                    if (aud_to_host_rdptr != aud_to_host_wrptr) 
                    {
                        SET_SHARED_GLOBAL(g_aud_to_host_buffer, aud_to_host_rdptr);
                    }
                    else
                    {
                        inUnderflow = 1;
                        SET_SHARED_GLOBAL(g_aud_to_host_buffer, g_aud_to_host_zeros);
                  
                    }
                }  
 
                /* Request to send packet */          
                {
                    int p, len; 
                    GET_SHARED_GLOBAL(p, g_aud_to_host_buffer);
                    asm("ldw %0, %1[0]":"=r"(len):"r"(p));
                    XUD_SetReady_InPtr(aud_to_host_usb_ep, p+4, len);
                    aud_in_ready = 1;
                }
                continue;
            }
        }
#endif // INPUT


//#if 0
#ifdef IAP
        GET_SHARED_GLOBAL(iap_reset, g_iap_reset);          
        if (iap_reset) 
        {
           iap_send_reset(c_iap); // What if this happen in the middle of a send/ack?
           iap_reset = 0;
           iap_expecting_length = 1;
           SET_SHARED_GLOBAL(g_iap_reset, iap_reset); // Reset has been signalled
           iap_waiting_on_send_to_host = 0;
           iap_data_collected_from_device = 0;
           SET_SHARED_GLOBAL(g_iap_to_host_flag, 0);
        }
        // Need to handle sending ZLP on iap_to_host_usb_ep_int to signal iOS device to collect from bulk endpoint.
        /* Check if buffer() has send IAP packet to host */
        GET_SHARED_GLOBAL(iap_to_host_flag, g_iap_to_host_flag);          
        if (iap_to_host_flag) 
        {
            /* The buffer has been sent to the host, so we can ack the iap thread */
            SET_SHARED_GLOBAL(g_iap_to_host_flag, 0);
            if (iap_data_collected_from_device != 0) 
            {
                /* We have some more data to send set the amount of data to send */
                write_via_xc_ptr(iap_to_host_buffer_being_collected, iap_data_collected_from_device);

                /* Swap the collecting and sending buffer */
                swap(iap_to_host_buffer_being_collected, iap_to_host_buffer_being_sent);
            
                /* Request to send packet */
                XUD_SetReady_In(iap_to_host_int_usb_ep, gc_zero_buffer, 0); // ZLP to int ep
                XUD_SetReady_InPtr(iap_to_host_usb_ep, iap_to_host_buffer_being_sent+4, iap_data_collected_from_device);

                /* Mark as waiting for host to poll us */
                iap_waiting_on_send_to_host = 1;                                                                                                                  
                /* Reset the collected data count */
                iap_data_collected_from_device = 0;
            }
            else
            {
                iap_waiting_on_send_to_host = 0;              
            }
            continue;
        }
        else 
        {
            /* Check if host has sent us iap OUT data */
            GET_SHARED_GLOBAL(iap_from_host_flag, g_iap_from_host_flag);
            if (iap_from_host_flag)
            {
                /* The buffer() thread has filled up a buffer */
                /* Reset flag */
                SET_SHARED_GLOBAL(g_iap_from_host_flag, 0);

                /* Read length from buffer[0] */
                read_via_xc_ptr(iap_data_remaining_to_device, iap_from_host_buffer);

                // Send length first so iAP thread knows how much data to expect
                // Don't expect ack from this to make it simpler
                outuint(c_iap, iap_data_remaining_to_device);

                //printintln(iap_data_remaining_to_device);

                /* Increment read pointer - buffer[0] is length */
                iap_from_host_rdptr = iap_from_host_buffer + 4;

                if (iap_data_remaining_to_device) 
                {
                    read_byte_via_xc_ptr(datum_iap, iap_from_host_rdptr);
                    //printintln(datum_iap);
                    outuint(c_iap, datum_iap);
                    iap_from_host_rdptr += 1;
                    iap_data_remaining_to_device -= 1;
                }
             }
        }

        select 
        {
            /* Received word from iap thread - Check for ACK or Data */
            case iap_get_ack_or_reset_or_data(c_iap, is_ack_iap, is_reset, datum_iap):

                if (is_ack_iap) 
                {
                    /* An ack from the iap/uart thread means it has accepted some data we sent it
                     * we are okay to send another word */
                    if (iap_data_remaining_to_device == 0) 
                    {
                        /* We have read an entire packet - Mark ready to receive another */
                        XUD_SetReady_OutPtr(iap_from_host_usb_ep, iap_from_host_buffer+4);
                    }
                    else 
                    {
                        /* Read another word from the fifo and output it to iap thread */
                        read_byte_via_xc_ptr(datum_iap, iap_from_host_rdptr);
                        outuint(c_iap, datum_iap);
                        iap_from_host_rdptr += 1;
                        iap_data_remaining_to_device -= 1;
                    }
                }
                else
                {
                    /* The iap/uart thread has sent us some data - handshake back */
                    iap_send_ack(c_iap);
                    if (iap_expecting_length) {
                       iap_expecting_data_length = datum_iap;
                       iap_expecting_length = 0;
                    } else {
                       if (iap_data_collected_from_device < IAP_USB_BUFFER_TO_HOST_SIZE)
                       {
                           /* There is room in the collecting buffer for the data */
                           xc_ptr p = (iap_to_host_buffer_being_collected + 4) + iap_data_collected_from_device;
                           // Add data to the buffer
                           write_byte_via_xc_ptr(p, datum_iap);
                           iap_data_collected_from_device += 1;
                       }
                       else 
                       {
                           // Too many events from device - drop 
                       } 

                       // If we are not sending data to the host then initiate it (ONLY IF GOT WHOLE MESSAGE)
                       if (!iap_waiting_on_send_to_host && (iap_data_collected_from_device == iap_expecting_data_length)) 
                       {
                           // Set first element of buffer to length i.e. iap_to_host_buffer_being_collected[0] = iap_data_collected_from_device;
                           write_via_xc_ptr(iap_to_host_buffer_being_collected, iap_data_collected_from_device);

                           swap(iap_to_host_buffer_being_collected, iap_to_host_buffer_being_sent);

                           // Signal other side to swap
                           XUD_SetReady_In(iap_to_host_int_usb_ep, gc_zero_buffer, 0);
                           //printintln(iap_data_collected_from_device);
                           XUD_SetReady_InPtr(iap_to_host_usb_ep, iap_to_host_buffer_being_sent+4, iap_data_collected_from_device);
                           iap_data_collected_from_device = 0;
                           iap_waiting_on_send_to_host = 1;
                           iap_expecting_length = 1;
                       }
                    }
                }
                break;
            default:
                break;
        }
#endif // IAP
    }
}

