#include <xs1.h>
#include <print.h>
#define GLXID  0x0001
#define XS1_GLX_PERIPH_USB_ID 0x1
#define XS1_GLX_CFG_RST_MISC_ADRS 0x50
#define XS1_UIFM_PHY_CONTROL_REG 0x3c
#define XS1_UIFM_PHY_CONTROL_FORCERESET 0x0
#define XS1_GLX_CFG_USB_CLK_EN_BASE 0x3
#define XS1_GLX_CFG_USB_EN_BASE 0x2
#define XS1_GLX_PERIPH_SCTH_ID 0x3


#define XS1_UIFM_FUNC_CONTROL_REG 0xc
#define XS1_UIFM_FUNC_CONTROL_XCVRSELECT 0x0
#define XS1_UIFM_FUNC_CONTROL_TERMSELECT 0x1
int write_sswitch_reg_blind(unsigned coreid, unsigned reg, unsigned data);
void write_sswitch_reg_verify(unsigned coreid, unsigned reg, unsigned data, unsigned failval);
int write_glx_periph_word(unsigned destId, unsigned periphAddress, unsigned destRegAddr, unsigned data);
int write_glx_periph_reg(unsigned dest_id, unsigned periph_addr, unsigned dest_reg_addr, unsigned bad_packet, unsigned data_size, char buf[]);
void read_sswitch_reg_verify(unsigned coreid, unsigned reg, unsigned &data, unsigned failval);

/* Reboots XMOS device by writing to the PLL config register */
void device_reboot_implementation(chanend spare) 
{
//#ifdef ARCH_S
 #if 1

    unsigned wdata;
    char wdatac[1]; 

    write_glx_periph_word(GLXID, XS1_GLX_PERIPH_USB_ID, XS1_UIFM_FUNC_CONTROL_REG, 4);
                 //     (0<<XS1_UIFM_FUNC_CONTROL_XCVRSELECT) 
                  //  | (0<<XS1_UIFM_FUNC_CONTROL_TERMSELECT));

    // Turn off All term resistors and d+ pullup 
    // Term select and opmode 

#if 0
    /* Write to glx scratch reg so we know rebooting into DFU mode */  
    wdatac[0] = 0x77;
    write_glx_periph_reg(GLXID, XS1_GLX_PERIPH_SCTH_ID, 0x1, 0, 1, wdatac);
    
    // Issue soft boot
    wdata = 0x000c0001;
    write_sswitch_reg_verify(GLXID, XS1_GLX_CFG_RST_MISC_ADRS, wdata, 2);

    while(1); // Should reset before it executes this.
#endif
    /* Keep usb clock active, enter active mode */
    //rite_sswitch_reg(GLXID, XS1_GLX_CFG_RST_MISC_ADRS, (0 << XS1_GLX_CFG_USB_CLK_EN_BASE) | (0<<XS1_GLX_CFG_USB_EN_BASE)  );
    
    /* Now reset the phy */
   // write_glx_periph_word(GLXID, XS1_GLX_PERIPH_USB_ID, XS1_UIFM_PHY_CONTROL_REG,  (1<<XS1_UIFM_PHY_CONTROL_FORCERESET));


    /* Enable the USB clock */
    //write_sswitch_reg(GLXID, XS1_GLX_CFG_RST_MISC_ADRS, ( ( 0 << XS1_GLX_CFG_USB_CLK_EN_BASE ) ) );
#endif

#if 1
    outct(spare, XS1_CT_END); // have to do this before freeing the chanend
    inct(spare); // Receive end ct from usb_buffer to close down in both directions
    // Need a spare chanend so we can talk to the pll register
    asm("freer res[%0]"::"r"(spare));
    {
       unsigned int pllVal;
       unsigned int core_id = get_core_id();
       read_sswitch_reg(core_id, 6, pllVal);
       write_sswitch_reg_blind(core_id^0x8000, 6, pllVal);
       write_sswitch_reg_blind(core_id, 6, pllVal);
    }

    
#endif
}
