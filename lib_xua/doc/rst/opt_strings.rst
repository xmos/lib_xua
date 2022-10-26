
Strings and ID's
================

The codebase includes various strings and ID's that should be customised to match the product requirements. 
These are listed in ::ref:`opt_strings_defines`.

The Vendor ID (VID) should be acquired from the USB Implementers Forum (www.usb.org). Under no circumstances 
should the XMOS VID or any other VID be used without express permission.

The VID and Product ID (PID) pair must be unique to each product, otherwise driver incompatibilities may arise.

.. tabularcolumns:: lp{5cm}l

.. _opt_strings_defines:

.. list-table:: String & ID defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``VENDOR_STR``
     - Name of vendor/manufacturer, note the is appended to various strings. 
     - ``"XMOS"``
   * - ``PRODUCT_STR_A2``
     - Name of the product when running in Audio Class 2.0 mode
     - ``"XMOS xCORE (UAC2.0)"``
   * - ``PRODUCT_STR_A1``
     - Name of the product when running in Audio Class 1.0 mode
     - ``"XMOS xCORE (UAC1.0)"``
   * - ``PID_AUDIO_2``
     - Product ID when running in Audio Class 2.0 mode
     - ``0x0002``
   * - ``PID_AUDIO_1``
     - Product ID when running in Audio Class 1.0 mode
     - ``0x0003``



