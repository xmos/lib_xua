/**
 * @brief       Defines relating to device configuration and customisation.
 * @author      Ross Owen, XMOS Limited
 */

#ifndef _DEVICEDEFINES_H_
#define _DEVICEDEFINES_H_

#include "customdefines.h"

/* Default tile arrangement */
#ifndef AUDIO_IO_TILE
#define AUDIO_IO_TILE   0
#endif

#ifndef XUD_TILE
#define XUD_TILE        0
#endif

#ifndef IAP_TILE
#define IAP_TILE        AUDIO_IO_TILE
#endif

#ifndef MIDI_TILE
#define MIDI_TILE       AUDIO_IO_TILE
#endif

/* Tidy up historical INPUT/OUTPUT defines. INPUT/OUTPUT now enabled based on channel count defines */
#if !defined(NUM_USB_CHAN_IN)
    #error NUM_USB_CHAN_IN must be defined!
#else
    #if (NUM_USB_CHAN_IN == 0)
        #undef INPUT
    #else
        #define INPUT 1
    #endif
#endif

#if !defined(NUM_USB_CHAN_OUT)
    #error NUM_USB_CHAN_OUT must be defined!
#else
    #if (NUM_USB_CHAN_OUT == 0)
        #undef OUTPUT
    #else
        #define OUTPUT 1
    #endif
#endif

#if defined(DSD_CHANS_DAC)
    #if defined(NATIVE_DSD) && (NATIVE_DSD == 0)
        #undef NATIVE_DSD
    #else
        #define NATIVE_DSD       1  /* Always enable Native DSD when DSD mode is enabled */
    #endif
#else
    #define DSD_CHANS_DAC        0
#endif

/* Max supported sample freq for device */
#ifndef MAX_FREQ
#define MAX_FREQ                 (192000)
#endif

/* Min supported sample freq for device */
#ifndef MIN_FREQ
#define MIN_FREQ                 (44100)
#endif

#if ((MCLK_44 % MIN_FREQ) == 0)
#define MIN_FREQ_44 MIN_FREQ
#define MIN_FREQ_48 ((48000 * 512)/((44100 * 512)/MIN_FREQ))
#endif

#if ((MCLK_48 % MIN_FREQ) == 0)
#define MIN_FREQ_48 MIN_FREQ
/* * 2 required since we want the next 44.1 based freq above MIN_FREQ */
#define MIN_FREQ_44 (((44100*512)/((48000 * 512)/MIN_FREQ))*2)
#endif

/* For Audio Class 1.0 and Full-speed Audio 2.0 we always have at most 2 channels */
#if (NUM_USB_CHAN_OUT > 2)
#define NUM_USB_CHAN_OUT_FS  (2)
#else
#define NUM_USB_CHAN_OUT_FS  (NUM_USB_CHAN_OUT)
#endif

#if (NUM_USB_CHAN_IN > 2)
#define NUM_USB_CHAN_IN_FS (2)
#else
#define NUM_USB_CHAN_IN_FS  (NUM_USB_CHAN_IN)
#endif

/* Apply sample-rate restrictions to full-speed operation */
#if (NUM_USB_CHAN_OUT_FS > 0) && (NUM_USB_CHAN_IN_FS > 0)
    #if(MAX_FREQ > 48000)
        #define MAX_FREQ_FS              48000
    #else
        #define MAX_FREQ_FS              MAX_FREQ
    #endif
#else
    #if (MAX_FREQ > 96000)
        #define MAX_FREQ_FS              96000
    #else
        #define MAX_FREQ_FS              MAX_FREQ
    #endif
#endif

#if defined(IAP) && (IAP == 0)
#undef IAP
#endif

#if defined(IAP_INT_EP) && (IAP_INT_EP == 0)
#undef IAP_INT_EP
#endif

#if defined(SU1_ADC_ENABLE) && (SU1_ADC_ENABLE == 0)
#undef SU1_ADC_ENABLE
#endif

#if defined(HID_CONTROLS) && (HID_CONTROLS == 0)
#undef HID_CONTROLS
#endif

