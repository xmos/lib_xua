PDM Microphones
~~~~~~~~~~~~~~~

The codebase supports up to input from PDM microphones. 

PDM microphone support is provided via ``lib_mic_array``.  Settings for PDM microphones controlled via the following defines:

.. _opt_pdm_defines:

.. list-table:: Other defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - XUA_NUM_PDM_MICS
     - The desired number of output channels via I2S (0 for disabled)
     - N/A (Must be defined)
   * - PDM_MIC_INDEX
     - The desired number of input channels via I2S (0 for disabled)
     - N/A (Must be defined)


