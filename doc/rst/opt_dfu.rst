|newpage|

DFU
===

The codebase supports DFU over USB implementation compliant with version 1.1 of
`Universal Serial Bus Device Class Specification for Device Firmware Upgrade <https://www.usb.org/sites/default/files/DFU_1.1.pdf>`_.

:ref:`DFU defines<opt_dfu>` lists the DFU related configuration options.

.. _opt_dfu:

|beginfullwidth|

.. list-table:: DFU defines
   :header-rows: 1
   :widths: 40 40 40

   * - Define
     - Description
     - Default
   * - ``XUA_DFU_EN``
     - Enable DFU functionality
     - ``1`` (Enabled)
   * - ``DFU_PID``
     - Product ID when enumerating in DFU mode. This is recommended to be different from the runtime device PID
     - ``PID_AUDIO_2`` or ``PID_AUDIO_1`` depending on whether the device is running Audio Class 2.0 or 1.0
   * - ``DFU_VENDOR_ID``
     - Vendor ID when enumerating in DFU mode.
     - Same as the runtime device VENDOR_ID and cannot be overriden
   * - ``DFU_MANUFACTURER_STR_INDEX``
     - Index to string descriptor containing the Vendor/Manufacturer name in the DFU mode device descriptor
     - Same as index to string descriptor containing the Vendor/Manufacturer name in the runtime mode device descriptor
       and cannot be overriden.
   * - ``DFU_PRODUCT_STR_INDEX``
     - Index to string descriptor containing the Product name in the DFU mode device descriptor
     - Same as index to string descriptor containing the Product name in the runtime mode device descriptor
       and cannot be overriden.

|endfullwidth|
