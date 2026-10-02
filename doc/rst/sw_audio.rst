|newpage|

.. _usb_audio_sec_audio:

Audio Hub and I²S
=================

The ``AudioHub`` task performs many functions. It receives and transmits samples from/to the Decoupler
or Mixer thread over a channel.

It also drives several in and out I²S/TDM channels to/from a CODEC, DAC, ADC etc. From now on these
external devices will be termed "audio hardware".

If the firmware is configured with the `xcore` as I²S master the required clock lines will also be
driven from this task. It also has the task of forwarding on and receiving samples to/from other
audio related tasks/threads such as S/PDIF tasks, ADAT etc.

In master mode, the `xcore` generates the I²S "Continuous Serial Clock" (SCK), or "Bit-Clock" (BCLK)
and the "Word Select" (WS) or "Left-Right Clock" (LRCLK) signals. Any CODEC or DAC/ADC combination
that supports I²S and can be used.

The LR-clock, bit-clock and data are all derived from the incoming master clock (typically the
output of the external oscillator or PLL). This is not part of the I²S standard but is commonly
included for synchronizing the internal operation of the analog/digital converters.

The Audio Hub task is implemented in the file ``xua_audiohub.xc``.

:numref:`usb_audio_codec_signals` shows the signals used to communicate audio between the XMOS device
and the external audio hardware.

.. _usb_audio_codec_signals:

.. list-table:: I²S Signals
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

In the case where the `xcore` is the master, it divides the MCLK to generate the required signals for both BCLK and LRCLK,
with BCLK then being used to clock data in (SDIN) and data out (SDOUT) of the external audio hardware.

:numref:`usb_audio_example_clock_divides` shows some example clock frequencies and divides for different sample rates:

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

For `xcore-200` devices the master clock must be supplied by an external source e.g. clock generator,
fixed oscillators, PLL etc. `xcore.ai` devices may use the integrated secondary PLL.

Two master clock frequencies are required to support 44.1 kHz and 48 kHz audio frequencies (e.g. 11.2896/22.5792MHz
and 12.288/24.576MHz respectively).  This master clock input is then provided to the external audio
hardware and the `xcore` device.

Port configuration (xcore master)
---------------------------------

The default software configuration is of `xcore` being the I²S master.
That is, the `xcore` device provides the BCLK and LRCLK signals to the external audio hardware

`xcore` ports and clocks provide many valuable features for implementing I²S.
This section describes how these are configured and used to drive the I²S interface.

.. only:: latex

  .. _i2s_ports_and_clocks:

  .. figure:: images/port_config.pdf

    Ports and clocks (`xcore` master)

.. only:: html

  .. figure:: images/port_config.png

    Ports and clocks (`xcore` master)

The code to configure the ports and clocks is in the ``ConfigAudioPorts()`` function. Developers should not need to modify this.

The `xcore` inputs MCLK and divides it down to generate BCLK and LRCLK.

To achieve this MCLK is input into the device using the 1-bit port ``p_mclk``.
This is attached to the clock block ``clk_audio_bclk``. This clock block is configured to divide the external MCLK signal to the desired
BCLK rate.

The ``p_bclk`` port is clocked from ``clk_audio_bclk`` and is set to a mode where it simply outputs
the clock output of ``clk_audio_bclk``.
See ``configure_port_clock_output()`` in ``xs1.h`` for details.

``p_i2s_dac``, ``p_i2s_adc`` and ``p_lrclk`` are all clocked by ``clk_audio_bclk``.
This makes them indirectly synchronous with the external BCLK signal because ``p_bclk`` outputs the
clock from the same clock block.

The I²S data ports are configured as buffered ports with a transfer width of 32, so all 32 bits are
input/output in one statement. This allows the software to input, process and output 32-bit words,
whilst the ports serialize and deserialize to the single I/O pin connected to each port.

In I²S mode the ``p_lrclk`` port outputs a high or low pattern for ``XUA_I2S_N_BITS`` BCLK cycles per channel.
For stereo I²S this divides BCLK by ``2 * XUA_I2S_N_BITS``: by 32 for 16-bit slots, or by 64 for 32-bit slots.
This gives a signal that transitions one bit-clock before the data (as required by the I²S standard),
alternating between high and low for the left and right channels of audio.

Changing audio sample frequency
-------------------------------

.. _usb_audio_sec_chang-audio-sample:

When the host changes sample frequency, a new frequency is sent to
the audio driver thread by Endpoint 0 (via the buffering threads and mixer).

First, a change of sample frequency is reported by sending the new frequency over an XC channel. The audio thread
detects this by checking for the presence of a control token on the channel channel

Upon receiving the change of sample frequency request, the audio
thread stops the I²S/TDM interface and calls the CODEC/port configuration
functions.

Once this is complete, the I²S/TDM interface (i.e. the main loop in ``AudioHub``) is restarted at the new frequency.
