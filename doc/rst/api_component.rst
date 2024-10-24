.. _sec_api_component:

Component API
=============

The following functions can be called from the top level main of an
application and implement the various components described in
:ref:`usb_audio_sec_architecture`.

When using the USB audio framework the ``c_ep_in`` array is always
composed in the following order:
   
  * Endpoint 0 (in)
  * Audio Feedback endpoint (if output enabled)
  * Audio IN endpoint (if input enabled)
  * MIDI IN endpoint (if MIDI enabled)
  * Clock Interrupt endpoint

The array ``c_ep_out`` is always composed in the following order:
   
  * Endpoint 0 (out)
  * Audio OUT endpoint (if output enabled)
  * MIDI OUT endpoint (if MIDI enabled)

.. doxygenfunction:: XUA_Endpoint0

.. doxygenfunction:: XUA_Buffer

.. doxygenfunction:: XUA_AudioHub

.. doxygenfunction:: mixer

.. doxygenfunction:: clockGen

.. doxygenfunction:: usb_midi
