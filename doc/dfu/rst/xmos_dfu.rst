Using the DFU loader - macOS (via the XMOS DFU loader)
======================================================

The XMOS DFU loader is provided as source as part of the XMOS USB Audio software
framework, see :ref:`usb_audiosec_building_xmos_dfu`.

NOTE: Windows requires the installation of libusbK drivers on the DFU endpoint.
We recommend using `Zadig <https://zadig.akeo.ie/>`_.

Set up the image loader
-----------------------

#. Open a terminal
#. Change directory to where the loader has been built
#. Point DYLD_LIBRARY_PATH to libusb/OSX64:

    ``export DYLD_LIBRARY_PATH=$PWD/libusb/OSX64:$DYLD_LIBRARY_PATH``

Download new firmware
---------------------

To program the new firmware run the command:

   ``./bin/xmosdfu XMOS_L2_AUDIO2_PID --download new_firmware.bin``

Replace ``XMOS_L2_AUDIO2_PID`` with product ID of your target device. Invoke
``xmosdfu`` with no arguments to get a list of all supported product IDs.

Note that once this is done the device restarts. The original factory default
application is still present but the device is now running the upgraded
application firmware.

Uploading existing firmware from the device
-------------------------------------------

You can retrieve a firmware image from the device, providing an upgrade image is
present.

Run the command:

  ``./bin/xmosdfu XMOS_L2_AUDIO2_PID --upload currentfirmware.bin``

The file ``currentfirmware.bin`` contains the latest upgrade image. This file is
an exact copy of the data from the flash and can be downloaded to the device
again to test.

Reverting firmware to factory image
-----------------------------------

To revert the device back to its factory (i.e XFLASH) installed state from the
new firmware, run the command:

  ``./bin/xmosdfu XMOS_L2_AUDIO2_PID --revertfactory``

The device will now be running, and only contain the factory firmware.
