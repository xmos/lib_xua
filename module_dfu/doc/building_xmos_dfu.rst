.. _usb_audiosec_building_xmos_dfu:

Building the XMOS DFU loader - OS X
===================================

The XMOS DFU loader is provided as source as part of the USB Audio
framework, located in sc_usb_audio/module_dfu/host/xmos_dfu_osx.

The loader is compiled using libusb, the code for the loader is contained in the
file ``xmosdfu.cpp``

To build the loader a Makefile is provided, which can be run as follows:

  ``make -f Makefile.OSX all``

This Makefile contains the following:

.. literalinclude:: Makefile.OSX
