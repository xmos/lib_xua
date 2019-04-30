
/* A very simple *example* of I2S loopback (analog in to analog out)
 * using the I2S module built into lib_xua (usb audio library)
 *
 *
 */

#include <xs1.h>
#include <platform.h>
#include <xscope.h>

#include "xua_commands.h"
#include "xua.h"

/* Port declarations. Note, the defines come from the xn file */
buffered out port:32 p_i2s_dac[]    = {I2S_MIC_DATA};   /* I2S Data-line(s) */
buffered in port:32 p_i2s_adc[]    	= {I2S_DATA_IN};   /* I2S Data-line(s) */

#if defined(CODEC_MASTER) && (CODEC_MASTER != 0)
on tile[AUDIO_IO_TILE] : buffered in port:32 p_lrclk  = PORT_I2S_LRCLK;
on tile[AUDIO_IO_TILE] : buffered in port:32 p_bclk   = PORT_I2S_BCLK;
#else
on tile[AUDIO_IO_TILE] : buffered out port:32 p_lrclk = PORT_I2S_LRCLK;
on tile[AUDIO_IO_TILE] : buffered out port:32 p_bclk  = PORT_I2S_BCLK;
#endif


/* Port declarations. Note, the defines come from the xn file */
/* Master clock for the audio IO tile */
//on tile[1]: in port p_mclk_in_tile1             = PORT_MCLK_TILE1;
on tile[0]: in port p_mclk_in                   = PORT_PDM_MCLK;


/* Resources for USB feedback */
in port p_for_mclk_count            = PORT_MCLK_COUNT;   /* Extra port for counting master clock ticks */

/* Clock-block declarations */
clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_3;   /* Bit clock */
clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_2;   /* Master clock */
//clock clk_audio_mclk_usb            = on tile[1]: XS1_CLKBLK_1;   /* Master clock for USB tile */



on tile[0] : clock mclk_internal = XS1_CLKBLK_5;

void set_node_pll_reg(tileref tile_ref, unsigned reg_val){
    write_sswitch_reg(get_tile_id(tile_ref), XS1_SSWITCH_PLL_CTL_NUM, reg_val);
}

void run_clock(void) {
    configure_clock_xcore(mclk_internal, 10); // 24.576 MHz
    configure_port_clock_output(p_mclk_in, mclk_internal);
    start_clock(mclk_internal);
}

// Nominal setting is ref div = 25, fb_div = 1024, op_div = 2
// PCF Freq 0.96MHz

#define PLL_NOM  0xC003FF18 // This is 3.072MHz

void set_pll(void) {
    set_node_pll_reg(tile[0], PLL_NOM);
    run_clock();
}

void loopback(chanend c_i2s)
{
    int samps[NUM_USB_CHAN_OUT] = {0};
    int sampFreq = FREQ;

    /* This block is the protocol for sample rate change */
    inuint(c_i2s);
    outct(c_i2s, SET_SAMPLE_FREQ);
    outuint(c_i2s, sampFreq);
    chkct(c_i2s, XS1_CT_END);

    while(1)
    {
        inuint(c_i2s);
        for (int i=0; i<NUM_USB_CHAN_OUT; i++) outuint(c_i2s, samps[i]);
        for (int i=0; i<NUM_USB_CHAN_IN; i++) samps[i] = inuint(c_i2s);

        xscope_int(0, samps[0]);
    }
}

int main()
{
    chan c_i2s;

    par {
        on tile[0]: {
            if (CODEC_MASTER == 0) set_pll();
            par {
                    XUA_AudioHub(c_i2s, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc); //I2S
                    loopback(c_i2s);
            }
        }
    }
    return 0;
}


