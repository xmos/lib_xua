.. _usb_audio_sec_dfu:

Device Firmware Upgrade (DFU) over USB
======================================

The DFU implementation in ``lib_xua`` is compliant with version 1.1 of
`Universal Serial Bus Device Class Specification for Device Firmware Upgrade <https://www.usb.org/sites/default/files/DFU_1.1.pdf>`_.

This section describes the DFU implementation in ``lib_xua``. For information about using a DFU loader to send DFU
commands to the USB Audio device, refer to appnote
`AN02019: Using Device Firmware Upgrade (DFU) in USB Audio <www.xmos.com/file/an02019>`_.

The USB device descriptors expose a DFU interface that handles updates to the boot image of the device over USB.

The host sends DFU requests as Host to Device Class requests to the DFU interface.
On receiving DFU commands from the host, the ``DFUDeviceRequests`` function is called from the Endpoint 0 thread.
This function calls the DFU handler functions over the ``dfuInterface`` XC interface.
The DFU handler thread, ``DFUHandler`` that implements the server side of the ``dfuInterface`` has to be
scheduled on the same tile as the flash so it can access the flash memory.
The ``dfuInterface`` interface essentially links USB to the
`XMOS flash user library <https://www.xmos.com/file/libflash-api#libflash-api>`_.

The DFU interface is enabled by default (See ``XUA_DFU_EN`` define in `xua_conf_default.h`).
When DFU is enabled, there are two sets of descriptors that the device can export, depending on the mode in which it operates.
There is a descriptor set for the runtime mode, which is the mode the device normally operates in and a set of descriptors for the DFU mode.

In the runtime mode, the DFU interface is one of potentially multiple interfaces that the device exposes.
:numref:`dfu_interface_runtime` shows the DFU interface
descriptors when enumerating in runtime mode as seen in a `Beagle USB analyser <https://www.totalphase.com/products/data-center/>`_ trace.

 .. _dfu_interface_runtime:

 .. figure:: images/dfu_interface_runtime_mode.png
   :width: 100%

   DFU interface when part of runtime mode descriptor set

Note the **bInterfaceProtocol** field set to **Runtime**.

In DFU mode, the device exports the DFU descriptor set. The DFU mode descriptors specify only one interface, the DFU interface.
:numref:`dfu_interface_dfu` shows the DFU interface
descriptors when enumerating in DFU mode as seen in a Beagle USB analyser trace.

 .. _dfu_interface_dfu:

 .. figure:: images/dfu_interface_dfu_mode.png
   :width: 100%

   DFU interface when part of DFU mode descriptor set

Note the **bInterfaceProtocol** field set to **DFU mode**.

Before starting the DFU upload or download process, the host sends a ``DFU_DETACH`` command to detach the device from runtime to DFU mode.
In response to the ``DFU_DETACH`` command, the device reboots itself into DFU mode and enumerates using the DFU mode descriptors.
Once the device is in DFU mode, the DFU interface can accept commands defined by the
`DFU 1.1 class specification <https://www.usb.org/sites/default/files/DFU_1.1.pdf>`_.

After detaching the device, the host proceeds with the DFU download/upload commands to write/read the firmware upgrade image to/from the device.
Once the DFU download or upload process is complete, the host sends a ``DETACH`` command, and the device reboots itself back in runtime mode.

.. note::

   It is recommended that the runtime mode and DFU mode descriptors have different product IDs. This is to ensure that the host operating
   system loads the correct driver as the device switches between runtime and DFU modes. The runtime and DFU PID are defined as overridable
   defines ``PID_AUDIO_2`` and ``DFU_PID`` respectively in ``xua_conf_default.h``. Users can define custom PIDs in their application by overriding these defines.

During the DFU download process, on receiving the first ``DFU_DNLOAD`` command (``wBlockNum`` = 0), the device erases
``FLASH_MAX_UPGRADE_SIZE`` bytes of the upgrade section of the flash. This is done by repeatedly calling ``flash_cmd_start_write_image``
and can take several seconds. To avoid the ``DFU_DNLOAD`` request timing out, the flash erase is instead done in the ``DFU_GETSTATUS`` handling
code for block 0. So for block 0, the device ends up returning the status as ``dfuDNBUSY`` several times while the flash
erase is in progress. :numref:`dfu_download_seq_diag` describes the DFU download process.

 .. _dfu_download_seq_diag:

 .. figure:: images/dfu_download.png
   :width: 75%

   Message sequence chart for the DFU download operation

.. note::

   Once a valid upgrade image is loaded in flash, on subsequent reboots, the device will boot from the upgrade image.
   If the upgrade image is invalid, the factory image will be loaded. To revert back to the factory image, download an invalid upgrade file to the device.
   For example, DFU download a file containing the word 0xFFFFFFFF to the device.

|newpage|

Enumerating as a WinUSB device on Windows
-----------------------------------------

The Endpoint 0 code supports extra descriptors called the `Microsoft Operating System (MSOS) descriptors <https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors>`_
that allow the device to enumerate as a WinUSB device on Windows.
The MSOS descriptors report the compatible ID as *WINUSB* which enables Windows to load `Winusb.sys` as the device's
function driver without a custom INF file. This means that when the device is connected, the DFU interface
shows up as WinUSB compatible automatically, without requiring the user to manually load a driver for it using a utility like Zadig.

The MSOS descriptors are present in the file ``xua_ep0_msos_descriptors.h``. In order to enumerate as a device capable of supplying MSOS
descriptors, the device's ``bcdUSB`` version in the device descriptor has to be **0x0201**. On seeing the ``bcdUSB`` version as 0x0201 when the device
enumerates, the host requests for a descriptor called the Binary Device Object Store (BOS) descriptor.
This descriptor contains information about the capability of the device. It specifies the device to be MSOS 2.0 capable and contains information about
the vendor request code (``bRequest``) and the request length (``wLength``) that the host needs to use to when making a vendor request to query for the MSOS
descriptor.

The host then makes a vendor request with the ``bRequest`` and ``wLength`` as specified in the BOS platform descriptor querying for the MSOS descriptor.

.. warning::
   If writing a host application that also sends vendor requests to the device, users should ensure that they do not use the ``bRequest`` that is reserved
   for the MSOS descriptor. The MSOS descriptor vendor request's ``bRequest`` is defined as the
   ``REQUEST_GET_MS_DESCRIPTOR`` define in ``xua_ep0_msos_descriptors.h``.

   .. code-block:: c

      #define REQUEST_GET_MS_DESCRIPTOR   0x20


The MSOS descriptor reports the compatible ID as *WINUSB* for the DFU interface. It also specifies the device interface GUID in its registry property.
The GUID is required to access the DFU interface from a user application running on the host (for example the Thesycon DFU driver or the dfu-util DFU application)

.. note::

   The default device interface GUID for the DFU interfaces is specified in the ``WINUSB_DEVICE_INTERFACE_GUID_DFU`` define in ``xua_conf_default.h``.
   Users can override this by redefining ``WINUSB_DEVICE_INTERFACE_GUID_DFU`` in the application. A utility such as `guidgenerator <https://guidgenerator.com/>`_ can be used for generating a GUID.

.. tip::

   The MSOS descriptors for reporting WinUSB compatibility are only relevant for Windows.

