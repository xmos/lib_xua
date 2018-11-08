#include <xs1.h>
#include <assert.h>
#include <platform.h>
#include <print.h>
#include <stdio.h>
#include "i2c.h"
#include "xua.h"
#define DEBUG_UNIT AUDIO_CFG
#define DEBUG_PRINT_ENABLE_AUDIO_CFG 1
#include "debug_print.h"


// TLV320DAC3101 Device I2C Address
#define DAC3101_I2C_DEVICE_ADDR 0x18

// TLV320DAC3101 Register Addresses
// Page 0
#define DAC3101_PAGE_CTRL     0x00 // Register 0 - Page Control
#define DAC3101_SW_RST        0x01 // Register 1 - Software Reset
#define DAC3101_CLK_GEN_MUX   0x04 // Register 4 - Clock-Gen Muxing
#define DAC3101_PLL_P_R       0x05 // Register 5 - PLL P and R Values
#define DAC3101_PLL_J         0x06 // Register 6 - PLL J Value
#define DAC3101_PLL_D_MSB     0x07 // Register 7 - PLL D Value (MSB)
#define DAC3101_PLL_D_LSB     0x08 // Register 8 - PLL D Value (LSB)
#define DAC3101_NDAC_VAL      0x0B // Register 11 - NDAC Divider Value
#define DAC3101_MDAC_VAL      0x0C // Register 12 - MDAC Divider Value
#define DAC3101_DOSR_VAL_LSB  0x0E // Register 14 - DOSR Divider Value (LS Byte)
#define DAC3101_CLKOUT_MUX    0x19 // Register 25 - CLKOUT MUX
#define DAC3101_CLKOUT_M_VAL  0x1A // Register 26 - CLKOUT M_VAL
#define DAC3101_CODEC_IF      0x1B // Register 27 - CODEC Interface Control
#define DAC3101_DAC_DAT_PATH  0x3F // Register 63 - DAC Data Path Setup
#define DAC3101_DAC_VOL       0x40 // Register 64 - DAC Vol Control
#define DAC3101_DACL_VOL_D    0x41 // Register 65 - DAC Left Digital Vol Control
#define DAC3101_DACR_VOL_D    0x42 // Register 66 - DAC Right Digital Vol Control
#define DAC3101_GPIO1_IO      0x33 // Register 51 - GPIO1 In/Out Pin Control
// Page 1
#define DAC3101_HP_DRVR       0x1F // Register 31 - Headphone Drivers
#define DAC3101_SPK_AMP       0x20 // Register 32 - Class-D Speaker Amp
#define DAC3101_HP_DEPOP      0x21 // Register 33 - Headphone Driver De-pop
#define DAC3101_DAC_OP_MIX    0x23 // Register 35 - DAC_L and DAC_R Output Mixer Routing
#define DAC3101_HPL_VOL_A     0x24 // Register 36 - Analog Volume to HPL
#define DAC3101_HPR_VOL_A     0x25 // Register 37 - Analog Volume to HPR
#define DAC3101_SPKL_VOL_A    0x26 // Register 38 - Analog Volume to Left Speaker
#define DAC3101_SPKR_VOL_A    0x27 // Register 39 - Analog Volume to Right Speaker
#define DAC3101_HPL_DRVR      0x28 // Register 40 - Headphone Left Driver
#define DAC3101_HPR_DRVR      0x29 // Register 41 - Headphone Right Driver
#define DAC3101_SPKL_DRVR     0x2A // Register 42 - Left Class-D Speaker Driver
#define DAC3101_SPKR_DRVR     0x2B // Register 43 - Right Class-D Speaker Driver

// TLV320DAC3101 easy register access defines
#define DAC3101_REGWRITE(reg, val) {i_i2c.write_reg(DAC3101_I2C_DEVICE_ADDR, reg, val);}



static void set_node_pll_reg(tileref tile_ref, unsigned reg_val){
    write_sswitch_reg(get_tile_id(tile_ref), XS1_SSWITCH_PLL_CTL_NUM, reg_val);
}

// Nominal setting is ref div = 25, fb_div = 1024, op_div = 2
// PCF Freq 0.96MHz

enum clock_nudge{
    PLL_SLOWER = -1,
    PLL_NOMINAL = 0,
    PLL_FASTER = 1
};

#define PLL_LOW  0xC003FE18 // This is 3.069MHz
#define PLL_NOM  0xC003FF18 // This is 3.072MHz
#define PLL_HIGH 0xC0040018 // This is 3.075MHz

on tile[0]: out port p_leds = XS1_PORT_4F;

int old_nudge = 0;
void pll_nudge(int nudge) {
    if (nudge > 0){
        set_node_pll_reg(tile[0], PLL_HIGH);
        p_leds <: 0xf;
    }
    else if (nudge < 0){
        set_node_pll_reg(tile[0], PLL_LOW);
        p_leds <: 0xf;

    }
    else {
        p_leds <: 0x0;
    }
    set_node_pll_reg(tile[0], PLL_NOM);
    //if(nudge != old_nudge && nudge){debug_printf("nudge: %d\n", nudge); }old_nudge = nudge;
}

