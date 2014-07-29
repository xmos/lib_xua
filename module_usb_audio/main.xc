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

#include "devicedefines.h"       /* Device specific defines */
#include "endpoint0.h"
#include "usb_buffer.h"
#include "decouple.h"
#ifdef MIDI
#include "usb_midi.h"
#endif
#include "audio.h"

#ifdef IAP
#include "i2c_shared.h"
#include "iap.h"
#endif

#ifdef MIXER
#include "mixer.h"
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

#if (XUD_SERIES_SUPPORT == XUD_L_SERIES) && (AUDIO_IO_TILE == XUD_TILE)
/* Note: L series ref clocked clocked from USB clock when USB enabled - use another clockblock for MIDI
 * if MIDI and XUD on same tile. See XUD documentation.
 *
 * This is a clash with S/PDIF Tx but simultaneous S/PDIF and MIDI not currently supported on single tile device
 *
 */
#define CLKBLK_MIDI        XS1_CLKBLK_1;
#else
#define CLKBLK_MIDI        XS1_CLKBLK_REF;
#endif
#define CLKBLK_SPDIF_TX    XS1_CLKBLK_1
#define CLKBLK_MCLK        XS1_CLKBLK_2
#define CLKBLK_I2S_BIT     XS1_CLKBLK_3
#define CLKBLK_XUD         XS1_CLKBLK_4 /* Note XUD for U-series uses CLKBLK_5 also (see XUD_Ports.xc) */

#ifndef CODEC_MASTER
on tile[AUDIO_IO_TILE] : buffered out port:32 p_lrclk       = PORT_I2S_LRCLK;
on tile[AUDIO_IO_TILE] : buffered out port:32 p_bclk        = PORT_I2S_BCLK;
#else
on tile[AUDIO_IO_TILE] : in port p_lrclk                    = PORT_I2S_LRCLK;
on tile[AUDIO_IO_TILE] : in port p_bclk                     = PORT_I2S_BCLK;
#endif

on tile[AUDIO_IO_TILE] : port p_mclk_in                     = PORT_MCLK_IN;
on tile[XUD_TILE] : in port p_for_mclk_count                = PORT_MCLK_COUNT;

#ifdef SPDIF
on tile[AUDIO_IO_TILE] : buffered out port:32 p_spdif_tx    = PORT_SPDIF_OUT;
#endif

#ifdef MIDI
on tile[AUDIO_IO_TILE] :  port p_midi_tx                    = PORT_MIDI_OUT;

#if(MIDI_RX_PORT_WIDTH == 4)
on tile[AUDIO_IO_TILE] :  buffered in port:4 p_midi_rx         = PORT_MIDI_IN;
#elif(MIDI_RX_PORT_WIDTH == 1)
on tile[AUDIO_IO_TILE] :  buffered in port:1 p_midi_rx         = PORT_MIDI_IN;
#endif
#endif

/* Clock blocks */
#ifdef MIDI
on tile[AUDIO_IO_TILE] : clock    clk_midi                  = CLKBLK_MIDI;
#endif

#ifdef SPDIF
on tile[AUDIO_IO_TILE] : clock    clk_mst_spd               = CLKBLK_SPDIF_TX;
#endif

on tile[AUDIO_IO_TILE] : clock    clk_audio_mclk            = CLKBLK_MCLK;       /* Master clock */

#if(AUDIO_IO_TILE != XUD_TILE)
on tile[XUD_TILE] : clock    clk_audio_mclk2                = CLKBLK_MCLK;       /* Master clock */
on tile[XUD_TILE] : in port  p_mclk_in2                     = PORT_MCLK_IN2;
#endif

on tile[AUDIO_IO_TILE] : clock    clk_audio_bclk            = CLKBLK_I2S_BIT;    /* Bit clock */


/* L/G Series needs a port to use for USB reset */
#if (XUD_SERIES_SUPPORT != XUD_U_SERIES) && defined(PORT_USB_RESET)
/* This define is checked since it could be on a shift reg or similar */
on tile[XUD_TILE] : out port p_usb_rst                      = PORT_USB_RESET;
#else
/* Reset port not required for U series due to built in Phy */
#define p_usb_rst   null
#endif

#if (XUD_SERIES_SUPPORT != XUD_U_SERIES)
/* L Series also needs a clock block for this port */
on tile[XUD_TILE] : clock clk                               = CLKBLK_XUD;
#else
#define clk         null
#endif

#ifdef IAP
/* I2C ports - in a struct for use with module_i2s_simple */
on tile [IAP_TILE] : struct r_i2c r_i2c = {PORT_I2C_SCL, PORT_I2C_SDA};
#endif


