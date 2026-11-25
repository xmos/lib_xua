// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Adaptive playback clock recovery test.
 *
 * Drives XUA_Buffer_Ep() with SOFs and playback packets from a fake XUD and checks the rate errors
 * it sends to the SW PLL, which is replaced by the checker.
 *
 * MCLK is emulated by clocking the MCLK count port at 25 MHz from the reference clock, with a
 * (micro)frame period chosen so that each 1 ms measurement interval is exactly 48 samples of MCLK
 * ticks. The host sample rate is set by the number of samples in each packet:
 *  - Nominal: 48 samples per interval, so an error of 0.
 *  - Fast:    one extra sample every ADJUST_PERIOD intervals, so a negative error. The first report
 *             spans the change of rate and is not checked. One packet is dropped, which must
 *             discard its window rather than report a corrupt error.
 *  - Slow:    one sample fewer every ADJUST_PERIOD intervals, so a positive error. The phase starts
 *             with an MCLK rate change, which restarts the SW PLL and discards the current window.
 * Phase lengths give at least MIN_CHECKED_REPORTS complete windows in each phase.
 */

#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include "print.h"
#include "xua.h"
#include "xud.h"
#include "xua_commands.h"
#include "sw_pll_wrapper.h"
#include "testct_byref.h"
#include "xc_ptr.h"

#ifndef BUS_SPEED
#error BUS_SPEED should be defined
#endif

#define EP_COUNT_OUT            (2)
#define EP_COUNT_IN             (1)
#define EP_NUM_AUD_OUT          (1)

#define SAMPLE_FREQ             (48000)
#define MCLKS_PER_SAMPLE        (MCLK_48 / SAMPLE_FREQ)
#define SAMPLES_PER_INTERVAL    (SAMPLE_FREQ / 1000)

/* MCLK count port clocked at 100 MHz / (2 * 2) = 25 MHz */
#define MCLK_PORT_DIV           (2)
#define REF_TICKS_PER_MCLK      (2 * MCLK_PORT_DIV)
#define INTERVAL_TICKS          (SAMPLES_PER_INTERVAL * MCLKS_PER_SAMPLE * REF_TICKS_PER_MCLK)

#if (BUS_SPEED == 2) // XUD_SPEED_HS
#define FRAMES_PER_INTERVAL     (8)
#else
#define FRAMES_PER_INTERVAL     (1)
#endif
#define FRAME_TICKS             (INTERVAL_TICKS / FRAMES_PER_INTERVAL)
#define SAMPLES_PER_FRAME       (SAMPLES_PER_INTERVAL / FRAMES_PER_INTERVAL)
#define PACKET_DELAY_TICKS      (1000)

/* Stream format sent at stream start. The subslot differs from the descriptor default, so the
 * test fails if XUA_Buffer_Ep() counts samples using the default instead. */
#define NUM_CHANS               (2)
#define SUBSLOT_BYTES           (3)

#define WINDOW_INTERVALS        (XUA_RATE_WINDOW_ADAPTIVE_INTERVALS)
#define ADJUST_PERIOD           (4)
#define ADJUSTED_ERROR          ((WINDOW_INTERVALS / ADJUST_PERIOD) * MCLKS_PER_SAMPLE)
#define ERROR_TOLERANCE         (16)
#define DROP_INTERVAL           (2 * WINDOW_INTERVALS + 3)
#define MIN_CHECKED_REPORTS     (2)

#if (WINDOW_INTERVALS % ADJUST_PERIOD)
#error Windows must contain whole adjustment periods
#endif

typedef enum
{
    PHASE_NOMINAL,
    PHASE_FAST,
    PHASE_SLOW,
    PHASE_COUNT,
} phase_t;

/* Phase lengths in intervals must be whole adjustment periods */
static const int phase_intervals[PHASE_COUNT] = {2 * WINDOW_INTERVALS + ADJUST_PERIOD,
                                                 4 * WINDOW_INTERVALS,
                                                 2 * WINDOW_INTERVALS + ADJUST_PERIOD};
static const int phase_adjust[PHASE_COUNT] = {0, 1, -1};
static const int phase_error[PHASE_COUNT] = {0, -ADJUSTED_ERROR, ADJUSTED_ERROR};
static const int phase_skip[PHASE_COUNT] = {0, 1, 0};

in port p_off_mclk = XS1_PORT_1M;
clock clk_mclk = XS1_CLKBLK_1;

extern XUD_BusSpeed_t g_curUsbSpeed;

/* XUA_Buffer_Ep() writes each packet's length here. Normally set up by XUA_Buffer_Decouple(). */
extern unsigned g_aud_from_host_buffer;
unsigned packet_length;
extern XUD_ep_info ep_info[USB_MAX_NUM_EP];

void exit(int);

/* From lib_xud */
void SetupEndpoints(chanend c_ep_out[], int noEpOut, chanend c_ep_in[], int noEpIn, XUD_EpType epTypeTableOut[], XUD_EpType epTypeTableIn[]);

/* Signal reception of an OUT packet as XUD does (see XUD_GetBuffer_Finish()). Only the length is
 * used by XUA_Buffer_Ep(), so no data is written. */
static void send_packet(chanend c_ep, unsigned bytes)
{
    unsigned rx_bytes = bytes + 2; /* Includes CRC */
    ep_info[EP_NUM_AUD_OUT].actualPid = ep_info[EP_NUM_AUD_OUT].pid;
    unsigned tail_bits = (rx_bytes & 3) << 3;
    outuint(c_ep, rx_bytes >> 2);
    asm volatile("outt res[%0], %1" :: "r"(c_ep), "r"(tail_bits));
}

