/** 
 * @file    DeviceDescriptors.h 
 * @brief   Device Descriptors 
 * @author  Ross Owen, XMOS Limited
*/



#ifndef _DEVICE_DESCRIPTORS_
#define _DEVICE_DESCRIPTORS_

#include "usb.h"
#include "usbaudio20.h"             /* Defines from the USB Audio 2.0 Specifications */
#include "devicedefines.h"      	/* Define specific define */   

/***** Device Descriptors *****/

#if defined(AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS==1)
/* Device Descriptor for Audio Class 1.0 (Assumes Full-Speed) */
unsigned char devDesc_Audio1[] = 
{                                                                                                                                                            
    18,                            	/* 0  bLength : Size of descriptor in Bytes (18 Bytes) */ 
	USB_DEVICE,     				/* 1  bdescriptorType */  
	0x0, 							/* 2  bcd USB */	
    0x2,          					/* 3  bcdUSB */ 
    0,  							/* 4  bDeviceClass */ 
    0,                              /* 5  bDeviceSubClass */ 
    0,                              /* 6  bDeviceProtocol */ 
    64,                             /* 7  bMaxPacketSize */                                                               
    (VENDOR_ID & 0xFF), 			/* 8  idVendor */ 
    (VENDOR_ID >> 8),              	/* 9  idVendor */ 
    (PID_AUDIO_1 & 0xFF),           /* 10 idProduct */ 
    (PID_AUDIO_1 >> 8),             /* 11 idProduct */ 
    (BCD_DEVICE & 0xFF),           	/* 12 bcdDevice : Device release number */ 
    (BCD_DEVICE >> 8),              /* 13 bcdDevice : Device release number */ 
    MANUFACTURER_STR_INDEX,         /* 14 iManufacturer : Index of manufacturer string */ 
    8,                              /* 15 iProduct : Index of product string descriptor */ 
    0,//SERIAL_STR_INDEX,           /* 16 iSerialNumber : Index of serial number decriptor */ 
	0x01             				/* 17 bNumConfigurations : Number of possible configs. */ 
};    
#endif

/* Device Descriptor for Audio Class 2.0 (Assumes High-Speed ) */
unsigned char devDesc_Audio2[] = 
{
    18,              				/* 0  bLength : Size of descriptor in Bytes (18 Bytes) */ 
    USB_DEVICE,               		/* 1  bdescriptorType */ 
    0,               				/* 2  bcdUSB */ 
    2,               				/* 3  bcdUSB */ 
    0xEF,            				/* 4  bDeviceClass (See Audio Class Spec page 45) */ 
    0x02,               			/* 5  bDeviceSubClass */ 
    0x01,               			/* 6  bDeviceProtocol */ 
    64,              				/* 7  bMaxPacketSize */ 
    (VENDOR_ID & 0xFF),            	/* 8  idVendor */ 
    (VENDOR_ID >> 8),              	/* 9  idVendor */ 
    (PID_AUDIO_2 & 0xFF),           /* 10 idProduct */ 
    (PID_AUDIO_2 >> 8),             /* 11 idProduct */ 
    (BCD_DEVICE & 0xFF),           	/* 12 bcdDevice : Device release number */ 
    (BCD_DEVICE >> 8),              /* 13 bcdDevice : Device release number */ 
    MANUFACTURER_STR_INDEX,         /* 14 iManufacturer : Index of manufacturer string */ 
    PRODUCT_STR_INDEX,           	/* 15 iProduct : Index of product string descriptor */ 
    0,//    SERIAL_STR_INDEX,            	/* 16 iSerialNumber : Index of serial number decriptor */ 
    0x02             				/* 17 bNumConfigurations : Number of possible configs. Set to 2 so that Windows 
                        				  does not load Composite driver. */ 
};

/* Device Descriptor for Null Device */
unsigned char devDesc_Null[] = 
{
    18,              				/* 0  bLength : Size of descriptor in Bytes (18 Bytes) */ 
    USB_DEVICE,               		/* 1  bdescriptorType */ 
    0,               				/* 2  bcdUSB */ 
    2,               				/* 3  bcdUSB */ 
    0x0,            				/* 4  bDeviceClass */ 
    0x0  ,               			/* 5  bDeviceSubClass */ 
    0x00,               			/* 6  bDeviceProtocol */ 
    64,              				/* 7  bMaxPacketSize */ 
    (VENDOR_ID & 0xFF),            	/* 8  idVendor */ 
    (VENDOR_ID >> 8),              	/* 9  idVendor */ 
    (PID_AUDIO_2 & 0xFF),           /* 10 idProduct */ 
    (PID_AUDIO_2 >> 8),             /* 11 idProduct */ 
    (BCD_DEVICE & 0xFF),           	/* 12 bcdDevice : Device release number */ 
    (BCD_DEVICE >> 8),              /* 13 bcdDevice : Device release number */ 
    MANUFACTURER_STR_INDEX,         /* 14 iManufacturer : Index of manufacturer string */ 
	PRODUCT_STR_INDEX,              /* 15 iProduct : Index of product string descriptor */ 
    0,//SERIAL_STR_INDEX,            	/* 16 iSerialNumber : Index of serial number decriptor */ 
    0x01             				/* 17 bNumConfigurations : Number of possible configs */
};


/****** Device Qualifier Descriptors *****/

/* Device Qualifier Descriptor for Audio 2.0 device (Use when running at full-speed. Matches audio 2.0 device descriptor) */
unsigned char devQualDesc_Audio2[] =
{ 
    10,                             /* 0  bLength (10 Bytes) */ 
    USB_DEVICE_QUALIFIER, 			/* 1  bDescriptorType */ 
    0x00,							/* 2  bcdUSB (Binary Coded Decimal of usb version) */ 
    0x02,      						/* 3  bcdUSB */ 
    0xEF,                           /* 4  bDeviceClass */ 
    0x02,                           /* 5  bDeviceSubClass */ 
    0x01,                           /* 6  bDeviceProtocol */ 
    64,                             /* 7  bMaxPacketSize */ 
    0x01,                           /* 8  bNumConfigurations : Number of possible configs */ 
    0x00                            /* 9  bReserved (must be zero) */ 
};

#if defined(AUDIO_CLASS_FALLBACK) || (AUDIO_CLASS==1)
/* Device Qualifier Descriptor for running at high-speed (matches audio 1.0 device descriptor) */
unsigned char devQualDesc_Audio1[] =
{ 
    10,                             /* 0  bLength (10 Bytes) */ 
    USB_DEVICE_QUALIFIER, 			/* 1  bDescriptorType */ 
    0x00,							/* 2  bcdUSB (Binary Coded Decimal of usb version) */ 
    0x02,      						/* 3  bcdUSB */ 
    0x00,                           /* 4  bDeviceClass */ 
    0x00,                           /* 5  bDeviceSubClass */ 
    0x00,                           /* 6  bDeviceProtocol */ 
    64,                             /* 7  bMaxPacketSize */ 
    0x01,                           /* 8  bNumConfigurations : Number of possible configs */ 
    0x00                            /* 9  bReserved (must be zero) */ 
};
#endif

/* Device Qualifier Descriptor for Null Device (Use when running at high-speed) */
unsigned char devQualDesc_Null[] =
{ 
    10,                             /* 0  bLength (10 Bytes) */ 
    USB_DEVICE_QUALIFIER, 			/* 1  bDescriptorType */ 
    0x00,							/* 2  bcdUSB (Binary Coded Decimal of usb version) */ 
    0x02,      						/* 3  bcdUSB */ 
    0x00,                           /* 4  bDeviceClass */ 
    0x00,                           /* 5  bDeviceSubClass */ 
    0x00,                           /* 6  bDeviceProtocol */ 
    64,                             /* 7  bMaxPacketSize */ 
    0x01,                           /* 8  bNumConfigurations : Number of possible configs */ 
    0x00                            /* 9  bReserved (must be zero) */ 
};


#if defined(MIXER) && !defined(AUDIO_PATH_XUS)
#warning Extention units on the audio path are required for mixer.  Enabling them now.
#define AUDIO_PATH_XUS
#endif

/* Lenths of input/output term descriptors - from spec */
#define LEN_OUTPUT_TERMINAL         (0x0C)
#define LEN_INPUT_TERMINAL          (0x11)

/* Lengh of out terminal descriptors in total */
#define LEN_TERMS_OUT               ((LEN_OUTPUT_TERMINAL + LEN_INPUT_TERMINAL) * OUTPUT_INTERFACES)
#define LEN_TERMS_IN                ((LEN_OUTPUT_TERMINAL + LEN_INPUT_TERMINAL) * INPUT_INTERFACES)

/* Calc total length of configuration desc based on defines */
#ifdef OUTPUT
#define LEN_FU_OUT                  (6 + (NUM_USB_CHAN_OUT + 1) * 4)    
#else
#define LEN_FU_OUT                  0
#endif

#ifdef INPUT
#define LEN_FU_IN                   (6 + (NUM_USB_CHAN_IN + 1) * 4)    
#else
#define LEN_FU_IN                   0
#endif


#ifdef MIDI
#define MIDI_LENGTH                 (92)
#else 
#define MIDI_LENGTH                 (0)
#endif

#ifdef IAP
#define IAP_LENGTH                  (30)
#else
#define IAP_LENGTH                  (0)
#endif

#if defined(SPDIF_RX) || defined(ADAT_RX)
#define AUD_INT_EP_LEN              (7)
#else
#define AUD_INT_EP_LEN              (0)
#endif

#ifdef AUDIO_PATH_XUS
#define LEN_XU_OUT                  (16 * OUTPUT_INTERFACES)
#define LEN_XU_IN                   (16 * INPUT_INTERFACES)
#else
#define LEN_XU_OUT                  (0)
#define LEN_XU_IN                   (0)
#endif

#ifdef MIXER
    #define LEN_XU_MIX                  (17)
    #define MIX_BMCONTROLS_LEN_TMP      ((MAX_MIX_COUNT * MIX_INPUTS) / 8)

    #if ((MAX_MIX_COUNT * MIX_INPUTS)%8)==0
        #define MIX_BMCONTROLS_LEN          (MIX_BMCONTROLS_LEN_TMP)
    #else
        #define MIX_BMCONTROLS_LEN          (MIX_BMCONTROLS_LEN_TMP+1)
    #endif
    #define MIXER_LENGTH                (13+1+MIX_BMCONTROLS_LEN)
#else
    #define LEN_XU_MIX                  (0)
    #define MIXER_LENGTH                (0)
#endif

#define LEN_CLK                     (8)
#define LEN_CLK_SEL                 (7 + NUM_CLOCKS)
#define LEN_CLOCKING                (LEN_CLK_SEL + (NUM_CLOCKS * LEN_CLK))

/* Total length of the Class-Specific AC Interface Descriptor - Clock Entities, Units and Terminals */
#define LEN_AC                      (9)
#define TLEN_AC                     (LEN_AC + LEN_FU_OUT + LEN_FU_IN + LEN_CLOCKING + LEN_TERMS_OUT + LEN_TERMS_IN + LEN_XU_OUT + LEN_XU_IN + LEN_XU_MIX)


