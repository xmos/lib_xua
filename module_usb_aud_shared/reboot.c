#include <xs1.h>
#include <print.h>


int write_sswitch_reg_blind(unsigned coreid, unsigned reg, unsigned data);

/* Reboots XMOS device by writing to the PLL config register */
void device_reboot(void) 
{
    unsigned int pllVal;
    unsigned int core_id = get_core_id();
    read_sswitch_reg(core_id, 6, &pllVal);
    write_sswitch_reg_blind(core_id^0x8000, 6, pllVal);
    write_sswitch_reg_blind(core_id, 6, pllVal);
}
