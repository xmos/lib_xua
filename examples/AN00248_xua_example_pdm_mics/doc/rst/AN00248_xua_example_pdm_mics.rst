.. include:: ../../README.rst

|newpage|

Overview
--------

Introduction
............

The XMOS USB Audio (XUA) library provides an implementation of USB Audio Class versions 1.0 and 2.0.

This application note demonstrates the implementation of a basic USB Audio Device with 
record functionality from PDM microphones on the xCORE-200 Array Microphone board.

Core PDM microphone functionality is contained in` ``lib_mic_array``. This library includes both the physical
interfacing to the PDM microphones as well as efficient decimation to user selectable output
sample rates - essentially providing PDM to PCM conversion.

To reduce complexity this application note does not enable any other audio interfaces other than recording 
from PDM microphones (i.e. no I2S and the on board DAC is not configured. 

Readers are encouraged to read application note AN00246 in conjunction with this application note.


The Makefile
------------

To start using ``lib_xua``, you need to add ``lib_xua`` to the Makefile. ``lib_mic_array`` should also be 
added for this application example::

  USED_MODULES = .. lib_xua lib_mic_array ...

This demo also uses the XMOS USB Device library (``lib_xud``) for low-level USB connectivity.
The Makefile therefore also includes this lib::

  USED_MODULES = .. lib_xud ..

``lib_xud`` library requires some flags for correct operation. Namely the 
tile on which ``lib_xud`` will be executed, for example::

    XCC_FLAGS = .. -DUSB_TILE=tile[1] ..

Includes
--------

This application requires the system header files that contains XMOS xCORE specific
defines for declaring and initialising hardware:

.. literalinclude:: app_xua_simple.xc
   :start-on: include <xs1.h>
   :end-before: include "xua.h"

The XUA and XUD library functions are defined in header files ``xua.h`` and ``xud_device.h`` respectively. These headers must
be included in the code in order to use these libraries. 

.. literalinclude:: app_xua_simple.xc
   :start-on: include "xua.h"
   :end-on: include "xud_device.h"

The application uses PDM interfacing and decimation code from ``lib_mic_array``. This header
must be included in the code.

.. literalinclude:: app_xua_simple.xc
   :start-on: /* From lib_mic
   :end-on: include "mic_array.h"

Declarations
------------

Allocating hardware resources for lib_xua
.........................................

A minimal implementation of a USB Audio device using ``lib_xua``,  without I2S functionality,
requires the follow I/O pins:

    - Audio Master clock (from clock source to xCORE)

On an xCORE the pins are controlled by ``ports``. The application therefore declares a 
port for the master clock input signal.

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Lib_xua port declaration
   :end-on: in port p_mclk_in

``lib_xua`` also requires two ports for internally calculating USB feedback. Please refer to 
the ``lib_xua`` library documentation for further details.  In this example ``XUA_Buffer()`` and ``XUA_AudioHub()``
reside on the same tile and can therefore make use of the same master-clock port.

These ports are declared as follows:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Resources for USB feedback
   :end-on: in port p_for

In addition to ``port`` resources a single clock-block resource is also required:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Clock-block declarations
   :end-on: clock clk_audio_mclk

Again, for the same reasoning as the master-clock ports, only one master-clock clock-blocks is required.


Allocating hardware resources for lib_mic_array
...............................................

``lib_mic_array`` requires a single 8-bit port for PDM data from up to 8 microphones. This port must be declared
as 32-bit buffered:

.. literalinclude:: app_xua_simple.xc
   :start-on: in buffered port:32 p_pdm_mics
   :end-before: clock clk_pdm

The microphones must be clocked by an audio related clock - typically 3.072MHz.

The xCORE-200 Array Microphone Board expects the xCORE to divide down the audio master clock input (24.576MHz)
and output the result to the microphones.

Two ports for this purpose are declared as follows:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Lib_mic_array declarations
   :end-before: in buffered


Please see the ``lib_mic_array`` library documentation for full details.


Other declarations
..................

``lib_xua`` currently requires the manual declaration of tables for the endpoint types for
``lib_xud`` and the calling the main XUD function in a par (``XUD_Main()``).

For a simple application the following endpoints are required:

    - ``Control`` endpoint zero 
    - ``Isochonous`` endpoint for each direction for audio data to/from the USB host

These are declared as follows:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Endpoint type tables
   :end-on: XUD_EpType epTypeTableIn

Configuring lib_xua
-------------------

``lib_xua`` must be configured to enable support for PDM microphones.

``lib_xua`` has many parameters than can be configured at build time, some examples include:

    - Supported sample-rates
    - Channel counts
    - Audio Class version
    - Product/Vendor ID's
    - Various product strings
    - Master clock frequency

To enable PDM microphone support  ``XUA_NUM_PDM_MICS`` must be set to a non-zero value.  Setting this will cause the ``XUA_AudioHub``
task to forward sample rate information and receive samples from the relevant microphone related tasks. 

These parameters are set via defines in an optional ``xua_conf.h`` header file. For this simple application the 
complete contents of this file are as follows:

.. literalinclude:: xua_conf.h
   :start-on: // Copyright
   :end-on: #endif

The application main() function
-------------------------------

The ``main()`` function sets up and runs the tasks in the application.

Channel declarations
....................

Various channels are required in order to allow the required tasks to communicate. 
These must first be declared:

.. literalinclude:: app_xua_simple.xc
   :start-on: /* Channels for lib_xud
   :end-on: chan c_mic_pcm

Standard ``lib_xua`` tasks
..........................

The rest of the ``main()`` function starts all of the tasks in parallel
using the xC ``par`` construct.

Firstly the standard ``lib_xua`` tasks are run on tile 1:  

.. literalinclude:: app_xua_simple.xc
   :start-on: par
   :end-before: on tile[0]

This code starts the low-level USB task, an Endpoint 0 task, an Audio buffering task and a task to handle 
the audio I/O (``XUA_AudioHub``).

Note, since there is no I2S functionality in this example the ``XUA_AudioHub`` task essentially just receives 
samples from the PDM buffer task and forwards samples to the ``XUA_Buffer`` task for forwarding to the USB host. 

Microphone related tasks
........................

Microphone related tasks are executed on tile 0 as follows:

.. literalinclude:: app_xua_simple.xc
   :start-on: Microphone related tasks
   :end-before: return 0

Two functions from ``lib_mic_array`` are used - a PDM receiver task (``mic_array_pdm_rx()``) and a decimation task (``mic_array_decimate_to_pcm_4ch()``).

Each call to ``mic_array_decimate_to_pcm_4ch()`` can handle the decimation of up to 4 microphone signals. Since the xCORE-200 Array microphone
board is equipped with seven microphones two instances of this task are run.

The ``mic_array_pdm_rx()`` task expects the PDM microphone port to be clocked from the PDM clock.


|appendix|
|newpage|

Demo Hardware Setup
-------------------

To run the demo, connect a USB cable to power the xCORE-200 Array Microphone board 
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
Array Microphone board.

Once built there will be a ``bin/`` directory within the project which contains
the binary for the xCORE device. The xCORE binary has a XMOS standard .xe extension.

Launching from the command line
...............................

From the command line you use the ``xrun`` tool to download and run the code
on the xCORE device::

  xrun --xscope bin/app_xua_simple.xe

Once this command has executed the application will be running on the
xCORE-200 Array Microphone board

Launching from xTIMEcomposer Studio
...................................

From xTIMEcomposer Studio use the run mechanism to download code to xCORE device.
Select the xCORE binary from the ``bin/`` directory, right click and go to Run
Configurations. Double click on xCORE application to create a new run configuration,
enable the xSCOPE I/O mode in the dialog box and then
select Run.

Once this command has executed the application will be running on the
xCORE-200 Array Microphone board.

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
