Using the DFU loader - OS X (via the XMOS DFU loader)
=====================================================

Set up the image loader
-----------------------

#. Open a terminal
#. Change directory to where the files have been extracted
#. Source the ``setup.sh`` script

Download new firmware
---------------------

To program the new firmware run the command:

   ``./xmosdfu --download new_firmware.bin``

Note that once this is done the device restarts. The original factory default
application is still present but the device is now running the upgraded
application firmware.

Uploading existing firmware from the device
-------------------------------------------

You can retrieve a firmware image from the device, providing an upgrade image is
present.

Run the command:

  ``./xmosdfu --upload currentfirmware.bin``

The file ``currentfirmware.bin`` contains the latest upgrade image. This file is
an exact copy of the data from the flash and can be downloaded to the device
again to test.

Reverting firmware to factory image
-----------------------------------

To revert the device back to its factory (i.e XFLASH) installed state from the
new firmware, run the command:

  ``./xmosdfu --revertfactory``

The device will now only contain the factory firmware and will be running as an
audio 2 device again.

The device will now be running, and only contain the factory firmware.
