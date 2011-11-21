
#include <xs1.h>
#include <print.h>

//In this file xud.h is not included since we are interpreting the
//assembly functions GetData/SetData as taking xc_ptrs
//#include "xud.h"

#define XUD_SPEED_HS 2

#include "usb.h"
#include "devicedefines.h"
#include "usb_midi.h"
#include "xc_ptr.h"
#include "clockcmds.h"
#include "xud.h"


//typedef unsigned int XUD_ep;

//int XUD_GetData_NoReq(chanend c, xc_ptr buffer);
//int XUD_SetData_NoReq(chanend c, xc_ptr buffer, unsigned datalength, unsigned startIndex);
XUD_ep XUD_Init_Ep(chanend c_ep);

inline void XUD_SetNotReady(XUD_ep e)
{
  int chan_array_ptr;
  asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(e));
  asm ("stw %0, %1[0]"::"r"(0),"r"(chan_array_ptr));
}

void GetADCCounts(unsigned samFreq, int &min, int &mid, int &max);
#define BUFFER_SIZE_OUT       (1028 >> 2)
#define BUFFER_SIZE_IN        (1028 >> 2)

/* Packet buffers for audio data */

extern unsigned int g_curSamFreqMultiplier;


/* Global var for speed.  Related to feedback. Used by input stream to determine IN packet size */
unsigned g_speed;
unsigned g_freqChange = 0;

/* Interrupt EP data */
unsigned char g_intData[8];

#ifdef MIDI
static inline void swap(xc_ptr &a, xc_ptr &b) 
{
  xc_ptr tmp;
  tmp = a;
  a = b;
  b = tmp;
  return;
}
#endif

unsigned char fb_clocks[16];

//#define FB_TOLERANCE_TEST
#define FB_TOLERANCE 0x100

extern unsigned inZeroBuff[];
extern unsigned g_numUsbChanIn;
/** 
 * Buffers data from audio endpoints 
 * @param   c_aud_out     chanend for audio from xud
 * @param   c_aud_in      chanend for audio to xud
 * @param   c_aud_fb      chanend for feeback to xud
 * @return  void
 */
void buffer(register chanend c_aud_out, register chanend c_aud_in, chanend c_aud_fb, 
            chanend c_midi_from_host, 
            chanend c_midi_to_host, 
            chanend c_int, chanend c_sof, 
            chanend c_aud_ctl,
            in port p_off_mclk
            )
{
  XUD_ep ep_aud_out = XUD_Init_Ep(c_aud_out);
  XUD_ep ep_aud_in = XUD_Init_Ep(c_aud_in);
  XUD_ep ep_aud_fb = XUD_Init_Ep(c_aud_fb);
#ifdef MIDI
  XUD_ep ep_midi_from_host = XUD_Init_Ep(c_midi_from_host);
  XUD_ep ep_midi_to_host = XUD_Init_Ep(c_midi_to_host);
#endif
#if defined(SPDIF_RX) || defined(ADAT_RX)
  XUD_ep ep_int = XUD_Init_Ep(c_int);
#endif
  
    unsigned datalength;
    unsigned tmp;
    unsigned sampleFreq = 0;
    unsigned lastClock;

    unsigned clocks = 0;


    unsigned bufferIn = 1;
    unsigned remnant = 0, cycles;
    unsigned sofCount = 0;
    unsigned freqChange = 0;
    //unsigned expected = (DEFAULT_FREQ/8000)<<16;

#ifdef FB_TOLERANCE_TEST
    unsigned expected_fb = 0;
#endif

    xc_ptr aud_from_host_buffer = 0;

#ifdef MIDI
    xc_ptr midi_from_host_buffer = 0;
    xc_ptr midi_to_host_buffer = 0;
    xc_ptr midi_to_host_waiting_buffer = 0;
#endif

    
    set_thread_fast_mode_on();

#if defined(SPDIF_RX) || defined(ADAT_RX)
    asm("stw %0, dp[int_usb_ep]"::"r"(ep_int));    
#endif
    asm("stw %0, dp[aud_from_host_usb_ep]"::"r"(ep_aud_out));
    asm("stw %0, dp[aud_to_host_usb_ep]"::"r"(ep_aud_in));
    asm("stw %0, dp[buffer_aud_ctl_chan]"::"r"(c_aud_ctl));    

    /* Wait for USB connect then setup our first packet */     
    {
        int min, mid, max;
        int usb_speed = 0;
        int frameTime;

        while(usb_speed == 0)
            asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usb_speed) :);

        GetADCCounts(DEFAULT_FREQ, min, mid, max);
        asm("stw %0, dp[g_speed]"::"r"(mid << 16));
        if (usb_speed == XUD_SPEED_HS) 
          mid*=NUM_USB_CHAN_IN*4;
        else
          mid*=NUM_USB_CHAN_IN*3;
        asm("stw %0, %1[0]"::"r"(mid),"r"(inZeroBuff));

