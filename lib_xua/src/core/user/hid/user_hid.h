// Copyright (c) 2013-2019, XMOS Ltd, All rights reserved

/* These defines relate to the HID report desc - do not mod */
#define HID_CONTROL_PLAYPAUSE_SHIFT  0x00
#define HID_CONTROL_NEXT_SHIFT       0x01
#define HID_CONTROL_PREV_SHIFT       0x02
#define HID_CONTROL_VOLUP_SHIFT      0x03
#define HID_CONTROL_VOLDN_SHIFT      0x04
#define HID_CONTROL_MUTE_SHIFT       0x05

#define HID_DATA_SIZE 1

#if( 0 < HID_CONTROLS )

void UserInitHIDData( void );
void UserReadHIDData( unsigned char hidData[ HID_DATA_SIZE ]);
void UserSetHIDData( const unsigned hidData );

#endif /* ( 0 < HID_CONTROLS ) */
