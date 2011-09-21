#include <xs1.h>
#include <print.h>
#include <assert.h>
#include "xc_ptr.h"
#define NO_INLINE_MIDI_SELECT_HANDLER 1
#include "usb_midi.h"
#define NO_INLINE_IAP_SELECT_HANDLER 1
#include "iAP.h"
#include "devicedefines.h"
#include "testct_byref.h"
#include "interrupt.h"
#include "clockcmds.h"
#include "xud.h"
#include "usb.h"

/* This function changes the buffer staged for an IN transaction.
 * **It can only be used if you know that the IN transaction will not occur**
 * Otherwise a race condition can occur.
 *
 */
inline void XUD_Change_ReadyIn_Buffer(XUD_ep e, unsigned bufferPtr, int len)
{
    int chan_array_ptr;
    int xud_chan;
    int my_chan;
    int tail;

    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(e));
    asm ("ldw %0, %1[2]":"=r"(my_chan):"r"(e));

    tail = len & 0x3;
    bufferPtr += (len-tail);
    tail <<= 5;
                                                
    asm ("ldw %0, %1[1]":"=r"(xud_chan):"r"(e));

    len >>= 2;
    len = -len;

    /* Store buffer pointer */
    asm ("stw %0, %1[5]"::"r"(bufferPtr),"r"(e));
                                                                        
    /* Store length */
    asm ("stw %0, %1[3]"::"r"(len),"r"(e));

    /* Mark EP ready with pointer */
    asm ("stw %0, %1[0]"::"r"(xud_chan),"r"(chan_array_ptr));
}


#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define MAX_CLASS_ONE_FREQ 96000
#define MAX_CLASS_ONE_CHAN 2

#define CLASS_TWO_PACKET_SIZE ((((MAX_FREQ+7999)/8000))+3)
#define CLASS_ONE_PACKET_SIZE  ((((MAX_CLASS_ONE_FREQ+999)/1000))+3)

#define BUFF_SIZE_OUT MAX(4 * CLASS_TWO_PACKET_SIZE * NUM_USB_CHAN_OUT, 4 * CLASS_ONE_PACKET_SIZE * MAX_CLASS_ONE_CHAN)
#define BUFF_SIZE_IN  MAX(4 * CLASS_TWO_PACKET_SIZE * NUM_USB_CHAN_IN, 4 * CLASS_ONE_PACKET_SIZE * MAX_CLASS_ONE_CHAN)
#define MAX_USB_AUD_PACKET_SIZE 1028
//#define OUT_BUFFER_PREFILL (2*4*BUFF_SIZE_OUT/3)
//#define OUT_BUFFER_PREFILL MAX(CLASS_ONE_PACKET_SIZE*3+4,CLASS_TWO_PACKET_SIZE*4+4)*2
//#define IN_BUFFER_PREFILL MAX(CLASS_ONE_PACKET_SIZE*3+4,CLASS_TWO_PACKET_SIZE*4+4)*2
#define OUT_BUFFER_PREFILL (MAX(MAX_CLASS_ONE_CHAN*CLASS_ONE_PACKET_SIZE*3+4,NUM_USB_CHAN_OUT*CLASS_TWO_PACKET_SIZE*4+4)*1)
#define IN_BUFFER_PREFILL (MAX(CLASS_ONE_PACKET_SIZE*3+4,CLASS_TWO_PACKET_SIZE*4+4)*2)
//#pragma xta command "add exclusion out_underflow"
//#pragma xta command "add exclusion freq_change"
//#pragma xta command "add exclusion print_err"is_as
//#pragma xta command "add exclusion out_soverflow"
//#pragma xta command "analyse path mixer_request mixer_request"
//#pragma xta command "set required - 5200 ns"             /* 192kHz */

/* Volume and mute tables */ 
#ifndef OUT_VOLUME_IN_MIXER
unsigned int multOut[NUM_USB_CHAN_OUT + 1];
#endif
#ifndef IN_VOLUME_IN_MIXER
unsigned int multIn[NUM_USB_CHAN_IN + 1];
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

unsigned ledVal = 1;
unsigned dir = 0;

void led(chanend ?c_led)
{
  if(dir == 0)
    ledVal <<= 1;
  else 
    ledVal >>= 1;

  if(ledVal == 0b10000000 || ledVal == 1)
    dir = !dir;

  if (!isnull(c_led)) {
    c_led <: ledVal;
  }
}

