:orphan:

#####################################
lib_xua: USB Audio components library
#####################################

:vendor: XMOS
:version: 5.1.0
:scope: General Use
:description: USB Audio components library
:category: Audio
:keywords: USB Audio, I2S, MIDI, HID, DFU
:devices: xcore.ai, xcore-200

*******
Summary
*******

``lib_xua`` contains shared components for use in the XMOS USB Audio (XUA) Reference Designs.

These components enable the development of USB Audio devices on the `XMOS xcore` architecture.

.. tip::

    Reference design applications that use `lib_xua` are located in `sw_usb_audio` folder of the
    USB Audio reference design `download <https://www.xmos.com/develop/usb-multichannel-audio/>`_.
    This is a typical entry point for most developers.

********
Features
********

- USB Audio Class 1.0/2.0 compliant

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

************
Known issues
************

- When in DSD mode with S/PDIF output enabled, DSD samples are transmitted over S/PDIF if the DSD and S/PDIF channels are shared, this may or may not be desired (#14762)

- I2S input is completely disabled when DSD output is active - any input stream to the host will contain 0 samples (#14173)

- Operating the design at a sample rate of less than or equal to the SOF rate (i.e. 8kHz at HS, 1kHz at FS) may expose a corner case relating to 0 length packet handling in both the driver and device and should be considered unsupported at this time (#14780)

- Before DoP mode is detected a small number of DSD samples will be played out as PCM via I2S (lib_xua #162)

- Volume control settings currently affect samples in both DSD and PCM modes. This results in invalid DSD output if volume control not set to 0 (#14887)

- 88.2kHz and 176.4kHz sample frequencies are not exposed in Windows control panels.  These are known OS restrictions.

- When DFU flash access fails the device NAKS the host indefinitely (sw_usb_audio #54)

- In synchronous mode there is no nice transition of the reference signal when moving between internal and SOF clocks (lib_xua #275)

- Binary images exceeding FLASH_MAX_UPGRADE_SIZE fail silently on DFU download (lib_xua #165)

- Input does not come out of underflow for USB Audio Class 2 when sample rate is 16kHz and channel count is 2. (lib_xua #434). This will result in silence being streamed to the host. Please use USB Audio Class 1 for low channel count and sample rates.

****************
Development repo
****************

  * `lib_xua <https://www.github.com/xmos/lib_xua>`_

**************
Required tools
**************

  * XMOS XTC Tools: 15.3.0


************************
Host system requirements
************************

USB Audio devices built using `lib_xua` have the following host system requirements.

- Mac OSX version 10.6 or later

- Windows 10 or 11, with Thesycon Audio Class 2.0 driver for Windows (Tested against version 5.70.0). Please contact XMOS for details.

- Windows 10 or 11 with built-in USB Audio Class 1.0 driver.

- Windows 10 or 11 with built-in USB Audio Class 2.0 driver.

Older versions of Windows are not guaranteed to operate as expected. Devices are also expected to operate with various Linux distributions including mobile variants.

*********************************
Required libraries (dependencies)
*********************************

  * `lib_adat <https://www.xmos.com/file/lib_adat>`_
  * `lib_locks <https://www.xmos.com/file/lib_locks>`_
  * `lib_logging <https://www.xmos.com/file/lib_logging>`_
  * `lib_mic_array <https://www.xmos.com/file/lib_mic_array>`_
  * `lib_xassert <https://www.xmos.com/file/lib_xassert>`_
  * `lib_xcore_math <https://www.xmos.com/file/lib_xcore_math>`_
  * `lib_spdif <https://www.xmos.com/file/lib_spdif>`_
  * `lib_sw_pll <https://www.xmos.com/file/lib_sw_pll>`_
  * `lib_xud <https://www.xmos.com/file/lib_xud>`_

*************************
Related application notes
*************************

The following application notes use this library:

  * `AN02019: Using Device Firmware Upgrade (DFU) for USB Audio` <www.xmos.com/file/an02019>`_

*******
Support
*******

This package is supported by XMOS Ltd. Issues can be raised against the software at
`http://www.xmos.com/support <http://www.xmos.com/support>`_

