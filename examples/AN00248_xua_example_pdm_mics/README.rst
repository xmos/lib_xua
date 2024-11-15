:orphan:

#########################################
AN00248: Using lib_xua with lib_mic_array
#########################################

:vendor: XMOS
:version: 1.0.0
:scope: Example
:description: Using lib_xua with lib_mic_array
:category: General Purpose
:keywords: USB Audio, PDM microphones
:hardware: XK-EVK-XU316

*******
Summary
*******

This application note describes how to use ``lib_mic_array`` in conjunction with ``lib_xua``
to implement a USB Audio device with the ability to record from multiple PDM microphones.


**************
Required tools
**************

  * XMOS XTC Tools: 15.3.0

*****************
Required hardware
*****************

The example code provided with the application has been implemented
and tested on the XK-EVK-XU316 board.

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



