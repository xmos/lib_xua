
.. _sec_api_defines:

Configuration Defines
=====================

An application using the USB audio framework needs to have defines set for configuration.
Defaults for these defines are found in ``xua_conf_default.h``.

These defines should be over-ridden in an optional header file  ``xua_conf.h`` file or in the ``Makefile``
for a relevant build configuration. 

This section fully documents all of the settable defines and their default values (where appropriate).  

Code Location (tile)
--------------------

.. doxygendefine:: AUDIO_IO_TILE
.. doxygendefine:: XUD_TILE
.. doxygendefine:: MIDI_TILE
.. doxygendefine:: SPDIF_TX_TILE
.. doxygendefine:: PDM_TILE
.. doxygendefine:: PLL_REF_TILE

Channel Counts
--------------

.. doxygendefine:: NUM_USB_CHAN_OUT 
.. doxygendefine:: NUM_USB_CHAN_IN 
.. doxygendefine:: I2S_CHANS_DAC 
.. doxygendefine:: I2S_CHANS_ADC 

Frequencies and Clocks 
----------------------

.. doxygendefine:: MAX_FREQ
.. doxygendefine:: MIN_FREQ
.. doxygendefine:: DEFAULT_FREQ
.. doxygendefine:: MCLK_441
.. doxygendefine:: MCLK_48

Audio Class
-----------

.. doxygendefine:: AUDIO_CLASS
.. doxygendefine:: AUDIO_CLASS_FALLBACK
.. doxygendefine:: FULL_SPEED_AUDIO_2


Feature Configuration
---------------------

I2S/TDM
^^^^^^^

.. doxygendefine:: I2S_CHANS_DAC
.. doxygendefine:: I2S_CHANS_ADC
.. doxygendefine:: CODEC_MASTER
.. doxygendefine:: XUA_I2S_N_BITS
.. doxygendefine:: XUA_PCM_FORMAT

MIDI
^^^^

.. doxygendefine:: MIDI
.. doxygendefine:: MIDI_RX_PORT_WIDTH

S/PDIF
^^^^^^

.. doxygendefine:: XUA_SPDIF_TX_EN
.. doxygendefine:: SPDIF_TX_INDEX
.. doxygendefine:: XUA_SPDIF_RX_EN
.. doxygendefine:: SPDIF_RX_INDEX

ADAT
^^^^

.. doxygendefine:: XUA_ADAT_RX_EN
.. doxygendefine:: ADAT_RX_INDEX

PDM Microphones
^^^^^^^^^^^^^^^

.. doxygendefine:: XUA_NUM_PDM_MICS

DFU
^^^

.. doxygendefine:: XUA_DFU_EN

.. .. doxygendefine:: DFU_FLASH_DEVICE

HID
^^^

.. doxygendefine:: HID_CONTROLS


CODEC Interface
^^^^^^^^^^^^^^^

.. doxygendefine:: CODEC_MASTER


USB Device Configuration
------------------------

.. doxygendefine:: VENDOR_STR
.. doxygendefine:: VENDOR_ID
.. doxygendefine:: PRODUCT_STR
.. doxygendefine:: PRODUCT_STR_A2
.. doxygendefine:: PRODUCT_STR_A1
.. doxygendefine:: PID_AUDIO_1
.. doxygendefine:: PID_AUDIO_2
.. doxygendefine:: BCD_DEVICE


Stream Formats
--------------

Output/Playback
^^^^^^^^^^^^^^^

.. doxygendefine:: OUTPUT_FORMAT_COUNT

.. doxygendefine:: STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS
.. doxygendefine:: STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS
.. doxygendefine:: STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS

.. doxygendefine:: HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES
.. doxygendefine:: HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES
.. doxygendefine:: HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES

.. doxygendefine:: FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES
.. doxygendefine:: FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES
.. doxygendefine:: FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES

.. doxygendefine:: STREAM_FORMAT_OUTPUT_1_DATAFORMAT
.. doxygendefine:: STREAM_FORMAT_OUTPUT_2_DATAFORMAT
.. doxygendefine:: STREAM_FORMAT_OUTPUT_3_DATAFORMAT

Input/Recording
^^^^^^^^^^^^^^^

.. doxygendefine:: INPUT_FORMAT_COUNT

.. doxygendefine:: STREAM_FORMAT_INPUT_1_RESOLUTION_BITS

.. doxygendefine:: HS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES

.. doxygendefine:: FS_STREAM_FORMAT_INPUT_1_SUBSLOT_BYTES

.. doxygendefine:: STREAM_FORMAT_INPUT_1_DATAFORMAT

Volume Control
--------------

.. doxygendefine:: OUTPUT_VOLUME_CONTROL
.. doxygendefine:: INPUT_VOLUME_CONTROL
.. doxygendefine:: MIN_VOLUME
.. doxygendefine:: MAX_VOLUME
.. doxygendefine:: VOLUME_RES

Mixing
------

.. doxygendefine:: MIXER
.. doxygendefine:: MAX_MIX_COUNT
.. doxygendefine:: MIX_INPUTS
.. doxygendefine:: MIN_MIXER_VOLUME
.. doxygendefine:: MAX_MIXER_VOLUME
.. doxygendefine:: VOLUME_RES_MIXER

Power
-----

.. doxygendefine:: XUA_POWERMODE

