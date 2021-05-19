
Using lib_xua with lib_mic_array
================================

Summary
-------

This applicaition note describes how to use ``lib_mic_array`` in conjunction with ``lib_xua``
to implement a USB Audio device with the ability to record from multiple PDM microphones.

Software dependencies
.....................

For a list of direct dependencies, look for USED_MODULES in the Makefile.

Required hardware
.................

The example code provided with the application has been implemented
and tested on the xCORE-200 Array Microphone board.

Prerequisites
.............

 * This document assumes familiarity with the XMOS xCORE architecture,
   the XMOS tool chain and the xC language. Documentation related to these
   aspects which are not specific to this application note are linked to in
   the references appendix.

 * For a description of XMOS related terms found in this document
   please see the XMOS Glossary [#]_.

.. [#] http://www.xmos.com/published/glossary