void setup_audio_gpio(out port p_gpio){
  // Reset DAC and disable MUTE
  p_gpio <: 0x0;
  delay_milliseconds(1);
  p_gpio <: 0x1;
  delay_milliseconds(1);
}

void AudioHwConfigure(unsigned samFreq, client i2c_master_if i_i2c)
{

    // Wait for 2ms because we apply reset for 1ms from other tile
    delay_milliseconds(2);
    
    // Set register page to 0
    DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);
    // Initiate SW reset (PLL is powered off as part of reset)
    DAC3101_REGWRITE(DAC3101_SW_RST, 0x01);
    
    // so I've got 24MHz in to PLL, I want 24.576MHz or 22.5792MHz out.
    
    // I will always be using fractional-N (D != 0) so we must set R = 1
    // PLL_CLKIN/P must be between 10 and 20MHz so we must set P = 2
    
    // PLL_CLK = CLKIN * ((RxJ.D)/P)
    // We know R = 1, P = 2.
    // PLL_CLK = CLKIN * (J.D / 2)
                
    // For 24.576MHz:
    // J = 8
    // D = 1920
    // So PLL_CLK = 24 * (8.192/2) = 24 x 4.096 = 98.304MHz
    // Then:
    // NDAC = 4
    // MDAC = 4
    // DOSR = 128
    // So:
    // DAC_CLK = PLL_CLK / 4 = 24.576MHz.
    // DAC_MOD_CLK = DAC_CLK / 4 = 6.144MHz.
    // DAC_FS = DAC_MOD_CLK / 128 = 48kHz.
    
    // For 22.5792MHz:
    // J = 7
    // D = 5264
    // So PLL_CLK = 24 * (7.5264/2) = 24 x 3.7632 = 90.3168MHz
    // Then:
    // NDAC = 4
    // MDAC = 4
    // DOSR = 128
    // So:
    // DAC_CLK = PLL_CLK / 4 = 22.5792MHz.
    // DAC_MOD_CLK = DAC_CLK / 4 = 5.6448MHz.
    // DAC_FS = DAC_MOD_CLK / 128 = 44.1kHz.

#if XUA_ADAPTIVE
    //Set nominal clock speed on PLL
    write_sswitch_reg(get_tile_id(tile[0]), XS1_SSWITCH_PLL_CTL_NUM, PLL_NOM);

    // We are assuming 48kHz family only and we generate MCLK in the DAC from BLCK supplied by XCORE
    // Set PLL J Value to 8
    DAC3101_REGWRITE(DAC3101_PLL_J, 0x08);
    // Set PLL D to 0 ...
    // Set PLL D MSB Value to 0x00
    DAC3101_REGWRITE(DAC3101_PLL_D_MSB, 0x00);
    // Set PLL D LSB Value to 0x00
    DAC3101_REGWRITE(DAC3101_PLL_D_LSB, 0x00);

    delay_milliseconds(1);
    
    // Set PLL_CLKIN = BCLK (device pin), CODEC_CLKIN = PLL_CLK (generated on-chip)
    DAC3101_REGWRITE(DAC3101_CLK_GEN_MUX, 0x07);
    
    // Set PLL P=1 and R=4 values and power up.
    DAC3101_REGWRITE(DAC3101_PLL_P_R, 0x94);
    // Set NDAC clock divider to 4 and power up.
    DAC3101_REGWRITE(DAC3101_NDAC_VAL, 0x84);
    // Set MDAC clock divider to 4 and power up.
    DAC3101_REGWRITE(DAC3101_MDAC_VAL, 0x84);
    // Set OSR clock divider to 128.
    DAC3101_REGWRITE(DAC3101_DOSR_VAL_LSB, 0x80);

