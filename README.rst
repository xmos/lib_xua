lib_xua
=======

:Latest release: 3.3.0


:Scope: General Use

Summary
-------

lib_xua contains shared components for use in the XMOS USB Audio (XUA) Reference Designs.

These components enable the development of USB Audio devices on the XMOS xCORE architecture.

Features
~~~~~~~~

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

- I2S slave & master modes

- TDM slave & master modes

- MIDI input/output (Compliant to USB Class Specification for MIDI devices)

- DSD output ("native" and DoP mode) at DSD64 and DSD128 rates

- Mixer with flexible routing

- Simple playback controls via USB Human Interface Device (HID) Class

Note, not all features may be supported at all sample frequencies, simultaneously or on all devices.  
Some features may also require specific host driver support.

Host System Requirements
~~~~~~~~~~~~~~~~~~~~~~~~

USB Audio devices built using `lib_xua` have the following host system requirements.

- Mac OSX version 10.6 or later

- Windows Vista, 7, 8 or 10 with Thesycon Audio Class 2.0 driver for Windows (Tested against version 3.20). Please contact XMOS for details.
 
- Windows Vista, 7, 8 or 10 with built-in USB Audio Class 1.0 driver.

Older versions of Windows are not guaranteed to operate as expected. Devices are also expected to operate with various Linux distributions including mobile variants.

Related Application Notes
~~~~~~~~~~~~~~~~~~~~~~~~~

The following application notes use this library:

    * AN000246 - Simple USB Audio Device using lib_xua
    * AN000247 - Using lib_xua with lib_spdif (transmit)
    * AN000248 - Using lib_xua with lib_mic_array

Required software (dependencies)
================================

  * lib_locks (git@github.com:xmos/lib_locks.git)
  * lib_logging (git@github.com:xmos/lib_logging.git)
  * lib_mic_array (git@github.com:xmos/lib_mic_array.git)
  * lib_xassert (git@github.com:xmos/lib_xassert.git)
  * lib_dsp (git@github.com:xmos/lib_dsp)
  * lib_i2c (git@github.com:xmos/lib_i2c.git)
  * lib_i2s (git@github.com:xmos/lib_i2s.git)
  * lib_gpio (git@github.com:xmos/lib_gpio.git)
  * lib_mic_array_board_support (git@github.com:xmos/lib_mic_array_board_support.git)
  * lib_spdif (git@github.com:xmos/lib_spdif.git)
  * lib_xud (git@github.com:xmos/lib_xud.git)
  * lib_adat (git@github.com:xmos/lib_adat)

