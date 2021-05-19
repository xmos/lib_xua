// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/* This file contains the MID device descriptor
   It is intended to be included in the main device descriptor definition */

/* MIDI Descriptors */
/* Table B-3: MIDI Adapter Standard AC Interface Descriptor */
   0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x04,                            /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
   MIDI_INTERFACE_1,                /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
   0x00,                            /* 3 bAlternateSetting : Index of this setting. (field size 1 bytes) */
   0x00,                            /* 4 bNumEndpoints : 0 endpoints. (field size 1 bytes) */
   0x01,                            /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
   0x01,                            /* 6 bInterfaceSubclass : AUDIO_CONTROL. (field size 1 bytes) */
   0x00,                            /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
   0x00,                            /* 8 iInterface : Unused. (field size 1 bytes) */
   // 9
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
   //9
/* Table B-5: MIDI Adapter Standard MS Interface Descriptor */
   0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x04,                            /* 1 bDescriptorType : INTERFACE descriptor. (field size 1 bytes) */
   MIDI_INTERFACE_2,                /* 2 bInterfaceNumber : Index of this interface. (field size 1 bytes) */
   0x00,                            /* 3 bAlternateSetting : Index of this alternate setting. (field size 1 bytes) */
   0x02,                            /* 4 bNumEndpoints : 2 endpoints. (field size 1 bytes) */
   0x01,                            /* 5 bInterfaceClass : AUDIO. (field size 1 bytes) */
   0x03,                            /* 6 bInterfaceSubclass : MIDISTREAMING. (field size 1 bytes) */
   0x00,                            /* 7 bInterfaceProtocol : Unused. (field size 1 bytes) */
   0x00,                            /* 8 iInterface : Unused. (field size 1 bytes) */
   //9
/* Table B-6: MIDI Adapter Class-specific MS Interface Descriptor */
   0x07,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
   0x01,                            /* 2 bDescriptorSubtype : MS_HEADER subtype. (field size 1 bytes) */
   0x00,                            /* 3 BcdADC : Revision of this class specification. (field size 2 bytes) */
   0x01,                            /* 4 BcdADC */
   0x41,                            /* 5 wTotalLength : Total size of class-specific descriptors. (field size 2 bytes) */
   0x00,                            /* 6 wTotalLength */
   //7
/* Table B-7: MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
   0x06,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
   0x02,                            /* 2 bDescriptorSubtype : MIDI_IN_JACK subtype. (field size 1 bytes) */
   0x01,                            /* 3 bJackType : EMBEDDED. (field size 1 bytes) */
   0x01,                            /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
   0x00,                            /* 5 iJack : Unused. (field size 1 bytes) */
   //6
/* Table B-8: MIDI Adapter MIDI IN Jack Descriptor (External) */
   0x06,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
   0x02,                            /* 2 bDescriptorSubtype : MIDI_IN_JACK subtype. (field size 1 bytes) */
   0x02,                            /* 3 bJackType : EXTERNAL. (field size 1 bytes) */
   0x02,                            /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
   0x00,                            /* 5 iJack : Unused. (field size 1 bytes) */
   //6
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
   //9
/* Table B-10: MIDI Adapter MIDI OUT Jack Descriptor (External) */
   0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x24,                            /* 1 bDescriptorType : CS_INTERFACE. (field size 1 bytes) */
   0x03,                            /* 2 bDescriptorSubtype : MIDI_OUT_JACK subtype. (field size 1 bytes) */
   0x02,                            /* 3 bJackType : EXTERNAL. (field size 1 bytes) */
   0x04,                            /* 4 bJackID : ID of this Jack. (field size 1 bytes) */
   0x01,                            /* 5 bNrInputPins : Number of Input Pins of this Jack. (field size 1 bytes) */
   0x01,                            /* 6 BaSourceID(1) : ID of the Entity to which this Pin is connected. (field size 1 bytes) */
   0x01,                            /* 7 BaSourcePin(1) : Output Pin number of the Entity to which this Input Pin is connected. */
   0x00,                            /* 8 iJack : Unused. (field size 1 bytes) */
   //9
/* Table B-11: MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
   0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x05,                            /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
   0x04,                            /* 2 bEndpointAddress : OUT Endpoint 4. (field size 1 bytes) */
   0x02,                            /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
   0x00,                            /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) */
   0x02,                            /* 5 wMaxPacketSize */
   0x00,                            /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
   0x00,                            /* 7 bRefresh : Unused. (field size 1 bytes) */
   0x00,                            /* 8 bSynchAddress : Unused. (field size 1 bytes) */
   //9
/* Table B-12: MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor */
   0x05,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x25,                            /* 1 bDescriptorType : CS_ENDPOINT descriptor (field size 1 bytes) */
   0x01,                            /* 2 bDescriptorSubtype : MS_GENERAL subtype. (field size 1 bytes) */
   0x01,                            /* 3 bNumEmbMIDIJack : Number of embedded MIDI IN Jacks. (field size 1 bytes) */
   0x01,                            /* 4 BaAssocJackID(1) : ID of the Embedded MIDI IN Jack. (field size 1 bytes) */
   //5
/* Table B-13: MIDI Adapter Standard Bulk IN Endpoint Descriptor */
   0x09,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x05,                            /* 1 bDescriptorType : ENDPOINT descriptor. (field size 1 bytes) */
   0x85,                            /* 2 bEndpointAddress : IN Endpoint 5. (field size 1 bytes) */
   0x02,                            /* 3 bmAttributes : Bulk, not shared. (field size 1 bytes) */
   0x00,                            /* 4 wMaxPacketSize : 64 bytes per packet. (field size 2 bytes) */
   0x02,                            /* 5 wMaxPacketSize */
   0x00,                            /* 6 bInterval : Ignored for Bulk. Set to zero. (field size 1 bytes) */
   0x00,                            /* 7 bRefresh : Unused. (field size 1 bytes) */
   0x00,                            /* 8 bSynchAddress : Unused. (field size 1 bytes) */
   //9
/* Table B-14: MIDI Adapter Class-specific Bulk IN Endpoint Descriptor */
   0x05,                            /* 0 bLength : Size of this descriptor, in bytes. (field size 1 bytes) */
   0x25,                            /* 1 bDescriptorType : CS_ENDPOINT descriptor (field size 1 bytes) */
   0x01,                            /* 2 bDescriptorSubtype : MS_GENERAL subtype. (field size 1 bytes) */
   0x01,                            /* 3 bNumEmbMIDIJack : Number of embedded MIDI OUT Jacks. (field size 1 bytes) */
   0x03,                             /* 4 BaAssocJackID(1) : ID of the Embedded MIDI OUT Jack. (field size 1 bytes) */
   //5
