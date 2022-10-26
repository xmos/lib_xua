|newpage|

S/PDIF Receive
==============

The codebase supports a single, stereo, S/PDIF receiver. This can be input via 75 Ω coaxial or optical fibre. 
In order to provide S/PDIF functionality ``lib_xua`` uses ``lib_spdif`` (https://www.github.com/xmos/lib_spdif).

Basic configuration of S/PDIF receive functionality is achieved with the defines in :ref:`opt_spdif_rx_defines`.

.. _opt_spdif_rx_defines:

.. list-table:: S/PDIF rx defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_SPDIF_RX_EN``
     - Enable S/PDIF receive
     - ``0`` (Disabled)
   * - ``SPDIF_RX_INDEX``
     - Defines which channels S/PDIF will be input on
     - N/A (must defined)

.. note::

   S/PDIF receive always runs on the tile defined by ``AUDIO_IO_TILE``

The codebase expects the S/PDIF receive port to be defined in the application XN file as ``PORT_SPDIF_IN``. 
This must be a 1-bit port, for example::

    <Port Location="XS1_PORT_1A"  Name="PORT_SPDIF_IN"/>

When S/PDIF receive is enabled the codebase expects to drive a synchronisation signal to an external 
Cirrus Logic CS2100 device for master-clock generation.

The programmer should ensure the define in :ref:`opt_spdif_rx_ref_defines` is set appropriately.

.. _opt_spdif_rx_ref_defines:

.. list-table:: Reference Clock Location
   :header-rows: 1
   :widths: 20 80 20

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

