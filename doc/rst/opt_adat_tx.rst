|newpage|

ADAT Transmit
=============

The codebase supports a single ADAT transmitter that can transmit
eight channels of uncompressed digital audio at sample-rates of 44.1 or 48 kHz over an optical cable.
Higher rates are supported with a reduced number of samples via S/MUX (‘sample multiplexing’). Using S/MUX,
the ADAT transmitter can transmit four channels at 88.2 or 96 kHz or two channels at 176.4 or 192 kHz.

In order to provide ADAT transmit functionality ``lib_xua`` uses ``lib_adat`` (https://www.github.com/xmos/lib_adat).

Basic configuration of ADAT transmit functionality is achieved with the following :ref:`defines<opt_adat_tx_defines>`:

.. _opt_adat_tx_defines:

|beginfullwidth|

.. list-table:: ADAT tx defines
   :header-rows: 1
   :widths: 20 40 40

   * - Define
     - Description
     - Default
   * - ``XUA_ADAT_TX_EN``
     - Enable ADAT transmit
     - ``0`` (Disabled)
   * - ``ADAT_TX_MAX_CHANS``
     - Maximum number of channels to transmit over the ADAT interface
     - ``8, 4 or 2 depending on the min and max sampling freq supported in the application``
   * - ``ADAT_TX_INDEX``
     - Start channel index of ADAT TX channels
     - N/A (must be defined by the application)

|endfullwidth|

ADAT transmitter runs on the same tile as the Audio IO (``AUDIO_IO_TILE``)

The codebase expects the ADAT transmit port to be defined in the application XN file as ``PORT_ADAT_OUT``.
This must be a 1-bit port, for example::

    <Port Location="XS1_PORT_1G"  Name="PORT_ADAT_OUT"/>
