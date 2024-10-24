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

  * `lib_xua <www.github.com/xmos/lib_adat>`_
  * `lib_adat <www.github.com/xmos/lib_adat>`_
  * `lib_locks <www.github.com/xmos/lib_locks>`_
  * `lib_logging <www.github.com/xmos/lib_logging>`_
  * `lib_mic_array <www.github.com/xmos/lib_mic_array>`_
  * `lib_xassert <www.github.com/xmos/lib_xassert>`_
  * `lib_dsp <www.github.com/xmos/lib_dsp>`_
  * `lib_spdif <www.github.com/xmos/lib_spdif>`_
  * `lib_sw_pll <www.github.com/xmos/lib_sw_pll>`_
  * `lib_xud <www.github.com/xmos/lib_xud>`_
  * `lib_board_support <www.github.com/xmos/lib_board_support>`_

*************************
Related Application Notes
*************************

 * None

*******
Support
*******

This package is supported by XMOS Ltd. Issues can be raised against the software at: http://www.xmos.com/support



