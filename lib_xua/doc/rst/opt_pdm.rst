|newpage|

PDM Microphones
===============

The codebase supports input from up to 8 PDM microphones although this is extensible.

PDM microphone support is provided via ``lib_mic_array``.  Settings for PDM microphones are controlled
via the defines in :ref:`opt_pdm_defines`. 

.. _opt_pdm_defines:

.. list-table:: PDM defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_NUM_PDM_MICS``
     - The number of mics to enable (0 for disabled)
     - ``0`` (disabled)
   * - ``PDM_MIC_INDEX``
     - Defines which starting input channel the mics map to 
     - ``0``

Please see the `PDM Microphones <sw_pdm_main>`_ section for further details. 
