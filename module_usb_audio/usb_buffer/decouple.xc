#include <xs1.h>
#include "xc_ptr.h"
#include "devicedefines.h"
#include "interrupt.h"
#include "commands.h"
#include "xud.h"

#ifdef NATIVE_DSD
#include "usbaudio20.h"             /* Defines from the USB Audio 2.0 Specifications */
#endif

#ifdef HID_CONTROLS
#include "user_hid.h"
#endif
#define MAX(x,y) ((x)>(y) ? (x) : (y))

#define HS_PACKET_SIZE ((((MAX_FREQ+7999)/8000))+3)     // Samples per channel
#define FS_PACKET_SIZE  ((((MAX_FREQ_FS+999)/1000))+3)  // Samples per channel

/* TODO use SLOTSIZE to potentially save memory */
#define BUFF_SIZE_OUT MAX(4 * HS_PACKET_SIZE * NUM_USB_CHAN_OUT, 4 * FS_PACKET_SIZE * NUM_USB_CHAN_OUT_FS)
#define BUFF_SIZE_IN  MAX(4 * HS_PACKET_SIZE * NUM_USB_CHAN_IN, 4 * FS_PACKET_SIZE * NUM_USB_CHAN_IN_FS)

/* Maximum USB buffer size (1024 bytes + 1 word to store length) */
#define MAX_USB_AUD_PACKET_SIZE 1028

#define OUT_BUFFER_PREFILL (MAX(NUM_USB_CHAN_OUT_FS*FS_PACKET_SIZE*3+4,NUM_USB_CHAN_OUT*HS_PACKET_SIZE*4+4)*1)
#define IN_BUFFER_PREFILL (MAX(FS_PACKET_SIZE*2+4, HS_PACKET_SIZE*4+4)*2)

/* Volume and mute tables */
#ifndef OUT_VOLUME_IN_MIXER
unsigned int multOut[NUM_USB_CHAN_OUT + 1];
static xc_ptr p_multOut;
#endif
#ifndef IN_VOLUME_IN_MIXER
unsigned int multIn[NUM_USB_CHAN_IN + 1];
static xc_ptr p_multIn;
#endif

/* Number of channels to/from the USB bus - initialised to HS Audio 2.0 */
unsigned g_numUsbChan_Out = NUM_USB_CHAN_OUT;
unsigned g_numUsbChan_In = NUM_USB_CHAN_IN;

/* Note we could improve on this, for one subslot is set to 4 */
#define MAX_DEVICE_AUD_PACKET_SIZE_MULT_HS  ((MAX_FREQ/8000+1)*4)
#define MAX_DEVICE_AUD_PACKET_SIZE_MULT_FS  ((MAX_FREQ_FS/1000+1)*4)

#define MAX_DEVICE_AUD_PACKET_SIZE_HS ((MAX_FREQ/8000+1)*NUM_USB_CHAN_IN*4)
#define MAX_DEVICE_AUD_PACKET_SIZE_FS ((MAX_FREQ_FS/1000+1)*NUM_USB_CHAN_IN_FS*4)

#define MAX_DEVICE_AUD_PACKET_SIZE (MAX(MAX_DEVICE_AUD_PACKET_SIZE_FS, MAX_DEVICE_AUD_PACKET_SIZE_HS))

/* Circular audio buffers */
unsigned outAudioBuff[BUFF_SIZE_OUT + (MAX_USB_AUD_PACKET_SIZE>>2) + 4];
unsigned audioBuffIn[BUFF_SIZE_IN + (MAX_DEVICE_AUD_PACKET_SIZE>>2) + 4];

unsigned inZeroBuff[(MAX_DEVICE_AUD_PACKET_SIZE>>2)+4];

void GetADCCounts(unsigned samFreq, int &min, int &mid, int &max);

/* Globals for EP types */
XUD_ep aud_from_host_usb_ep = 0;
XUD_ep aud_to_host_usb_ep = 0;

/* Shared global audio buffering variables */
unsigned g_aud_from_host_buffer;
unsigned g_aud_to_host_buffer;
unsigned g_aud_to_host_flag = 0;
int buffer_aud_ctl_chan = 0;
unsigned g_aud_from_host_flag = 0;
unsigned g_aud_from_host_info;
unsigned g_freqChange_flag = 0;
unsigned g_freqChange_sampFreq;

