// Copyright 2011-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"
#if XUA_USB_EN
#include <xs1.h>
#include <stdint.h>

#include "xc_ptr.h"
#include "xua_commands.h"
#include "xud.h"
#include "testct_byref.h"

#if XUA_HID_ENABLED
#include "xua_hid_report.h"
#include "user_hid.h"
#include "xua_hid.h"
unsigned char g_hidData[HID_MAX_DATA_BYTES] = {0U};
#endif

void GetADCCounts(unsigned samFreq, int &min, int &mid, int &max);
#define BUFFER_SIZE_OUT       (1028 >> 2)
#define BUFFER_SIZE_IN        (1028 >> 2)

extern unsigned int g_curSamFreqMultiplier;

#ifdef CHAN_BUFF_CTRL
#define SET_SHARED_GLOBAL0(x,y) SET_SHARED_GLOBAL(x,y); outuchar(c_buff_ctrl, 0);
#else
#define SET_SHARED_GLOBAL0(x,y) SET_SHARED_GLOBAL(x,y)
#endif

/* Initialise g_speed now so we get a sensible packet size until we start properly calculating feedback in the SoF case */
/* Without this, zero size input packets fill the input FIFO and it takes a long time to clear out when feedback starts */
/* This can cause a delay to the decouple ISR being serviced pushing our I2S timing. Initialising solves this */
unsigned g_speed = (AUDIO_CLASS == 2) ? (DEFAULT_FREQ/8000) << 16 : (DEFAULT_FREQ/1000) << 16;
unsigned g_freqChange = 0;
unsigned feedbackValid = 0;

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
/* When digital Rx enabled we enable an interrupt EP to inform host about changes in clock validity */
/* Interrupt EP report data */
unsigned char g_intData[8] =
{
    0,    // Class-specific, caused by interface
    1,    // attribute: CUR
    0,    // CN/ MCN
    0,    // CS
    0,    // interface
    0,    // ID of entity causing interrupt - this will get modified;
    0,    // Spare
    0,    // Spare
};

unsigned g_intFlag = 0;
#endif

#if defined (MIDI) || defined(IAP)
static inline void swap(xc_ptr &a, xc_ptr &b)
{
  xc_ptr tmp;
  tmp = a;
  a = b;
  b = tmp;
  return;
}
#endif

#ifdef MIDI
static unsigned int g_midi_to_host_buffer_A[MIDI_USB_BUFFER_TO_HOST_SIZE/4];
static unsigned int g_midi_to_host_buffer_B[MIDI_USB_BUFFER_TO_HOST_SIZE/4];
static unsigned int g_midi_from_host_buffer[MAX_USB_MIDI_PACKET_SIZE/4];
#endif

#ifdef IAP
unsigned char  gc_zero_buffer[4];
#endif

unsigned int fb_clocks[4];

//#define FB_TOLERANCE_TEST
#define FB_TOLERANCE 0x100

void XUA_Buffer(
    register chanend c_aud_out,
#if (NUM_USB_CHAN_IN > 0)
    register chanend c_aud_in,
#endif
#if (NUM_USB_CHAN_IN == 0) || defined (UAC_FORCE_FEEDBACK_EP)
    chanend c_aud_fb,
#endif
#ifdef MIDI
    chanend c_midi_from_host,
    chanend c_midi_to_host,
    chanend c_midi,
#endif
#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN)
    chanend ?c_ep_int,
    chanend ?c_clk_int,
#endif
    chanend c_sof,
    chanend c_aud_ctl,
    in port p_off_mclk
#if (HID_CONTROLS )
    , chanend c_hid
#endif
    , chanend c_aud
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
    , client interface pll_ref_if i_pll_ref
#endif
)
{
#ifdef CHAN_BUFF_CTRL
#warning Using channel to control buffering - this may reduce performance but improve power consumption
    chan c_buff_ctrl;
#endif

    par
    {
        XUA_Buffer_Ep(c_aud_out,          /* USB Audio Out*/
#if (NUM_USB_CHAN_IN > 0)
                c_aud_in,                 /* USB Audio In */
#endif
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
                c_aud_fb,                 /* Audio FB */
#endif
#ifdef MIDI
                c_midi_from_host,         /* MIDI Out */ // 2
                c_midi_to_host,           /* MIDI In */  // 4
                c_midi,
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                /* Audio Interrupt - only used for interrupts on external clock change */
                c_ep_int,
                c_clk_int,
#endif
                c_sof, c_aud_ctl, p_off_mclk
#if XUA_HID_ENABLED
                , c_hid
#endif
#ifdef CHAN_BUFF_CTRL
                , c_buff_ctrl
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
                , i_pll_ref
#endif
            );

        {
            XUA_Buffer_Decouple(c_aud
#ifdef CHAN_BUFF_CTRL
                , c_buff_ctrl
#endif
            );
        }
    }
}

// Allows us to externally modify masterClockFreq
unsafe{volatile unsigned * unsafe masterClockFreq_ptr;}

/**
 * Buffers data from audio endpoints
 * @param   c_aud_out     chanend for audio from xud
 * @param   c_aud_in      chanend for audio to xud
 * @param   c_aud_fb      chanend for feeback to xud
 * @return  void
 */
