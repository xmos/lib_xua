// Copyright 2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __XUA_EP0_MIDI_DESCRIPTORS__
#define __XUA_EP0_MIDI_DESCRIPTORS__

#include "xud_std_descriptors.h"

/* Table B-4: MIDI Adapter Class-specific AC Interface Descriptor */
typedef struct
{
    unsigned char bLength;             /* Size of the descriptor (bytes) */
    unsigned char bDescriptorType;     /* Type of the descriptor, either a value in \ref USB_DescriptorTypes_t
                                        * or a value given by the specific class */
    unsigned char  bDescriptorSubtype;
	unsigned short bcdADC;              /* Binary coded decimal indicating the supported Audio Class version */
    unsigned short wTotalLength;        /* Total size of class specific descriptors */

    unsigned char bInCollection;       /* Number of streaming interfaces */
    unsigned char baInterfaceNr;       /* MIDIStreaming interface 1 belongs to this AudioControl interface. */
} __attribute__((packed)) MIDI_CS_AC_Interface_Descriptor_t;

typedef struct
{
    unsigned char  bLength;             /* Size of  descriptor (bytes) */
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned short bcdADC;              /* Binary coded decimal indicating the supported Audio Class version */
    unsigned short wTotalLength;        /* Total size of the class-specific descriptors */
} __attribute__((packed)) MIDI_CS_MS_Interface_Descriptor_t;

typedef struct
{
    unsigned char  bLength;             /* Size of  descriptor (bytes) */
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bJackType;
    unsigned char  bJackID;
    unsigned char  iJack;
}__attribute__((packed)) MIDI_IN_Jack_Descriptor_t;

typedef struct
{
    unsigned char  bLength;             /* Size of  descriptor (bytes) */
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bJackType;
    unsigned char  bJackID;             /* ID of this Jack */
    unsigned char  bNrInputPins;        /* Number of input Pins of this Jack */
    unsigned char  BaSourceID;          /* ID of the entity to which this Pin is connected*/
    unsigned char  BaSourcePin;         /* Output Pin number the entity to which this Input Pin is connected*/
    unsigned char  iJack;
}__attribute__((packed)) MIDI_OUT_Jack_Descriptor_t;

typedef struct
{
    unsigned char  bLength;             /* Size of the descriptor (bytes) */
    unsigned char  bDescriptorType;     /* ENDPOINT descriptor*/
    unsigned char  bEndpointAddress;    /* Address of the endpoint, includes a direction mask */
    unsigned char  bmAttributes;        /* Bulk, not shared*/
    unsigned short wMaxPacketSize;      /* Maximum packet size (bytes) that the endpoint can receive */
    unsigned char  bInterval;           /* Ignored for Bulk. Set to 0 */
    unsigned char  bRefresh;            /* Unused*/
    unsigned char  bSynchAddress;       /* Unused*/
} __attribute__((packed)) MIDI_Standard_Bulk_Endpoint_Descriptor_t;

typedef struct
{
    unsigned char  bLength;             /* Size of the descriptor (bytes) */
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bNumEmbMIDIJack;     /* Number of embedded MIDI Jacks */
    unsigned char  BaAssocJackID;       /* ID of the embedded MIDI Jack */
} __attribute__((packed)) MIDI_CS_MS_Bulk_Endpoint_Descriptor_t;

typedef struct
{
    /* Table B-3: MIDI Adapter Standard AC Interface Descriptor */
    USB_Descriptor_Interface_t  MIDI_Std_AC_Interface; // 9

    /* Table B-4: MIDI Adapter Class-specific AC Interface Descriptor */
    MIDI_CS_AC_Interface_Descriptor_t   MIDI_CS_AC_Interface; // 9

    /* Table B-5: MIDI Adapter Standard MS Interface Descriptor */
    USB_Descriptor_Interface_t          MIDI_Std_MS_Interface; // 9

    /* Table B-6: MIDI Adapter Class-specific MS Interface Descriptor */
    MIDI_CS_MS_Interface_Descriptor_t   MIDI_CS_MS_Interface; // 7

    /* Table B-7: MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
    MIDI_IN_Jack_Descriptor_t           MIDI_IN_Jack_Embedded; // 6

    /* Table B-8: MIDI Adapter MIDI IN Jack Descriptor (External) */
    MIDI_IN_Jack_Descriptor_t           MIDI_IN_Jack_External; // 6

    /* Table B-9: MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
    MIDI_OUT_Jack_Descriptor_t          MIDI_OUT_Jack_Embedded; // 9

    /* Table B-10: MIDI Adapter MIDI OUT Jack Descriptor (External) */
    MIDI_OUT_Jack_Descriptor_t          MIDI_OUT_Jack_External; // 9

    /* Table B-11: MIDI Adapter Standard Bulk OUT Endpoint Descriptor */
    MIDI_Standard_Bulk_Endpoint_Descriptor_t    MIDI_Standard_Bulk_OUT_Endpoint; // 9

    /* Table B-12: MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor */
    MIDI_CS_MS_Bulk_Endpoint_Descriptor_t          MIDI_CS_Bulk_OUT_Endpoint; // 5

    /* Table B-13: MIDI Adapter Standard Bulk IN Endpoint Descriptor */
    MIDI_Standard_Bulk_Endpoint_Descriptor_t    MIDI_Standard_Bulk_IN_Endpoint; // 9

    /* Table B-14: MIDI Adapter Class-specific Bulk IN Endpoint Descriptor */
    MIDI_CS_MS_Bulk_Endpoint_Descriptor_t          MIDI_CS_Bulk_IN_Endpoint; // 5
}__attribute__((packed)) MIDI_Descriptor_t;

#endif
