// Copyright (c) 2017-2018, XMOS Ltd, All rights reserved

// A very simple *example* of a USB audio application (and as such is un-verified for production)
#include <stdint.h>

#include <xs1.h>
#include <platform.h>

#include "xua.h"
#include "xud.h"
#include "i2s.h"
#include "i2c.h"
#include "mic_array.h"
#include "xua_buffer_lite.h"
#include "xua_ep0_wrapper.h"
#include "pdm_mic.h"
#include "audio_config.h"
#include "audio_hub.h"

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
on tile[0]: clock clk_audio_bclk    = XS1_CLKBLK_2;   // Bit clock
on tile[0]: clock clk_audio_mclk    = XS1_CLKBLK_3;   // Master clock
//XUD uses XS1_CLKBLK_4, XS1_CLKBLK_5 on tile[1]

//Mic array resources
on tile[0]: out port p_pdm_clk               = XS1_PORT_1L;
on tile[0]: in buffered port:32 p_pdm_mics   = XS1_PORT_4E;

on tile[0]: clock pdmclk                     = XS1_CLKBLK_4;
on tile[0]: clock pdmclk6                    = XS1_CLKBLK_5;


// Endpoint type tables - informs XUD what the transfer types for each Endpoint in use and also
// if the endpoint wishes to be informed of USB bus resets

XUD_EpType epTypeTableOut[]   = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
XUD_EpType epTypeTableIn[]    = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO, XUD_EPTYPE_ISO};

void burn_normal_priority(void){
    while(1);
}

void burn_high_priority(void){
    set_core_high_priority_on();
    while(1);
}

int main()
{
    // Channels for lib_xud
    chan c_ep_out[XUA_ENDPOINT_COUNT_OUT];
    chan c_ep_in[XUA_ENDPOINT_COUNT_IN];

    // Channel for communicating SOF notifications from XUD to the Buffering cores
    chan c_sof;

    interface i2s_frame_callback_if i_i2s;
    interface i2c_master_if i_i2c[1];

    streaming chan c_audio; //We use the channel buffering (48B across switch each way)
    streaming chan c_ds_output[1];

    interface ep0_control_if i_ep0_ctl;

    par
    {
        on tile[0]: {
            //Set the GPIOs needed for audio (reset and mute)
            setup_audio_gpio(p_gpio);
            c_audio <: 0;       //Signal that we can now do i2c setup
            c_audio :> int _;   //Now wait until i2c has finished mclk setup

            const unsigned micDiv = MCLK_48/3072000;
            mic_array_setup_ddr(pdmclk, pdmclk6, p_mclk_in, p_pdm_clk, p_pdm_mics, micDiv);

            par {
                i2s_frame_master(i_i2s, p_i2s_dac, 1, p_i2s_adc, 1, p_bclk, p_lrclk, p_mclk_in, clk_audio_bclk);
                [[distribute]] AudioHub(i_i2s, c_audio, c_ds_output);
                pdm_mic(c_ds_output[0], p_pdm_mics);

                par (int i = 0; i < 5; i++) burn_normal_priority();
                par (int i = 0; i < 0; i++) burn_high_priority();
            }
        }
        on tile[1]:unsafe{
            // Connect master-clock input clock-block to clock-block pin for asnch feedback calculation
            set_clock_src(clk_usb_mclk, p_mclk_in_usb);           // Clock clock-block from mclk pin
            set_port_clock(p_for_mclk_count, clk_usb_mclk);       // Clock the "count" port from the clock block
            start_clock(clk_usb_mclk);                            // Set the clock off running

            //Setup DAC over i2c and then return so we do not use a thread
            c_audio :> int _; //Wait for reset to be asserted/deasserted by other tile
            par{
                i2c_master(i_i2c, 1, p_scl, p_sda, 100);
                AudioHwConfigure(DEFAULT_FREQ, i_i2c[0]);
            }
            c_audio <: 0; //Signal to tile[0] that mclk is now good

            par {
                // Low level USB device layer core
                XUD_Main(c_ep_out, XUA_ENDPOINT_COUNT_OUT, c_ep_in, XUA_ENDPOINT_COUNT_IN,
                          c_sof, epTypeTableOut, epTypeTableIn,
                          null, null, -1 ,
                          (AUDIO_CLASS == 1) ? XUD_SPEED_FS : XUD_SPEED_HS, XUD_PWR_BUS);

                // // Buffering core - handles audio and control data to/from EP's and gives/gets data to/from the audio I/O core
                // XUA_Buffer_lite(c_ep_out[0],
                //                 c_ep_in[0],
                //                 c_ep_out[1],
                //                 null, //c_ep_in[XUA_ENDPOINT_COUNT_IN - 2],/*feedback*/
                //                 c_ep_in[XUA_ENDPOINT_COUNT_IN - 1],
                //                 c_sof, p_for_mclk_count, c_audio);

                //[[combine]]
                par{
                XUA_Buffer_lite2(i_ep0_ctl,
                                c_ep_out[1],
                                null, //c_ep_in[XUA_ENDPOINT_COUNT_IN - 2],/*feedback*/
                                c_ep_in[XUA_ENDPOINT_COUNT_IN - 1],
                                c_sof, p_for_mclk_count, c_audio);
                XUA_Endpoint0_select(c_ep_out[0], c_ep_in[0], i_ep0_ctl, null VENDOR_REQUESTS_PARAMS_DEC_);
                }
                par (int i = 0; i < 3; i++) burn_normal_priority();
                par (int i = 0; i < 2; i++) burn_high_priority();
            }
        }//Tile[1] par
    }//Top level par

    return 0;
}


