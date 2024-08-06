// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/**
 * @file    xua_ep0_descriptors.h
 * @brief   Device Descriptors
 * @author  Ross Owen, XMOS Limited
*/

#ifndef _MSOS_DESCRIPTORS_
#define _MSOS_DESCRIPTORS_

#include <stddef.h>

#if _XUA_ENABLE_BOS_DESC

// total length, number of device caps
#define _U16(_high, _low)   ((uint16_t) (((_high) << 8) | (_low)))
#define _U16_HIGH(_u16)     ((uint8_t) (((_u16) >> 8) & 0x00ff))
#define _U16_LOW(_u16)      ((uint8_t) ((_u16)       & 0x00ff))
#define U16_TO_U8S_BE(_u16)   _U16_HIGH(_u16), _U16_LOW(_u16)
#define U16_TO_U8S_LE(_u16)   _U16_LOW(_u16), _U16_HIGH(_u16)

#define _U32_BYTE3(_u32)    ((uint8_t) ((((uint32_t) _u32) >> 24) & 0x000000ff)) // MSB
#define _U32_BYTE2(_u32)    ((uint8_t) ((((uint32_t) _u32) >> 16) & 0x000000ff))
#define _U32_BYTE1(_u32)    ((uint8_t) ((((uint32_t) _u32) >>  8) & 0x000000ff))
#define _U32_BYTE0(_u32)    ((uint8_t) (((uint32_t)  _u32)        & 0x000000ff)) // LSB

#define U32_TO_U8S_BE(_u32)   _U32_BYTE3(_u32), _U32_BYTE2(_u32), _U32_BYTE1(_u32), _U32_BYTE0(_u32)
#define U32_TO_U8S_LE(_u32)   _U32_BYTE0(_u32), _U32_BYTE1(_u32), _U32_BYTE2(_u32), _U32_BYTE3(_u32)

// BOS descriptor related defines
#define USB_DESCTYPE_BOS                    (0x0F)
#define USB_DESCTYPE_DEVICE_CAPABILITY      (0x10)
#define  DEVICE_CAPABILITY_PLATFORM  0x05
#define USB_BOS_MS_OS_20_UUID \
    0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C, \
  0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F

#define REQUEST_GET_MS_DESCRIPTOR    0x20 /* bRequest of the D2H vendor request that the host will send to read the MSOS descriptor. */

// USB Binary Device Object Store (BOS) Descriptor
typedef struct {
  uint8_t  bLength         ; ///< Size of this descriptor in bytes
  uint8_t  bDescriptorType ; ///< CONFIGURATION Descriptor Type
  uint16_t wTotalLength    ; ///< Total length of data returned for this descriptor
  uint8_t  bNumDeviceCaps  ; ///< Number of device capability descriptors in the BOS
} __attribute__((packed)) USB_Descriptor_BOS_standard_t;

// Platform device capability BOS descriptor
// MSOS 2.0 platform capability
typedef struct
{
  uint8_t bLength;
  uint8_t bDescriptorType ;
  uint8_t bDevCapabilityType;
  uint8_t bReserved;
  uint8_t PlatformCapabilityUUID[16];
  uint8_t CapabilityData[8]; // defined as Variable sized for the descriptor in general but 8 for us
} __attribute__((packed)) USB_Descriptor_BOS_platform_t;

typedef struct
{
  USB_Descriptor_BOS_standard_t usb_desc_bos_standard;
  USB_Descriptor_BOS_platform_t usb_desc_bos_platform;
} __attribute__((packed)) USB_Descriptor_BOS_t;


USB_Descriptor_BOS_standard_t desc_bos_standard =
{
  .bLength = sizeof(USB_Descriptor_BOS_standard_t),
  .bDescriptorType = USB_DESCTYPE_BOS,
  .wTotalLength = sizeof(USB_Descriptor_BOS_standard_t) + sizeof(USB_Descriptor_BOS_platform_t),
  .bNumDeviceCaps = 1
};


