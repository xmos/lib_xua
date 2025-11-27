// Copyright 2016-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdlib.h>
#include <stdio.h>
#include <xclib.h>
#include <timer.h>
#include "xua.h"
#include "xua_commands.h" // Internal header, not part of lib_xua user API

#define DEBUG_UNIT MAIN
#include "debug_print.h"

/* For talking to the checker plugin */
out port setup_strobe_port = on tile[0]: XS1_PORT_1L;
out port setup_data_port = on tile[0]: XS1_PORT_16A;
in port  setup_resp_port = on tile[0]: XS1_PORT_1M;

static void send_data_to_tester(
        out port setup_strobe_port,
        out port setup_data_port,
        unsigned data){
    setup_data_port <: data;
    sync(setup_data_port);
    setup_strobe_port <: 1;
    setup_strobe_port <: 0;
    sync(setup_strobe_port);
}

static void broadcast(unsigned bclk_freq, unsigned num_in, unsigned num_out, int is_i2s_justified, unsigned data_bits, unsigned num_frames){
    setup_strobe_port <: 0;

    send_data_to_tester(setup_strobe_port, setup_data_port, bclk_freq>>16);
    send_data_to_tester(setup_strobe_port, setup_data_port, bclk_freq);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_in);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_out);
    send_data_to_tester(setup_strobe_port, setup_data_port, is_i2s_justified);
    send_data_to_tester(setup_strobe_port, setup_data_port, data_bits);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_frames);
 }

static int request_response(
        out port setup_strobe_port,
        in port setup_resp_port){
    int r = 0;
    while(!r) {
        setup_resp_port :> r;
    }
    setup_strobe_port <: 1;
    setup_strobe_port <: 0;
    setup_resp_port :> r;
    return r;
}

static int tester_finished(in port setup_resp_port)
{
    int r;
    setup_resp_port :> r;
    return r;
}


#if I2S_WIRES_ADC > 0
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered in port:32 p_i2s_adc[I2S_WIRES_ADC] =
                {XS1_PORT_1D,
#endif
#if I2S_WIRES_ADC > 1
                XS1_PORT_1E,
#endif
#if I2S_WIRES_ADC > 2
                XS1_PORT_1F,
#endif
#if I2S_WIRES_ADC > 3
                XS1_PORT_1G,
#endif
#if I2S_WIRES_ADC > 4
#error I2S_WIRES_ADC value is too large!
#endif
#if I2S_WIRES_ADC > 0
                };
#endif
#if I2S_WIRES_ADC == 0
#define p_i2s_adc null
#endif


#if I2S_WIRES_DAC > 0
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered out port:32 p_i2s_dac[I2S_WIRES_DAC] =
                {XS1_PORT_1H,
#endif
#if I2S_WIRES_DAC > 1
                XS1_PORT_1I,
#endif
#if I2S_WIRES_DAC > 2
                XS1_PORT_1J,
#endif
#if I2S_WIRES_DAC > 3
                XS1_PORT_1K,
#endif
#if I2S_WIRES_DAC > 4
#error I2S_WIRES_DAC value is too large!
#endif
#if I2S_WIRES_DAC > 0
                };
#endif
#if I2S_WIRES_DAC == 0
#define p_i2s_dac null
#endif

// Overridable configs
#ifndef BACKPRESSURE_TICKS_START
#define BACKPRESSURE_TICKS_START    0
#endif
#ifndef BACKPRESSURE_TICKS_STEP
#define BACKPRESSURE_TICKS_STEP     0
#endif
#ifndef TOTAL_TEST_FRAMES
#define TOTAL_TEST_FRAMES           14
#endif


buffered in port:32 p_bclk          = on tile[0] : XS1_PORT_1B;
buffered in port:32 p_lrclk         = on tile[0] : XS1_PORT_1C;
#define p_mclk_in null


/* Clock-block declarations */
clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_1;   /* Bit clock */
clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_2;   /* Master clock */


#define SHIFT (32 - XUA_I2S_N_BITS) /* Note, we shift samples up such that we can test down to 16bit I2S */
#define SAMPLE(frame_count, channel_num) ((((frame_count << 8) | (channel_num & 0xFF))<<SHIFT) | 0x00000000) 