#ifdef ADAT_RX
#define INPUT_ALT_LENGTH            (46)
#else
#define INPUT_ALT_LENGTH            (0)
#endif

#ifdef ADAT_TX
#define OUTPUT_ALT_LENGTH_ADAT      (46)
#else
#define OUTPUT_ALT_LENGTH_ADAT      (0)
#endif

#ifdef NATIVE_DSD
#define ALT_SETTING_DSD             (2)
#endif

#ifdef ALT_SETTING_DSD
#define ALT_SETTING_ADAT_TX         (3)
#define OUTPUT_ALT_LENGTH_DSD       (53)
#else
#define ALT_SETTING_ADAT_TX         (2)
#define OUTPUT_ALT_LENGTH_DSD       (0)
#endif

#define OUTPUT_ALT_LENGTH           (OUTPUT_ALT_LENGTH_ADAT + OUTPUT_ALT_LENGTH_DSD)


// Positions in strDescs_Audio2
#define INTERNAL_CLOCK_STRING_INDEX (14)
#define SPDIF_CLOCK_STRING_INDEX    (15)

#ifdef SPDIF_RX
#define ADAT_CLOCK_STRING_INDEX (SPDIF_CLOCK_STRING_INDEX + 1)
#else
#define ADAT_CLOCK_STRING_INDEX (SPDIF_CLOCK_STRING_INDEX)
#endif

#ifdef ADAT_RX
#define DFU_STRING_INDEX (ADAT_CLOCK_STRING_INDEX + 1)
#else
#define DFU_STRING_INDEX (ADAT_CLOCK_STRING_INDEX)
#endif

#ifdef DFU
#define MIDI_OUT_STRING_INDEX (DFU_STRING_INDEX + 1)
#else
#define MIDI_OUT_STRING_INDEX (DFU_STRING_INDEX)
#endif

#define MIDI_IN_STRING_INDEX (MIDI_OUT_STRING_INDEX + 1)

#ifdef MIDI
#define OUTPUT_INTERFACE_STRING_INDEX (MIDI_OUT_STRING_INDEX + 2)
#else
#define OUTPUT_INTERFACE_STRING_INDEX (MIDI_OUT_STRING_INDEX)
#endif

#define INPUT_INTERFACE_STRING_INDEX (OUTPUT_INTERFACE_STRING_INDEX + NUM_USB_CHAN_OUT)

#define MIXER_STRING_INDEX (INPUT_INTERFACE_STRING_INDEX + NUM_USB_CHAN_IN)

#ifdef MIXER
#define IAP_INTERFACE_STRING_INDEX (MIXER_STRING_INDEX + MAX_MIX_COUNT)
#else
#define IAP_INTERFACE_STRING_INDEX (MIXER_STRING_INDEX)
#endif

#ifdef HID_CONTROLS
unsigned char hidReportDescriptor[] = {
0x05, 0x0c,     /* Usage Page (Consumer Device) */
0x09, 0x01,     /* Usage (Consumer Control) */
0xa1, 0x01,     /* Collection (Application) */
0x15, 0x00,     /* Logical Minimum (0) */
0x25, 0x01,     /* Logical Maximum (1) */
0x09, 0xb0,     /* Usage (Play) */
0x09, 0xb5,     /* Usage (Scan Next Track) */
0x09, 0xb6,     /* Usage (Scan Previous Track) */
0x09, 0xe9,     /* Usage (Volume Up) */
0x09, 0xea,     /* Usage (Volume Down) */
0x09, 0xe2,     /* Usage (Mute) */
0x75, 0x01,     /* Report Size (1) */
0x95, 0x06,     /* Report Count (6) */
0x81, 0x02,     /* Input (Data, Var, Abs) */
0x95, 0x02,     /* Report Count (2) */
0x81, 0x01,     /* Input (Cnst, Ary, Abs) */
0xc0            /* End collection */
};

#endif

#define HID_LENGTH (25 * HID_INTERFACES)

/* Total length of config descriptor */
#define CFG_TOTAL_LENGTH_A2			(7 + 19 + (AUD_INT_EP_LEN) + (INPUT_INTERFACES * 55) + (OUTPUT_INTERFACES * 62) + (MIDI_LENGTH) + (DFU_INTERFACES * 18)  + TLEN_AC + (MIXER_LENGTH) + IAP_LENGTH + INPUT_ALT_LENGTH + OUTPUT_ALT_LENGTH + HID_LENGTH)

