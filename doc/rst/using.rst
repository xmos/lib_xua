|newpage|

***********
Basic Usage
***********

This section describes the basic usage of `lib_xua` and provides a guide on how to program USB Audio devices.

Library Structure
=================

The code is split into several directories.

.. list-table:: lib_xua structure

 * - core
   - Common code for USB audio applications
 * - midi
   - MIDI I/O code
 * - dfu
   - Device Firmware Upgrade code
 * - hid
   - Human Interface Device code

Note, the midi and dfu directories are potential candidates for separate libs in their own right.

Using in an application
=======================

``lib_xua`` is intended to be used with `XCommon CMake <https://www.xmos.com/file/xcommon-cmake-documentation/?version=latest>`_
, the `XMOS` application build and dependency management system.

To use ``lib_xua`` in an application, add ``lib_xua``, to the list of dependent modules in the application's `CMakeLists.txt` file.

  set(APP_DEPENDENT_MODULES "lib_xua")

All `lib_xua` functions can be accessed via the ``xua.h`` header file::

  #include <xua.h>

.. _sec_basic_usage_codeless:

"Codeless" programming model
============================

Whilst it is possible to code a USB Audio device using the building blocks provided by `lib_xua`,
it is realised that this might not be desirable for many classes of customers or products.

For instance, some users may not have a large software development experience and simply want to
customise some basic settings such as strings, sample-rates, channel-counts etc.
Others may want to fully customise the implementation - adding additional functionality such as
integrating DSP or possibly only using a subset of the functions provided - just ``XUA_AudioHub``,
for example.

In addition, the large number of supported features can lead to a large number of tasks, hardware
resources, communication channels etc, requiring quite a lot of code to be authored for each product.

In order to cater for the former class of users, a "codeless" option is provided. Put simply, a file
``main.xc`` is provided which includes a pre-authored ``main()`` function along with all of the
required hardware resource declarations. Code is generated based on the options provided by the
developer in ``xua_conf.h``.

Using this development model the user simply includes a ``xua_conf.h`` with their settings and
optional implementations of any 'user functions' as desired. This, along with an XN file for their
hardware platform, is all that is required to build a fully featured and functioning product. This
XN file should contain definitions of the ports used for the various ``lib_xua`` functionality,
see :ref:`sec_options`.

This development model also provides the benefit of a full and verified codebase as a basis for a product.

This behaviour described in this section is the default behaviour of `lib_xua`, to disable this please
set ``EXCLUDE_USB_AUDIO_MAIN`` to 1 in the application `CMakeLists.txt` or ``xua_conf.h`` and see
:ref:`sec_advanced_usage`.


Extending the "Codeless" application
====================================

The ``main.xc`` function allows insertion of extra code using the preprocessor. For example, you may wish
to add some control code to control buttons or LEDs or DSP tasks for audio enhancement.

Adding globals
..............

To add globals to your application, you may declare them in a dedicated source file in your project. They
may then be referenced from other files using the ``extern`` keyword.

Alternatively, you may add an optional header file to your project called ``user_main_globals.h``.
If this file exists, its contents will be inserted into ``main.xc`` in global scope.

Example contents of ``user_main_globals.h``::

  unsigned my_global_var = 42;

Adding main function declarations
.................................

To add declarations to your application, for example channels or interfaces to connect between tasks, you can
define the token ``USER_MAIN_DECLARATIONS`` from your ``xua_conf.h`` configuration file. This inserts the 
macro inside ``main.xc`` after ``main()`` but before the main ``par`` statement. 

Alternatively, you may add an optional header file to your project called ``user_main_declarations.h``.
If this file exists, its contents will be inserted into ``main.xc`` before the main ``par`` statement.

Example contents of ``user_main_declarations.h``::

  chan c_usb_to_user_interface;


Adding main function tasks
..........................

To add extra tasks to your application, you can define the token ``USER_MAIN_CORES`` from your ``xua_conf.h`` configuration file. This inserts the  macro inside ``main.xc`` after the main ``par`` statement meaning the
compiler will run these tasks in parallel, either on a dedicated hardware thread or combine with other tasks if the task is marked as ``[[combinable]]``. 
                   
Alternatively, you may add an optional header file to your project called ``user_main_cores.h``.
If this file exists, its contents will be inserted into ``main.xc`` after the main ``par`` statement.

Example contents of ``user_main_cores.h``::

  on tile[1]: my_user_interface_task(c_usb_to_user_interface);

Configuring lib_xua
===================

Configuration of the various build time options of ``lib_xua`` is done via the optional header `xua_conf.h`.
To allow the build scripts to locate this file it should reside somewhere in the application `src` directory.

Such build time options include audio class version, sample rates, channel counts etc. Please see
:ref:`sec_api` for full listings.

The build system will automatically include the `xua_conf.h` header file as appropriate - the developer
should continue to include `xua.h` as previously directed. A simple example `xua_conf.h` file is
shown below::

    #ifndef _XUA_CONF_H_
    #define _XUA_CONF_H_

    /* Output channel count */
    #define XUA_NUM_USB_CHAN_OUT (2)

    /* Product string */
    #define XUA_PRODUCT_STR_A2 "My Product"

    #endif

User functions
==============

To enable custom functionality, such as configuring external audio hardware, bespoke behaviour on
stream start/stop etc, various functions can be overridden by the user. (see :ref:`sec_api` for
full listings). The default implementations of these functions are empty.

