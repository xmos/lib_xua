.. |I2S| replace:: I\ :sup:`2`\ S

Simple USB Audio Device using lib_xua
=====================================

Summary
-------

This application note demonstrates the use of the XMOS XUA library to
create a USB device that can play two channels of audio from the host out
via |I2S|. This is connected to a DAC and the audio can be heard on the
output jack.

Required hardware
.................

The example code provided with the application has been implemented
and tested on the xCORE.ai Multi-channel Audio Board

Prerequisites
.............

 * This document assumes familiarity with the XMOS xCORE architecture,
   the XMOS tool chain and the xC language. Documentation related to these
   aspects which are not specific to this application note are linked to in
   the references appendix.

