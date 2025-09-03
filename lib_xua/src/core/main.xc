// Copyright 2012-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua.h"                          /* Device specific defines */
#ifndef EXCLUDE_USB_AUDIO_MAIN

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

#if XUA_USB_EN
#include "xud_device.h"                 /* XMOS USB Device Layer defines and functions */
#include "xua_endpoint0.h"
#endif

#include "uac_hwresources.h"

#if (XUA_SPDIF_RX_EN || XUA_SPDIF_TX_EN)
#include "spdif.h"                     /* From lib_spdif */
#endif

#if (XUA_ADAT_RX_EN)
#include "adat_rx.h"
#endif

#if (XUA_NUM_PDM_MICS > 0)
#include "xua_pdm_mic.h"
#endif

#if (XUA_DFU_EN == 1)
[[distributable]]
void DFUHandler(server interface i_dfu i, chanend ?c_user_cmd);
#endif

/* Audio I/O - Port declarations */
#if I2S_WIRES_DAC > 0
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered out port:32 p_i2s_dac[I2S_WIRES_DAC] =
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
#else
    #define p_i2s_dac null
#endif

#if I2S_WIRES_ADC > 0
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered in port:32 p_i2s_adc[I2S_WIRES_ADC] =
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
#else
    #define p_i2s_adc null
#endif


#if CODEC_MASTER
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered in port:32 p_lrclk        = PORT_I2S_LRCLK;
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered in port:32 p_bclk         = PORT_I2S_BCLK;
#else
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered out port:32 p_lrclk       = PORT_I2S_LRCLK;
on tile[XUA_AUDIO_IO_TILE_NUM] : buffered out port:32 p_bclk        = PORT_I2S_BCLK;
#endif

#if (MCLK_REQUIRED)
/* Audio master clock input */
on tile[XUA_AUDIO_IO_TILE_NUM] :  in port p_mclk_in                 = PORT_MCLK_IN;
#else
#define p_mclk_in null
#endif

#if (SECOND_MCLK_REQUIRED)
/* If audio I/O and USB running on different tiles we need a separate port for
 * the master clock input (to use for USB async feedback calculation) */
on tile[XUA_XUD_TILE_NUM] : in port p_mclk_in_usb                   = PORT_MCLK_IN_USB;
#endif

#if XUA_USB_EN
on tile[XUA_XUD_TILE_NUM] : in port p_for_mclk_count                = PORT_MCLK_COUNT;
#endif

#if (XUA_SPDIF_TX_EN)
on tile[XUA_SPDIF_TX_TILE_NUM] : buffered out port:32 p_spdif_tx    = PORT_SPDIF_OUT;
#endif

#if (XUA_ADAT_TX_EN)
on stdcore[XUA_AUDIO_IO_TILE_NUM] : buffered out port:32 p_adat_tx  = PORT_ADAT_OUT;
#endif

#if (XUA_ADAT_RX_EN)
on stdcore[XUA_XUD_TILE_NUM] : buffered in port:32 p_adat_rx        = PORT_ADAT_IN;
#endif

#if (XUA_SPDIF_RX_EN)
on tile[XUA_XUD_TILE_NUM] : in port p_spdif_rx                      = PORT_SPDIF_IN;
#endif

#if (XUA_SPDIF_RX_EN) || (XUA_ADAT_RX_EN) || (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
/* Reference to external clock multiplier */
on tile[XUA_PLL_REF_TILE_NUM] : out port p_pll_ref                  = PORT_PLL_REF;
#ifdef __XS3A__
on tile[XUA_AUDIO_IO_TILE_NUM] : port p_for_mclk_count_audio        = PORT_MCLK_COUNT_2;
#else /* __XS3A__ */
#define p_for_mclk_count_audio                              null
#endif /* __XS3A__ */
#endif

#ifdef MIDI
on tile[XUA_MIDI_TILE_NUM] :  port p_midi_tx                        = PORT_MIDI_OUT;