#if defined(MIDI) && (MIDI == 0)
#undef MIDI
#endif

#if defined(SPDIF) && (SPDIF == 0)
#undef SPDIF
#endif

#if defined(INPUT) && (INPUT == 0)
#undef INPUT
#endif

#if defined(OUTPUT) && (OUTPUT == 0)
#undef OUTPUT
#endif

#if defined(SPDIF_RX) && (SPDIF_RX == 0)
#undef SPDIF_RX
#endif

#if defined(ADAT_RX) && (ADAT_RX == 0)
#undef ADAT_RX
#endif

#if !defined(DFU)
/* Enable DFU by default */
#define DFU          1
#elif defined(DFU) && (DFU == 0)
#undef DFU
#endif

#if defined(CODEC_MASTER) && (CODEC_MASTER == 0)
#undef CODEC_MASTER
#endif

#if defined(LEVEL_METER_LEDS) && !defined(LEVEL_UPDATE_RATE)
#define LEVEL_UPDATE_RATE   400000
#endif

/* USB Audio Class Version. Default to 2.0  */
#ifndef AUDIO_CLASS
#define AUDIO_CLASS 2
#endif

/* Whether or not to fall back to Audio Class 1.0 in USB Full-speed */
#ifndef AUDIO_CLASS_FALLBACK
#define AUDIO_CLASS_FALLBACK  0     /* Default to not falling back to UAC 1 */
#endif

#if defined(AUDIO_CLASS_FALLBACK) && (AUDIO_CLASS_FALLBACK==0)
#undef AUDIO_CLASS_FALLBACK
#endif

/* Whether to run in Audio Class 2.0 mode in USB Full-speed */
#if !defined(FULL_SPEED_AUDIO_2) && !defined(AUDIO_CLASS_FALLBACK)
#define FULL_SPEED_AUDIO_2    1     /* Default to falling back to UAC2 */
#endif

#if defined(FULL_SPEED_AUDIO_2) && (FULL_SPEED_AUDIO_2 == 0)
#undef FULL_SPEED_AUDIO_2
#endif

/* Some checks on full-speed functionality */
#if defined(FULL_SPEED_AUDIO_2) && defined(AUDIO_CLASS_FALLBACK)
#error FULL_SPEED_AUDIO_2 and AUDIO_CLASS_FALLBACK enabled!
#endif

#if (AUDIO_CLASS == 1) && defined(FULL_SPEED_AUDIO_2)
#error AUDIO_CLASS set to 1 and FULL_SPEED_AUDIO_2 enabled!
#endif

/* Number of IS2 chans to DAC */
#ifndef I2S_CHANS_DAC
#error I2S_CHANS_DAC not defined
#else
#define I2S_WIRES_DAC               (I2S_CHANS_DAC >> 1)
#endif

/* Number of I2S chans from ADC */
#ifndef I2S_CHANS_ADC
#error I2S_CHANS_ADC not defined
#else
#define I2S_WIRES_ADC               (I2S_CHANS_ADC >> 1)
#endif

/* SPDIF and ADAT first input chan indices */
#ifdef SPDIF_RX
#ifndef SPDIF_RX_INDEX
#error SPDIF_RX_INDEX not defined and SPDIF_RX defined
#endif
#endif

#ifdef ADAT_RX
#ifndef ADAT_RX_INDEX
#error ADAT_RX_INDEX not defined and ADAT_RX defined
#endif
#endif

/* S/PDIF Tx channel index */
#ifndef SPDIF_TX_INDEX
#define SPDIF_TX_INDEX                  (0)
#endif

/* Default device sample frequency */
#ifndef DEFAULT_FREQ
#define DEFAULT_FREQ                   (MIN_FREQ)
#endif

/* Master clock defines (in Hz) */
#ifndef MCLK_441
#error MCLK_441 not defined
#endif

#ifndef MCLK_48
#error MCLK_441 not defined
#endif

