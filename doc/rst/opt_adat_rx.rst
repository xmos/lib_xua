|newpage|

ADAT Receive
============

The codebase supports a single ADAT receiver that can receive up to eight channels of audio at a sample rate
of 44.1kHz or 48kHz over an optical interface.
In order to provide ADAT functionality ``lib_xua`` uses `lib_adat <https://www.xmos.com/file/lib_adat>`_.

Basic configuration of ADAT receive functionality is achieved with the following :ref:`defines<opt_adat_rx_defines>`:

.. _opt_adat_rx_defines:

|beginfullwidth|

.. list-table:: ADAT RX defines
   :header-rows: 1
   :widths: 25 40 40

   * - Define
     - Description
     - Default
   * - ``XUA_ADAT_RX_EN``
     - Enable ADAT receive
     - ``0`` (Disabled)
   * - ``ADAT_RX_MAX_CHANS``
     - Maximum number of channels to receive over the ADAT interface
     - ``8, 4 or 2`` depending on the min and max sampling freq supported in the application
   * - ``ADAT_RX_INDEX``
     - Start channel index of ADAT RX channels
     - N/A (must be defined by the application)

|endfullwidth|

.. note::

   ADAT receive always runs on the tile defined by ``AUDIO_IO_TILE``

The codebase expects the ADAT receive port to be defined in the application XN file as ``PORT_ADAT_IN``.
This must be a 1-bit port, for example::

    <Port Location="XS1_PORT_1O"  Name="PORT_ADAT_IN"/>

When ADAT receive is enabled the codebase expects to either drive a synchronisation signal to an external
Cirrus Logic CS2100 device or use lib_sw_pll (xcore.ai only) for generating a master clock that is synchronised
to the ADAT digital stream.

The programmer should ensure the following :ref:`defines<opt_adat_rx_ref_defines>` are set appropriately:

.. _opt_adat_rx_ref_defines:

|beginfullwidth|

.. list-table:: Reference Clock Location
   :header-rows: 1
   :widths: 20 60 20

   * - Define
     - Description
     - Default
   * - ``PLL_REF_TILE``
     - Tile location of reference signal to CS2100 device
     - ``AUDIO_IO_TILE``

|endfullwidth|

The codebase expects this reference signal port to be defined in the application XN file as ``PORT_PLL_REF``.
This may be a port of any bit-width, however, connection to bit[0] is assumed::

    <Port Location="XS1_PORT_1A"  Name="PORT_PLL_REF"/>

Configuration of the external CS2100 device (typically via I²C) is beyond the scope of this document.
