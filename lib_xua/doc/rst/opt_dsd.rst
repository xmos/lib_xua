
Direct Stream Digital (DSD)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Both DSD over PCM (DoP) and a "Native" implementation are available to support DSD output. 
DSD is disabled by default.

DSD is enabled with by setting the following define to a non-zero value.

.. _opt_dsd_defines:

.. list-table:: DSD Defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - DSD_CHANS_DAC
     - Number of DSD channels
     - 0 (Disabled)

Typically this would be set to `2` for stereo output.

By default both "Native" and DoP functionality are enabled when DSD is enabled. The Native DSD implementation uses
an alternative streaming interface to such that the host can inform the device that DSD data is being streamed. 
See: ::ref:`sec_opt_audio_formats` for details.

If only DoP functionalty is desired the Native implementation can be disabled with the following define.

.. _opt_nativedsd_defines:

.. list-table:: Native DSD Defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - NATIVE_DSD
     - Enable/Disable "Native" DSD implementation
     - 1 (Enabled)

DoP support follows the method described in `DoP Open Standard 1.1 
<http://dsd-guide.com/sites/default/files/white-papers/DoP_openStandard_1v1.pdf>`_  

DSD over PCM (DoP)
..................

While Native DSD support is available in Windows though a driver, OSX incorporates a USB driver
that only supports PCM, this is also true of the central audio engine, CoreAudio.  It is
therefore not possible to use the scheme defined above using the built in driver support of OSX.

Since the Apple OS only allows a PCM path a method of transporting DSD audio data over PCM frames 
has been developed.  This data can then be sent via the native USB Audio support.

Standard DSD  has a sample size of 1 bit and a sample rate of 2.8224MHz - this is 64x the speed of CD. 
This equates to the same data-rate as a 16 bit PCM stream at 176.4kHz. 

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
