#include <xs1.h>
#include <platform.h>
#include <xs1_su.h>

#define XS1_SU_PERIPH_USB_ID 0x1

//Normally we would enumerate the XUD_SERIES_SUPPORT possibilities using defines in
//xud.h but we have hard coded them to remove dependancy of sc_xud

#if (XUD_SERIES_SUPPORT == 4)
#include "xs2_su_registers.h"
#define XS2_SU_PERIPH_USB_ID 0x1
#define PLL_MASK 0x7FFFFFFF
#else
#define PLL_MASK 0xFFFFFFFF
#endif

/* Note, this function is prototyped in xs1.h only from 13 tools onwards */
unsigned get_tile_id(tileref);

extern tileref tile[];

/* Function to reset the given tile */
void reset_tile(unsigned tileId)
{
    unsigned int pllVal;

    /* Cannot cast tileref to unsigned! */
    read_sswitch_reg(tileId, 6, pllVal);
    pllVal &= PLL_MASK;
    write_sswitch_reg_no_ack(tileId, 6, pllVal);
}        

void device_reboot_aux(void)
{
#if (XUD_SERIES_SUPPORT == 1)
    /* Disconnect from bus */
    unsigned data[] = {4};
    write_periph_32(usb_tile, XS1_SU_PERIPH_USB_ID, XS1_SU_PER_UIFM_FUNC_CONTROL_NUM, 1, data);

    /* Ideally we would reset SU1 here but then we loose power to the xcore and therefore the DFU flag */
    /* Disable USB and issue reset to xcore only - not analogue chip */
    write_node_config_reg(usb_tile, XS1_SU_CFG_RST_MISC_NUM,0b10);
#else
    unsigned int tileId;
    unsigned int localTileId = get_local_tile_id();
    unsigned int tileArrayLength;

#if (XUD_SERIES_SUPPORT == 4)
    /* Disconnect from bus */
    unsigned data[] = {4};
    write_periph_32(usb_tile, XS2_SU_PERIPH_USB_ID, XS1_GLX_PER_UIFM_FUNC_CONTROL_NUM, 1, data);
#endif

    /* Find size of tile array - note in future tools versions this will be available from platform.h */
    asm volatile ("ldc %0, tile.globound":"=r"(tileArrayLength));

    #ifndef __XS2A__
    /* Reset all tiles, starting from the remote ones */
    for(int i = tileArrayLength-1;  i>=0; i--)
    #else
    /* Reset all even tiles, starting from the remote ones */
    for(int i = tileArrayLength-2;  i>=0; i=i-2)
    #endif 
    {
        /* Do not reboot local tile yet! */
        if(localTileId != tileId)
        {
            reset_tile(tileId);
        }
    }
    /* Finally reboot this tile! */
    reset_tile(localTileId);
#endif
}

/* Reboots XMOS device by writing to the PLL config register */
void device_reboot(chanend spare)
{
    device_reboot_aux();
    while(1);
}
