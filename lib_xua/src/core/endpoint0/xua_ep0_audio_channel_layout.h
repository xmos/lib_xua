// Copyright 2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __XUA_EP0_AUDIO_CHANNEL_LAYOUT__
#define __XUA_EP0_AUDIO_CHANNEL_LAYOUT__

#ifndef __ASSEMBLER__

// Audio Class-Audio Channel Configuration UAC2
// Section 3.13.1, USB Device Class Definition for Audio Devices (Release 2.0)
typedef enum
{
  AUD20_CH_CONFIG_NON_PREDEFINED             = 0x00000000,
  AUD20_CH_CONFIG_FRONT_LEFT                 = 0x00000001,
  AUD20_CH_CONFIG_FRONT_RIGHT                = 0x00000002,
  AUD20_CH_CONFIG_FRONT_CENTER               = 0x00000004,
  AUD20_CH_CONFIG_LOW_FRQ_EFFECTS            = 0x00000008,
  AUD20_CH_CONFIG_BACK_LEFT                  = 0x00000010,
  AUD20_CH_CONFIG_BACK_RIGHT                 = 0x00000020,
  AUD20_CH_CONFIG_FRONT_LEFT_OF_CENTER       = 0x00000040,
  AUD20_CH_CONFIG_FRONT_RIGHT_OF_CENTER      = 0x00000080,
  AUD20_CH_CONFIG_BACK_CENTER                = 0x00000100,
  AUD20_CH_CONFIG_SIDE_LEFT                  = 0x00000200,
  AUD20_CH_CONFIG_SIDE_RIGHT                 = 0x00000400,
  AUD20_CH_CONFIG_TOP_CENTER                 = 0x00000800,
  AUD20_CH_CONFIG_TOP_FRONT_LEFT             = 0x00001000,
  AUD20_CH_CONFIG_TOP_FRONT_CENTER           = 0x00002000,
  AUD20_CH_CONFIG_TOP_FRONT_RIGHT            = 0x00004000,
  AUD20_CH_CONFIG_TOP_BACK_LEFT              = 0x00008000,
  AUD20_CH_CONFIG_TOP_BACK_CENTER            = 0x00010000,
  AUD20_CH_CONFIG_TOP_BACK_RIGHT             = 0x00020000,
  AUD20_CH_CONFIG_TOP_FRONT_LEFT_OF_CENTER   = 0x00040000,
  AUD20_CH_CONFIG_TOP_FRONT_RIGHT_OF_CENTER  = 0x00080000,
  AUD20_CH_CONFIG_LEFT_LOW_FRQ_EFFECTS       = 0x00100000,
  AUD20_CH_CONFIG_RIGHT_LOW_FRQ_EFFECTS      = 0x00200000,
  AUD20_CH_CONFIG_TOP_SIDE_LEFT              = 0x00400000,
  AUD20_CH_CONFIG_TOP_SIDE_RIGHT             = 0x00800000,
  AUD20_CH_CONFIG_BOTTOM_CENTER              = 0x01000000,
  AUD20_CH_CONFIG_BACK_LEFT_OF_CENTER        = 0x02000000,
  AUD20_CH_CONFIG_BACK_RIGHT_OF_CENTER       = 0x04000000,
  AUD20_CH_CONFIG_RAW_DATA                   = 0x80000000,
} audio20_channel_config_t;

// Audio Class-Audio Channel Configuration UAC1
// Section 3.7.2.3, USB Device Class Definition for Audio Devices (Release 1.0)
typedef enum {
  AUD10_CH_CONFIG_NON_PREDEFINED = 0x0000,
  AUD10_CH_CONFIG_LEFT_FRONT = 0x0001,
  AUD10_CH_CONFIG_RIGHT_FRONT = 0x0002,
  AUD10_CH_CONFIG_CENTER_FRONT = 0x0004,
  AUD10_CH_CONFIG_LOW_FRQ_EFFECTS = 0x0008,
  AUD10_CH_CONFIG_LEFT_SURROUND = 0x0010,
  AUD10_CH_CONFIG_RIGHT_SURROUND = 0x0020,
  AUD10_CH_CONFIG_LEFT_OF_CENTER = 0x0040,
  AUD10_CH_CONFIG_RIGHT_OF_CENTER = 0x0080,
  AUD10_CH_CONFIG_SURROUND = 0x0100,
  AUD10_CH_CONFIG_SIDE_LEFT = 0x0200,
  AUD10_CH_CONFIG_SIDE_RIGHT = 0x0400,
  AUD10_CH_CONFIG_TOP = 0x0800,
} audio10_channel_config_t;