/* Endpoint type tables for XUD */
XUD_EpType epTypeTableOut[ENDPOINT_COUNT_OUT] = { XUD_EPTYPE_CTL | XUD_STATUS_ENABLE,
                                            XUD_EPTYPE_ISO,    /* Audio */
#ifdef MIDI
                                            XUD_EPTYPE_BUL,    /* MIDI */
#endif
#ifdef IAP
                                            XUD_EPTYPE_BUL, /* iAP */
#ifdef IAP_EA_NATIVE_TRANS
                                            XUD_EPTYPE_BUL, /* EA Native Transport */
#endif
#endif
                                        };

XUD_EpType epTypeTableIn[ENDPOINT_COUNT_IN] = { XUD_EPTYPE_CTL | XUD_STATUS_ENABLE,
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
                                            XUD_EPTYPE_BUL | XUD_STATUS_ENABLE,
#ifdef IAP_INT_EP
                                            XUD_EPTYPE_BUL | XUD_STATUS_ENABLE,
#endif
#ifdef IAP_EA_NATIVE_TRANS
                                            XUD_EPTYPE_BUL | XUD_STATUS_ENABLE,
#endif
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

/* Core USB Audio functions - must be called on the Tile connected to the USB Phy */
void usb_audio_core(chanend c_mix_out
#ifdef MIDI
, chanend c_midi
#endif
#ifdef IAP
, chanend c_iap
#ifdef IAP_EA_NATIVE_TRANS
, chanend c_ea_data
#endif
#endif
#ifdef MIXER
, chanend c_mix_ctl
#endif
)
{
    chan c_sof;
    chan c_xud_out[ENDPOINT_COUNT_OUT];              /* Endpoint channels for XUD */
    chan c_xud_in[ENDPOINT_COUNT_IN];
    chan c_aud_ctl;
#ifdef CHAN_BUFF_CTRL
#warning Using channel to control buffering - this may reduce performance but improve power consumption
    chan c_buff_ctrl;
#endif

#ifndef MIXER
#define c_mix_ctl null
#endif

#ifdef IAP_EA_NATIVE_TRANS
    chan c_EANativeTransport_ctrl;
#else
#define c_EANativeTransport_ctrl null
#endif

    par
    {
        /* USB Interface Core */
#if (AUDIO_CLASS==2)
        XUD_Manager(c_xud_out, ENDPOINT_COUNT_OUT, c_xud_in, ENDPOINT_COUNT_IN,
            c_sof, epTypeTableOut, epTypeTableIn, p_usb_rst,
            clk, 1, XUD_SPEED_HS, pwrConfig);
#else
        XUD_Manager(c_xud_out, ENDPOINT_COUNT_OUT, c_xud_in, ENDPOINT_COUNT_IN,
            c_sof, epTypeTableOut, epTypeTableIn, p_usb_rst,
            clk, 1, XUD_SPEED_FS, pwrConfig);
#endif

        /* USB Packet buffering Core */
        {
            unsigned x;
            thread_speed();

            /* Attach mclk count port to mclk clock-block (for feedback) */
            //set_port_clock(p_for_mclk_count, clk_audio_mclk);
#if(AUDIO_IO_TILE != 0)
            set_clock_src(clk_audio_mclk2, p_mclk_in2);
            set_port_clock(p_for_mclk_count, clk_audio_mclk2);
            start_clock(clk_audio_mclk2);
#else
            /* Uses same clock-block as I2S */
            asm("ldw %0, dp[clk_audio_mclk]":"=r"(x));
            asm("setclk res[%0], %1"::"r"(p_for_mclk_count), "r"(x));
#endif
            //:buffer
            buffer(c_xud_out[ENDPOINT_NUMBER_OUT_AUDIO],/* Audio Out*/
                c_xud_in[ENDPOINT_NUMBER_IN_AUDIO],     /* Audio In */
                c_xud_in[ENDPOINT_NUMBER_IN_FEEDBACK],      /* Audio FB */
#ifdef MIDI
                c_xud_out[ENDPOINT_NUMBER_OUT_MIDI],  /* MIDI Out */ // 2
                c_xud_in[ENDPOINT_NUMBER_IN_MIDI],    /* MIDI In */  // 4
                c_midi,
#endif
#ifdef IAP
                c_xud_out[ENDPOINT_NUMBER_OUT_IAP],   /* iAP Out */
                c_xud_in[ENDPOINT_NUMBER_IN_IAP],     /* iAP In */
#ifdef IAP_INT_EP
                c_xud_in[ENDPOINT_NUMBER_IN_IAP_INT], /* iAP Interrupt In */
#endif
                c_iap,
#endif
#if defined(SPDIF_RX) || defined(ADAT_RX)
                /* Audio Interrupt - only used for interrupts on external clock change */
                c_xud_in[ENDPOINT_NUMBER_IN_INTERRUPT],
#endif
                c_sof, c_aud_ctl, p_for_mclk_count
#ifdef HID_CONTROLS
                , c_xud_in[ENDPOINT_NUMBER_IN_HID]
#endif
#ifdef CHAN_BUFF_CTRL
                , c_buff_ctrl
#endif
            );
            //:
        }

        /* Endpoint 0 Core */
        {
            thread_speed();
            Endpoint0( c_xud_out[0], c_xud_in[0], c_aud_ctl, c_mix_ctl, null, c_EANativeTransport_ctrl);
        }

        /* Decoupling core */
        {
            thread_speed();
            decouple(c_mix_out, null
#ifdef CHAN_BUFF_CTRL
                , c_buff_ctrl
#endif
            );
        }

#if defined(IAP_EA_NATIVE_TRANS)
        /* EA Native Transport Endpoint manager */
        {
            thread_speed();
            iAP2_EANativeTransport_EPManager(
                c_xud_out[ENDPOINT_NUMBER_OUT_IAP_EA_NATIVE_TRANS],
                c_xud_in[ENDPOINT_NUMBER_IN_IAP_EA_NATIVE_TRANS],
                c_EANativeTransport_ctrl, c_ea_data);
        }
#endif
        //:
    }
}

void usb_audio_io(chanend c_aud_in, chanend ?c_adc
#ifdef MIXER
, chanend c_mix_ctl
#endif
, chanend ?c_aud_cfg
)
{
#ifdef MIXER
    chan c_mix_out;
#endif

#if defined (SPDIF_RX) || (defined ADAT_RX)
    chan c_dig_rx;
#else
#define c_dig_rx null
#endif

    par
    {

#ifdef MIXER
        /* Mixer cores(s) */
        {
            thread_speed();
            mixer(c_aud_in, c_mix_out, c_mix_ctl);
        }
#endif
        /* Audio I/O Core (pars additional S/PDIF TX Core) */
        {
            thread_speed();
#ifdef MIXER
            audio(c_mix_out, c_dig_rx, c_aud_cfg, c_adc);
#else
            audio(c_aud_in, c_dig_rx, c_aud_cfg, c_adc);
#endif
        }
        //:
    }
}

#ifndef USER_MAIN_DECLARATIONS
#define USER_MAIN_DECLARATIONS
#endif

#ifndef USER_MAIN_CORES
#define USER_MAIN_CORES
#endif

/* Main for USB Audio Applications */
int main()
{
    chan c_mix_out;
#ifdef MIDI
    chan c_midi;
#endif
#ifdef IAP
    chan c_iap;
#ifdef IAP_EA_NATIVE_TRANS
    chan c_ea_data;
#endif
#endif
#ifdef SU1_ADC_ENABLE
    chan c_adc;
#else
#define c_adc null
#endif

#ifdef MIXER
    chan c_mix_ctl;
#endif

#ifdef AUDIO_CFG_CHAN
    chan c_aud_cfg;
#else
#define c_aud_cfg null
#endif

    USER_MAIN_DECLARATIONS

    par
    {
        on tile[XUD_TILE]: usb_audio_core(c_mix_out
#ifdef MIDI
            , c_midi
#endif
#ifdef IAP
            , c_iap
#ifdef IAP_EA_NATIVE_TRANS
            , c_ea_data
#endif
#endif
#ifdef MIXER
            , c_mix_ctl
#endif
);

        on tile[AUDIO_IO_TILE]: usb_audio_io(c_mix_out, c_adc
#ifdef MIXER
            , c_mix_ctl
#endif
            , c_aud_cfg
        );

#if defined(MIDI) && defined(IAP) && (IAP_TILE == MIDI_TILE)
        /* MIDI and IAP share a core */
        on tile[IAP_TILE]:
        {
            thread_speed();
            usb_midi(p_midi_rx, p_midi_tx, clk_midi, c_midi, 0, c_iap, null, null, null);
        }
#else
#if defined(MIDI)
        /* MIDI core */
        on tile[MIDI_TILE]:
        {
            thread_speed();
            usb_midi(p_midi_rx, p_midi_tx, clk_midi, c_midi, 0, null, null, null, null);
        }
#endif
#if defined(IAP)
        on tile[IAP_TILE]:
        {
            thread_speed();
            iAP(c_iap, null, null, null);
        }
#endif
#endif

        USER_MAIN_CORES
    }

#ifdef SU1_ADC_ENABLE
        xs1_su_adc_service(c_adc);
#endif

    return 0;
}

