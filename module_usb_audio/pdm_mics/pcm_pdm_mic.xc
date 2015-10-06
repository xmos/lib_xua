/* This file includes an integration of lib_array_mic into USB Audio */

#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <xclib.h>
//#include "debug_print.h"
#include "devicedefines.h"
#include "mic_array.h"
#include "mic_array_board_support.h"

#if 1
in port p_pdm_clk             = PORT_PDM_CLK;
in port p_pdm_mics = PORT_PDM_DATA;

in port p_mclk                 = PORT_PDM_MCLK;
clock mclk                     = on tile[PDM_TILE]: XS1_CLKBLK_1;
clock pdmclk                   = on tile[PDM_TILE]: XS1_CLKBLK_3;


on tile[0]:p_leds leds = DEFAULT_INIT;


// LEDs
//out port p_led0to7              = on tile[0]: XS1_PORT_8C;
//out port p_led8                 = on tile[0]: XS1_PORT_1K;
//out port p_led9                 = on tile[0]: XS1_PORT_1L;
//out port p_led10to12            = on tile[0]: XS1_PORT_8D;
//out port p_leds_oen             = on tile[0]: XS1_PORT_1P;
// Buttons
in port p_buttons               = on tile[0]: XS1_PORT_4A;

enum buttons
{
  BUTTON_A=1<<0,
  BUTTON_B=1<<1,
  BUTTON_C=1<<2,
  BUTTON_D=1<<3
};

#if 0
void lightLeds(int only_one_mic)
{
    if(only_one_mic)
    {
        p_led10to12 <: 0x3;
        p_led0to7 <: 0xff;
        p_led8 <: 1;
        p_led9 <: 1;
    }
    else
    {
        p_led0to7 <: 0;
        p_led10to12 <: 0x4;
        p_led8 <: 0;
        p_led9 <: 0;
    }
}
#endif

#if 0
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


  lightLeds(only_one_mic);
  only_one_mic? printstrln("one"):printstrln("all");
     
  level[0] = 0xff;
  while (1) {
    select
    {

      case buttons_active => p_buttons when pinsneq(button_val) :> unsigned new_button_val:

        if BUTTON_PRESSED(BUTTON_A, button_val, new_button_val) {
          debug_printf("Button A\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");

          lightLeds(only_one_mic);
          
          c <: 0;
          buttons_active = 0;
        }
        if BUTTON_PRESSED(BUTTON_B, button_val, new_button_val) {
          debug_printf("Button B\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");
          lightLeds(only_one_mic);
          c <: 0;
          buttons_active = 0;
        }
        if BUTTON_PRESSED(BUTTON_C, button_val, new_button_val) {
          debug_printf("Button C\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");
          c <: 0;
          buttons_active = 0;
        }
        if BUTTON_PRESSED(BUTTON_D, button_val, new_button_val) {
          debug_printf("Button D\n");
          only_one_mic = 1-only_one_mic;
          only_one_mic? printstrln("one"):printstrln("all");
          lightLeds(only_one_mic);
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
#endif


static int dc_offset_removal(int sample, int &prex_x, int &prev_y){
    int r =  prev_y- (prev_y>>5) + (sample - prex_x);
    prex_x = sample;
    prev_y = r;
    return r;
}

#if 0
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

            int a = dc_offset_removal( audio[buffer].data[0][0].ch_a, prev_x[0], prev_y[0]);
            int b = dc_offset_removal( audio[buffer].data[0][0].ch_b, prev_x[0], prev_y[0]);
            int c = dc_offset_removal( audio[buffer].data[1][0].ch_a, prev_x[1], prev_y[1]);
            int d = dc_offset_removal( audio[buffer].data[1][0].ch_b, prev_x[2], prev_y[2]);
            int e = dc_offset_removal( audio[buffer].data[2][0].ch_a, prev_x[3], prev_y[3]);
            int f = dc_offset_removal( audio[buffer].data[2][0].ch_b, prev_x[4], prev_y[4]);
            int g = dc_offset_removal( audio[buffer].data[3][0].ch_a, prev_x[5], prev_y[5]);
            int h = dc_offset_removal( audio[buffer].data[3][0].ch_b, prev_x[6], prev_y[6]);//Expect dead

            //printf("%x %x %x %x %x %x %x %x\n", a, b, c, d, e, f, g, h);

            unsigned v = a*a;

            select {
                case cc:> int:{
                    only_one_mic = 1-only_one_mic;
                    break;
                }
                default:break;
            }

#define GAIN 5

            if((-a) > max) max = (-a);
            if(a > max) max = a;
            int output;
            if(only_one_mic){
                output = a<<GAIN;
                c_pcm_out :> unsigned req;
                c_pcm_out <: output;
                c_pcm_out <: output;
                c_pcm_out <: output;
                c_pcm_out <: output;
                c_pcm_out <: output;
                c_pcm_out <: output;
                c_pcm_out <: output;
                c_pcm_out <: output;


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
                if((-g) > max) max = (-g);
                if(g > max) max = g;
                output = a+b+c+d+e+f+g+g;
                output >>=3;
                output = output<<(GAIN);
                c_pcm_out :> unsigned req;
                c_pcm_out <: output;
                c_pcm_out <: a << GAIN;
                c_pcm_out <: b << GAIN;
                c_pcm_out <: c << GAIN;
                c_pcm_out <: d << GAIN;
                c_pcm_out <: e << GAIN;
                c_pcm_out <: f << GAIN;
                c_pcm_out <: g << GAIN;
           
            }

            max = max - (max>>17);

            
        }
    }
}
#endif

