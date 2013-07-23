#include <xs1.h>
#include <platform.h>
#include <xs1_su.h>
#include <print.h>

#define XS1_SU_PERIPH_USB_ID 0x1

/* Reboots XMOS device by writing to the PLL config register */
void device_reboot_implementation(chanend spare) 
{
#ifdef XUD_ON_U_SERIES
    /* Disconnect from bus */
    unsigned data[] = {4};
    write_periph_32(usb_tile, XS1_SU_PERIPH_USB_ID, XS1_SU_PER_UIFM_FUNC_CONTROL_NUM, 1, data);

    /* Ideally we would reset SU1 here but then we loose power to the xcore and therefore the DFU flag */
    /* Disable USB and issue reset to xcore only - not analogue chip */
    write_node_config_reg(usb_tile, XS1_SU_CFG_RST_MISC_NUM,0b10);
#else
    outct(spare, XS1_CT_END);   // have to do this before freeing the chanend
    inct(spare);                // Receive end ct from usb_buffer to close down in both directions
    
    // Need a spare chanend so we can talk to the pll register
    asm("freer res[%0]"::"r"(spare));
    
    {
       unsigned int pllVal;
       unsigned int tile_id = get_local_tile_id();
       read_sswitch_reg(tile_id, 6, pllVal);

       /* Not this accounts for 2 tiles of an L2.. */
       write_sswitch_reg_no_ack(tile_id^0x8000, 6, pllVal);
       write_sswitch_reg_no_ack(tile_id, 6, pllVal);
    }
#endif

}