/* Configuration Descriptor for Audio 2.0 (HS) operation */
unsigned char cfgDesc_Audio2[] = 
{
    0x09,            				/* 0  bLength */ 
    USB_CONFIGURATION,            	/* 1  bDescriptorType */ 
    (CFG_TOTAL_LENGTH_A2 & 0xFF),   /* 2  wTotalLength */ 
    (CFG_TOTAL_LENGTH_A2 >> 8),     /* 3  wTotalLength */ 
    NUM_INTERFACES,               	/* 4  bNumInterface: Number of interfaces*/ 
	0x01,            				/* 5  bConfigurationValue */ 
    0x00,            				/* 6  iConfiguration */ 
#ifdef SELF_POWERED
    192,                            /* 7  bmAttributes */
#else
    128,                            /* 7  bmAttributes */ 
#endif   
    BMAX_POWER,             		/* 8  bMaxPower */  

    /* Interface Association Descriptor */ 
    0x08,            				/* 0  bLength */
    0x0b,            				/* 1  bDescriptorType */
    0x00,            				/* 2  bFirstInterface */
    AUDIO_INTERFACES,            	/* 3  bInterfaceCount */
    AUDIO_FUNCTION,            		/* 4  bFunctionClass: AUDIO_FUNCTION */
    FUNCTION_SUBCLASS_UNDEFINED,    /* 5  bFunctionSubClass: FUNCTION_SUBCLASS_UNDEFINED */
    AF_VERSION_02_00,            	/* 6  bFunctionProtocol: AF_VERSION_02_00 */
    0x00,            				/* 7  iFunction (String Index) *(re-use iProduct) */

    /* Standard Audio Control Interface Descriptor (Note: Must be first with lowest interface number)r */
    0x09,            				/* 0  bLength: 9 */
    USB_INTERFACE,            		/* 1  bDescriptorType: INTERFACE */
    0x00,            				/* 2  bInterfaceNumber */
    0x00,            				/* 3  bAlternateSetting: Must be 0 */
#if defined(SPDIF_RX) || defined(ADAT_RX)
    0x01,            				/* 4  bNumEndpoints (0 or 1 if optional interrupt endpoint is present */
#else
    0x00,
#endif
    AUDIO,               			/* 5  bInterfaceClass: AUDIO */
    AUDIOCONTROL,               	/* 6  bInterfaceSubClass: AUDIOCONTROL*/
    IP_VERSION_02_00,            	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    PRODUCT_STR_INDEX,              /* 8  iInterface (re-use iProduct) */ 

    /* Class Specific Audio Control Interface Header Descriptor: */
    LEN_AC,            				/* 0   bLength */
    CS_INTERFACE,           		/* 1   bDescriptorType: 0x24 */
    HEADER,          				/* 2   bDescriptorSubtype: HEADER */
    0x00, 0x02,      				/* 3:4 bcdUSB */
    IO_BOX,             			/* 5   bCatagory (Primary use of audio function) */
    (TLEN_AC & 0xFF),             	/* 6   wTotalLength */
    (TLEN_AC >> 8),               	/* 7   wTotalLength */
    0x00,            				/* 8   bmControls (0:1 Latency Control, 2:7 must be 0 */ 

    /* Clock Source Descriptor (4.7.2.1) */ 
    LEN_CLK,           				/* 0   bLength: 8 */
    CS_INTERFACE,           		/* 1   bDescriptorType */
    CLOCK_SOURCE,           		/* 2   bDescriptorSubtype */
    ID_CLKSRC_INT,             		/* 3   bClockID */
    0x03,              				/* 4   bmAttributes:   
                       						D[1:0] :
                                				00: External Clock 
                                				01: Internal Fixed Clock
                                				10: Internal Variable Clock
                                				11: Internal Progamable Clock 
                       						D[2]   : Clock synced to SOF
                       						D[7:3] : Reserved (0) */
    0x07,            				/* 5   bmControls       
									 		D[1:0] : Clock Freq Control
                							D[3:2] : Clock Validity Control
                							D[7:4] : Reserved (0) */
    0x00,        					/* 6   bAssocTerminal */
    INTERNAL_CLOCK_STRING_INDEX,        					/* 7   iClockSource (String Index) */

#ifdef SPDIF_RX
    /* Clock Source Descriptor (4.7.2.1) */ 
    LEN_CLK,           				/* 0   bLength: 8 */
    CS_INTERFACE,           		/* 1   bDescriptorType */
    CLOCK_SOURCE,           		/* 2   bDescriptorSubtype */
    ID_CLKSRC_EXT,             		/* 3   bClockID */
    0x00,              				/* 4   bmAttributes:   
                       						D[1:0] :
                                				00: External Clock 
                                				01: Internal Fixed Clock
                                				10: Internal Variable Clock
                                				11: Internal Progamable Clock 
                       						D[2]   : Clock synced to SOF
                       						D[7:3] : Reserved (0) */
    0x07,            				/* 5   bmControls       
									 		D[1:0] : Clock Freq Control
                							D[3:2] : Clock Validity Control
                							D[7:4] : Reserved (0) */
    0x00,        					/* 6   bAssocTerminal */
    SPDIF_CLOCK_STRING_INDEX,        					    /* 7   iClockSource (String Index) */
#endif
#ifdef ADAT_RX
    /* Clock Source Descriptor (4.7.2.1) */ 
    LEN_CLK,           				/* 0   bLength: 8 */
    CS_INTERFACE,           		/* 1   bDescriptorType */
    CLOCK_SOURCE,           		/* 2   bDescriptorSubtype */
    ID_CLKSRC_ADAT,             		/* 3   bClockID */
    0x00,              				/* 4   bmAttributes:   
                       						D[1:0] :
                                				00: External Clock 
                                				01: Internal Fixed Clock
                                				10: Internal Variable Clock
                                				11: Internal Progamable Clock 
                       						D[2]   : Clock synced to SOF
                       						D[7:3] : Reserved (0) */
    0x07,            				/* 5   bmControls       
									 		D[1:0] : Clock Freq Control
                							D[3:2] : Clock Validity Control
                							D[7:4] : Reserved (0) */
    0x00,        					/* 6   bAssocTerminal */
    ADAT_CLOCK_STRING_INDEX,        					    /* 7   iClockSource (String Index) */
#endif
    /* Clock Selector Descriptor (4.7.2.2) */ 
    LEN_CLK_SEL,                    /* 0    bLength */
    CS_INTERFACE,                   /* 1    bDescriptorType */
    CLOCK_SELECTOR,                 /* 2    bDescriptorSubtype */
    ID_CLKSEL,                      /* 3    bClockID */
    NUM_CLOCKS,                     /* 4    Number of input pins*/
    ID_CLKSRC_INT,
#ifdef SPDIF_RX
    ID_CLKSRC_EXT,                     
#endif
#ifdef ADAT_RX
   ID_CLKSRC_ADAT,
#endif
    0x03,                           /* 5   bmControls       
                                            D[1:0] : Clock Selector Control
                                            D[7:4] : Reserved (0) */
    13,                              /* 7   iClockSel (String Index) */  

#ifdef OUTPUT
	/* OUTPUT PATH FROM HOST TO DEVICE */
    /* Input Terminal Descriptor (USB Input Terminal) */
    0x11, 		     				/* 0  bLength in bytes: 17 */
    CS_INTERFACE, 		     		/* 1  bDescriptorType: 0x24 */
    INPUT_TERMINAL, 		     	/* 2  bDescriptorSubType: INPUT_TERMINAL */
    ID_IT_USB, 			     		/* 3  bTerminalID */
    (USB_STREAMING&0xff),USB_STREAMING>>8, /* 4  wTerminalType: USB Streaming */
    0x00, 		     				/* 6  bAssocTerminal */
    ID_CLKSEL, 		     		    /* 7  bCSourceID: ID of Clock Entity */
    NUM_USB_CHAN_OUT,		     	/* 8  bNrChannels */
    0,0,0,0,                 		/* 9  bmChannelConfig */
    OUTPUT_INTERFACE_STRING_INDEX,  /* 13 iChannelNames */
    0x00, 0x00,     		        /* 14 bmControls */
    6,           		            /* 16 iTerminal */

#ifdef AUDIO_PATH_XUS
    /* Extension Unit Descriptor (4.7.2.12) */
    LEN_XU_OUT,                     /* 0    bLength (15 + p, when p is number of sources) */
    CS_INTERFACE,                   /* 1    bDescriptorType */
    EXTENSION_UNIT,                 /* 2    bDescriptorSubtype */
    ID_XU_OUT,                      /* 3    bUnitID */
    0,                              /* 4    wExtensionCode */
    0,                              /* 5    wExtensionCode */
    1,                              /* 6    bNrPins */
    ID_IT_USB,                      /* 7    baSourceId(1) */
    NUM_USB_CHAN_OUT,               /* 8+p  bNrChannels */
    0,                              /* 9+p  bmChannelConfig */  
    0,                              /* 10+p bmChannelConfig */  
    0,                              /* 11+p bmChannelConfig */  
    0,                              /* 12+p bmChannelConfig */  
    0,                              /* 13+p iChannelNames */
    3,                              /* 14+p bmControls */
    0,                              /* 15+p iExtension */
#endif

    /* Feature Unit Descriptor */ 
    LEN_FU_OUT,                     /* 0  bLength: 6+(ch + 1)*4 */ 
    0x24,    						/* 1  bDescriptorType: CS_INTERFACE */ 
    0x06,    						/* 2  bDescriptorSubType: FEATURE_UNIT */
    FU_USBOUT, 						/* 3  bUnitID */
#ifdef AUDIO_PATH_XUS
    ID_XU_OUT,                      /* 4  bSourceID */
#else
    ID_IT_USB,						/* 4  bSourceID */
#endif
#if (NUM_USB_CHAN_OUT > 0)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(0) : Mute and Volume host read and writable */
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(1) */
#endif
#if (NUM_USB_CHAN_OUT > 1)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(2) */
#endif
#if (NUM_USB_CHAN_OUT > 2)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(3) */
#endif
#if (NUM_USB_CHAN_OUT > 3)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(4) */
#endif
#if (NUM_USB_CHAN_OUT > 4)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(5) */
#endif
#if (NUM_USB_CHAN_OUT > 5)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(6) */
#endif
#if (NUM_USB_CHAN_OUT > 6)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(7) */
#endif
#if (NUM_USB_CHAN_OUT > 7)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(8) */
#endif
#if (NUM_USB_CHAN_OUT > 8)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(9) */
#endif
#if (NUM_USB_CHAN_OUT > 9)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(10) */
#endif
#if (NUM_USB_CHAN_OUT > 10)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(11) */
#endif
#if (NUM_USB_CHAN_OUT > 11)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(12) */
#endif
#if (NUM_USB_CHAN_OUT > 12)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(13) */
#endif
#if (NUM_USB_CHAN_OUT > 13)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(14) */
#endif
#if (NUM_USB_CHAN_OUT > 14)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(15) */
#endif
#if (NUM_USB_CHAN_OUT > 15)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(16) */
#endif
#if (NUM_USB_CHAN_OUT > 16)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(17) */
#endif
#if (NUM_USB_CHAN_OUT > 17)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(18) */
#endif
#if (NUM_USB_CHAN_OUT > 18)
#error NUM_USB_CHAN_OUT > 18
#endif
    0,    							/* 60 iFeature */ 

	/* Output Terminal Descriptor (Audio) */
    0x0C, 	           				/* 0  bLength */
    CS_INTERFACE,        			/* 1  bDescriptorType: 0x24 */
    OUTPUT_TERMINAL, 	           	/* 2  bDescriptorSubType: OUTPUT_TERMINAL */
    ID_OT_AUD, 	           			/* 3  bTerminalID */
    (SPEAKER&0xff),SPEAKER>>8,  			/* 4  wTerminalType */
    0x00,        					/* 6  bAssocTerminal */
    FU_USBOUT,    					/* 7  bSourceID Connect to analog input feature unit*/
    ID_CLKSEL,  	           		/* 8  bCSourceUD */
    0x00, 0x00,  					/* 9  bmControls */
    0,        						/* 11 iTerminal */
#endif

#ifdef INPUT
	/* INPUT FROM DEVICE TO HOST PATH */
    /* Input Terminal Descriptor (Analogue Input Terminal) */
    0x11, 		     				/* 0  bLength in bytes: 17 */
    CS_INTERFACE, 		     		/* 1  bDescriptorType: 0x24 */
    INPUT_TERMINAL, 		     	/* 2  bDescriptorSubType: INPUT_TERMINAL */
    ID_IT_AUD, 			     		/* 3  bTerminalID */
    (MICROPHONE_&0xff),MICROPHONE_>>8,     /* 4  wTerminalType: USB Streaming */
    0x00, 		    				/* 6  bAssocTerminal */
    ID_CLKSEL, 	    	     		/* 7  bCSourceID: ID of Clock Entity */
    NUM_USB_CHAN_IN,		     	/* 8  bNrChannels */
    0,0,0,0,                 		/* 9  bmChannelConfig */
    INPUT_INTERFACE_STRING_INDEX,           	                /* 13 iChannelNames */
    0x00, 0x00,     		        /* 14 bmControls */
    0,           		            /* 16 iTerminal */

#ifdef AUDIO_PATH_XUS
    /* Extension Unit Descriptor (4.7.2.12) */
    LEN_XU_IN,                      /* 0    bLength (15 + p, when p is number of sources) */
    CS_INTERFACE,                   /* 1    bDescriptorType */
    EXTENSION_UNIT,                 /* 2    bDescriptorSubtype */
    ID_XU_IN,                       /* 3    bUnitID */
    0,                              /* 4    wExtensionCode */
    0,                              /* 5    wExtensionCode */
    1,                              /* 6    bNrPins */
    ID_IT_AUD,                      /* 7    baSourceId(1) */
    NUM_USB_CHAN_OUT,               /* 8+p  bNrChannels */
    0,                              /* 9+p  bmChannelConfig */  
    0,                              /* 10+p bmChannelConfig */  
    0,                              /* 11+p bmChannelConfig */  
    0,                              /* 12+p bmChannelConfig */  
    0,                              /* 13+p iChannelNames */
    3,                              /* 14+p bmControls */
    0,                              /* 15+p iExtension */
#endif

    /* Feature Unit Descriptor */ 
    LEN_FU_IN,    				/* 0  bLength: 6+(ch+1)*4 */ 
    CS_INTERFACE,    				/* 1  bDescriptorType: CS_INTERFACE */ 
    FEATURE_UNIT,    				/* 2  bDescriptorSubType: FEATURE_UNIT */
    FU_USBIN, 						/* 3  bUnitID */
#ifdef AUDIO_PATH_XUS
    ID_XU_IN,						/* 4  bSourceID */
#else
    ID_IT_AUD,						/* 4  bSourceID */
#endif
#if (NUM_USB_CHAN_IN > 0)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(0) : Mute and Volume host read and writable */
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(1) */
#endif
#if (NUM_USB_CHAN_IN > 1)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(2) */
#endif
#if (NUM_USB_CHAN_IN > 2)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(3) */
#endif
#if (NUM_USB_CHAN_IN > 3)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(4) */
#endif
#if (NUM_USB_CHAN_IN > 4)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(5) */
#endif
#if (NUM_USB_CHAN_IN > 5)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(6) */
#endif
#if (NUM_USB_CHAN_IN > 6)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(7) */
#endif
#if (NUM_USB_CHAN_IN > 7)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(8) */
#endif
#if (NUM_USB_CHAN_IN > 8)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(9) */
#endif
#if (NUM_USB_CHAN_IN > 9)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(10) */
#endif
#if (NUM_USB_CHAN_IN > 10)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(11) */
#endif
#if (NUM_USB_CHAN_IN > 11)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(12) */
#endif
#if (NUM_USB_CHAN_IN > 12)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(13) */
#endif
#if (NUM_USB_CHAN_IN > 13)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(14) */
#endif
#if (NUM_USB_CHAN_IN > 14)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(15) */
#endif
#if (NUM_USB_CHAN_IN > 15)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(16) */
#endif
#if (NUM_USB_CHAN_IN > 16)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(17) */
#endif
#if (NUM_USB_CHAN_IN > 17)
    0x0F, 0x00, 0x00, 0x00,         /* bmaControls(18) */
#endif
#if (NUM_USB_CHAN_IN > 18)
#error NUM_USB_CHAN > 18
#endif
    0,                             /* 60 iFeature */ 

    /* Output Terminal Descriptor (USB Streaming) */
    0x0C, 	           				/* 0  bLength */
    CS_INTERFACE,        			/* 1  bDescriptorType: 0x24 */
    OUTPUT_TERMINAL, 	           	/* 2  bDescriptorSubType: OUTPUT_TERMINAL */
    ID_OT_USB, 	           			/* 3  bTerminalID */
    (USB_STREAMING&0xff),USB_STREAMING>>8, /* 4  wTerminalType */
    0x00,        					/* 6  bAssocTerminal */
    FU_USBIN,    					/* 7  bSourceID Connect to analog input feature unit*/
    ID_CLKSEL, 	               		/* 8  bCSourceUD */
    0x00, 0x00,  					/* 9  bmControls */
    7,        						/* 11 iTerminal */
#endif

   

#ifdef MIXER
    /* Extension Unit Descriptor (4.7.2.12) */
    LEN_XU_MIX,                     /* 0    bLength (15 + p, when p is number of sources) */
    CS_INTERFACE,                   /* 1    bDescriptorType */
    EXTENSION_UNIT,                 /* 2    bDescriptorSubtype */
    ID_XU_MIXSEL,                   /* 3    bUnitID */
    0,                              /* 4    wExtensionCode */
    0,                              /* 5    wExtensionCode */
    2,                              /* 6    bNrPins */
    ID_IT_USB,                      /* 7    baSourceId(1) */
    ID_IT_AUD,                      /* 7    baSourceId(2) */
    MIX_INPUTS,                     /* 8+p  bNrChannels */
    0,                              /* 9+p  bmChannelConfig */  
    0,                              /* 10+p bmChannelConfig */  
    0,                              /* 11+p bmChannelConfig */  
    0,                              /* 12+p bmChannelConfig */  
    0,                              /* 13+p iChannelNames */
    3,                              /* 14+p bmControls */
    0,                              /* 15+p iExtension */




/* Mixer Unit Descriptors */


/* N = 144 (18 * 8) */
/* Mixer Unit Bitmap - 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff */
    MIXER_LENGTH,                  /* 0 bLength : 13 + num inputs + bit map (inputs * outputs) */
    CS_INTERFACE,                    /* 1  bDescriptorType: 0x24 */
    0x04,                            /* bDescriptorSubtype: MIXER_UNIT */
    ID_MIXER_1,                      /* Mixer unit id */
    0x01,                            /* Number of input pins */
    ID_XU_MIXSEL,                    /* Connected terminal or unit id for input pin */
    MAX_MIX_COUNT,                   /* Number of mixer output channels */
    0x00, 0x00, 0x00, 0x00,          /* Spacial location ???? */
    MIXER_STRING_INDEX,                              /* Channel name index */
#if MIX_BMCONTROLS_LEN > 0           /* Mixer programmable control bitmap */
    0xff, 
#endif    
#if MIX_BMCONTROLS_LEN > 1
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 2
    0xff, 
#endif      
#if MIX_BMCONTROLS_LEN > 3
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 4
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 5
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 6
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 7
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 8
    0xff, 
#endif 
#if MIX_BMCONTROLS_LEN > 9
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 10
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 11
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 12
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 13
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 14
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 15
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 16
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 17
    0xff, 
#endif   
#if MIX_BMCONTROLS_LEN > 18
#error unxpected BMCONTROLS_LEN
#endif   
    0x00,                           /* bmControls */
    0,                              /* Mixer unit string descriptor index */
#endif

#if defined(SPDIF_RX) || defined (ADAT_RX)
    /* Standard AS Interrupt Endpoint Descriptor (4.8.2.1): */ 
    0x07,                           /* 0  bLength: 7 */
    0x05,                           /* 1  bDescriptorType: ENDPOINT */
    EP_ADR_IN_AUD_INT,              /* 2  bEndpointAddress (D7: 0:out, 1:in) */
    3,                              /* 3  bmAttributes (bitmap)  */ 
    6,0,                            /* 4  wMaxPacketSize */
    8,                              /* 6  bInterval */
#endif

#ifdef OUTPUT
    /* Zero bandwith alternative 0 */
    /* Standard AS Interface Descriptor (4.9.1) */
    0x09,           				/* 0  bLength: (in bytes, 9) */
    USB_INTERFACE,           		/* 1  bDescriptorType: INTERFACE */
    1,              				/* 2  bInterfaceNumber: Number of interface */
    0,              				/* 3  bAlternateSetting */
    0,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    IP_VERSION_02_00,           	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    4,              				/* 8  iInterface: (Sting index) */

    /* Alternative 1 */
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    0x09,           				/* 0  bLength: (in bytes, 9) */
    USB_INTERFACE,           		/* 1  bDescriptorType: INTERFACE */
    1,              				/* 2  bInterfaceNumber: Number of interface */
    1,              				/* 3  bAlternateSetting */
    2,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    IP_VERSION_02_00,           	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    4,              				/* 8  iInterface: (Sting index) */

    /* Class Specific AS Interface Descriptor */
    0x10,           				/* 0  bLength: 16 */
    CS_INTERFACE,           		/* 1  bDescriptorType: 0x24 */
    AS_GENERAL,     				/* 2  bDescriptorSubType */
    ID_IT_USB,              		/* 3  bTerminalLink (Linked to USB input terminal) */
    0x00,           				/* 4  bmControls */
    0x01,           				/* 5  bFormatType */
    PCM, 0x00, 0x00, 0x00,  		/* 6:10  bmFormats (note this is a bitmap) */
    NUM_USB_CHAN_OUT,               /* 11 bNrChannels */
    0,0,0,0,    					/* 12:14: bmChannelConfig */
    OUTPUT_INTERFACE_STRING_INDEX,  /* 15 iChannelNames */

    /* Type 1 Format Type Descriptor */
    0x06,         					/* 0  bLength (in bytes): 6 */
    CS_INTERFACE,         			/* 1  bDescriptorType: 0x24 */
    FORMAT_TYPE,         			/* 2  bDescriptorSubtype: FORMAT_TYPE */
    FORMAT_TYPE_I,         			/* 3  bFormatType: FORMAT_TYPE_1 */
    0x04,         					/* 4  bSubslotSize (Number of bytes per subslot) */
    24,         					/* 5  bBitResolution (Number of bits used per subslot) */ 

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x01,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    0x05,              				/* 3  bmAttributes (bitmap)  */ 
    0,4,            				/* 4  wMaxPacketSize */
    1,              				/* 6  bInterval */

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    0x08,           				/* 0   bLength */
    CS_ENDPOINT,           			/* 1   bDescriptorType */
    0x01,           				/* 2   bDescriptorSubtype */
    0x00,           				/* 3   bmAttributes */
    0x00,           				/* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
    0x02,           				/* 5   bLockDelayUnits: Decoded PCM samples */
    8,0,            				/* 6:7 bLockDelay */

    /* Feedback EP */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x81,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    17,              				/* 3  bmAttributes (bitmap)  */ 
    4,0,            				/* 4  wMaxPacketSize */
    4,              				/* 6  bInterval. Only values <= 1 frame (4) supported by MS */

#ifdef NATIVE_DSD
     /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    0x09,           				/* 0  bLength: (in bytes, 9) */
    USB_INTERFACE,           		/* 1  bDescriptorType: INTERFACE */
    1,              				/* 2  bInterfaceNumber: Number of interface */
    ALT_SETTING_DSD,              	/* 3  bAlternateSetting */
    2,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    IP_VERSION_02_00,           	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    4,              				/* 8  iInterface: (Sting index) */

    /* Class Specific AS Interface Descriptor */
    0x10,           				/* 0  bLength: 16 */
    CS_INTERFACE,           		/* 1  bDescriptorType: 0x24 */
    AS_GENERAL,     				/* 2  bDescriptorSubType */
    ID_IT_USB,              		/* 3  bTerminalLink (Linked to USB input terminal) */
    0x00,           				/* 4  bmControls */
    0x01,           				/* 5  bFormatType */
    0x00, 0x00, 0x00, TYPE_1_RAW_DATA, /* 6:10  bmFormats (note this is a bitmap) */
    NUM_USB_CHAN_OUT,             	/* 11 bNrChannels */
    0,0,0,0,    					/* 12:14: bmChannelConfig */
    OUTPUT_INTERFACE_STRING_INDEX,  /* 15 iChannelNames */

    /* Type 1 Format Type Descriptor */
    0x06,         					/* 0  bLength (in bytes): 6 */
    CS_INTERFACE,         			/* 1  bDescriptorType: 0x24 */
    FORMAT_TYPE,         			/* 2  bDescriptorSubtype: FORMAT_TYPE */
    FORMAT_TYPE_I,         			/* 3  bFormatType: FORMAT_TYPE_1 */
    0x04,         					/* 4  bSubslotSize (Number of bytes per subslot) */
    32,         					/* 5  bBitResolution (Number of bits used per subslot) */ 

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x01,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    0x05,              				/* 3  bmAttributes (bitmap)  */ 
    0,4,            				/* 4  wMaxPacketSize */
    1,              				/* 6  bInterval */

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    0x08,           				/* 0   bLength */
    CS_ENDPOINT,           			/* 1   bDescriptorType */
    0x01,           				/* 2   bDescriptorSubtype */
    0x00,           				/* 3   bmAttributes */
    0x00,           				/* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
    0x02,           				/* 5   bLockDelayUnits: Decoded PCM samples */
    8,0,            				/* 6:7 bLockDelay */

    /* Feedback EP */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x81,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    17,              				/* 3  bmAttributes (bitmap)  */ 
    4,0,            				/* 4  wMaxPacketSize */
    4,              				/* 6  bInterval */

#endif /* NATIVE_DSD */


#ifdef ADAT_TX
     /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    0x09,           				/* 0  bLength: (in bytes, 9) */
    USB_INTERFACE,           		/* 1  bDescriptorType: INTERFACE */
    1,              				/* 2  bInterfaceNumber: Number of interface */
    ALT_SETTING_ADAT_TX,            /* 3  bAlternateSetting */
    2,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    IP_VERSION_02_00,           	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    4,              				/* 8  iInterface: (Sting index) */

    /* Class Specific AS Interface Descriptor */
    0x10,           				/* 0  bLength: 16 */
    CS_INTERFACE,           		/* 1  bDescriptorType: 0x24 */
    AS_GENERAL,     				/* 2  bDescriptorSubType */
    ID_IT_USB,              		/* 3  bTerminalLink (Linked to USB input terminal) */
    0x00,           				/* 4  bmControls */
    0x01,           				/* 5  bFormatType */
    PCM, 0x00, 0x00, 0x00,  		/* 6:10  bmFormats (note this is a bitmap) */
    NUM_USB_CHAN_OUT,             	/* 11 bNrChannels */
    0,0,0,0,    					/* 12:14: bmChannelConfig */
    INPUT_INTERFACE_STRING_INDEX,             				/* 15 iChannelNames */

    /* Type 1 Format Type Descriptor */
    0x06,         					/* 0  bLength (in bytes): 6 */
    CS_INTERFACE,         			/* 1  bDescriptorType: 0x24 */
    FORMAT_TYPE,         			/* 2  bDescriptorSubtype: FORMAT_TYPE */
    FORMAT_TYPE_I,         			/* 3  bFormatType: FORMAT_TYPE_1 */
    0x04,         					/* 4  bSubslotSize (Number of bytes per subslot) */
    24,         					/* 5  bBitResolution (Number of bits used per subslot) */ 

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x01,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    0x05,              				/* 3  bmAttributes (bitmap)  */ 
    0,4,            				/* 4  wMaxPacketSize */
    1,              				/* 6  bInterval */

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    0x08,           				/* 0   bLength */
    CS_ENDPOINT,           			/* 1   bDescriptorType */
    0x01,           				/* 2   bDescriptorSubtype */
    0x00,           				/* 3   bmAttributes */
    0x00,           				/* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
    0x02,           				/* 5   bLockDelayUnits: Decoded PCM samples */
    8,0,            				/* 6:7 bLockDelay */

    /* Feedback EP */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x81,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    17,              				/* 3  bmAttributes (bitmap)  */ 
    4,0,            				/* 4  wMaxPacketSize */
    4,              				/* 6  bInterval */

#endif /* ADAT_TX */
#endif /* OUTPUT */

#ifdef INPUT
    /* Standard AS Interface Descriptor (4.9.1) */
    0x09,          			 		/* 0  bLength: (in bytes, 9) */
	USB_INTERFACE,           		/* 1  bDescriptorType: INTERFACE */
    (OUTPUT_INTERFACES + 1),        /* 2  bInterfaceNumber: Number of interface */
    0,              				/* 3  bAlternateSetting */
    0,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    0x20,           				/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    5,              				/* 8  iInterface: (Sting index) */

    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    0x09,           				/* 0  bLength: (in bytes, 9) */
    USB_INTERFACE,           		/* 1  bDescriptorType: INTERFACE */
    (OUTPUT_INTERFACES + 1),        /* 2  bInterfaceNumber: Number of interface */
    1,              				/* 3  bAlternateSetting */
    1,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    IP_VERSION_02_00,           	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    5,              				/* 8  iInterface: (Sting index) */

    /* Class Specific AS Interface Descriptor */
    0x10,           				/* 0  bLength: 16 */
    CS_INTERFACE,           		/* 1  bDescriptorType: 0x24 */
    AS_GENERAL,     				/* 2  bDescriptorSubType */
    ID_OT_USB,              		/* 3  bTerminalLink */
    0x00,           				/* 4  bmControls */
    0x01,           				/* 5  bFormatType */
    PCM, 0x00, 0x00, 0x00,  		/* 6:10  bmFormats (note this is a bitmap) */
    NUM_USB_CHAN_IN,            /* 11 bNrChannels */
    0,0,0,0,    					/* 12:14: bmChannelConfig */
    INPUT_INTERFACE_STRING_INDEX,            				    /* 15 iChannelNames */

    /* Type 1 Format Type Descriptor */
    0x06,         					/* 0  bLength (in bytes): 6 */
    CS_INTERFACE,         			/* 1  bDescriptorType: 0x24 */
    FORMAT_TYPE,         			/* 2  bDescriptorSubtype: FORMAT_TYPE */	
	FORMAT_TYPE_I,         			/* 3  bFormatType: FORMAT_TYPE_1 */
    0x04,         					/* 4  bSubslotSize (Number of bytes per subslot) */
    24,         					/* 5  bBitResolution (Number of bits used per subslot) */ 

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x82,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    5,              				/* 3  bmAttributes (bitmap)  */ 
    0,4,            				/* 4  wMaxPacketSize */
    1,              				/* 6  bInterval */

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    0x08,           				/* 0   bLength */
    CS_ENDPOINT,           			/* 1   bDescriptorType */
    0x01,           				/* 2   bDescriptorSubtype */
    0x00,          					/* 3   bmAttributes */
    0x00,           				/* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
    0x02,           				/* 5   bLockDelayUnits: Decoded PCM samples */
    8,0,             				/* 6:7 bLockDelay */

#ifdef ADAT_RX
 /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    0x09,           				/* 0  bLength: (in bytes, 9) */
    USB_INTERFACE,           		/* 1  bDescriptorType: INTERFACE */
    (OUTPUT_INTERFACES + 1),        /* 2  bInterfaceNumber: Number of interface */
    2,              				/* 3  bAlternateSetting */
    1,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    IP_VERSION_02_00,           	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    5,              				/* 8  iInterface: (Sting index) */

    /* Class Specific AS Interface Descriptor */
    0x10,           				/* 0  bLength: 16 */
    CS_INTERFACE,           		/* 1  bDescriptorType: 0x24 */
    AS_GENERAL,     				/* 2  bDescriptorSubType */
    ID_OT_USB,              		/* 3  bTerminalLink */
    0x00,           				/* 4  bmControls */
    0x01,           				/* 5  bFormatType */
    PCM, 0x00, 0x00, 0x00,  		/* 6:10  bmFormats (note this is a bitmap) */
    NUM_USB_CHAN_IN - 4,            /* 11 bNrChannels */
    0,0,0,0,    					/* 12:14: bmChannelConfig */
    INPUT_INTERFACE_STRING_INDEX,            				    /* 15 iChannelNames */

    /* Type 1 Format Type Descriptor */
    0x06,         					/* 0  bLength (in bytes): 6 */
    CS_INTERFACE,         			/* 1  bDescriptorType: 0x24 */
    FORMAT_TYPE,         			/* 2  bDescriptorSubtype: FORMAT_TYPE */	
	FORMAT_TYPE_I,         			/* 3  bFormatType: FORMAT_TYPE_1 */
    0x04,         					/* 4  bSubslotSize (Number of bytes per subslot) */
    24,         					/* 5  bBitResolution (Number of bits used per subslot) */ 

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    0x07,           				/* 0  bLength: 7 */
    USB_ENDPOINT,           		/* 1  bDescriptorType: ENDPOINT */
    0x82,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    5,              				/* 3  bmAttributes (bitmap)  */ 
    0,4,            				/* 4  wMaxPacketSize */
    1,              				/* 6  bInterval */

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    0x08,           				/* 0   bLength */
    CS_ENDPOINT,           			/* 1   bDescriptorType */
    0x01,           				/* 2   bDescriptorSubtype */
    0x00,          					/* 3   bmAttributes */
    0x00,           				/* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
    0x02,           				/* 5   bLockDelayUnits: Decoded PCM samples */
    8,0,             				/* 6:7 bLockDelay */

#if 0
    /* Standard AS Interface Descriptor (4.9.1) (Alt) */
    0x09,           				/* 0  bLength: (in bytes, 9) */
    INTERFACE,           			/* 1  bDescriptorType: INTERFACE */
    (OUTPUT_INTERFACES + 1),        /* 2  bInterfaceNumber: Number of interface */
    3,              				/* 3  bAlternateSetting */
    1,              				/* 4  bNumEndpoints */
    AUDIO,           				/* 5  bInterfaceClass: AUDIO */
    AUDIOSTREAMING,           		/* 6  bInterfaceSubClass: AUDIO_STREAMING */
    IP_VERSION_02_00,           	/* 7  bInterfaceProtocol: IP_VERSION_02_00 */
    11,              				/* 8  iInterface: (Sting index) */

    /* Class Specific AS Interface Descriptor */
    0x10,           				/* 0  bLength: 16 */
    CS_INTERFACE,           		/* 1  bDescriptorType: 0x24 */
    AS_GENERAL,     				/* 2  bDescriptorSubType */
    ID_OT_USB,              		/* 3  bTerminalLink */
    0x00,           				/* 4  bmControls */
    0x01,           				/* 5  bFormatType */
    PCM, 0x00, 0x00, 0x00,  		/* 6:10  bmFormats (note this is a bitmap) */
    NUM_USB_CHAN_IN-6,                /* 11 bNrChannels */
    0,0,0,0,    					/* 12:14: bmChannelConfig */
    29,            				    /* 15 iChannelNames */

    /* Type 1 Format Type Descriptor */
    0x06,         					/* 0  bLength (in bytes): 6 */
    CS_INTERFACE,         			/* 1  bDescriptorType: 0x24 */
    FORMAT_TYPE,         			/* 2  bDescriptorSubtype: FORMAT_TYPE */	
	FORMAT_TYPE_I,         			/* 3  bFormatType: FORMAT_TYPE_1 */
    0x04,         					/* 4  bSubslotSize (Number of bytes per subslot) */
    24,         					/* 5  bBitResolution (Number of bits used per subslot) */ 

    /* Standard AS Isochronous Audio Data Endpoint Descriptor (4.10.1.1) */
    0x07,           				/* 0  bLength: 7 */
    ENDPOINT,           			/* 1  bDescriptorType: ENDPOINT */
    0x82,            				/* 2  bEndpointAddress (D7: 0:out, 1:in) */
    5,              				/* 3  bmAttributes (bitmap)  */ 
    0,4,            				/* 4  wMaxPacketSize */
    1,              				/* 6  bInterval */

    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor (4.10.1.2) */
    0x08,           				/* 0   bLength */
    CS_ENDPOINT,           			/* 1   bDescriptorType */
    0x01,           				/* 2   bDescriptorSubtype */
    0x00,          					/* 3   bmAttributes */
    0x00,           				/* 4   bmControls (Bitmap: Pitch control, over/underun etc) */
    0x02,           				/* 5   bLockDelayUnits: Decoded PCM samples */
    8,0,             				/* 6:7 bLockDelay */
#endif
#endif


#endif

#ifdef MIDI
/* MIDI Descriptors */
/* Table B-3: MIDI Adapter Standard AC Interface Descriptor */
    0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                            /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    (INPUT_INTERFACES + OUTPUT_INTERFACES + 1),/* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                            /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                            /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0x01,                            /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x01,                            /* 6 bInterfaceSubclass : AUDIO_CONTROL. (field size 1 bytes) */
    0x00,                            /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                            /* 8 iInterface : Unused. (field size 1 bytes) */

/* Table B-4: MIDI Adapter Class-specific AC Interface Descriptor */
    0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                            /* 1 bDescriptorType : 0x24. (field size 1 bytes) */
    0x01,                            /* 2 bDescriptorSubtype : HEADER subtype. (field size 1 bytes) */
    0x00,                            /* 3 bcdADC : Revision of class specification - 1.0 (field size 2 bytes) */
    0x01,                            /* 4 bcdADC */
    0x09,                            /* 5 wTotalLength : Total size of class specific descriptors. (field size 2 bytes) */
    0x00,                            /* 6 wTotalLength */
    0x01,                            /* 7 bInCollection : Number of streaming interfaces. (field size 1 bytes) */
    0x01,                            /* 8 baInterfaceNr(1) : MIDIStreaming interface 1 belongs to this AudioControl interface */

/* Table B-5: MIDI Adapter Standard MS Interface Descriptor */
    0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                            /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    (INPUT_INTERFACES+OUTPUT_INTERFACES+2),                            /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                            /* 3 bAlternateSetting : Index of this alternate setting. (field size 1 bytes) */
    0x02,                            /* 4 bNumEndpoints : 2 endpoints. (field size 1 bytes) */
    0x01,                            /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
    0x03,                            /* 6 bInterfaceSubclass : MIDISTREAMING. (field size 1 bytes) */
    0x00,                            /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                            /* 8 iInterface : Unused. (field size 1 bytes) */

/* Table B-6: MIDI Adapter Class-specific MS Interface Descriptor */
    0x07,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x01,                            /* 2 bDescriptorSubtype : MS_HEADER subtype. (field size 1 bytes) */
    0x00,                            /* 3 BcdADC : Revision of this class specification. (field size 2 bytes) */
    0x01,                            /* 4 BcdADC */
    0x41,                            /* 5 wTotalLength : Total size of class-specific descriptors. (field size 2 bytes) */
    0x00,                            /* 6 wTotalLength */

/* Table B-7: MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
    0x06,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x02,                            /* 2 bDescriptorSubtype : MIDI_IN_JACK subtype. (field size 1 bytes) */
    0x01,                            /* 3 bJackType : EMBEDDED. (field size 1 bytes) */
    0x01,                            /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    0x00,                            /* 5 iJack : Unused. (field size 1 bytes) */

/* Table B-8: MIDI Adapter MIDI IN Jack Descriptor (External) */
    0x06,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x02,                            /* 2 bDescriptorSubtype : MIDI_IN_JACK subtype. (field size 1 bytes) */
    0x02,                            /* 3 bJackType : EXTERNAL. (field size 1 bytes) */
    0x02,                            /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    MIDI_IN_STRING_INDEX,            /* 5 iJack : Unused. (field size 1 bytes) */

/* Table B-9: MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
    0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x03,                            /* 2 bDescriptorSubtype : MIDI_OUT_JACK subtype. (field size 1 bytes) */
    0x01,                            /* 3 bJackType : EMBEDDED. (field size 1 bytes) */
    0x03,                            /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    0x01,                            /* 5 bNrInputPins : Number of Input Pins of this Jack. (field size 1 bytes) */
    0x02,                            /* 6 BaSourceID(1) : ID of the Entity to which this Pin is connected. (field size 1 bytes) */
    0x01,                            /* 7 BaSourcePin(1) : Output Pin number of the Entityt o which this Input Pin is connected. */
    0x00,                            /* 8 iJack : Unused. (field size 1 bytes) */

/* Table B-10: MIDI Adapter MIDI OUT Jack Descriptor (External) */
    0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
    0x03,                            /* 2 bDescriptorSubtype : MIDI_OUT_JACK subtype. (field size 1 bytes) */
    0x02,                            /* 3 bJackType : EXTERNAL. (field size 1 bytes) */
    0x04,                            /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
    0x01,                            /* 5 bNrInputPins : Number of Input Pins of this Jack. (field size 1 bytes) */
    0x01,                            /* 6 BaSourceID(1) : ID of the Entity to which this Pin is connected. (field size 1 bytes) */
    0x01,                            /* 7 BaSourcePin(1) : Output Pin number of the Entity to which this Input Pin is connected. */
    MIDI_OUT_STRING_INDEX,           /* 8 iJack : Unused. (field size 1 bytes) */

/* Table B-11: MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
    0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x05,                            /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
    EP_ADR_OUT_MIDI,           /* 2 bEndpointAddress : OUT Endpoint 3. (field size 1 bytes) */
    0x02,                            /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
    0x00,                            /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) - has to be 0x200 for compliance*/
    0x02,                            /* 5 wMaxPacketSize */
    0x00,                            /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    0x00,                            /* 7 bRefresh : Unused. (field size 1 bytes) */
    0x00,                            /* 8 bSynchAddress : Unused. (field size 1 bytes) */

