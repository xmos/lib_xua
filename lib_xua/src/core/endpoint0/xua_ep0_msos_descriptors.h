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

#define USB_DESCTYPE_BOS                    (0x0F)
#define USB_DESCTYPE_DEVICE_CAPABILITY      (0x10)

#define MS_OS_20_DESC_LEN_RUNTIME  0xB2
#define MS_OS_20_DESC_LEN_DFU  0xA2
#define REQUEST_GET_MS_DESCRIPTOR    0x20

// Microsoft OS 2.0 Descriptors, Table 8
#define MS_OS_20_DESCRIPTOR_INDEX 7

////////////////////////////////////////From TUSB code///////////////////////////////////////////////////////
// total length, number of device caps
#define TU_U16(_high, _low)   ((uint16_t) (((_high) << 8) | (_low)))
#define TU_U16_HIGH(_u16)     ((uint8_t) (((_u16) >> 8) & 0x00ff))
#define TU_U16_LOW(_u16)      ((uint8_t) ((_u16)       & 0x00ff))
#define U16_TO_U8S_BE(_u16)   TU_U16_HIGH(_u16), TU_U16_LOW(_u16)
#define U16_TO_U8S_LE(_u16)   TU_U16_LOW(_u16), TU_U16_HIGH(_u16)

#define TU_U32_BYTE3(_u32)    ((uint8_t) ((((uint32_t) _u32) >> 24) & 0x000000ff)) // MSB
#define TU_U32_BYTE2(_u32)    ((uint8_t) ((((uint32_t) _u32) >> 16) & 0x000000ff))
#define TU_U32_BYTE1(_u32)    ((uint8_t) ((((uint32_t) _u32) >>  8) & 0x000000ff))
#define TU_U32_BYTE0(_u32)    ((uint8_t) (((uint32_t)  _u32)        & 0x000000ff)) // LSB

#define U32_TO_U8S_BE(_u32)   TU_U32_BYTE3(_u32), TU_U32_BYTE2(_u32), TU_U32_BYTE1(_u32), TU_U32_BYTE0(_u32)
#define U32_TO_U8S_LE(_u32)   TU_U32_BYTE0(_u32), TU_U32_BYTE1(_u32), TU_U32_BYTE2(_u32), TU_U32_BYTE3(_u32)


#define TUD_BOS_DESC_LEN      0x05
#define  DEVICE_CAPABILITY_PLATFORM  0x05

// total length, number of device caps
#define TUD_BOS_DESCRIPTOR(_total_len, _caps_num) \
  5, USB_DESCTYPE_BOS, U16_TO_U8S_LE(_total_len), _caps_num

#define TUD_BOS_MICROSOFT_OS_DESC_LEN   28
// Device Capability Platform 128-bit UUID + Data
#define TUD_BOS_PLATFORM_DESCRIPTOR(...) \
  TUD_BOS_MICROSOFT_OS_DESC_LEN, USB_DESCTYPE_DEVICE_CAPABILITY, DEVICE_CAPABILITY_PLATFORM, 0x00, __VA_ARGS__

//------------- Microsoft OS 2.0 Platform -------------//

// Total Length of descriptor set, vendor code
#define TUD_BOS_MS_OS_20_DESCRIPTOR(_desc_set_len, _vendor_code) \
  TUD_BOS_PLATFORM_DESCRIPTOR(TUD_BOS_MS_OS_20_UUID, U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(_desc_set_len), _vendor_code, 0)

#define TUD_BOS_MS_OS_20_UUID \
    0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C, \
  0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F


///////////////////////////////////////////////////////////////////////////////////////////////
#if XUA_ENABLE_BOS_DESC
#define BOS_TOTAL_LEN      (TUD_BOS_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

unsigned char const desc_bos_runtime[] =
{
  // total length, number of device caps
  TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 1),

  // Microsoft OS 2.0 descriptor
  TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN_RUNTIME, REQUEST_GET_MS_DESCRIPTOR)
};

unsigned char const desc_bos_dfu[] =
{
  // total length, number of device caps
  TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 1),

  // Microsoft OS 2.0 descriptor
  TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN_DFU, REQUEST_GET_MS_DESCRIPTOR)
};


uint8_t desc_ms_os_20_runtime[] =
{
  // Set header: length, type, windows version, total length
  U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN_RUNTIME),

  // Configuration subset header: length, type, configuration index, reserved, configuration total length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN_RUNTIME-0x0A),

  // Function Subset header: length, type, first interface, reserved, subset length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), INTERFACE_NUMBER_DFU, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN_RUNTIME-0x0A-0x08),

  // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
  U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

  // MS OS 2.0 Registry property descriptor: length, type
  U16_TO_U8S_LE(MS_OS_20_DESC_LEN_RUNTIME-0x0A-0x08-0x08-0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
  U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
  'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
  U16_TO_U8S_LE(0x0050), // wPropertyDataLength
  // [DriverInterface] bPropertyData
  // InterfaceGUID = {89C14132-D389-4FF7-944E-2E33379BB59D}
  '{', 0x00, '8', 0x00, '9', 0x00, 'C', 0x00, '1', 0x00, '4', 0x00, '1', 0x00, '3', 0x00, '2', 0x00, '-', 0x00,
  'D', 0x00, '3', 0x00, '8', 0x00, '9', 0x00, '-', 0x00, '4', 0x00, 'F', 0x00, 'F', 0x00, '7', 0x00, '-', 0x00,
  '9', 0x00, '4', 0x00, '4', 0x00, 'E', 0x00, '-', 0x00, '2', 0x00, 'E', 0x00, '3', 0x00, '3', 0x00, '3', 0x00,
  '7', 0x00, '9', 0x00, 'B', 0x00, 'B', 0x00, '5', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};


uint8_t const desc_ms_os_20_dfu[] =
{
  // Set header: length, type, windows version, total length
  U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN_DFU),

  // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
  U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

  // MS OS 2.0 Registry property descriptor: length, type
  U16_TO_U8S_LE(MS_OS_20_DESC_LEN_DFU-0x0A-0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
  U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
  'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
  U16_TO_U8S_LE(0x0050), // wPropertyDataLength
  // [DriverInterface] bPropertyData
  // InterfaceGUID = {89C14132-D389-4FF7-944E-2E33379BB59D}
  '{', 0x00, '8', 0x00, '9', 0x00, 'C', 0x00, '1', 0x00, '4', 0x00, '1', 0x00, '3', 0x00, '2', 0x00, '-', 0x00,
  'D', 0x00, '3', 0x00, '8', 0x00, '9', 0x00, '-', 0x00, '4', 0x00, 'F', 0x00, 'F', 0x00, '7', 0x00, '-', 0x00,
  '9', 0x00, '4', 0x00, '4', 0x00, 'E', 0x00, '-', 0x00, '2', 0x00, 'E', 0x00, '3', 0x00, '3', 0x00, '3', 0x00,
  '7', 0x00, '9', 0x00, 'B', 0x00, 'B', 0x00, '5', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

#endif
