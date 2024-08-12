.. _usb_audio_sec_dfu:

Device Firmware Upgrade (DFU)
=============================

The DFU interface handles updates to the boot image of the device. The DFU implementation in lib_xua is
compliant with version 1.1 of
`Universal Serial Bus Device Class Specification for Device Firmware Upgrade <https://www.usb.org/sites/default/files/DFU_1.1.pdf>`_.
On receiving DFU commands from the host, the ``DFUDeviceRequests`` function is called from the Endpoint 0 core. This function calls the DFU handler function
over the ``dfuInterface`` XC interface. The DFU handler thread, ``DFUHandler`` that implements the server side of the ``dfuInterface`` has to be
scheduled on the same tile as the flash so it can access the flash memory.

The DFU interface is enabled by default (See XUA_DFU_EN define in `lib_xua <https://github.com/xmos/lib_xua/blob/develop/lib_xua/api/xua_conf_default.h>`_).
There are 2 set of descriptors, the descriptors for the runtime mode, which is the mode the device normally operates in and a set of descriptors for the DFU mode.
In the runtime mode, the DFU interface is one of potentially multiple interfaces that the device exposes. When seen in a Beagle trace, the DFU interface in the runtime
mode descriptors enumerates as following:

.. figure:: images/dfu_interface.png
    :width: 100%

The interface links USB to the XMOS flash user library (see :ref:`libflash_api`). In Application
mode the DFU can accept commands to reset the device into DFU mode. There are two ways to do this:

-  The host can send a ``DETACH`` request and then reset the
   device. If the device is reset by the host within a specified
   timeout, it will start in DFU mode (this is initially set to
   one second and is configurable from the host).

-  The host can send a custom user request
   ``XMOS_DFU_RESETDEVICE`` to the DFU interface that
   resets the device immediately into DFU mode.


Once the device is in DFU mode. The DFU interface can accept commands defined by the
`DFU 1.1 class specification <http://www.usb.org/developers/devclass_docs/DFU_1.1.pdf*USB>`_. In
addition the interface accepts the custom command ``XMOS_DFU_REVERTFACTORY`` which reverts the active
boot image to the factory image. Note that the XMOS specific command request
identifiers are defined in ``dfu_types.h`` within ``module_dfu``.
