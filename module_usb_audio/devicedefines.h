/** 
 * @brief       Defines relating to device configuration and customisation.
 * @author      Ross Owen, XMOS Limited
 */

#ifndef _DEVICEDEFINES_H_
#define _DEVICEDEFINES_H_

#include "customdefines.h"

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
#define MIN_FREQ_44 (((44100*512)/((48000 * 512)/MIN_FREQ))*2)
#endif

#if (MAX_FREQ > 96000)
#define MAX_FREQ_A1              96000
#else
#define MAX_FREQ_A1              MAX_FREQ
#endif

/* For Audio Class 1.0 we always have at most 2 channels */
#if (NUM_USB_CHAN_OUT > 2)
#define NUM_USB_CHAN_OUT_A1  (2)
#else
#define NUM_USB_CHAN_OUT_A1  (NUM_USB_CHAN_OUT)
#endif

#if (NUM_USB_CHAN_IN > 2)
#define NUM_USB_CHAN_IN_A1  (2)
#else
#define NUM_USB_CHAN_IN_A1  (NUM_USB_CHAN_IN)
#endif

/* Channel count defines for FS mode */
#define NUM_USB_CHAN_OUT_FS     (NUM_USB_CHAN_OUT_A1)
#define NUM_USB_CHAN_IN_FS      (NUM_USB_CHAN_IN_A1)


#if defined(IO_EXPANSION) && (IO_EXPANSION == 0)
#undef IO_EXPANSION
#endif

#if defined(IAP) && (IAP == 0)
#undef IAP
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

#if defined(DFU) && (DFU == 0)
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
#define AUDIO_CLASS_FALLBACK 0
#endif

#if defined(AUDIO_CLASS_FALLBACK) && (AUDIO_CLASS_FALLBACK==0)
#undef AUDIO_CLASS_FALLBACK
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

/* The number of clock ticks to wait for the audio PLL to lock */
#ifndef AUDIO_PLL_LOCK_DELAY
#define AUDIO_PLL_LOCK_DELAY     (40000000) 
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

#define BCD_DEVICE_J             6
#define BCD_DEVICE_M             3
#define BCD_DEVICE_N             0

/* Device release number in BCD: 0xJJMN */
#ifndef BCD_DEVICE
#define BCD_DEVICE               ((BCD_DEVICE_J << 8) | ((BCD_DEVICE_M & 0xF) << 4) | (BCD_DEVICE_N & 0xF))
#endif

#if defined(IAP) && !defined(ACCESSORY_FIRMWARE_VERSION)
#define ACCESSORY_FIRMWARE_VERSION {BCD_DEVICE_J, BCD_DEVICE_M, BCD_DEVICE_N}
#endif

/* Addition interfaces based on defines */
#if defined(DFU) && DFU != 0
#define DFU_INTERFACES           (1)               /* DFU interface count */
#else
#define DFU_INTERFACES           (0)
#endif

#ifdef INPUT
#define INPUT_INTERFACES         (1)
#else
#define INPUT_INTERFACES         (0)
#endif

#if defined(OUTPUT) && OUTPUT != 0
#define OUTPUT_INTERFACES       (1)
#else
#define OUTPUT_INTERFACES       (0)
#endif

#define EP_CNT_OUT_AUD          (OUTPUT_INTERFACES)
#define EP_CNT_IN_AUD           (OUTPUT_INTERFACES + INPUT_INTERFACES)

#if defined(MIDI)
#define MIDI_INTERFACES         (2)
#define EP_CNT_OUT_MIDI         (1)
#define EP_CNT_IN_MIDI          (1)
#else
#define MIDI_INTERFACES         (0)
#define EP_CNT_OUT_MIDI         (0)
#define EP_CNT_IN_MIDI          (0)
#endif

#if defined(IAP)
#define IAP_INTERFACES         (1)
#else
#define IAP_INTERFACES         (0)
#endif

#if defined(HID_CONTROLS)
#define HID_INTERFACES          (1)
#else
#define HID_INTERFACES          (0)
#endif

#define EP_CNT_OUT_IAP          (IAP_INTERFACES)
#define EP_CNT_IN_IAP           (IAP_INTERFACES * 2)

#define EP_CNT_OUT_HID          (0)
#define EP_CNT_IN_HID           (HID_INTERFACES)

#if defined(SPDIF_RX) || defined(ADAT_RX)
#define EP_CNT_IN_AUD_INT        (1)
#else
#define EP_CNT_IN_AUD_INT        (0)
#endif

/* Define for number of audio interfaces (+1 for mandatory control interface) */
#define AUDIO_INTERFACES		 (INPUT_INTERFACES + OUTPUT_INTERFACES + 1) 

/* Interface number defines */
#define INTERFACE_NUM_IAP (INPUT_INTERFACES+OUTPUT_INTERFACES+MIDI_INTERFACES+DFU_INTERFACES+1)
#define INTERFACE_NUM_HID (INPUT_INTERFACES+OUTPUT_INTERFACES+MIDI_INTERFACES+DFU_INTERFACES+IAP_INTERFACES+1)