#if(MIDI_RX_PORT_WIDTH == 4)
on tile[XUA_MIDI_TILE_NUM] :  buffered in port:4 p_midi_rx          = PORT_MIDI_IN;
#elif(MIDI_RX_PORT_WIDTH == 1)
on tile[XUA_MIDI_TILE_NUM] :  buffered in port:1 p_midi_rx          = PORT_MIDI_IN;
#endif
#endif


#ifdef MIDI
on tile[XUA_MIDI_TILE_NUM] : clock    clk_midi                      = CLKBLK_MIDI;
#endif

#if (XUA_SPDIF_TX_EN || XUA_ADAT_TX_EN)
on tile[XUA_SPDIF_TX_TILE_NUM] : clock    clk_mst_spd               = CLKBLK_SPDIF_TX;
#endif

#if (XUA_SPDIF_RX_EN)
on tile[XUA_XUD_TILE_NUM] : clock    clk_spd_rx                     = CLKBLK_SPDIF_RX;
#endif

on tile[XUA_AUDIO_IO_TILE_NUM] : clock clk_audio_mclk               = CLKBLK_MCLK;       /* Master clock */

#if (XUA_AUDIO_IO_TILE_NUM != XUA_XUD_TILE_NUM) && XUA_USB_EN
/* Separate clock/port for USB feedback calculation */
on tile[XUA_XUD_TILE_NUM] : clock clk_audio_mclk_usb                = CLKBLK_MCLK;       /* Master clock */
#endif

on tile[XUA_AUDIO_IO_TILE_NUM] : clock clk_audio_bclk               = CLKBLK_I2S_BIT;    /* Bit clock */

#if XUA_USB_EN
/* Endpoint type tables for XUD */
XUD_EpType epTypeTableOut[ENDPOINT_COUNT_OUT] = { XUD_EPTYPE_CTL | XUD_STATUS_ENABLE,
#if (NUM_USB_CHAN_OUT > 0)
                                            XUD_EPTYPE_ISO,    /* Audio */
#endif
#ifdef MIDI
                                            XUD_EPTYPE_BUL,    /* MIDI */
#endif
#if HID_OUT_REQUIRED
                                            XUD_EPTYPE_INT,
#endif
                                        };

XUD_EpType epTypeTableIn[ENDPOINT_COUNT_IN] = { XUD_EPTYPE_CTL | XUD_STATUS_ENABLE,
#if (NUM_USB_CHAN_IN > 0)
                                            XUD_EPTYPE_ISO,
#endif
#if (NUM_USB_CHAN_OUT > 0) && ((NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP))
                                            XUD_EPTYPE_ISO,    /* Async feedback endpoint */
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                                            XUD_EPTYPE_INT,
#endif
#ifdef MIDI
                                            XUD_EPTYPE_BUL,
#endif
#if XUA_OR_STATIC_HID_ENABLED
                                            XUD_EPTYPE_INT,
#endif
                                        };
#endif /* XUA_USB_EN */

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

#if (XUA_SPDIF_TX_EN) && (XUA_SPDIF_TX_TILE_NUM != XUA_AUDIO_IO_TILE_NUM)
void SpdifTxWrapper(chanend c_spdif_tx)
{
    unsigned portId;
    //configure_clock_src(clk, p_mclk);

    // TODO could share clock block here..
    // NOTE, Assuming SPDIF tile == USB tile here..
    asm("ldw %0, dp[p_mclk_in_usb]":"=r"(portId));
    asm("setclk res[%0], %1"::"r"(clk_mst_spd), "r"(portId));
    configure_out_port_no_ready(p_spdif_tx, clk_mst_spd, 0);
    set_clock_fall_delay(clk_mst_spd, 7);
    start_clock(clk_mst_spd);

    while(1)
    {
        spdif_tx(p_spdif_tx, c_spdif_tx);
    }
}
#endif

