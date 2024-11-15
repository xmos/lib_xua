S/PDIF receive
==============

`xcore` devices can support S/PDIF receive up to 192 kHz - see `lib_spdif <https://www.xmos.com/file/lib_spdif>`__
for full specifications.

The S/PDIF receiver module uses a clock-block and a buffered one-bit port.
The clock-block is divided off a 100 MHz reference clock. The one bit port is buffered to 32 bits.
The receiver code uses this clock to over sample the input data.

The receiver outputs audio samples over a *streaming channel end* where data can be input using the
built-in input operator. ``lib_spdif`` also provides API functions that wrap up this communication.

The S/PDIF receive function never returns. The 32-bit value from the channel input comprises of
fields shown in :numref:`spdif_rx_word_structure`.

.. _spdif_rx_word_structure:

.. list-table:: S/PDIF receive word structure
     :header-rows: 1
     :widths: 10 32

     * - Bits
       -
     * - 0:3
       - A tag (see below)
     * - 4:28
       - PCM encoded sample value
     * - 29:31
       - User bits (parity, etc)

The tag has one of three values, as shown in :numref:`spdif_rx_tag`.

.. _spdif_rx_tag:

.. list-table:: S/PDIF receive tags
     :header-rows: 1
     :widths: 10 32

     * - Tag
       - Meaning
     * - FRAME\_X
       - Sample on channel 0 (Left for stereo)
     * - FRAME\_Y
       - Sample on another channel (Right if for stereo)
     * - FRAME\_Z
       - Sample on channel 0 (Left), and the first sample of a frame; can be used if the user bits need to be reconstructed.

See S/PDIF, IEC 60958-3:2006, specification for further details on format, user bits etc.

Usage and integration
---------------------

Since S/PDIF is a digital stream, the device's master clock must be synchronised to it. This is typically
done with an external device or the `xcore.ai` secondary PLL.
See :ref:`usb_audio_sec_clock_recovery`.

.. note::

   Due to the requirement for this clock recovery S/PDIF receive can only be used in Asynchronous
   mode.

The S/PDIF receive function communicates with the Clock Gen thread, which in turn passes audio data to the
Audio Hub thread. The Clock Gen thread also handles locking to the S/PDIF clock source. Again, see
:ref:`usb_audio_sec_clock_recovery`.

The parity of each word/sample received is checked.  This is done using the ``spdif_rx_check_parity()``
function provided by ``lib_spdif``:

.. literalinclude:: ../../lib_xua/src/core/clocking/clockgen.xc
  :start-at: /* Check parity and ignore if bad */
  :end-at: continue

If bad parity is detected the word/sample is ignored, otherwise the tag is inspected for channel
(i.e. left or right) and the sample stored.

The following code snippet illustrates how the output of the S/PDIF receive component is
fundamentally used.
Note the use of helper defines/macros for frame identification and sample data extraction, provided
by ``lib_spdif``::

  while(1)
  {
     c_spdif_rx :> data;

     if(spdif_check_parity(data)
       continue;

     tag = data & SPDIF_RX_PREAMBLE_MASK;

     /* Extract 24bit audio sample */
     sample = SPDIF_RX_EXTRACT_SAMPLE(data);

     switch(tag)
     {
       case SPDIF_FRAME_X:
       case SPDIF_FRAME_X:
         // Store left sample
         break;

       case SPDIF)FRAME_Z:
         // Store right sample
         break;
     }
  }

The Clock Gen thread stores samples in a small FIFO before they are communicated to the Audio Hub thread.

