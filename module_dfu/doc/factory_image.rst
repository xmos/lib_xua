Installing the factory image to the device
==========================================

The DFU device interface is enabled by default in the XMOS USB Audio framework
(see ``devicedefines.h``), and the explicitly enabled in each reference design
by the following line included in the ``customdefines.h`` file of each
application::

  #define DFU                (1)

Use the XMOS Development Tools to run the command:

  ``xflash --boot-partition-size 0x20000 usb_audio.xe``

  Where the size passed using the ``--boot-partition-size n`` argument specifies
  in bytes the minimum size required to store the boot loader, factory image and
  any upgrade images.

This programs the factory default firmware image into the flash device.
The device will now support the DFU mechanism, and can use it to safely receive
firmware updates, as well as revert to the factory firmware image when required,
such as in the event of a failed upgrade attempt.

To use the firmware upgrade mechanism you need to build a firmware upgrade
image:

#. Edit the ``customdefines.h`` file of the application for the hardware in use,
   and change the Device Version Number by defining ``BCD_DEVICE``.
#. Rebuild the application.

To generate the firmware upgrade image run the following command:

  ``xflash --factory-version 13 --upgrade 1 usb_audio.xe 0x20000 -o new_firmware.bin``

  Where the tools version passed using the ``--factory-version version``
  argument specifies the version of the tools used to create the factory image.
  This should be passed as ``12`` for images created using tools versions 10, 11
  and 12.

  The ``--upgrade id xe-file [size]`` argument specifies xe-file as an upgrade
  image with version ``id``. Each version number must be a unique number greater
  than 0.

You should now have the file ``new_firmware.bin`` which contains the
firmware with the newly specified Device Version Number.

For further details on the use of XFLASH to create factory and upgrade firmware
images please see the XFLASH Command-Line Manual section of the
`xTIMEcomposer User Guide <https://www.xmos.com/published/xtimecomposer-user-guide>`_.
