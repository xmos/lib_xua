#ifndef DFU_VENDOR_ID
#define DFU_VENDOR_ID          0x20B1
#endif

//#ifndef DFU_PRODUCT_ID
//#define DFU_PRODUCT_ID         0x0003
//#error DFU_PRODUCT_ID not defined!
//#endif

#ifndef PID_DFU
#error PID_DFU not defined!
#endif

#ifndef DFU_BCD_DEVICE 
#warning DFU_BCD_DEVICE NOT DEFINED
#define DFU_BCD_DEVICE        0x0000
#endif

#ifndef DFU_SERIAL_STR_INDEX
/* By default no serial string */
#define DFU_SERIAL_STR_INDEX  0x00
#endif

#ifndef DFU_PRODUCT_INDEX
#error DFU_PROFUCT_INDEX NOT DEFINED!
//#define DFU_PRODUCT_INDEX      0x1
#endif

#ifndef DFU_MANUFACTURER_INDEX
#error DFU_MANUFACTURING_INDEX NOT DEFINED!
//#define DFU_MANUFACTURER_INDEX 0x2
#endif

unsigned char DFUdevDesc[] = {
    18,        /* 0  bLength : Size of descriptor in Bytes (18 Bytes) */
    1,         /* 1  bdescriptorType */
    0,         /* 2  bcdUSB */
    2,         /* 3  bcdUSB */
    0x00,      /* 4  bDeviceClass:      See interface */
    0x00,      /* 5  bDeviceSubClass:   See interface */
    0,         /* 6  bDeviceProtocol:   See interface */
    64,        /* 7  bMaxPacketSize */
    (DFU_VENDOR_ID & 0xFF),         /* 8  idVendor */ 
    (DFU_VENDOR_ID >> 8),           /* 9  idVendor */ 
    (PID_DFU & 0xFF),               /* 10 idProduct */ 
    (PID_DFU >> 8),                 /* 11 idProduct */ 
    (DFU_BCD_DEVICE & 0xFF),        /* 12 bcdDevice : Device release number */ 
    (DFU_BCD_DEVICE >> 8),          /* 13 bcdDevice : Device release number */ 
    DFU_MANUFACTURER_INDEX,         /* 14 iManufacturer : Index of manufacturer string */
    DFU_PRODUCT_INDEX,              /* 15 iProduct : Index of product string descriptor */
    DFU_SERIAL_STR_INDEX,           /* 16 iSerialNumber : Index of serial number decriptor */
    0x01                            /* 17 bNumConfigurations : Number of possible configs */
};

unsigned char DFUcfgDesc[] = {
  /* Standard USB device descriptor */ 
    0x09,      /* 0  bLength */
    CONFIGURATION,      /* 1  bDescriptorType */
    0x1b,      /* 2  wTotalLength */
    0x00,      /* 3  wTotalLength */
    1,         /* 4  bNumInterface: Number of interfaces*/
    0x01,      /* 5  bConfigurationValue */
    0x00,      /* 6  iConfiguration */
    0xC0,      /* 7  bmAttributes */
    0x32,      /* 8  bMaxPower */

    /* Standard DFU class interface descriptor */
    0x09,      /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,      /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    0x00,      /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,      /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,      /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0xFE,      /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x01,      /* 6 bInterfaceSubclass : AUDIO_CONTROL. (field size 1 bytes) */
    0x02,      /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,      /* 8 iInterface : Unused. (field size 1 bytes) */

#if 0
  /* DFU 1.0 Standard DFU class functional descriptor */
    0x07,
    0x21,
    0x07,
    0xFA,
    0x00,
    0x40,
    0x00
#else
   /* DFU 1.1 Run-Time DFU Functional Descriptor */
    0x09,                           /* 0    Size */
    0x21,                           /* 1    bDescriptorType : DFU FUNCTIONAL */
    0x07,                           /* 2    bmAttributes */
    0xFA,                           /* 3    wDetachTimeOut */
    0x00,                           /* 4    wDetachTimeOut */
    0x40,                           /* 5    wTransferSize */
    0x00,                           /* 6    wTransferSize */
    0x10,                           /* 7    bcdDFUVersion */
    0x01,                           /* 7    bcdDFUVersion */
#endif


};

unsigned char DFUoSpeedCfgDesc[] = 
{
  /* Standard USB device descriptor */ 
    0x09,      /* 0  bLength */
    OTHER_SPEED_CONFIGURATION,      /* 1  bDescriptorType */
    0x1b,      /* 2  wTotalLength */
    0x00,      /* 3  wTotalLength */
    1,         /* 4  bNumInterface: Number of interfaces*/
    0x01,      /* 5  bConfigurationValue */
    0x00,      /* 6  iConfiguration */
    0xC0,      /* 7  bmAttributes */
    0x32,      /* 8  bMaxPower */

    /* Standard DFU class interface descriptor */
    0x09,      /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,      /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    0x00,      /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,      /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,      /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0xFE,      /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x01,      /* 6 bInterfaceSubclass : AUDIO_CONTROL. (field size 1 bytes) */
    0x02,      /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,      /* 8 iInterface : Unused. (field size 1 bytes) */

    /* DFU 1.1 Run-Time DFU Functional Descriptor */
    0x09,                           /* 0    Size */
    0x21,                           /* 1    bDescriptorType : DFU FUNCTIONAL */
    0x07,                           /* 2    bmAttributes */
    0xFA,                           /* 3    wDetachTimeOut */
    0x00,                           /* 4    wDetachTimeOut */
    0x40,                           /* 5    wTransferSize */
    0x00,                           /* 6    wTransferSize */
    0x10,                           /* 7    bcdDFUVersion */
    0x01,                           /* 7    bcdDFUVersion */
};

unsigned char DFUdevQualDesc[] = 
{
    10,                         /* 0  bLength : Size of descriptor in Bytes (18 Bytes) */
    DEVICE_QUALIFIER,           /* 1  bdescriptorType */
    0,                          /* 2  bcdUSB */
    2,                          /* 3  bcdUSB */
    0xfe,                       /* 4  bDeviceClass */
    1,                          /* 5  bDeviceSubClass */
    0,                          /* 6  bDeviceProtocol */
    64,                         /* 7  bMaxPacketSize */
    0x01,                       /* 8  bNumConfigurations : Number of possible configs */ \
    0x00                        /* 9  bReserved (must be zero) */ \
};

int DFUReportResetState(chanend ?c_user_cmd);
int DFUDeviceRequests(XUD_ep c_ep0_out, XUD_ep &?ep0_in, SetupPacket &sp, chanend ?c_user_cmd, unsigned int altInterface, unsigned int user_reset);