void XUA_Buffer_Ep(register chanend c_aud_out,
#if (NUM_USB_CHAN_IN > 0)
    register chanend c_aud_in,
#endif
#if (NUM_USB_CHAN_IN == 0) || defined (UAC_FORCE_FEEDBACK_EP)
    chanend c_aud_fb,
#endif
#ifdef MIDI
    chanend c_midi_from_host,
    chanend c_midi_to_host,
    chanend c_midi,
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    chanend ?c_ep_int,
    chanend ?c_clk_int,
#endif
    chanend c_sof,
    chanend c_aud_ctl,
    in port p_off_mclk
#if(HID_CONTROLS)
    , chanend c_hid
#endif
#ifdef CHAN_BUFF_CTRL
    , chanend c_buff_ctrl
#endif
#if XUA_SYNCMODE == XUA_SYNCMODE_SYNC
    , client interface pll_ref_if i_pll_ref
#endif
    )
{
    XUD_ep ep_aud_out = XUD_InitEp(c_aud_out);

#if (NUM_USB_CHAN_IN > 0)
    XUD_ep ep_aud_in = XUD_InitEp(c_aud_in);
#endif

#if (NUM_USB_CHAN_IN == 0) || defined (UAC_FORCE_FEEDBACK_EP)
    XUD_ep ep_aud_fb = XUD_InitEp(c_aud_fb);
#endif

#ifdef MIDI
    XUD_ep ep_midi_from_host = XUD_InitEp(c_midi_from_host);
    XUD_ep ep_midi_to_host = XUD_InitEp(c_midi_to_host);
#endif
#ifdef IAP
    XUD_ep ep_iap_from_host   = XUD_InitEp(c_iap_from_host);
    XUD_ep ep_iap_to_host     = XUD_InitEp(c_iap_to_host);
#ifdef IAP_INT_EP
    XUD_ep ep_iap_to_host_int = XUD_InitEp(c_iap_to_host_int);
#endif
#ifdef IAP_EA_NATIVE_TRANS
    XUD_ep ep_iap_ea_native_out = XUD_InitEp(c_iap_ea_native_out);
    XUD_ep ep_iap_ea_native_in = XUD_InitEp(c_iap_ea_native_in);
#endif
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    XUD_ep ep_int = XUD_InitEp(c_ep_int);
#endif

#if XUA_HID_ENABLED
    XUD_ep ep_hid = XUD_InitEp(c_hid);
#endif
    unsigned u_tmp;
    unsigned sampleFreq = DEFAULT_FREQ;
    unsigned masterClockFreq = DEFAULT_MCLK_FREQ;

#if (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
    unsigned lastClock = 0;
    unsigned freqChange = 0;
#endif
    unsafe{masterClockFreq_ptr = &masterClockFreq;}

    unsigned clocks = 0;
    long long clockcounter = 0;

#if FB_USE_REF_CLOCK
    unsigned long long clock_remainder = 0;        /* The carry term from the 100MHz -> MCLK */
#endif

#if (NUM_USB_CHAN_IN > 0)
    unsigned bufferIn = 1;
#endif
    unsigned sofCount = 0;

    unsigned mod_from_last_time = 0;
#ifdef FB_TOLERANCE_TEST
    unsigned expected_fb = 0;
#endif

#if (NUM_USB_CHAN_OUT > 0)
    xc_ptr aud_from_host_buffer = 0;
#endif

#ifdef MIDI
    xc_ptr midi_from_host_buffer = array_to_xc_ptr(g_midi_from_host_buffer);

    xc_ptr midi_from_host_rdptr;
    xc_ptr midi_to_host_buffer_being_sent = array_to_xc_ptr(g_midi_to_host_buffer_A);
    xc_ptr midi_to_host_buffer_being_collected = array_to_xc_ptr(g_midi_to_host_buffer_B);

    int is_ack;
    unsigned int datum;
    int midi_data_remaining_to_device = 0;
    int midi_data_collected_from_device = 0;
    int midi_waiting_on_send_to_host = 0;
#endif

#ifdef IAP
    xc_ptr iap_from_host_rdptr;
    unsigned char iap_from_host_buffer[IAP_MAX_PACKET_SIZE+4];
    unsigned char iap_to_host_buffer[IAP_MAX_PACKET_SIZE];

    int is_ack_iap;
    int is_reset;
    unsigned int datum_iap;
    int iap_data_remaining_to_device = 0;
    int iap_data_collected_from_device = 0;
    int iap_expected_data_length = 0;
    int iap_draining_chan = 0;

#ifdef IAP_EA_NATIVE_TRANS
    unsigned char iap_ea_native_control_flag;
    unsigned char iap_ea_native_rx_buffer[IAP2_EA_NATIVE_TRANS_MAX_PACKET_SIZE];
    unsigned char iap_ea_native_tx_buffer[IAP2_EA_NATIVE_TRANS_MAX_PACKET_SIZE];
    unsigned iap_ea_native_rx_length = 0;
    unsigned iap_ea_native_tx_length = 0;
    unsigned iap_ea_native_interface_alt_setting = 0;
    unsigned iap_ea_native_control_to_send = 0;
    unsigned iap_ea_native_incoming = 0;

#endif
#endif

    /* Store EP's to globals so that decouple() can access them */
    asm("stw %0, dp[aud_from_host_usb_ep]"::"r"(ep_aud_out));
#if (NUM_USB_CHAN_IN > 0)
    asm("stw %0, dp[aud_to_host_usb_ep]"::"r"(ep_aud_in));
#endif
    asm("stw %0, dp[buffer_aud_ctl_chan]"::"r"(c_aud_ctl));

#ifdef FB_TOLERANCE_TEST
    expected_fb = ((DEFAULT_FREQ * 0x2000) / 1000);
#endif

#if (NUM_USB_CHAN_OUT > 0)
    SET_SHARED_GLOBAL(g_aud_from_host_flag, 1);
#endif

#if (NUM_USB_CHAN_IN > 0)
    SET_SHARED_GLOBAL(g_aud_to_host_flag, 1);
#endif

    fb_clocks[0] = 0;

    /* Mark OUT endpoints ready to receive data from host */
#ifdef MIDI
    XUD_SetReady_OutPtr(ep_midi_from_host, midi_from_host_buffer);
#endif

#ifdef IAP
    XUD_SetReady_Out(ep_iap_from_host, iap_from_host_buffer);

#ifdef IAP_EA_NATIVE_TRANS
    XUD_SetReady_Out(ep_iap_ea_native_out, iap_ea_native_rx_buffer);
#endif
#endif

#if XUA_HID_ENABLED

    while (!hidIsReportDescriptorPrepared())
        ;

    UserHIDInit();
    unsigned hid_ready_flag = 0U;
    unsigned hid_ready_id = 0U;

#endif

#if (AUDIO_CLASS == 1)
#if (NUM_USB_CHAN_IN == 0) || defined (UAC_FORCE_FEEDBACK_EP)
    /* In UAC1 we dont use a stream start event (and we are always FS) so mark FB EP ready now */
    XUD_SetReady_In(ep_aud_fb, (fb_clocks, unsigned char[]), 3);
#endif
#endif

#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
#ifndef LOCAL_CLOCK_INCREMENT
#define LOCAL_CLOCK_INCREMENT       (100000)  /* 500Hz */
#endif
#ifndef LOCAL_CLOCK_MARGIN
#define LOCAL_CLOCK_MARGIN          (1000)
#endif
    timer t_sofCheck;
    unsigned timeLastEdge;
    unsigned timeNextEdge;
    t_sofCheck :> timeLastEdge;
    timeNextEdge + LOCAL_CLOCK_INCREMENT;
    i_pll_ref.toggle();
#endif

    while(1)
    {
        XUD_Result_t result;
        unsigned length;

        /* Wait for response from XUD and service relevant EP */
        select
        {
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
            /* Clocking thread wants to produce an interrupt... */
            case inuint_byref(c_clk_int, u_tmp):
                chkct(c_clk_int, XS1_CT_END);

                /* Check if we have interrupt pending.
                 * Note, this his means we can loose interrupts... */
                if(!g_intFlag)
                {
                    g_intFlag = 1;

                    /* Append Unit ID onto packet */
                    g_intData[5] = u_tmp;

                    XUD_SetReady_In(ep_int, g_intData, 6);
                }
                break;

            /* Interrupt EP data sent, clear flag */
            case XUD_SetData_Select(c_ep_int, ep_int, result):
            {
                g_intFlag = 0;
                break;
            }
#endif
            /* Sample Freq or chan count update from Endpoint 0 core */
            case testct_byref(c_aud_ctl, u_tmp):
            {
                if (u_tmp)
                {
                   // is a control token sent by reboot_device
                   inct(c_aud_ctl);
                   outct(c_aud_ctl, XS1_CT_END);
                   while(1) {};
                }
                else
                {
                    unsigned cmd = inuint(c_aud_ctl);

                    if(cmd == SET_SAMPLE_FREQ)
                    {
                        unsigned receivedSampleFreq = inuint(c_aud_ctl);

#if (MAX_FREQ != MIN_FREQ)
                        /* Don't update things for DFU command.. */
                        if(receivedSampleFreq != AUDIO_STOP_FOR_DFU)
                        {
                            sampleFreq = receivedSampleFreq;
#ifdef FB_TOLERANCE_TEST
                            expected_fb = ((sampleFreq * 0x2000) / frameTime);
#endif
                            /* Reset FB */
                            /* Note, Endpoint 0 will hold off host for a sufficient period to allow our feedback
                             * to stabilise (i.e. sofCount == 128 to fire) */
                            sofCount = 1;
                            clocks = 0;
                            clockcounter = 0;
                            mod_from_last_time = 0;
                            feedbackValid = 0;
#if FB_USE_REF_CLOCK
                            clock_remainder = 0;
#endif

                            /* Set g_speed to something sensible. We expect it to get over-written before stream time */
                            int min, mid, max;
                            GetADCCounts(sampleFreq, min, mid, max);
                            g_speed = mid<<16;

                            if((MCLK_48 % sampleFreq) == 0)
                            {
                                masterClockFreq = MCLK_48;
                            }
                            else
                            {
                                masterClockFreq = MCLK_441;
                            }
                        }
#endif
                        /* Ideally we want to wait for handshake (and pass back up) here.  But we cannot keep this
                        * core locked, it must stay responsive to packets (MIDI etc) and SOFs.  So, set a flag and check for
                        * handshake elsewhere */
                        SET_SHARED_GLOBAL(g_freqChange_sampFreq, receivedSampleFreq);
                    }
#if (AUDIO_CLASS == 2)
                    else if(cmd == SET_STREAM_FORMAT_IN)
                    {
                        unsigned formatChange_DataFormat = inuint(c_aud_ctl);
                        unsigned formatChange_NumChans = inuint(c_aud_ctl);
                        unsigned formatChange_SubSlot = inuint(c_aud_ctl);
                        unsigned formatChange_SampRes = inuint(c_aud_ctl);

                        SET_SHARED_GLOBAL(g_formatChange_NumChans, formatChange_NumChans);
                        SET_SHARED_GLOBAL(g_formatChange_SubSlot, formatChange_SubSlot);
                        SET_SHARED_GLOBAL(g_formatChange_DataFormat, formatChange_DataFormat);
                        SET_SHARED_GLOBAL(g_formatChange_SampRes, formatChange_SampRes);
                    }
                    /* FIXME when FB EP is enabled there is no inital XUD_SetReady */
                    else if (cmd == SET_STREAM_FORMAT_OUT)
                    {

                        XUD_BusSpeed_t busSpeed;
                        unsigned formatChange_DataFormat = inuint(c_aud_ctl);
                        unsigned formatChange_NumChans = inuint(c_aud_ctl);
                        unsigned formatChange_SubSlot = inuint(c_aud_ctl);
                        unsigned formatChange_SampRes = inuint(c_aud_ctl);

                        SET_SHARED_GLOBAL(g_formatChange_NumChans, formatChange_NumChans);
                        SET_SHARED_GLOBAL(g_formatChange_SubSlot, formatChange_SubSlot);
                        SET_SHARED_GLOBAL(g_formatChange_DataFormat, formatChange_DataFormat);
                        SET_SHARED_GLOBAL(g_formatChange_SampRes, formatChange_SampRes);

#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP)
                        /* Host is starting up the output stream. Setup (or potentially resize) feedback packet based on bus-speed
                         * This is only really important on inital start up (when bus-speed
                           was unknown) and when changing bus-speeds */
                        GET_SHARED_GLOBAL(busSpeed, g_curUsbSpeed);

                        if (busSpeed == XUD_SPEED_HS)
                        {
                            XUD_SetReady_In(ep_aud_fb, (fb_clocks, unsigned char[]), 4);
                        }
                        else
                        {
                            XUD_SetReady_In(ep_aud_fb, (fb_clocks, unsigned char[]), 3);
                        }
#endif
                    }
#endif
                    /* Pass on sample freq change to decouple() via global flag (saves a chanend) */
                    /* Note: freqChange flags now used to communicate other commands also */
                    SET_SHARED_GLOBAL0(g_freqChange, cmd);                /* Set command */
                    SET_SHARED_GLOBAL(g_freqChange_flag, cmd);  /* Set Flag */
                }
                break;
            }
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
            case t_sofCheck when timerafter(timeNextEdge) :> void:
                i_pll_ref.toggle();
                timeLastEdge = timeNextEdge;
                timeNextEdge += LOCAL_CLOCK_INCREMENT;
                break;
#endif

            #define MASK_16_13            (7)   /* Bits that should not be transmitted as part of feedback */
            #define MASK_16_10            (127) /* For Audio 1.0 we use a mask 1 bit longer than expected to avoid Windows LSB issues */
                                                /* (previously used 63 instead of 127) */

            /* SOF notification from XUD_Manager() */
            case inuint_byref(c_sof, u_tmp):
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
                /* This really could (should) be done in decouple. However, for a quick demo this is okay
                 * Decouple expects a 16:16 number in fixed point stored in the global g_speed */
                unsigned usbSpeed;
                int framesPerSec;
                GET_SHARED_GLOBAL(usbSpeed, g_curUsbSpeed);
                static int sofCount = 0;

                framesPerSec = (usbSpeed == XUD_SPEED_HS) ? 8000 : 1000;

                clocks =  ((int64_t) sampleFreq << 16) / framesPerSec;

                asm volatile("stw %0, dp[g_speed]"::"r"(clocks));

                sofCount += 1000;
                if (sofCount == framesPerSec)
                {
                    /* Port is accessed via interface to allow flexibilty with location */
                    i_pll_ref.toggle();
                    t_sofCheck :> timeLastEdge;
                    sofCount = 0;
                    timeNextEdge = timeLastEdge + LOCAL_CLOCK_INCREMENT + LOCAL_CLOCK_MARGIN;
                }
#elif (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)

                /* NOTE our feedback will be wrong for a couple of SOF's after a SF change due to
                 * lastClock being incorrect */

#if FB_USE_REF_CLOCK
                /* Get core time i.e. 100MHz clock */
                asm volatile("gettime %0" : "=r"(u_tmp));
#else
                /* Get MCLK count */
                asm volatile(" getts %0, res[%1]" : "=r" (u_tmp) : "r" (p_off_mclk));
#endif
                /* The time we base feedback on will be invalid until we get 2 SOF's */
                /* Additionally whilst the SR is being changed we could get some invalid values due to clocks being changed etc */
                GET_SHARED_GLOBAL(freqChange, g_freqChange);
                if((freqChange == SET_SAMPLE_FREQ) || !feedbackValid)
                {
                     /* Keep getting MCLK counts */
                    lastClock = u_tmp;
                    feedbackValid = 1;
                }
                else
                {
                    unsigned usb_speed;
                    GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

#if FB_USE_REF_CLOCK
                    unsigned long long feedbackMul = 64ULL;

                    if(usb_speed != XUD_SPEED_HS)
                        feedbackMul = 8ULL;  /* TODO Use 4 instead of 8 to avoid windows LSB issues? */

                    /* Number of MCLK ticks in this SOF period (E.g = 125 * 100 = 12500) */
                    int count = u_tmp - lastClock;

                    unsigned long long full_result = count * feedbackMul * sampleFreq;

                    /* This section scales from the 100MHz ref clock to the current MCLK */
                    if (masterClockFreq == MCLK_48)
                    {
                        full_result = (full_result * 768) / 3125;
                        clock_remainder += (full_result * 768) % 3125;
                        if (clock_remainder >= 3125)
                        {
                            clock_remainder -= 3125;
                            full_result++;
                        }
                    }
                    else //MCLK_441
                    {
                        full_result = (full_result * 762) / 3375;
                        clock_remainder += (full_result * 762) % 3375;
                        if (clock_remainder >= 3375)
                        {
                            clock_remainder -= 3375;
                            full_result++;
                        }
                    }

                    clockcounter += full_result;

                    /* Store MCLK for next time around... */
                    lastClock = u_tmp;

                    /* Reset counts based on SOF counting.  Expect 16ms (128 HS SOFs/16 FS SOFS) per feedback poll
                     * We always count 128 SOFs, so 16ms @ HS, 128ms @ FS */
                    if(sofCount == 128)
                    {
                        sofCount = 0;

                        clockcounter += mod_from_last_time;
                        clocks = clockcounter / masterClockFreq;
                        mod_from_last_time = clockcounter % masterClockFreq;

                        if(usb_speed == XUD_SPEED_HS)
                        {
                            clocks <<= 3;
                        }
                        else
                        {
                            clocks <<= 6;
                        }

                        {
                            int usb_speed;
                            asm volatile("stw %0, dp[g_speed]"::"r"(clocks));   // g_speed = clocks

                            GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

                            if (usb_speed == XUD_SPEED_HS)
                            {
                                (fb_clocks, unsigned[])[0] = clocks;
                            }
                            else
                            {
                                (fb_clocks, unsigned[])[0] = clocks >> 2;
                            }
                        }
                        clockcounter = 0;
                    }
#else

                    /* Assuming 48kHz from a 24.576 master clock (0.0407uS period)
                     * MCLK ticks per SOF = 125uS / 0.0407 = 3072 MCLK ticks per SOF.
                     * expected Feedback is 48000/8000 = 6 samples. so 0x60000 in 16:16 format.
                     * Average over 128 SOFs - 128 x 3072 = 0x60000.
                     */

                    unsigned long long feedbackMul = 64ULL;

                    if(usb_speed != XUD_SPEED_HS)
                        feedbackMul = 8ULL;  /* TODO Use 4 instead of 8 to avoid windows LSB issues? */

                    /* Number of MCLK ticks in this SOF period (E.g = 125 * 24.576 = 3072) */
                    int count = (int) ((short)(u_tmp - lastClock));

                    unsigned long long full_result = count * feedbackMul * sampleFreq;

                    clockcounter += full_result;

                    /* Store MCLK for next time around... */
                    lastClock = u_tmp;

                    /* Reset counts based on SOF counting.  Expect 16ms (128 HS SOFs/16 FS SOFS) per feedback poll
                     * We always count 128 SOFs, so 16ms @ HS, 128ms @ FS */
                    if(sofCount == 128)
                    {
                        sofCount = 0;

                        clockcounter += mod_from_last_time;
                        clocks = clockcounter / masterClockFreq;
                        mod_from_last_time = clockcounter % masterClockFreq;

                        if(usb_speed == XUD_SPEED_HS)
                        {
                            clocks <<= 3;
                        }
                        else
                        {
                            clocks <<= 6;
                        }
#ifdef FB_TOLERANCE_TEST
                        if (clocks > (expected_fb - FB_TOLERANCE) &&
                            clocks < (expected_fb + FB_TOLERANCE))
#endif
                        {
                            int usb_speed;
                            asm volatile("stw %0, dp[g_speed]"::"r"(clocks));   // g_speed = clocks

                            GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

                            if (usb_speed == XUD_SPEED_HS)
                            {
                                fb_clocks[0] = clocks;
                            }
                            else
                            {
                                fb_clocks[0] = clocks >> 2;
                            }
                        }
#ifdef FB_TOLERANCE_TEST
                        else
                        {
                        }
#endif
                        clockcounter = 0;
                    }
#endif
                    sofCount++;
                }
#endif
            break;

#if (NUM_USB_CHAN_IN > 0)
            /* Sent audio packet DEVICE -> HOST */
            case XUD_SetData_Select(c_aud_in, ep_aud_in, result):
            {
                /* Inform stream that buffer sent */
                SET_SHARED_GLOBAL0(g_aud_to_host_flag, bufferIn+1);
                break;
            }
#endif

#if (NUM_USB_CHAN_OUT > 0)
#if (NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP) && (XUA_SYNCMODE == XUA_SYNCMODE_ASYNC)
            /* Feedback Pipe */
            case XUD_SetData_Select(c_aud_fb, ep_aud_fb, result):
            {
                XUD_BusSpeed_t busSpeed;

                GET_SHARED_GLOBAL(busSpeed, g_curUsbSpeed);

                if (busSpeed == XUD_SPEED_HS)
                {
                    XUD_SetReady_In(ep_aud_fb, (fb_clocks, unsigned char[]), 4);
                }
                else
                {
                    XUD_SetReady_In(ep_aud_fb, (fb_clocks, unsigned char[]), 3);
                }
                break;
            }
#endif
            /* Received Audio packet HOST -> DEVICE. Datalength written to length */
            case XUD_GetData_Select(c_aud_out, ep_aud_out, length, result):
            {
                GET_SHARED_GLOBAL(aud_from_host_buffer, g_aud_from_host_buffer);

                write_via_xc_ptr(aud_from_host_buffer, length);

                /* Sync with decouple thread */
                SET_SHARED_GLOBAL0(g_aud_from_host_flag, 1);
                break;
             }
#endif

#ifdef MIDI
            case XUD_GetData_Select(c_midi_from_host, ep_midi_from_host, length, result):

                if((result == XUD_RES_OKAY) && (length > 0))
                {
                    /* Get buffer data from host - MIDI OUT from host always into a single buffer */
                    midi_data_remaining_to_device = length;

                    midi_from_host_rdptr = midi_from_host_buffer;

                    if (midi_data_remaining_to_device)
                    {
                        read_via_xc_ptr(datum, midi_from_host_rdptr);
                        outuint(c_midi, datum);
                        midi_from_host_rdptr += 4;
                        midi_data_remaining_to_device -= 4;
                    }
                }
                break;

            /* MIDI IN to host */
            case XUD_SetData_Select(c_midi_to_host, ep_midi_to_host, result):

                /* The buffer has been sent to the host, so we can ack the midi thread */
                if (midi_data_collected_from_device != 0)
                {
                    /* Swap the collecting and sending buffer */
                    swap(midi_to_host_buffer_being_collected, midi_to_host_buffer_being_sent);

                    /* Request to send packet */
                    XUD_SetReady_InPtr(ep_midi_to_host, midi_to_host_buffer_being_sent, midi_data_collected_from_device);

                    /* Mark as waiting for host to poll us */
                    midi_waiting_on_send_to_host = 1;
                    /* Reset the collected data count */
                    midi_data_collected_from_device = 0;
                }
                else
                {
                    midi_waiting_on_send_to_host = 0;
                }
                break;
#endif

#ifdef IAP
            /* IAP OUT from host. Datalength writen to tmp */
            case XUD_GetData_Select(c_iap_from_host, ep_iap_from_host, length, result):

                if((result == XUD_RES_OKAY) && (length > 0))
                {
                    iap_data_remaining_to_device = length;

                    if(iap_data_remaining_to_device)
                    {
                        // Send length first so iAP thread knows how much data to expect
                        // Don't expect ack from this to make it simpler
                        outuint(c_iap, iap_data_remaining_to_device);

                        /* Send out first byte in buffer */
                        datum_iap = iap_from_host_buffer[0];
                        outuint(c_iap, datum_iap);

                        /* Set read ptr to next byte in buffer */
                        iap_from_host_rdptr = 1;
                        iap_data_remaining_to_device -= 1;
                    }
                }
                break;

            /* IAP IN to host */
            case XUD_SetData_Select(c_iap_to_host, ep_iap_to_host, result):

                if(result == XUD_RES_RST)
                {
                    XUD_ResetEndpoint(ep_iap_to_host, null);
#ifdef IAP_INT_EP
                    XUD_ResetEndpoint(ep_iap_to_host_int, null);
#endif
                    iap_send_reset(c_iap);
                    iap_draining_chan = 1; // Drain c_iap until a reset is sent back
                    iap_data_collected_from_device = 0;
                    iap_data_remaining_to_device = -1;
                    iap_expected_data_length = 0;
                    iap_from_host_rdptr = 0;
                }
                else
                {
                    /* Send out an iAP packet to host, ACK last msg from iAP to let it know we can move on..*/
                    iap_send_ack(c_iap);
                }
                break;  /* IAP IN to host */

#ifdef IAP_INT_EP
            case XUD_SetData_Select(c_iap_to_host_int, ep_iap_to_host_int, result):

                /* Do nothing.. */
                /* Note, could get a reset notification here, but deal with it in the case above */
                break;
#endif

#ifdef IAP_EA_NATIVE_TRANS
            /* iAP EA Native Transport OUT from host */
            case XUD_GetData_Select(c_iap_ea_native_out, ep_iap_ea_native_out, iap_ea_native_rx_length, result):
                if ((result == XUD_RES_OKAY) && iap_ea_native_rx_length > 0)
                {
                    // Notify EA Protocol user code we have iOS app data from XUD
                    iAP2_EANativeTransport_writeToChan_start(c_iap_ea_native_data, EA_NATIVE_SEND_DATA);
                }
                break;

            /* iAP EA Native Transport IN to host */
            case XUD_SetData_Select(c_iap_ea_native_in, ep_iap_ea_native_in, result):
                switch (result)
                {
                    case XUD_RES_RST:
                        XUD_ResetEndpoint(ep_iap_ea_native_in, null);
                        // Notify user code of USB reset to allow any state to be cleared
                        iAP2_EANativeTransport_writeToChan_start(c_iap_ea_native_data, EA_NATIVE_SEND_CONTROL);
                        // Set up the control flag to send to EA Protocol user code when it responds
                        iap_ea_native_control_flag = EA_NATIVE_RESET;
                        iap_ea_native_control_to_send = 1;
                        break;

                    case XUD_RES_OKAY: // EA Protocol user data successfully passed to XUD
                        // Notify user code
                        iAP2_EANativeTransport_writeToChan_start(c_iap_ea_native_data, EA_NATIVE_SEND_CONTROL);
                        // Set up the control flag to send to EA Protocol user code when it responds
                        iap_ea_native_control_flag = EA_NATIVE_DATA_SENT;
                        iap_ea_native_control_to_send = 1;
                        break;
                }
                break;
                //::
#endif
#endif

#if XUA_HID_ENABLED
                /* HID Report Data */
            case XUD_SetData_Select(c_hid, ep_hid, result):
                hid_ready_flag = 0U;
                unsigned reportTime;
                timer tmr;
                tmr :> reportTime;
                hidCaptureReportTime(hid_ready_id, reportTime);
                hidCalcNextReportTime(hid_ready_id);
                hidClearChangePending(hid_ready_id);
                break;
#endif

#ifdef MIDI
                /* Received word from MIDI thread - Check for ACK or Data */
            case midi_get_ack_or_data(c_midi, is_ack, datum):
                if (is_ack)
                {
                    /* An ack from the midi/uart thread means it has accepted some data we sent it
                        * we are okay to send another word */
                    if (midi_data_remaining_to_device <= 0)
                    {
                        /* We have read an entire packet - Mark ready to receive another */
                        int reset = XUD_SetReady_OutPtr(ep_midi_from_host, midi_from_host_buffer);

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
                        xc_ptr p = midi_to_host_buffer_being_collected + midi_data_collected_from_device;
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
                        swap(midi_to_host_buffer_being_collected, midi_to_host_buffer_being_sent);

                        // Signal other side to swap
                        XUD_SetReady_InPtr(ep_midi_to_host, midi_to_host_buffer_being_sent, midi_data_collected_from_device);
                        midi_data_collected_from_device = 0;
                        midi_waiting_on_send_to_host = 1;
                    }
                }
                break;
#endif  /* ifdef MIDI */

#ifdef IAP
            /* Received word from iap thread - Check for ACK or Data */
            case iap_get_ack_or_reset_or_data(c_iap, is_ack_iap, is_reset, datum_iap):

                if (iap_draining_chan)
                {
                    /* As we're draining the iAP channel now, ignore ACKs and data */
                    if (is_reset)
                    {
                        // The iAP core has returned a reset token, so we can stop draining the iAP channel now
                        iap_draining_chan = 0;
                    }
                }
                else
                {
                    if (is_ack_iap)
                    {
                        /* An ack from the iap/uart thread means it has accepted some data we sent it
                            * we are okay to send another word */
                        if (iap_data_remaining_to_device == 0)
                        {
                            /* We have read an entire packet - Mark ready to receive another */
                            XUD_SetReady_Out(ep_iap_from_host, iap_from_host_buffer);
                        }
                        else
                        {
                            /* Read another byte from the fifo and output it to iap thread */
                            datum_iap = iap_from_host_buffer[iap_from_host_rdptr];
                            outuint(c_iap, datum_iap);
                            iap_from_host_rdptr += 1;
                            iap_data_remaining_to_device -= 1;
                        }
                    }
                    else if (!is_reset)
                    {
                        if (iap_expected_data_length == 0)
                        {
                            /* Expect a length from iAP core */
                            iap_send_ack(c_iap);
                            iap_expected_data_length = datum_iap;
                        }
                        else
                        {
                            if (iap_data_collected_from_device < IAP_MAX_PACKET_SIZE)
                            {
                                /* There is room in the collecting buffer for the data..  */
                                iap_to_host_buffer[iap_data_collected_from_device] = datum_iap;
                                iap_data_collected_from_device += 1;
                            }
                            else
                            {
                                // Too many events from device - drop
                            }

                            /* Once we have the whole message, sent it to host */
                            /* Note we don't ack the last byte yet... */
                            if (iap_data_collected_from_device == iap_expected_data_length)
                            {
                                XUD_Result_t result1 = XUD_RES_OKAY, result2;
#ifdef IAP_INT_EP
                                result1 = XUD_SetReady_In(ep_iap_to_host_int, gc_zero_buffer, 0);
#endif
                                result2 = XUD_SetReady_In(ep_iap_to_host, iap_to_host_buffer, iap_data_collected_from_device);

                                if((result1 == XUD_RES_RST) || (result2 == XUD_RES_RST))
                                {
#ifdef IAP_INT_EP
                                    XUD_ResetEndpoint(ep_iap_to_host_int, null);
#endif
                                    XUD_ResetEndpoint(ep_iap_to_host, null);
                                    iap_send_reset(c_iap);
                                    iap_draining_chan = 1; // Drain c_iap until a reset is sent back
                                    iap_data_remaining_to_device = -1;
                                    iap_from_host_rdptr = 0;
                                }

                                iap_data_collected_from_device = 0;
                                iap_expected_data_length = 0;
                            }
                            else
                            {
                                /* The iap/uart thread has sent us some data - handshake back */
                                iap_send_ack(c_iap);
                            }
                        }
                    }
                }
                break;

    # if IAP_EA_NATIVE_TRANS
            /* Change of EA Native Transport interface setting */
            case inuint_byref(c_iap_ea_native_ctrl, iap_ea_native_interface_alt_setting):
                /* Handshake */
                outct(c_iap_ea_native_ctrl, XS1_CT_END);

                if (iap_ea_native_interface_alt_setting == 0) // EA Protocol session closed by Apple device
                {
                    // Notify user code of USB reset to allow any state to be cleared
                    iAP2_EANativeTransport_writeToChan_start(c_iap_ea_native_data, EA_NATIVE_SEND_CONTROL);
                    // Set up the control flag to send to EA Protocol user code when it responds
                    iap_ea_native_control_flag = EA_NATIVE_DISCONNECTED;
                    iap_ea_native_control_to_send = 1;
                }
                else if (iap_ea_native_interface_alt_setting == 1) // EA Protocol session opened by Apple device
                {
                    // Notify user code of USB reset to allow any state to be cleared
                    iAP2_EANativeTransport_writeToChan_start(c_iap_ea_native_data, EA_NATIVE_SEND_CONTROL);
                    // Set up the control flag to send to EA Protocol user code when it responds
                    iap_ea_native_control_flag = EA_NATIVE_CONNECTED;
                    iap_ea_native_control_to_send = 1;
                }
                break;

            /* Receive data from the EA Protocol user core */
            case c_iap_ea_native_data :> iap_ea_native_incoming:
                // Check if this is a ready flag or unsolicited data
                switch (iap_ea_native_incoming)
                {
                    case EA_NATIVE_RECEIVER_READY: // EA Protocol user core ready to receive data
                        // Check if we are sending a control flag, or OUT data
                        if (iap_ea_native_control_to_send)
                        {
                            unsigned char ea_control[] = {iap_ea_native_control_flag};
                            iAP2_EANativeTransport_writeToChan_data(c_iap_ea_native_data,
                                                                    ea_control,
                                                                    1);
                            iap_ea_native_control_to_send = 0;
                        }
                        else
                        {
                            iAP2_EANativeTransport_writeToChan_data(c_iap_ea_native_data,
                                                                    iap_ea_native_rx_buffer,
                                                                    iap_ea_native_rx_length);
                            // Mark the OUT EP as ready again now we have sent all the data
                            XUD_SetReady_Out(ep_iap_ea_native_out, iap_ea_native_rx_buffer);
                        }
                        break;

                    case EA_NATIVE_SEND_DATA: // Unsolicited data from user core for IN ep
                        iAP2_EANativeTransport_readFromChan_data(c_iap_ea_native_data,
                                                                 iap_ea_native_tx_buffer,
                                                                 iap_ea_native_tx_length);
                        // Mark the IN EP as ready now we have all the data
                        XUD_SetReady_In(ep_iap_ea_native_in, iap_ea_native_tx_buffer, iap_ea_native_tx_length);
                        break;
                }
                break;
#endif  // if IAP_EA_NATIVE_TRANS

#endif  // ifdef IAP

            default:
#if ( 0 < HID_CONTROLS )
                if (!hid_ready_flag)
                {
                    for (unsigned id = hidIsReportIdInUse(); id < hidGetReportIdLimit(); id++)
                    {
                        if ( hidIsChangePending(id) || !HidIsSetIdleSilenced(id) )
                        {
                            int hidDataLength = (int) UserHIDGetData(id, g_hidData);
                            XUD_SetReady_In(ep_hid, g_hidData, hidDataLength);

                            hid_ready_id = id;
                            hid_ready_flag = 1U;
                            break;
                        }
                    }
                }
#endif
                break;
                //::


        }
    }
}
#endif /* XUA_USB_EN */
