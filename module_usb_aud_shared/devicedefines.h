/** 
 * @file        internaldefines.h
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

#if defined(IO_EXPANSION) && (IO_EXPANSION == 0)
#undef IO_EXPANSION
#endif

#if defined(IAP) && (IAP == 0)
#undef IAP
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

#if defined(CODEC_SLAVE) && (CODEC_SLAVE == 0)
#undef CODEC_SLAVE
#endif

#if defined(LEVEL_METER_LEDS) && !defined(LEVEL_UPDATE_RATE)
#define LEVEL_UPDATE_RATE   400000
#endif

#if(AUDIO_CLASS != 1) && (AUDIO_CLASS != 2)
#warning AUDIO_CLASS not defined, using 2
#define AUDIO_CLASS 2
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

#ifndef SPDIF_TX_INDEX
#warning SPDIF_TX_INDEX not defined! Using 0
#define SPDIF_TX_INDEX                  (0)
#endif

/* Max supported freq for device */
#ifndef MAX_FREQ
#warning MAX_FREQ not defined! Using 48000
#define MAX_FREQ                        (48000)       
#endif

/* Default device freq */
#ifndef DEFAULT_FREQ
#warning DEFAULT not defined! Using MAX_FREQ
#define DEFAULT_FREQ                    (MAX_FREQ)     
#endif

/* Master clock defines (in Hz) */
#ifndef MCLK_441                 
#error MCLK_441 not defined
#endif

#ifndef MCLK_48                
#error MCLK_441 not defined
#endif

/* The number of clock ticks to wait for the audio PLL to lock */
#define AUDIO_PLL_LOCK_DELAY     (40000000) 

/* Vendor/Product strings */
#ifndef VENDOR_STR
#define VENDOR_STR               "XMOS "   
//#warning VENDOR_STR not defined. Using XMOS
#endif

#ifndef VENDOR_ID
#warning VENDOR_ID not defined. Using XMOS vendor ID
#define VENDOR_ID                (0x20B1)        /* XMOS VID */
#endif

#ifndef PID_AUDIO_1
#define PID_AUDIO_1               (0x0001)        
#warning PRODUCT_ID not defined. Using 0x0001
#endif

#ifndef PID_AUDIO_2
#define PID_AUDIO_2               (0x0001)        
#warning PRODUCT_ID not defined. Using 0x0001
#endif

/* Device release number in BCD: 0xJJMNi */
#ifndef BCD_DEVICE
#define BCD_DEVICE               (0x0000)       
#warning BCD_DEVICE not defined. Using 0x0000
#endif

/* Addition interfaces based on defines */
#if defined(DFU) && DFU != 0
#define DFU_INTERFACES          (1)               /* DFU interface count */
#else
#define DFU_INTERFACES          (0)
#endif

#ifdef INPUT
#define INPUT_INTERFACES        (1)
#else
#define INPUT_INTERFACES        (0)
#endif

#if defined(OUTPUT) && OUTPUT != 0
#define OUTPUT_INTERFACES       (1)
#else
#define OUTPUT_INTERFACES       (0)
#endif

#define NUM_EP_OUT_AUD          (OUTPUT_INTERFACES)
#define NUM_EP_IN_AUD           (OUTPUT_INTERFACES + INPUT_INTERFACES)

#if defined(MIDI)
#define MIDI_INTERFACES         (2)
#define NUM_EP_OUT_MIDI         (1)
#define NUM_EP_IN_MIDI          (1)
#else
#define MIDI_INTERFACES         (0)
#define NUM_EP_OUT_MIDI         (0)
#define NUM_EP_IN_MIDI          (0)
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

#define NUM_EP_OUT_IAP          (IAP_INTERFACES)
#define NUM_EP_IN_IAP           (IAP_INTERFACES * 2)

#define NUM_EP_OUT_HID          (0)
#define NUM_EP_IN_HID           (HID_INTERFACES)


/* Define for number of audio interfaces (+1 for mandatory control interface) */
#define AUDIO_INTERFACES			(INPUT_INTERFACES + OUTPUT_INTERFACES + 1) 

/* Interface number defines */
#define INTERFACE_NUM_IAP (INPUT_INTERFACES+OUTPUT_INTERFACES+MIDI_INTERFACES+DFU_INTERFACES+1)
#define INTERFACE_NUM_HID (INPUT_INTERFACES+OUTPUT_INTERFACES+MIDI_INTERFACES+DFU_INTERFACES+IAP_INTERFACES+1)

