|newpage|

.. _sec_opt_audio_formats:

Audio Stream Formats
====================

The design currently supports up to three different stream formats for playback, selectable at
run time.  This is implemented using standard Alternative Settings to the Audio Streaming interfaces. 

An Audio Streaming interface can have Alternate Settings that can be used to change certain characteristics
of the interface and underlying endpoint. A typical use of Alternate Settings is to provide a way to 
change the subframe size and/or number of channels on an active Audio Streaming interface. 
Whenever an Audio Streaming interface requires an isochronous data endpoint, it must at least provide
the default Alternate Setting (Alternate Setting 0) with zero bandwidth requirements (no isochronous
data endpoint defined) and an additional Alternate Setting that contains the actual isochronous
data endpoint.  This zero bandwidth alternative setting 0 is always implemented by the design.

For further information refer to 3.16.2 of `USB Audio Device Class Definition for Audio Devices <http://www.usb.org/developers/devclass_docs/Audio2.0_final.zip>`_

Customisable parameters for the Alternate Settings provided by the design are as follows.:

    * Audio sample resolution
    * Audio sample subslot size
    * Audio data format

.. note::

    Currently only a single format is supported for the recording stream

By default the design exposes two sets of Alternative Settings for the playback Audio Streaming interface, one for 16-bit and another for
24-bit playback. When DSD is enabled an additional (32-bit) alternative is exposed.
    
Audio Subslot
-------------

An audio subslot holds a single audio sample. See `USB Device Class Definition for Audio Data Formats 
<http://www.usb.org/developers/devclass_docs/Audio2.0_final.zip>`_ for full details. 
This is represented by `bSubslotSize` in the devices descriptor set.

An audio subslot always contains an integer number of bytes. The specification limits the possible
audio subslot size to 1, 2, 3 or 4 bytes per audio subslot.

Since the xCORE is a 32-bit machine the value 4 is typically used for `bSubSlot` - this means that
packing/unpacking samples to/from packets is trivial.  Other values can, however, be used and the design
supports values 4, 3 and 2.

Values other than 4 may be used for the following reasons:

    * Bus-bandwidth needs to be efficiently utilised. For example maximising channel-count/sample-rates in 
      full-speed operation.

    * To support restrictions with certain hosts. For example many Android based hosts support only 16bit
      samples in a 2-byte subslot. 

`bSubSlot` size is set using the following defines:

    * When running in high-speed: 

      * `HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES`
      
      * `HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES`
      
      * `HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES` 
    
    * When running in full-speed: 
      
      * `FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES`
      
      * `FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES`
      
      * `FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES` 


Audio Sample Resolution
-----------------------

An audio sample is represented using a number of bits (`bBitResolution`) less than or equal to the number
of total bits available in the audio subslot i.e. `bBitResolution` <= `bSubslotSize` * 8).  The design 
supports values 16, 24 and 32.

`bBitResolution` is set using the following defines:

    * When operating at high-speed: 

      * `HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS`

      * `HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS`

      * `HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS`
    
    * When operating at full-speed: 
      
      * `FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS`

      * `FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS`
        
      * `FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS`


Audio Format
------------

The design supports two audio formats, PCM and, when "Native" DSD is enabled, Direct Stream Digital (DSD).
A DSD capable DAC is required for the latter.

The USB Audio `Raw Data` format is used to indicate DSD data (2.3.1.7.5 of `USB Device Class
Definition for Audio Data Formats <http://www.usb.org/developers/devclass_docs/Audio2.0_final.zip>`_).
This use of a RAW/DSD format in an alternative setting is termed by XMOS as  *Native DSD*

The following defines affect both full-speed and high-speed operation:

    * STREAM_FORMAT_OUTPUT_1_DATAFORMAT               
    
    * STREAM_FORMAT_OUTPUT_2_DATAFORMAT               
    
    * STREAM_FORMAT_OUTPUT_3_DATAFORMAT               

The following options are supported:
    
    * UAC_FORMAT_TYPEI_RAW_DATA
    
    * UAC_FORMAT_TYPEI_PCM

.. note::

    Currently DSD is only supported on the output/playback stream

.. note::

    4 byte slot size with a 32 bit resolution is required for RAW/DSD format

Native DSD requires driver support and is available in the Thesycon Windows driver via ASIO.