USB_Descriptor_BOS_platform_t desc_bos_msos_platform_capability =
{
  .bLength = sizeof(USB_Descriptor_BOS_platform_t),
  .bDescriptorType = USB_DESCTYPE_DEVICE_CAPABILITY,
  .bDevCapabilityType = DEVICE_CAPABILITY_PLATFORM,
  .bReserved = 0,
  .PlatformCapabilityUUID = {USB_BOS_MS_OS_20_UUID},
  .CapabilityData = {U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(0) /*msos2.0 desc len filled at runtime*/, REQUEST_GET_MS_DESCRIPTOR, 0}
};

//MSOS descriptor related defines
#define MSOS_PROPERTY_NAME_LEN  (42) /* bytes*/
#define MSOS_INTERFACE_GUID_LEN (80) /* bytes*/
#define DEVICE_INTERFACE_GUID_MAX_STRLEN ((MSOS_INTERFACE_GUID_LEN-4)/2) /* -4 to exclude null character + 2 extra bytes at the end. \
                                                                          /2 to go from utf-16 to single byte characters that make up the GUID string*/
// Microsoft OS 2.0 Descriptors, Table 8
#define MS_OS_20_DESCRIPTOR_INDEX 7 /* wIndex of the D2H vendor request sent by the host to read the MSOS descriptor*/

typedef enum
{
  MS_OS_20_SET_HEADER_DESCRIPTOR       = 0x00,
  MS_OS_20_SUBSET_HEADER_CONFIGURATION = 0x01,
  MS_OS_20_SUBSET_HEADER_FUNCTION      = 0x02,
  MS_OS_20_FEATURE_COMPATBLE_ID        = 0x03,
  MS_OS_20_FEATURE_REG_PROPERTY        = 0x04,
  MS_OS_20_FEATURE_MIN_RESUME_TIME     = 0x05,
  MS_OS_20_FEATURE_MODEL_ID            = 0x06,
  MS_OS_20_FEATURE_CCGP_DEVICE         = 0x07,
  MS_OS_20_FEATURE_VENDOR_REVISION     = 0x08
} microsoft_os_20_type_t;



// USB MSOS2.0 Descriptors
typedef struct {
  uint16_t wLength;
  uint16_t  wDescriptorType;
  uint32_t dwWindowsVersion;
  uint16_t  wTotalLength;
} __attribute__((packed)) MSOS_desc_header_t;

typedef struct {
  uint16_t wLength;
  uint16_t  wDescriptorType;
  uint8_t bConfigurationValue;
  uint8_t bReserved;
  uint16_t wTotalLength;
}__attribute__((packed)) MSOS_desc_cfg_subset_header_t;

typedef struct {
  uint16_t wLength;
  uint16_t  wDescriptorType;
  uint8_t bFirstInterface;
  uint8_t bReserved;
  uint16_t wSubsetLength;
}__attribute__((packed)) MSOS_desc_fn_subset_header_t;

typedef struct {
  uint16_t wLength;
  uint16_t  wDescriptorType;
  uint8_t CompatibleID[8];
  uint8_t SubCompatibleID[8];
}__attribute__((packed)) MSOS_desc_compat_id_t;


typedef struct
{
  uint16_t wLength;
  uint16_t  wDescriptorType;
  uint16_t wPropertyDataType;
  uint16_t wPropertyNameLength;
  uint8_t PropertyName[MSOS_PROPERTY_NAME_LEN];
  uint16_t wPropertyDataLength;
  uint8_t PropertyData[MSOS_INTERFACE_GUID_LEN];
}__attribute__((packed)) MSOS_desc_registry_property_t;

typedef struct {
  MSOS_desc_header_t              msos_desc_header;
  MSOS_desc_cfg_subset_header_t   msos_desc_cfg_subset_header;
  MSOS_desc_fn_subset_header_t    msos_fn_subset_header;
  MSOS_desc_compat_id_t           msos_desc_compat_id;
  MSOS_desc_registry_property_t   msos_desc_registry_property;
}__attribute__((packed)) MSOS_desc_composite_t;

typedef struct {
  MSOS_desc_header_t              msos_desc_header;
  MSOS_desc_compat_id_t           msos_desc_compat_id;
  MSOS_desc_registry_property_t   msos_desc_registry_property;
}__attribute__((packed)) MSOS_desc_simple_t;