#ifdef FB_TOLERANCE_TEST
        expected_fb = ((DEFAULT_FREQ * 0x2000) / 1000);
#endif
        
    } 

#ifdef MIDI
    // get the two buffers to use for midi device->host
    asm("ldaw %0, dp[g_midi_to_host_buffer_A]":"=r"(midi_to_host_buffer));
    asm("ldaw %0, dp[g_midi_to_host_buffer_B]":"=r"(midi_to_host_waiting_buffer));
    asm("ldaw %0, dp[g_midi_from_host_buffer]":"=r"(midi_from_host_buffer));


    // pass the midi->XUD chanends to decouple so that thread can
    // initialize comm with XUD
    asm("stw %0, dp[midi_to_host_usb_ep]"::"r"(ep_midi_to_host));
    asm("stw %0, dp[midi_from_host_usb_ep]"::"r"(ep_midi_from_host));    
    swap(midi_to_host_buffer, midi_to_host_waiting_buffer);
    SET_SHARED_GLOBAL(g_midi_from_host_flag, 1);    
#endif

#ifdef OUTPUT
    SET_SHARED_GLOBAL(g_aud_from_host_flag, 1);    
#endif

#ifdef INPUT
    SET_SHARED_GLOBAL(g_aud_to_host_flag, 1);    
