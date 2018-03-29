.. _usb_audiosec_building_xmos_dfu:

Building the XMOS DFU loader
============================

The XMOS DFU loader is provided as source as part of the USB Audio
framework, located in lib_xua/host/xmosdfu.

The loader is compiled using libusb, the code for the loader is contained in the
file ``xmosdfu.cpp``

macOS
-----

To build the loader a Makefile is provided, which can be run as follows:

  ``make -f Makefile.OSX64 all``

This Makefile contains the following:

.. literalinclude:: Makefile.OSX64

There is also a 32bit OS X makefile, ``Makefile.OSX32``.

Linux
-----

Similarly to macOS, there are two Linux makefiles provided, ``Makefile.Linux32`` and ``Makefile.Linux64``:

.. literalinclude:: Makefile.Linux64

System-wide libusb is used. On Debian-derived systems this can be installed with:

  ``apt-get install libusb-1.0.0-dev``

Raspberry Pi
------------

A makefile is provided for Raspbian. libusb is required and can be installed using the ``apt-get`` command from previous Linux section.

.. literalinclude:: Makefile.Pi
