// Copyright (c) 2013-2018, XMOS Ltd, All rights reserved


/* These defines relate to the HID report desc - do not mod */
#define HID_CONTROL_PLAYPAUSE_SHIFT  0x00
#define HID_CONTROL_NEXT_SHIFT       0x01
#define HID_CONTROL_PREV_SHIFT       0x02
#define HID_CONTROL_VOLUP_SHIFT      0x03
#define HID_CONTROL_VOLDN_SHIFT      0x04
#define HID_CONTROL_MUTE_SHIFT       0x05

#define HID_DATA_SIZE 1

#ifndef HID_SIMULATE_INTERRUPTS
#define HID_SIMULATE_INTERRUPTS 0
#endif

#ifndef INTERRUPT_ASSERT_LEVEL
#define INTERRUPT_ASSERT_LEVEL 0
#endif

#if( 0 < HID_CONTROLS )

#if( 0 < NDP_ASSERT_LEVEL )
#define INT_ASSERT_LEVEL   1
#define INT_DEASSERT_LEVEL 0
#else
#define INT_ASSERT_LEVEL   0
#define INT_DEASSERT_LEVEL 1
#endif

#if( 0 < HID_SIMULATE_INTERRUPTS )
#define HID_DEASSERT_COUNT       10000
#define HID_INTERRUPT_COUNT 1000000000
#endif

void UserInitHIDData(void);
void UserReadHIDData(unsigned char hidData[ HID_DATA_SIZE ]);
void UserSetHIDData(const unsigned hidData);

#endif /* ( 0 < HID_CONTROLS ) */
