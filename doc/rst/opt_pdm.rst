|newpage|

PDM microphones
===============

``lib_xua`` supports input from up to 8 PDM microphones although this is extensible.

PDM microphone support is provided via `lib_mic_array <https://www.xmos.com/file/lib_mic_array>`__.
Settings for PDM microphones are controlled with the defines in :numref:`opt_pdm_defines`.

.. _opt_pdm_defines:

.. list-table:: PDM defines
   :header-rows: 1
   :widths: 60 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_NUM_PDM_MICS``
     - The number of mic outputs to enable (0 for disabled). This enables compilation of the PDM to PCM code also.
     - ``0`` (disabled)
   * - ``XUA_NUM_PDM_MICS_IN``
     - The number of mic input lines. This defines the width of the PDM data port and must be at least ``XUA_NUM_PDM_MICS``.
     - ``XUA_NUM_PDM_MICS``
   * - ``XUA_PDM_MIC_INDEX``
     - Defines which starting input channel the mics map to
     - ``0``
   * - ``XUA_PDM_MIC_USE_PDM_ISR``
     - Define as 1 to enable merging of the PDM receive task and decimation task into a single thread using an ISR
     - ``1`` (Run PDM RX service as an ISR)
   * - ``XUA_PDM_MIC_USE_DDR``
     - Define as 1 to enable two microphones sharing a single data line (DDR mode)
     - ``1`` (DDR mode)

.. note::

   mic array task always runs on the tile defined by ``XUA_MIC_PDM_TILE_NUM``, the value of which is inferred
   from the ``PORT_PDM_CLK`` port define in the application XN file.

.. note::

   Currently the only supported sampling rates for the PDM microphones are 16kHz, 32kHz and 48kHz.

.. note::

   Setting ``XUA_PDM_MIC_USE_PDM_ISR`` is only recommended for PDM mic counts below 8.


Please see the :ref:`PDM Microphones<sw_pdm_main>` section for further details.