/* Endpoint Number Defines */
#define EP_NUM_IN_FB             (1)     /* Always 1 */
#define EP_NUM_IN_AUD            (2)     /* Always 2 */
#define EP_NUM_IN_AUD_INT        (EP_NUM_IN_AUD + EP_CNT_IN_AUD_INT)     /* Audio interrupt/status EP */
#define EP_NUM_IN_MIDI           (EP_NUM_IN_AUD_INT + 1)
#define EP_NUM_IN_HID            (EP_NUM_IN_AUD_INT + EP_CNT_IN_MIDI + 1)
#define EP_NUM_IN_IAP            (EP_NUM_IN_AUD_INT + EP_CNT_IN_MIDI + EP_CNT_IN_HID + 1) /* iAP Bulk */
#define EP_NUM_IN_IAP_INT        (EP_NUM_IN_AUD_INT + EP_CNT_IN_MIDI + EP_CNT_IN_HID + 2) /* iAP interrupt */

#define EP_NUM_OUT_AUD           (1)     /* Always 1 */
#define EP_NUM_OUT_MIDI          (2)     /* Always 2 */
#define EP_NUM_OUT_IAP           (EP_NUM_OUT_AUD + EP_CNT_OUT_MIDI + 1)

/* Endpoint Address Defines */
#define EP_ADR_IN_FB             (EP_NUM_IN_FB | 0x80)
#define EP_ADR_IN_AUD            (EP_NUM_IN_AUD | 0x80)
#define EP_ADR_IN_AUD_INT        (EP_NUM_IN_AUD_INT | 0x80)
#define EP_ADR_IN_MIDI           (EP_NUM_IN_MIDI | 0x80)
#define EP_ADR_IN_HID            (EP_NUM_IN_HID | 0x80)
#define EP_ADR_IN_IAP            (EP_NUM_IN_IAP | 0x80)
#define EP_ADR_IN_IAP_INT        (EP_NUM_IN_IAP_INT | 0x80)

#define EP_ADR_OUT_AUD           EP_NUM_OUT_AUD            
#define EP_ADR_OUT_MIDI          EP_NUM_OUT_MIDI           
#define EP_ADR_OUT_IAP           EP_NUM_OUT_IAP            


/* Endpoint count totals */
#define EP_CNT_OUT               (1 + 1 /*NUM_EP_OUT_AUD*/ + EP_CNT_OUT_MIDI + EP_CNT_OUT_IAP) /* +1 due to EP0 */ 
#define EP_CNT_IN                (1 + 2 /*NUM_EP_IN_AUD*/ + EP_CNT_IN_AUD_INT + EP_CNT_IN_MIDI + EP_CNT_IN_IAP + EP_CNT_IN_HID)    /* +1 due to EP0 */

#define AUDIO_STOP_FOR_DFU       (0x12345678)
#define AUDIO_START_FROM_DFU     (0x87654321)
#define AUDIO_REBOOT_FROM_DFU    (0xa5a5a5a5)

#define MAX_VOL                  (0x20000000)

#ifdef SELF_POWERED
#define BMAX_POWER 0
#else
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


/* Total number of USB interfaces this device implements (+1 for required control interface) */
#define NUM_INTERFACES          INPUT_INTERFACES + OUTPUT_INTERFACES + DFU_INTERFACES + MIDI_INTERFACES + IAP_INTERFACES + 1 + HID_INTERFACES

/* Number of interfaces for Audio 1.0 */
#define NUM_INTERFACES_A1        (1+INPUT_INTERFACES+OUTPUT_INTERFACES)


/* Audio Unit ID defines */
#define FU_USBIN                 11              /* Feature Unit: USB Audio device -> host */ 
#define FU_USBOUT                10              /* Feature Unit: USB Audio host -> device*/ 
#define ID_IT_USB                2               /* Input terminal: USB streaming */
#define ID_IT_AUD                1               /* Input terminal: Analogue input */
#define ID_OT_USB                22              /* Output terminal: USB streaming */
#define ID_OT_AUD                20              /* Output terminal: Analogue output */

#define ID_CLKSEL                40              /* Clock selector ID */
#define ID_CLKSRC_INT            41              /* Clock source ID (internal) */
#define ID_CLKSRC_EXT            42              /* Clock source ID (external) */
#define ID_CLKSRC_ADAT           43              /* Clock source ID (external) */

#define ID_XU_MIXSEL             50
#define ID_XU_OUT                51
#define ID_XU_IN                 52

#define ID_MIXER_1               60

#define MANUFACTURER_STR_INDEX	 0x01
#define PRODUCT_STR_INDEX_A2     0x03

/* Mixer defines */
#ifndef MIX_INPUTS
#define MIX_INPUTS                  18
#endif

#ifndef MAX_MIX_COUNT
#define MAX_MIX_COUNT               8
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
#define DFU_MANUFACTURER_STR_INDEX  MANUFACTURER_STR_INDEX
#define DFU_PRODUCT_STR_INDEX       PRODUCT_STR_INDEX_A2
#endif

#if defined(FAST_MODE) && (FAST_MODE == 0)
#undef FAST_MODE
#endif
