// Copyright 2012-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xua.h"
#if (XUA_DFU_EN == 1)
#include "uac_hwresources.h"
#include <xs1.h>
#include <xclib.h>
#ifdef QUAD_SPI_FLASH
#include <quadflashlib.h>
#else
#include <flashlib.h>
#endif
#include <print.h>

#define settw(a,b) {__asm__ __volatile__("settw res[%0], %1": : "r" (a) , "r" (b));}
#define setc(a,b) {__asm__  __volatile__("setc res[%0], %1": : "r" (a) , "r" (b));}
#define setclk(a,b) {__asm__ __volatile__("setclk res[%0], %1": : "r" (a) , "r" (b));}
#define portin(a,b) {__asm__  __volatile__("in %0, res[%1]": "=r" (b) : "r" (a));}
#define portout(a,b) {__asm__  __volatile__("out res[%0], %1": : "r" (a) , "r" (b));}

#ifdef DFU_FLASH_DEVICE

#ifdef QUAD_SPI_FLASH
/* Using specified flash device rather than all supported in tools */
fl_QuadDeviceSpec flash_devices[] = {DFU_FLASH_DEVICE};
#else
/* Using specified flash device rather than all supported in tools */
fl_DeviceSpec flash_devices[] = {DFU_FLASH_DEVICE};
#endif
#endif

#ifdef QUAD_SPI_FLASH
/*
typedef struct {
      out port qspiCS;
      out port qspiSCLK;
      out buffered port:32 qspiSIO;
      clock qspiClkblk;
} fl_QSPIPorts;
*/
fl_QSPIPorts p_qflash =
{
    XS1_PORT_1B,
    XS1_PORT_1C,
    XS1_PORT_4B,
    CLKBLK_FLASHLIB
};
#else
fl_PortHolderStruct p_flash =
{
    XS1_PORT_1A,
    XS1_PORT_1B,
    XS1_PORT_1C,
    XS1_PORT_1D,
    CLKBLK_FLASHLIB
};
#endif

/* return 1 for opened ports successfully */
int flash_cmd_enable_ports()
{
    int result = 0;
#ifdef QUAD_SPI_FLASH
    /* Ports not shared */
#else
    setc(p_flash.spiMISO, XS1_SETC_INUSE_OFF);
    setc(p_flash.spiCLK, XS1_SETC_INUSE_OFF);
    setc(p_flash.spiMOSI, XS1_SETC_INUSE_OFF);
    setc(p_flash.spiSS, XS1_SETC_INUSE_OFF);
    setc(p_flash.spiClkblk, XS1_SETC_INUSE_OFF);

    setc(p_flash.spiMISO, XS1_SETC_INUSE_ON);
    setc(p_flash.spiCLK, XS1_SETC_INUSE_ON);
    setc(p_flash.spiMOSI, XS1_SETC_INUSE_ON);
    setc(p_flash.spiSS, XS1_SETC_INUSE_ON);
    setc(p_flash.spiClkblk, XS1_SETC_INUSE_ON);
    setc(p_flash.spiClkblk, XS1_SETC_INUSE_ON);

    setclk(p_flash.spiMISO, XS1_CLKBLK_REF);
    setclk(p_flash.spiCLK, XS1_CLKBLK_REF);
    setclk(p_flash.spiMOSI, XS1_CLKBLK_REF);
    setclk(p_flash.spiSS, XS1_CLKBLK_REF);

    setc(p_flash.spiMISO, XS1_SETC_BUF_BUFFERS);
    setc(p_flash.spiMOSI, XS1_SETC_BUF_BUFFERS);

    settw(p_flash.spiMISO, 8);
    settw(p_flash.spiMOSI, 8);
#endif

#ifdef DFU_FLASH_DEVICE
#ifdef QUAD_SPI_FLASH
    result = fl_connectToDevice(&p_qflash, flash_devices, sizeof(flash_devices) / sizeof(fl_QuadDeviceSpec));
#else
    result = fl_connectToDevice(&p_flash, flash_devices, sizeof(flash_devices) / sizeof(fl_DeviceSpec));
#endif
#else
    /* Use default flash list */
#ifdef QUAD_SPI_FLASH
    result = fl_connect(&p_qflash);
#else
    result = fl_connect(&p_flash);
#endif
#endif
    if (!result)
    {
        /* All okay.. */
        return 1;
    }
    else
    {
        return 0;
    }
}

int flash_cmd_disable_ports()
{
    fl_disconnect();

#ifndef QUAD_SPI_FLASH
    setc(p_flash.spiMISO, XS1_SETC_INUSE_OFF);
    setc(p_flash.spiCLK, XS1_SETC_INUSE_OFF);
    setc(p_flash.spiMOSI, XS1_SETC_INUSE_OFF);
    setc(p_flash.spiSS, XS1_SETC_INUSE_OFF);
#endif

    return 1;
}
#endif
