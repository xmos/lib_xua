// Copyright (c) 2011-2021, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <platform.h>
#include <print.h>
#include <xs1_su.h>

#define XS1_SU_PERIPH_USB_ID 0x1

//Normally we would enumerate the XUD_SERIES_SUPPORT possibilities using defines in
//xud.h but we have hard coded them to remove dependancy of sc_xud

#if (XUD_SERIES_SUPPORT == 4)
#include "xs2_su_registers.h"
#define XS2_SU_PERIPH_USB_ID 0x1
#define PLL_MASK 0x3FFFFFFF
#else
#define PLL_MASK 0xFFFFFFFF
#endif

/* Note, this function is prototyped in xs1.h only from 13 tools onwards */
unsigned get_tile_id(tileref);

extern tileref tile[];

/* Function to reset the given tile */
static void reset_tile(unsigned const tileId)
{
    unsigned int pllVal;

    read_sswitch_reg(tileId, 6, pllVal);
    pllVal &= PLL_MASK;
    write_sswitch_reg_no_ack(tileId, 6, pllVal);
}        

/* Reboots XMOS device by writing to the PLL config register
 * Note - resetting is per *node* not tile
 */
void device_reboot(void)
{
#if (XUD_SERIES_SUPPORT == 1)
    /* Disconnect from bus */
    unsigned data[] = {4};
    write_periph_32(usb_tile, XS1_SU_PERIPH_USB_ID, XS1_SU_PER_UIFM_FUNC_CONTROL_NUM, 1, data);

    /* Ideally we would reset SU1 here but then we loose power to the xcore and therefore the DFU flag */
    /* Disable USB and issue reset to xcore only - not analogue chip */
    write_node_config_reg(usb_tile, XS1_SU_CFG_RST_MISC_NUM,0b10);
#else

    unsigned int localTileId = get_local_tile_id();
    unsigned int tileId;
    unsigned int tileArrayLength;
    unsigned int localTileNum;

#if (XUD_SERIES_SUPPORT == 4)
    /* Disconnect from bus */
    unsigned data[] = {4};
    write_periph_32(usb_tile, XS2_SU_PERIPH_USB_ID, XS1_GLX_PER_UIFM_FUNC_CONTROL_NUM, 1, data);
#endif

    tileArrayLength = sizeof(tile)/sizeof(tileref);

    /* Note - we could be in trouble if this doesn't return 0/1 since 
     * this code doesn't properly handle any network topology other than a
     * simple line 
     */
    /* Find tile index of the local tile ID */
    for(int tileNum = 0;  tileNum<tileArrayLength; tileNum++) 
    {
        if (get_tile_id(tile[tileNum]) == localTileId)
        {
            localTileNum = tileNum;
            break;
        }
    }

#if defined(__XS2A__) || defined(__XS3A__)
    /* Reset all even tiles, starting from the remote ones */
    for(int tileNum = tileArrayLength-2;  tileNum>=0; tileNum-=2)
#else
    /* Reset all tiles, starting from the remote ones */
    for(int tileNum = tileArrayLength-1;  tileNum>=0; tileNum--)
#endif 
    {
        /* Cannot cast tileref to unsigned! */
        tileId = get_tile_id(tile[tileNum]);

        /* Do not reboot local tile (or tiles residing on the same node) yet */
#if defined(__XS2A__) || defined(__XS3A__)
        if((localTileNum | 1) != (tileNum | 1))

#else
        if(localTileNum != tileNum)
#endif    
        {
            reset_tile(tileId);
        }
    }

    /* Finally reboot the node this tile resides on */
    reset_tile(localTileId);
#endif

    while (1);
}