int generator_ready = 0;
unsigned frame_count;
unsafe{
  volatile int * unsafe rdy_flag = &generator_ready;
  volatile unsigned * unsafe frame_count_ptr = &frame_count;
}

// Pretend to be decouple
void generator(chanend c_out)
{
  int underflow_word;
  unsigned bclk_freq = DEFAULT_FREQ * XUA_I2S_N_BITS * 2; // TODO support TDM

  printf("Broadcast: %u %u %u %u %u %u\n", bclk_freq, I2S_WIRES_ADC, I2S_WIRES_DAC, XUA_PCM_FORMAT!=XUA_PCM_FORMAT_TDM, XUA_I2S_N_BITS, TOTAL_TEST_FRAMES);
  broadcast(bclk_freq, I2S_WIRES_ADC, I2S_WIRES_DAC, XUA_PCM_FORMAT==XUA_PCM_FORMAT_I2S, XUA_I2S_N_BITS, TOTAL_TEST_FRAMES);
  unsafe{*rdy_flag = 1;}

  if(BACKPRESSURE_TICKS_START > 0 || BACKPRESSURE_TICKS_STEP > 0){
    printf("BACKPRESSURE_TICKS_START: %d (frame time %d) BACKPRESSURE_TICKS_STEP: %d\n",
            BACKPRESSURE_TICKS_START,
            (XS1_TIMER_HZ / DEFAULT_FREQ),
            BACKPRESSURE_TICKS_STEP);
  }

  const int skip_frames_rx = 2; // How many frames it takes before we get data back in. Used for initial offset
  unsigned received_values[TOTAL_TEST_FRAMES][I2S_CHANS_ADC] = {{0}};

  printf("DUT start\n");

  unsigned final_frame_count = 0;
  for(frame_count = 0; frame_count < TOTAL_TEST_FRAMES; frame_count++) {
    underflow_word = inuint(c_out);

    for (unsigned i = 0; i < I2S_CHANS_DAC; i++) {
      outuint(c_out, SAMPLE(frame_count, i));
    }
    for (unsigned i = 0; i < I2S_CHANS_ADC; i++) {
      received_values[frame_count][i] = inuint(c_out);
    }
    // Add backpressure
    delay_ticks(BACKPRESSURE_TICKS_START + BACKPRESSURE_TICKS_STEP * frame_count);
    if(tester_finished(setup_resp_port)){
        final_frame_count =  frame_count;
        break;
    }
  }
  printf("DUT Samples all sent\n");


  // Check received
  for(frame_count = 0; frame_count < final_frame_count; frame_count++) {
    for (unsigned i = 0; i < I2S_CHANS_ADC; i++) {
      int expected = frame_count < skip_frames_rx ? 0 : SAMPLE(frame_count - skip_frames_rx, i);
      if(received_values[frame_count][i] != expected){
        printf("ADC unexpected frame %u ch %u: 0x%x (0x%x)\n", frame_count, i, received_values[frame_count][i], expected);
      }
    }
  } 

  int error_from_harness = request_response(setup_strobe_port, setup_resp_port);
  printf("DUT Finished: %d\n", error_from_harness);

  // Audiohub never exits so kill all
  _Exit(error_from_harness);
}



void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC)
{
  printf("AudioHwConfig\n");
  unsafe{while(*rdy_flag == 0);}
}

void AudioHwInit()
{
  // nothing
}


extern unsigned syncError;
unsafe{
  volatile unsigned * unsafe syncError_ptr = &syncError;
}
void syncError_monitor(void){
  unsafe{
    unsigned old_syncError = 0;
    timer tmr;
    unsigned time;
    while(1){
      if(*syncError_ptr != old_syncError){
        old_syncError = *syncError_ptr; // Have to grab this first because may be cleared after print
        tmr :> time;
        if(*syncError_ptr){
          printf("DUT syncError detected at frame %u time: %u\n", *frame_count_ptr, time);
        } else {
          printf("DUT syncError cleared at frame %u time: %u\n", *frame_count_ptr, time);
        }
      }
    }
  }
}


int main(void)
{
    chan c_out;

    par
    {
        on tile[0]:
        {
            par
            {
                XUA_AudioHub(c_out, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc);
                generator(c_out);
                syncError_monitor();
            }
        }
    }

    return 0;
}