static void set_dir(client interface led_button_if lb, unsigned dir){

    for(unsigned i=0;i<13;i++)
        lb.set_led_brightness(i, 0);
    switch(dir){
    case 0:{
        lb.set_led_brightness(0, 255);
        lb.set_led_brightness(1, 255);
        break;
    }
    case 1:{
        lb.set_led_brightness(2, 255);
        lb.set_led_brightness(3, 255);
        break;
    }
    case 2:{
        lb.set_led_brightness(4, 255);
        lb.set_led_brightness(5, 255);
        break;
    }
    case 3:{
        lb.set_led_brightness(6, 255);
        lb.set_led_brightness(7, 255);
        break;
    }
    case 4:{
        lb.set_led_brightness(8, 255);
        lb.set_led_brightness(9, 255);
        break;
    }
    case 5:{
        lb.set_led_brightness(10, 255);
        lb.set_led_brightness(11, 255);
        break;
    }
    }
}



void lores_DAS_fixed(streaming chanend c_ds_output_0, streaming chanend c_ds_output_1,
        client interface led_button_if lb, chanend c_audio){

    unsigned buffer = 1;     //buffer index
    frame_audio audio[2];    //double buffered
    memset(audio, sizeof(frame_audio), 0);

#define MAX_DELAY 128

    unsigned delay = 6;
    int delay_buffer[MAX_DELAY][7];
    memset(delay_buffer, sizeof(int)*8*8, 0);
    unsigned delay_head = 0;
    unsigned dir = 0;
    set_dir(lb, dir);

    unsafe{
        c_ds_output_0 <: (frame_audio * unsafe)audio[0].data[0];
        c_ds_output_1 <: (frame_audio * unsafe)audio[0].data[4];

        while(1){

            schkct(c_ds_output_0, 8);
            schkct(c_ds_output_1, 8);

            c_ds_output_0 <: (frame_audio * unsafe)audio[buffer].data[0];
            c_ds_output_1 <: (frame_audio * unsafe)audio[buffer].data[4];

            buffer = 1 - buffer;

            //copy the current sample to the delay buffer
            for(unsigned i=0;i<7;i++)
                delay_buffer[delay_head][i] = audio[buffer].data[i][0];

            //light the LED for the current direction

            int t;

            select {
                case lb.button_event():{
                    unsigned button;
                    e_button_state pressed;
                    lb.get_button_event(button, pressed);
                    if(pressed == BUTTON_PRESSED){
                        switch(button){
                        case 0:{
                            dir--;
                            if(dir == -1)
                                dir = 5;
                            break;
                        }
                        case 1:{
                            if(delay +1 < MAX_DELAY){
                                delay++;
                                printf("n: %d\n", delay);
                            }
                            break;
                        }
                        case 2:{
                            if(delay > 0){
                                delay--;
                                printf("n: %d\n", delay);
                            }
                            break;
                        }
                        case 3:{
                            dir++;
                            if(dir == 6)
                                dir = 0;
                            break;
                        }
                        }
                        set_dir(lb, dir);
                    }
                    break;
                }
                default:break;
            }
#if 1
            int output = - 2*delay_buffer[(delay_head-delay)%MAX_DELAY][0];
            switch(dir){
            case 0:
                output = delay_buffer[delay_head][1] + delay_buffer[(delay_head-2*delay)%MAX_DELAY][4];
                break;
            case 1:
                output = delay_buffer[delay_head][2] + delay_buffer[(delay_head-2*delay)%MAX_DELAY][5];
                break;
            case 2:
                output = delay_buffer[delay_head][3] + delay_buffer[(delay_head-2*delay)%MAX_DELAY][6];
                break;
            case 3:
                output = delay_buffer[delay_head][4] + delay_buffer[(delay_head-2*delay)%MAX_DELAY][1];
                break;
            case 4:
                output = delay_buffer[delay_head][5] + delay_buffer[(delay_head-2*delay)%MAX_DELAY][2];
                break;
            case 5:
                output = delay_buffer[delay_head][6] + delay_buffer[(delay_head-2*delay)%MAX_DELAY][3];
                break;
            }
            c_audio <: output<<2;
            c_audio <: output<<2;
#else
            int output = 0;
            for(unsigned i=1;i<6;i++){
                output += audio[buffer].data[i][0];
            }
            c_audio <: output;
            c_audio <: output;
#endif
            delay_head++;
            delay_head%=MAX_DELAY;
        }
    }
}



void pcm_pdm_mic(chanend c_pcm_out)
{
    streaming chan c_multi_channel_pdm, c_sync, c_4x_pdm_mic_0, c_4x_pdm_mic_1;
    streaming chan c_ds_output_0, c_ds_output_1;
    streaming chan c_buffer_mic0, c_buffer_mic1;
    
    interface led_button_if lb;
    
    configure_clock_src(mclk, p_mclk);
    configure_clock_src_divide(pdmclk, p_mclk, 2);
    configure_port_clock_output(p_pdm_clk, pdmclk);
    configure_in_port(p_pdm_mics, pdmclk);
    start_clock(mclk);
    start_clock(pdmclk);

    decimator_config dc = {0, 1, 0, 0};
    
    unsafe
    {
        par 
        {
                button_and_led_server(lb, leds, p_buttons);
                pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);
                decimate_to_pcm_4ch_48KHz(c_4x_pdm_mic_0, c_ds_output_0, dc);
                decimate_to_pcm_4ch_48KHz(c_4x_pdm_mic_1, c_ds_output_1, dc);
                lores_DAS_fixed(c_ds_output_0, c_ds_output_1, lb, c_pcm_out);
        }

    }
}

#endif