/* Global vars for sharing stream format change between buffer and decouple (save a channel) */
unsigned g_formatChange_SubSlot;
unsigned g_formatChange_DataFormat;
unsigned g_formatChange_NumChans;
unsigned g_formatChange_SampRes;

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
int sampsToWrite = DEFAULT_FREQ/8000;  /* HS assumed here. Expect to be junked during a overflow before stream start */
int totalSampsToWrite = DEFAULT_FREQ/8000;
int aud_data_remaining_to_device = 0;

/* Audio over/under flow flags */
unsigned outUnderflow = 1;
unsigned outOverflow = 0;
unsigned inUnderflow = 1;

int aud_req_in_count = 0;
int aud_req_out_count = 0;

unsigned unpackState = 0;
unsigned unpackData = 0;

unsigned packState = 0;
unsigned packData = 0;

/* Default to something sensible but the following are setup at stream start: */
unsigned g_curSubSlot_Out = HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES;
unsigned g_curSubSlot_In  = HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES;

/* Init to something sensible, but expect to be re-set before stream start */
#if (AUDIO_CLASS==2)
int g_maxPacketSize = MAX_DEVICE_AUD_PACKET_SIZE_HS;
#else
int g_maxPacketSize = MAX_DEVICE_AUD_PACKET_SIZE_FS;
#endif

#pragma select handler
#pragma unsafe arrays
void handle_audio_request(chanend c_mix_out)
{
    int outSamps;
    int space_left;

    /* Input word that triggered interrupt and handshake back */
    unsigned underflowSample = inuint(c_mix_out);

#if (NUM_USB_CHAN_OUT == 0)
    outuint(c_mix_out, 0);
#else
    if(outUnderflow)
    {
#pragma xta endpoint "out_underflow"
        /* We're still pre-buffering, send out 0 samps */
        for(int i = 0; i < NUM_USB_CHAN_OUT; i++)
        {
            outuint(c_mix_out, underflowSample);
        }

        /* Calc how many samples left in buffer */
        outSamps = g_aud_from_host_wrptr - g_aud_from_host_rdptr;
        if (outSamps < 0)
        {
            outSamps += BUFF_SIZE_OUT*4;
        }

        /* If we have a decent number of samples, come out of underflow cond */
        if(outSamps >= (OUT_BUFFER_PREFILL))
        {
            outUnderflow = 0;
            outSamps++;
        }
    }
    else
    {
        switch(g_curSubSlot_Out)
        {

            case 2:
#if (STREAM_FORMAT_OUTPUT_SUBSLOT_2_USED == 0)
__builtin_unreachable();
#endif
                /* Buffering not underflow condition send out some samples...*/
                for(int i = 0; i < g_numUsbChan_Out; i++)
                {
#pragma xta endpoint "mixer_request"
                    int sample;
                    int mult;
                    int h;
                    unsigned l;

                    read_short_via_xc_ptr(sample, g_aud_from_host_rdptr);
                    g_aud_from_host_rdptr+=2;
                    sample <<= 16;

#if (OUTPUT_VOLUME_CONTROL == 1) && !defined(OUT_VOLUME_IN_MIXER)
                    asm volatile("ldw %0, %1[%2]":"=r"(mult):"r"(p_multOut),"r"(i));
                    {h, l} = macs(mult, sample, 0, 0);
                    /* Note, in 2 byte subslot mode - ignore lower result of macs */
                    h <<= 3;
                    outuint(c_mix_out, h);
#else
                    outuint(c_mix_out, sample);
#endif
                }
                break;

            case 4:
#if (STREAM_FORMAT_OUTPUT_SUBSLOT_4_USED == 0)
__builtin_unreachable();
#endif
                /* Buffering not underflow condition send out some samples...*/
                for(int i = 0; i < g_numUsbChan_Out; i++)
                {
#pragma xta endpoint "mixer_request"
                    int sample;
                    int mult;
                    int h;
                    unsigned l;

                    read_via_xc_ptr(sample, g_aud_from_host_rdptr);
                    g_aud_from_host_rdptr+=4;

#if (OUTPUT_VOLUME_CONTROL == 1) && !defined(OUT_VOLUME_IN_MIXER)
                    asm volatile("ldw %0, %1[%2]":"=r"(mult):"r"(p_multOut),"r"(i));
                    {h, l} = macs(mult, sample, 0, 0);
                    h <<= 3;
#if (STREAM_FORMAT_OUTPUT_RESOLUTION_32BIT_USED == 1)
                    h |= (l >>29)& 0x7; // Note: This step is not required if we assume sample depth is 24bit (rather than 32bit)
                                        // Note: We need all 32bits for Native DSD
#endif
                    outuint(c_mix_out, h);
#else
                    outuint(c_mix_out, sample);
#endif
                }

                break;

            case 3:
#if (STREAM_FORMAT_OUTPUT_SUBSLOT_3_USED == 0)
__builtin_unreachable();
#endif
                /* Buffering not underflow condition send out some samples...*/
                for(int i = 0; i < g_numUsbChan_Out; i++)
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

#if (OUTPUT_VOLUME_CONTROL == 1) && !defined(OUT_VOLUME_IN_MIXER)
                    asm volatile("ldw %0, %1[%2]":"=r"(mult):"r"(p_multOut),"r"(i));
                    {h, l} = macs(mult, sample, 0, 0);
                    h <<= 3;
                    outuint(c_mix_out, h);
#else
                    outuint(c_mix_out, sample);

#endif
                }
                break;


            default:
                __builtin_unreachable();
                break;

        } /* switch(g_curSubSlot_Out) */

        for(int i = 0; i < NUM_USB_CHAN_OUT - g_numUsbChan_Out; i++)
        {
            outuint(c_mix_out, 0);
        }

        /* 3/4 bytes per sample */
        aud_data_remaining_to_device -= (g_numUsbChan_Out * g_curSubSlot_Out);
    }

