|newpage|

Code Location
=============

When designing a system there is a choice as to which hardware resources to use for each interface.
In a multi-tile system the codebase needs to be informed as to which tiles to use for these hardware
resources and associated code.

A series of defines are used to allow the programmer to easily move code between tiles. Arguably the
most important of these are ``XUA_AUDIO_IO_TILE_NUM`` and ``XUA_XUD_TILE_NUM``.
:numref:`opt_location_defines` shows a full listing of these ``TILE`` defines.

.. note::

    If not explicitly defined by user, tile numbers will be derived from the application XN file
    ``PORT`` defines. In general this is the recommended approach.

.. tabularcolumns:: lp{5cm}l
.. _opt_location_defines:
.. list-table:: Tile defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_AUDIO_IO_TILE_NUM``
     - Tile on which I2S/TDM, ADAT Rx, S/PDIF Rx & mixer resides
     - Derived from related port defines in the application XN file
   * - ``XUA_XUD_TILE_NUM``
     - Tile on which USB resides, including buffering for all USB interfaces/endppoints
     - ``0``
   * - ``XUA_MIDI_TILE_NUM``
     - Tile on which MIDI resides
     - Derived from MIDI related port defines in the application XN file
   * - ``XUA_SPDIF_TX_TILE_NUM``
     - Tile on which S/PDIF Tx resides
     - Derived from PORT_SPDIF_OUT port define in the application XN file
   * - ``XUA_MIC_PDM_TILE_NUM``
     - Tile on which PDM microphones resides
     - Derived from Mic related port defines in the application XN file
   * - ``XUA_PLL_REF_TILE_NUM``
     - Tile on which reference signal to CS2100 resides
     - Derived from PORT_PLL_REF port define in the application XN file

.. note::

    It should be ensured that the relevant port defines in the application XN file match the code
    location defines
