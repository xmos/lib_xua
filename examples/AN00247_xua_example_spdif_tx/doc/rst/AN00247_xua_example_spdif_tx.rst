.. include:: ../../README.rst

|newpage|

Overview
--------

Introduction
............

The XMOS USB Audio (XUA) library provides an implementation of USB Audio Class versions 1.0 and 2.0.

This application note demonstrates the implementation of a basic USB Audio Device with 
S/PDIF transmit functionality the xCORE.ai Multichannel (MC) Audio board.

To reduce complexity this application note does not enable any other audio interfaces other that S/PDIF transmit 
(i.e. no I2S). Readers are encouraged to read application note AN00246 in conjunction with this application
note.


The Makefile
------------

To start using ``lib_xua``, you need to add ``lib_xua`` and ``lib_spdif`` to your Makefile::

  USED_MODULES = .. lib_xua lib_spdif ...

This demo also uses the XMOS USB Device library (``lib_xud``) for low-level USB connectivity.
The Makefile also includes::

  USED_MODULES = .. lib_xud ..

``lib_xud`` library requires some flags for correct operation. Namely the 
tile on which ``lib_xud`` will be execute, for example::

    XCC_FLAGS = .. -DUSB_TILE=tile[0] ..


Includes
--------

This application requires the system header that defines XMOS xCORE specific
defines for declaring and initialising hardware:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: include <xs1.h>
   :end-before: include "xua.h"

The XUA library functions are defined in ``xua.h``. This header must
be included in your code to use the library. 

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: include "xua.h"
   :end-on: include "xud_device.h"

The application uses the S/PDIF transmitter from ``lib_spdif``. This header
must be included in your code.

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: /* From lib_spdif 
   :end-on: include "spdif.h"

Declarations
------------

Allocating hardware resources for lib_xua
.........................................

A minimal implementation of a USB Audio device, without I2S functionality,
using ``lib_xua`` requires the follow pins:

    - Audio Master clock (from clock source to xCORE)

On an xCORE the pins are controlled by ``ports``. The application therefore declares a 
port for the master clock input signal.

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: /* Lib_xua port declaration
   :end-on: in port p_mclk_in

``lib_xua`` also requires two ports for internally calculating USB feedback. Please refer to 
the ``lib_xua`` library documentation for further details.  The additional input port for the master
clock is required since USB and S/PDIF do not reside of the same tiles on the example hardware.

These ports are declared as follows:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: /* Resources for USB feedback
   :end-on: in port p_mclk_in_usb

In addition to ``port`` resources two clock-block resources are also required:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: /* Clock-block
   :end-on: clock clk_audio_mclk_usb

Again, for the same reasoning as the master-clock ports, two master-clock clock-blocks are required
- one on each tile.


Allocating hardware resources for lib_spdif
...........................................

The S/PDIF transmitter requires a single (buffered) 1-bit port:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: /* Lib_spdif port 
   :end-on: buffered out port 

This port must be clocked from the audio master clock. This application note chooses to declare
an extra clock-block as follows:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: clock clk_spdif_tx
   :end-before: /* Lib_xua


Other declarations
..................

``lib_xua`` currently requires the manual declaration of tables for the endpoint types for
``lib_xud`` and the calling the main XUD function in a par (``XUD_Main()``).

For a simple application the following endpoints are required:

    - ``Control`` endpoint zero 
    - ``Isochonous`` endpoint for each direction for audio data to/from the USB host

These are declared as follows:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: /* Endpoint type tables
   :end-on: XUD_EpType epTypeTableIn

Configuring lib_xua
-------------------

``lib_xua`` must be configured to enable S/PDIF Tx functionality. 

``lib_xua`` has many parameters than can be configured at build time, some examples include:

    - Sample-rates
    - Channel counts
    - Audio Class version
    - Product/Vendor ID's
    - Various product strings
    - Master clock frequency

To enable S/PDIF functionality ``XUA_SPDIF_TX_EN`` must be set to a non-zero value. Setting this will cause the ``XUA_AudioHub``
tasks to forward samples and sample rate information to the S/PDIF transmitter task. 

These parameters are set via defines in an optional ``xua_conf.h`` header file. For this simple application the 
complete contents of this file are as follows:

.. literalinclude:: xua_conf.h
   :start-on: // Copyright
   :end-on: #endif

The application main() function
-------------------------------

The ``main()`` function sets up the tasks in the application.

Various channels are required in order to allow the required tasks to communicate. 
These must first be declared:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: /* Channels for lib_xud
   :end-on: chan c_spdif_tx

The rest of the ``main()`` function starts all of the tasks in parallel
using the xC ``par`` construct:

.. literalinclude:: app_xua_spdiftx.xc
   :start-on: par
   :end-before: return 0

This code starts the low-level USB task, an Endpoint 0 task, an Audio buffering task and a task to handle 
the audio I/O. Note, since there is no I2S functionality in this example this task simply forwards samples to the 
SPDIF transmitter task. In addition the ``spdif_tx()`` task is also run.

Note that the ``spdif_tx_port_config()`` function is called before a nested ``par`` of ``spdif_tx()`` and ``XUA_AudioHub()``.
This is because of the "shared" nature of ``p_mclk_in`` and avoids a parallel usage check failure by the XMOS tool-chain.

|appendix|
|newpage|

Demo Hardware Setup
-------------------

To run the demo, use a USB cable to connect the on-board xTAG debug adapter (marked DEBUG) to your development computer. 
Use another USB cable to connect the USB receptacle marked USB DEVICE to the device you wish to play audio from. 

A device capable of receiving an S/PDIF signal (ie. a speaker) should be connected to COAX TX. 

.. figure:: images/hw_setup.*
   :width: 80%

   Hardware setup

|newpage|

Launching the demo application
------------------------------

Once the demo example has been built either from the command line using xmake or
via the build mechanism of xTIMEcomposer studio it can be executed on the xCORE.ai
MC Audio board.

Once built there will be a ``bin/`` directory within the project which contains
the binary for the xCORE device. The xCORE binary has a XMOS standard .xe extension.

Launching from the command line
...............................

From the command line you use the ``xrun`` tool to download and run the code
on the xCORE device::

  xrun --xscope bin/app_xua_simple.xe

Once this command has executed the application will be running on the
xCORE.ai MC Audio Board

Launching from xTIMEcomposer Studio
...................................

From xTIMEcomposer Studio use the run mechanism to download code to xCORE device.
Select the xCORE binary from the ``bin/`` directory, right click and go to Run
Configurations. Double click on xCORE application to create a new run configuration,
enable the xSCOPE I/O mode in the dialog box and then
select Run.

Once this command has executed the application will be running on the
xCORE.ai MC Audio board.

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

.. literalinclude:: app_xua_spdiftx.xc
  :largelisting:

|newpage|
