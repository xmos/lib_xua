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

********
Overview
********

This application note demonstrates the use of an S/PDIF transmitter with
the XMOS XUA library to create a USB device that can play two channels of
audio from the host out of the co-axial connector.

*****************
Required hardware
*****************

The example code provided with the application has been implemented
and tested on the xCORE.ai Multi-channel Audio Board

**************
Required Tools
**************

  * XMOS XTC Tools: 15.3.0

*********************************
Required Libraries (dependencies)
*********************************

  * lib_adat (www.github.com/xmos/lib_adat)
  * lib_locks (www.github.com/xmos/lib_locks)
  * lib_logging (www.github.com/xmos/lib_logging)
  * lib_mic_array (www.github.com/xmos/lib_mic_array)
  * lib_xassert (www.github.com/xmos/lib_xassert)
  * lib_dsp (www.github.com/xmos/lib_dsp)
  * lib_spdif (www.github.com/xmos/lib_spdif)
  * lib_sw_pll (www.github.com/xmos/lib_sw_pll)
  * lib_xud (www.github.com/xmos/lib_xud)
  * lib_board_support (www.github.com/xmos/lib_board_support)

*************************
Related Application Notes
*************************

 * None

*******
Support
*******

This package is supported by XMOS Ltd. Issues can be raised against the software at: http://www.xmos.com/support