#endif

    (fb_clocks, unsigned[])[0] = 0;


    {
        int usb_speed;
        int x;
       
        asm("ldaw %0, dp[fb_clocks]":"=r"(x));
        asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usb_speed) :);
        
        if (usb_speed == XUD_SPEED_HS)  
        {                  
                
            XUD_SetReady_In(ep_aud_fb, PIDn_DATA0, x, 4);
                
        }
        else 
        {
        
            XUD_SetReady_In(ep_aud_fb, PIDn_DATA0, x, 3);
        }
    }

    while(1)
    {
        /* Wait for response from XUD and service relevant EP */
        select
        {
#if defined(SPDIF_RX) || defined(ADAT_RX)
            /* Interrupt EP, send back interrupt data.  Note, request made from decouple */
            case inuint_byref(c_int, tmp):
            { 
                int sent_ok = 0;
                /* Start XUD_SetData */
                
                XUD_SetData_Inline(ep_int, c_int);

#if 0
                while (!sent_ok) 
                {
                    outct(c_int, 64);
                    asm("ldw %0, dp[g_intData]":"=r"(tmp));
                    outuint(c_int, tmp);
                    asm("ldw %0, dp[g_intData+4]":"=r"(tmp));
                    outct(c_int, 64);
                    outuint(c_int, tmp);
                    sent_ok = inuint(c_int);
                    /* End XUD_SetData */
                }
#endif

                asm("stw   %0, dp[g_intFlag]" :: "r" (0)  );  
                XUD_SetNotReady(ep_int);
                break;
              }
#endif

            /* Sample Freq our chan count update from ep 0 */     
            case inuint_byref(c_aud_ctl, tmp):
            {
                int min, mid, max;
                int usb_speed;
                int frameTime;
                asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usb_speed) :);

                if(tmp == SET_SAMPLE_FREQ)
                { 
                    sampleFreq = inuint(c_aud_ctl);
             
                    /* Tidy up double buffer, note we can do better than this for 44.1 etc but better 
                     * than sending two packets at old speed! */               
                    if (usb_speed == XUD_SPEED_HS) 
                      frameTime = 8000;
                    else 
                      frameTime = 1000;

                    min = sampleFreq / frameTime;

                    max = min + 1;
                
                    mid = min;
                
                    /* Check for INT(SampFreq/8000) == SampFreq/8000 */    
                    if((sampleFreq % frameTime) == 0)
                    {
                        min -= 1;
                    }
#ifdef FB_TOLERANCE_TEST
                    expected_fb = ((sampleFreq * 0x2000) / frametime);
#endif
                    
                    asm("stw %0, dp[g_speed]"::"r"(mid << 16));

                    if (usb_speed == XUD_SPEED_HS) 
                      mid *= NUM_USB_CHAN_IN*4;
                    else 
                      mid *= NUM_USB_CHAN_IN*3;

                    asm("stw %0, %1[0]"::"r"(mid),"r"(inZeroBuff));
                    
                    /* Reset FB */
                    /* Note, Endpoint 0 will hold off host for a sufficient period to allow out feedback 
                     * to stabilise (i.e. sofCount == 128 to fire) */ 
                    sofCount = 0;
                    clocks = 0;
                    remnant = 0;

                    /* Ideally we want to wait for handshake (and pass back up) here.  But we cannot keep this
                    * thread locked, it must stay responsive to packets/SOFs.  So, set a flag and check for 
                    * handshake elsewhere */
                    /* Pass on sample freq change to decouple */
                    SET_SHARED_GLOBAL(g_freqChange, SET_SAMPLE_FREQ);
                    SET_SHARED_GLOBAL(g_freqChange_sampFreq, sampleFreq);
                    SET_SHARED_GLOBAL(g_freqChange_flag, SET_SAMPLE_FREQ);
                }
                else
                {
                    sampleFreq = inuint(c_aud_ctl);         
                    SET_SHARED_GLOBAL(g_freqChange, tmp);   /* Set command */
                    SET_SHARED_GLOBAL(g_freqChange_sampFreq, sampleFreq); /* Set flag */
                    SET_SHARED_GLOBAL(g_freqChange_flag, tmp);
                }
                
               

                break;
            }

            #define MASK_16_13            (7)                                       // Bits that should not be transmitted as part of feedback.
            #define MASK_16_10            (127) //(63)      /* For Audio 1.0 we use a mask 1 bit longer than expected to avoid Windows LSB isses */

            case inuint_byref(c_sof, tmp):
               
                /* NOTE our feedback will be wrong for a couple of SOF's after a SF change due to 
                 * lastClock being incorrect */ 
                asm("#sof");
                
                /* Get MCLK count */
                asm (" getts %0, res[%1]" : "=r" (tmp) : "r" (p_off_mclk));   
              
                GET_SHARED_GLOBAL(freqChange, g_freqChange);
                if(freqChange == SET_SAMPLE_FREQ)
                {
                    /* Keep getting MCLK counts */
                  lastClock = tmp;
                }
                else
                {
                    unsigned mask = MASK_16_13, usb_speed;

                    asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usb_speed) :);
                    
                    if(usb_speed != XUD_SPEED_HS)
                        mask = MASK_16_10;
                    
                    /* Number of MCLKS this SOF, approx 125 * 24 (3000), sample by sample rate */
                    asm("ldw %0, dp[g_curSamFreqMultiplier]":"=r"(cycles));
                    cycles = ((int)((short)(tmp - lastClock))) * cycles;
                
                    /* Any odd bits (lower than 16.23) have to be kept seperate */
                    remnant += cycles & mask;                                   
                
                    /* Add 16.13 bits into clock count */  
                    clocks += (cycles & ~mask) + (remnant & ~mask);      

                    /* and overflow from odd bits. Remove overflow from odd bits. */
                    remnant &= mask;                                                             
                
                    /* Store MCLK for next time around... */
                    lastClock = tmp;                                                    

                    /* Reset counts based on SOF counting.  Expect 16ms (128 HS SOFs/16 FS SOFS) per feedback poll 
                     * We always could 128 sofs, so 16ms @ HS, 128ms @ FS */   
                    if(sofCount == 128)  
                    {
                        sofCount = 0;
#ifdef FB_TOLERANCE_TEST
                        if (clocks > (expected_fb - FB_TOLERANCE) &&
                            clocks < (expected_fb + FB_TOLERANCE)) 
#endif
                        {
                            int usb_speed;
                            asm("stw %0, dp[g_speed]"::"r"(clocks));   // g_speed = clocks
                            //fb_clocks = clocks;

                            asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usb_speed) :);
        
                            if (usb_speed == XUD_SPEED_HS)  
                            {                     
                                (fb_clocks, unsigned[])[0] = clocks;
                            }
                            else 
                            {
                                (fb_clocks, unsigned[])[0] = clocks>>2;
                            }
                        }
