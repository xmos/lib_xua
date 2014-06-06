Using the DFU loader - Windows (via the Thesycon driver)
========================================================

Set up the image loader
-----------------------

#. Open the DFU Console Window from the Thesycon folder on the Start menu
   (Program Files>Thesycon>USB Audio 2.0 Class Driver EVAL>DFU console).
#. Change directory to where the files have been extracted.

To check the device has been detected, run the following command in the DFU
console:

  ``dfucons info``

The console shows the DFU devices that have been detected.

Download new firmware
---------------------

To program the new firmware run the command:

  ``dfucons download new_firmware.bin``

Note that once this is done the device restarts. The original factory default
application is still present but the device is now running the upgraded
application firmware.

You can check the device has been updated by running the command:

  ``dfucons info``

This will display the device revision.

Uploading existing firmware from the device
-------------------------------------------

You can retrieve a firmware image from the device, providing an upgrade image is
present.
Run the command:

  ``dfucons upload currentfirmware.bin``

The file ``currentfirmware.bin`` contains the latest upgrade image. This file is
an exact copy of the data from the flash and can be downloaded to the device
again to test.

Reverting firmware to factory image
-----------------------------------

To revert the device back to its factory (i.e XFLASH) installed state from the
new firmware, run the command:

  ``dfucons revertfactory``

The device will now be running, and only contain the factory firmware, which can
be seen by checking the device version once more.
