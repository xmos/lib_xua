lib_xua
=======

:Latest release: 3.2.0rc1


:Scope: General Use

Summary
-------

USB Audio Shared Components for use in the XMOS USB Audio Refererence Designs.

This library enables the development of USB Audio devices on the XMOS xCORE architecture.

Features
........

Key features of the various applications in this repository are as follows

- USB Audio Class 1.0/2.0 Compliant

- Fully Asynchronous operation

- Support for the following sample frequencies: 8, 11.025, 12, 16, 32, 44.1, 48, 88.2, 96, 176.4, 192, 352.8, 384kHz

- Input/output channel and individual volume/mute controls supported

- Support for dynamically selectable output audio formats (e.g. resolution)

- Field firmware upgrade compliant to the USB Device Firmware Upgrade (DFU) Class Specification

- S/PDIF output

- S/PDIF input

- ADAT output

- ADAT input

- Synchronisation to external digital streams i.e. S/PDIF or ADAT

- I2S slave & master modes

- TDM slave & master modes

- MIDI input/output (Compliant to USB Class Specification for MIDI devices)

- DSD output (Native and DoP mode) at DSD64 and DSD128 rates

- Mixer with flexible routing

- Simple playback controls via Human Interface Device (HID)

- Support for operation with Apple devices (requires software module sc_mfi for MFI licensees only - please contact XMOS)

Note, not all features may be supported at all sample frequencies, simultaneously or on all devices.  Some features also require specific host driver support.

Software version and dependencies
.................................

The CHANGELOG contains information about the current and previous versions.
For a list of direct dependencies, look for DEPENDENT_MODULES in lib_xua/module_build_info.

Related Application Notes
.........................

The following application notes use this library:

    * AN000246 - Simple USB Audio Device using lib_xua
    * AN000247 - Using lib_xua with lib_spdif (transmit)
    * AN000248 - Using lib_xua with lib_mic_array

Required software (dependencies)
================================

  * lib_locks (git@github.com:xmos/lib_locks.git)
  * lib_logging (git@github.com:xmos/lib_logging)
  * lib_mic_array (git@github.com:xmos/lib_mic_array.git)
  * lib_xassert (git@github.com:xmos/lib_xassert)
  * lib_dsp (git@github.com:xmos/lib_dsp)
  * lib_spdif (git@github.com:xmos/lib_spdif)
  * lib_xud (git@github.com:xmos/lib_xud)

