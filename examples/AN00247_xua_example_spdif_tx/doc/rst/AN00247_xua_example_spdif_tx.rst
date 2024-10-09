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

To start using the XMOS XUA library, you need to add ``lib_xua`` to the dependent module list
in the CMakeLists.txt file. This application note also uses ``lib_spdif``, so this must be
added to the list. The ``lib_board_support`` software repository is also used to provide common
code to configure the xCORE.ai Multichannel (MC) Audio board for use. This should also be added
to the list of dependent modules in the CMakeLists.txt file::

  set(APP_DEPENDENT_MODULES "lib_xua"
                            "lib_spdif"
                            "lib_board_support")

The dependencies for this example are specified by ``deps.cmake`` in the ``examples`` directory
and are included in the application ``CMakeLists.txt`` file.

The ``lib_xud`` library requires some flags for correct operation. Namely the
tile on which ``lib_xud`` will be executed, for example::

  set(APP_COMPILER_FLAGS ... -DUSB_TILE=tile[0] ...)

The ``lib_board_support`` requires a compiler flag to select the hardware type::

  set(APP_COMPILER_FLAGS ... -DBOARD_SUPPORT_BOARD=XK_AUDIO_316_MC_AB ...)

Includes
--------

This application requires the system header that defines XMOS xCORE specific
defines for declaring and initialising hardware:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: include <xs1.h>
   :end-before: include "xua.h"

The XUA library functions are defined in ``xua.h``. This header must
be included in your code to use the library.  Headers are also required
for ``lib_xud``, ``lib_spdif`` and the board setup code for the
xCORE.ai Multichannel Audio board.

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: include "xua.h"
   :end-at: include "xk_audio_316_mc_ab/board.h"

Declarations
------------

Allocating hardware resources for lib_xua
.........................................

A minimal implementation of a USB Audio device, without I2S functionality,
using ``lib_xua`` requires the follow pins:

    - Audio Master clock (from clock source to xCORE)

On an xCORE the pins are controlled by ``ports``. The application therefore declares a
port for the master clock input signal.

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: /* Lib_xua port declaration
   :end-at: in port p_mclk_in

``lib_xua`` also requires two ports for internally calculating USB feedback. Please refer to
the ``lib_xua`` library documentation for further details.  The additional input port for the master
clock is required since USB and S/PDIF do not reside of the same tiles on the example hardware.

These ports are declared as follows:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: /* Resources for USB feedback
   :end-at: in port p_mclk_in_usb

In addition to ``port`` resources two clock-block resources are also required:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: /* Clock-block
   :end-at: clock clk_audio_mclk_usb

Again, for the same reasoning as the master-clock ports, two master-clock clock-blocks are required
- one on each tile.


Allocating hardware resources for lib_spdif
...........................................

The S/PDIF transmitter requires a single (buffered) 1-bit port:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: /* Lib_spdif port
   :end-at: buffered out port

This port must be clocked from the audio master clock. This application note chooses to declare
an extra clock-block as follows:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: clock clk_spdif_tx
   :end-before: /* Lib_xua


Other declarations
..................

``lib_xua`` currently requires the manual declaration of tables for the endpoint types for
``lib_xud`` and the calling the main XUD function in a par (``XUD_Main()``).

For a simple application the following endpoints are required:

    - ``Control`` endpoint zero
    - ``Isochonous`` endpoint for each direction for audio data to/from the USB host

These are declared as follows:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: /* Endpoint type tables
   :end-at: XUD_EpType epTypeTableIn

Hardware Setup
--------------

Some code is needed to perform the hardware-specific setup for the board being used
in this application note.

The ``xk_audio_316_mc_ab_config_t`` structure is used to specify hardware-specific
configuration options, such as clocking modes and frequencies.

The ``i_i2c_client`` unsafe client interface is required to have a globally-scoped variable
for gaining access to the ``i2c_master_if`` interface from the audio hardware functions.

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: /* Board configuration from lib_board_support */
   :end-at: unsafe client interface i2c_master_if i_i2c_client

The following functions are called by ``XUA_AudioHub`` to configure the hardware; they are
defined as wrapper functions around the board-specific code from ``lib_board_support``.

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: void AudioHwInit()
   :end-before: int main()

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

.. literalinclude:: ../../src/xua_conf.h
   :start-at: // Copyright
   :end-at: #endif

The application main() function
-------------------------------

The ``main()`` function sets up the tasks in the application.

Various channels/interfaces are required in order to allow the required tasks to communicate.
These must first be declared:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: /* Channels for lib_xud
   :end-at: interface i2c_master_if i2c[1]

The rest of the ``main()`` function starts all of the tasks in parallel
using the xC ``par`` construct:

.. literalinclude:: ../../src/app_xua_spdiftx.xc
   :start-at: par
   :end-before: return 0

This code starts the low-level USB task, an Endpoint 0 task, an Audio buffering task and a task to handle
the audio I/O. Note, since there is no I2S functionality in this example this task simply forwards samples to the
SPDIF transmitter task. In addition the ``spdif_tx()`` task is also run.

Note that the ``spdif_tx_port_config()`` function is called before a nested ``par`` of ``spdif_tx()`` and ``XUA_AudioHub()``.
This is because of the "shared" nature of ``p_mclk_in`` and avoids a parallel usage check failure by the XMOS tool-chain.

It also runs ``xk_audio_316_mc_ab_board_setup()`` and ``xk_audio_316_mc_ab_i2c_master()`` from ``lib_board_support``
that are used for setting up the hardware.

|newpage|

Building the Application
------------------------

The following section assumes you have downloaded and installed the `XMOS XTC tools <https://www.xmos.com/software-tools/>`_
(see `README` for required version). Installation instructions can be found `here <https://xmos.com/xtc-install-guide>`_.
Be sure to pay attention to the section `Installation of required third-party tools
<https://www.xmos.com/documentation/XM-014363-PC-10/html/installation/install-configure/install-tools/install_prerequisites.html>`_.

The application uses the `xcommon-cmake <https://www.xmos.com/file/xcommon-cmake-documentation/?version=latest>`_
build system as bundled with the XTC tools.

The ``AN00247_xua_example_spdiftx`` software zip-file should be downloaded and unzipped to a chosen directory.

To configure the build run the following from an XTC command prompt::

    cd examples
    cd AN00247_xua_example_spdif_tx
    cmake -G "Unix Makefiles" -B build

Finally, the application binaries can be built using ``xmake``::

    xmake -C build

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

  xrun --xscope bin/app_xua_spdiftx.xe

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

  * XMOS Tools User Guide

    https://www.xmos.com/documentation/XM-014363-PC-9/html/

  * XMOS xCORE Programming Guide

    https://www.xmos.com/published/xmos-programming-guide

  * XMOS Libraries

    https://www.xmos.com/libraries/

|newpage|

