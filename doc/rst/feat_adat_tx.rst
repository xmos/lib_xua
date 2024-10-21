ADAT Transmit
=============

``lib_xua`` supports the development of devices with ADAT transmit functionality through the use of
``lib_adat``. The XMOS ADAT transmitter runs on a single core and supports transmitting 8 channels of
digital audio at 44.1 or 48 kHz. Higher rates are supported with a reduced number of samples via S/MUX (‘sample multiplexing’). Using S/MUX,
the ADAT transmitter can transmit four channels at 88.2 or 96 kHz (SMUX II) or two channels at 176.4 or 192 kHz (SMUX IV).

ADAT transmitter requires a logical core to run on. Blocks of audio samples are transmitted from the ``XUA_AudioHub()`` to the ADAT transmitter,
over either a dedicated channel or a combination of a channel and shared memory.

Each block of audio samples is made of 8 samples. At sampling rates 44.1/48 kHz (SMUX I), this consists of a single sample of each of the
eight ADAT channels:

  * Channel 0 sample
  * Channel 1 sample
  * Channel 2 sample
  * Channel 3 sample
  * Channel 4 sample
  * Channel 5 sample
  * Channel 6 sample
  * Channel 7 sample

At 88.2/96 kHz (SMUX II), the audio sample block consists of two samples per four ADAT channels:

  * Channel 0 sample 0
  * Channel 1 sample 1
  * Channel 1 sample 0
  * Channel 1 sample 1
  * Channel 2 sample 0
  * Channel 2 sample 1
  * Channel 3 sample 0
  * Channel 3 sample 1

At 176.4/192 kHz (SMUX IV), the audio sample block consists of four samples per two ADAT channels:

  * Channel 0 sample 0
  * Channel 0 sample 1
  * Channel 0 sample 2
  * Channel 0 sample 3
  * Channel 1 sample 0
  * Channel 1 sample 1
  * Channel 1 sample 2
  * Channel 1 sample 3

The configuration option ``ADAT_TX_USE_SHARED_BUFF`` determines whether the audio samples block is transmitted using only a channel
(``ADAT_TX_USE_SHARED_BUFF`` is not defined) or a channel + shared memory (``ADAT_TX_USE_SHARED_BUFF`` is defined).
When using a channel + shared memory for samples transfer, it is required that the
ADAT transmitter and the ``XUA_AudioHub()`` tasks run on the same tile.

The USB Audio reference applications with ADAT interface enabled in ``sw_usb_audio`` are only tested with ``ADAT_TX_USE_SHARED_BUFF`` defined.
The sample transfer sequence described in :ref:`xua_adat_tx` assumes that ``ADAT_TX_USE_SHARED_BUFF`` is defined.

Declarations
------------

The channel should be declared as normal::

    chan c_adat_out

In order to use the ADAT transmitter with ``lib_xua`` a 1-bit port must be declared e.g::

    on stdcore[AUDIO_IO_TILE] : buffered out port:32 p_adat_tx  = PORT_ADAT_OUT;

This port should be clocked from the master-clock::

    configure_out_port_no_ready(p_adat_tx, clk_audio_mclk, 0);
    set_clock_fall_delay(clk_audio_mclk, 7);


Finally the ADAT transmitter task is run - passing in the port and channel for communication with ``XUA_AudioHub``::

    adat_tx_port(c_adat_out, p_adat_tx);


.. _xua_adat_tx:

Communication between ``XUA_AudioHub`` and the ``adat_tx_port`` task
--------------------------------------------------------------------

To begin with, ``XUA_AudioHub`` sends two values on the channel - the master clock multiplier and
the S/MUX setting.

The master clock multiplier is the ratio between the mclk freqency and the sampling frequency. The S/MUX setting is
1, 2 or 4, depending on the sampling frequency:

.. literalinclude:: ../../src/core/audiohub/xua_audiohub.xc
   :start-at: /* Calculate what master clock we should be using */
   :end-before: /* Calculate master clock to bit clock

This is followed by communicating the address of a block of memory holding the audio samples block.
The ``XUA_AudioHub`` "runs ahead" of the ADAT transmitter task, assembling the next sample block while the
ADAT transmitter converts the current block into an ADAT stream to transmit over the optical interface.

The ADAT transmitter, once done processing the current block, acknowledges this by sending a data token over the channel
to ``XUA_AudioHub``. Only then, ``XUA_AudioHub`` sends the address of the next block of samples over the channel.

Note that a ``XS1_CT_END`` end token is not sent between blocks of data, leading to the channel remaining open and getting used
as a streaming channel.

``XUA_AudioHub`` only terminates the connection by sending a ``XS1_CT_END`` token when there's a sampling frequency change
that requires the ``XUA_AudioHub`` to re-communicate the master clock multiplier and
the S/MUX setting.

In case of a sampling frequency change, the ``XUA_AudioHub`` receives the pending handshake from the ADAT transmitter,
follwed by sending the ``XS1_CT_END`` token indicating the end of data streaming to the ADAT task.
Once this is done, a fresh transmission is started by sending the new master clock multiplier and
the S/MUX setting to the ADAT transmitter, follwed by audio blocks transfer as described above.

For further details please see the documentation, application notes and examples provided for ``lib_adat``.





