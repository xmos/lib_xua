Overview
========

The DFU loader is a flash device firmware upgrade mechanism. To work correctly
your development board must contain the latest DFU enabled firmware.

The firmware upgrade for XMOS USB devices implementation uses the USB standard
DFU device class mechanism and is based on the following specification:
http://www.usb.org/developers/devclass_docs/DFU_1.1.pdf

Supported functionality:

- Download of new firmware to device
- Upload of existing firmware from device
- Revert device back to factory image
- Automatic reboot of device on firmware upgrade

You must use XMOS Development Tools version 10.4.1 (or later).

The DFU device on Windows requires the Theyscon USB Audio 2.0 Windows driver
version 1.13.3 or later.