void usb_audio_io(chanend ?c_aud_in,
#if (XUA_SPDIF_TX_EN) && (XUA_SPDIF_TX_TILE_NUM != XUA_AUDIO_IO_TILE_NUM)
    chanend c_spdif_tx,
#endif
#if (MIXER)
    chanend c_mix_ctl,
#endif
    streaming chanend ?c_spdif_rx,
    streaming chanend ?c_adat_rx,
    chanend ?c_clk_ctl,
    chanend ?c_clk_int
#if (XUA_XUD_TILE_NUM != 0)  && (XUA_AUDIO_IO_TILE_NUM == 0) && (XUA_DFU_EN == 1)
    , server interface i_dfu ?dfuInterface
#endif
#if (XUA_NUM_PDM_MICS > 0)
    , chanend c_pdm_pcm
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    , client interface pll_ref_if i_pll_ref
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
    , chanend c_audio_rate_change
#endif
#if ((XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN) && XUA_USE_SW_PLL)
    , port p_for_mclk_count_aud
    , chanend c_sw_pll
#endif
)
{
#if (MIXER)
    chan c_mix_out;
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    chan c_dig_rx;
    chan c_audio_rate_change; /* Notification of new mclk freq to clockgen and synch */
#if XUA_USE_SW_PLL
    /* Connect p_for_mclk_count_aud to clk_audio_mclk so we can count mclks/timestamp in digital rx*/
    unsigned x = 0;
    asm("ldw %0, dp[clk_audio_mclk]":"=r"(x));
    asm("setclk res[%0], %1"::"r"(p_for_mclk_count_aud), "r"(x));
#endif /* XUA_USE_SW_PLL */
#endif /* (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN) */


#if (XUA_SPDIF_TX_EN) && (XUA_SPDIF_TX_TILE_NUM == XUA_AUDIO_IO_TILE_NUM)
    chan c_spdif_tx;

    /* Setup S/PDIF tx port - note this is done before par since sharing clock-block/port */
    spdif_tx_port_config(p_spdif_tx, clk_audio_mclk, p_mclk_in, 7);
#endif

    par
    {
#if (MIXER && XUA_USB_EN)
        /* Mixer cores(s) */
        {
            thread_speed();
            mixer(c_aud_in, c_mix_out, c_mix_ctl);
        }
#endif

#if (XUA_SPDIF_TX_EN) && (XUA_SPDIF_TX_TILE_NUM == XUA_AUDIO_IO_TILE_NUM)
        while(1)
        {
            spdif_tx(p_spdif_tx, c_spdif_tx);
        }
#endif

        /* Audio I/O core (pars additional S/PDIF TX Core) */
        {
            thread_speed();
#if (MIXER)
#define AUDIO_CHANNEL c_mix_out
#else
#define AUDIO_CHANNEL c_aud_in
#endif
            XUA_AudioHub(AUDIO_CHANNEL, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc
#if (XUA_SPDIF_TX_EN) //&& (XUA_SPDIF_TX_TILE_NUM != XUA_AUDIO_IO_TILE_NUM)
                , c_spdif_tx
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                , c_dig_rx
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                , c_audio_rate_change
#endif
#if (XUA_XUD_TILE_NUM != 0) && (XUA_AUDIO_IO_TILE_NUM == 0) && (XUA_DFU_EN == 1)
                , dfuInterface
#endif
#if (XUA_NUM_PDM_MICS > 0)
                , c_pdm_pcm
#endif
            );
        }

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
        {
            /* ClockGen must currently run on same tile as AudioHub due to shared memory buffer
             * However, due to the use of an interface the pll reference signal port can be on another tile
             */
            thread_speed();
            clockGen(   c_spdif_rx,
                        c_adat_rx,
                        i_pll_ref,
                        c_dig_rx,
                        c_clk_ctl,
                        c_clk_int,
                        c_audio_rate_change
#if XUA_USE_SW_PLL
                        , p_for_mclk_count_aud
                        , c_sw_pll
#endif
                        );
        }
#endif

    } // par
}

