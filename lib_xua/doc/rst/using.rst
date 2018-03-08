Using lib_xud
-------------

This sections describes the basic usage of `lib_xud`. It provides a guide on how to program the USB Audio Devices using `lib_xud` including instructions for building and running
programs and creating your own custom USB audio applications.

Reviewing application note AN00246 is highly recommended.

Library structure
~~~~~~~~~~~~~~~~~

The code is split into several directories.

.. list-table:: lib_xua structure

 * - core
   - Common code for USB audio applications
 * - midi
   - MIDI I/O code
 * - dfu
   - Device Firmware Upgrade code


Note, the midi and dfu directories are potential canditates for separate libs in their own right.


Including in a project
~~~~~~~~~~~~~~~~~~~~~~~~~

All `lib_xua` functions can be accessed via the ``xud.h`` header filer::

  #include <xua.h>

It is also requited to to add ``lib_xua`` to the ``USED_MODULES`` field of your application Makefile.



