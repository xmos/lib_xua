
S/PDIF Receive
==============

``lib_xua`` supports the development of devices with S/PDIF receive functionality through the use of 
``lib_spdif``. The XMOS S/PDIF receiver runs in a single core and supports rates up to 192kHz.

The S/PDIF receiver inputs data via a port and outputs samples via a channel. It requires a 1-bit port
which must be 4-bit buffered. For example::

    buffered in port:4 p_spdif_rx = PORT_SPDIF_IN;

It also requires a clock-block, for example::

    clock clk_spd_rx = XS1_CLKBLK_1;

Finally, a channel for the output samples must be declared, note, this should be a streaming channel::

    streaming chan c_spdif_rx;

The S/PDIF receiver should be called on the appropriate tile::

    spdif_rx(c_spdif_rx,p_spdif_rx,clk_spd_rx,192000);

.. note:: 

    It is recomended to use the value 192000 for the ``sample_freq_estimate`` parameter

With the steps above an S/PDIF stream can be captured by the xCORE. To be functionally useful the audio
master clock must be able to synchronise to this external digital stream. Additionally, the host can be 
notified regarding changes in the validity of this stream, it's frequency etc. To synchronise to external 
streams the codebase assumes the use of an external Cirrus Logic CS2100 device.

The ``ClockGen()`` task from ``lib_xua`` provides the reference signal to the CS2100 device and also handles
recording of clock validity etc. See :ref:`usb_audio_sec_clock_recovery` for full details regarding ``ClockGen()``.

It also provides a small FIFO for S/PDIF samples before they are forwarded to the ``AudioHub`` core.
As such it requires to be inserted in the communication path between the S/PDIF receiver and the 
``AudioHub`` core.  For example::

    chan c_dig_rx;
    streaming chan c_spdif_rx;

    par
    {
        SpdifReceive(..., c_spdif_rx, ...);    

        clockGen(c_spdif_rx, ..., c_dig_rx, ...);

        XUA_AudioHub(..., c_dig_rx, ...);
    }

