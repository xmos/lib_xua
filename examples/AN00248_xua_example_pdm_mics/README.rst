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

********
Overview
********

This applicaition note describes how to use ``lib_mic_array`` in conjunction with ``lib_xua``
to implement a USB Audio device with the ability to record from multiple PDM microphones.


**************
Required Tools
**************

  * XMOS XTC Tools: 15.3.0

*****************
Required hardware
*****************

The example code provided with the application has been implemented
and tested on the XK-EVK-XU316 board.

*********************************
Required Libraries (dependencies)
*********************************

  * `lib_xua <https://www.github.com/xmos/lib_xua>`_
  * `lib_adat <https://www.github.com/xmos/lib_adat>`_
  * `lib_locks <https://www.github.com/xmos/lib_locks>`_
  * `lib_logging <https://www.github.com/xmos/lib_logging>`_
  * `lib_mic_array <https://www.github.com/xmos/lib_mic_array>`_
  * `lib_xassert <https://www.github.com/xmos/lib_xassert>`_
  * `lib_xcore_math <https://www.github.com/xmos/lib_xcore_math>`_
  * `lib_spdif <https://www.github.com/xmos/lib_spdif>`_
  * `lib_sw_pll <https://www.github.com/xmos/lib_sw_pll>`_
  * `lib_xud <https://www.github.com/xmos/lib_xud>`_
  * `lib_board_support <https://www.github.com/xmos/lib_board_support>`_

*************************
Related Application Notes
*************************

 * None

*******
Support
*******

This package is supported by XMOS Ltd. Issues can be raised against the software at: http://www.xmos.com/support



