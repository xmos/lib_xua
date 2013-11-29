/**
 * @file    main.xc
 * @brief   Top level for XMOS USB 2.0 Audio 2.0 Reference Designs.
 * @author  Ross Owen, XMOS Semiconductor Ltd 
 */                               
#include <syscall.h>
#include <platform.h>
#include <xs1.h>
#include <xclib.h>
#include <print.h>
#ifdef XSCOPE
#include <xscope.h>
#endif

#include "xud.h"                 /* XMOS USB Device Layer defines and functions */
#include "usb.h"                 /* Defines from the USB 2.0 Specification */

#include "devicedefines.h"       /* Device specific defines */  
#include "endpoint0.h"
#include "usb_buffer.h"
#include "decouple.h"
#ifdef MIDI
#include "usb_midi.h"
#endif
#include "audio.h"

#ifdef IAP
#include "iAP.h"
#endif

#ifndef AUDIO_IO_TILE
#define AUDIO_IO_TILE   0
#endif

#ifndef XUD_TILE
#define XUD_TILE        0
#endif

/* Audio I/O - Port declarations */
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
#error Not supported
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
#error Not supported
#endif
#if I2S_WIRES_ADC > 0
                };
#endif


#ifndef CODEC_MASTER
on tile[AUDIO_IO_TILE] : buffered out port:32 p_lrclk       = PORT_I2S_LRCLK;
on tile[AUDIO_IO_TILE] : buffered out port:32 p_bclk        = PORT_I2S_BCLK;
#else
on tile[AUDIO_IO_TILE] : in port p_lrclk                    = PORT_I2S_LRCLK;
on tile[AUDIO_IO_TILE] : in port p_bclk                     = PORT_I2S_BCLK;
#endif

on tile[AUDIO_IO_TILE] : port p_mclk_in                     = PORT_MCLK_IN;
on tile[0] : in port p_for_mclk_count                       = PORT_MCLK_COUNT;

#ifdef SPDIF  
on tile[AUDIO_IO_TILE] : buffered out port:32 p_spdif_tx    = PORT_SPDIF_OUT;
#endif

#ifdef MIDI
on tile[AUDIO_IO_TILE] :  port p_midi_tx                    = PORT_MIDI_OUT;
on tile[AUDIO_IO_TILE] :  port p_midi_rx                    = PORT_MIDI_IN;
#endif

/* Clock blocks */
#ifdef MIDI
on tile[AUDIO_IO_TILE] : clock    clk_midi                  = XS1_CLKBLK_REF;
#endif
on tile[AUDIO_IO_TILE] : clock    clk_audio_mclk            = XS1_CLKBLK_2;     /* Master clock */

#if(AUDIO_IO_TILE != 0)
on tile[0] : clock    clk_audio_mclk2                       = XS1_CLKBLK_2;     /* Master clock */
on tile[0] : in port  p_mclk_in2                            = PORT_MCLK_IN2; 
#endif


on tile[AUDIO_IO_TILE] : clock    clk_audio_bclk            = XS1_CLKBLK_3;     /* Bit clock */
#ifdef SPDIF
on tile[AUDIO_IO_TILE] : clock    clk_mst_spd               = XS1_CLKBLK_1;
#endif

/* L Series needs a port to use for USB reset */
#ifdef ARCH_L
#ifdef PORT_USB_RESET
/* This define is checked since it could be on a shift reg or similar */
on tile[XUD_TILE] : out port p_usb_rst                      = PORT_USB_RESET;
#endif
/* L Series also needs a clock for this port */
on tile[XUD_TILE] : clock clk                               = XS1_CLKBLK_4;
#else
/* Reset port not required for SU1 due to built in Phy */
#define p_usb_rst   null
#define clk         null
#endif

#ifdef IAP
on tile [AUDIO_IO_TILE] : port p_i2c_sda                    = PORT_I2C_SDA;
on tile [AUDIO_IO_TILE] : port p_i2c_scl                    = PORT_I2C_SCL;
#endif


/* Endpoint type tables for XUD */
XUD_EpType epTypeTableOut[EP_CNT_OUT] = { XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, 
                                            XUD_EPTYPE_ISO,    /* Audio */
#ifdef MIDI
                                            XUD_EPTYPE_BUL,    /* MIDI */
#endif
#ifdef IAP
                                            XUD_EPTYPE_BUL | XUD_STATUS_ENABLE /* iAP */
#endif
                                        };    

XUD_EpType epTypeTableIn[EP_CNT_IN] = { XUD_EPTYPE_CTL | XUD_STATUS_ENABLE,
                                            XUD_EPTYPE_ISO, 
                                            XUD_EPTYPE_ISO,
#if defined (SPDIF_RX) || defined (ADAT_RX)
                                            XUD_EPTYPE_BUL,
#endif
#ifdef MIDI
                                            XUD_EPTYPE_BUL,
#endif
#ifdef HID_CONTROLS
                                            XUD_EPTYPE_INT,
#endif
#ifdef IAP
                                            XUD_EPTYPE_BUL,
                                            XUD_EPTYPE_BUL,
#endif
                                        };

