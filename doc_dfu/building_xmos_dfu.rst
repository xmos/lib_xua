.. _usb_audiosec_building_xmos_dfu:

Building the XMOS DFU loader - macOS
====================================

The XMOS DFU loader is provided as source as part of the USB Audio
framework, located in lib_xua/host/xmosdfu.

The loader is compiled using libusb, the code for the loader is contained in the
file ``xmosdfu.cpp``

To build the loader a Makefile is provided, which can be run as follows:

  ``make -f Makefile.OSX all``

This Makefile contains the following:

.. literalinclude:: Makefile.OSX
