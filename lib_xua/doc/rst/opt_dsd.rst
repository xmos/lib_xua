|newpage|

Direct Stream Digital (DSD)
===========================

Direct Stream Digital (DSD) is used for digitally encoding audio signals on Super Audio CDs (SACD).
It uses pulse-density modulation (PDM) encoding.

The codebase supports DSD playback from the host via "DSD over PCM" (DoP) and a "Native" implementation
which is, while USB specification based, proprietary to XMOS.

DSD is enabled with by setting the define in :ref:`opt_dsd_defines` to a non-zero value.

.. _opt_dsd_defines:

.. list-table:: DSD defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``DSD_CHANS_DAC``
     - Number of DSD channels
     - ``0`` (Disabled)

Typically this would be set to ``2`` for stereo output.

By default both "Native" and DoP functionality are enabled when DSD is enabled. The Native DSD implementation uses
an alternative streaming interface such that the host can inform the device that DSD data is being streamed. 
See: ::ref:`sec_opt_audio_formats` for details.

If only DoP functionality is desired the Native implementation can be disabled with the define in
:ref:`opt_nativedsd_defines`.

.. _opt_nativedsd_defines:

.. list-table:: Native DSD defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``NATIVE_DSD``
     - Enable/Disable "Native" DSD implementation
     - ``1`` (Enabled)


DSD over PCM (DoP)
------------------

DoP support follows the method described in the `DoP Open Standard 1.1 
<http://dsd-guide.com/sites/default/files/white-papers/DoP_openStandard_1v1.pdf>`_. 

While Native DSD support is available in Windows though a driver, OSX incorporates a USB driver
that only supports PCM, this is also true of the central audio engine, CoreAudio.  It is
therefore not possible to use the "Native" scheme defined above using the built in driver of OSX.

Since the Apple OS only allows a PCM path a method of transporting DSD audio data over PCM frames 
has been developed.

Standard DSD  has a sample size of 1 bit and a sample rate of 2.8224MHz - this is 64x the speed of a
compact disc (CD). This equates to the same data-rate as a 16 bit PCM stream at 176.4kHz. 

In order to clearly identify when this PCM stream contains DSD and when it contains PCM some header
bits are added to the sample.  A 24-bit PCM stream is therefore used, with the most significant
byte being used for a DSD marker (alternating 0x05 and 0xFA values).

When enabled, if USB audio design detects a un-interrupted run of these samples (above a defined 
threshold) it switches to DSD mode, using the lower 16-bits as DSD sample data.  When this check for 
DSD headers fails the design falls back to PCM mode.  DoP detection and switching is done completely 
in the Audio/I2S core (`audio.xc`). All other code handles the audio samples as PCM. 

The design supports higher DSD/DoP rates (i.e. DSD128) by simply raising the underlying PCM sample
rate e.g. from 176.4kHz to 352.8kHz. The marker byte scheme remains exactly the same regardless
of rate.

.. note::
    
    DoP requires bit-perfect transmission - therefore any audio/volume processing will break the stream.

"Native" vs DoP
---------------

Since the DoP specification requires header bytes this eats into the data bandwidth. The "Native" implementation
has no such overhead and can therefore transfer the same DSD rate and half the effective PCM rate of DoP.
Such a property may be desired when upporting DSD128 without exposing a 352.8kHz PCM rate, for example.

Ports
-----

The codebase expects 1-bit ports to be defined in the application XN file for the DSD data and  
clock lines for example::

    <Port Location="XS1_PORT_1M"  Name="PORT_DSD_DAC0"/>
    <port Location="XS1_PORT_1N"  Name="PORT_DSD_DAC1"/>
    <Port Location="XS1_PORT_1G"  Name="PORT_DSD_CLK"/>

.. note::

   The DSD ports may or may not overlap the I2S ports - the codebase will reconfigure the ports as appropriate
   when switching between PCM and DSD modes.

