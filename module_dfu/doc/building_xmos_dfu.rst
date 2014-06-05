Building the loader–OSX
=======================

The loader is compiled using libusb, the code for the loader is contained in the
file xmosdfu.cpp

To build the loader
-------------------

  ``g++ -m32 -o xmosdfu -I. xmosdfu.cpp libusb-1.0.0.dylib``