/* Endpoint Number Defines */
#define EP_NUM_IN_FB            (1)     /* Always 1 */
#define EP_NUM_IN_AUD           (2)    /* Always 2 */
#define EP_NUM_IN_AUD_INT       (3)     /* Audio interrupt/status EP */
#define EP_NUM_IN_MIDI          ((EP_NUM_IN_AUD_INT + 1))
#define EP_NUM_IN_HID           ((EP_NUM_IN_AUD_INT + NUM_EP_IN_MIDI + 1))
#define EP_NUM_IN_IAP           ((EP_NUM_IN_AUD_INT + NUM_EP_IN_MIDI + NUM_EP_IN_HID + 1)) /* iAP Bulk */
#define EP_NUM_IN_IAP_INT       ((EP_NUM_IN_AUD_INT + NUM_EP_IN_MIDI + NUM_EP_IN_HID + 2)) /* iAP interrupt */

#define EP_NUM_OUT_AUD          1       /* Always 1 */
#define EP_NUM_OUT_MIDI         2       /* Always 2 */
#define EP_NUM_OUT_IAP          3       /* Always 3 */

/* Endpoint Address Defines */
#define EP_ADR_IN_FB            (EP_NUM_IN_FB | 0x80)
#define EP_ADR_IN_AUD           (EP_NUM_IN_AUD | 0x80)
#define EP_ADR_IN_AUD_INT       (EP_NUM_IN_AUD_INT | 0x80)
#define EP_ADR_IN_MIDI          (EP_NUM_IN_MIDI | 0x80)
#define EP_ADR_IN_HID           (EP_NUM_IN_HID | 0x80)
#define EP_ADR_IN_IAP           (EP_NUM_IN_IAP | 0x80)
#define EP_ADR_IN_IAP_INT       (EP_NUM_IAP_INT | 0x80)

#define EP_ADR_OUT_AUD          EP_NUM_OUT_AUD            
#define EP_ADR_OUT_MIDI         EP_NUM_OUT_MIDI           
#define EP_ADR_OUT_IAP          EP_NUM_OUT_IAP            

/* Endpoint count totals */
#define NUM_EP_OUT              (1 + 1 /*NUM_EP_OUT_AUD*/ + NUM_EP_OUT_MIDI + NUM_EP_OUT_IAP) /* +1 due to EP0 */ 
#define NUM_EP_IN               (2 + 2 /*NUM_EP_IN_AUD*/ + NUM_EP_IN_MIDI + NUM_EP_IN_IAP + NUM_EP_IN_HID)    /* +1 due to EP0 and Int EP */

#define AUDIO_STOP_FOR_DFU      (0x12345678)
#define AUDIO_START_FROM_DFU    (0x87654321)
#define AUDIO_REBOOT_FROM_DFU   (0xa5a5a5a5)

#define MAX_VOL                 (0x20000000)


/* Length of clock unit/clock-selector units */
#if defined(SPDIF_RX) && defined(ADAT_RX)
#define NUM_CLOCKS                  3
#elif defined(SPDIF_RX) || defined(ADAT_RX)
#define NUM_CLOCKS                  2
#else 
#define NUM_CLOCKS                  1
#endif


/* Total number of USB interfaces this device implements (+1 for required control interface) */
#define NUM_INTERFACES          INPUT_INTERFACES + OUTPUT_INTERFACES + DFU_INTERFACES + MIDI_INTERFACES + IAP_INTERFACES + 1 + HID_INTERFACES

/* Number of interfaces for Audio 1.0 */
#define NUM_INTERFACES_A1 (1+INPUT_INTERFACES+OUTPUT_INTERFACES)


/* Audio Unit ID defines */
#define FU_USBIN                11              /* Feature Unit: USB Audio device -> host */ 
#define FU_USBOUT               10              /* Feature Unit: USB Audio host -> device*/ 
#define ID_IT_USB               2               /* Input terminal: USB streaming */
#define ID_IT_AUD               1               /* Input terminal: Analogue input */
#define ID_OT_USB               22              /* Output terminal: USB streaming */
#define ID_OT_AUD               20              /* Output terminal: Analogue output */

#define ID_CLKSEL               40              /* Clock selector ID */
#define ID_CLKSRC_INT           41              /* Clock source ID (internal) */
#define ID_CLKSRC_EXT           42              /* Clock source ID (external) */
#define ID_CLKSRC_ADAT           43              /* Clock source ID (external) */

#define ID_XU_MIXSEL                50
#define ID_XU_OUT                   51
#define ID_XU_IN                    52

#define ID_MIXER_1                  60

#define MANUFACTURER_STR_INDEX	    0x01
#define PRODUCT_STR_INDEX           0x02

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

#if defined(AUDIO_CLASS_FALLBACK) && (AUDIO_CLASS_FALLBACK==0)
#undef AUDIO_CLASS_FALLBACK
#endif

/* Defines for DFU */
#ifndef PID_DFU
#define PID_DFU PID_AUDIO_2
#endif

#define DFU_VENDOR_ID               VENDOR_ID
#define DFU_BCD_DEVICE              BCD_DEVICE
#define DFU_MANUFACTURER_INDEX      MANUFACTURER_STR_INDEX
#define DFU_PRODUCT_INDEX           PRODUCT_STR_INDEX
#endif
