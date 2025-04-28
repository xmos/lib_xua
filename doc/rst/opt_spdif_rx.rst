|newpage|

S/PDIF receive
==============

The codebase supports a single, stereo, S/PDIF receiver. This can be input via 75 Ω coaxial or optical fibre.
In order to provide S/PDIF functionality ``lib_xua`` uses `lib_spdif <https://www.xmos.com/file/lib_spdif>`__.

Basic configuration of S/PDIF receive functionality is achieved with the defines in
:numref:`opt_spdif_rx_defines`

.. _opt_spdif_rx_defines:

.. list-table:: S/PDIF rx defines
   :header-rows: 1

   * - Define
     - Description
     - Default
   * - ``XUA_SPDIF_RX_EN``
     - Enable S/PDIF receive
     - ``0`` (Disabled)
   * - ``SPDIF_RX_INDEX``
     - Defines which channels S/PDIF will be input on
     - N/A (must be defined)

.. note::

   S/PDIF receive always runs on the tile defined by ``AUDIO_IO_TILE``

The codebase expects the S/PDIF receive port to be defined in the application XN file as ``PORT_SPDIF_IN``.
This must be a 1-bit port, for example::

    <Port Location="XS1_PORT_1A"  Name="PORT_SPDIF_IN"/>

When S/PDIF receive is enabled the codebase expects to either drive a synchronisation signal to an external
Cirrus Logic CS2100 device or use lib_sw_pll (xcore.ai only) for master-clock generation.

The programmer should ensure the defines in :numref:`opt_spdif_rx_ref_defines` are set appropriately

.. _opt_spdif_rx_ref_defines:

.. list-table:: Reference clock location
   :header-rows: 1

   * - Define
     - Description
     - Default
   * - ``PLL_REF_TILE``
     - Tile location of reference to CS2100 device
     - ``AUDIO_IO_TILE``

The codebase expects this reference signal port to be defined in the application XN file as ``PORT_PLL_REF``.
This may be a port of any bit-width, however, connection to bit[0] is assumed::

    <Port Location="XS1_PORT_1A"  Name="PORT_PLL_REF"/>

Configuration of the external CS2100 device (typically via I2C) is beyond the scope of this document.

