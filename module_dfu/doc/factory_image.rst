Installing the factory image to the device
==========================================

To rebuild the USB audio firmware with the DFU device interface edit the
device_defines.h file and comment in the #define DFU line.

Use the XMOS Development Tools to run the command:

  ``xflash --boot-partition-size 0x20000 usb_audio.xe``

This programs the factory default firmware image into the flash device. This
will add a new interface to the device that supports the DFU mechanism.

To use the firmware upgrade mechanism you need to build a firmware upgrade
image:

#. Edit the ``device_defines.h`` file and change the BCD_DEVICE number for the
   application.
#. Rebuild the application.

To generate the firmware upgrade image run the following command:

  ``xflash --upgrade 1 usb_audio.xe 0x20000 -o new_firmware.bin``

You should now have the file ``usb_audio_class1.bin`` which contains the
firmware for the audio class 1 implementation.