void fake_xud(chanend c_out[], chanend c_in[], chanend c_sof, chanend c_aud_ctl,
              chanend c_audio_rate_change, chanend c_phase)
{
    XUD_EpType epTypeTableOut[EP_COUNT_OUT] = {XUD_EPTYPE_CTL, XUD_EPTYPE_ISO};
    XUD_EpType epTypeTableIn[EP_COUNT_IN] = {XUD_EPTYPE_CTL};
    timer t;
    unsigned time;
    unsigned frame = 0;

    SetupEndpoints(c_out, EP_COUNT_OUT, c_in, EP_COUNT_IN, epTypeTableOut, epTypeTableIn);

    /* Playback stream start, as from Endpoint 0 */
    outct(c_aud_ctl, XUA_AUDCTL_SET_STREAM_OUTPUT_START);
    outuint(c_aud_ctl, 1);                  /* Data format (unused) */
    outuint(c_aud_ctl, NUM_CHANS);
    outuint(c_aud_ctl, SUBSLOT_BYTES);
    outuint(c_aud_ctl, SUBSLOT_BYTES * 8);  /* Resolution (unused) */

    t :> time;

    for (int phase = 0; phase < PHASE_COUNT; phase++)
    {
        c_phase <: phase;

        if (phase == PHASE_SLOW)
        {
            /* MCLK rate change, as from the audio hub. Restarts the SW PLL. */
            int ack;
            c_audio_rate_change <: MCLK_48;
            c_audio_rate_change <: SAMPLE_FREQ;
            c_audio_rate_change :> ack;
        }

        for (int interval = 0; interval < phase_intervals[phase]; interval++)
        {
            for (int f = 0; f < FRAMES_PER_INTERVAL; f++)
            {
                time += FRAME_TICKS;
                t when timerafter(time) :> void;
                outuint(c_sof, frame++);

                unsigned samples = SAMPLES_PER_FRAME;
                if ((f == 0) && ((interval % ADJUST_PERIOD) == 0))
                {
                    samples += phase_adjust[phase];
                }

                t when timerafter(time + PACKET_DELAY_TICKS) :> void;
                if (!((phase == PHASE_FAST) && (interval == DROP_INTERVAL) && (f == 0)))
                {
                    send_packet(c_out[EP_NUM_AUD_OUT], samples * NUM_CHANS * SUBSLOT_BYTES);
                }
            }
        }
    }

    c_phase <: (int) PHASE_COUNT;
}

/* Stands in for sw_pll_task(), checking the errors sent to it */
void checker(chanend c_sw_pll, chanend c_phase)
{
    int phase = -1;
    int reports = 0;
    int checked = 0;
    int restarts = 0;
    int fail = 0;
    unsigned word;

    while (1)
    {
        select
        {
            case inuint_byref(c_sw_pll, word):
                chkct(c_sw_pll, XS1_CT_END);
                if (word == DISABLE_SDM)
                {
                    unsigned mclk_rate = inuint(c_sw_pll);
                    chkct(c_sw_pll, XS1_CT_END);
                    if (mclk_rate != MCLK_48)
                    {
                        printstr("Bad MCLK rate on PLL restart: ");
                        printintln(mclk_rate);
                        fail = 1;
                    }
                    restarts++;

                    /* PLL ready */
                    outuint(c_sw_pll, 0);
                    outct(c_sw_pll, XS1_CT_END);
                }
                else if (phase < 0)
                {
                    printstrln("Error report before stream start");
                    fail = 1;
                }
                else if (reports++ >= phase_skip[phase])
                {
                    int error = (int) word;
                    checked++;
                    if (abs(error - phase_error[phase]) > ERROR_TOLERANCE)
                    {
                        printstr("Phase ");
                        printint(phase);
                        printstr(": expected error ");
                        printint(phase_error[phase]);
                        printstr(", got ");
                        printintln(error);
                        fail = 1;
                    }
                }
                break;

            case c_phase :> int new_phase:
                if ((phase >= 0) && (checked < MIN_CHECKED_REPORTS))
                {
                    printstr("Phase ");
                    printint(phase);
                    printstr(": too few error reports: ");
                    printintln(checked);
                    fail = 1;
                }

                if (new_phase == PHASE_COUNT)
                {
                    /* Startup and one rate change */
                    if (restarts != 2)
                    {
                        printstr("Expected 2 PLL restarts, got ");
                        printintln(restarts);
                        fail = 1;
                    }
                    if (!fail)
                    {
                        printstrln("PASS");
                    }
                    exit(0);
                }

                phase = new_phase;
                reports = 0;
                checked = 0;
                break;
        }
    }
}

int main()
{
    chan c_out[EP_COUNT_OUT];
    chan c_in[EP_COUNT_IN];
    chan c_sof;
    chan c_aud_ctl;
    chan c_audio_rate_change;
    chan c_sw_pll;
    chan c_phase;

    par
    {
        {
            g_curUsbSpeed = BUS_SPEED;
            SET_SHARED_GLOBAL(g_aud_from_host_buffer, array_to_xc_ptr((packet_length, unsigned[])));

            configure_clock_ref(clk_mclk, MCLK_PORT_DIV);
            configure_in_port(p_off_mclk, clk_mclk);
            start_clock(clk_mclk);

            XUA_Buffer_Ep(c_out[EP_NUM_AUD_OUT],    /* USB Audio Out */
                          c_sof, c_aud_ctl, p_off_mclk, c_audio_rate_change, c_sw_pll);
        }

        fake_xud(c_out, c_in, c_sof, c_aud_ctl, c_audio_rate_change, c_phase);

        checker(c_sw_pll, c_phase);
    }

    return 0;
}
