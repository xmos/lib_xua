
#include <xs1.h>
#include <platform.h>
#include <print.h>

void powerDown()
{
    unsigned data = 1;
    //read_tile_config_reg(tile[0], XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, data);
    //printintln(data);
    write_tile_config_reg(tile[0], XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, data);
    //setps(XS1_PS_XCORE_CTRL0, 0x00000010); // Enable the core clock divider
}

void powerUp()
{
    unsigned data = 0;
    //write_tile_config_reg(tile[0], XS1_PSWITCH_PLL_CLK_DIVIDER_NUM, data);
    //setps(XS1_PS_XCORE_CTRL0, 0x00000000);
}
