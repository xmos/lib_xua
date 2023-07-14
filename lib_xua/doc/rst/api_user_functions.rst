Required User Function Definitions
==================================

The following functions need to be defined by an application using `lib_xua`.

External Audio Hardware Configuration Functions
-----------------------------------------------

.. doxygenfunction:: AudioHwInit
.. doxygenfunction:: AudioHwConfig
.. doxygenfunction:: AudioHwConfig_Mute
.. doxygenfunction:: AudioHwConfig_UnMute


Audio Streaming Functions
-------------------------

The following functions can be optionally used by the design. They can be useful for mute lines etc.

.. c:function:: void AudioStreamStart(void)

  This function is called when the audio stream from device to host
  starts.

.. c:function:: void AudioStreamStop(void)

  This function is called when the audio stream from device to host stops.

Host Active
-----------

The following function can be used to signal that the device is connected to a valid host.

This is called on a change in state.

.. c:function:: void AudioStreamStart(int active)

   :param active: Indicates if the host is active or not. 1 for active else 0.


HID Controls
------------

The following function is called when the device wishes to read physical user input (buttons etc).

.. c:function:: void UserReadHIDButtons(unsigned char hidData[])

    :param hidData: The function should write relevant HID bits into this array. The bit ordering and functionality is defined by the HID report descriptor used.

