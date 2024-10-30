:orphan:

################################################
AN00247: Using lib_xua with lib_spdif (transmit)
################################################

:vendor: XMOS
:version: 1.0.0
:scope: Example
:description: Using lib_xua with lib_spdif (transmit)
:category: General Purpose
:keywords: USB Audio, S/PDIF
:hardware: XK-AUDIO-316-MC

*******
Summary
*******

This application note demonstrates the use of an S/PDIF transmitter with
the XMOS XUA library to create a USB device that can play two channels of
audio from the host out of the co-axial connector.

*****************
Required hardware
*****************

The example code provided with the application has been implemented
and tested on the xCORE.ai Multi-channel Audio Board

**************
Required tools
**************

  * XMOS XTC Tools: 15.3.0

*********************************
Required libraries (dependencies)
*********************************

  * `lib_xua <https://www.xmos.com/file/lib_xua>`_
  * `lib_adat <https://www.xmos.com/file/lib_adat>`_
  * `lib_locks <https://www.xmos.com/file/lib_locks>`_
  * `lib_logging <https://www.xmos.com/file/lib_logging>`_
  * `lib_mic_array <https://www.xmos.com/file/lib_mic_array>`_
  * `lib_xassert <https://www.xmos.com/file/lib_xassert>`_
  * `lib_xcore_math <https://www.xmos.com/file/lib_xcore_math>`_
  * `lib_spdif <https://www.xmos.com/file/lib_spdif>`_
  * `lib_sw_pll <https://www.xmos.com/file/lib_sw_pll>`_
  * `lib_xud <https://www.xmos.com/file/lib_xud>`_
  * `lib_board_support <https://www.xmos.com/file/lib_board_support>`_

*************************
Related application notes
*************************

 * None

*******
Support
*******

This package is supported by XMOS Ltd. Issues can be raised against the software at: http://www.xmos.com/support
