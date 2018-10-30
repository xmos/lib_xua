// Copyright (c) 2017-2018, XMOS Ltd, All rights reserved

// A very simple *example* of a USB audio application (and as such is un-verified for production)
#include <stdint.h>

#include <xs1.h>
#include <platform.h>

#include "xua.h"
#include "xud.h"
#include "i2s.h"
#include "i2c.h"
#include "gpio.h"

#define DEBUG_UNIT XUA_APP
#define DEBUG_PRINT_ENABLE_XUA_APP 1
#include "debug_print.h"

// Port declarations. Note, the defines come from the xn file 
on tile[0]: buffered out port:32 p_i2s_dac[]    = {XS1_PORT_1N};   //DAC
on tile[0]: buffered in port:32 p_i2s_adc[]    	= {XS1_PORT_1F};   //Unused currently
on tile[0]: buffered out port:32 p_lrclk        = XS1_PORT_1O;     //I2S Bit-clock 
on tile[0]: out port p_bclk                     = XS1_PORT_1P;     //I2S L/R-clock 

// Master clock for the audio IO tile 
on tile[0]: in port p_mclk_in       = XS1_PORT_1K;

// [0] : DAC_RESET_N
// [1] : I2C_INTERRUPT_N
// [2] : MUTE_EN
// [3] : LED
on tile[0]: out port p_gpio         = XS1_PORT_4D;

on tile[1]: port p_scl              = XS1_PORT_1C;
on tile[1]: port p_sda              = XS1_PORT_1D;
on tile[1]: in port p_mclk_in_usb   = XS1_PORT_1A;
on tile[1]: in port p_for_mclk_count= XS1_PORT_16A;   // Extra port for counting master clock ticks 
on tile[1]: clock clk_usb_mclk      = XS1_CLKBLK_3;   // Master clock 


// Clock-block declarations 
on tile[0]:clock clk_audio_bclk     = XS1_CLKBLK_2;   // Bit clock     
on tile[0]:clock clk_audio_mclk     = XS1_CLKBLK_3;   // Master clock 
//XUD uses XS1_CLKBLK_4, XS1_CLKBLK_5 on tile[1]

// Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
// if the endpoint wishes to be informed of USB bus resets 

XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO, XUD_EPTYPE_ISO};

void XUA_Buffer_lite(chanend c_ep0_out, chanend c_ep0_in, chanend c_aud_out, chanend c_feedback, chanend c_aud_in, chanend c_sof, in port p_for_mclk_count, streaming chanend c_aud_host);
[[distributable]]
void AudioHub(server i2s_frame_callback_if i2s, streaming chanend c_aud, client i2c_master_if ?i2c, client output_gpio_if dac_reset);
void AudioHwConfigure(unsigned samFreq, client i2c_master_if i_i2c);
void XUA_Endpoint0_select(chanend c_ep0_out, chanend c_ep0_in, chanend c_audioControl,
    chanend ?c_mix_ctl, chanend ?c_clk_ctl, chanend ?c_EANativeTransport_ctrl, CLIENT_INTERFACE(i_dfu, ?dfuInterface) VENDOR_REQUESTS_PARAMS_DEC_);

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

    streaming chan c_audio; //We use the channel buffering (48B across switch each way)

    par
    {
        on tile[0]: {
            par {
                i2s_frame_master(i_i2s, p_i2s_dac, 1, p_i2s_adc, 1, p_bclk, p_lrclk, p_mclk_in, clk_audio_bclk);
                [[distribute]]AudioHub(i_i2s, c_audio, null, i_gpio[0]);
                [[distribute]]output_gpio(i_gpio, 1, p_gpio, null);
            }
        }
        on tile[1]:{
            // Connect master-clock clock-block to clock-block pin 
            set_clock_src(clk_usb_mclk, p_mclk_in_usb);           // Clock clock-block from mclk pin 
            set_port_clock(p_for_mclk_count, clk_usb_mclk);       // Clock the "count" port from the clock block 
            start_clock(clk_usb_mclk);                            // Set the clock off running 

            //Setup DAC and then return so we do not use a thread
            par{
                i2c_master(i_i2c, 1, p_scl, p_sda, 100);
                AudioHwConfigure(DEFAULT_FREQ, i_i2c[0]);
            }

            debug_printf("XUD SPEED: %d\n", (AUDIO_CLASS == 1) ? XUD_SPEED_FS : XUD_SPEED_HS); //FS = 1, HS = 2
            par{
                // Low level USB device layer core  
                XUD_Main(c_ep_out, 2, c_ep_in, 3,
                          c_sof, epTypeTableOut, epTypeTableIn, 
                          null, null, -1 , 
                          (AUDIO_CLASS == 1) ? XUD_SPEED_FS : XUD_SPEED_HS, XUD_PWR_BUS);

                // Buffering cores - handles audio and control data to/from EP's and gives/gets data to/from the audio I/O core 
                XUA_Buffer_lite(c_ep_out[0], c_ep_in[0], c_ep_out[1], c_ep_in[1], c_ep_in[2], c_sof, p_for_mclk_count, c_audio);
            }
        }//Tile[1] par
    }//Top level par
    
    return 0;
}


