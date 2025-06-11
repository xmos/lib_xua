|newpage|

USB Audio Class version
=======================

The codebase supports USB Audio Class (UAC) versions 1.0 and 2.0.

UAC 2.0 offers many improvements over UAC 1.0, most notable is the complete support for high-speed
(HS) operation.  This means that Audio Class devices are no longer limited to full-speed (FS)
operation allowing greater channel counts, sample frequencies and sample bit-depths.
Additional improvements, amongst others, include:

- Added support for multiple clock domains, clock description and clock control

- Extensive support for interrupts to inform the host about dynamic changes that occur to different
  entities such as Clocks etc

Driver support
--------------

Audio Class 1.0
^^^^^^^^^^^^^^^

 - Supported in Apple macOS.
 - Supported in all modern Microsoft Windows operating systems (i.e. Windows XP and later).

Audio Class 2.0
^^^^^^^^^^^^^^^

 - Supported in Apple macOS since version 10.6.4.
 - Supported in Windows since version 10, release 1703.

Third party Windows drivers are also available, however, documentation of these is beyond the scope
of this document, please contact `XMOS` for further details.

Configuring Audio Class version
-------------------------------

Configuring the ``AUDIO_CLASS`` define to ``1`` or ``2`` will set the UAC version for the device
to 1.0 or 2.0 respectively.

The default value is ``2`` which causes the device to run as a HS UAC 2.0 device when connected to
a HS host/hub and as a FS UAC 2.0 device when connected to a FS host/hub.

Setting ``AUDIO_CLASS`` to ``1`` will cause the device to run as a FS Audio Class 1.0 device.

.. warning::

    To ensure specification compliance, Audio Class 1.0 mode is not supported at high-speed.

Defines are also provided to allow a different UAC version for HS and FS:

- ``XUA_AUDIO_CLASS_HS``: UAC version to run at high-speed (0: Disabled, 2: Audio Class 2.0)
- ``XUA_AUDIO_CLASS_FS``: UAC version to run at full-speed (0: Disabled, 1: Audio Class 1.0, 2: Audio Class 2.0)

.. warning::

    Disabling Audio Class support or dynamically switching between Audio Class 1.0 and 2.0 based on
    USB bus speed may lead to USB compliance issues.

    The USB-IF views such behavior as a significant functional change, which may violate compliance
    requirements. Devices are expected to maintain consistent functionality regardless of bus speed,
    and altering the USB class or interface descriptors dynamically can result in unpredictable host
    behavior and test failures during USB-IF certification.

    Recommendation: Avoid switching USB Audio Class modes based on bus speed.

Due to bandwidth limitations of FS USB the following restrictions are applied during FS
operation for both UAC 1.0 and 2.0 modes:

-  Sample rate is limited to a maximum of 48kHz if both input *and* output is enabled.
-  Sample rate is limited to a maximum of 96kHz if only input *or* output is enabled.
-  Channel count is limited to a maximum of 2 channels for both input and output paths.

Audio Class 1.0 devices
^^^^^^^^^^^^^^^^^^^^^^^

Some products may opt to operate in UAC 1.0 mode to enable driver-less compatibility
with older Windows versions or certain embedded hosts. This mode was historically preferred for
ensuring basic plug-and-play audio functionality without requiring custom drivers.

However, the need for UAC 1.0 support is diminishing as UAC 2.0 becomes more widely
supported across modern operating systems and embedded platforms. UAC 2.0 offers better
performance, higher sample rates, and more robust feature support, and is now natively supported
on Windows 10+, macOS, Linux, and many embedded systems.

Recommendation: Where possible, default to UAC 2.0 for new designs unless specific host
compatibility requirements mandate support for UAC 1.0.

The device will operate in FS UAC 1.0 mode if one of the following is true:

- The code is compiled for USB Audio Class 1.0 *only* i.e.
  - ``AUDIO_CLASS`` is set to ``1`` *or*
  - ``XUA_AUDIO_CLASS_HS`` is set to ``0`` and ``XUA_AUDIO_CLASS_FS`` is set to ``1``.
- The code is compiled for UAC 2.0 at HS and UAC 1.0 at FS i.e. ``XUA_AUDIO_CLASS_HS``
  is set to ``2`` and ``XUA_AUDIO_CLASS_FS`` is set to ``1`` and the device is connected to a host
  via a full-speed link

Related defines
---------------

:numref:`opt_audio_class_defines` describes the defines that affect audio class selection:

.. _opt_audio_class_defines:

.. list-table:: Audio Class defines
   :header-rows: 1
   :widths: 40 40 30

   * - Define
     - Description
     - Default
   * - ``AUDIO_CLASS``
     - Audio Class version (1 or 2)
     - ``2``
   * - ``XUA_AUDIO_CLASS_HS``
     - Audio Class version to run at high-speed (0: Disabled,  2 UAC 2.0)
     - ``2``
   * - ``XUA_AUDIO_CLASS_FS``
     - Audio Class version to run at full-speed (0: Disabled, 1: UAC 1.0, 2: UAC 2.)
     - ``2``

