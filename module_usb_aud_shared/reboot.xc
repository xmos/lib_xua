#include <xs1.h>
#include <print.h>

int write_sswitch_reg_blind(unsigned coreid, unsigned reg, unsigned data);

/* Reboots XMOS device by writing to the PLL config register */
void device_reboot_implementation(chanend spare) 
{
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
}