#ifdef FB_TOLERANCE_TEST
                        else {
                        }
#endif
                        clocks = 0;
                    }
			
                    sofCount++;
                }
            break;

#ifdef OUTPUT
            /* Audio HOST -> DEVICE */
            case inuint_byref(c_aud_out, tmp):

                asm("#h->d aud data");
                GET_SHARED_GLOBAL(aud_from_host_buffer, g_aud_from_host_buffer);
              
                // XUD_GetData
                {
                    xc_ptr p = aud_from_host_buffer+4;
                    xc_ptr p0 = p;
                    int tail;
                    while (!testct(c_aud_out)) 
                    {
                        unsigned int datum = inuint(c_aud_out);
                        write_via_xc_ptr(p, datum);
                        p += 4;
                    }  
                    tail = inct(c_aud_out);
                    datalength = p - p0 - 4;
                    switch (tail) 
                    {                  
                        case 10:
                        // the tail is 0 which means 
                        datalength -= 2;
                        break;
                        default:
                        // the tail is 2 which means the input was word aligned      
                        break;
                    }                
                }

                XUD_SetNotReady(ep_aud_out);              
                write_via_xc_ptr(aud_from_host_buffer, datalength);
                /* Sync with audio thread */
                SET_SHARED_GLOBAL(g_aud_from_host_flag, 1);             
                         
                break;
#endif

#ifdef INPUT
            /* DEVICE -> HOST */
            case inuint_byref(c_aud_in, tmp):
            {
                XUD_SetData_Inline(ep_aud_in, c_aud_in);

                XUD_SetNotReady(ep_aud_in);

                /* Inform stream that buffer sent */
                SET_SHARED_GLOBAL(g_aud_to_host_flag, bufferIn+1);             
              }
              break;
                
#endif
                
#ifdef OUTPUT 
            /* Feedback Pipe */
            case inuint_byref(c_aud_fb, tmp):
            {

                int usb_speed;
                int x;

                asm("#aud fb");
                
                XUD_SetData_Inline(ep_aud_fb, c_aud_fb);

                asm("ldaw %0, dp[fb_clocks]":"=r"(x));
                asm("ldw   %0, dp[g_curUsbSpeed]" : "=r" (usb_speed) :);
        
                if (usb_speed == XUD_SPEED_HS)  
                {                     
                    XUD_SetReady_In(ep_aud_fb, PIDn_DATA0, x, 4);
                }
                else 
                {
                    XUD_SetReady_In(ep_aud_fb, PIDn_DATA0, x, 3);
                }
            }
            break;
#endif


#ifdef MIDI
        case inuint_byref(c_midi_from_host, tmp):
            asm("#midi h->d");

            /* Get buffer data from host - MIDI OUT from host always into a single buffer */
            {
                xc_ptr p = midi_from_host_buffer + 4;
                xc_ptr p0 = p;
                xc_ptr p1 = p + MAX_USB_MIDI_PACKET_SIZE;
                while (!testct(c_midi_from_host)) 
                {
                    unsigned int datum = inuint(c_midi_from_host);
                    write_via_xc_ptr(p, datum);
                    p += 4;
                }  
                (void) inct(c_midi_from_host);            
                datalength = p - p0 - 4;            
            }
          
            XUD_SetNotReady(ep_midi_from_host);                     

            write_via_xc_ptr(midi_from_host_buffer, datalength);
                    
            /* release the buffer */
            SET_SHARED_GLOBAL(g_midi_from_host_flag, 1);

            break;
 
        /* MIDI IN to host */                  
        case inuint_byref(c_midi_to_host, tmp): 
            asm("#midi d->h");
            
            // fill in the data
            XUD_SetData_Inline(ep_midi_to_host, c_midi_to_host);

            XUD_SetNotReady(ep_midi_to_host);                     

            // ack the decouple thread to say it has been sent to host  
            SET_SHARED_GLOBAL(g_midi_to_host_flag, 1);

            swap(midi_to_host_buffer, midi_to_host_waiting_buffer);

          break;
#endif
        }

    }

     set_thread_fast_mode_off();

}
