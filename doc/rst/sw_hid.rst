Audio controls via Human Interface Device (HID)
===============================================

The design supports simple audio controls such as play/pause, volume up/down etc via the `USB Human
Interface Device Class Specification <https://www.usb.org/document-library/device-class-definition-hid-111>`_.

This functionality is enabled by setting the ``HID_CONTROLS`` define to ``1``.  Setting to ``0``
disables this feature.

When turned on the following items are enabled:

#. HID descriptors are enabled in the Configuration Descriptor informing the host that the device has a HID interface.
#. A Get Report Descriptor request is enabled in ``endpoint0``.
#. Endpoint data handling is enabled in the ``buffer`` thread.

The Get Descriptor Request enabled in endpoint 0 returns the report descriptor for the HID device.
This details the format of the HID reports returned from the device to the host.  It maps a bit in
the report to a function such as play/pause.

The USB Audio Framework implements a report descriptor that should fit most basic audio device controls.
If further controls are necessary the HID Report Descriptor in `hid_report_descriptor.h` should be modified.
The default report size is 1 byte with the format as follows:

.. table:: Default HID Report Format

   +-------------+-------------------------+
   | Bit         | Function                |
   +=============+=========================+
   | 0           | Play/Pause              |
   +-------------+-------------------------+
   | 1           | Scan Next Track         |
   +-------------+-------------------------+
   | 2           | Scan Prev Track         |
   +-------------+-------------------------+
   | 3           | Volume Up               |
   +-------------+-------------------------+
   | 4           | Volume Down             |
   +-------------+-------------------------+
   | 5           | Mute                    |
   +-------------+-------------------------+
   | 6-7         | Unused                  |
   +-------------+-------------------------+

On each HID report request from the host the function ``UserHIDGetData()`` is called from
``XUA_Buffer_Ep()``.  This function is passed an array ``hidData[]`` by reference.
The programmer should report the state of the buttons into this array.
For example, if a volume up command is desired, bit 3 should be set to 1, else 0.

Since the ``UserHIDGetData()`` function is called from the ``XUA_Buffer_Ep()`` thread, care should
be taken not to add to much execution time to this function since this could cause issues with
servicing other endpoints.

