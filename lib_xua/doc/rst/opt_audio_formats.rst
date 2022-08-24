
.. _sec_opt_audio_formats:

Audio Stream Formats
~~~~~~~~~~~~~~~~~~~~

The design currently supports up to 3 different stream formats for output/playback, selectable at
run time.  This is implemented using Alternative Settings to the AudioStreaming interfaces. 

An AudioStreaming interface can have Alternate Settings that can be used to change certain characteristics
of the interface and underlying endpoint. A typical use of Alternate Settings is to provide a way to 
change the subframe size and/or number of channels on an active AudioStreaming interface. 
Whenever an AudioStreaming interface requires an isochronous data endpoint, it must at least provide
the default Alternate Setting (Alternate Setting 0) with zero bandwidth requirements (no isochronous
data endpoint defined) and an additional Alternate Setting that contains the actual isochronous
data endpoint.

For further information refer to 3.16.2 of `USB Audio Device Class Definition for Audio Devices <http://www.usb.org/developers/devclass_docs/Audio2.0_final.zip>`_

Note, a 0-bandwidth alternative setting 0 is always implemented by the design (as required by the USB
specifications).

Customisatble parameters for the Alternate Settings are as follows.:

    * Audio sample resolution
    * Audio sample subslot size
    * Audio data format

.. note::

    Currently only a single format is supported for the input/recording stream
    
Audio Subslot
.............

An audio subslot holds a single audio sample. See `USB Device Class Definition for Audio Data Formats 
<http://www.usb.org/developers/devclass_docs/Audio2.0_final.zip>`_ for full details. 
This is represented by `bSubslotSize` in the devices descriptors

An audio subslot always contains an integer number of bytes. The specification limits the possible
audio sublot size (`bSubslotSize`) to 1, 2, 3 or 4 bytes per audio subslot.

Typically, since it is run on a 32-bit machine, the value 4 is used for subslot - this means that
packing/unpacking samples is trivial.  Other values can be used (currently 4, 3 and 2 are supported
by the design). 

Other values may be used for the the following reasons:

    * Bus-bandwidth needs to be efficiently utilised. For example maximising channel-count/sample-rates in 
      full-speed operation.

    * To support restrictions with certain hosts. For example many Android based hosts support only 16bit
      samples in a 2-byte subslot. 

bSubSlot size is set using the following defines:

    * When running in high-speed: 

      * `HS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES`
      
      * `HS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES`
      
      * `HS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES` 
    
    * When running in full-speed: 
      
      * `FS_STREAM_FORMAT_OUTPUT_1_SUBSLOT_BYTES`
      
      * `FS_STREAM_FORMAT_OUTPUT_2_SUBSLOT_BYTES`
      
      * `FS_STREAM_FORMAT_OUTPUT_3_SUBSLOT_BYTES` 


Audio Sample Resolution
.......................

An audio sample is represented using a number of bits (`bBitResolution`) less than or equal to the number
of total bits available in the audio subslot i.e. `bBitResolution` <= `bSubslotSize` * 8.  Supported values
are 16, 24 and 32.

The following defines 

    * The following defines affect high-speed operation: 

      * `HS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS`

      * `HS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS`

      * `HS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS`
    
    * The following defines affect full-speed operation: 
      
      * `FS_STREAM_FORMAT_OUTPUT_1_RESOLUTION_BITS`

      * `FS_STREAM_FORMAT_OUTPUT_2_RESOLUTION_BITS`
        
      * `FS_STREAM_FORMAT_OUTPUT_3_RESOLUTION_BITS`


Audio Format
............

The design supports two audio formats, PCM and Direct Stream Digital (DSD).
A DSD capable DAC is required for the latter.

The USB Audio Raw Data format is used to indicate DSD data (2.3.1.7.5 of `USB Device Class
Definition for Audio Data Formats <http://www.usb.org/developers/devclass_docs/Audio2.0_final.zip>`_).
This use of a RAW/DSD format in an alternative setting is termed *Native DSD*

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


