Using lib_xud
-------------

This sections describes the basic usage of `lib_xud`. It provides a guide on how to program the USB Audio Devices using `lib_xud`.

Reviewing application note AN00246 is highly recommended at this point.

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

All `lib_xua` functions can be accessed via the ``xua.h`` header filer::

  #include <xua.h>

It is also required to add ``lib_xua`` to the ``USED_MODULES`` field of your application Makefile::

  USED_MODULES = .. lib_xua ...


Core hardware resources
~~~~~~~~~~~~~~~~~~~~~~~

The user must declare and initialise relevant hardware resources (globally) and pass them to the relevant function of `lib_xua`.

As an absolute minimum the following resources are required:

- A 1-bit port for audio master clock input  
- A n-bit port for internal feedback calculation (typically a free, unused port is used e.g. `16B`)
- A clock-block, which will be clocked from the master clock input port

Example declaration of these resources might look as follows::

    in port p_mclk_in                   = PORT_MCLK_IN;
    in port p_for_mclk_count            = PORT_MCLK_COUNT;   /* Extra port for counting master clock ticks */
    clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_5;   /* Master clock */

.. note::

    The `PORT_MCLK_IN` and `PORT_MCLK_COUNT` defintions are derived from the projects XN file 


The ``XUA_AudioHub()`` function requires an audio master clock input to clock the physical audio I/O. Less obvious is the reasoning for the ``XUA_Buffer()`` 
task having the same requirement - it is used for the USB feedback system and packet sizing.

Due to the above, if the ``XUD_AudioHub()`` and ``XUA_Buffer()`` cores must reside on separate tiles a separate master clock input port must be provided to each, for example::

    /* Master clock for the audio IO tile */
    in port p_mclk_in                   = PORT_MCLK_IN;

    /* Resources for USB feedback */
    in port p_mclk_in_usb               = PORT_MCLK_IN_USB;  /* Extra master clock input for the USB tile */

Whilst the hardware resources described in this section satisfy the basic requirements for the operation (or build) of `lib_xua` projects typically also needs some additional audio I/O, 
I2S or SPDIF for example. 

These should be passed into the various cores as required - see API and Features sections.

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
        XUA_AudioHub(c_aud, ...) ;
    }

``XUA_Buffer()`` expects its ``p_for_mclk_count`` argument to be clocked from the audio master clock before being passed it.
The following code satisfies this requirement::

    {
            /* Connect master-clock clock-block to clock-block pin */
            set_clock_src(clk_audio_mclk_usb, p_mclk_in_usb);           /* Clock clock-block from mclk pin */
            set_port_clock(p_for_mclk_count, clk_audio_mclk_usb);       /* Clock the "count" port from the clock block */
            start_clock(clk_audio_mclk_usb);                            /* Set the clock off running */

            XUA_Buffer(c_ep_out[1], c_ep_in[1], c_sof, c_aud_ctl, p_for_mclk_count, c_aud);

    }

.. note:: Keeping this configuration outside of ``XUA_Buffer()`` does not preclude the possibllty of sharing ``p_mclk_in_usb`` port with additional components

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


This section provides enough information to implement a skeleton program for a USB Audio device. When running the xCORE device will present itself as a USB Audio Class device on the bus.


Configuring XUA
~~~~~~~~~~~~~~~

Configuration of the various build time options of ``lib_xua`` is done via the optional header `xua_conf.h`. Such build time options include audio class version, sample rates, channel counts etc. 
Please see the API section for full listings.

The build system will automatically include the `xua_conf.h` header file as appropriate - the user should continue to include `xua.h` as previously directed. A simple example is shown below::

    #ifndef _XUA_CONF_H_
    #define _XUA_CONF_H_

    /* Output channel count */
    #define XUA_NUM_USB_CHAN_OUT (2)
    
    /* Product string */
    #define XUA_PRODUCT_STR_A2 "My Product"

    #endif


User functions
~~~~~~~~~~~~~~

To enable custom functionality, such as configuring external audio hardware, custom functionality on stream start/stop etc various user overridable functions are provided (see API section for full listings). The default implementations are empty. 


Codeless programming model
~~~~~~~~~~~~~~~~~~~~~~~~~~

Whilst it is possible to code a USB Audio device using the building blocks provided by `lib_xud` it is realised that this might not be desirable for some classes of customers or product.

For instance, some users may not have a large software development experience and simply want to customise some basic settings such as strings. Others may want to fully customise the implementation - adding additional functionality such as adding DSD or possibly only using a subset of the functions provided - just ``XUA_AudioHub``, for example.

In addition, the large number of supported features can lead a large number of tasks, hardware resources, communication channels etc, requiring quite a lot of code to be authored for each product.

In order to cater for the former class of users, a "codeless" option is provided. Put simply, a file ``main.xc`` is provided which includes a pre-authored ``main()`` function along with all of the required hardware resource declarations. Code is generated based on the options provided in ``xua_conf.h``

Using this development model the user simply must include a ``xua_conf.h`` with their settings and optionally implementations of any 'user functions' as desired. This, along with an XN file for their hardware platform, is all that is required to build a fully featured and functioning product.

This model also provides the benefit of a known-good, full codebase as a basis for a product. 

This behaviour described in this section is the default behaviour of `lib_xud`, to disable this please set ``EXCLUDE_USB_AUDIO_MAIN`` to 1 in the application makefile or ``xua_conf.h``.