/* USER_MAIN_GLOBALS can be defined either via xua_conf.h or by xua_conf_globals.h */
#ifdef __xua_conf_globals_h_exists__
    #include "xua_conf_globals.h"
#endif

#ifndef USER_MAIN_GLOBALS
#define USER_MAIN_GLOBALS
#endif

#ifndef USER_MAIN_DECLARATIONS
#define USER_MAIN_DECLARATIONS
#endif

#ifndef USER_MAIN_CORES
#define USER_MAIN_CORES
#endif

#ifndef USER_MAIN_TASKS
#define USER_MAIN_TASKS
#endif

    USER_MAIN_GLOBALS

/* Main for USB Audio Applications */
int main()
{
#if !XUA_USB_EN
    #define c_mix_out null
#else
    chan c_mix_out;
#endif

#ifdef MIDI
    chan c_midi;
#endif

#if (MIXER)
    chan c_mix_ctl;
#endif

#if (XUA_SPDIF_RX_EN)
    streaming chan c_spdif_rx;
#else
#define c_spdif_rx null
#endif

#if (XUA_ADAT_RX_EN)
    streaming chan c_adat_rx;
#else
#define c_adat_rx null
#endif

#if (XUA_SPDIF_TX_EN) && (XUA_SPDIF_TX_TILE_NUM != XUA_AUDIO_IO_TILE_NUM)
    chan c_spdif_tx;
#endif

#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
    chan c_clk_ctl;
    chan c_clk_int;
#else
#define c_clk_int null
#define c_clk_ctl null
#endif

#if (XUA_DFU_EN == 1)
    interface i_dfu dfuInterface;
#else
    #define dfuInterface null
#endif

#if (XUA_NUM_PDM_MICS > 0)
    chan c_pdm_pcm;
#endif

#if (((XUA_SYNCMODE == XUA_SYNCMODE_SYNC && !XUA_USE_SW_PLL) || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN) )
    interface pll_ref_if i_pll_ref;
#endif

#if ((XUA_SYNCMODE == XUA_SYNCMODE_SYNC || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN) && XUA_USE_SW_PLL)
    chan c_sw_pll;
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
    chan c_audio_rate_change; /* Notification of new mclk freq to ep_buffer */
#endif
    chan c_sof;
    chan c_xud_out[ENDPOINT_COUNT_OUT];              /* Endpoint channels for XUD */
    chan c_xud_in[ENDPOINT_COUNT_IN];

    /* Used to communicate controls/setting from XUA_Endpoint0() to the Audio/Buffering sub-system */
    chan c_aud_ctl;

#if (!MIXER)
#define c_mix_ctl null
#endif

/* USER_MAIN_DECLARATIONS can be defined either via xua_conf.h or by xua_conf_declarations.h */
#ifdef __xua_conf_declarations_h_exists__
    #include "xua_conf_declarations.h"
#endif

    USER_MAIN_DECLARATIONS

    par
    {

/* USER_MAIN_CORES can be defined either via xua_conf.h or by xua_conf_tasks.h */
#ifdef __xua_conf_cores_h_exists__
    #include "xua_conf_cores.h"
#endif
#ifdef __xua_conf_tasks_h_exists__
    #include "xua_conf_tasks.h"
#endif
        USER_MAIN_CORES
        USER_MAIN_TASKS

#if (((XUA_SYNCMODE == XUA_SYNCMODE_SYNC  && !XUA_USE_SW_PLL) || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN))
        on tile[XUA_PLL_REF_TILE_NUM]: PllRefPinTask(i_pll_ref, p_pll_ref);
#endif
        on tile[XUA_XUD_TILE_NUM]:
        par
        {
#if XUA_USB_EN
#if ((XUA_XUD_TILE_NUM == 0) && (XUA_DFU_EN == 1))
            /* Check if USB is on the flash tile (tile 0) */
            /* Expect to be distrbuted into XUA_Endpoint0() */
            [[distribute]]
            DFUHandler(dfuInterface, null);
#endif

            /* Core USB task, buffering, USB etc */
            {
#ifdef XUD_PRIORITY_HIGH
                set_core_high_priority_on();
#endif
                unsigned xudPwrCfg = (XUA_POWERMODE == XUA_POWERMODE_SELF) ? XUD_PWR_SELF : XUD_PWR_BUS;

                /* USB interface core */
                XUD_Main(c_xud_out, ENDPOINT_COUNT_OUT, c_xud_in, ENDPOINT_COUNT_IN,
                         c_sof, epTypeTableOut, epTypeTableIn, XUA_USB_BUS_SPEED, xudPwrCfg);
            }

#if (NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0) || XUA_HID_ENABLED || defined(MIDI)
            /* Core USB audio task, buffering, USB etc */
            {
                unsigned x;
                thread_speed();

                /* Attach mclk count port to mclk clock-block (for feedback) */
                //set_port_clock(p_for_mclk_count, clk_audio_mclk);
#if(SECOND_MCLK_REQUIRED)
                set_clock_src(clk_audio_mclk_usb, p_mclk_in_usb);
                set_port_clock(p_for_mclk_count, clk_audio_mclk_usb);
                start_clock(clk_audio_mclk_usb);
#else
                /* XUA_AUDIO_IO_TILE_NUM == XUA_XUD_TILE_NUM */
                /* Clock port from same clock-block as I2S */
                /* TODO remove asm() */
                asm("ldw %0, dp[clk_audio_mclk]":"=r"(x));
                asm("setclk res[%0], %1"::"r"(p_for_mclk_count), "r"(x));
                /* This clock block is started in audiohub in case we first need to connect other logic
                   e.g. digital Tx to it before starting */
#endif
                /* Endpoint & audio buffering cores - buffers all EP's other than 0 */
                XUA_Buffer(
#if (NUM_USB_CHAN_OUT > 0)
                           c_xud_out[ENDPOINT_NUMBER_OUT_AUDIO],       /* Audio Out*/
#endif
#if (NUM_USB_CHAN_IN > 0)
                           c_xud_in[ENDPOINT_NUMBER_IN_AUDIO],         /* Audio In */
#endif
#if (NUM_USB_CHAN_OUT > 0) && ((NUM_USB_CHAN_IN == 0) || defined(UAC_FORCE_FEEDBACK_EP))
                           c_xud_in[ENDPOINT_NUMBER_IN_FEEDBACK],      /* Audio FB */
#endif
#ifdef MIDI
                           c_xud_out[ENDPOINT_NUMBER_OUT_MIDI],        /* MIDI Out */ // 2
                           c_xud_in[ENDPOINT_NUMBER_IN_MIDI],          /* MIDI In */  // 4
                           c_midi,
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                           /* Audio Interrupt - only used for interrupts on external clock change */
                           c_xud_in[ENDPOINT_NUMBER_IN_INTERRUPT],
                           c_clk_int,
#endif
                           c_sof, c_aud_ctl, p_for_mclk_count
#if (XUA_HID_ENABLED)
                           , c_xud_in[ENDPOINT_NUMBER_IN_HID]
#endif
                           , c_mix_out
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
                           , c_audio_rate_change
    #if (!XUA_USE_SW_PLL)
                           , i_pll_ref
    #else
                           , c_sw_pll
    #endif
#endif
                    );
                //:
            }
#endif

            /* Endpoint 0 Core */
            {
                thread_speed();
                XUA_Endpoint0( c_xud_out[0], c_xud_in[0], c_aud_ctl, c_mix_ctl, c_clk_ctl, dfuInterface VENDOR_REQUESTS_PARAMS_);
            }

#endif /* XUA_USB_EN */
        }

#if ((XUA_SYNCMODE == XUA_SYNCMODE_SYNC || XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN) && XUA_USE_SW_PLL)
        on tile[XUA_AUDIO_IO_TILE_NUM]: sw_pll_task(c_sw_pll);
#endif

        on tile[XUA_AUDIO_IO_TILE_NUM]:
        {
            /* Audio I/O task, includes mixing etc */
            usb_audio_io(
#if (NUM_USB_CHAN_OUT > 0) || (NUM_USB_CHAN_IN > 0) || XUA_HID_ENABLED || defined(MIDI)
                /* Connect audio system to XUA_Buffer(); */
                c_mix_out
#else
                /* Connect to XUA_Endpoint0() */
                c_aud_ctl
#endif
#if (XUA_SPDIF_TX_EN) && (XUA_SPDIF_TX_TILE_NUM != XUA_AUDIO_IO_TILE_NUM)
                , c_spdif_tx
#endif
#if (MIXER)
                , c_mix_ctl
#endif
                , c_spdif_rx, c_adat_rx, c_clk_ctl, c_clk_int
#if (XUA_XUD_TILE_NUM != 0) && (XUA_AUDIO_IO_TILE_NUM == 0) && (XUA_DFU_EN == 1)
                , dfuInterface
#endif
#if (XUA_NUM_PDM_MICS > 0)
                , c_pdm_pcm
#endif
#if (XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN)
                , i_pll_ref
#endif
#if (XUA_SYNCMODE == XUA_SYNCMODE_SYNC)
                , c_audio_rate_change
#endif
#if ((XUA_SPDIF_RX_EN || XUA_ADAT_RX_EN) && XUA_USE_SW_PLL)
                , p_for_mclk_count_audio
                , c_sw_pll
#endif
            );
        }
        //:

#if (XUA_SPDIF_TX_EN) && (XUA_SPDIF_TX_TILE_NUM != XUA_AUDIO_IO_TILE_NUM)
        on tile[XUA_SPDIF_TX_TILE_NUM]:
        {
            thread_speed();
            SpdifTxWrapper(c_spdif_tx);
        }
#endif

#ifdef MIDI
        /* MIDI core */
        on tile[XUA_MIDI_TILE_NUM]:
        {
            thread_speed();
            usb_midi(p_midi_rx, p_midi_tx, clk_midi, c_midi, 0);
        }
#endif

#if (XUA_SPDIF_RX_EN)
        on tile[XUA_XUD_TILE_NUM]:
        {
            thread_speed();
            spdif_rx(c_spdif_rx, p_spdif_rx, clk_spd_rx, 192000);
        }
#endif

#if (XUA_ADAT_RX_EN)
        on stdcore[XUA_XUD_TILE_NUM] :
        {
            set_thread_fast_mode_on();

            while (1)
            {
                adatReceiver48000(p_adat_rx, c_adat_rx);
                adatReceiver44100(p_adat_rx, c_adat_rx);
            }
        }
#endif


#if XUA_USB_EN
#if (XUA_XUD_TILE_NUM != 0) && (XUA_AUDIO_IO_TILE_NUM != 0) && (XUA_DFU_EN == 1)
        /* Run flash code on its own - hope it gets combined */
        //#warning Running DFU flash code on its own
        on stdcore[0]: DFUHandler(dfuInterface, null);
#endif
#endif

#if (XUA_NUM_PDM_MICS > 0)
        /* PDM Mics running on a separate to AudioHub */
        on stdcore[XUA_MIC_PDM_TILE_NUM]:
        {
             mic_array_task(c_pdm_pcm);
        }
#endif /*XUA_NUM_PDM_MICS > 0*/
    }

    return 0;
}
#endif // ndef EXCLUDE_USB_AUDIO_MAIN