MSOS_desc_composite_t desc_ms_os_20_composite =
{
  .msos_desc_header =
  {
    .wLength = sizeof(MSOS_desc_header_t),
    .wDescriptorType = MS_OS_20_SET_HEADER_DESCRIPTOR,
    .dwWindowsVersion = 0x06030000,
    .wTotalLength = sizeof(MSOS_desc_composite_t)
  },
  .msos_desc_cfg_subset_header =
  {
    .wLength = sizeof(MSOS_desc_cfg_subset_header_t),
    .wDescriptorType = MS_OS_20_SUBSET_HEADER_CONFIGURATION,
    .bConfigurationValue = 0,
    .bReserved = 0,
    .wTotalLength = sizeof(MSOS_desc_composite_t) - sizeof(MSOS_desc_header_t)
  },
  .msos_fn_subset_header =
  {
    .wLength = sizeof(MSOS_desc_fn_subset_header_t),
    .wDescriptorType = MS_OS_20_SUBSET_HEADER_FUNCTION,
    .bFirstInterface = INTERFACE_NUMBER_DFU,
    .bReserved = 0,
    .wSubsetLength = sizeof(MSOS_desc_composite_t) - sizeof(MSOS_desc_header_t) - sizeof(MSOS_desc_cfg_subset_header_t)
  },
  .msos_desc_compat_id =
  {
    .wLength = sizeof(MSOS_desc_compat_id_t),
    .wDescriptorType = MS_OS_20_FEATURE_COMPATBLE_ID,
    .CompatibleID = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00}, // "WINUSB\0\0"
    .SubCompatibleID = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  },
  .msos_desc_registry_property =
  {
    .wLength = sizeof(MSOS_desc_registry_property_t),
    .wDescriptorType = MS_OS_20_FEATURE_REG_PROPERTY,
    .wPropertyDataType = 0x0007, // REG_MULTI_SZ as defined in Table 15 of Microsoft OS 2.0 Descriptors Specification. Check IMPORTANT NOTE 2 in https://github.com/pbatard/libwdi/wiki/WCID-Devices
    // for why we need REG_MULTI_SZ and not REG_SZ
    .wPropertyNameLength = MSOS_PROPERTY_NAME_LEN,
    .PropertyName = {'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
                      'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00}, //"DeviceInterfaceGUIDs\0" in UTF-16
    .wPropertyDataLength = MSOS_INTERFACE_GUID_LEN,
    .PropertyData = { // "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\0\0"
                      '{', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00,
                      'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00,
                      'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00,
                      'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
                    }, //defined in the WINUSB_DEVICE_INTERFACE_GUID define and updated in the descriptor at runtime
  }
};

MSOS_desc_simple_t desc_ms_os_20_simple =
{
  .msos_desc_header =
  {
    .wLength = sizeof(MSOS_desc_header_t),
    .wDescriptorType = MS_OS_20_SET_HEADER_DESCRIPTOR,
    .dwWindowsVersion = 0x06030000,
    .wTotalLength = sizeof(MSOS_desc_simple_t)
  },
  .msos_desc_compat_id =
  {
    .wLength = sizeof(MSOS_desc_compat_id_t),
    .wDescriptorType = MS_OS_20_FEATURE_COMPATBLE_ID,
    .CompatibleID = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00},
    .SubCompatibleID = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  },
  .msos_desc_registry_property =
  {
    .wLength = sizeof(MSOS_desc_registry_property_t),
    .wDescriptorType = MS_OS_20_FEATURE_REG_PROPERTY,
    .wPropertyDataType = 0x0007, // According to IMPORTANT NOTE 2 in https://github.com/pbatard/libwdi/wiki/WCID-Devices, single interface devices could work with wPropertyDataType=REG_SZ,
    // PropertyName "DeviceInterfaceGUID\0" and PropertyData "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\0" but I haven't tried it.
    .wPropertyNameLength = MSOS_PROPERTY_NAME_LEN,
    .PropertyName = {'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
                      'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00}, //"DeviceInterfaceGUIDs\0" in UTF-16
    .wPropertyDataLength = MSOS_INTERFACE_GUID_LEN,
    .PropertyData = {
                      '{', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00,
                      'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00,
                      'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '-', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00,
                      'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, 'x', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
                    }, //defined in the WINUSB_DEVICE_INTERFACE_GUID define and updated in the descriptor at runtime
  }
};

#endif

#endif
