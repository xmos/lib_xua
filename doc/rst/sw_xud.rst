
|newpage|

XMOS USB Device (XUD) Library
=============================

All low level communication with the USB host is handled by the XMOS USB Device (XUD) library - `lib_xud`

The ``XUD_Main()`` function runs in its own core and communicates with endpoint cores though a 
mixture of shared memory and channel communications.

For more details and full XUD API documentation please refer to `lib_xud`.

:ref:`usb_audio_threads` shows the XUD library communicating with two other cores:

-  Endpoint 0: This core controls the enumeration/configuration tasks of the USB device.

-  Endpoint Buffer: This core sends/receives data packets from the XUD library.  
   The core receives audio data from the AudioHub, MIDI data from the MIDI core etc.

