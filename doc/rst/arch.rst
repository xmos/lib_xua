
|newpage|

.. _usb_audio_sec_architecture:

*********************
Software architecture
*********************

This section describes the required software architecture of a USB Audio device implemented using `lib_xua`, its dependencies and other supporting libraries.

`lib_xua` provides fundamental building blocks for producing USB Audio products on `XMOS` devices. Every system is required to have the components from `lib_xua` listed in :numref:`usb_audio_shared_components`.

.. _usb_audio_shared_components:

.. list-table:: Required XUA Components
 :header-rows: 1
 :widths: 40 60

 * - Component
   - Description
 * - Endpoint 0
   - Provides the logic for Endpoint 0 which handles
     enumeration and control of the device including DFU related requests.
 * - Endpoint buffer
   - Buffers endpoint data packets to and from the host. Manages delivery of audio packets between the endpoint buffer
     component and the audio components. It can also handle volume control processing. Note, this currently utilises two cores
 * - AudioHub
   - Handles audio I/O over I2S and manages audio data
     to/from other digital audio I/O components.


In addition low-level USB I/0 is required and is provided by the external dependency `lib_xud <www.xmos.com/file/lib_xud>`_.

.. list-table:: Required external components
 :header-rows: 1
 :widths: 100 60

 * - Component
   - Description
 * - XMOS USB Device Driver (XUD)
   - Handles the low level USB I/O.

In addition :numref:`usb_audio_optional_components` shows optional components that can be added/enabled from within `lib_xua`

.. _usb_audio_optional_components:
.. list-table:: Optional Components
 :header-rows: 1
 :widths: 40 60

 * - Component
   - Description
 * - Mixer
   - Allows digital mixing of input and output channels.  It can also
     handle volume control instead of the decoupler.
 * - Clockgen
   - Drives an external frequency generator (PLL) and manages
     changes between internal clocks and external clocks arising
     from digital input. On xcore.ai Clockgen may also work in
     conjunction with lib_sw_pll to produce a local clock from
     the XCORE which is locked to the incoming digital stream.
 * - MIDI
   - Outputs and inputs MIDI over a serial UART interface.

`lib_xua` also provides optional support for integrating with the following external dependencies listed in :numref:`usb_audio_external_components`

.. _usb_audio_external_components:
.. list-table:: Optional external components
 :header-rows: 1
 :widths: 40 60

 * - Component
   - Description
 * - S/PDIF Transmitter (`lib_spdif <www.xmos.com/file/lib_spdif>`_)
   - Outputs samples on an S/PDIF digital audio interface.
 * - S/PDIF Receiver (`lib_spdif <www.xmos.com/file/lib_spdif>`)
   - Inputs samples off an S/PDIF digital audio interface (requires the
     clockgen component).
 * - ADAT Transmitter (`lib_adat <www.xmos.com/file/lib_adat>`_)
   - Outputs samples on an ADAT digital audio interface.
 * - ADAT Receiver (`lib_adat <www.xmos.com/file/lib_adat>`_)
   - Inputs samples off an ADAT digital audio interface (requires the
     clockgen component).
 * - PDM Microphones (`lib_mic_array <www.xmos.com/file/lib_mic_array>`_)
   - Receives PDM data from microphones and performs PDM to PCM conversion

.. _usb_audio_threads:

.. figure:: images/threads-crop.*
      :width: 100%

      USB Audio thread diagram

:numref:`usb_audio_threads` shows how the components interact with each
other in a typical system.
The green circles represent threads with arrows indicating inter-thread communications.