#if ((MCLK_441 % DEFAULT_FREQ) == 0)
#define DEFAULT_MCLK_FREQ MCLK_441
#elif ((MCLK_48 % DEFAULT_FREQ) == 0)
#define DEFAULT_MCLK_FREQ MCLK_48
#else
#error Bad DEFAULT_MCLK_FREQ
#endif

/* The number of clock ticks to wait for the audio feeback to stabalise
 * Note, feedback always counts 128 SOFs (16ms @ HS, 128ms @ FS) */
#ifndef FEEDBACK_STABILITY_DELAY_HS
#define FEEDBACK_STABILITY_DELAY_HS     (2000000)
#endif

#ifndef FEEDBACK_STABILITY_DELAY_FS
#define FEEDBACK_STABILITY_DELAY_FS     (20000000)
#endif

/* Vendor String */
#ifndef VENDOR_STR
#define VENDOR_STR               "XMOS"
#endif

/* USB Vendor ID */
#ifndef VENDOR_ID
#define VENDOR_ID                (0x20B1)
#endif

/* USB Product String */
#ifdef PRODUCT_STR
#define PRODUCT_STR_A2 PRODUCT_STR
#define PRODUCT_STR_A1 PRODUCT_STR
#endif

/* Product string for Audio Class 2.0 mode */
#ifndef PRODUCT_STR_A2
#define PRODUCT_STR_A2           "xCORE USB Audio 2.0"
#endif

/* Product string for Audio Class 1.0 mode */
#ifndef PRODUCT_STR_A1
#define PRODUCT_STR_A1           "xCORE USB Audio 1.0"
#endif

/* USB Product ID (PID) for Audio Class 1.0 mode */
#if (AUDIO_CLASS==1) || defined(AUDIO_CLASS_FALLBACK)
#ifndef PID_AUDIO_1
#define PID_AUDIO_1              (0x0003)
#endif
#endif

/* USB Product ID (PID) for Audio Class 2.0 mode */
#ifndef PID_AUDIO_2
#define PID_AUDIO_2              (0x0002)
#endif

/* Device release number in BCD: 0xJJMN */
#define BCD_DEVICE_J             6
#define BCD_DEVICE_M             5
#define BCD_DEVICE_N             1

#ifndef BCD_DEVICE
#define BCD_DEVICE               ((BCD_DEVICE_J << 8) | ((BCD_DEVICE_M & 0xF) << 4) | (BCD_DEVICE_N & 0xF))
#endif

#ifndef OUTPUT_FORMAT_COUNT
    #ifndef NATIVE_DSD
        /* Default format count is 2 (16bit, 24bit) */
        #define OUTPUT_FORMAT_COUNT 2
    #else
        /* Default format count is 3 (16bit, 24bit, DSD) */
        #define OUTPUT_FORMAT_COUNT 3
    #endif
#endif

#if(OUTPUT_FORMAT_COUNT > 3)
    #error only OUTPUT_FORMAT_COUNT of 3 or less supported
#endif

#if defined(NATIVE_DSD) && (OUTPUT_FORMAT_COUNT == 1)
    #error OUTPUT_FORMAT_COUNT should be >= 2 when NATIVE_DSD enabled
#endif

#ifdef NATIVE_DSD
    /* DSD always the last format */
    #define NATIVE_DSD_FORMAT_NUM   (OUTPUT_FORMAT_COUNT)
#endif


/* Default resolutions */
/* Note, 24 on the lowests in case OUTPUT_FORMAT_COUNT = 1 */
#ifndef STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS
    #if (NATIVE_DSD_FORMAT_NUM == 1)
        #define STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS      32  /* DSD requires 32bits */
    #else
        #define STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS      24
    #endif
#endif

#ifndef STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS
#if (NATIVE_DSD_FORMAT_NUM == 2)
        #define STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS      32  /* DSD requires 32bits */
    #else
        #define STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS      16
    #endif
#endif

#ifndef STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
    #if (NATIVE_DSD_FORMAT_NUM == 3)
        #define STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS      32  /* DSD requires 32bits */
    #else
        #define STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS      32
    #endif
