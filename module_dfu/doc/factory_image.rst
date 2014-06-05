Installing the factory image to the device
==========================================

The DFU device interface is enabled by default in the XMOS USB Audio framework
(see ``devicedefines.h``), and the explicitly enabled in each reference design
by the following line included in the ``customdefines.h`` file of each
application::

  #define DFU                (1)

Use the XMOS Development Tools to run the command:

  ``xflash --boot-partition-size 0x20000 usb_audio.xe``

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

  ``xflash --upgrade 1 usb_audio.xe 0x20000 -o new_firmware.bin``

You should now have the file ``new_firmware.bin`` which contains the
firmware with the newly specified Device Version Number.
