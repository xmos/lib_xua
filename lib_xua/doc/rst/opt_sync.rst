
|newpage|

Synchronisation
===============

The codebase supports "Synchronous" and "Asynchronous" modes for USB transfer as defined by the 
USB specification(s).

Asynchronous mode (``XUA_SYNCMODE_ASYNC``) has the advantage that the device is clock-master. This means that 
a high-quality local master-clock source can be utilised. It also has the benefit that the device may 
synchronise it's master clock to an external digital input stream e.g. S/PDIF and thus avoiding sample-rate
conversion.

The drawback of this mode is that it burdens the host with syncing to the device which some hosts 
may not support. This is especially pertinent to embedded hosts, however, most PC's and mobile devices
will indeed support this mode.

Synchronous mode (``XUA_SYNCMODE_SYNC``) is an option if the target host does not support asynchronous mode
or if it is desirable to synchronise many devices to a single host. It should be noted, however, that input 
from digital streams, such as S/PDIF, are not currently supported in this mode.

.. note::
    
   The selection of synchronisation mode is done at build time and cannot be changed dynamically.

Setting the synchronisation mode of the device is done using the define in :ref:`opt_sync_defines`

.. _opt_sync_defines:

.. list-table:: Sync Define
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_SYNCMODE``
     - USB synchronisation mode
     - ``XUA_SYNCMODE_ASYNC``

When operating in synchronous mode a local master clock must be generated that is synchronised to the incoming 
SoF rate from USB. Either an external Cirrus Logic CS2100 device is required for this purpose 
or, on xcore.ai devices, the on-chip application PLL may be used via lib_sw_pll.
In the case of using the CS2100, the codebase expects to drive a synchronisation signal to this external device
as a reference.

The programmer should ensure the define in :ref:`opt_sync_ref_defines` is set appropriately.

.. _opt_sync_ref_defines:

.. list-table:: Reference clock location
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``PLL_REF_TILE``
     - Tile location of reference to CS2100 device
     - ``AUDIO_IO_TILE``
   * - ``XUA_USE_SW_PLL``
     - Whether or not to use sw_pll to recover the clock (xcore.ai only)
     - 1 for xcore.ai targets. May be overridden to 0 in ``xua_conf.h``

The codebase expects the CS2100 reference signal port to be defined in the application XN file as ``PORT_PLL_REF``. 
This may be a port of any bit-width, however, connection to bit[0] is assumed::

    <Port Location="XS1_PORT_1A"  Name="PORT_PLL_REF"/>

Configuration of the external CS2100 device (typically via I2C) is beyond the scope of this document.

