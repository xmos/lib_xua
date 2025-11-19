|newpage|

.. _sw_pdm_main:

PDM microphones
===============

``lib_xua`` is capable of integrating with PDM microphones.
The PDM stream from the microphones is converted to PCM and output to the host via USB.

Interfacing to the PDM microphones is done using the `XMOS` microphone array library
(`lib_mic_array <https://www.xmos.com/file/lib_mic_array>`_).
``lib_mic_array`` is designed to allow interfacing to PDM microphones coupled to efficient decimation filters
at a user configurable output sample rate.

.. note::
    The ``lib_mic_array`` library is only available for `xcore.ai` series devices since it uses the
    Vector Processing Unit of the XS3 architecture.

Up to eight PDM microphones can be attached the PDM interface (``mic_array_task()``) but it is
possible to extend this.

After PDM capture and decimation to the output sample-rate various other steps take place e.g. DC offset elimination etc.
Please refer to the documentation provided with  ``lib_mic_array`` for further implementation detail and a complete feature set.

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

When initialising the mic array, a ``pdm_rx_resources_t`` structure is passed to ``mic_array_init()``. This contains the hardware
resource IDs required for PDM capture:

.. code-block:: c

  unsigned mClk = MCLK_48, pdmClk = 3072000;
  #if (XUA_PDM_MIC_USE_DDR)
        pdm_rx_resources_t pdm_res = PDM_RX_RESOURCES_DDR(
            PORT_MCLK_IN,
            PORT_PDM_CLK,
            PORT_PDM_DATA,
            mClk,
            pdmClk,
            XS1_CLKBLK_1,
            XS1_CLKBLK_2);
    #else
        pdm_rx_resources_t pdm_res = PDM_RX_RESOURCES_SDR(
            PORT_MCLK_IN,
            PORT_PDM_CLK,
            PORT_PDM_DATA,
            mClk,
            pdmClk,
            XS1_CLKBLK_1);
    #endif
        mic_array_init(&pdm_res, NULL, mic_samp_rate);

For full details of the ``pdm_rx_resources_t`` structure and mic array initialisation,
please refer to the `lib_mic_array documentation <https://www.xmos.com/file/lib_mic_array>`_.

Usage & integration
-------------------

Mic array task
^^^^^^^^^^^^^^

A PDM microphone wrapper :c:func:`mic_array_task()` is called from ``main()`` and takes one channel argument connecting it to the rest of the system:

.. code-block:: c

  on stdcore[XUA_MIC_PDM_TILE_NUM]: mic_array_task(c_pdm_pcm);

The implementation of :c:func:`mic_array_task()` can be found in the file ``mic_array_task.c``. It typically takes one hardware thread.

:c:func:`mic_array_task()` runs a ``while(1)`` loop. At the start of the loop, it waits to receive the current sampling rate over the
``c_pdm_pcm`` channel. Once received, it initialises the mic array for the requested sampling rate (using ``mic_array_init()``) and
then starts the mic array thread(s) via ``mic_array_start()``.
``mic_array_start()`` launches either one or two hardware threads, depending on the value of :c:macro:`XUA_PDM_MIC_USE_PDM_ISR`.

The ``c_pdm_pcm`` channel is passed to the mic array decimator thread, over which it sends decimated PCM frames to :c:func:`XUA_AudioHub()`.

Note, it is assumed that the system shares a global master-clock, therefore no additional buffering or rate-matching/conversion
is required. This ensures the PDM subsystem and XUA Audiohub are synchronous.

Receiving PCM samples
^^^^^^^^^^^^^^^^^^^^^

``XUA_AudioHub's`` main IO loop (``AudioHub_MainLoop()``) calls the mic array function ``ma_frame_rx()`` using the
other end of the channel passed to the mic array thread to receive PCM frames from the mic array:

.. code-block:: c

  ma_frame_rx(mic_samps_base_addr, c_m2a, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, MIC_ARRAY_CONFIG_MIC_COUNT);

Note that the interface between mic array and xua is sample based - so the only supported value of MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME is 1.

Restarting the mic array
^^^^^^^^^^^^^^^^^^^^^^^^

If ``AudioHub_MainLoop()`` function exits, the mic array is shut down by calling ``ma_shutdown()`` from :c:func:`XUA_AudioHub()`:

.. code-block:: c

  ma_shutdown((chanend_t)c_pdm_in); // shutdown mics

Note, that the channel passed to ``ma_shutdown()`` is the same one used by ``ma_frame_rx()`` to receive PCM frames,
so care must be taken to ensure that ``ma_frame_rx()`` is not being called concurrently when ``ma_shutdown()`` is invoked.

Calling ``ma_shutdown()`` causes the mic array thread(s) to terminate. When this happens, the ``mic_array_start()`` call inside
:c:func:`mic_array_task()` returns, after which :c:func:`mic_array_task()` waits again to receive the next
sampling-rate value on the channel before restarting the mic-array thread(s).

Weak callback APIs
^^^^^^^^^^^^^^^^^^

Two weak callback APIs are provided which optionally allow user code to be executed at startup (post PDM microphone initialisation)
and after each sample frame is formed.
These can be useful for custom hardware initialisation required by the PDM microphone or post processing such as gain control
before samples are forwarded to XUA::

    void user_pdm_init();
    void user_pdm_process(int32_t mic_audio[MIC_ARRAY_CONFIG_MIC_COUNT]);

Be aware that ``user_pdm_process()`` is called in the main Audio Hub loop and so and processing should be kept very short to avoid breaking timing of I²S etc. Typically a small fraction of sample period is acceptable although the headroom is much larger at lower sample rates. The array of samples ``mic_audio`` can modified in-place.

