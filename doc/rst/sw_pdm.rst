|newpage|

.. _sw_pdm_main:

PDM microphones
===============

``lib_xua`` is capable of integrating with PDM microphones.
The PDM stream from the microphones is converted to PCM and output to the host via USB.

Interfacing to the PDM microphones is done using the `XMOS` microphone array library
(`lib_mic_array <https://www.xmos.com/file/lib_mic_array>`_).
``lib_mic_array`` is designed to allow interfacing to PDM microphones coupled to efficient decimation filters
at a user configurable output sample rate. Currently dynamic sample rate changing is not supported.

.. note::
    The ``lib_mic_array`` library is only available for `xcore.ai` series devices since it uses the
    Vector Processing Unit of the XS3 architecture.

Up to eight PDM microphones can be attached the PDM interface (``mic_array_task()``) but it is
possible to extend this.

After PDM capture and decimation to the output sample-rate various other steps take place e.g. DC offset elimination etc. Please refer to the documentation provided with  ``lib_mic_array`` for further implementation detail and a complete feature set.

By default the sample rates supported are 16 kHz, 32 kHz and 48 kHz although other rates are supportable with some modifications.

Please see `AN00248 Using lib_xua with lib_mic_array <https://github.com/xmos/lib_xua/tree/develop/examples/AN00248_xua_example_pdm_mics>`_ for a practical example of this feature.

Hardware characteristics
------------------------

The PDM microphones require a *clock input* and provide the PDM signal on a *data output*. All of
the PDM microphones must share the same clock signal (buffered on the PCB as appropriate), and
output onto the data wire(s) that are connected to the capture port. The lines required to interface
with PDM microphones are listed in :numref:`pdm_wire_table`.

.. _pdm_wire_table:

.. list-table:: PDM microphone data and signal wires
     :class: vertical-borders horizontal-borders
     :header-rows: 1

     * - Signal
       - Description
     * - CLOCK
       - The PDM clock the used by the microphones to drive the data out.
     * - DQ_PDM
       - The data from the PDM microphones on the capture port.

.. note::
    The clocking for PDM microphones may be single data rate (one microphone per pin) or double data rate (two microphones per pin clocking on alternate edges). By default ``lib_xua`` assumes double data rate which provides more efficient port usage.

No arguments are passed into ``lib_mic_array``. The library is configured statically using the following defines in ``xua_conf.h``:

- ``MIC_ARRAY_CONFIG_PORT_MCLK`` - The port resource for the MCLK from which the PDM_CLK is derived. Normally XS1_PORT_1D on Tile[1].
- ``MIC_ARRAY_CONFIG_PORT_PDM_CLK`` - The port resource which drives out the PDM clock.
- ``MIC_ARRAY_CONFIG_PORT_PDM_DATA`` - The port used to receive PDM data. May be 1 bit, 4 bit or 8 bits wide.
- ``MIC_ARRAY_CONFIG_CLOCK_BLOCK_A`` - The clock block used to generate the PDM clock signal.
- ``MIC_ARRAY_CONFIG_CLOCK_BLOCK_B``  - The clock block used to capture the PDM data (Only needed if DDR is used).
- ``XUA_PDM_MIC_FREQ``  - The output sample rate of lib_mic_array.

Optionally, the following defines may be overridden if needed:

- ``MIC_ARRAY_CONFIG_MCLK_FREQ`` - The system MCLK frequency in Hz, usually set to XUA_PDM_MIC_FREQ.
- ``MIC_ARRAY_CONFIG_PDM_FREQ`` - The PDM clock frequency in Hz. Usually set to 3072000.
- ``MIC_ARRAY_CONFIG_USE_DC_ELIMINATION`` - Whether or not to run a DC elimination filter. Set to 1 by default.
- ``MIC_ARRAY_CONFIG_USE_DDR`` - Whether or not to use Double Data Rate data capture on the PDM microphones. Set to 1 by default.


For full details of the effect of these defines please refer to the `lib_mic_array documentation <https://www.xmos.com/file/lib_mic_array>`_.

Usage & integration
-------------------

A PDM microphone wrapper is called from ``main()`` and takes one channel argument connecting it to the rest of the system:

    ``mic_array_task(c_pdm_pcm);``

The implementation of this function can be found in the file ``mic_array_task.c`` but it nominally takes one hardware thread.

Note, it is assumed that the system shares a global master-clock, therefore no additional buffering or rate-matching/conversion
is required. This ensures the PDM subsystem and XUA Audiohub are synchronous.

Two weak callback APIs are provided which optionally allow user code to be executed at startup (post PDM microphone initialisation) and after each sample frame is formed. These can be useful for custom hardware initialisation required by the PDM microphone or post processing such as gain control before samples are forwarded to XUA::

    void user_pdm_init();
    void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT]);

Be aware that ``user_pdm_process()`` is called in the main Audio Hub loop and so and processing should be kept very short to avoid breaking timing of I²S etc. Typically a small fraction of sample period is acceptable although the headroom is much larger at lower sample rates. The array of samples ``mic_audio`` can modified in-place.

