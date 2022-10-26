
S/PDIF Transmit
===============

``lib_xua`` supports the development of devices with S/PDIF transmit functionality through the use of 
``lib_spdif``. The XMOS S/PDIF transmitter runs in a single core and supports rates up to 192kHz.

The S/PDIF transmitter core takes PCM audio samples via a channel and outputs them in S/PDIF format to a port.
Samples are provided to the S/PDIF transmitter task from the ``XUA_AudioHub()`` task.

The channel should be declared as normal::

    chan c_spdif_tx


In order to use the S/PDIF transmitter with ``lib_xua`` a 1-bit port must be declared e.g::

    buffered out port:32 p_spdif_tx = PORT_SPDIF_OUT;     /* SPDIF transmit port */

This port should be clocked from the master-clock, ``lib_spdif`` provides a helper function for setting up the port::

    spdif_tx_port_config(p_spdif_tx, clk_audio_mclk, p_mclk_in, delay);

.. note:: If sharing the master-clock port and clockblock with ``XUA_AudioHub()`` (or any other task) then this setup
          should be done before running the tasks in a ``par`` statement.

Finally the S/PDIF transmitter task must be run - passing in the port and channel for communication with ``XUA_AudioHub``.
For example::

    par
    {
        while(1)
        {
            /* Run the S/PDIF transmitter task */
            spdif_tx(p_spdif_tx, c_spdif_tx);   
        }
    
        /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as 
         * a hub for all audio).
         * Note, since we are not using I2S we pass in null for LR and Bit 
         * clock ports and the I2S dataline ports */
        XUA_AudioHub(c_aud, clk_audio_mclk, null, p_mclk_in, null, null, 
            null, null, c_spdif_tx);
    }

For further details please see the documentation, application notes and examples provided for ``lib_spdif``.

