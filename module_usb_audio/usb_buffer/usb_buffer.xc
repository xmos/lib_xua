
#include <xs1.h>
#include <print.h>

#include "usb.h"
#include "devicedefines.h"
#ifdef MIDI
#include "usb_midi.h"
#endif
#ifdef IAP
#include "iap.h"
#endif
#include "xc_ptr.h"
#include "commands.h"
#include "xud.h"
#include "testct_byref.h"

#ifdef HID_CONTROLS
#include "user_hid.h"
unsigned char g_hidData[1] = {0};
#endif

void GetADCCounts(unsigned samFreq, int &min, int &mid, int &max);
#define BUFFER_SIZE_OUT       (1028 >> 2)
#define BUFFER_SIZE_IN        (1028 >> 2)

/* Packet nuffers for audio data */

extern unsigned int g_curSamFreqMultiplier;

#ifdef CHAN_BUFF_CTRL
#define SET_SHARED_GLOBAL0(x,y) SET_SHARED_GLOBAL(x,y); outuchar(c_buff_ctrl, 0);
#else
#define SET_SHARED_GLOBAL0(x,y) SET_SHARED_GLOBAL(x,y)
#endif


/* Global var for speed.  Related to feedback. Used by input stream to determine IN packet size */
unsigned g_speed;
unsigned g_freqChange = 0;

/* Interrupt EP data */
unsigned char g_intData[8];

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
unsigned g_iap_reset = 1;
#endif

unsigned char fb_clocks[16];

//#define FB_TOLERANCE_TEST
#define FB_TOLERANCE 0x100

extern unsigned inZeroBuff[];
/**
 * Buffers data from audio endpoints
 * @param   c_aud_out     chanend for audio from xud
 * @param   c_aud_in      chanend for audio to xud
 * @param   c_aud_fb      chanend for feeback to xud
 * @return  void
 */
void buffer(register chanend c_aud_out, register chanend c_aud_in, chanend c_aud_fb,
#ifdef MIDI
            chanend c_midi_from_host,
            chanend c_midi_to_host,
            chanend c_midi,
#endif
#ifdef IAP
            chanend c_iap_from_host,
            chanend c_iap_to_host,
            chanend c_iap_to_host_int,
            chanend c_iap,
#endif
#if defined(SPDIF_RX) || defined(ADAT_RX)
            chanend ?c_int,
#endif
            chanend c_sof,
            chanend c_aud_ctl,
            in port p_off_mclk
#ifdef HID_CONTROLS
            , chanend c_hid
#endif
#ifdef CHAN_BUFF_CTRL
            , chanend c_buff_ctrl
#endif
            )
{
    XUD_ep ep_aud_out = XUD_InitEp(c_aud_out);
    XUD_ep ep_aud_in = XUD_InitEp(c_aud_in);
    XUD_ep ep_aud_fb = XUD_InitEp(c_aud_fb);
#ifdef MIDI
    XUD_ep ep_midi_from_host = XUD_InitEp(c_midi_from_host);
    XUD_ep ep_midi_to_host = XUD_InitEp(c_midi_to_host);
#endif
#ifdef IAP
    XUD_ep ep_iap_from_host   = XUD_InitEp(c_iap_from_host);
    XUD_ep ep_iap_to_host     = XUD_InitEp(c_iap_to_host);
    XUD_ep ep_iap_to_host_int = XUD_InitEp(c_iap_to_host_int);
#endif
#if defined(SPDIF_RX) || defined(ADAT_RX)
    XUD_ep ep_int = XUD_InitEp(c_int);
#endif

#ifdef HID_CONTROLS
    XUD_ep ep_hid = XUD_InitEp(c_hid);
#endif


    int tmp;
    unsigned u_tmp;
    unsigned sampleFreq = 0;
    unsigned lastClock;

    unsigned clocks = 0;

#ifdef INPUT
    unsigned bufferIn = 1;
#endif
    unsigned remnant = 0, cycles;
    unsigned sofCount = 0;
    unsigned freqChange = 0;

#ifdef FB_TOLERANCE_TEST
    unsigned expected_fb = 0;
#endif

    xc_ptr aud_from_host_buffer = 0;

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
    unsigned char iap_from_host_buffer[MAX_IAP_PACKET_SIZE+4];
    unsigned char iap_to_host_buffer[MAX_IAP_PACKET_SIZE+4];


    int is_ack_iap;
    int is_reset;
    unsigned int datum_iap;
    int iap_data_remaining_to_device = 0;
    int iap_data_collected_from_device = 0;
    int iap_expected_data_length = 0;
#endif


    xc_ptr p_inZeroBuff = array_to_xc_ptr(inZeroBuff);

#ifdef IAP
    XUD_ResetEndpoint(ep_iap_from_host, null);
    iap_send_reset(c_iap);
#endif

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
        {
           GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);
        }

        GetADCCounts(DEFAULT_FREQ, min, mid, max);
        asm("stw %0, dp[g_speed]"::"r"(mid << 16));
        if (usb_speed == XUD_SPEED_HS)
          mid*=NUM_USB_CHAN_IN*SAMPLE_SUBSLOT_SIZE_HS;
        else
          mid*=NUM_USB_CHAN_IN_FS*SAMPLE_SUBSLOT_SIZE_FS;

        asm("stw %0, %1[0]"::"r"(mid),"r"(p_inZeroBuff));

