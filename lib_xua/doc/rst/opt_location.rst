|newpage|

Code Location
=============

When designing a system there is a choice as to which hardware resources to use for each interface.
In a multi-tile system the codebase needs to be informed as to which tiles to use for these hardware
resources and associated code.

A series of defines are used to allow the programmer to easily move code between tiles. Arguably the 
most important of these are ``AUDIO_IO_TILE`` and ``XUD_TILE``. :ref:`opt_location_defines` shows a 
full listing of these ``TILE`` defines.

.. tabularcolumns:: lp{5cm}l
.. _opt_location_defines:
.. list-table:: Tile defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``AUDIO_IO_TILE``
     - Tile on which I2S/TDM, ADAT Rx, S/PDIF Rx & mixer resides
     - ``0``
   * - ``XUD_TILE``
     - Tile on which USB resides, including buffering for all USB interfaces/endppoints
     - ``0`` 
   * - ``MIDI_TILE``
     - Tile on which MIDI resides
     - Same as ``AUDIO_IO_TILE``
   * - ``SPDIF_TX_TILE``
     - Tile on which S/PDIF Tx resides
     - Same as ``AUDIO_IO_TILE``
   * - ``PDM_TILE``
     - Tile on which PDM microphones resides
     - Same as ``AUDIO_IO_TILE``
   * - ``PLL_REF_TILE``
     - Tile on which reference signal to CS2100 resides
     - Same as ``AUDIO_IO_TILE``

.. note:: 
    
    It should be ensured that the relevant port defines in the application XN file match the code location defines
