Using lib_xud
-------------

This sections describes the basic usage of `lib_xud`. It provides a guide on how to program the USB Audio Devices using `lib_xud` including instructions for building and running
programs and creating your own custom USB audio applications.

Reviewing application note AN00246 is highly recommended.

Library structure
~~~~~~~~~~~~~~~~~

The code is split into several directories.

.. list-table:: lib_xua structure

 * - core
   - Common code for USB audio applications
 * - midi
   - MIDI I/O code
 * - dfu
   - Device Firmware Upgrade code


Note, the midi and dfu directories are potential candidates for separate libs in their own right.


Including in a project
~~~~~~~~~~~~~~~~~~~~~~

All `lib_xua` functions can be accessed via the ``xud.h`` header filer::

  #include <xua.h>

It is also requited to to add ``lib_xua`` to the ``USED_MODULES`` field of your application Makefile.


Core hardware resources
~~~~~~~~~~~~~~~~~~~~~~~

Currently all hardware resources used by `lib_xua` are simply declared globally.

As an absolute minimum the following resources are required

- A 1-bit port for audio master clock input  
- A n-bit port for internal feedback calculation (typically a free, unused port is used e.g. `16B`)
- A clock-block, which will be clocked from the master clock input port

.. note:: 
    
    Since these resources are accessed globally naming is of importance

Example declaration of these resources might look as follows::

    in port p_mclk_in                   = PORT_MCLK_IN;
    in port p_for_mclk_count            = PORT_MCLK_COUNT;   /* Extra port for counting master clock ticks */
    clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_5;   /* Master clock */


If the ``XUD_AudioHub()`` and ``XUD_Buffer()`` cores reside on separate tiles a separate master clock input port must be provided to each, for example::

    /* Master clock for the audio IO tile */
    in port p_mclk_in                   = PORT_MCLK_IN;

    /* Resources for USB feedback */
    in port p_mclk_in_usb               = PORT_MCLK_IN_USB;  /* Extra master clock input for the USB tile */

Whilst this satisfies the basic requirements for the operation of  `lib_xua` projects typically also needs some additional audio I/O, I2S or SPDIF for example. 
These should be passed into the various cores as required (see API section)

Running the core components
~~~~~~~~~~~~~~~~~~~~~~~~~~~

In their most basic form the core components can be run as follows::

    par
    {
        /* Endpoint 0 core from lib_xua */
        XUA_Endpoint0(c_ep_out[0], c_ep_in[0], c_aud_ctl, null, null, null, null);

        /* Buffering cores - handles audio data to/from EP's and gives/gets data to/from the audio I/O core */
        /* Note, this spawns two cores */
        XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

        /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
        XUA_AudioHub(c_aud);
    }

``XUA_Buffer()`` expects its ``p_for_mclk_count`` argument to be clocked from the audio master clock
The following code satisfies this requirement::

    {
            /* Connect master-clock clock-block to clock-block pin */
            set_clock_src(clk_audio_mclk_usb, p_mclk_in_usb);           /* Clock clock-block from mclk pin */
            set_port_clock(p_for_mclk_count, clk_audio_mclk_usb);       /* Clock the "count" port from the clock block */
            start_clock(clk_audio_mclk_usb);                            /* Set the clock off running */

            XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

    }

.. note:: By keeping this configuraiton outside of the ``XUA_Buffer()`` function allow the possiblity to share the ``p_mclk_in_usb`` port with additional components

To produce a fully operating device a call to ``XUD_Main()`` (from ``lib_xud``) must also be made for USB connectivity::

    /* Low level USB device layer core */ 
    on tile[1]: XUD_Main(c_ep_out, 2, c_ep_in, 2, c_sof, epTypeTableOut, epTypeTableIn, null, null, -1, XUD_SPEED_HS, XUD_PWR_SELF);

Additionally the required communication channels must also be declared::

    /* Channel arrays for lib_xud */
    chan c_ep_out[2];
    chan c_ep_in[2];

    /* Channel for communicating SOF notifications from XUD to the Buffering cores */
    chan c_sof;

    /* Channel for audio data between buffering cores and AudioHub/IO core */
    chan c_aud;
    
    /* Channel for communicating control messages from EP0 to the rest of the device (via the buffering cores) */
    chan c_aud_ctl;



Configuring XUA
~~~~~~~~~~~~~~~

Built in main()
~~~~~~~~~~~~~~~

Enabling Additional Features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This sections describes only the basic feature set of ``lib_xua`` details on enabling additional features e.g. S/PDIF can be found later in this document.