void thread_speed()
{
#ifdef FAST_MODE
#warning Building with fast mode enabled
    set_thread_fast_mode_on();
#else
    set_thread_fast_mode_off();
#endif
}

#ifdef XSCOPE
void xscope_user_init()
{
    xscope_register(0, 0, "", 0, "");

    xscope_config_io(XSCOPE_IO_BASIC);
}
#endif

#ifdef SELF_POWERED
#define pwrConfig XUD_PWR_SELF
#else
#define pwrConfig XUD_PWR_BUS
#endif


int main()
{
    chan c_sof;
    chan c_xud_out[EP_CNT_OUT];              /* Endpoint channels for XUD */
    chan c_xud_in[EP_CNT_IN];
    chan c_aud_ctl;
    chan c_mix_out;
#ifdef MIDI
    chan c_midi;
#endif
#ifdef IAP
    chan c_iap;
#endif

#ifdef TEST_MODE_SUPPORT
#warning Building with test mode support
    chan c_usb_test;
#else
#define c_usb_test null
#endif

#ifdef SU1_ADC_ENABLE
    chan c_adc;
#else
#define c_adc null
#endif

#ifdef CHAN_BUFF_CTRL
#warning Using channel to control buffering - this may reduce performance but improve power consumption
    chan c_buff_ctrl;
#endif



    par 
    {
    
        /* USB Interface */
#if (AUDIO_CLASS==2) 
        on tile[0]: XUD_Manager(c_xud_out, EP_CNT_OUT, c_xud_in, EP_CNT_IN, 
                  c_sof, epTypeTableOut, epTypeTableIn, p_usb_rst, 
                  clk, 1, XUD_SPEED_HS, c_usb_test, pwrConfig);  
#else
        on tile[0]:XUD_Manager(c_xud_out, EP_CNT_OUT, c_xud_in, EP_CNT_IN, 
                  c_sof, epTypeTableOut, epTypeTableIn, p_usb_rst, 
                  clk, 1, XUD_SPEED_FS, c_usb_test, pwrConfig);  
#endif
        
              on tile[0]:
        {
            thread_speed();
            
            /* Attach mclk count port to mclk clock-block (for feedback) */
            //set_port_clock(p_for_mclk_count, clk_audio_mclk);
            {
                unsigned x;
#if(AUDIO_IO_TILE != 0)
                set_clock_src(clk_audio_mclk2, p_mclk_in2);
                set_port_clock(p_for_mclk_count, clk_audio_mclk2); 
                start_clock(clk_audio_mclk2);
#else
                /* Uses same clock-block as I2S */
                asm("ldw %0, dp[clk_audio_mclk]":"=r"(x));
                asm("setclk res[%0], %1"::"r"(p_for_mclk_count), "r"(x));
#endif
            }

            buffer(c_xud_out[EP_NUM_OUT_AUD],/* Audio Out*/
                c_xud_in[EP_NUM_IN_AUD],     /* Audio In */
                c_xud_in[EP_NUM_IN_FB],      /* Audio FB */
#ifdef MIDI 
                c_xud_out[EP_NUM_OUT_MIDI],  /* MIDI Out */ // 2
                c_xud_in[EP_NUM_IN_MIDI],    /* MIDI In */  // 4
                c_midi,
#endif
#ifdef IAP
                c_xud_out[EP_NUM_OUT_IAP], c_xud_in[EP_NUM_IN_IAP], c_xud_in[EP_NUM_IN_IAP_INT], c_iap,
#endif
#if defined(SPDIF_RX) || defined(ADAT_RX)
                /* Audio Interrupt - only used for interrupts on external clock change */
                c_xud_in[EP_NUM_IN_AUD_INT], 
#endif                
                c_sof, c_aud_ctl, p_for_mclk_count
#ifdef HID_CONTROLS
                , c_xud_in[EP_NUM_IN_HID]
#endif
#ifdef CHAN_BUFF_CTRL
                , c_buff_ctrl
#endif

                );

        }

        on tile[AUDIO_IO_TILE]:
        {
            thread_speed();

            /* Audio I/O (pars additional S/PDIF TX thread) */ 
            audio(c_mix_out, null, null, c_adc);
        }
        
        on tile[0]:
        {
            thread_speed();
            decouple(c_mix_out, null
#ifdef CHAN_BUFF_CTRL
            , c_buff_ctrl
#endif
            );
        }

          /* Endpoint 0 */
        on tile[0]:
        {
            thread_speed();
            Endpoint0( c_xud_out[0], c_xud_in[0], c_aud_ctl, null, null, c_usb_test);
        }

#if defined  (MIDI) || defined IAP
        on tile[AUDIO_IO_TILE]:
        {
            thread_speed();
#ifdef MIDI
            usb_midi(p_midi_rx, p_midi_tx, clk_midi, c_midi, 0, null, null, null, null);
#else
            iAP(c_iap, null, null, null);
#endif        
        }
#endif



#ifdef SU1_ADC_ENABLE
        xs1_su_adc_service(c_adc);
#endif

    }
    return 0;
}



