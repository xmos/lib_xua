Core hardware resources
=======================

The user must declare and initialise relevant hardware resources (globally) and pass them to the
relevant function of ``lib_xua``.

As an absolute minimum the following resources are required:

- A 1-bit port for audio master clock input
- A clock-block, which will be clocked from the master clock input port

When using the default asynchronous mode of operation an additional port is required:

- A n-bit port for internal feedback calculation (typically a free, unused port is used e.g. ``XS1_PORT_16B``)

Example declaration of these resources might look as follows::

    in port p_mclk_in        = PORT_MCLK_IN;
    in port p_for_mclk_count = PORT_MCLK_COUNT;   /* Extra port for counting master clock ticks */
    clock clk_audio_mclk     = on tile[0]: XS1_CLKBLK_5;   /* Master clock */

.. note::

    The ``PORT_MCLK_IN`` and ``PORT_MCLK_COUNT`` definitions are derived from the projects XN file

The ``XUA_AudioHub()`` function typically requires an audio master clock input to clock the physical
audio I/O such as S/PDIF transmit and I²S [#]_ .
Less obvious is the reasoning for the ``XUA_Buffer()`` task having the same requirement when running in
asynchronous mode - it is used for the USB feedback system and packet sizing.

Due to the above, if the ``XUD_AudioHub()`` and ``XUA_Buffer()`` cores must reside on separate
tiles a separate master clock input port must be provided to each, for example::

    /* Master clock for the audio IO tile */
    in port p_mclk_in                   = PORT_MCLK_IN;

    /* Resources for USB feedback */
    in port p_mclk_in_usb               = PORT_MCLK_IN_USB;  /* Extra master clock input for the USB tile */

Whilst the hardware resources described in this section satisfy the basic requirements for the operation (or build)
of ``lib_xua``, projects typically also need some additional audio I/O, I²S or S/PDIF for example.

These should be passed into the various task functions as required - see :ref:`sec_api`.

.. [#] This is not the case when *only* I²S *slave* is used.