#endif

/* Default resolutions for HS */
#ifndef HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS
    #define HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS   STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS
#endif

#ifndef HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS
    #define HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS   STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS
#endif

#ifndef  HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
    #define HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS   STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
#endif

/* Default resolutions for FS (same as HS) */
#ifndef FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS
    #define FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS   STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS
#endif

#ifndef FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS
    #define FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS   STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS
#endif

#ifndef FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
    #define FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS   STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS
#endif

/* Setup default subslot size based on resolution
 * Catch special 24bit case where 4 byte subslot is nicer for our 32-bit machine.
 * Typically do not care about this extra bus overhead at High-speed */
#ifndef HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES
    #if (HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS == 24)
        #define HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES        4 /* 4 byte subslot is nicer for our 32 bit machine to unpack.. */
    #else
        #define HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES        (HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS/8)
    #endif
#endif

#ifndef HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES
    #if (HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS == 24)
        #define HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES        4 /* 4 byte subslot is nicer for our 32 bit machine to unpack.. */
    #else
        #define HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES        (HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS/8)
    #endif
#endif

#ifndef HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES
    #if (HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS == 24)
        #define HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES        4 /* 4 byte subslot is nicer for our 32 bit machine to unpack.. */
    #else
        #define HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES        (HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS/8)
    #endif
#endif

/* Setup default FS subslot sizes - make as small as possible */
#ifndef FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES
    #define FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES            (FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS/8)
#endif

#ifndef FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES
    #define FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES            (FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS/8)
#endif

#ifndef FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES
    #define FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES            (FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS/8)
#endif

/* Setup default formats */
#ifndef STREAM_FORMAT_OUTPUT_1_DATAFORMAT
    #if (NATIVE_DSD_FORMAT_NUM == 1)
        #define STREAM_FORMAT_OUTPUT_1_DATAFORMAT               UAC_FORMAT_TYPEI_RAW_DATA
    #else
        #define STREAM_FORMAT_OUTPUT_1_DATAFORMAT               UAC_FORMAT_TYPEI_PCM
    #endif
#endif

#ifndef STREAM_FORMAT_OUTPUT_2_DATAFORMAT
    #if (NATIVE_DSD_FORMAT_NUM == 2)
        #define STREAM_FORMAT_OUTPUT_2_DATAFORMAT               UAC_FORMAT_TYPEI_RAW_DATA
    #else
        #define STREAM_FORMAT_OUTPUT_2_DATAFORMAT               UAC_FORMAT_TYPEI_PCM
    #endif
#endif

#ifndef STREAM_FORMAT_OUTPUT_3_DATAFORMAT
    #if (NATIVE_DSD_FORMAT_NUM == 3)
        #define STREAM_FORMAT_OUTPUT_3_DATAFORMAT               UAC_FORMAT_TYPEI_RAW_DATA
    #else
        #define STREAM_FORMAT_OUTPUT_3_DATAFORMAT               UAC_FORMAT_TYPEI_PCM
    #endif
#endif

/* Some stream format checks */
#if (OUTPUT_FORMAT_COUNT > 0)
    #if !defined(HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS) || \
                                        !defined(HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES) || \
                                        !defined(STREAM_FORMAT_OUTPUT_1_DATAFORMAT)
        #error HS_OUTPUT_STREAM_1 not properly defined
    #endif
#endif

#if (OUTPUT_FORMAT_COUNT > 1)
    #if !defined(HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS) || \
                                        !defined(HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES) || \
                                        !defined(STREAM_FORMAT_OUTPUT_2_DATAFORMAT)
        #error HS_OUTPUT_STREAM_2 not properly defined
    #endif
#endif

#if (OUTPUT_FORMAT_COUNT > 2)
    #if !defined(HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS) || \
                                        !defined(HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES) || \
                                        !defined(STREAM_FORMAT_OUTPUT_3_DATAFORMAT)
        #error HS_OUTPUT_STREAM_3 not properly defined
    #endif
#endif

