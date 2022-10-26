|newpage|

.. _usb_audio_sec_resource_usage:

Resource Usage
==============

The following table details the resource usage of each component of the reference design software.
Note, memory usage is approximate and varies based on device used, compiler settings etc.

.. table:: Resource Usage

 +---------------+---------------+---------------------+-------------------------------------+
 |   Component   |   Cores       |   Memory (KB)       |   Ports                             |
 +===============+===============+=====================+=====================================+
 | XUD library   |  1            | 9 (6 code)          | USB ports                           |
 |               |               |                     |                                     |
 +---------------+---------------+---------------------+-------------------------------------+
 | Endpoint 0    |  1            | 17.5 (10.5 code)    | none                                |
 +---------------+---------------+---------------------+-------------------------------------+
 | USB Buffering |  2            | 22.5 (1 code)       | 1 x n bit port                      |
 +---------------+---------------+---------------------+-------------------------------------+
 | Audio Hub     |  1            | 8.5 (6 code)        | See :ref:`usb_audio_sec_audio`      |
 +---------------+---------------+---------------------+-------------------------------------+
 | S/PDIF Tx     |  1            | 3.5 (2 code)        | 1 x 1 bit port                      |
 +---------------+---------------+---------------------+-------------------------------------+
 | S/PDIF Rx     |  1            | 3.7 (3.7 code)      | 1 x 1 bit port                      |
 +---------------+---------------+---------------------+-------------------------------------+
 | ADAT Rx       |  1            | 3.2 (3.2 code)      | 1 x 1 bit port                      |
 +---------------+---------------+---------------------+-------------------------------------+
 | MIDI          |  1            | 6.5 (1.5 code)      | 2 x 1 bit ports                     |
 +---------------+---------------+---------------------+-------------------------------------+
 | Mixer         |  (up to) 2    | 8.7 (6.5 code)      |                                     |
 +---------------+---------------+---------------------+-------------------------------------+
 | ClockGen      |  1            | 2.5 (2.4 code)      |                                     |
 +---------------+---------------+---------------------+-------------------------------------+

.. note::

    These resource estimates are based on the multichannel reference design with
    all options of that design enabled. For fewer channels, the resource
    usage is likely to decrease.

.. note::

    The XUD library requires an 80MIPS core to function correctly
    (i.e. on a 500MHz part only six cores can run).

.. note::

   Unlike other interfaces, since the USB PHY is internal the USB ports are a fixed set of ports
   and cannot be modified.  See ``lib_xud`` documentation for full details.

