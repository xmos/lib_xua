#include <xscope.h>
#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include "beam.h"
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <xclib.h>
#include "static_constants.h"
#include "debug_print.h"

#if 1
on tile[0]: in port p_pdm_clk = XS1_PORT_1E;
on tile[0]: in buffered port:8 p_pdm_mics = XS1_PORT_8B;
in port p_mclk                 = on tile[0]: XS1_PORT_1F;
clock mclk                     = on tile[0]: XS1_CLKBLK_1;
clock pdmclk                   = on tile[0]: XS1_CLKBLK_3;


// LEDs
out port p_led0to7              = on tile[0]: XS1_PORT_8C;
out port p_led8                 = on tile[0]: XS1_PORT_1K;
out port p_led9                 = on tile[0]: XS1_PORT_1L;
out port p_led10to12            = on tile[0]: XS1_PORT_8D;
out port p_leds_oen             = on tile[0]: XS1_PORT_1P;
// Buttons
in port p_buttons               = on tile[0]: XS1_PORT_4A;

enum buttons
{
  BUTTON_A=1<<0,
  BUTTON_B=1<<1,
  BUTTON_C=1<<2,
  BUTTON_D=1<<3
};

#define BUTTON_PRESSED(but_mask, old_val, new_val) (((old_val) & (but_mask)) == (but_mask) && ((new_val) & (but_mask)) == 0)
#define BUTTON_DEBOUNCE_DELAY (20000000)
#define LED_ON 0xFFFF
void buttons_and_leds(chanend c)
{
  int button_val;
  int buttons_active = 1;
  unsigned buttons_timeout;
  unsigned time;
  unsigned glow_time;
  timer button_tmr;
  timer leds_tmr;
  timer glow_tmr;
  const int pwm_cycle = 100000; // The period in 100Mhz timer ticks of the pwm
  const int pwm_res = 256; // The resolution of the pwm
  const int pwm_delay = pwm_cycle / pwm_res; // The period between updates to the port output
  int count = 0; // The count that tracks where we are in the pwm cycle

  int period = 1 * 1000 * 1000 * 100 * 15; // period from off to on = 1s;
  unsigned res = 300;                  // increment the brightness in this
                                      // number of steps
  int delay = period / res;           // how long to wait between updates
  // int delay = 1 * 1000 * 1000 * 100;
  int dir = 1;
  int on_led = 0;

  p_leds_oen <: 1;
  p_leds_oen <: 0;
  // This array stores the pwm levels for the leds
  int level[13] = {0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (int i=0; i < 13; i++) {
    level[i] = level[i] / (0xFFFF / pwm_res);
  }

  p_buttons :> button_val;
  leds_tmr :> time;
  glow_tmr :> glow_time;

  int only_one_mic = 1;

  p_led0to7 <:~0;
  p_led8  <:~0;
  p_led9   <:~0;
  //p_led10to12   <:~0;
//  p_leds_oen  <:~0;


  p_led10to12 <: ~((only_one_mic&0x1)<<2);
  level[0] = 0xff;
  while (1) {
    select
    {

      case buttons_active => p_buttons when pinsneq(button_val) :> unsigned new_button_val:

        if BUTTON_PRESSED(BUTTON_A, button_val, new_button_val) {
          debug_printf("Button A\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");
          p_led10to12 <: ~((only_one_mic&0x1)<<2);
          c <: 0;
          buttons_active = 0;
        }
        if BUTTON_PRESSED(BUTTON_B, button_val, new_button_val) {
          debug_printf("Button B\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");
          p_led10to12 <: ~((only_one_mic&0x1)<<2);
          c <: 0;
          buttons_active = 0;
        }
        if BUTTON_PRESSED(BUTTON_C, button_val, new_button_val) {
          debug_printf("Button C\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");
          p_led10to12 <: ~((only_one_mic&0x1)<<2);
          c <: 0;
          buttons_active = 0;
        }
        if BUTTON_PRESSED(BUTTON_D, button_val, new_button_val) {
          debug_printf("Button D\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");
          p_led10to12 <: ~((only_one_mic&0x1)<<2);
          c <: 0;
          buttons_active = 0;
        }
        if (!buttons_active)
        {
          button_tmr :> buttons_timeout;
          buttons_timeout += BUTTON_DEBOUNCE_DELAY;
        }
        button_val = new_button_val;
        break;
      case !buttons_active => button_tmr when timerafter(buttons_timeout) :> void:
        buttons_active = 1;
        p_buttons :> button_val;
        break;


    }
  }
}


typedef struct {
    unsigned ch_a;
    unsigned ch_b;
} double_packed_audio;

typedef struct {
    double_packed_audio data[4][1<<FRAME_SIZE_LOG2];
} synchronised_audio;


static int dc_offset_removal(int sample, int &prex_x, int &prev_y){
    int r =  prev_y- (prev_y>>5) + (sample - prex_x);
    prex_x = sample;
    prev_y = r;
    return r;
}

void example(streaming chanend c_ds_output_0, streaming chanend c_ds_output_1, streaming chanend c_pcm_out, chanend cc)
{


    unsigned buffer = 1;            //buffer index
    synchronised_audio audio[2];    //double buffered
    memset(audio, sizeof(synchronised_audio), 2);

    int     prev_x[7] = {0};
    int     prev_y[7] = {0};

    int max = 0;

    unsafe
    {
        c_ds_output_0 <: (synchronised_audio * unsafe)audio[0].data[0];
        c_ds_output_1 <: (synchronised_audio * unsafe)audio[0].data[2];

        int only_one_mic=1;
        while(1)
        {   

            schkct(c_ds_output_0, 8);
            schkct(c_ds_output_1, 8);

            c_ds_output_0 <: (synchronised_audio * unsafe)audio[buffer].data[0];
            c_ds_output_1 <: (synchronised_audio * unsafe)audio[buffer].data[2];

            buffer = 1 - buffer;

            // audio[buffer] is good to go

            int a = dc_offset_removal( audio[buffer].data[0][0].ch_b, prev_x[0], prev_y[0]);
            int b = dc_offset_removal( audio[buffer].data[1][0].ch_a, prev_x[1], prev_y[1]);
            int c = dc_offset_removal( audio[buffer].data[1][0].ch_b, prev_x[2], prev_y[2]);
            int d = dc_offset_removal( audio[buffer].data[2][0].ch_a, prev_x[3], prev_y[3]);
            int e = dc_offset_removal( audio[buffer].data[2][0].ch_b, prev_x[4], prev_y[4]);
            int f = dc_offset_removal( audio[buffer].data[3][0].ch_a, prev_x[5], prev_y[5]);
            int g = dc_offset_removal( audio[buffer].data[3][0].ch_b, prev_x[6], prev_y[6]);

           // printf("%x %x %x %x %x %x %x\n", a, b, c, d, e, f, g);

            unsigned v = a*a;


            select {
                case cc:> int:{
                    only_one_mic = 1-only_one_mic;
                    break;
                }
                default:break;
            }

            if((-a) > max) max = (-a);
            if(a > max) max = a;
            int output;
            if(only_one_mic){
                output = a<<(clz(max)-1);
            } else {
                if((-a) > max) max = (-a);
                if(a > max) max = a;
                if((-b) > max) max = (-b);
                if(b > max) max = b;
                if((-c) > max) max = (-c);
                if(c > max) max = c;
                if((-d) > max) max = (-d);
                if(d > max) max = d;
                if((-e) > max) max = (-e);
                if(e > max) max = e;
                if((-f) > max) max = (-f);
                if(f > max) max = f;
                output = a+b+c+d+e+f+g+g;
                output >>=3;
                output = output<<(clz(max)-1);
            }

            max = max - (max>>17);

            c_pcm_out :> unsigned req;
            c_pcm_out <: output;
            c_pcm_out <: output;
        }
    }
}


void pcm_pdm_mic(streaming chanend c_pcm_out)
{
    streaming chan c_multi_channel_pdm, c_sync, c_4x_pdm_mic_0, c_4x_pdm_mic_1;
    streaming chan c_ds_output_0, c_ds_output_1;
    streaming chan c_buffer_mic0, c_buffer_mic1;
    unsigned long long shared_memory[2] = {0};

    configure_clock_src(mclk, p_mclk);
    configure_clock_src_divide(pdmclk, p_mclk, 2);
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(mclk);
    start_clock(pdmclk);

    chan c;
    par {
        buttons_and_leds(c);

        unsafe
        {
            unsigned long long * unsafe p_shared_memory = shared_memory;
            par
            {

                //Input stage
                pdm_first_stage(p_pdm_mics, p_shared_memory,
                                PDM_BUFFER_LENGTH_LOG2, c_sync,
                                c_4x_pdm_mic_0, c_4x_pdm_mic_1);

                pdm_to_pcm_4x(c_4x_pdm_mic_0, c_ds_output_0);
                        pdm_to_pcm_4x(c_4x_pdm_mic_1, c_ds_output_1);

                example(c_ds_output_0, c_ds_output_1, c_pcm_out, c);

            }
        }

    }
}

#endif
