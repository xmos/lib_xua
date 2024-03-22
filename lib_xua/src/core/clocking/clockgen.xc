// Copyright 2011-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <assert.h>
#include <print.h>

#include "xua.h"
#include "xua_commands.h"
#include "xua_clocking.h"

#if (XUA_SPDIF_RX_EN)
#include "spdif.h"
#endif

#define LOCAL_CLOCK_INCREMENT       (166667)
#define LOCAL_CLOCK_MARGIN          (1666)

#define MAX_SAMPLES                 (64)                    /* Must be power of 2 */
#define MAX_SPDIF_SAMPLES           (2 * MAX_SAMPLES)       /* Must be power of 2 */
#define MAX_ADAT_SAMPLES            (8 * MAX_SAMPLES)       /* Must be power of 2 */

#define SPDIF_FRAME_ERRORS_THRESH	(40)

unsigned g_digData[10];

typedef struct
 {
    int receivedSamples;        /* Uses by clockgen to count number of dig rx samples to ascertain clock specs */
    int samples;                /* Raw sample count - rolling int and never reset */
    int savedSamples;           /* Used by validSamples() to store state of last raw sample count */
    int lastDiff;               /* Used by validSamples() to store state of last sample count diff */
    unsigned identicaldiffs;    /* Used by validSamples() to store state of number of identical diffs */
    int samplesPerTick;
} Counter;

static int clockFreq[NUM_CLOCKS];                           /* Store current clock freq for each clock unit */
static int clockValid[NUM_CLOCKS];                          /* Store current validity of each clock unit */
static int clockInt[NUM_CLOCKS];                            /* Interupt flag for clocks */
static int clockId[NUM_CLOCKS];


[[distributable]]
void PllRefPinTask(server interface pll_ref_if i_pll_ref, out port p_pll_ref)
{
    static unsigned pinVal= 0;
    static unsigned short pinTime = 0;

    while(1)
    {
        select
        {
            case i_pll_ref.toggle():
                pinVal = ~pinVal;
                 p_pll_ref <: pinVal;
                break;

            case i_pll_ref.init():
                p_pll_ref <: pinVal @ pinTime;
                pinTime += (unsigned short)(LOCAL_CLOCK_INCREMENT - (LOCAL_CLOCK_INCREMENT/2));
                p_pll_ref @ pinTime <: pinVal;
                break;

            case i_pll_ref.toggle_timed(int relative):

                if (!relative)
                {
                    pinTime += (short) LOCAL_CLOCK_INCREMENT;
                    pinVal = !pinVal;
                    p_pll_ref @ pinTime <: pinVal;
                }
                else
                {
                    p_pll_ref <: pinVal @ pinTime;
                    pinTime += (short) LOCAL_CLOCK_INCREMENT;
                    pinVal = !pinVal;
                    p_pll_ref @ pinTime <: pinVal;
                }
                break;
        }
    }
}


#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN)
static int abs(int x)
{
    if (x < 0) return -x;
    return x;
}

static void outInterrupt(chanend c_interruptControl, int value)
{
    outuint(c_interruptControl, value);
    outct(c_interruptControl, XS1_CT_END);
}
#endif

#ifdef CLOCK_VALIDITY_CALL
void VendorClockValidity(int valid);
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
static inline void setClockValidity(chanend c_interruptControl, int clkIndex, int valid, int currentClkMode)
{
    if (clockValid[clkIndex] != valid)
    {
        clockValid[clkIndex] = valid;
        outInterrupt(c_interruptControl, clockId[clkIndex]);

#ifdef CLOCK_VALIDITY_CALL
#if (XUA_ADAT_RX_EN)
        if (currentClkMode == CLOCK_ADAT && clkIndex == CLOCK_ADAT_INDEX)
        {
            VendorClockValidity(valid);
        }
#endif
#if (XUA_SPDIF_RX_EN)
        if (currentClkMode == CLOCK_SPDIF && clkIndex == CLOCK_SPDIF_INDEX)
        {
            VendorClockValidity(valid);
        }
#endif
#endif
    }
}

