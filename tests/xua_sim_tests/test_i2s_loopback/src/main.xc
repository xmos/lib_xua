// Copyright 2016-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdlib.h>
#include <print.h>
#include <timer.h>
#include "xua.h"
#include "xua_commands.h" // Internal header, not part of lib_xua user API

#define DEBUG_UNIT MAIN
#include "debug_print.h"

/* Port declarations. Note, the defines come from the xn file */
#if I2S_WIRES_DAC > 0
on tile[AUDIO_IO_TILE] : buffered out port:32 p_i2s_dac[I2S_WIRES_DAC] =
                {PORT_I2S_DAC0,
#endif
#if I2S_WIRES_DAC > 1
                PORT_I2S_DAC1,
#endif
#if I2S_WIRES_DAC > 2
                PORT_I2S_DAC2,
#endif
#if I2S_WIRES_DAC > 3
                PORT_I2S_DAC3,
#endif
#if I2S_WIRES_DAC > 4
                PORT_I2S_DAC4,
#endif
#if I2S_WIRES_DAC > 5
                PORT_I2S_DAC5,
#endif
#if I2S_WIRES_DAC > 6
                PORT_I2S_DAC6,
#endif
#if I2S_WIRES_DAC > 7
#error I2S_WIRES_DAC value is too large!
#endif
#if I2S_WIRES_DAC > 0
                };
#endif

#if I2S_WIRES_ADC > 0
on tile[AUDIO_IO_TILE] : buffered in port:32 p_i2s_adc[I2S_WIRES_ADC] =
                {PORT_I2S_ADC0,
#endif
#if I2S_WIRES_ADC > 1
                PORT_I2S_ADC1,
#endif
#if I2S_WIRES_ADC > 2
                PORT_I2S_ADC2,
#endif
#if I2S_WIRES_ADC > 3
                PORT_I2S_ADC3,
#endif
#if I2S_WIRES_ADC > 4
                PORT_I2S_ADC4,
#endif
#if I2S_WIRES_ADC > 5
                PORT_I2S_ADC5,
#endif
#if I2S_WIRES_ADC > 6
                PORT_I2S_ADC6,
#endif
#if I2S_WIRES_ADC > 7
#error I2S_WIRES_ADC value is too large!
#endif
#if I2S_WIRES_ADC > 0
                };
#endif


#if CODEC_MASTER
buffered in port:32 p_lrclk         = PORT_I2S_LRCLK;
buffered in port:32 p_bclk          = PORT_I2S_BCLK;
#define p_mclk_in null
#else
buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S Bit-clock */
buffered out port:32 p_bclk         = PORT_I2S_BCLK;     /* I2S L/R-clock */
in port p_mclk_in                   = PORT_MCLK_IN;
#endif

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[AUDIO_IO_TILE]: XS1_CLKBLK_1;   /* Bit clock */
clock clk_audio_mclk                = on tile[AUDIO_IO_TILE]: XS1_CLKBLK_2;   /* Master clock */

#ifdef SIMULATION
#define INITIAL_SKIP_FRAMES 10
#define TOTAL_TEST_FRAMES 100
#else
#define INITIAL_SKIP_FRAMES 1000
#define TOTAL_TEST_FRAMES (5 * DEFAULT_FREQ)
#endif

#define SHIFT (16) /* Note, we shift samples up such that we can test down to 16bit I2S */
#define SAMPLE(frame_count, channel_num) ((((frame_count) << 8) | ((channel_num) & 0xFF))<<SHIFT)
#define SAMPLE_FRAME_NUM(test_word) ((test_word>>SHIFT) >> 8)
#define SAMPLE_CHANNEL_NUM(test_word) ((test_word>>SHIFT) & 0xFF)

void generator(chanend c_checker, chanend c_out)
{
  unsigned frame_count = 0;
  int underflow_word;
  int fail = 1;
  int i;
  unsigned time;

  while (1) {
    underflow_word = inuint(c_out);

#pragma loop unroll
    for (i = 0; i < NUM_USB_CHAN_OUT; i++) {
      outuint(c_out, SAMPLE(frame_count, i));
    }

    fail = inuint(c_checker);

#pragma loop unroll
    for (i = 0; i < NUM_USB_CHAN_IN; i++) {
      outuint(c_checker, inuint(c_out));
    }


    if (frame_count == TOTAL_TEST_FRAMES) {
      if (!fail) {
        debug_printf("PASS\n");
      }
      outct(c_out, XUA_AUDCTL_SET_SAMPLE_FREQ);
      //inuint(c_out); //This causes the DFUhandler to be called with exceptiopn in slave mode so skip this - we are out of here anyhow

    /* Give some time for AudioHub() to react to the command to stop accessing ports otherwise exit()
     * will trap in the port destructors
     * Note, this is a bit of a cludge since ideally program would completely shutdown, however, AudioHub
     * has a while(1)
     */
      timer t;
      t :> time;
      t when timerafter(time + 1000) :> void;

      exit(0);
    }

    frame_count++;
  }
}