/* Some defines that allow us to remove unused code */

/* Useful for dropping lower part of macs in volume processing... */
#if (FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS > 24) || (FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS > 24) || \
    (FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS > 24) || (HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS > 24)  || \
    (HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS > 24) || (HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS > 24)
    #define STREAM_FORMAT_OUTPUT_RESOLUTION_32BIT_USED 1
#else
    #define STREAM_FORMAT_OUTPUT_RESOLUTION_32BIT_USED 0
#endif

/* SUBSLOT defines useful for removing packing/unpacking code in USB buffering code */
#if (FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES == 2) || (FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES == 2) || \
    (FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES == 2) || (HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES == 2) || \
    (HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES == 2) || (HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES == 2)
    #define STREAM_FORMAT_OUTPUT_SUBSLOT_2_USED 1
#else
    #define STREAM_FORMAT_OUTPUT_SUBSLOT_2_USED 0
#endif

#if (FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES == 3) || (FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES == 3) || \
    (FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES == 3) || (HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES == 3) || \
    (HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES == 3) || (HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES == 3)
    #define STREAM_FORMAT_OUTPUT_SUBSLOT_3_USED 1
#else
    #define STREAM_FORMAT_OUTPUT_SUBSLOT_3_USED 0
#endif

#if (FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES == 4) || (FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES == 4) || \
    (FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES == 4) || (HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES == 4) || \
    (HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES == 4) || (HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES == 4)
    #define STREAM_FORMAT_OUTPUT_SUBSLOT_4_USED 1
#else
    #define STREAM_FORMAT_OUTPUT_SUBSLOT_4_USED 0
#endif

//#ifdef INPUT
#if 1

    /* Only one Input stream format currently supported */
    #ifndef INPUT_FORMAT_COUNT
        #define INPUT_FORMAT_COUNT 1
    #endif

    #if (INPUT_FORMAT_COUNT > 1)
        #error
    #endif

    #ifndef STREAM_FORMAT_INPUT_1_RESOLUTION_BITS
        #define STREAM_FORMAT_INPUT_1_RESOLUTION_BITS       24
    #endif

    /* Default resolutions for HS */
    #ifndef HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS
        #define HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS        STREAM_FORMAT_INPUT_1_RESOLUTION_BITS
    #endif

    /* Default resolutions for FS (same as HS) */
    #ifndef FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS
        #define FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS        STREAM_FORMAT_INPUT_1_RESOLUTION_BITS
    #endif

    /* Setup default subslot sized based on resolution */
    #ifndef HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES
        #if (HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS == 24)
            #define HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES      4 /* 4 byte subslot is nicer for our 32 bit machine to unpack.. */
        #else
            #define HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES      (HS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS/8)
        #endif
    #endif

    /* Setup default FS subslot sizes */
    #ifndef FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES
        #define FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES         (FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS/8)
    #endif

    /* Setup default formats */
    #ifndef STREAM_FORMAT_INPUT_1_DATAFORMAT
        #define STREAM_FORMAT_INPUT_1_DATAFORMAT               UAC_FORMAT_TYPEI_PCM
    #endif

    /* Useful for dropping lower part of macs in volume processing... */
    #if (FS_STREAM_FORMAT_INPUT_1_RESOLUTION_BITS > 24) || (FS_STREAM_FORMAT_INPUT_2_RESOLUTION_BITS > 24)
        #define STREAM_FORMAT_INPUT_RESOLUTION_32BIT_USED 1
    #else
        #define STREAM_FORMAT_INPUT_RESOLUTION_32BIT_USED 0
    #endif

    #if((FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES == 4) || (HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES == 4))
        #define STREAM_FORMAT_INPUT_SUBSLOT_4_USED 1
    #else
        #define STREAM_FORMAT_INPUT_SUBSLOT_4_USED 0
    #endif

    #if((FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES == 3) || (HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES == 3))
        #define STREAM_FORMAT_INPUT_SUBSLOT_3_USED 1
    #else
        #define STREAM_FORMAT_INPUT_SUBSLOT_3_USED 0
    #endif

    #if((FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES == 2) || (HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES == 2))
        #define STREAM_FORMAT_INPUT_SUBSLOT_2_USED 1
    #else
        #define STREAM_FORMAT_INPUT_SUBSLOT_2_USED 0
    #endif