#ifdef FB_TOLERANCE_TEST
        expected_fb = ((DEFAULT_FREQ * 0x2000) / 1000);
#endif

    }

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
        GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

        if (usb_speed == XUD_SPEED_HS)
        {
            XUD_SetReady_In(ep_aud_fb, fb_clocks, 4);
        }
        else
        {
            XUD_SetReady_In(ep_aud_fb, fb_clocks, 3);
        }
    }

#ifdef MIDI
    XUD_SetReady_OutPtr(ep_midi_from_host, midi_from_host_buffer);
#endif

#ifdef IAP
    XUD_SetReady_Out(ep_iap_from_host, iap_from_host_buffer);
#endif

#ifdef HID_CONTROLS
    XUD_SetReady_In(ep_hid, g_hidData, 1);
#endif

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
                XUD_SetData_Inline(ep_int, c_int);
                asm("stw   %0, dp[g_intFlag]" :: "r" (0)  );
                break;
              }
#endif

            /* Sample Freq or chan count update from ep 0 */
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
                    int min, mid, max;
                    int usb_speed;
                    int frameTime;
                    tmp = inuint(c_aud_ctl);
                    GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

                    if(tmp == SET_SAMPLE_FREQ)
                    {
                        sampleFreq = inuint(c_aud_ctl);

                        /* Don't update things for DFU command.. */
                        if(sampleFreq != AUDIO_STOP_FOR_DFU)
                        {

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
                            expected_fb = ((sampleFreq * 0x2000) / frameTime);
   #endif

                            asm("stw %0, dp[g_speed]"::"r"(mid << 16));

                            if (usb_speed == XUD_SPEED_HS)
                                mid *= NUM_USB_CHAN_IN*SAMPLE_SUBSLOT_SIZE_HS;
                            else
                                mid *= NUM_USB_CHAN_IN_FS*SAMPLE_SUBSLOT_SIZE_FS;

                            asm("stw %0, %1[0]"::"r"(mid),"r"(p_inZeroBuff));

                            /* Reset FB */
                            /* Note, Endpoint 0 will hold off host for a sufficient period to allow our feedback
                             * to stabilise (i.e. sofCount == 128 to fire) */
                            sofCount = 0;
                            clocks = 0;
                            remnant = 0;

                        }
                        /* Ideally we want to wait for handshake (and pass back up) here.  But we cannot keep this
                        * thread locked, it must stay responsive to packets/SOFs.  So, set a flag and check for
                        * handshake elsewhere */
                        /* Pass on sample freq change to decouple */
                        SET_SHARED_GLOBAL0(g_freqChange, SET_SAMPLE_FREQ);
                        SET_SHARED_GLOBAL(g_freqChange_sampFreq, sampleFreq);
                        SET_SHARED_GLOBAL(g_freqChange_flag, SET_SAMPLE_FREQ);
                    }
                    else
                    {
                        sampleFreq = inuint(c_aud_ctl);

                        SET_SHARED_GLOBAL0(g_freqChange, tmp);                /* Set command */
                        SET_SHARED_GLOBAL(g_freqChange_sampFreq, sampleFreq); /* Set flag */
                        SET_SHARED_GLOBAL(g_freqChange_flag, tmp);
                    }
                }
                break;
            }

            #define MASK_16_13            (7)   /* Bits that should not be transmitted as part of feedback */
            #define MASK_16_10            (127) /* For Audio 1.0 we use a mask 1 bit longer than expected to avoid Windows LSB issues */
                                                /* (previously used 63 instead of 127) */

            case inuint_byref(c_sof, u_tmp):

                /* NOTE our feedback will be wrong for a couple of SOF's after a SF change due to
                 * lastClock being incorrect */
                asm("#sof");

                /* Get MCLK count */
                asm (" getts %0, res[%1]" : "=r" (u_tmp) : "r" (p_off_mclk));

                GET_SHARED_GLOBAL(freqChange, g_freqChange);
                if(freqChange == SET_SAMPLE_FREQ)
                {
                    /* Keep getting MCLK counts */
                    lastClock = u_tmp;
                }
                else
                {
                    unsigned mask = MASK_16_13, usb_speed;

                    GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

                    if(usb_speed != XUD_SPEED_HS)
                        mask = MASK_16_10;

                    /* Number of MCLKS this SOF, approx 125 * 24 (3000), sample by sample rate */
                    GET_SHARED_GLOBAL(cycles, g_curSamFreqMultiplier);
                    cycles = ((int)((short)(u_tmp - lastClock))) * cycles;

                    /* Any odd bits (lower than 16.23) have to be kept seperate */
                    remnant += cycles & mask;

                    /* Add 16.13 bits into clock count */
                    clocks += (cycles & ~mask) + (remnant & ~mask);

                    /* and overflow from odd bits. Remove overflow from odd bits. */
                    remnant &= mask;

                    /* Store MCLK for next time around... */
                    lastClock = u_tmp;

                    /* Reset counts based on SOF counting.  Expect 16ms (128 HS SOFs/16 FS SOFS) per feedback poll
                     * We always count 128 SOFs, so 16ms @ HS, 128ms @ FS */
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

                            GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

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
                        else
                        {
                        }
#endif
                        clocks = 0;
                    }

                    sofCount++;
                }
            break;