/* Table B-12: MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor */
    0x05,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x25,                            /* 1 bDescriptorType : CS_ENDPOINT descriptor (field size 1 bytes) */
    0x01,                            /* 2 bDescriptorSubtype : MS_GENERAL subtype. (field size 1 bytes) */
    0x01,                            /* 3 bNumEmbMIDIJack : Number of embedded MIDI IN Jacks. (field size 1 bytes) */
    0x01,                            /* 4 BaAssocJackID(1) : ID of the Embedded MIDI IN Jack. (field size 1 bytes) */

/* Table B-13: MIDI Adapter Standard Bulk IN Endpoint Descriptor */
    0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x05,                            /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
    EP_ADR_IN_MIDI,            /* 2 bEndpointAddress : IN Endpoint 3. (field size 1 bytes) */
    0x02,                            /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
    0x00,                            /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) - has to be 0x200 for compliance*/
    0x02,                            /* 5 wMaxPacketSize */
    0x00,                            /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
    0x00,                            /* 7 bRefresh : Unused. (field size 1 bytes) */
    0x00,                            /* 8 bSynchAddress : Unused. (field size 1 bytes) */

/* Table B-14: MIDI Adapter Class-specific Bulk IN Endpoint Descriptor */
    0x05,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x25,                            /* 1 bDescriptorType : CS_ENDPOINT descriptor (field size 1 bytes) */
    0x01,                            /* 2 bDescriptorSubtype : MS_GENERAL subtype. (field size 1 bytes) */
    0x01,                            /* 3 bNumEmbMIDIJack : Number of embedded MIDI OUT Jacks. (field size 1 bytes) */
    0x03,                             /* 4 BaAssocJackID(1) : ID of the Embedded MIDI OUT Jack. (field size 1 bytes) */