#endif

/* Endpoint addresses enums */
enum USBEndpointNumber_In
{
    ENDPOINT_NUMBER_IN_CONTROL,     /* Endpoint 0 */
    ENDPOINT_NUMBER_IN_FEEDBACK,  
    ENDPOINT_NUMBER_IN_AUDIO,
#if defined(SPDIF_RX) || defined(ADAT_RX)
    ENDPOINT_NUMBER_IN_INTERRUPT,   /* Audio interrupt/status EP */
#endif
#ifdef MIDI    
    ENDPOINT_NUMBER_IN_MIDI,
#endif
#ifdef HID_CONTROLS
    ENDPOINT_NUMBER_IN_HID,
#endif
#ifdef IAP 
#ifdef IAP_INT_EP
    ENDPOINT_NUMBER_IN_IAP_INT, 
#endif
    ENDPOINT_NUMBER_IN_IAP, 
#endif
    ENDPOINT_COUNT_IN               /* End marker */ 
};

enum USBEndpointNumber_Out
{
    ENDPOINT_NUMBER_OUT_CONTROL,    /* Endpoint 0 */
    ENDPOINT_NUMBER_OUT_AUDIO, 
#ifdef MIDI
    ENDPOINT_NUMBER_OUT_MIDI,
#endif 
#ifdef IAP 
    ENDPOINT_NUMBER_OUT_IAP,
#endif    
    ENDPOINT_COUNT_OUT              /* End marker */
};

#define AUDIO_STOP_FOR_DFU       (0x12345678)
#define AUDIO_START_FROM_DFU     (0x87654321)
#define AUDIO_REBOOT_FROM_DFU    (0xa5a5a5a5)

#define MAX_VOL                  (0x20000000)


#ifdef SELF_POWERED
    /* Default to taking no power from the bus in self-powered mode */
    #ifndef BMAX_POWER
        #define BMAX_POWER 0
    #endif
#else
    /* Default to taking 500mA from the bus in bus-powered mode */
    #ifndef BMAX_POWER
        #define BMAX_POWER 250
    #endif
#endif

/* Length of clock unit/clock-selector units */
#if defined(SPDIF_RX) && defined(ADAT_RX)
#define NUM_CLOCKS               (3)
#elif defined(SPDIF_RX) || defined(ADAT_RX)
#define NUM_CLOCKS               (2)
#else
#define NUM_CLOCKS               (1)
#endif

/* Audio Unit ID defines */
#define FU_USBIN                 11              /* Feature Unit: USB Audio device -> host */
#define FU_USBOUT                10              /* Feature Unit: USB Audio host -> device*/
#define ID_IT_USB                2               /* Input terminal: USB streaming */
#define ID_IT_AUD                1               /* Input terminal: Analogue input */
#define ID_OT_USB                22              /* Output terminal: USB streaming */
#define ID_OT_AUD                20              /* Output terminal: Analogue output */

#define ID_CLKSEL                40              /* Clock selector ID */
#define ID_CLKSRC_INT            41              /* Clock source ID (internal) */
#define ID_CLKSRC_SPDIF          42              /* Clock source ID (external) */
#define ID_CLKSRC_ADAT           43              /* Clock source ID (external) */

#define ID_XU_MIXSEL             50
#define ID_XU_OUT                51
#define ID_XU_IN                 52

#define ID_MIXER_1               60

/* Mixer defines */
#ifndef MIX_INPUTS
#define MIX_INPUTS                  18
#endif

#ifdef MIXER
    #ifndef MAX_MIX_COUNT
    #define MAX_MIX_COUNT        8
    #endif
#else
    #define MAX_MIX_COUNT        0
#endif


/* Volume defines */