/* Returns 1 for valid clock found else 0 */
static inline int validSamples(Counter &counter, int clockIndex)
{
    int diff = counter.samples - counter.savedSamples;

    counter.savedSamples = counter.samples;

    /* Check for stable sample rate (with some small margin) */
    if (diff != 0 && abs( diff - counter.lastDiff ) < 5 )
    {
        counter.identicaldiffs++;

        if (counter.identicaldiffs > 10)
        {
            /* Detect current sample rate (round to nearest) */
            int s = -1;

            if (diff > 137 && diff < 157)
            {
                s = 147;
            }
            else if (diff > 150 && diff < 170)
            {
                s = 160;
            }
            else if(diff > 284 && diff < 304)
            {
                s = 294;
            }
            else if (diff > 310 && diff < 330)
            {
                s = 320;
            }
            else if (diff > 578 && diff < 598)
            {
                s = 588;
            }
            else if (diff > 630 && diff < 650)
            {
                s = 640;
            }

            /* Check if we found a valid freq */
            if (s != -1)
            {
                /* Update expected samples per tick */
                counter.samplesPerTick = s;

                /* Update record of external clock source sample frequency */
                s *= 300;


                if (clockFreq[clockIndex] != s)
                {
                    clockFreq[clockIndex] = s;
                }

                return 1;
            }
            else
            {
                /* Not a valid frequency - Reset counter and find another run of samples */
                counter.identicaldiffs = 0;
            }
        }
    }
    else /* No valid frequency found - reset state */
    {
        counter.identicaldiffs = 0;
        counter.lastDiff = diff;
    }
    return 0;
}
#endif

#if XUA_USE_SW_PLL
unsafe
{
    unsigned * unsafe selected_mclk_rate_ptr = NULL;
}
#endif

#ifdef LEVEL_METER_LEDS
void VendorLedRefresh(unsigned levelData[]);
unsigned g_inputLevelData[NUM_USB_CHAN_IN];
extern int samples_to_host_inputs[NUM_USB_CHAN_IN];
extern int samples_to_host_inputs_buff[NUM_USB_CHAN_IN];
#endif

int VendorAudCoreReqs(unsigned cmd, chanend c);

