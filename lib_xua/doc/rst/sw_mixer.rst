.. _usb_audio_sec_mixer:

Digital Mixer
-------------

The Mixer core(s) take outgoing audio from the Decouple core and incoming audio from the Audio Hub
core. It then applies the volume to each channel and passes incoming audio on to Decouple and outgoing
audio to Audio Hub. The volume update is achieved using the built-in 32bit to 64bit signed 
multiply-accumulate function (``macs``). The mixer is implemented in the file ``mixer.xc``.

The mixer takes (up to) two cores and can perform eight mixes with up to 18 inputs at sample rates 
up to 96kHz and two mixes with up to 18 inputs at higher sample rates. The component automatically 
reverts to generating two mixes when running at the higher rate.

The mixer can take inputs from either:

   * The USB outputs from the host---these samples come from the Decouple core.
   * The inputs from the audio interfaces on the device---these samples come from the Audio Hub core
     and includes samples from digital input streams.

Since the sum of these inputs may be more then the 18 possible mix inputs to each mixer, there is a
mapping from all the possible inputs to the mixer inputs.

After the mix occurs, the final outputs are created. There are two possible output destinations
for each mix.

   * The USB inputs to the host---these samples are sent to the Decouple core.

   * The outputs to the audio interface on the device---these samples are sent to the Audio Hub
     core

For each possible output from the device, a mapping exists to inform the mixer what it's source is. 
The possible sources are the output from the USB host, the inputs from the Audio Hub core or the
outputs from the mixes.

Essentially the mixer/router can be configured such that any device input can be used as an input to
any mix or routed directly to any device output. Additionally, any device output can be derived from
any mixer output or any device input.  

As mentioned in :ref:`usb_audio_sec_audio-requ-volume`, the mixer can also handle processing or
volume controls. If the mixer is configured to handle volume but the number of mixes is set to zero
(such that the core is solely doing volume setting) then the component will use only one core. This
is sometimes a useful configuration for large channel count devices.

Control
~~~~~~~

The mixers can receive the control commands from the host via USB Control Requests to Endpoint 0.
The Endpoint 0 core relays these to the Mixer cores(s) via a channel (``c_mix_ctl``). These commands
are described in :ref:`table_mixer_commands`.

.. _table_mixer_commands:

.. list-table:: Mixer Component Commands
 :header-rows: 1

 * - Command
   - Description

 * - ``SET_SAMPLES_TO_HOST_MAP``
   - Sets the source of one of the audio streams going to the host.

 * - ``SET_SAMPLES_TO_DEVICE_MAP``
   - Sets the source of one of the audio streams going to the audio
     driver.

 * - ``SET_MIX_MULT``
   - Sets the multiplier for one of the inputs to a mixer.

 * - ``SET_MIX_MAP``
   - Sets the source of one of the inputs to a mixer.

 * - ``SET_MIX_IN_VOL``
   - If volume adjustment is being done in the mixer, this command
     sets the volume multiplier of one of the USB audio inputs.

 * - ``SET_MIX_OUT_VOL``
   - If volume adjustment is being done in the mixer, this command
     sets the volume multiplier of one of the USB audio outputs.

Host Control
~~~~~~~~~~~~

The mixer can be controlled from a host PC by sending requests to Endpoint 0. XMOS provides a simple 
command line based sample application demonstrating how the mixer can be controlled. This is
intended as an example of how you might add mixer control to your own control application. It is not
intended to be exposed to end users. 

For details, consult the README file in the host_usb_mixer_control directory.

The main requirements of this control utility are to

  * Set the mapping of input channels into the mixer
  * Set the coefficients for each mixer output for each input
  * Set the mapping for physical outputs which can either come
    directly from the inputs or via the mixer.

.. note::

    The flexibility within this configuration space us such that there is often multiple ways
    of producing the desired result.  Product developers may only want to expose a subset of this
    functionality to their end users.

Whilst using the XMOS Host control example application, consider the example of setting the
mixer to perform a loop-back from analogue inputs 1 and 2 to analogue outputs 1 and 2. 

Firstly consider the inputs to the mixer. The following will displays which channels are mapped 
to which mixer inputs:: 

  ./xmos_mixer --display-aud-channel-map 0

The following command will displays which channels could possibly be mapped to mixer inputs. Notice
that analogue inputs 1 and 2 are on mixer inputs 10 and 11::

./xmos_mixer --display-aud-channel-map-sources 0

Now examine the audio output mapping using the following command::

  ./xmos_mixer --display-aud-channel-map 0

This displays which channels are mapped to which outputs. By default all
of these bypass the mixer. We can also see what all the possible
mappings are with the following command::

  ./xmos_mixer --display-aud-channel-map-sources 0

We will now map the first two mixer outputs to physical outputs 1 and 2::

  ./xmos_mixer --set-aud-channel-map 0 26
  ./xmos_mixer --set-aud-channel-map 1 27

You can confirm the effect of this by re-checking the map::

  ./xmos_mixer --display-aud-channel-map 0

This now derives analogue outputs 1 and 2 from the mixer, rather than directly from USB. However,
since the mixer is still mapped to pass the USB channels through to the outputs there will be no
functional change.

The mixer nodes need to be individually set. They can be displayed
with the following command::

  ./xmos_mixer --display-mixer-nodes 0

To get the audio from the analogue inputs to outputs 1 and 2, nodes 80
and 89 need to be set::

  ./xmos_mixer --set-value 0 80 0
  ./xmos_mixer --set-value 0 89 0

At the same time, the original mixer outputs can be muted::

  ./xmos_mixer --set-value 0 0 -inf
  ./xmos_mixer --set-value 0 9 -inf

Now audio inputs on analogue 1/2 should be heard on outputs 1/2. 

As mentioned above, the flexibility of the mixer is such that there will be multiple ways to create
a particular mix. Another option to create the same routing would be to change the mixer sources
such that mixer 1/2 outputs come from the analogue inputs. 

To demonstrate this, firstly undo the changes above (or simply reset the device)::

  ./xmos_mixer --set-value 0 80 -inf
  ./xmos_mixer --set-value 0 89 -inf
  ./xmos_mixer --set-value 0 0 0
  ./xmos_mixer --set-value 0 9 0

The mixer should now have the default values. The sources for mixer 1/2 can now be changed::

  ./xmos_mixer --set-mixer-source 0 0 10
  ./xmos_mixer --set-mixer-source 0 1 11

If you re-run the following command then the first column now has "AUD - Analogue 1 and 2" rather
than "DAW (Digital Audio Workstation i.e. the host) - Analogue 1 and 2" confirming the new mapping. 
Again, by playing audio into analogue inputs 1/2 this can be heard looped through to analogue outputs 1/2::
  
    ./xmos_mixer --display-mixer-nodes 0