#endif

#ifdef DFU
	/* Standard DFU class Interface descriptor */
   	0x09,                          	/* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   	0x04,                           /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
   	(INPUT_INTERFACES+OUTPUT_INTERFACES+MIDI_INTERFACES+1),                           /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
   	0x00,                           /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
   	0x00,                           /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
   	0xFE,                           /* 5 bInterfaceClass : DFU. (field size 1 bytes) */
   	0x01,                           /* 6 bInterfaceSubclass : (field size 1 bytes) */
   	0x01,                           /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
        DFU_STRING_INDEX,               /* 8 iInterface : Unused. (field size 1 bytes) */

#if 0
	/* DFU 1.0 Run-Time DFU Functional Descriptor */
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
#endif
    
#ifdef IAP
    /* Interface descriptor */
   	0x09,                          	/* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   	0x04,                           /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
   	(INPUT_INTERFACES+OUTPUT_INTERFACES+MIDI_INTERFACES+DFU_INTERFACES+1),                           /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
   	0x00,                           /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
   	0x03,                           /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
   	0xFF,                           /* 5 bInterfaceClass : DFU. (field size 1 bytes) */
   	0xF0,                           /* 6 bInterfaceSubclass : (field size 1 bytes) */
   	0x00,                           /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
   	IAP_INTERFACE_STRING_INDEX,           /* 8 iInterface : Used. (field size 1 bytes) */

    /* iAP Bulk OUT Endpoint Descriptor */
    0x07,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x05,                            /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
    EP_ADR_OUT_IAP,                  /* 2 bEndpointAddress : OUT Endpoint 3. High bit isIn (field size 1 bytes) */
    0x02,                            /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
    0x00,                            /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) - has to be 0x200 for compliance*/
    0x02,                            /* 5 wMaxPacketSize */
    0x00,                            /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */

    /* iAP Bulk IN Endpoint Descriptor */
    0x07,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x05,                            /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
    EP_ADR_IN_IAP,                            /* 2 bEndpointAddress : IN Endpoint 5. (field size 1 bytes) */
    0x02,                            /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
    0x00,                            /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) - has to be 0x200 for compliance*/
    0x02,                            /* 5 wMaxPacketSize */
    0x00,                            /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */

    /* iAP Interrupt IN Endpoint Descriptor */
    0x07,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x05,                            /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
    EP_ADR_IN_IAP_INT,               /* 2 bEndpointAddress : IN Endpoint 6. (field size 1 bytes) */
    0x03,                            /* 3 bmAttributes : Interrupt, not shared. (field size 1 bytes) */
    0x40,                            /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) - has to be 0x40 for compliance*/
    0x00,                            /* 5 wMaxPacketSize */
    0x08,                            /* 6 bInterval : (2^(bInterval-1))/8 ms. Must be between 4 and 32ms (field size 1 bytes) */