#endif

    {
        /* Store samples from mixer into sample buffer */
        switch(g_curSubSlot_In)
        {
            case 2:
#if (STREAM_FORMAT_INPUT_SUBSLOT_2_USED == 0)
__builtin_unreachable();
#endif
                for(int i = 0; i < g_numUsbChan_In; i++)
                {
                    /* Receive sample */
                    int sample = inuint(c_mix_out);
#if (INPUT_VOLUME_CONTROL == 1)
#if !defined(IN_VOLUME_IN_MIXER)
                    /* Apply volume */
                    int mult;
                    int h;
                    unsigned l;
                    asm volatile("ldw %0, %1[%2]":"=r"(mult):"r"(p_multIn),"r"(i));
                    {h, l} = macs(mult, sample, 0, 0);
                    sample = h << 3;

                    /* Note, in 2 byte sub slot - ignore lower bits of macs */
#elif defined(IN_VOLUME_IN_MIXER) && defined(IN_VOLUME_AFTER_MIX)
                    sample = sample << 3;
#endif
#endif
                    write_short_via_xc_ptr(g_aud_to_host_dptr, sample>>16);
                    g_aud_to_host_dptr+=2;
                }
                break;

            case 4:
            {
#if (STREAM_FORMAT_INPUT_SUBSLOT_4_USED == 0)
__builtin_unreachable();
#endif
                unsigned ptr = g_aud_to_host_dptr;

                for(int i = 0; i < g_numUsbChan_In; i++)
                {
                    /* Receive sample */
                    int sample = inuint(c_mix_out);
#if(INPUT_VOLUME_CONTROL == 1)
#if !defined(IN_VOLUME_IN_MIXER)
                    /* Apply volume */
                    int mult;
                    int h;
                    unsigned l;
                    asm volatile("ldw %0, %1[%2]":"=r"(mult):"r"(p_multIn),"r"(i));
                    {h, l} = macs(mult, sample, 0, 0);
                    sample = h << 3;
#if (STREAM_FORMAT_INPUT_RESOLUTION_32BIT_USED == 1)
                    sample |= (l >> 29) & 0x7; // Note, this step is not required if we assume sample depth is 24 (rather than 32)
#endif
#elif defined(IN_VOLUME_IN_MIXER) && defined(IN_VOLUME_AFTER_MIX)
                    sample = sample << 3;
#endif
#endif
                    /* Write into fifo */
                    write_via_xc_ptr(ptr, sample);
                    ptr+=4;
                }

                /* Update global pointer */
                g_aud_to_host_dptr = ptr;

                break;
            }

            case 3:
#if (STREAM_FORMAT_INPUT_SUBSLOT_3_USED == 0)
__builtin_unreachable();
#endif
                for(int i = 0; i < g_numUsbChan_In; i++)
                {
                    /* Receive sample */
                    int sample = inuint(c_mix_out);
#if (INPUT_VOLUME_CONTROL) && !defined(IN_VOLUME_IN_MIXER)
                    /* Apply volume */
                    int mult;
                    int h;
                    unsigned l;
                    asm volatile("ldw %0, %1[%2]":"=r"(mult):"r"(p_multIn),"r"(i));
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
                            packData = (packData >> 8) | ((sample & 0xff00)<<16);
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
                break;

            default:
                __builtin_unreachable();
               break;
        }

        /* Input any remaining channels - past this thread we always operate on max channel count */
        for(int i = 0; i < NUM_USB_CHAN_IN - g_numUsbChan_In; i++)
        {
            inuint(c_mix_out);
        }

        sampsToWrite--;
    }

    {
        /* Finished creating packet - commit it to the FIFO */
        /* Total samps to write could start at 0 (i.e. no MCLK) so need to check for < 0) */
        if (sampsToWrite <= 0)
        {
            int speed;
            packState = 0;

            /* Write last packet length into FIFO */
            unsigned datasize = totalSampsToWrite * g_curSubSlot_In * g_numUsbChan_In;

            write_via_xc_ptr(g_aud_to_host_wrptr, datasize);

            /* Round up to nearest word - note, not needed for slotsize == 4! */
            datasize = (datasize+3) & (~0x3);

            /* Move wr ptr on by old packet length */
            g_aud_to_host_wrptr += 4+datasize;

            /* Do wrap */
            if (g_aud_to_host_wrptr >= aud_to_host_fifo_end)
            {
                g_aud_to_host_wrptr = aud_to_host_fifo_start;
            }

            g_aud_to_host_dptr = g_aud_to_host_wrptr + 4;

            /* Now calculate new packet length...
             * First get feedback val (ideally this would be syncronised)
             * Note, if customer hasn't applied a valid MCLK this could go to 0
             * we need to handle this gracefully */
            asm volatile("ldw   %0, dp[g_speed]" : "=r" (speed) :);

            /* Calc packet size to send back based on our fb */
            speedRem += speed;
            totalSampsToWrite = speedRem >> 16;
            speedRem &= 0xffff;

# if 0
            if (totalSampsToWrite < 0 || totalSampsToWrite * g_curSubSlot_In * g_numUsbChan_In > g_maxPacketSize)
            {
                totalSampsToWrite = 0;
            }
#endif

            /* Calc slots left in fifo */
            space_left = g_aud_to_host_rdptr - g_aud_to_host_wrptr;

            /* Mod and special case */
            if ((space_left <= 0) && (g_aud_to_host_rdptr == aud_to_host_fifo_start))
            {
                space_left = aud_to_host_fifo_end - g_aud_to_host_wrptr;
            }

            //if((space_left > 0) && (space_left < (totalSampsToWrite * g_numUsbChan_In * g_curSubSlot_In + 4)))
            if((space_left < (totalSampsToWrite * g_numUsbChan_In * g_curSubSlot_In + 4)))
            {
                /* In pipe has filled its buffer - we need to overflow
                 * Accept the packet, and throw away the oldest in the buffer */

                /* Keep throwing away packets until buffer is at a nice level.. */
                do
                {
                    unsigned rdPtr;

                    /* Read length of packet in buffer at read pointer */
                    unsigned datalength;

                    GET_SHARED_GLOBAL(rdPtr, g_aud_to_host_rdptr);
                    asm volatile("ldw %0, %1[0]":"=r"(datalength):"r"(rdPtr));

                    /* Round up datalength */
                    datalength = ((datalength+3) & ~0x3) + 4;

                    /* Move read pointer on by length */
                    rdPtr += datalength;
                    if (rdPtr >= aud_to_host_fifo_end)
                    {
                        rdPtr = aud_to_host_fifo_start;
                    }

                    space_left += datalength;
                    SET_SHARED_GLOBAL(g_aud_to_host_rdptr, rdPtr);

                } while(space_left < (BUFF_SIZE_IN*4/2));
            }

            sampsToWrite = totalSampsToWrite;
        }
    }

    if (!outUnderflow && (aud_data_remaining_to_device<(g_curSubSlot_Out * g_numUsbChan_Out)))
    {
        /* Handle any tail - incase a bad driver sent us a datalength not a multiple of chan count */
        if (aud_data_remaining_to_device)
        {
            /* Round up to nearest word */
            aud_data_remaining_to_device +=3 - (unpackState&0x3);
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

/* Mark Endpoint (IN) ready with an appropriately sized zero buffer */
static inline void SetupZerosSendBuffer(XUD_ep aud_to_host_usb_ep, unsigned sampFreq, unsigned slotSize)
{
    int min, mid, max, p;
    GetADCCounts(sampFreq, min, mid, max);

    /* Set IN stream packet size to something sensible. We expect the buffer to
     * over flow and this to be reset */
    SET_SHARED_GLOBAL(sampsToWrite, 0);
    SET_SHARED_GLOBAL(totalSampsToWrite, 0);

    mid *= g_numUsbChan_In * slotSize;

    asm volatile("stw %0, %1[0]"::"r"(mid),"r"(g_aud_to_host_zeros));

    /* Mark EP ready with the zero buffer. Note this will simply update the packet size
    * if it is already ready */

    /* g_aud_to_host_buffer is already set to g_aud_to_host_zeros */

    GET_SHARED_GLOBAL(p, g_aud_to_host_buffer);

    XUD_SetReady_InPtr(aud_to_host_usb_ep, p+4, mid);
}



unsigned char tmpBuffer[1026];

#pragma unsafe arrays
void decouple(chanend c_mix_out
#ifdef CHAN_BUFF_CTRL
    , chanend c_buf_ctrl
#endif
)
{
    unsigned sampFreq = DEFAULT_FREQ;
#if (NUM_USB_CHAN_OUT > 0)
    int aud_from_host_flag=0;
    xc_ptr released_buffer;
#endif
#if (NUM_USB_CHAN_IN > 0)
    int aud_to_host_flag = 0;
#endif

    int t = array_to_xc_ptr(outAudioBuff);

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
    g_aud_to_host_dptr = aud_to_host_fifo_start + 4;

    /* Setup pointer to In stream 0 buffer. Note, length will be innited to 0
     * However, this should be over-written on first stream start (assuming host
       properly sends a SetInterface() before streaming. In any case we will send
       0 length packets, which is reasonable behaviour */
    t = array_to_xc_ptr(inZeroBuff);
    g_aud_to_host_zeros = t;

    /* Init vol mult tables */
#ifndef OUT_VOLUME_IN_MIXER
    for (int i = 0; i < NUM_USB_CHAN_OUT + 1; i++)
    {
        asm volatile("stw %0, %1[%2]"::"r"(MAX_VOL),"r"(p_multOut),"r"(i));
    }
#endif

#ifndef IN_VOLUME_IN_MIXER
    for (int i = 0; i < NUM_USB_CHAN_IN + 1; i++)
    {
        asm volatile("stw %0, %1[%2]"::"r"(MAX_VOL),"r"(p_multIn),"r"(i));
    }
#endif

    set_interrupt_handler(handle_audio_request, 1, c_mix_out, 0);

    /* Wait for usb_buffer() to set up globals for us to use
     * Note: assumed that buffer_aud_ctl_chan is also setup before these globals are !0 */
#if (NUM_USB_CHAN_OUT > 0)
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

#if (NUM_USB_CHAN_IN > 0)
    /* Wait for usb_buffer to set up */
    while(!aud_to_host_flag)
    {
        GET_SHARED_GLOBAL(aud_to_host_flag, g_aud_to_host_flag);
    }

    aud_to_host_flag = 0;
    SET_SHARED_GLOBAL(g_aud_to_host_flag, aud_to_host_flag);

    /* NOTE: IN EP not marked ready at this point - Initial size of zero buffer not known
     * since we don't know the USB bus-speed yet.
     * The host will send a SetAltInterface before streaming which will lead to this core
     * getting a SET_CHANNEL_COUNT_IN. This will setup the EP for the first packet */
#endif

    while(1)
    {
        int tmp;

#ifdef CHAN_BUFF_CTRL
        if(!outOverflow)
        {
            /* Need to keep polling in overflow case */
            inuchar(c_buf_ctrl);
        }
#endif
        {
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
                outct(c_mix_out, SET_SAMPLE_FREQ);
                outuint(c_mix_out, sampFreq);

                inUnderflow = 1;
                SET_SHARED_GLOBAL(g_aud_to_host_rdptr, aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(g_aud_to_host_wrptr, aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(g_aud_to_host_dptr,aud_to_host_fifo_start+4);

                /* Set buffer to send back to zeros buffer */
                SET_SHARED_GLOBAL(g_aud_to_host_buffer, g_aud_to_host_zeros);

                /* Update size of zeros buffer (and sampsToWrite) */
                SetupZerosSendBuffer(aud_to_host_usb_ep, sampFreq, g_curSubSlot_In);

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
                asm volatile("outct res[%0],%1"::"r"(buffer_aud_ctl_chan),"r"(XS1_CT_END));

                ENABLE_INTERRUPTS();

                speedRem = 0;
                continue;
            }
            else if(tmp == SET_STREAM_FORMAT_IN)
            {
                unsigned dataFormat, usbSpeed;

                /* Change in IN channel count */
                DISABLE_INTERRUPTS();
                SET_SHARED_GLOBAL(g_freqChange_flag, 0);

                GET_SHARED_GLOBAL(g_numUsbChan_In, g_formatChange_NumChans);
                GET_SHARED_GLOBAL(g_curSubSlot_In, g_formatChange_SubSlot);
                GET_SHARED_GLOBAL(dataFormat, g_formatChange_DataFormat); /* Not currently used for input stream */

                /* Reset IN buffer state */
                inUnderflow = 1;
                SET_SHARED_GLOBAL(g_aud_to_host_rdptr, aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(g_aud_to_host_wrptr,aud_to_host_fifo_start);
                SET_SHARED_GLOBAL(g_aud_to_host_dptr,aud_to_host_fifo_start+4);

                /* Set buffer back to zeros buffer */
                SET_SHARED_GLOBAL(g_aud_to_host_buffer, g_aud_to_host_zeros);

                /* Update size of zeros buffer (and sampsToWrite) */
                SetupZerosSendBuffer(aud_to_host_usb_ep, sampFreq, g_curSubSlot_In);

                GET_SHARED_GLOBAL(usbSpeed, g_curUsbSpeed);
                if (usbSpeed == XUD_SPEED_HS)
                {
                    g_maxPacketSize = (MAX_DEVICE_AUD_PACKET_SIZE_MULT_HS * g_numUsbChan_In);
                }
                else
                {
                    g_maxPacketSize = (MAX_DEVICE_AUD_PACKET_SIZE_MULT_FS * g_numUsbChan_In);
                }

                SET_SHARED_GLOBAL(g_freqChange, 0);
                asm volatile("outct res[%0],%1"::"r"(buffer_aud_ctl_chan),"r"(XS1_CT_END));

                ENABLE_INTERRUPTS();
            }
            else if(tmp == SET_STREAM_FORMAT_OUT)
            {
                unsigned dataFormat, sampRes;
                unsigned dsdMode = DSD_MODE_OFF;

                /* Change in OUT channel count - note we expect this on every stream start event */
                DISABLE_INTERRUPTS();
                SET_SHARED_GLOBAL(g_freqChange_flag, 0);
                GET_SHARED_GLOBAL(g_numUsbChan_Out, g_formatChange_NumChans);
                GET_SHARED_GLOBAL(g_curSubSlot_Out, g_formatChange_SubSlot);
                GET_SHARED_GLOBAL(dataFormat, g_formatChange_DataFormat);
                GET_SHARED_GLOBAL(sampRes, g_formatChange_SampRes);

                /* Reset OUT buffer state */
                SET_SHARED_GLOBAL(g_aud_from_host_rdptr, aud_from_host_fifo_start);
                SET_SHARED_GLOBAL(g_aud_from_host_wrptr, aud_from_host_fifo_start);

                unpackState = 0;

                outUnderflow = 1;
                if(outOverflow)
                {
                    /* If we were previously in overflow we wont have marked as ready */
                    XUD_SetReady_OutPtr(aud_from_host_usb_ep, aud_from_host_fifo_start+4);
                    outOverflow = 0;
                }

#ifdef NATIVE_DSD
                if(dataFormat == UAC_FORMAT_TYPEI_RAW_DATA)
                {
                    dsdMode = DSD_MODE_NATIVE;
                }
#endif
                /* Wait for the audio code to request samples and respond with command */
                inuint(c_mix_out);
                outct(c_mix_out, SET_STREAM_FORMAT_OUT);
                outuint(c_mix_out, dsdMode);
                outuint(c_mix_out, sampRes);

                /* Wait for handshake back */
                chkct(c_mix_out, XS1_CT_END);
                asm volatile("outct res[%0],%1"::"r"(buffer_aud_ctl_chan),"r"(XS1_CT_END));

                SET_SHARED_GLOBAL(g_freqChange, 0);
                ENABLE_INTERRUPTS();
            }
        }

#if (NUM_USB_CHAN_OUT > 0)
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
            if((datalength >= (g_numUsbChan_Out * g_curSubSlot_Out)) && (released_buffer == aud_from_host_wrptr))
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

#if (NUM_USB_CHAN_IN > 0)
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
                    asm volatile("ldw %0, %1[0]":"=r"(len):"r"(p));
                    XUD_SetReady_InPtr(aud_to_host_usb_ep, p+4, len);
                }
                continue;
            }
        }
#endif /* NUM_USB_CHAN_IN > 0 */
    }
}