#pragma unsafe arrays
void clockGen ( streaming chanend ?c_spdif_rx,
                chanend ?c_adat_rx,
                client interface pll_ref_if i_pll_ref,
                chanend c_dig_rx,
                chanend c_clk_ctl,
                chanend c_clk_int,
                chanend c_audio_rate_change
#if XUA_USE_SW_PLL
                , port p_for_mclk_count_aud
                , chanend c_sw_pll
#endif
)
{
    timer t_local;
    unsigned timeNextEdge, timeLastEdge, timeNextClockDetection;
    unsigned clkMode = CLOCK_INTERNAL;              /* Current clocking mode in operation */
    unsigned tmp;

    /* Start in no-SMUX (8-channel) mode */
    int smux = 0;

#ifdef LEVEL_METER_LEDS
    timer t_level;
    unsigned levelTime;
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    timer t_external;
    unsigned selected_mclk_rate = MCLK_48; // Assume 24.576MHz initial clock 
    unsigned selected_sample_rate = 0;
#if XUA_USE_SW_PLL

    unsigned mclks_per_sample = 0;
    unsigned short mclk_time_stamp = 0;

    /* Get MCLK count */
    asm volatile(" getts %0, res[%1]" : "=r" (mclk_time_stamp) : "r" (p_for_mclk_count_aud));
#endif
#endif

#if (XUA_SPDIF_RX_EN)
    /* S/PDIF buffer state */
    int spdifSamples[MAX_SPDIF_SAMPLES];           /* S/PDIF sample buffer */
    int spdifWr = 0;                               /* Write index */
    int spdifRd = 0;                               /* Read index */ //(spdifWriteIndex ^ (MAX_SPDIF_SAMPLES >> 1)) & ~1;   // Start in middle
    int spdifOverflow = 0;                         /* Overflow/undeflow flags */
    int spdifUnderflow = 1;
    int spdifSamps = 0;                            /* Number of samples in buffer */
    Counter spdifCounters;
    int spdifRxTime;
    unsigned tmp2;
    unsigned spdifLeft = 0;
    unsigned spdifRxData;
#endif

#if (XUA_ADAT_RX_EN)
    /* ADAT buffer state */
    int adatSamples[MAX_ADAT_SAMPLES];
    int adatWr = 0;
    int adatRd = 0;
    int adatOverflow = 0;
    int adatUnderflow = 1;
    //int adatFrameErrors = 0;
    int adatSamps = 0;
    Counter adatCounters;
    int adatReceivedTime;

    unsigned adatFrame[8];
    int adatChannel = 0;
    int adatSamplesEver = 0;
#endif
    for(int i = 0; i < 10; i++)
    {
       g_digData[i] = 0;
    }

    /* Init clock unit state */
    clockFreq[CLOCK_INTERNAL] = 0;
    clockId[CLOCK_INTERNAL] = ID_CLKSRC_INT;
    clockValid[CLOCK_INTERNAL] = 0;
    clockInt[CLOCK_INTERNAL] = 0;
#if (XUA_SPDIF_RX_EN)
    clockFreq[CLOCK_SPDIF] = 0;
    clockValid[CLOCK_SPDIF] = 0;
    clockInt[CLOCK_SPDIF] = 0;
    clockId[CLOCK_SPDIF] = ID_CLKSRC_SPDIF;
#endif
#if (XUA_ADAT_RX_EN)
    clockFreq[CLOCK_ADAT] = 0;
    clockInt[CLOCK_ADAT] = 0;
    clockValid[CLOCK_ADAT] = 0;
    clockId[CLOCK_ADAT] = ID_CLKSRC_ADAT;
#endif
#if (XUA_SPDIF_RX_EN)
    spdifCounters.receivedSamples = 0;
    spdifCounters.samples = 0;
    spdifCounters.savedSamples = 0;
    spdifCounters.lastDiff = 0;
    spdifCounters.identicaldiffs = 0;
    spdifCounters.samplesPerTick = 0;
#endif

#if (XUA_ADAT_RX_EN)
    adatCounters.receivedSamples = 0;
    adatCounters.samples = 0;
    adatCounters.savedSamples = 0;
    adatCounters.lastDiff = 0;
    adatCounters.identicaldiffs = 0;
    adatCounters.samplesPerTick = 0;
#endif

    t_local :> timeNextEdge;
    timeLastEdge = timeNextEdge;
    timeNextClockDetection = timeNextEdge + (LOCAL_CLOCK_INCREMENT / 2);
    timeNextEdge += LOCAL_CLOCK_INCREMENT;

#ifdef LEVEL_METER_LEDS
    t_level :> levelTime;
    levelTime+= LEVEL_UPDATE_RATE;
#endif

#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN)
    /* Fill channel */
    outuint(c_dig_rx, 1);
#endif

    /* Initial ref clock output and get timestamp */
    i_pll_ref.init();

#if ((XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN) && XUA_USE_SW_PLL)
    int reset_sw_pll_pfd = 1;
    int require_ack_to_audio = 0;
    stability_state_t stability_state;
    init_stability(stability_state, 5, 10);                 /* Typically runs at -1 to +1 maximum when locked so 5 or less is good. 
                                                               10 = number of error calcs at below threshold before considered stable */

    restart_sigma_delta(c_sw_pll, MCLK_48); /* default to 48kHz - this will be reset shortly when host selects rate */
#endif

    while(1)
    {
        select
        {
#ifdef LEVEL_METER_LEDS
#warning Level metering enabled
            case t_level when timerafter(levelTime) :> void:

                levelTime += LEVEL_UPDATE_RATE;

                /* Copy over level data and reset */
                for(int i = 0; i< NUM_USB_CHAN_IN; i++)
                {
                    int tmp;

                    /* Read level data */
                    //g_inputLevelData[i] = samples_to_host_inputs[i];
                    asm volatile("ldw %0, %1[%2]":"=r"(tmp):"r"((const int *)samples_to_host_inputs),"r"(i));
                    g_inputLevelData[i] = tmp;

                    /* Reset level data */
                    //samples_to_host_inputs[i] = 0;
                    asm volatile("stw %0, %1[%2]"::"r"(0),"r"((const int *)samples_to_host_inputs),"r"(i));

                    /* Guard against host polling slower than timer and missing peaks */
                    asm volatile("ldw %0, %1[%2]":"=r"(tmp):"r"((const int *)samples_to_host_inputs_buff),"r"(i));

                    if (g_inputLevelData[i] > tmp)
                    //if(g_inputLevelData[i] > samples_to_host_inputs_buff[i])
                    {
                        //samples_to_host_inputs_buff[i] = g_inputLevelData[i];
                        asm volatile("stw %0, %1[%2]"::"r"(tmp),"r"((const int *)samples_to_host_inputs),"r"(i));
                    }
                }

                /* Call user LED refresh */
                VendorLedRefresh(g_inputLevelData);

                break;
#endif

            /* Updates to clock settings from endpoint 0 */
            case inuint_byref(c_clk_ctl, tmp):
                switch(tmp)
                {
                    case GET_SEL:
                        chkct(c_clk_ctl, XS1_CT_END);

                        /* Send back current clock mode */
                        outuint(c_clk_ctl, clkMode);
                        outct(c_clk_ctl, XS1_CT_END);

                        break;

                    case SET_SEL:
                        /* Update clock mode */
                        clkMode = inuint(c_clk_ctl);
                        chkct(c_clk_ctl, XS1_CT_END);

#ifdef CLOCK_VALIDITY_CALL
                        switch(clkMode)
                        {
                            case CLOCK_INTERNAL:
                                VendorClockValidity(1);
                                break;
#if (XUA_ADAT_RX_EN)
                            case CLOCK_ADAT:
                                VendorClockValidity(clockValid[CLOCK_ADAT]);
                                break;
#endif
#if (XUA_SPDIF_RX_EN)
                            case CLOCK_SPDIF:
                                VendorClockValidity(clockValid[CLOCK_SPDIF]);
                                break;
#endif
                        }
#endif
                        break;

                    case GET_VALID:
                        /* Clock Unit Index */
                        tmp = inuint(c_clk_ctl);
                        chkct(c_clk_ctl, XS1_CT_END);
                        outuint(c_clk_ctl, clockValid[tmp]);
                        outct(c_clk_ctl, XS1_CT_END);
                        break;

                    case GET_FREQ:
                        tmp = inuint(c_clk_ctl);
                        chkct(c_clk_ctl, XS1_CT_END);
                        outuint(c_clk_ctl, clockFreq[tmp]);
                        outct(c_clk_ctl, XS1_CT_END);
                        break;

                    case SET_SMUX:
                        smux = inuint(c_clk_ctl);
#if (XUA_ADAT_RX_EN)
                        adatRd = 0; /* Reset adat FIFO */
                        adatWr = 0;
                        adatSamps = 0;
#endif
                        chkct(c_clk_ctl, XS1_CT_END);
                        break;

                    default:
#ifdef VENDOR_AUDCORE_REQS
                            if(VendorAudCoreReqs(tmp, c_clk_ctl))
#endif
                            printstrln("ERR: Bad req in clockgen\n");
                        break;
                }

                break;

            /* Generate local clock from timer */
            case t_local when timerafter(timeNextEdge) :> void:

#if XUA_USE_SW_PLL
                /* Hold the most recent sw_pll setting */
                if(require_ack_to_audio)
                {
                    c_audio_rate_change <: 0;     /* ACK back to audio to release to start I2S */
                    require_ack_to_audio = 0;
                    printstr("stable int\n");
                }
#else
                /* Setup next local clock edge */
                i_pll_ref.toggle_timed(0);
#endif
                /* Record time of edge */
                timeLastEdge = timeNextEdge;

                /* Setup for next edge */
                timeNextClockDetection = timeNextEdge + (LOCAL_CLOCK_INCREMENT/2);
                timeNextEdge += LOCAL_CLOCK_INCREMENT;

                /* If we are in an external clock mode and this fire, then clock invalid
                 * reset counters in case we are moved to digital clock - we want a well timed
                 * first edge */
#if (XUA_SPDIF_RX_EN)
                spdifCounters.receivedSamples = 0;
#endif
#if (XUA_ADAT_RX_EN)
                adatCounters.receivedSamples = 0;
#endif

#ifdef CLOCK_VALIDITY_CALL
                if(clkMode == CLOCK_INTERNAL)
                {
                    /* Internal clock always valid */
                    VendorClockValidity(1);
                }
#endif
                break;

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
            case t_external when timerafter(timeNextClockDetection) :> void:
                {
                    int valid;
                    timeNextClockDetection += (LOCAL_CLOCK_INCREMENT);
#if (XUA_SPDIF_RX_EN)
                    /* Returns 1 if valid clock found */
                    valid = validSamples(spdifCounters, CLOCK_SPDIF);
                    setClockValidity(c_clk_int, CLOCK_SPDIF, valid, clkMode);
#endif
#if (XUA_ADAT_RX_EN)
                    /* Returns 1 if valid clock found */
                    valid = validSamples(adatCounters, CLOCK_ADAT);
                    setClockValidity(c_clk_int, CLOCK_ADAT, valid, clkMode);
#endif
                }
                break;
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                /* Receive notification of audio streaming settings change and store */
            case c_audio_rate_change :> selected_mclk_rate:
                c_audio_rate_change :> selected_sample_rate;
#if XUA_USE_SW_PLL
                mclks_per_sample = selected_mclk_rate / selected_sample_rate;
                restart_sigma_delta(c_sw_pll, selected_mclk_rate);
                reset_sw_pll_pfd = 1;
                reset_stability(stability_state);
                /* We will shedule an ACK when sigma delta is up and running */
                require_ack_to_audio = 1;
                printstr("SR\n");
#else
                /* Send ACK immediately as we are good to go if not using SW_PLL */
                c_audio_rate_change <: 0;
#endif
                break;
#endif

#if (XUA_SPDIF_RX_EN)
            /* Receive sample from S/PDIF RX thread (streaming chan) */
            case c_spdif_rx :> spdifRxData:

#if XUA_USE_SW_PLL
                /* Record time of sample */
                asm volatile(" getts %0, res[%1]" : "=r" (mclk_time_stamp) : "r" (p_for_mclk_count_aud));
#endif
                t_local :> spdifRxTime;

                /* Check parity and ignore if bad */
                if(spdif_rx_check_parity(spdifRxData))
                    continue;

                /* Get preamble */
                unsigned preamble = spdifRxData & SPDIF_RX_PREAMBLE_MASK;

                switch(preamble)
                {
                    /* LEFT */
                    case SPDIF_FRAME_X:
                    case SPDIF_FRAME_Z:
                        spdifLeft = SPDIF_RX_EXTRACT_SAMPLE(spdifRxData);
                        break;

                    /* RIGHT */
                    case SPDIF_FRAME_Y:

                        /* Only store sample if not in overflow and stream is reasonably valid */
                        if(!spdifOverflow && clockValid[CLOCK_SPDIF])
                        {
                            /* Store left and right sample pair to buffer */
                            spdifSamples[spdifWr] = spdifLeft;
                            spdifSamples[spdifWr+1] = SPDIF_RX_EXTRACT_SAMPLE(spdifRxData);

                            spdifWr = (spdifWr + 2) & (MAX_SPDIF_SAMPLES - 1);

                            spdifSamps += 2;

                            /* Check for over flow */
                            if(spdifSamps > MAX_SPDIF_SAMPLES-1)
                            {
                                spdifOverflow = 1;
                            }

                            /* Check for coming out of under flow */
                            if(spdifUnderflow && (spdifSamps >= (MAX_SPDIF_SAMPLES >> 1)))
                            {
                                spdifUnderflow = 0;
                            }
                        }
                        break;

                        default:
                            /* Bad sample, skip */
                            continue;
                            break;
                    }

                spdifCounters.samples += 1;

                if(clkMode == CLOCK_SPDIF && clockValid[CLOCK_SPDIF])
                {
                    spdifCounters.receivedSamples+=1;

                    /* Inspect for if we need to produce an edge */
                    if((spdifCounters.receivedSamples >=  spdifCounters.samplesPerTick))
                    {
                        /* Check edge is about right... S/PDIF may have changed freq... */
                        if(timeafter(spdifRxTime, (timeLastEdge + LOCAL_CLOCK_INCREMENT - LOCAL_CLOCK_MARGIN)))
                        {
                            /* Record edge time */
                            timeLastEdge = spdifRxTime;

                            /* Setup for next edge */
                            timeNextEdge = spdifRxTime + LOCAL_CLOCK_INCREMENT + LOCAL_CLOCK_MARGIN;

#if XUA_USE_SW_PLL
                            int stable = do_sw_pll_phase_frequency_detector_dig_rx( mclk_time_stamp,
                                                                                    mclks_per_sample,
                                                                                    c_sw_pll,
                                                                                    spdifCounters.receivedSamples,
                                                                                    reset_sw_pll_pfd,
                                                                                    stability_state);
                            if(stable && require_ack_to_audio)
                            {
                                c_audio_rate_change <: 0;     /* ACK back to audio to release to start */
                                require_ack_to_audio = 0;
                                printstr("stable ext\n");
                            }
#else
                            /* Toggle edge */
                            i_pll_ref.toggle_timed(1);
#endif
                            /* Reset counters */
                            spdifCounters.receivedSamples = 0;
                        }
                    }
                }
                break;
#endif
#if (XUA_ADAT_RX_EN)
                /* receive sample from ADAT rx thread (streaming channel with CT_END) */
                case inuint_byref(c_adat_rx, tmp):

#if XUA_USE_SW_PLL
                    /* record time of sample */
                    asm volatile(" getts %0, res[%1]" : "=r" (mclk_time_stamp) : "r" (p_for_mclk_count_aud));
#endif 
                    t_local :> adatReceivedTime;

                    /* Sync is: 1 | (user_byte << 4) */
                    if(tmp&1)
                    {
                        /* user bits - start of frame */
                        adatChannel = 0;
                        continue;
                    }
                    else
                    {
                        /* audio sample */
                        adatSamplesEver++;
                        adatFrame[adatChannel] = tmp;

                        adatChannel++;
                        if (adatChannel == 8)
                        {
                            /* only store left samples if not in overflow and stream is reasonably valid */
                            if (!adatOverflow && clockValid[CLOCK_ADAT])
                            {
                                /* Unpick the SMUX.. */
                                if(smux == 2)
                                {
                                    adatSamples[adatWr + 0] = adatFrame[0];
                                    adatSamples[adatWr + 1] = adatFrame[4];
                                    adatSamples[adatWr + 2] = adatFrame[1];
                                    adatSamples[adatWr + 3] = adatFrame[5];
                                    adatSamples[adatWr + 4] = adatFrame[2];
                                    adatSamples[adatWr + 5] = adatFrame[6];
                                    adatSamples[adatWr + 6] = adatFrame[3];
                                    adatSamples[adatWr + 7] = adatFrame[7];
                                }
                                else if(smux)
                                {

                                    adatSamples[adatWr + 0] = adatFrame[0];
                                    adatSamples[adatWr + 1] = adatFrame[2];
                                    adatSamples[adatWr + 2] = adatFrame[4];
                                    adatSamples[adatWr + 3] = adatFrame[6];
                                    adatSamples[adatWr + 4] = adatFrame[1];
                                    adatSamples[adatWr + 5] = adatFrame[3];
                                    adatSamples[adatWr + 6] = adatFrame[5];
                                    adatSamples[adatWr + 7] = adatFrame[7];
                                }
                                else
                                {
                                    adatSamples[adatWr + 0] = adatFrame[0];
                                    adatSamples[adatWr + 1] = adatFrame[1];
                                    adatSamples[adatWr + 2] = adatFrame[2];
                                    adatSamples[adatWr + 3] = adatFrame[3];
                                    adatSamples[adatWr + 4] = adatFrame[4];
                                    adatSamples[adatWr + 5] = adatFrame[5];
                                    adatSamples[adatWr + 6] = adatFrame[6];
                                    adatSamples[adatWr + 7] = adatFrame[7];
                                 }
                                    adatWr = (adatWr + 8) & (MAX_ADAT_SAMPLES - 1);
                                    adatSamps += 8;

                                    /* check for overflow */
                                    if (adatSamps > MAX_ADAT_SAMPLES - 1)
                                    {
                                        adatOverflow = 1;
                                    }

                                    /* check for coming out of underflow */
                                    if (adatUnderflow && (adatSamps >= (MAX_ADAT_SAMPLES >> 1)))
                                    {
                                        adatUnderflow = 0;
                                    }
                                }
                        }
                        if(adatChannel == 4 || adatChannel == 8)
                        {
                            adatCounters.samples += 1;

                                if (clkMode == CLOCK_ADAT && clockValid[CLOCK_ADAT])
                                {
                                    adatCounters.receivedSamples += 1;

                                    /* Inspect for if we need to produce an edge */
                                    if ((adatCounters.receivedSamples >= adatCounters.samplesPerTick))
                                    {
                                        /* Check edge is about right... ADAT may have changed freq... */
                                        if (timeafter(adatReceivedTime, (timeLastEdge + LOCAL_CLOCK_INCREMENT - LOCAL_CLOCK_MARGIN)))
                                        {
                                            /* Record edge time */
                                            timeLastEdge = adatReceivedTime;

                                            /* Setup for next edge */
                                            timeNextEdge = adatReceivedTime + LOCAL_CLOCK_INCREMENT + LOCAL_CLOCK_MARGIN;

#if XUA_USE_SW_PLL
                                            do_sw_pll_phase_frequency_detector_dig_rx(  mclk_time_stamp,
                                                                                        mclks_per_sample,
                                                                                        c_sw_pll,
                                                                                        adatCounters.receivedSamples,
                                                                                        reset_sw_pll_pfd);

                                            if(stable && require_ack_to_audio)
                                            {
                                                c_audio_rate_change <: 0;     /* ACK back to audio to release to start */
                                                require_ack_to_audio = 0;
                                                printstr("stable ext\n");
                                            }
#else
                                            /* Toggle edge */
                                            i_pll_ref.toggle_timed(1);
#endif
                                            
                                            /* Reset counters */
                                            adatCounters.receivedSamples = 0;
                                        }
                                    }
                                }
                            }
                            if (adatChannel == 8)
                              adatChannel = 0;
                        }
                    break;
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                /* AudioHub requests data */
                case inuint_byref(c_dig_rx, tmp):
#if (XUA_SPDIF_RX_EN)
                    if(spdifUnderflow)
                    {
                        /* S/PDIF underflowing, send out zero samples */
                        g_digData[0] = 0;
                        g_digData[1] = 0;
                    }
                    else
                    {
                        /* Read out samples from S/PDIF buffer and send... */
                        tmp = spdifSamples[spdifRd];
                        tmp2 = spdifSamples[spdifRd + 1];

                        spdifRd += 2;
                        spdifRd &= (MAX_SPDIF_SAMPLES - 1);

                        g_digData[0] = tmp;
                        g_digData[1] = tmp2;

                        spdifSamps -= 2;

                        /* spdifSamps could go to -1 */
                        if(spdifSamps <= 0)
                        {
                            /* We're out of S/PDIF samples, mark underflow condition */
                            spdifUnderflow = 1;
                            spdifLeft = 0;
                        }

                        /* If we are in over flow condition and we have a sensible number of samples
                            * come out of overflow condition */
                        if(spdifOverflow && (spdifSamps < (MAX_SPDIF_SAMPLES>>1)))
                        {
                            spdifOverflow = 0;
                        }
                    }
#endif
#if (XUA_ADAT_RX_EN)
                if (adatUnderflow)
                {
                    /* ADAT underflowing, send out zero samples */
                    g_digData[2] = 0;
                    g_digData[3] = 0;
                    g_digData[4] = 0;
                    g_digData[5] = 0;
                    g_digData[6] = 0;
                    g_digData[7] = 0;
                    g_digData[8] = 0;
                    g_digData[9] = 0;
                }
                else
                {
                    /* read out samples from the ADAT buffer and send */
                    /* always return 8 samples */
                    /* SMUX II mode */
                    if (smux == 2)
                    {
                        /* SMUX2 mode - 2 samples from fifo and 4 zero samples */
                        g_digData[2] = adatSamples[adatRd + 0];
                        g_digData[3] = adatSamples[adatRd + 1];

                        g_digData[4] = 0;
                        g_digData[5] = 0;
                        g_digData[6] = 0;
                        g_digData[7] = 0;
                        g_digData[8] = 0;
                        g_digData[9] = 0;
                        adatRd = (adatRd + 2) & (MAX_ADAT_SAMPLES - 1);
                        adatSamps -= 2;
                    }
                    else if(smux)
                    {
                        /* SMUX mode - 4 samples from fifo and 4 zero samples */
                        g_digData[2] = adatSamples[adatRd + 0];
                        g_digData[3] = adatSamples[adatRd + 1];
                        g_digData[4] = adatSamples[adatRd + 2];
                        g_digData[5] = adatSamples[adatRd + 3];

                        g_digData[6] = 0;
                        g_digData[7] = 0;
                        g_digData[8] = 0;
                        g_digData[9] = 0;
                        adatRd = (adatRd + 4) & (MAX_ADAT_SAMPLES - 1);
                        adatSamps -= 4;
                    }
                    else
                    {
                        /* no SMUX mode - 8 samples from fifo */
                        g_digData[2] = adatSamples[adatRd + 0];
                        g_digData[3] = adatSamples[adatRd + 1];
                        g_digData[4] = adatSamples[adatRd + 2];
                        g_digData[5] = adatSamples[adatRd + 3];

                        g_digData[6] = adatSamples[adatRd + 4];
                        g_digData[7] = adatSamples[adatRd + 5];
                        g_digData[8] = adatSamples[adatRd + 6];
                        g_digData[9] = adatSamples[adatRd + 7];

                        adatRd = (adatRd + 8) & (MAX_ADAT_SAMPLES - 1);
                        adatSamps -= 8;
                    }

                    /* adatSamps could go to -1 */
                    if (adatSamps <= 0)
                    {
                        /* we're out of ADAT samples, mark underflow condition */
                        adatUnderflow = 1;
                    }

                    /* if we are in overflow condition and have a sensible number of samples
                       come out of overflow condition */
                    if (adatOverflow && adatSamps < (MAX_ADAT_SAMPLES >> 1))
                    {
                        adatOverflow = 0;
                    }
                }
#endif
                outuint(c_dig_rx, 1);
                break;
#endif
        } /* select */
    } /* while(1) */
} /* clkgen task scope */

