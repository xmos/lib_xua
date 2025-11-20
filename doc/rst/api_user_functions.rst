|newpage|

User function definitions
=========================

The following functions can be defined by an application using ``lib_xua``.

.. note:: Default, empty, implementations of these functions are provided in ``lib_xua``.
   These are marked as weak symbols so the application can simply define its own version of them.

External audio hardware configuration functions
-----------------------------------------------

The following functions can be optionally used by the design to configure external audio hardware.
As a minimum, in most applications, it is expected that a implementation of `AudioHwConfig()` will need
to be provided.

.. doxygenfunction:: AudioHwInit
.. doxygenfunction:: AudioHwConfig
.. doxygenfunction:: AudioHwConfig_Mute
.. doxygenfunction:: AudioHwConfig_UnMute
.. doxygenfunction:: AudioHwShutdown

Audio stream start/stop functions
---------------------------------

The following functions can be optionally used by the design. They can be useful for mute lines etc.

.. doxygenfunction:: UserAudioStreamState

Host active functions
---------------------

The following function can be used to signal that the device is connected to a valid host.

.. doxygenfunction:: UserHostActive

HID controls
------------

The following function is called when the device wishes to read physical user input (buttons etc).
The function should write relevant HID bits into this array. The bit ordering and functionality is defined by the HID report descriptor used.

.. doxygenfunction:: UserHIDGetData


PDM Mics user callback functions
--------------------------------

Two weak callback APIs are provided which optionally allow user code to be executed at startup (before PDM microphone initialisation)
and after each PCM sample is received.
These can be useful for custom hardware initialisation required by the PDM microphone or post processing such as gain control
before samples are forwarded to XUA

.. doxygenfunction:: xua_user_pdm_init

.. doxygenfunction:: xua_user_pdm_process
