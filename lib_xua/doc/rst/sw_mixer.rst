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
A list of arguments can also be seen with::

  $ ./xmos_mixer --help

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
mixer to perform a loop-back from analogue inputs 1 & 2 to analogue outputs 1 & 2. 

.. note::

    The command outputs shown are examples; the actual output will depend on the mixer configuration.

The following will show the index for each device output along with which channel is currently mapped to it.
In this example the analogue outputs 1 & 2 are 0 & 1 respectively:: 

  $ ./xmos_mixer --display-aud-channel-map

    Audio Output Channel Map
    ------------------------
  
  0 (DEVICE OUT - Analogue 1) source is  0 (DAW OUT - Analogue 1)
  1 (DEVICE OUT - Analogue 2) source is  1 (DAW OUT - Analogue 2)
  2 (DEVICE OUT - SPDIF 1) source is  2 (DAW OUT - SPDIF 1)
  3 (DEVICE OUT - SPDIF 2) source is  3 (DAW OUT - SPDIF 2)
  $ _

The DAW Output Map can be seen with::

  $ ./xmos_mixer --display-daw-channel-map

    DAW Output To Host Channel Map
    ------------------------
  
  0 (DEVICE IN - Analogue 1) source is  4 (DEVICE IN - Analogue 1)
  1 (DEVICE IN - Analogue 2) source is  5 (DEVICE IN - Analogue 2)
  $ _

.. note::

    In both cases, by default, these bypass the mixer.

The following command will list the channels which can be mapped to the device outputs from the 
Audio Output Channel Map. Note that, in this example, analogue inputs 1 & 2 are source 4 & 5 and 
Mix 1 & 2 are source 6 & 7::

  $ ./xmos_mixer --display-aud-channel-map-sources

    Audio Output Channel Map Source List
    ------------------------------------

  0 (DAW OUT - Analogue 1)
  1 (DAW OUT - Analogue 2)
  2 (DAW OUT - SPDIF 1)
  3 (DAW OUT - SPDIF 2)
  4 (DEVICE IN - Analogue 1)
  5 (DEVICE IN - Analogue 2)
  6 (MIX - Mix 1)
  7 (MIX - Mix 2)
  $ _

Using the indices from the previous commands, we will now re-map the first two mixer channels (Mix 1 & Mix 2) to device outputs 1 & 2::

  $ ./xmos_mixer --set-aud-channel-map 0 6
  $ ./xmos_mixer --set-aud-channel-map 1 7
  $ _

You can confirm the effect of this by re-checking the map::

  $ ./xmos_mixer --display-aud-channel-map

    Audio Output Channel Map
    ------------------------
  
  0 (DEVICE OUT - Analogue 1) source is  6 (MIX - Mix 1)
  1 (DEVICE OUT - Analogue 2) source is  7 (MIX - Mix 2)
  2 (DEVICE OUT - SPDIF 1) source is  2 (DAW OUT - SPDIF 1)
  3 (DEVICE OUT - SPDIF 2) source is  3 (DAW OUT - SPDIF 2)
  $ _

This now derives analogue outputs 1 & 2 from the mixer, rather than directly from USB. However,
since the mixer is mapped, by default, to just pass the USB channels through to the outputs there will be no
functional change.


.. note::

  The USB audio reference design has only one unit so the mixer_id argument should always be 0.

The mixer nodes need to be individually set. The nodes in mixer_id 0 can be displayed
with the following command::

  $ ./xmos_mixer --display-mixer-nodes 0

    Mixer Values (0)
    ----------------

                         Mixer outputs
                                  1              2
    DAW - Analogue 1       0:[0000.000]   1:[  -inf  ]
    DAW - Analogue 2       2:[  -inf  ]   3:[0000.000]
    DAW - SPDIF 1          4:[  -inf  ]   5:[  -inf  ]
    DAW - SPDIF 2          6:[  -inf  ]   7:[  -inf  ]
    AUD - Analogue 1       8:[  -inf  ]   9:[  -inf  ]
    AUD - Analogue 2      10:[  -inf  ]  11:[  -inf  ]
  $ _

With mixer outputs 1 & 2 mapped to device outputs analogue 1 & 2; to get the audio from the analogue inputs to device 
outputs mixer_id 0 node 8 and node 11 need to be set to 0db::

  $ ./xmos_mixer --set-value 0 8 0
  $ ./xmos_mixer --set-value 0 11 0
  $ _

At the same time, the original mixer outputs can be muted::

  $ ./xmos_mixer --set-value 0 0 -inf
  $ ./xmos_mixer --set-value 0 3 -inf
  $ _

Now audio inputs on analogue 1 and 2 should be heard on outputs 1 and 2 respectively. 

As mentioned above, the flexibility of the mixer is such that there will be multiple ways to create
a particular mix. Another option to create the same routing would be to change the mixer sources
such that mixer outputs 1 and 2 come from the analogue inputs 1 and 2. 

To demonstrate this, firstly undo the changes above (or simply reset the device)::

  $ ./xmos_mixer --set-value 0 8 -inf
  $ ./xmos_mixer --set-value 0 11 -inf
  $ ./xmos_mixer --set-value 0 0 0
  $ ./xmos_mixer --set-value 0 3 0
  $ _

The mixer should now have the default values. The sources for mixer 0 output 1 and 2 can now be changed 
using indices from the Audio Output Channel Map Source List::

  $ ./xmos_mixer --set-mixer-source 0 0 4

     Set mixer(0) input 0 to device input 4 (AUD - Analogue 1)
  $ ./xmos_mixer --set-mixer-source 0 1 5

     Set mixer(0) input 1 to device input 5 (AUD - Analogue 2)
  $ _

If you re-run the following command then the first column now has "AUD - Analogue 1 and 2" rather
than "DAW (Digital Audio Workstation i.e. the host) - Analogue 1 and 2" confirming the new mapping. 
Again, by playing audio into analogue inputs 1/2 this can be heard looped through to analogue outputs 1/2::
  
  $ ./xmos_mixer --display-mixer-nodes 0