#endif

#ifdef HID_CONTROLS
    /* HID */
    /* Interface descriptor details */
    9,                                /* 0  bLength : Size of descriptor in Bytes */
    4,                                /* 1  bDescriptorType (Interface: 0x04)*/
    INTERFACE_NUM_HID,                /* 2  bInterfacecNumber : Number of interface */
    0,                                /* 3  bAlternateSetting : Value used  alternate interfaces using SetInterface Request */
    1,                                /* 4: bNumEndpoints : Number of endpoitns for this interface (excluding 0) */
    3,                                /* 5: bInterfaceClass */
    0,                                /* 6: bInterfaceSubClass - no boot device */
    0,                                /* 7: bInterfaceProtocol*/
    0,                                /* 8  iInterface */



    /* The device implements HID Descriptor: */
    9,                                /* 0  bLength : Size of descriptor in Bytes */
    0x21,                             /* 1  bDescriptorType (HID) */
    0x10,                             /* 2  bcdHID */
    0x01,                             /* 3  bcdHID */
    0,                                /* 4  bCountryCode */
    1,                                /* 5  bNumDescriptors */
    0x22,                             /* 6  bDescriptorType[0] (Report) */
    sizeof(hidReportDescriptor) & 0xff,  /* 7  wDescriptorLength[0] */
    sizeof(hidReportDescriptor) >> 8,    /* 8  wDescriptorLength[0] */

    /* Endpoint descriptor (IN) */
    0x7,                              /* 0  bLength */
    5,                                /* 1  bDescriptorType */
    EP_ADR_IN_HID,                    /* 2  bEndpointAddress  */
    3,                                /* 3  bmAttributes (INTERRUPT) */
    64,                               /* 4  wMaxPacketSize */
    0,                                /* 5  wMaxPacketSize */
    8,                                /* 6  bInterval */

#endif







};



/* String table */
#ifdef SPDIF_RX
#define SPDIF_RX_NUM_STRS   1
#else
#define SPDIF_RX_NUM_STRS   0
#endif

#ifdef ADAT_RX
#define ADAT_RX_NUM_STRS    1
#else
#define ADAT_RX_NUM_STRS    0
#endif

#ifdef MIDI
#define MIDI_NUM_STRS       2
#else
#define MIDI_NUM_STRS       0
#endif

#ifdef DFU
#define DFU_NUM_STRS       1
#else
#define DFU_NUM_STRS       0
#endif

#define STR_INDEX_OUT_CHAN  (10 + SPDIF_RX_NUM_STRS + ADAT_RX_NUM_STRS + MIDI_NUM_STRS + DFU_NUM_STRS)
#define STR_INDEX_IN_CHAN   (STR_INDEX_OUT_CHAN + NUM_USB_CHAN_OUT)

#define VENDOR_STR_WITH_SPACE VENDOR_STR " "

#define APPEND_VENDOR_STR(x) VENDOR_STR_WITH_SPACE#x

#define APPEND_PRODUCT_STR_A2_(x) PRODUCT_STR_A2 #x
#define APPEND_PRODUCT_STR_A2(x) APPEND_PRODUCT_STR_A2_(x)

#define APPEND_PRODUCT_STR_A1_(x) PRODUCT_STR_A1 #x
#define APPEND_PRODUCT_STR_A1(x) APPEND_PRODUCT_STR_A1_(x)

