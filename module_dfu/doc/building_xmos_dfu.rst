Building the XMOS DFU loader - OS X
===================================

The loader is compiled using libusb, the code for the loader is contained in the
file ``xmosdfu.cpp``

To build the loader a Makefile is provided, which can be run as follows:

  ``make -f Makefile.OSX all``

This Makefile contains the following:

.. literalinclude:: Makefile.OSX