/* Returns the max and min packet sizes to send back to host for a given sample frequency 
 * See page 13 of USB Audio Device Class Definitions for Audio Data Formats Spec (v2.0) 
 * 
 * Audio samples per frame = INT(sampFreq/frametime); Variation allowed is + 1;
 *
 * For HS frame time = 8 * 1000
 *
 * so n = INT(SampFreq/8000) | INT (SampFreq/8000) + 1
 *
 * In the case where INT(SampFreq/8000) == SampFreq/8000) n may vary between
 *
 * INT(SamFreq/8000) - 1 | INT(SampFreq/8000) | INT (SampFreq/8000) + 1
 *
 * Note: Assumes HS (i.e. 8 frames per 1ms)
 *
 * Examples: 
 * 44100:  min: 5  max: 6
 * 48000:  min: 5  max: 7
 * 96000:  min: 11 max: 13
 * 88200:  min: 11 max: 12
 * 176400: min: 22 max: 23
 * 192000: min: 23 max: 25
 *
 * Note: This function uses the multiple return value feature of XC
 */

void GetADCCounts(unsigned samFreq, int &min, int &mid, int &max);

static inline void swap(xc_ptr &a, xc_ptr &b) 
{
  xc_ptr tmp;
  tmp = a;
  a = b;
  b = tmp;
  return;
}

// shared global midi buffering variables
#ifdef MIDI
unsigned g_midi_from_host_flag = 0;
unsigned g_midi_to_host_flag = 0;
int midi_to_host_usb_ep = 0;
int midi_from_host_usb_ep = 0;
#endif

#ifdef IAP
unsigned g_iap_reset = 0;
unsigned g_iap_from_host_flag = 0;
unsigned g_iap_to_host_flag = 0;
int iap_to_host_usb_ep = 0;
int iap_to_host_int_usb_ep = 0;
int iap_from_host_usb_ep = 0;
#endif

int aud_from_host_usb_ep = 0;
int aud_to_host_usb_ep = 0;
int int_usb_ep = 0;

#ifdef MIDI
unsigned int g_midi_to_host_buffer_A[MAX_USB_MIDI_PACKET_SIZE/4+4];
unsigned int g_midi_to_host_buffer_B[MAX_USB_MIDI_PACKET_SIZE/4+4];
int g_midi_from_host_buffer[MAX_USB_MIDI_PACKET_SIZE/4+4];
#endif

#ifdef IAP
unsigned int g_iap_to_host_buffer_A[MAX_IAP_PACKET_SIZE/4+4];
unsigned int g_iap_to_host_buffer_B[MAX_IAP_PACKET_SIZE/4+4];
int g_iap_from_host_buffer[MAX_IAP_PACKET_SIZE/4+4];
unsigned g_zero_buffer[1];
#endif

// shared global aud buffering variables

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


/* Over/under flow flags */
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
void handle_audio_request(chanend c_mix_out, chanend ?c_led) 
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
                asm("ldw %0, %1[%2]":"=r"(mult):"r"(multIn),"r"(i));          
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
                asm("ldw %0, %1[%2]":"=r"(mult):"r"(multIn),"r"(i));          
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
                asm("ldw %0, %1[%2]":"=r"(mult):"r"(multOut),"r"(i));          
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
            asm("ldw %0, %1[%2]":"=r"(mult):"r"(multOut),"r"(i));          
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
#ifdef DEBUG_LEDS
        else 
        {
            led(c_led);
        }
#endif
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

                asm("ldaw %0, dp[g_intData]":"=r"(x));
                XUD_SetReady_In(int_usb_ep, 0,x,6); 
            }

            break;
        default:
            break;
    }
}


#pragma unsafe arrays
void decouple(chanend c_mix_out,
              chanend ?c_midi, chanend ?c_clk_int
#ifdef IAP
, chanend ?c_iap
#endif
)
{   
    unsigned sampFreq = DEFAULT_FREQ;
    int aud_from_host_flag=0;
    int aud_to_host_flag=0;
    xc_ptr released_buffer;

#ifdef MIDI
    xc_ptr midi_from_host_rdptr;
    xc_ptr midi_from_host_buffer;
    xc_ptr midi_to_host_buffer_being_sent = array_to_xc_ptr(g_midi_to_host_buffer_A);
    xc_ptr midi_to_host_buffer_being_collected = array_to_xc_ptr(g_midi_to_host_buffer_B);
    
    int is_ack;
    unsigned int datum;
    int midi_data_remaining_to_device = 0;
    int midi_data_collected_from_device = 0;
    int midi_waiting_on_send_to_host = 0;
    int midi_to_host_flag = 0;
    int midi_from_host_flag = 0;
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
      asm("stw %0, %1[%2]"::"r"(MAX_VOL),"r"(multOut),"r"(i));
    }
#endif
    
#ifndef IN_VOLUME_IN_MIXER
    for (int i = 0; i < NUM_USB_CHAN_IN + 1; i++)
    {
      asm("stw %0, %1[%2]"::"r"(MAX_VOL),"r"(multIn),"r"(i));
    }
#endif


    { int c=0;
      while(!c) {
        asm("ldw %0, dp[buffer_aud_ctl_chan]":"=r"(c));
      }
    }


    set_interrupt_handler(handle_audio_request, 200, 1, c_mix_out, 0);

