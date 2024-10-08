|newpage|

PDM Microphones
===============

The codebase supports input from up to 8 PDM microphones. 

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
     - Defines which input channel the mics map to 
     - ``0``

The codebase expects 1-bit ports to be defined in the application XN file for ``PORT_PDM_CLK`` and ``PORT_PDM_MCLK``.
An 8-bit port is expected for ``PORT_PDM_DATA``. For example::

    <Tile Number="0" Reference="tile[0]">
        <!-- Mic related ports -->
        <Port Location="XS1_PORT_1E" Name="PORT_PDM_CLK"/>
        <Port Location="XS1_PORT_8B" Name="PORT_PDM_DATA"/>
        <Port Location="XS1_PORT_1F" Name="PORT_PDM_MCLK"/>
    </Tile>