void checker(chanend c_checker, int disable)
{
  unsigned x[NUM_USB_CHAN_IN];
  int last_frame_number;
  unsigned frame_count;
  int fail;
  int i;

  if (disable)
    debug_printf("checker disabled\n");

  /*debug_printf("%s %d/%d %d\n",
    I2S_MODE_TDM ? "TDM" : "I2S", NUM_USB_CHAN_IN, NUM_USB_CHAN_OUT, DEFAULT_FREQ);*/

  fail = 0;
  frame_count = 0;
  last_frame_number = -1;

  while (1) {
    outuint(c_checker, fail);

#pragma loop unroll
    for (i = 0; i < NUM_USB_CHAN_IN; i++) {
      x[i] = inuint(c_checker);
    }

    if (frame_count > INITIAL_SKIP_FRAMES) {
      // check that frame number is incrementing
      if (!disable && SAMPLE_FRAME_NUM(x[0]) != last_frame_number + 1) {
        debug_printf("%d: 0x%x (%d)\n", frame_count, x[0], last_frame_number);
        fail = 1;
      }

      for (i = 0; i < NUM_USB_CHAN_IN; i++) {
        // check channel numbers are 0 to N-1 in a frame
        if (!disable && SAMPLE_CHANNEL_NUM(x[i]) != i) {
          debug_printf("%d,%d: 0x%x\n", frame_count, i, x[i]);
          fail = 1;
        }

        // check frame number doesn't change in a frame
        if (!disable && SAMPLE_FRAME_NUM(x[i]) != SAMPLE_FRAME_NUM(x[0])) {
          debug_printf("%d,%d: 0x%x (0x%x)\n", frame_count, i, x[i], x[0]);
          fail = 1;
        }
      }
    }

    last_frame_number = SAMPLE_FRAME_NUM(x[0]);
    frame_count++;
  }
}

#ifdef SIMULATION

out port p_mclk_gen       = on tile[AUDIO_IO_TILE] :  XS1_PORT_1A;
clock clk_audio_mclk_gen  = on tile[AUDIO_IO_TILE] : XS1_CLKBLK_3;
in port p_for_mclk_count  = on tile[XUD_TILE] : XS1_PORT_16A;
void master_mode_clk_setup(void);

#if CODEC_MASTER
out port  p_bclk_gen      = on tile[AUDIO_IO_TILE] : XS1_PORT_1B;
clock clk_audio_bclk_gen  = on tile[AUDIO_IO_TILE] : XS1_CLKBLK_4;
out port  p_lrclk_gen     = on tile[AUDIO_IO_TILE] : XS1_PORT_1C;
clock clk_audio_lrclk_gen = on tile[AUDIO_IO_TILE] : XS1_CLKBLK_5;
void slave_mode_clk_setup(const unsigned samFreq, const unsigned chans_per_frame);
#endif


void mclk_checker(void)
{
  if(AUDIO_IO_TILE == XUD_TILE)
  {
    int x;
    asm("ldw %0, dp[clk_audio_mclk]":"=r"(x));
    asm("setclk res[%0], %1"::"r"(p_for_mclk_count), "r"(x));

    unsigned c0, c1;
    // startup value of port timer
    delay_microseconds(5); // Wait for I2S to start the clock
    p_for_mclk_count :> void @ c0;
    delay_microseconds(10); // Wait for port timer to increment a bit more. Should be just over 25 ticks per microsecond, so 250 + a few instruction times

    // Input will fail if not clocked so have timeout
    timer t;
    int timeout;
    t :> timeout;
    timeout += XS1_TIMER_MHZ; // 1 us in the future

    select{
      case t when timerafter(timeout) :> int _: 
        debug_printf("TIMEOUT in mclk_checker - MCLK port could not input due to no clock\n");
        break;

      case p_for_mclk_count :> void @ c1:
        if( ((c1 - c0) > 270) || ((c1 - c0) < 230) ){
          debug_printf("mclk_checker error - MCLK port could input but timer not incrementing at correct rate. Expecting ~250 for 25MHz but got %d\n", (c1 - c0));
        }
        break;
    }
    // All good, print nothing
  }
}

#endif // SIM

#if (XUA_PCM_FORMAT == XUA_PCM_FORMAT_TDM)
const int i2s_tdm_mode = 8;
#else
const int i2s_tdm_mode = 2;
#endif

int main(void)
{
    chan c_checker;
    chan c_out;

    par
    {
        on tile[AUDIO_IO_TILE]:
        {
            par
            {
                XUA_AudioHub(c_out, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc);
                generator(c_checker, c_out);
                checker(c_checker, 0);
#ifdef SIMULATION
#if CODEC_MASTER
                slave_mode_clk_setup(DEFAULT_FREQ, i2s_tdm_mode);
#else
                master_mode_clk_setup();
#endif
#endif
            }
        }
#ifdef SIMULATION
        on tile[XUD_TILE]: mclk_checker();
#endif
    }

    return 0;
}
