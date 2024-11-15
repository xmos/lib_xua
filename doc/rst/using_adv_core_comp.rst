|newpage|

Running the core components
===========================

In their most basic form the core components can be run as follows::

    par
    {
        /* Endpoint 0 thread from lib_xua */
        XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, ...);

        /* Buffering threads - handles audio data to/from EP's and gives/gets data to/from the audio I/O thread */
        /* Note, this spawns two threads */
        XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

        /* AudioHub/IO thread does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
        XUA_AudioHub(c_aud, ...) ;
    }

``XUA_Buffer()`` expects the ``p_for_mclk_count`` argument to be clocked from the audio master clock
before receiving it as a parameter. The following code satisfies this requirement::

    {
        /* Connect master-clock clock-block to clock-block pin */

        /* Clock clock-block from mclk pin */
        set_clock_src(clk_audio_mclk_usb, p_mclk_in_usb);

        /* Clock the "count" port from the clock block */
        set_port_clock(p_for_mclk_count, clk_audio_mclk_usb);

        /* Set the clock off running */
        start_clock(clk_audio_mclk_usb);

        XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);
    }

.. note:: Keeping this configuration outside of ``XUA_Buffer()`` means the possibility of sharing the
   ``p_mclk_in_usb`` port with additional tasks is not precluded.

For USB connectivity a call to ``XUD_Main()`` (from `lib_xud <www.xmos.com/file/lib_xud>`_) must
also be made::

    /* Low level USB device layer thread */
    on tile[1]: XUD_Main(c_ep_out, 2, c_ep_in, 2, c_sof, epTypeTableOut, epTypeTableIn, null, null, -1, XUD_SPEED_HS, XUD_PWR_SELF);

Additionally, the required communication channels must also be declared::

    /* Channel arrays for lib_xud */
    chan c_ep_out[2];
    chan c_ep_in[2];

    /* Channel for communicating SOF notifications from XUD to the Buffering threads */
    chan c_sof;

    /* Channel for audio data between buffering threads and AudioHub/IO thread */
    chan c_aud;

    /* Channel for communicating control messages from EP0 to the rest of the device (via the buffering threads) */
    chan c_aud_ctl;

This section provides enough information to implement a skeleton program for a USB Audio device.
When running the `xcore` device will present itself as a USB Audio Class device on the bus.
Audio streaming will be impaired since tasks relating to physical audio interfaces are yet to be
instantiated.