#ifdef MIDI
    //asm("ldaw %0, dp[g_midi_to_host_buffer]":"=r"(midi_to_host_buffer));
    asm("ldaw %0, dp[g_midi_from_host_buffer]":"=r"(midi_from_host_buffer));

    // wait for usb_buffer to set up
    while(!midi_from_host_flag) {
      GET_SHARED_GLOBAL(midi_from_host_flag, g_midi_from_host_flag);
    }
    
    midi_from_host_flag = 0;
    SET_SHARED_GLOBAL(g_midi_from_host_flag, midi_from_host_flag);

    // send the current host -> device buffer out of the fifo
    XUD_SetReady(midi_from_host_usb_ep, 1);
#endif

#ifdef IAP
    //asm("ldaw %0, dp[g_iap_to_host_buffer]":"=r"(iap_to_host_buffer));
    asm("ldaw %0, dp[g_iap_from_host_buffer]":"=r"(iap_from_host_buffer));

    // wait for usb_buffer to set up
    while(!iap_from_host_flag) {
      GET_SHARED_GLOBAL(iap_from_host_flag, g_iap_from_host_flag);
    }
    
    iap_from_host_flag = 0;
    SET_SHARED_GLOBAL(g_iap_from_host_flag, iap_from_host_flag);

    // send the current host -> device buffer out of the fifo
    XUD_SetReady(iap_from_host_usb_ep, 1);
#endif

#ifdef OUTPUT
    // wait for usb_buffer to set up
    while(!aud_from_host_flag) {
      GET_SHARED_GLOBAL(aud_from_host_flag, g_aud_from_host_flag);
    }
    
    aud_from_host_flag = 0;
    SET_SHARED_GLOBAL(g_aud_from_host_flag, aud_from_host_flag);

    // send the current host -> device buffer out of the fifo
    SET_SHARED_GLOBAL(g_aud_from_host_buffer, g_aud_from_host_wrptr);
    XUD_SetReady(aud_from_host_usb_ep, 1);
#endif

#ifdef INPUT
    // wait for usb_buffer to set up
    while(!aud_to_host_flag) {
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
        read_via_xc_ptr(len, p);

        XUD_SetReady_In(aud_to_host_usb_ep, PIDn_DATA0, g_aud_to_host_buffer, len); 
    }
#endif

    while(1)
    {
        if (!isnull(c_clk_int)) 
        {
          check_for_interrupt(c_clk_int);
        }
             
        asm("#decouple-default");

        /* Check for freq change or other update */
        {   
            int tmp;
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
                SET_SHARED_GLOBAL(g_aud_to_host_buffer,
                                  g_aud_to_host_zeros);

                /* Check if we have an IN packet ready to go */
                if (aud_in_ready)
                {
                    xc_ptr p;
                    int len;
                    
                    GET_SHARED_GLOBAL(p, g_aud_to_host_buffer);
                    read_via_xc_ptr(len, p);                   
                    
                    /* Update the audio in buffer to send the correct
                     * length back to the host for the new sample rate */
                    XUD_Change_ReadyIn_Buffer(aud_to_host_usb_ep, p+4, len);
                }

                /* Reset OUT buffer state */                
                outOverflow = 0;
                outUnderflow = 1;
                SET_SHARED_GLOBAL(g_aud_from_host_rdptr, aud_from_host_fifo_start);              
                SET_SHARED_GLOBAL(g_aud_from_host_wrptr, aud_from_host_fifo_start);
                SET_SHARED_GLOBAL(aud_data_remaining_to_device, 0);

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
            if ((datalength >= (g_numUsbChanOut * slotSize)) && (released_buffer == g_aud_from_host_wrptr))
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
                XUD_SetReady(aud_from_host_usb_ep, 1);
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
                XUD_SetReady(aud_from_host_usb_ep, 1);
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
                    XUD_SetReady_In(aud_to_host_usb_ep, PIDn_DATA0, p+4, len);
                    aud_in_ready = 1;
                }
                continue;
            }
        }
#endif // INPUT

