
|newpage|

Synchronisation & Clocking
==========================

The codebase supports "Synchronous" and "Asynchronous" modes for USB transfer as defined by the
USB specification(s).

Asynchronous mode (``XUA_SYNCMODE_ASYNC``) has the advantage that the device is clock-master. This means that
a high-quality local master-clock source can be utilised. It also has the benefit that the device may
synchronise it's master clock to an external digital input stream e.g. S/PDIF thus avoiding sample-rate
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

When operating in asynchronous mode xcore.ai based devices will be configured, by default, to use their internal
"Applications" PLL to generated an appropriate master-clock signal.  To disable this ``XUA_USE_APP_PLL`` should be
set to ``0``. For all other devices the developer is expected to supply external master-clock generation circuitry.

When operating in synchronous mode an xcore.ai based device, by default, will be configured to used it's internal
"application" PLL to generate a master-clock synchronised to the USB host.

xcore-200 based devices do not have this application PLL and so an external Cirrus Logic CS2100 device is required
for master clock generation. The codebase expects to drive a synchronisation signal to this external device.

In this case the developer should ensure the define in :ref:`opt_sync_ref_defines` is set appropriately.

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

The codebase expects this reference signal port to be defined in the application XN file as ``PORT_PLL_REF``.
This may be a port of any bit-width, however, connection to bit[0] is assumed::

    <Port Location="XS1_PORT_1A"  Name="PORT_PLL_REF"/>

Configuration of the external CS2100 device (typically via I2C) is beyond the scope of this document.

Note, in all cases the master-clocks are generated (when using the xcore.ai Application PLL) or should be generated
(if using external circuitry) to match the defines in :ref:`opt_sync_mclk_defines`.

.. _opt_sync_mclk_defines:

.. list-table:: Master clock frequencies
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``MCLK_48``
     - Master clock frequency (in Hz)used for sample-rates related to 48KHz
     - NOTE
   * - ``MCLK_441``
     - Master clock frequency (in Hz) used for sample-rates related to 44.1KHz
     - NONE

.. note::

   The master clock defines above are critical for proper operation and default values are not provided.
   If they are not defined by the devloper a build error will be emmited.
