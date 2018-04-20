
.. include:: ../../../README.rst


About This Document
-------------------

This document describes the structure of the library, its use and resources required. It also covers some implementation detail.

This document assumes familiarity with the XMOS xCORE architecture, the Universal Serial Bus 2.0 Specification (and related specifications),
the XMOS tool chain and XC language.


Host System Requirements
------------------------

USB Audio devices built using `lib_xua` have the following host system requirements.

- Mac OSX version 10.6 or later

- Windows Vista, 7, 8 or 10 with Thesycon Audio Class 2.0 driver for Windows (Tested against version 3.20). Please contact XMOS for details.
 
- Windows Vista, 7, 8 or 10 with built-in USB Audio Class 1.0 driver.

Older versions of Windows are not guaranteed to operate as expected. Devices are also expected to operate with various Linux distributions including mobile variants.

.. toctree::

    Overview <overview>
    Hardware Platforms <hw>
    Software Overview <sw>
    Using lib_xua <using>
    Features <feat>
    Software Detail <sw_detail>
    Known Issues <issues>



.. include:: ../../../CHANGELOG.rst
