|newpage|

.. _usb_audio_sec_audio:

Audio Hub
=========

The Audio Hub task performs many functions. It receives and transmits samples from/to the Decoupler
or Mixer core over a channel.

It also drives several in and out I2S/TDM channels to/from a CODEC, DAC, ADC etc. From now on these
external devices will be termed "audio hardware".

If the firmware is configured with the xCORE as I2S master the required clock lines will also be
driven from this task. It also has the task of forwarding on and receiving samples to/from other
audio related tasks/cores such as S/PDIF tasks, ADAT etc.

In master mode, the xCORE generates the I2S "Continuous Serial Clock (SCK)", or "Bit-Clock (BCLK)"
and the "Word Select (WS)" or "left-right clock (LRCLK)" signals. Any CODEC or DAC/ADC combination
that supports I2S and can be used.

The LR-clock, bit-clock and data are all derived from the incoming master clock (typically the 
output of the external oscillator or PLL). This is not part of the I2S standard but is commonly 
included for synchronizing the internal operation of the analog/digital converters.

The Audio Hub task is implemented in the file ``xua_audiohub.xc``.

:ref:`usb_audio_codec_signals` shows the signals used to communicate audio between the XMOS device
and the external audio hardware.

.. _usb_audio_codec_signals:

.. list-table:: I2S Signals
   :header-rows: 1
   :widths: 20 80
  
   * - Signal
     - Description
   * - LRCLK 
     - The word clock, transition at the start of a sample
   * - BCLK     
     - The bit clock, clocks data in and out
   * - SDIN
     - Sample data in (from CODEC/ADC to the XMOS device)
   * - SDOUT 
     - Sample data out (from the XMOS device to CODEC/DAC)
   * - MCLK
     - The master clock running the CODEC/DAC/ADC

The bit clock controls the rate at which data is transmitted to and from the external audio hardware.

In the case where the XMOS device is the master, it divides the MCLK to generate the required signals for both BCLK and LRCLK,
with BCLK then being used to clock data in (SDIN) and data out (SDOUT) of the external audio hardware.

:ref:`usb_audio_example_clock_divides` shows some example clock frequencies and divides for different sample rates:

.. _usb_audio_example_clock_divides:

.. list-table:: Clock Divide examples
  :header-rows: 1
  :widths: 30 25 25 20

  * - Sample Rate (kHz)    
    - MCLK (MHz)
    - BCLK (MHz)
    - Divide
  * - 44.1  
    - 11.2896 
    - 2.819 
    - 4
  * - 88.2 
    - 11.2896
    - 5.638 
    - 2
  * - 176.4 
    - 11.2896
    - 11.2896 
    - 1
  * - 48 
    - 24.576 
    - 3.072 
    - 8
  * - 96 
    - 24.576 
    - 6.144 
    - 4
  * - 192 
    - 24.576 
    - 12.288 
    - 2

For xCORE-200 devices the master clock must be supplied by an external source e.g. clock generator,
fixed oscillators, PLL etc. xCORE.ai devices may use the integrated secondary PLL. 

Two master clock frequencies to support 44.1kHz and 48kHz audio frequencies (e.g. 11.2896/22.5792MHz
and 12.288/24.576MHz respectively).  This master clock input is then provided to the external audio
hardware and the xCORE device. 

Port Configuration (xCORE Master)
---------------------------------

The default software configuration is xCORE is I2S master.  That is, the XMOS device provides the BCLK and LRCLK signals to the external audio hardware

xCORE ports and XMOS clocks provide many valuable features for implementing I2S. This section describes how these are configured
and used to drive the I2S interface.

.. only:: latex

  .. figure:: images/port_config.pdf

    Ports and Clocks (xCORE master)

.. only:: html

  .. figure:: images/port_config.png

    Ports and Clocks (xCORE master)

The code to configure the ports and clocks is in the ``ConfigAudioPorts()`` function. Developers should not need to modify this.

The xCORE inputs MCLK and divides it down to generate BCLK and LRCLK. 

To achieve this MCLK is input into the device using the 1-bit port ``p_mclk``. This is attached to the clock block ``clk_audio_mclk``, which is in
turn used to clock the BCLK port, ``p_bclk``. BCLK is used to clock the LRCLK (``p_lrclk``) and data signals SDIN (``p_sdin``) and SDOUT (``p_sdout``). 

Again, a clock block is used (``clk_audio_bclk``) which has ``p_bclk`` as its input and is used to clock the ports ``p_lrclk``, ``p_sdin`` and ``p_sdout``.
The preceding diagram shows the connectivity of ports and clock blocks.

``p_sdin`` and ``p_sdout`` are configured as buffered ports with a transfer width of 32, so all 32 bits are
input in one input statement. This allows the software to input, process and output 32-bit words, whilst the ports serialize and
deserialize to the single I/O pin connected to each port.

Unlike previous xCORE architectures, xCORE-200 (XS2) and xCORE.ai (XS3) series devices have the ability to divide an external clock in a clock-block.

The bit clock outputs 32 clock cycles per sample. In the special case where the divide is 1 (i.e. the bit clock frequency equals 
the master clock frequency), the ``p_bclk`` port is set to a special mode where it simply outputs its clock input (i.e. ``p_mclk``).  
See ``configure_port_clock_output()`` in ``xs1.h`` for details.

``p_lrclk`` is clocked by ``p_bclk``. In I2S mode the port outputs the pattern ``0x7fffffff``
followed by ``0x80000000`` repeatedly. This gives a signal that has a transition one bit-clock
before the data (as required by the I2S standard) and alternates between high and low for the left 
and right channels of audio.

Changing Audio Sample Frequency
-------------------------------

.. _usb_audio_sec_chang-audio-sample:

When the host changes sample frequency, a new frequency is sent to
the audio driver core by Endpoint 0 (via the buffering cores and mixer).

First, a change of sample frequency is reported by sending the new frequency over an XC channel. The audio core
detects this by checking for the presence of a control token on the channel channel 

Upon receiving the change of sample frequency request, the audio
core stops the I2S/TDM interface and calls the CODEC/port configuration 
functions. 

Once this is complete, the I2S/TDM interface (i.e. the main loop in AudioHub) is restarted at the new frequency.