static unsigned char strDescs[][40] = 
{
    "Langids",						            /* 0    LangIDs) place holder */ 
    APPEND_VENDOR_STR( ),                       // 1    iManufacturer (at MANUFACTURER_STRING_INDEX)

    /* Audio 2.0 Strings */
    PRODUCT_STR_A2,                             // 2    iProduct and iInterface for control interface (at PRODUCT_STR_INDEX)
    "",//SERIAL_STR,                            // 3    iSerialNumber (at SERIAL_STR_INDEX)
    APPEND_PRODUCT_STR_A2(),                    // 4    iInterface for Streaming interaces
    APPEND_PRODUCT_STR_A2(),                    // 5

    APPEND_PRODUCT_STR_A2(), 		            // 6    "USB Input Terminal" (User sees as output from host) 
    APPEND_PRODUCT_STR_A2(),  		            // 7    "USB Output Terminal" (User sees as input to host) 
 
    /* Audio 1.0 Strings */
    PRODUCT_STR_A1,                             // 8    iProduct and iInterface for control interface
	APPEND_PRODUCT_STR_A1(Output),              // 9    iInterface for Streaming interaces
	APPEND_PRODUCT_STR_A1(Input),               // 10
    APPEND_PRODUCT_STR_A1(Output), 	            // 11   "USB Input Terminal" (User sees as output from host) 
	APPEND_PRODUCT_STR_A1(Input),  	            // 12    "USB Output Terminal" (User sees as input to host) 

    APPEND_VENDOR_STR(Clock Selector),          // 13    iClockSel
    APPEND_VENDOR_STR(Internal Clock),          // 14    iClockSource
#ifdef SPDIF_RX
    APPEND_VENDOR_STR(S/PDIF Clock),            // iClockSource
#endif
#ifdef ADAT_RX
    APPEND_VENDOR_STR(ADAT Clock),              // iClockSource
#endif
#ifdef DFU
    APPEND_VENDOR_STR(DFU),                     // iInterface for DFU interface
#endif

#ifdef MIDI
    APPEND_VENDOR_STR(MIDI Out),                   // iJack for MIDI OUT
    APPEND_VENDOR_STR(MIDI In ),                   // iJack for MIDI IN
#endif
    
#if (NUM_USB_CHAN_OUT > 0)
    "Analogue 1",                                 // Output channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_OUT > 1)
    "Analogue 2",
#endif
#if (NUM_USB_CHAN_OUT > 2)
    "Analogue 3",
#endif
#if (NUM_USB_CHAN_OUT > 3)
    "Analogue 4",
#endif
#if (NUM_USB_CHAN_OUT > 4)
    "Analogue 5",
#endif
#if (NUM_USB_CHAN_OUT > 5)
    "Analogue 6",
#endif
#if (NUM_USB_CHAN_OUT > 6)
    "Analogue 7",
#endif
#if (NUM_USB_CHAN_OUT > 7)
    "Analogue 8",
#endif
#if (NUM_USB_CHAN_OUT > 8)
    "Analogue 9",
#endif
#if (NUM_USB_CHAN_OUT > 9)
    "Analogue 10",
#endif
#if (NUM_USB_CHAN_OUT > 10)
    "Analogue 11",
#endif
#if (NUM_USB_CHAN_OUT > 11)
    "Analogue 12",
#endif
#if (NUM_USB_CHAN_OUT > 12)
    "Analogue 13",
#endif
#if (NUM_USB_CHAN_OUT > 13)
    "Analogue 14",
#endif
#if (NUM_USB_CHAN_OUT > 14)
    "Analogue 15",
#endif
#if (NUM_USB_CHAN_OUT > 15)
    "Analogue 16",
#endif
#if (NUM_USB_CHAN_OUT > 16)
    "Analogue 17",
#endif
#if (NUM_USB_CHAN_OUT > 17)
    "Analogue 18",
#endif
#if (NUM_USB_CHAN_OUT > 18)
#error NUM_USB_CHAN > 18
#endif
    
#if (NUM_USB_CHAN_IN > 0)
    "Analogue 1",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 1)
    "Analogue 2",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 2)
    "Analogue 3",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 3)
    "Analogue 4",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 4)
    "Analogue 5",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 5)
    "Analogue 6",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 6)
    "Analogue 7",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 7)
    "Analogue 8",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 8)
    "Analogue 9",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 9)
    "Analogue 10",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 10)
    "Analogue 11",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 11)
    "Analogue 12",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 12)
    "Analogue 13",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 13)
    "Analogue 14",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 14)
    "Analogue 15",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 15)
    "Analogue 16",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 16)
    "Analogue 17",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 17)
    "Analogue 18",                                  // Input channel name place holders - Get customised at runtime based on device config 
#endif
#if (NUM_USB_CHAN_IN > 18)
#error NUM_USB_CHAN > 18
#endif

#ifdef MIXER
#if (MAX_MIX_COUNT > 0)
    "Mixer Out 1",                              // /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 1)
    "Mixer Out 2",                              //  /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 2)
    "Mixer Out 3",                              //  /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 3)
    "Mixer Out 4",                              //  /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 4)
    "Mixer Out 5",                              //  /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 5)
    "Mixer Out 6",                              //  /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 6)
    "Mixer Out 7",                              //  /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 7)
    "Mixer Out 8",                              //  /* Mixer output names */
#endif
#if (MAX_MIX_COUNT > 8)
#error MAX_MIX_COUNT > 8
#endif
#endif

#ifdef IAP
    "iAP Interface",                            //  /* Required name for iAP interface */
#endif
};

/* Configuration Descriptor for Null device */
unsigned char cfgDesc_Null[] = 
{
    0x09,                          	/* 0  bLength */
    USB_CONFIGURATION,		        /* 1  bDescriptorType */
    0x12,                           /* 2  wTotalLength */
    0x00,                           /* 3  wTotalLength */
    0x01,                           /* 4  bNumInterface: Number of interfaces*/
    0x01,                           /* 5  bConfigurationValue */
    0x00,                           /* 6  iConfiguration */
#ifdef SELF_POWERED
    192,                            /* 7  bmAttributes */
#else
    128, 
#endif
    BMAX_POWER,                     /* 8  bMaxPower */

    0x09,                           /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                           /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    0x00,                         	/* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                          	/* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                         	/* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0x00,                        	/* 5 bInterfaceClass :  */
    0x00,                          	/* 6 bInterfaceSubclass */
    0x00,                          	/* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                          	/* 8 iInterface : Unused. (field size 1 bytes) */
    0x09,            				/* 0  bLength */ 
};

#if 0
/* OtherSpeed Configuration Descriptor */
unsigned char oSpeedCfgDesc[] =
{
    0x09,                          	/* 0  bLength */
    OTHER_SPEED_CONFIGURATION,		/* 1  bDescriptorType */
    0x12,                           /* 2  wTotalLength */
    0x00,                           /* 3  wTotalLength */
    0x01,                           /* 4  bNumInterface: Number of interfaces*/
    0x00,                           /* 5  bConfigurationValue */
    0x00,                           /* 6  iConfiguration */
   	128,                            /* 7  bmAttributes */
    250,                            /* 8  bMaxPower */

    0x09,                           /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
    0x04,                           /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
    0x00,                         	/* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
    0x00,                          	/* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
    0x00,                         	/* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
    0x00,                        	/* 5 bInterfaceClass :  */
    0x00,                          	/* 6 bInterfaceSubclass */
    0x00,                          	/* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
    0x00,                          	/* 8 iInterface : Unused. (field size 1 bytes) */

};
#endif

/* Configuration descriptor for Audio v1.0 */      
#define AC_LENGTH                   (8 + INPUT_INTERFACES + OUTPUT_INTERFACES)
#define AC_TOTAL_LENGTH             (AC_LENGTH + (INPUT_INTERFACES * 31) + (OUTPUT_INTERFACES * 31))
#define STREAMING_INTERFACES        (INPUT_INTERFACES + OUTPUT_INTERFACES)


