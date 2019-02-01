// Copyright (c) 2019, XMOS Ltd, All rights reserved
#ifndef __xua_h__
#define __xua_h__

#include <xs1.h>

#define DFU_VENDOR_ID 0x20B1
#define DFU_PID 19
#define BCD_DEVICE 0x220

#define XUA_DFU_EN 1

#define QUAD_SPI_FLASH 1

#define FL_QUADDEVICE_ISSI_IS25LQ016B_12_5MHZ \
{ \
  20,                     /* Enum value to identify the flashspec in a list */ \
  256,                    /* page size */ \
  8192,                   /* num pages */ \
  3,                      /* address size */ \
  4,                      /* log2 clock divider */ \
  0x9F,                   /* QSPI_RDID */ \
  0,                      /* id dummy bytes */ \
  3,                      /* id size in bytes */ \
  0x9D4015,               /* device id */ \
  0x20,                   /* QSPI_SE */ \
  4096,                   /* Sector erase is always 4KB */ \
  0x06,                   /* QSPI_WREN */ \
  0x04,                   /* QSPI_WRDI */ \
  PROT_TYPE_NONE,         /* no protection */ \
  {{0,0},{0x00,0x00}},    /* QSPI_SP, QSPI_SU */ \
  0x02,                   /* QSPI_PP */ \
  0xEB,                   /* QSPI_READ_FAST */ \
  1,                      /* 1 read dummy byte */ \
  SECTOR_LAYOUT_REGULAR,  /* mad sectors */ \
  {4096,{0,{0}}},        /* regular sector sizes */ \
  0x05,                   /* QSPI_RDSR */ \
  0x01,                   /* QSPI_WRSR */ \
  0x01,                   /* QSPI_WIP_BIT_MASK */ \
}

#define DFU_FLASH_DEVICE FL_QUADDEVICE_ISSI_IS25LQ016B_12_5MHZ

#define FLASH_MAX_UPGRADE_SIZE (512 * 1024)

#endif
