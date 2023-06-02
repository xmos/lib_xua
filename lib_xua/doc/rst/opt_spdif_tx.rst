|newpage|

S/PDIF Transmit 
===============

The codebase supports a single, stereo, S/PDIF transmitter. This can be output over 75 Ω coaxial or optical fibre. 
In order to provide S/PDIF transmit functionality ``lib_xua`` uses ``lib_spdif`` (https://www.github.com/xmos/lib_spdif).

Basic configuration of S/PDIF transmit functionality is achieved with the defines in :ref:`opt_spdif_tx_defines`

.. _opt_spdif_tx_defines:

.. list-table:: S/PDIF tx defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_SPDIF_TX_EN``
     - Enable S/PDIF transmit
     - ``0`` (Disabled)
   * - ``SPDIF_TX_INDEX``
     - Output channel offset to use for S/PDIF transmit 
     - ``0``

In addition, the developer may choose which tile the S/PDIF transmitter runs on, see :ref:`opt_spdif_tx_tile_defines`.

.. _opt_spdif_tx_tile_defines:

.. list-table:: S/PDIF tile define
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``SPDIF_TX_TILE``
     - Tile that S/PDIF tx is connected to
     - ``AUDIO_IO_TILE``

The codebase expects the S/PDIF transmit port to be defined in the application XN file as ``PORT_SPDIF_OUT``. 
This must be a 1-bit port, for example::

    <Port Location="XS1_PORT_1A"  Name="PORT_SPDIF_OUT"/>

