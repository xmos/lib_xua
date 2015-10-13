/* This file includes an integration of lib_array_mic into USB Audio */

#include <platform.h>
#include <xs1.h>
#include <stdlib.h>
#include <print.h>
#include <stdio.h>
#include <string.h>
#include <xclib.h>
#include <stdint.h>
#include "devicedefines.h"

#include "fir_decimator.h"
#include "mic_array.h"
#include "mic_array_board_support.h"

#define FORM_BEAM 1

#if 1
in port p_pdm_clk                = PORT_PDM_CLK;
in buffered port:32 p_pdm_mics   = PORT_PDM_DATA;

in port p_mclk                  = PORT_PDM_MCLK;
clock mclk                      = on tile[PDM_TILE]: XS1_CLKBLK_1;
clock pdmclk                    = on tile[PDM_TILE]: XS1_CLKBLK_3;


on tile[0]:p_leds leds = DEFAULT_INIT;

// Buttons
in port p_buttons               = on tile[0]: XS1_PORT_4A;

enum buttons
{
  BUTTON_A=1<<0,
  BUTTON_B=1<<1,
  BUTTON_C=1<<2,
  BUTTON_D=1<<3
};


static const one_meter_thirty_degrees[6] = {0, 3, 8, 11, 8, 3};



static void set_dir(client interface led_button_if lb, unsigned dir, unsigned delay[]){

    for(unsigned i=0;i<13;i++)
        lb.set_led_brightness(i, 0);
    delay[0] = 5;
    for(unsigned i=0;i<6;i++)
        delay[i+1] = one_meter_thirty_degrees[(i - dir + 3 +6)%6];

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
        client interface led_button_if lb, chanend c_audio)
{

    unsigned buffer = 1;     //buffer index
    frame_audio audio[2];    //double buffered
    memset(audio, sizeof(frame_audio), 0);

    int omni = 0;

#define MAX_DELAY 128

    unsigned gain = 4096;
#ifdef FORM_BEAM
    unsigned delay[7] = {0, 0, 0, 0, 0, 0, 0};
    int delay_buffer[MAX_DELAY][7];
    memset(delay_buffer, sizeof(int)*8*8, 0);
    unsigned delay_head = 0;
    unsigned dir = 0;
    set_dir(lb, dir, delay);
#else
    int summed = 0;
    
    /* Light center LED, kill other LEDs */
    for(unsigned i=0;i<13;i++)
        lb.set_led_brightness(i, 0);
                                    
    lb.set_led_brightness(12, 255);

#endif
    unsafe
    {
        c_ds_output_0 <: (frame_audio * unsafe)audio[0].data[0];
        c_ds_output_1 <: (frame_audio * unsafe)audio[0].data[4];

        while(1){

            schkct(c_ds_output_0, 8);
            schkct(c_ds_output_1, 8);

            c_ds_output_0 <: (frame_audio * unsafe)audio[buffer].data[0];
            c_ds_output_1 <: (frame_audio * unsafe)audio[buffer].data[4];

            buffer = 1 - buffer;

#ifdef FORM_BEAM

            //copy the current sample to the delay buffer
            for(unsigned i=0;i<7;i++)
                delay_buffer[delay_head][i] = audio[buffer].data[i][0];

            //light the LED for the current direction
#endif


            int t;

            select {
                case lb.button_event():{
                    unsigned button;
                    e_button_state pressed;
                    lb.get_button_event(button, pressed);
                    if(pressed == BUTTON_PRESSED){
                        switch(button){
                        case 0:{

#ifdef FORM_BEAM
                            printf("beamed\n");
                            if(omni)
                            {
                                omni = 0;
                                lb.set_led_brightness(12, 0);
                                set_dir(lb, dir, delay);
                            }
                            else
                            {
                                dir--;
                                if(dir == -1)
                                    dir = 5;
                                set_dir(lb, dir, delay);
                                printf("dir %d\n", dir+1);
                                for(unsigned i=0;i<7;i++)
                                    printf("delay[%d] = %d\n", i, delay[i]);
                                printf("\n");
                            }
#else
                            summed = !summed;

                            if(summed)
                            {
                                for(unsigned i=0; i < 13; i++)
                                    lb.set_led_brightness(i, 255);
                            }
                            else
                            { 
                                for(unsigned i=0;i<13;i++)
                                    lb.set_led_brightness(i, 0);
                                    
                                lb.set_led_brightness(12, 255);

                            }

#endif
                            break;
                        }
                        case 1:{
                            gain = ((gain<<3) + gain)>>3;
                            printf("gain: %d\n", gain);
                            break;
                        }
                        case 2:{
                            gain = ((gain<<3) - gain)>>3;
                            printf("gain: %d\n", gain);
                            break;
                        }
                        case 3:{
#ifdef FORM_BEAM
                            lb.set_led_brightness(12, 255);

                            for(unsigned i=0;i<12;i++)
                                lb.set_led_brightness(i, 0);
#if 0
                            dir++;
                            if(dir == 6)
                                dir = 0;
                            set_dir(lb, dir, delay);
                            printf("dir %d\n", dir+1);
                            for(unsigned i=0;i<7;i++)
                                printf("delay[%d] = %d\n", i, delay[i]);
                            printf("\n");
 #endif 
                            printf("omni\n");
                            omni = 1; 
#endif  
                            break;
                        }
                        }
                    }
                    break;
                }
                default:break;
            }
            int output = 0;
 
#ifdef FORM_BEAM
            if(!omni)
            {
                /* Do the sum of the delayed mics */
                for(unsigned i=0;i<7;i++)
                    output += delay_buffer[(delay_head - delay[i])%MAX_DELAY][i];

                output = ((uint64_t)output*gain)>>8;
                
                c_audio <: output;
               
                /* Send out the individual mics */ 
                for(unsigned i=0;i<7;i++)
                {
                    /* Apply gain and output samples */
                    output = audio[buffer].data[i][0];
                    output = ((uint64_t)output*gain)>>8;
                    c_audio <: output;
                }
            }
            else
            {
                /* Send out Mic[0] 8 times */
                output = audio[buffer].data[0][0];
                output <<=2;
                
                output = ((uint64_t)output*gain)>>8;
                
                for(unsigned i=0;i<8;i++)
                    c_audio <: output;
            }
#else
            if(summed)
            {
                /* Output summed */
                for(unsigned i=0;i<7;i++)
                    output += audio[buffer].data[i][0];

                output = ((uint64_t)output*gain)>>8;
                c_audio <: output;

                /* Apply gain to all mics and send */
                for(unsigned i=0;i<7;i++)
                {    
                    output = audio[buffer].data[i][0];
                    output = ((uint64_t)output*gain)>>8;
                    c_audio <: output;
                }
            }
            else
            {  
                /* Send mic 0 out 8 times */
                for(unsigned i=0;i<8;i++)
                {
                    /* Apply gain and output samples */
                    output = audio[buffer].data[0][0];
                    output = ((uint64_t)output*gain)>>8;
                    c_audio <: output<<2;
                }
            }
#endif

#ifdef FORM_BEAM
            delay_head++;
            delay_head%=MAX_DELAY;
#endif
        }
    }
}

//TODO make these not global
int data_0[8*COEFS_PER_PHASE] = {0};
int data_1[8*COEFS_PER_PHASE] = {0};



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

    unsafe
    {
        const int * unsafe p[1] = {fir_1_coefs[0]};
        decimator_config dc0 = {0, 1, 0, 0, 1, p, data_0, 0, {0,0, 0, 0}};
        decimator_config dc1 = {0, 1, 0, 0, 1, p, data_1, 0, {0,0, 0, 0}};

        par 
        {
            button_and_led_server(lb, leds, p_buttons);
            pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, c_4x_pdm_mic_1);
            decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output_0, dc0);
            decimate_to_pcm_4ch(c_4x_pdm_mic_1, c_ds_output_1, dc1);
            lores_DAS_fixed(c_ds_output_0, c_ds_output_1, lb, c_pcm_out);
        }

    }
}

#endif