#ifdef MIDI
        /* Check if buffer() has send MIDI packet to host */
        GET_SHARED_GLOBAL(midi_to_host_flag, g_midi_to_host_flag);          
        if (midi_to_host_flag) 
        {
            /* The buffer has been sent to the host, so we can ack the midi thread */
            SET_SHARED_GLOBAL(g_midi_to_host_flag, 0);
            if (midi_data_collected_from_device != 0) 
            {
                /* We have some more data to send set the amount of data to send */
                write_via_xc_ptr(midi_to_host_buffer_being_collected, midi_data_collected_from_device);

                /* Swap the collecting and sending buffer */
                swap(midi_to_host_buffer_being_collected, midi_to_host_buffer_being_sent);
            
                /* Request to send packet */
                XUD_SetReady_In(midi_to_host_usb_ep, 0, midi_to_host_buffer_being_sent+4, midi_data_collected_from_device);

                /* Mark as waiting for host to poll us */
                midi_waiting_on_send_to_host = 1;                                                                                                                  
                /* Reset the collected data count */
                midi_data_collected_from_device = 0;
            }
            else
            {
                midi_waiting_on_send_to_host = 0;              
            }
            continue;
        }
        else 
        {
            /* Check if host has sent us MIDI OUT data */
            GET_SHARED_GLOBAL(midi_from_host_flag, g_midi_from_host_flag);
            if (midi_from_host_flag)
            {
                /* The buffer() thread has filled up a buffer */
                /* Reset flag */
                SET_SHARED_GLOBAL(g_midi_from_host_flag, 0);
           
                /* Read length from buffer[0] */
                read_via_xc_ptr(midi_data_remaining_to_device, midi_from_host_buffer);
            
                /* Increment read pointer - buffer[0] is length */
                midi_from_host_rdptr = midi_from_host_buffer + 4;
           
                if (midi_data_remaining_to_device) 
                {
                    read_via_xc_ptr(datum, midi_from_host_rdptr);
                    outuint(c_midi, datum);
                    midi_from_host_rdptr += 4;              
                    midi_data_remaining_to_device -= 4;
                }                        
             }
        }
   
        select 
        {   
            /* Received word from MIDI thread - Check for ACK or Data */                 
            case midi_get_ack_or_data(c_midi, is_ack, datum):
                if (is_ack) 
                {
                    /* An ack from the midi/uart thread means it has accepted some data we sent it
                     * we are okay to send another word */
                    if (midi_data_remaining_to_device == 0) 
                    {
                        /* We have read an entire packet - Mark ready to receive another */
                        XUD_SetReady(midi_from_host_usb_ep, 1);              
                    }
                    else 
                    {
                        /* Read another word from the fifo and output it to MIDI thread */
                        read_via_xc_ptr(datum, midi_from_host_rdptr);
                        outuint(c_midi, datum);        
                        midi_from_host_rdptr += 4;              
                        midi_data_remaining_to_device -= 4;
                    }         
                }
                else 
                {
                    /* The midi/uart thread has sent us some data - handshake back */
                    midi_send_ack(c_midi);
                    if (midi_data_collected_from_device < MIDI_USB_BUFFER_TO_HOST_SIZE)
                    {
                        /* There is room in the collecting buffer for the data */
                        xc_ptr p = (midi_to_host_buffer_being_collected + 4) + midi_data_collected_from_device;                                                            
                        // Add data to the buffer
                        write_via_xc_ptr(p, datum);
                        midi_data_collected_from_device += 4;
                    }
                    else 
                    {
                        // Too many events from device - drop 
                    } 
                
                    // If we are not sending data to the host then initiate it
                    if (!midi_waiting_on_send_to_host) 
                    {
                        write_via_xc_ptr(midi_to_host_buffer_being_collected, midi_data_collected_from_device);
 
                        swap(midi_to_host_buffer_being_collected, midi_to_host_buffer_being_sent);
                    
                        // Signal other side to swap
                        XUD_SetReady_In(midi_to_host_usb_ep, 0, midi_to_host_buffer_being_sent+4, midi_data_collected_from_device);
                        midi_data_collected_from_device = 0;
                        midi_waiting_on_send_to_host = 1;                  
                    }
                }          
                break;
            default:
                break;
        }
#endif // MIDI

#ifdef IAP
        GET_SHARED_GLOBAL(iap_reset, g_iap_reset);          
        if (iap_reset) {
           iap_send_reset(c_iap); // What if this happen in the middle of a send/ack?
           iap_reset = 0;
           iap_expecting_length = 1;
           SET_SHARED_GLOBAL(g_iap_reset, iap_reset); // Reset has been signalled
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
                XUD_SetReady_In(iap_to_host_int_usb_ep, 0, zero_buffer, 0); // ZLP to int ep
                XUD_SetReady_In(iap_to_host_usb_ep, 0, iap_to_host_buffer_being_sent+4, iap_data_collected_from_device);

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

                /* Increment read pointer - buffer[0] is length */
                iap_from_host_rdptr = iap_from_host_buffer + 4;

                if (iap_data_remaining_to_device) 
                {
                    read_byte_via_xc_ptr(datum_iap, iap_from_host_rdptr);
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
                        XUD_SetReady(iap_from_host_usb_ep, 1);
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
                           XUD_SetReady_In(iap_to_host_int_usb_ep, 0, zero_buffer, 0);
                           XUD_SetReady_In(iap_to_host_usb_ep, 0, iap_to_host_buffer_being_sent+4, iap_data_collected_from_device);
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