#ifdef INPUT
            /* DEVICE -> HOST */
            case XUD_SetData_Select(c_aud_in, ep_aud_in, tmp):
            {
                /* Inform stream that buffer sent */
                SET_SHARED_GLOBAL0(g_aud_to_host_flag, bufferIn+1);
            }
            break;
#endif

#ifdef OUTPUT
            /* Feedback Pipe */
            case XUD_SetData_Select(c_aud_fb, ep_aud_fb, tmp):
            {

                int usb_speed;
                int x;

                asm("#aud fb");

                asm("ldaw %0, dp[fb_clocks]":"=r"(x));
                GET_SHARED_GLOBAL(usb_speed, g_curUsbSpeed);

                if (usb_speed == XUD_SPEED_HS)
                {
                    XUD_SetReady_In(ep_aud_fb, fb_clocks, 4);
                }
                else
                {
                    XUD_SetReady_In(ep_aud_fb, fb_clocks, 3);
                }
            }
            break;

            /* Audio HOST -> DEVICE */
            case XUD_GetData_Select(c_aud_out, ep_aud_out, tmp):
            {
                asm("#h->d aud data");

                GET_SHARED_GLOBAL(aud_from_host_buffer, g_aud_from_host_buffer);

                write_via_xc_ptr(aud_from_host_buffer, tmp);

                /* Sync with decouple thread */
                SET_SHARED_GLOBAL0(g_aud_from_host_flag, 1);
             }
                break;
#endif

#ifdef MIDI
        case XUD_GetData_Select(c_midi_from_host, ep_midi_from_host, tmp):
            asm("#midi h->d");

            /* Get buffer data from host - MIDI OUT from host always into a single buffer */
            /* Write datalength (tmp) into buffer[0], data stored in buffer[4] onwards */
            midi_data_remaining_to_device = tmp;

            midi_from_host_rdptr = midi_from_host_buffer;

            if (midi_data_remaining_to_device)
            {
                read_via_xc_ptr(datum, midi_from_host_rdptr);
                outuint(c_midi, datum);
                midi_from_host_rdptr += 4;
                midi_data_remaining_to_device -= 4;
            }
            break;

        /* MIDI IN to host */
        case XUD_SetData_Select(c_midi_to_host, ep_midi_to_host, tmp):
            asm("#midi d->h");

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
        /* IAP OUT from host */
        case XUD_GetData_Select(c_iap_from_host, ep_iap_from_host, tmp):
            asm("#iap h->d");
            if(tmp >= 0)
            {
                iap_data_remaining_to_device = tmp;

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
            else if(tmp==-1)
            {
                XUD_ResetEndpoint(ep_iap_from_host, null);
                iap_send_reset(c_iap); // What if this happen in the middle of a send/ack?
                iap_data_collected_from_device = 0;
                iap_data_remaining_to_device = -1;
                iap_expected_data_length = 0;
                iap_from_host_rdptr = 0;
            }
            break;

        /* IAP IN to host */
        case XUD_SetData_Select(c_iap_to_host, ep_iap_to_host, tmp):
            asm("#iap d->h");

            /* Send out an iAP packet to host, ACK last msg from iAP to let it know we can move on..*/
            iap_send_ack(c_iap);
            break;  /* IAP IN to host */

        case XUD_SetData_Select(c_iap_to_host_int, ep_iap_to_host_int, tmp):
            asm("#iap int d->h");
            /* Do nothing.. */
            break;
#endif

#ifdef HID_CONTROLS
            /* HID Report Data */
            case XUD_SetData_Select(c_hid, ep_hid, tmp):
            {
                g_hidData[0]=0;
                UserReadHIDButtons(g_hidData);
                XUD_SetReady_In(ep_hid, g_hidData, 1);
            }
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
                        XUD_SetReady_OutPtr(ep_midi_from_host, midi_from_host_buffer);
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
                else
                {
                    if (iap_expected_data_length == 0)
                    {
                        /* Expect a length from iAP core */
                        iap_send_ack(c_iap);
                        iap_expected_data_length = datum_iap;
                    }
                    else
                    {
                        if (iap_data_collected_from_device < IAP_USB_BUFFER_TO_HOST_SIZE)
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
                            XUD_SetReady_In(ep_iap_to_host_int, gc_zero_buffer, 0);
                            XUD_SetReady_In(ep_iap_to_host, iap_to_host_buffer, iap_data_collected_from_device);
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
                break;
#endif


        }

    }
}
