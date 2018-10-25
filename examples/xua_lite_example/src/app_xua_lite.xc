// Copyright (c) 2017-2018, XMOS Ltd, All rights reserved

// A very simple *example* of a USB audio application (and as such is un-verified for production)

#include <xs1.h>
#include <platform.h>

#include "xua.h"
#include "i2s.h"
#include "i2c.h"
#include "gpio.h"

// Port declarations. Note, the defines come from the xn file 
on tile[0]: buffered out port:32 p_i2s_dac[]    = {XS1_PORT_1M};   //DAC
on tile[0]: buffered in port:32 p_i2s_adc[]    	= {XS1_PORT_1N};   //Unused currently
on tile[0]: buffered out port:32 p_lrclk        = XS1_PORT_1O;     //I2S Bit-clock 
on tile[0]: out port p_bclk                     = XS1_PORT_1P;     //I2S L/R-clock 

// Master clock for the audio IO tile 
on tile[0]: in port p_mclk_in       = XS1_PORT_1A;

// Resources for USB feedback 
on tile[0]: in port p_for_mclk_count= XS1_PORT_16A;   // Extra port for counting master clock ticks 

// [0] : DAC_RESET_N
// [1] : I2C_INTERRUPT_N
// [2] : MUTE_EN
// [3] : LED
on tile[0]: out port p_gpio         = XS1_PORT_4D;

on tile[1]: port p_scl              = XS1_PORT_1C;
on tile[1]: port p_sda              = XS1_PORT_1D;


// Clock-block declarations 
clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_2;   // Bit clock     
clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_3;   // Master clock 

// Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
// if the endpoint wishes to be informed of USB bus resets 

XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO, XUD_EPTYPE_ISO};

void XUA_Buffer_lite(chanend c_aud_out, chanend c_feedback, chanend c_aud_in, chanend c_sof, chanend c_aud_ctl, in port p_for_mclk_count, chanend c_aud_host);
[[distributable]]
void AudioHub(server i2s_frame_callback_if i2s, chanend c_aud, client i2c_master_if i2c, client output_gpio_if dac_reset);

int main()
{
    // Channels for lib_xud 
    chan c_ep_out[2];
    chan c_ep_in[3];

    // Channel for communicating SOF notifications from XUD to the Buffering cores 
    chan c_sof;

    interface i2s_frame_callback_if i_i2s;
    interface i2c_master_if i_i2c[1];
    interface output_gpio_if i_gpio[1];

    chan c_audio;

    
    // Channel for communicating control messages from EP0 to the rest of the device (via the buffering cores) 
    chan c_aud_ctl;

    par
    {
        on tile[0]: {
            // Connect master-clock clock-block to clock-block pin 
            set_clock_src(clk_audio_mclk, p_mclk_in);               // Clock clock-block from mclk pin 
            set_port_clock(p_for_mclk_count, clk_audio_mclk);       // Clock the "count" port from the clock block 
            start_clock(clk_audio_mclk);                            // Set the clock off running 


            par {
                // Low level USB device layer core  
                XUD_Main(c_ep_out, 2, c_ep_in, 3,
                          c_sof, epTypeTableOut, epTypeTableIn, 
                          null, null, -1 , 
                          XUD_SPEED_FS, XUD_PWR_BUS);
            
                // Endpoint 0 core from lib_xua 
                // Note, since we are not using many features we pass in null for quite a few params.. 
                XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);

                // Buffering cores - handles audio data to/from EP's and gives/gets data to/from the audio I/O core 
                XUA_Buffer_lite(c_ep_out[1], c_ep_in[2], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_audio);

                i2s_frame_master(i_i2s, p_i2s_dac, 1, p_i2s_adc, 1, p_bclk, p_lrclk, p_mclk_in, clk_audio_bclk);
                [[distribute]]AudioHub(i_i2s, c_audio, i_i2c[0], i_gpio[0]);
                [[distribute]]output_gpio(i_gpio, 1, p_gpio, null);
            }
        }
        on tile[1]:{
            par{
                i2c_master(i_i2c, 1, p_scl, p_sda, 100);
            }
        }
    }
    
    return 0;
}


