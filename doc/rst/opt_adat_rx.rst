|newpage|

ADAT receive
============

The codebase supports a single ADAT receiver that can receive up to eight channels of audio at a sample rate
of 44.1kHz or 48kHz over an optical interface.
Higher rates are supported with a reduced number of samples via S/MUX (‘sample multiplexing’). Using S/MUX,
the ADAT receiver can receive four channels at 88.2 or 96 kHz or two channels at 176.4 or 192 kHz.

In order to provide ADAT functionality ``lib_xua`` uses `lib_adat <https://www.xmos.com/file/lib_adat>`_.

Basic configuration of ADAT receive functionality is achieved with the defines in
:numref:`opt_adat_rx_defines`.

|beginfullwidth|

.. _opt_adat_rx_defines:

.. list-table:: ADAT RX defines
   :header-rows: 1
   :widths: 25 40 40

   * - Define
     - Description
     - Default
   * - ``XUA_ADAT_RX_EN``
     - Enable ADAT receive
     - ``0`` (Disabled)
   * - ``ADAT_RX_INDEX``
     - Start channel index of ADAT RX channels
     - N/A (must be defined by the application)

|endfullwidth|

The codebase expects the ADAT receive port to be defined in the application XN file as ``PORT_ADAT_IN``.
This must be a 1-bit port, for example::

    <Port Location="XS1_PORT_1O"  Name="PORT_ADAT_IN"/>

When ADAT receive is enabled the codebase expects to either drive a synchronisation signal to an external
Cirrus Logic CS2100 device or use `lib_sw_pll <https://www.xmos.com/file/lib_sw_pll>`_ (`xcore.ai`
only) for generating a master clock that is synchronised to the ADAT digital stream.

The programmer should ensure the defines in :numref:`opt_adat_rx_ref_defines` are set appropriately:

|beginfullwidth|

.. _opt_adat_rx_ref_defines:

.. list-table:: Reference clock location
   :header-rows: 1
   :widths: 20 60 20

   * - Define
     - Description
     - Default
   * - ``XUA_PLL_REF_TILE_NUM``
     - Tile location of reference signal to CS2100 device
     - Derived from location of ``PORT_PLL_REF`` in XN file

|endfullwidth|

The codebase expects this reference signal port to be defined in the application XN file as ``PORT_PLL_REF``.
This may be a port of any bit-width, however, connection to bit[0] is assumed::

    <Port Location="XS1_PORT_1A"  Name="PORT_PLL_REF"/>

Configuration of the external CS2100 device (typically via I²C) is beyond the scope of this document.
