
.. _usb_audio_sec_architecture:

Software Architecture
*********************

This section describes the required software architecture of a USB Audio device implemented using `lib_xua`, its dependencies and other supporting libraries.

`lib_xua` provides fundamental building blocks for producing USB Audio products on XMOS devices. Every system is required to have the components from `lib_xua` listed in :ref:`usb_audio_shared_components`.

.. tabularcolumns:: lp{5cm}l
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
    
In addition low-level USB I/0 is required and is provided by the external dependency `lib_xud`

.. tabularcolumns:: lp{5cm}l
.. list-table:: Additional Components Required
 :header-rows: 1
 :widths: 100 60

 * - Component
   - Description
 * - XMOS USB Device Driver (XUD)
   - Handles the low level USB I/O.

In addition :ref:`usb_audio_optional_components` shows optional components that can be added/enabled from within `lib_xua`

.. tabularcolumns:: lp{5cm}l
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
     from digital input.
 * - MIDI
   - Outputs and inputs MIDI over a serial UART interface.

`lib_xua` also provides optional support for integrating with the following external dependencies listed in :ref:`usb_audio_external_components`

.. tabularcolumns:: lp{5cm}l
.. _usb_audio_external_components:
.. list-table:: External Components
 :header-rows: 1
 :widths: 40 60

 * - Component
   - Description
 * - S/PDIF Transmitter (lib_spdif)
   - Outputs samples of an S/PDIF digital audio interface.
 * - S/PDIF Receiver (lib_spdif)
   - Inputs samples of an S/PDIF digital audio interface (requires the
     clockgen component).
 * - ADAT Receiver (lib_adat)
   - Inputs samples of an ADAT digital audio interface (requires the
     clockgen component).
 * - PDM Microphones (lib_mic_array)
   - Receives PDM data from microphones and performs PDM to PCM conversion

.. _usb_audio_threads:

.. figure:: images/threads-crop.*
      :width: 100%
 
      USB Audio Core Diagram

:ref:`usb_audio_threads` shows how the components interact with each
other in a typical system.  The green circles represent cores with arrows indicating inter-core communications.