#else
    /* Sample frequency dependent register settings */
    if ((samFreq % 11025) == 0)
    {
        // MCLK = 22.5792MHz (44.1,88.2,176.4kHz)
        // Set PLL J Value to 7
        DAC3101_REGWRITE(DAC3101_PLL_J, 0x07);
        // Set PLL D to 5264 ... (0x1490)
        // Set PLL D MSB Value to 0x14
        DAC3101_REGWRITE(DAC3101_PLL_D_MSB, 0x14);
        // Set PLL D LSB Value to 0x90
        DAC3101_REGWRITE(DAC3101_PLL_D_LSB, 0x90);

    }
    else if ((samFreq % 8000) == 0)
    {
        // MCLK = 24.576MHz (48,96,192kHz)
        // Set PLL J Value to 8
        DAC3101_REGWRITE(DAC3101_PLL_J, 0x08);
        // Set PLL D to 1920 ... (0x780)
        // Set PLL D MSB Value to 0x07
        DAC3101_REGWRITE(DAC3101_PLL_D_MSB, 0x07);
        // Set PLL D LSB Value to 0x80
        DAC3101_REGWRITE(DAC3101_PLL_D_LSB, 0x80);
    }
    else
    {
        //debug_printf("Unrecognised sample freq of %d in ConfigCodec\n", samFreq);
    }

    delay_milliseconds(1);
    
    // Set PLL_CLKIN = MCLK (device pin), CODEC_CLKIN = PLL_CLK (generated on-chip)
    DAC3101_REGWRITE(DAC3101_CLK_GEN_MUX, 0x03);
    
    // Set PLL P and R values and power up.
    DAC3101_REGWRITE(DAC3101_PLL_P_R, 0xA1);
    // Set NDAC clock divider to 4 and power up.
    DAC3101_REGWRITE(DAC3101_NDAC_VAL, 0x84);
    // Set MDAC clock divider to 4 and power up.
    DAC3101_REGWRITE(DAC3101_MDAC_VAL, 0x84);
    // Set OSR clock divider to 128.
    DAC3101_REGWRITE(DAC3101_DOSR_VAL_LSB, 0x80);

#endif

    // Set CLKOUT Mux to DAC_CLK
    DAC3101_REGWRITE(DAC3101_CLKOUT_MUX, 0x04);
    // Set CLKOUT M divider to 1 and power up.
    DAC3101_REGWRITE(DAC3101_CLKOUT_M_VAL, 0x81);
    // Set GPIO1 output to come from CLKOUT output.
    DAC3101_REGWRITE(DAC3101_GPIO1_IO, 0x10);
    
    // Set CODEC interface mode: I2S, 24 bit, slave mode (BCLK, WCLK both inputs).
    DAC3101_REGWRITE(DAC3101_CODEC_IF, 0x20);
    // Set register page to 1
    DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x01);
    // Program common-mode voltage to mid scale 1.65V.
    DAC3101_REGWRITE(DAC3101_HP_DRVR, 0x14);
    // Program headphone-specific depop settings.
    // De-pop, Power on = 800 ms, Step time = 4 ms
    DAC3101_REGWRITE(DAC3101_HP_DEPOP, 0x4E);
    // Program routing of DAC output to the output amplifier (headphone/lineout or speaker)
    // LDAC routed to left channel mixer amp, RDAC routed to right channel mixer amp
    DAC3101_REGWRITE(DAC3101_DAC_OP_MIX, 0x44);
    // Unmute and set gain of output driver
    // Unmute HPL, set gain = 0 db
    DAC3101_REGWRITE(DAC3101_HPL_DRVR, 0x06);
    // Unmute HPR, set gain = 0 dB
    DAC3101_REGWRITE(DAC3101_HPR_DRVR, 0x06);
    // Unmute Left Class-D, set gain = 12 dB
    DAC3101_REGWRITE(DAC3101_SPKL_DRVR, 0x0C);
    // Unmute Right Class-D, set gain = 12 dB
    DAC3101_REGWRITE(DAC3101_SPKR_DRVR, 0x0C);
    // Power up output drivers
    // HPL and HPR powered up
    DAC3101_REGWRITE(DAC3101_HP_DRVR, 0xD4);
    // Power-up L and R Class-D drivers
    DAC3101_REGWRITE(DAC3101_SPK_AMP, 0xC6);
    // Enable HPL output analog volume, set = -9 dB
    DAC3101_REGWRITE(DAC3101_HPL_VOL_A, 0x92);
    // Enable HPR output analog volume, set = -9 dB
    DAC3101_REGWRITE(DAC3101_HPR_VOL_A, 0x92);
    // Enable Left Class-D output analog volume, set = -9 dB
    DAC3101_REGWRITE(DAC3101_SPKL_VOL_A, 0x92);
    // Enable Right Class-D output analog volume, set = -9 dB
    DAC3101_REGWRITE(DAC3101_SPKR_VOL_A, 0x92);
    
    delay_milliseconds(100);

    // Power up DAC
    // Set register page to 0
    DAC3101_REGWRITE(DAC3101_PAGE_CTRL, 0x00);
    // Power up DAC channels and set digital gain
    // Powerup DAC left and right channels (soft step enabled)
    DAC3101_REGWRITE(DAC3101_DAC_DAT_PATH, 0xD4);
    // DAC Left gain = 0dB
    DAC3101_REGWRITE(DAC3101_DACL_VOL_D, 0x00);
    // DAC Right gain = 0dB
    DAC3101_REGWRITE(DAC3101_DACR_VOL_D, 0x00);
    // Unmute digital volume control
    // Unmute DAC left and right channels
    DAC3101_REGWRITE(DAC3101_DAC_VOL, 0x00);
    
    i_i2c.shutdown();
}


//These are here just to silence compiler warnings
void AudioHwInit(){}
void AudioHwConfig(unsigned samFreq, unsigned mClk, unsigned dsdMode, unsigned sampRes_DAC, unsigned sampRes_ADC){}