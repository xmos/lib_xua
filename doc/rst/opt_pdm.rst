|newpage|

PDM Microphones
===============

The codebase supports input from up to 8 PDM microphones although this is extensible.

PDM microphone support is provided via ``lib_mic_array`` (https://www.github.com/xmos/lib_mic_array).  Settings for PDM microphones are controlled
via the following :ref:`defines<opt_pdm_defines>`:

.. _opt_pdm_defines:

|beginfullwidth|

.. list-table:: PDM defines
   :header-rows: 1
   :widths: 40 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_NUM_PDM_MICS``
     - The number of mics to enable (0 for disabled). This enables compilation of the PDM to PCM code also.
     - ``0`` (disabled)
   * - ``PDM_MIC_INDEX``
     - Defines which starting input channel the mics map to
     - ``0``
   * - ``XUA_PDM_MIC_FREQ``
     - Defines the PCM output sample rate of ``lib_mic_array``
     - None (must be defined by the application if microphones enabled)

|endfullwidth|

Please see the :ref:`PDM Microphones<sw_pdm_main>` section for further details.
