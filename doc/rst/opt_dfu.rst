|newpage|

DFU
===

The codebase supports DFU over USB implementation compliant with version 1.1 of
`Universal Serial Bus Device Class Specification for Device Firmware Upgrade <https://www.usb.org/sites/default/files/DFU_1.1.pdf>`_.

:numref:`opt_dfu` lists the DFU related configuration options.

.. _opt_dfu:

.. list-table:: DFU defines
   :header-rows: 1
   :widths: 20 40 40

   * - Define
     - Description
     - Default
   * - ``XUA_DFU_EN``
     - Enable DFU functionality
     - ``1`` (Enabled)
   * - ``DFU_PID``
     - Product ID when enumerating in DFU mode. This is recommended to be different from the runtime device PID
     - ``PID_AUDIO_2`` or ``PID_AUDIO_1`` depending on whether the device is running Audio Class 2.0 or 1.0