#define CFG_TOTAL_LENGTH_A1            (18 + AC_TOTAL_LENGTH + (INPUT_INTERFACES * 61) + (OUTPUT_INTERFACES * 70))
#ifdef AUDIO_CLASS_FALLBACK
unsigned char cfgDesc_Audio1[] = 
{                       
    /* Configuration descriptor */
    0x09, 
    USB_CONFIGURATION, 
    (CFG_TOTAL_LENGTH_A1 & 0xFF),   /* wTotalLength */ 
    (CFG_TOTAL_LENGTH_A1 >> 8),     /* wTotalLength */
    NUM_INTERFACES_A1,              /* numInterfaces - we dont support MIDI in audio 1.0 mode*/
    0x01,                           /* ID of this configuration */
    0x00,                           /* Unused */
#ifdef SELF_POWERED
    192,                            /* 7  bmAttributes */
#else
    128,                            /* 7  bmAttributes */ 
#endif  
    BMAX_POWER,             		/* 8  bMaxPower */  

    /* Standard AC interface descriptor */
    0x09, 
    USB_INTERFACE, 
    0x00,                           /* Interface No */
    0x00,                           /* Alternate setting*/
    0x00,                           /* Num endpoints */
    AUDIO, 
    AUDIOCONTROL,                   
    0x00,                           /* Unused */
    8,                              /* iInterface - re-use iProduct */ 

    /* CS (Class Specific) AudioControl interface header descriptor (4.3.2) */
    AC_LENGTH, 
    CS_INTERFACE, 
    0x01,                           /* HEADER */
    0x00, 0x01,                     /* Class spec revision - 1.0 */
    (AC_TOTAL_LENGTH & 0xFF),       /* wTotallength (Combined length of this descriptor and all Unit and Terminal Descriptors) */ 
    (AC_TOTAL_LENGTH >> 8),         /* wTotalLength */
    STREAMING_INTERFACES,           /* Num streaming interfaces */
#ifdef OUTPUT
    0x01,                           /* AudioStreaming interface 1 belongs to AC interface */
#endif
#ifdef INPUT
    (OUTPUT_INTERFACES + 1),        /* AudioStreaming interface 2 belongs to AC interface */
#endif

#ifdef OUTPUT
    /* CS_Interface Input Terminal 1 Descriptor - USB streaming Host to Device */
    0x0C, 
    CS_INTERFACE,                   /* CS_INTERFACE */
    0x02,                           /* INPUT_TERMINAL */
    0x01,                           /* Terminal ID */
    0x01, 0x01,                     /* Type - streaming */
    0x00,                           /* Associated terminal - unused  */
    2,                              /* bNrChannels */
    0x03, 0x00,                     /* wChannelConfig */
    0x00,                           /* iChannelNames - Unused */
    11,                           /* iTerminal */

    /* CS_Interface class specific AC interface feature unit descriptor - mute & volume for dac */
    0x0A, 
    CS_INTERFACE,   
    FEATURE_UNIT,                           
    0x0A,                           /* unitID */
    0x01,                           /* sourceID - ID of the unit/terminal to which this feature unit is connected */
    0x01,                           /* controlSize - 1 */
    0x00,                           /* bmaControls(0) */
    0x03,                           /* bmaControls(1) */
    0x03,                           /* bmaControls(2) */
    0x00,                           /* String table index */  
    
    /* CS_Interface Output Terminal Descriptor - Analogue out to speaker */
    0x09, 
    CS_INTERFACE,  
    0x03,                           /* OUTPUT_TERMINAL */
    0x06,                           /* Terminal ID */
    0x01, 0x03,                     /* Type - streaming out, speaker */
    0x00,                           /* Associated terminal - unused */
    0x0A,                           /* sourceID  */
    0x00,                           /* Unused */

#endif

#ifdef INPUT
    /* CS_Interface Input Terminal 2 Descriptor - Analog in from line in */
    0x0C, 
    CS_INTERFACE, 
    0x02,                           /* INPUT_TERMINAL */
    0x02,                           /* Terminal ID */   
    0x01, 0x02,                     /* Type - streaming in, mic */
    0x00,                           /* Associated terminal - unused  */
    2,                              /* bNrChannels */          
    0x03, 0x00,                     /* wChannelConfigs */   
    0x00,                           /* iChannelNames */          
    12,                             /* iTerminal */                          
    
    /* CS_Interface Output Terminal Descriptor - USB Streaming Device to Host*/
    0x09, 
    CS_INTERFACE,   
    0x03,                           /* OUTPUT_TERMINAL */
    0x07,                           /* Terminal ID */    
    0x01, 0x01,                     /* Type - streaming */
    0x01,                           /* Associated terminal - unused */    
    0x0B,                           /* sourceID - from selector unit ?? */
    0x00,                           /* Unused */                          
    
    /* CS_Interface class specific AC interface feature unit descriptor - mute & volume for adc */
    0x0A, 
    CS_INTERFACE,                                                              
    FEATURE_UNIT,                                                             
    0x0B,                           /* unitID */                                                                    
    0x02,                           /* sourceID - ID of the unit/terminal to which this feature unit is connected */
    0x01,                           /* controlSize - 1 */                                      
    0x00,                           /* bmaControls(0) */     
    0x03,                           /* bmaControls(1) */     
    0x03,                           /* bmaControls(2) */                                                         0x00,                           /* String table index */                                                        
#endif


#ifdef OUTPUT
    /* Standard AS Interface Descriptor (4.5.1) */
    0x09,                           /* bLength */
    0x04,                           /* INTERFACE */
    0x01,                           /* bInterfaceNumber */
    0x00,                           /* bAlternateSetting */ 
    0x00,                           /* bnumEndpoints */
    0x01,                           /* bInterfaceClass - AUDIO */
    0x02,                           /* bInterfaceSubclass - AUDIO_STREAMING */
    0x00,                           /* bInterfaceProtocol - Not used */
    0x09,                           /* iInterface */

    /* Standard As Interface Descriptor (4.5.1) */
    0x09, 
    0x04,                           /* INTERFACE */           
    0x01,                           /* Interface no */              
    0x01,                           /* AlternateSetting */           
    0x02,                           /* num endpoints 2: audio EP and feedback EP */              
    0x01,                           /* Interface class - AUDIO */    
    0x02,                           /* subclass - AUDIO_STREAMING */ 
    0x00,                           /* Unused */                     
    0x04,                           /* String table index  */                      

    /* Class-Specific AS Interface Descriptor (4.5.2) */
    0x07, 
    CS_INTERFACE,                   /* bDescriptorType */                                         
    0x01,                           /* bDescriptorSubtype - GENERAL */
    0x01,                           /* iTerminalLink - linked to Streaming IN terminal */
    0x01,                           /* bDelay */
    0x01, 0x00,                     /* wFormatTag - PCM */

    /* CS_Interface Format Type Descriptor */
    0x14, 
    CS_INTERFACE, 
    0x02,                           /* Subtype - FORMAT_TYPE */
    0x01,                           /* Format type - FORMAT_TYPE_1 */
    2,                              /* nrChannels */
    0x03,                           /* subFrameSize - 4 bytes per slot */
    24,                             /* bitResolution - 24bit */
    0x04,                           /* SamFreqType - 4 sample freq */
    0x44, 0xAC, 0x00,               /* sampleFreq - 44.1Khz */
    0x80, 0xBB, 0x00,               /* sampleFreq - 48KHz */ 
#if defined(OUTPUT) && defined(INPUT)
    0x80, 0xBB, 0x00,               /* sampleFreq - 48KHz */ 
    0x80, 0xBB, 0x00,               /* sampleFreq - 48KHz */ 
#else
    0x88, 0x58, 0x01,               /* sampleFreq - 88.2KHz */ 
    0x00, 0x77, 0x01,               /* sampleFreq - 96KHz */ 
#endif

    /* Standard AS Isochronous Audio Data Endpoint Descriptor 4.6.1.1 */
    0x09, 
    0x05,                           /* ENDPOINT */
    0x01,                           /* endpointAddress - D7, direction (0 OUT, 1 IN). D6..4 reserved (0). D3..0 endpoint no. */ 
    0x05,                           /* attributes - isochronous async */
#if defined(OUTPUT) && defined(INPUT)  
    0x26, 0x01,                     /* maxPacketSize 294  */         
#else
    0x46, 0x02,                     /* maxPacketSize 582 */         
#endif
    0x01,                           /* bInterval */
    0x00,                           /* bRefresh */
    0x81,                           /* bSynchAdddress - address of EP used to communicate sync info */

    /* CS_Endpoint Descriptor ?? */
    0x07, 
    0x25,                           /* CS_ENDPOINT */
    0x01,                           /* subtype - GENERAL */
    0x01,                           /* attributes. D[0]: sample freq ctrl. */
    0x02,                           /* bLockDelayUnits */
    0x00, 0x00,                     /* bLockDelay */

    /* Feedback EP */
    0x09,     
    0x05,                           /* bDescriptorType: ENDPOINT */
    0x81,                           /* bEndpointAddress (D3:0 - EP no. D6:4 - reserved 0. D7 - 0:out, 1:in) */
    0x01,                           /* bmAttributes (bitmap)  */ 
    0x03,0x0,                       /* wMaxPacketSize */
    0x01,                           /* bInterval - Must be 1 for compliance */
    0x04,                           /* bRefresh 2^x */
    0x0,                            /* bSynchAddress */
#endif

#ifdef INPUT
    /* Standard Interface Descriptor - Audio streaming IN */
    0x09, 
    0x04,                           /* INTERFACE */                   
    (OUTPUT_INTERFACES + 1),        /* bInterfaceNumber*/               
    0x00,                           /* AlternateSetting */            
    0x00,                           /* num endpoints */               
    0x01,                           /* Interface class - AUDIO */     
    0x02,                           /* subclass - AUDIO_STREAMING */  
    0x00,                           /* Unused */                      
    0x05,                           /* String table index */                      

    /* Standard Interface Descriptor - Audio streaming IN */
    0x09, 
    0x04,                           /* INTERFACE */                   
    (OUTPUT_INTERFACES + 1),        /* bInterfaceNumber */               
    0x01,                           /* AlternateSetting */            
    0x01,                           /* num endpoints */                   
    0x01,                           /* Interface class - AUDIO */     
    0x02,                           /* Subclass - AUDIO_STREAMING */  
    0x00,                           /* Unused */                      
    0x0A,                           /* String table index */                      

    /* CS_Interface AC interface header descriptor */
    0x07, 
    CS_INTERFACE,                                         
    0x01,                           /* subtype - GENERAL */                                    
    0x07,                           /* TerminalLink - linked to Streaming OUT terminal */       
    0x01,                           /* Interface delay */                                      
    0x01,0x00,                      /* Format - PCM */                                   

    /* CS_Interface Terminal Descriptor */
    0x14, 
    CS_INTERFACE,                   
    0x02,                           /* Subtype - FORMAT_TYPE */          
    0x01,                           /* Format type - FORMAT_TYPE_1 */    
    2,                              /* bNrChannels - 2 */                 
    0x03,                           /* subFrameSize - 4 bytes per slot */
    24,                             /* bitResolution - 24bit */          
    0x04,                           /* SamFreqType - 4 sample freq */    
    0x44, 0xAC, 0x00,               /* sampleFreq - 44.1Khz */
    0x80, 0xBB, 0x00,               /* sampleFreq - 48KHz */ 
#if defined(OUTPUT) && defined(INPUT)
    0x80, 0xBB, 0x00,               /* sampleFreq - 48KHz */ 
    0x80, 0xBB, 0x00,               /* sampleFreq - 48KHz */ 
#else
    0x88, 0x58, 0x01,               /* sampleFreq - 88.2KHz */ 
    0x00, 0x77, 0x01,               /* sampleFreq - 96KHz */ 
#endif

    /* Standard Endpoint Descriptor */ 
    0x09, 
    0x05,                           /* ENDPOINT */       
    0x82,                           /* EndpointAddress */
    0x05,                           /* Attributes - isochronous async */
#if defined(OUTPUT) && defined(INPUT)  
    0x26, 0x01,                     /* maxPacketSize 294  */         
#else
    0x46, 0x02,                     /* maxPacketSize 582 */         
#endif
    0x01,                           /* bInterval */             
    0x00,                           /* bRefresh */                                       
    0x00,                           /* bSynchAddress */                                       

    /* CS_Endpoint Descriptor */
    0x07, 
    0x25,                           /* CS_ENDPOINT */                                
    0x01,                           /* Subtype - GENERAL */                          
    0x01,                           /* Attributes. D[0]: sample freq ctrl. */        
    0x00,                           /* Unused */                                     
    0x00, 0x00,                     /* Unused */  

#endif
#if 0
	/* Standard DFU class Interface descriptor */
    /* NOTE, DFU DISABLED FOR AUDIO CLASS 1.0 BY DEFAULT DUE TO WINDOWS REQUESTING DRIVER */
   	0x09,                          	/* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   	0x04,                           /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
   	(INPUT_INTERFACES+OUTPUT_INTERFACES+1),                           /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
   	0x00,                           /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
   	0x00,                           /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
   	0xFE,                           /* 5 bInterfaceClass : DFU. (field size 1 bytes) */
   	0x01,                           /* 6 bInterfaceSubclass : (field size 1 bytes) */
   	0x01,                           /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
   	8,                             /* 8 iInterface : Unused. (field size 1 bytes) */
#endif
#if 0
	/* DFU 1.0 Run-Time DFU Functional Descriptor */
   	0x07,
   	0x21,
   	0x07,
   	0xFA,
   	0x00,
   	0x40,
   	0x00
#else
#if 0
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
#endif
};

#endif

#define APPEND_VENDOR_STR(x) VENDOR_STR#x
#if 0
static unsigned char strDescs_Audio1[][40] =
{
	"Langids",						            /* String 0 (LangIDs) place holder */ 
	APPEND_VENDOR_STR(),                       // 1    iManufacturer
	APPEND_VENDOR_STR(USB Audio 1.0),           // 2    iProduct and iInterface for control interface
	"",//SERIAL_STR,                                 // 3    iSerialNumber

	APPEND_VENDOR_STR(USB 1.0 Audio Out),       // 4    iInterface for Streaming interaces
	APPEND_VENDOR_STR(USB 1.0 Audio In),        // 5

    APPEND_VENDOR_STR(Audio 1.0 Output), 		// 6    "USB Input Terminal" (User sees as output from host) 
	APPEND_VENDOR_STR(Audio 1.0 Input),  		// 7    "USB Output Terminal" (User sees as input to host) 
    
    APPEND_VENDOR_STR(DFU)                      // 8     iInterface for DFU interface
};                                                                                                                                                              
#endif      
#endif      