#ifndef MIN_VOLUME
/* The minimum volume setting above -inf. This is a signed 8.8 fixed point
   number that must be strictly greater than -128 (0x8000) */
/* Default min volume is -127db */
#define MIN_VOLUME (0x8100)
#endif

#ifndef MAX_VOLUME
/* The maximum volume setting. This is a signed 8.8 fixed point number. */
/* Default max volume is 0db */
#define MAX_VOLUME (0x0000)
#endif

#ifndef VOLUME_RES
/* The resolution of the volume control in db as a 8.8 fixed point number */
/* Default volume resolution 1db */
#define VOLUME_RES (0x100)
#endif

#ifndef MIN_MIXER_VOLUME
/* The minimum volume setting for the mixer unit above -inf.
   This is a signed 8.8 fixed point
   number that must be strictly greater than -128 (0x8000) */
/* Default min volume is -127db */
#define MIN_MIXER_VOLUME (0x8100)
#endif

#ifndef MAX_MIXER_VOLUME
/* The maximum volume setting for the mixer.
   This is a signed 8.8 fixed point number. */
/* Default max volume is 0db */
#define MAX_MIXER_VOLUME (0x0000)
#endif

#ifndef VOLUME_RES_MIXER
/* The resolution of the volume control in db as a 8.8 fixed point number */
/* Default volume resolution 1db */
#define VOLUME_RES_MIXER (0x100)
#endif

/* Handle out volume control in the mixer */
#if defined(OUT_VOLUME_IN_MIXER) && (OUT_VOLUME_IN_MIXER==0)
#undef OUT_VOLUME_IN_MIXER
#else
#if defined(MIXER)
// Enabled by default
#define OUT_VOLUME_IN_MIXER
#endif
#endif

/* Apply out volume controls after the mix */
#if defined(OUT_VOLUME_AFTER_MIX) && (OUT_VOLUME_AFTER_MIX==0)
#undef OUT_VOLUME_AFTER_MIX
#else
#if defined(MIXER) && defined(OUT_VOLUME_IN_MIXER)
// Enabled by default
#define OUT_VOLUME_AFTER_MIX
#endif
#endif

/* Define for reporting as self or bus-powered to the host */
#if defined(SELF_POWERED) && (SELF_POWERED==0)
#undef SELF_POWERED
#endif

/* Handle in volume control in the mixer */
#if defined(IN_VOLUME_IN_MIXER) && (IN_VOLUME_IN_MIXER==0)
#undef IN_VOLUME_IN_MIXER
#else
#if defined(MIXER)
/* Enabled by default */
#define IN_VOLUME_IN_MIXER
#endif
#endif

/* Apply in volume controls after the mix */
#if defined(IN_VOLUME_AFTER_MIX) && (IN_VOLUME_AFTER_MIX==0)
#undef IN_VOLUME_AFTER_MIX
#else
#if defined(MIXER) && defined(IN_VOLUME_IN_MIXER)
// Enabled by default
#define IN_VOLUME_AFTER_MIX
#endif
#endif

/* Defines for DFU */
#define DFU_PID                     PID_AUDIO_2
#define DFU_VENDOR_ID               VENDOR_ID
#define DFU_BCD_DEVICE              BCD_DEVICE
#define DFU_MANUFACTURER_STR_INDEX  offsetof(StringDescTable_t, vendorStr)/sizeof(char *)
#define DFU_PRODUCT_STR_INDEX       offsetof(StringDescTable_t, productStr_Audio2)/sizeof(char *)
#endif

/* USB test mode support enabled by default (Required for compliance testing) */
#if defined(TEST_MODE_SUPPORT) && (TEST_MODE_SUPPORT == 0)
#undef TEST_MODE_SUPPORT
#else
#define TEST_MODE_SUPPORT  1
#endif

#if defined(FAST_MODE) && (FAST_MODE == 0)
#undef FAST_MODE
#endif

#ifndef MIDI_RX_PORT_WIDTH
#define MIDI_RX_PORT_WIDTH 1
#endif
