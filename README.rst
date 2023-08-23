lib_xua
#######

:Version: 3.5.1
:Vendor: XMOS


:Scope: General Use

Summary
*******

lib_xua contains shared components for use in the XMOS USB Audio (XUA) Reference Designs.

These components enable the development of USB Audio devices on the XMOS xCORE architecture.

Features
========

Key features of the various components in this repository are as follows

- USB Audio Class 1.0/2.0 Compliant

- Fully Asynchronous operation (synchronous mode as an option)

- Support for the following sample frequencies: 8, 11.025, 12, 16, 32, 44.1, 48, 88.2, 96, 176.4, 192, 352.8, 384kHz

- Volume/mute controls for input/output (for both master and individual channels)

- Support for dynamically selectable output audio formats (e.g. resolution)

- Field firmware upgrade compliant to the USB Device Firmware Upgrade (DFU) Class Specification

- S/PDIF output

- S/PDIF input

- ADAT output

- ADAT input

- Synchronisation to external digital streams i.e. S/PDIF or ADAT (when in asynchronous mode)

- I2S (slave/master modes with configurable word-length)

- TDM (slave/master modes with configurable word-length)

- MIDI input/output (Compliant to USB Class Specification for MIDI devices)

- DSD output ("native" and DoP mode) at DSD64 and DSD128 rates

- Mixer with flexible routing

- Simple playback controls via USB Human Interface Device (HID) Class

- Support for adding custom HID interfaces

Note, not all features may be supported at all sample frequencies, simultaneously or on all devices.  
Some features may also require specific host driver support.

Host System Requirements
========================

USB Audio devices built using `lib_xua` have the following host system requirements.

- Mac OSX version 10.6 or later

- Windows Vista, 7, 8 or 10 with Thesycon Audio Class 2.0 driver for Windows (Tested against version 3.20). Please contact XMOS for details.
 
- Windows Vista, 7, 8 or 10 with built-in USB Audio Class 1.0 driver.

Older versions of Windows are not guaranteed to operate as expected. Devices are also expected to operate with various Linux distributions including mobile variants.

Related Application Notes
=========================

The following application notes use this library:

  * AN000246 - Simple USB Audio Device using lib_xua
  * AN000247 - Using lib_xua with lib_spdif (transmit)
  * AN000248 - Using lib_xua with lib_mic_array

Required Software (dependencies)
================================

  * lib_locks (www.github.com/xmos/lib_locks)
  * lib_logging (www.github.com/xmos/lib_logging)
  * lib_mic_array (www.github.com/xmos/lib_mic_array)
  * lib_xassert (www.github.com/xmos/lib_xassert)
  * lib_dsp (www.github.com/xmos/lib_dsp)
  * lib_spdif (www.github.com/xmos/lib_spdif)
  * lib_xud (www.github.com/xmos/lib_xud)
  * lib_adat (www.github.com/xmos/lib_adat)

Documentation
=============

You can find the documentation for this software in the /doc directory of the package.

Support
=======

This package is supported by XMOS Ltd. Issues can be raised against the software at: http://www.xmos.com/support

