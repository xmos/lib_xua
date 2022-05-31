.. include:: ../../README.rst

|newpage|

Overview
--------

Introduction
............

The XMOS USB Audio (XUA) library provides an implemention of USB Audio Class versions 1.0 and 2.0.

This application note demonstrates the implementation of a basic USB Audio Device on 
the xCORE-200 MC Audio board.


The Makefile
------------

To start using ``lib_xua``, you need to add ``lib_xua`` to your Makefile::

  USED_MODULES = .. lib_xua ...

This demo also uses the XMOS USB Device library (``lib_xud``) for low-level USB connectivity.
The Makefile also includes::

  USED_MODULES = .. lib_xud ..

``lib_xud`` library requires some flags for correct operation. Firstly the 
tile on which ``lib_xud`` will be execute, for example::

    XCC_FLAGS = .. -DUSB_TILE=tile[1] ..

Secondly, the architecture of the target device, for example::

  XCC_FLAGS = .. -DXUD_SERIES_SUPPORT=XUD_X200_SERIES ..

Includes
........

This application requires the system header that defines XMOS xCORE specific
defines for declaring and initialising hardware:

.. literalinclude:: app_xua_simple.xc
   :start-on: include <xs1.h>
   :end-before: include "xua.h"

The XUA library functions are defined in ``xua.h``. This header must
be included in your code to use the library. 

.. literalinclude:: app_xua_simple.xc
   :start-on: include "xua.h"
   :end-on: include "xud_device.h"

Allocating hardware resources
.............................

A basic implementation of a USB Audio device (i.e. simple stereo input and output via I2S)
using ``lib_xua`` requires the follow pins:

    - I2S Bit Clock (from xCORE to DAC)
    - I2S L/R clock (from xCORE to DAC)
    - I2S Data line (from xCORE to DAC)
    - I2S Data line (from ADC to xCORE)
    - Audio Master clock (from clock source to xCORE)

.. note::

    This application note assumes xCORE is I2S bus master

On an xCORE the pins are controlled by ``ports``. The application therefore declares various ``ports``
for this purpose:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Port declaration
   :end-on: in port p_mclk_in

``lib_xua`` also requires two ports for internally calculating USB feedback. Please refer to 
the ``lib_xua`` library documentation for further details.  The additonal input port for the master
clock is required since USB and S/PDIF do not reside of the same tiles on the example hardware.

These ports are declared as follows:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Resources for USB feedback
   :end-on: in port p_mclk_in_usb

In addition to ``port`` resources two clock-block resources are also required:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Clock-block
   :end-on: clock clk_audio_mclk_usb

Again, for the same reasoning as the master-clock ports, two master-clock clock-blocks are required
- one on each tile.


Other declarations
..................

``lib_xua`` currently requires the manual declaration of tables for the endpoint types for
``lib_xud`` and the calling the main XUD funtion in a par (``XUD_Main()``).

For a simple application the following endpoints are required:

    - ``Control`` enpoint zero 
    - ``Isochonous`` endpoint for each direction for audio data to/from the USB host

These are declared as follows:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Endpoint type tables
   :end-on: XUD_EpType epTypeTableIn

The application main() function
-------------------------------

The ``main()`` function sets up the tasks in the application.

Various channels are required in order to allow the required tasks to communcate. 
These must first be declared:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Channels for lib_xud
   :end-on: chan c_aud_ctl

The rest of the ``main()`` function starts all of the tasks in parallel
using the xC ``par`` construct:

.. literalinclude:: app_xua_simple.xc
   :start-on: par
   :end-before: return 0

This code starts the low-level USB task, an Endpoint 0 task, an Audio buffering task and a task to handle 
the audio I/O (i.e. I2S signalling).

Configuration 
.............

``lib_xua`` has many parameters than can be configured at build time, some examples include:

    - Sample-rates
    - Channel counts
    - Audio Class version
    - Product/Vendor ID's
    - Various product strings
    - Master clock frequency

These parameters are set via defines in an optional ``xua_conf.h`` header file. For this simple application the contents 
of this file might look something like the following:

.. literalinclude:: xua_conf.h
   :start-on: // Copyright
   :end-on: #endif

Some items have sensible default values, items like strings and sample rates for example. However, some items are specific to a hardware 
implentation e.g. master clock frequencies and must be defined.  Please see the ``lib_xua`` library documentation for full details.

|appendix|
|newpage|

Demo Hardware Setup
-------------------

To run the demo, connect a USB cable to power the xCORE-200 MC Audio board 
and plug the xTAG to the board and connect the xTAG USB cable to your
development machine.

.. figure:: images/hw_setup.*
   :width: 80%

   Hardware setup

|newpage|

Launching the demo application
------------------------------

Once the demo example has been built either from the command line using xmake or
via the build mechanism of xTIMEcomposer studio it can be executed on the xCORE-200
MC Audio board.

Once built there will be a ``bin/`` directory within the project which contains
the binary for the xCORE device. The xCORE binary has a XMOS standard .xe extension.

Launching from the command line
...............................

From the command line you use the ``xrun`` tool to download and run the code
on the xCORE device::

  xrun --xscope bin/app_xua_simple.xe

Once this command has executed the application will be running on the
xCORE-200 MC Audio Board

Launching from xTIMEcomposer Studio
...................................

From xTIMEcomposer Studio use the run mechanism to download code to xCORE device.
Select the xCORE binary from the ``bin/`` directory, right click and go to Run
Configurations. Double click on xCORE application to create a new run configuration,
enable the xSCOPE I/O mode in the dialog box and then
select Run.

Once this command has executed the application will be running on the
xCORE-200 MC Audio board.

Running the application
.......................

Once running the device will be detected as a USB Audio device - note, Windows operating 
systems may require a third party driver for correct operation 

|newpage|

References
----------

.. nopoints::

  * XMOS Tools User Guide

    http://www.xmos.com/published/xtimecomposer-user-guide

  * XMOS xCORE Programming Guide

    http://www.xmos.com/published/xmos-programming-guide

  * XMOS lib_xua Library

    http://www.xmos.com/support/libraries/lib_xua
    
  * XMOS lib_xud Library

    http://www.xmos.com/support/libraries/lib_xud

|newpage|

Full source code listing
------------------------

Source code for main.xc
.......................

.. literalinclude:: app_xua_simple.xc
  :largelisting:

|newpage|
