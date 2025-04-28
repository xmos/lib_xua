
|newpage|

XMOS USB Device (XUD) library
=============================

All low level communication with the USB host is handled by the XMOS USB Device (XUD) library - `lib_xud <https://www.xmos.com/file/lib_xud>`_

The ``XUD_Main()`` function runs in its own thread and communicates with endpoint threads though a
mixture of shared memory and channel communications.

For more details and full XUD API documentation please refer to `lib_xud <https://www.xmos.com/file/lib_xud>`__.

:numref:`usb_audio_threads` shows the XUD library communicating with two other threads:

-  Endpoint 0: This thread controls the enumeration/configuration tasks of the USB device.

-  Endpoint Buffer: This thread sends/receives data packets from the XUD library.
   The thread receives audio data from the AudioHub, MIDI data from the MIDI thread etc.