/**
 * @brief Default UAC2.0 channel-position bitmap for a given channel count.
 *
 * Returns a USB Audio Class 2.0 (UAC2) channel configuration mask for
 * commonly used channel counts.
 *
 * Note:
 * Microsoft Windows only recognizes a limited set of standard speaker
 * layouts for USB audio devices: Mono, Stereo, Quadraphonic, 5.1 Surround,
 * and 7.1 Surround. These are the configurations exposed in the Sound
 * control panel. The default mappings provided here follow these layouts
 * for maximum compatibility.
 *
 * @param ch Number of channels in the audio stream.
 *
 * @return Channel-position bitmap corresponding to the default layout for
 *         the given channel count, or AUD20_CH_CONFIG_NON_PREDEFINED if no
 *         standard layout is defined.
 */
#define AUDIO20_DEFAULT_CHANNEL_MASK(ch) \
( \
  (ch) == 1 ? AUD20_CH_CONFIG_FRONT_CENTER : \
  (ch) == 2 ? (AUD20_CH_CONFIG_FRONT_LEFT | AUD20_CH_CONFIG_FRONT_RIGHT) : \
  (ch) == 4 ? (AUD20_CH_CONFIG_FRONT_LEFT | AUD20_CH_CONFIG_FRONT_RIGHT | \
               AUD20_CH_CONFIG_BACK_LEFT | AUD20_CH_CONFIG_BACK_RIGHT) : \
  (ch) == 6 ? (AUD20_CH_CONFIG_FRONT_LEFT | AUD20_CH_CONFIG_FRONT_RIGHT | \
               AUD20_CH_CONFIG_FRONT_CENTER | AUD20_CH_CONFIG_LOW_FRQ_EFFECTS | \
               AUD20_CH_CONFIG_BACK_LEFT | AUD20_CH_CONFIG_BACK_RIGHT) : \
  (ch) == 8 ? (AUD20_CH_CONFIG_FRONT_LEFT | AUD20_CH_CONFIG_FRONT_RIGHT | \
               AUD20_CH_CONFIG_FRONT_CENTER | AUD20_CH_CONFIG_LOW_FRQ_EFFECTS | \
               AUD20_CH_CONFIG_BACK_LEFT | AUD20_CH_CONFIG_BACK_RIGHT | \
               AUD20_CH_CONFIG_SIDE_LEFT | AUD20_CH_CONFIG_SIDE_RIGHT) : \
  AUD20_CH_CONFIG_NON_PREDEFINED \
)

/**
 * @brief Default UAC1.0 channel-position bitmap for a given channel count.
 *
 * Returns a USB Audio Class 1.0 (UAC1) channel configuration bitmap
 * (wChannelConfig) for common channel counts.
 *
 * @param ch Number of channels in the audio stream.
 *
 * @return Channel-position bitmap for the given channel count, or
 *         AUD10_CH_CONFIG_NON_PREDEFINED if no predefined layout.
 */
#define AUDIO10_DEFAULT_CHANNEL_MASK(ch) \
( \
  (ch) == 1 ? AUD10_CH_CONFIG_CENTER_FRONT : \
  (ch) == 2 ? (AUD10_CH_CONFIG_LEFT_FRONT | AUD10_CH_CONFIG_RIGHT_FRONT) : \
  AUD10_CH_CONFIG_NON_PREDEFINED \
)

#endif /* __ASSEMBLER__ */

#endif