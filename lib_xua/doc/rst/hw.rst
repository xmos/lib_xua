
XMOS USB Audio Hardware Platforms
---------------------------------

A range of hardware platforms for evaluating USB Audio on XMOS devices.

Specific details for each platform/board are out of scope of this library documentation however, the features of the most popular platform are described below with the view of providing a worked example. 

Please also see application note AN00246.

xCORE.AI Multichannel Audio Board
...................................

The XMOS xCORE.ai Multichannel Audio board (XK-AUDIO-316-MC) is a complete hardware and reference software platform targeted at up to 32-channel USB audio applications, such as DJ decks and mixers and other musical instrument interfaces.  The board can also be used to prototype products with smaller feature sets or HiFi style products.

The Multichannel Audio Platform hardware is based around the XU316-1024-TQ128-C24 multicore microcontroller; an dual-tile xCORE.ai device with an integrated High Speed USB 2.0 PHY and 16 logical cores delivering up to 3200MIPS of deterministic and responsive processing power.

Exploiting the flexible programmability of the xCORE.ai architecture, the Multi-channel Audio Platform supports either USB or network audio source, streaming 8 analogue input and 8 analogue output audio channels simultaneously - at up to 192kHz.

The reference board has an associated firmware application that uses `lib_xua` to implemented a USB Audio Device. Full details of this application can be found later in this document.

Analogue Input & Output
+++++++++++++++++++++++

A total of eight single-ended analog input channels are provided via 3.5mm stereo jacks. These inputs feed into a pair of quad-channel PCM1865 ADCs from Texas Instruments.

A total of eight single-ended analog output channels are provided. These a fed from a for PCM5122 stereo DAC's from Texas instruments.

ADC's and DAC's are configured via an I2C bus.

The four digital I2S/TDM input and output channels are mapped to the xCORE input/outputs through a header array. These jumpers allow channel selection when the ADC/DAC is used in TDM mode

Digital Input & Output
++++++++++++++++++++++

Optical and coaxial digital audio transmitters are used to provide digital audio input output in formats such as IEC60958 consumer mode (S/PDIF) and ADAT.
The output data streams from the xCORE are re-clocked using the external master clock to synchronise the data into the audio clock domain. This is achieved using simple external D-type flip-flops.

MIDI
++++

MIDI I/O is provided on the board via standard 5-pin DIN connectors. The signals are buffered using 5V line drivers and are then connected to 1-bit ports on the xCORE, via a 5V to 3.3V buffer.

Audio Clocking
++++++++++++++

In order to accommodate a multitude of clocking options a flexible clocking scheme is provided for the audio subsystem.

Three methods of generating an audio master clock are provided on the board:

    * A Cirrus Logic CS2100-CP PLL device.  The CS2100 features both a clock generator and clock multiplier/jitter reduced clock frequency synthesizer (clean up) and can generate a low jitter audio clock based on a synchronisation signal provided by the xCORE

    * A Skyworks Si5351B PLL device. The Si5351 is an I2C configurable clock generator that is ideally suited for replacing crystals, crystal oscillators, VCXOs, phase-locked loops (PLLs), and fanout buffers.

    * xCORE.ai devices are equipped with a secondary (or 'application') PLL which can be used to generate audio clocks

Selection between these methods is done via writing to bits 6 and 7 of PORT 8D on tile[0]. 

Either the locally generated clock (from the PL611) or the recovered low jitter clock (from the CS2100) may be selected to clock the audio stages; the xCORE-200, the ADC/DAC and Digital output stages. Selection is controlled via an additional I/O, bit 5 of PORT 8C, see :ref:`hw_316_ctrlport`.

.. _hw_316_ctrlport:

Control I/O
+++++++++++

4 bits of PORT 8C are used to control external hardware on the board. This is described in :ref:`table_316_ctrlport`.

.. _table_316_ctrlport:

.. table:: PORT 8C functionality
    :class: horizontal-borders vertical_borders

    +--------+-----------------------------------------+------------+------------+
    | Bit(s) | Functionality                           |    0       |     1      |
    +========+=========================================+============+============+
    | [0:3]  | Unused                                  |            |            |
    +--------+-----------------------------------------+------------+------------+
    | 4      | Enable 3v3 power for digital (inverted) |  Enabled   |  Disabled  |
    +--------+-----------------------------------------+------------+------------+
    | 5      | Enable 3v3 power for analogue           |  Disabled  |  Enabled   |
    +--------+-----------------------------------------+------------+------------+
    | 6      | PLL Select                              |   CS2100   |   Si5351B  |
    +--------+-----------------------------------------+------------+------------+
    | 7      | Master clock direction                  |   Output   |   Input    |
    +--------+-----------------------------------------+------------+------------+


.. note::
     
    To use the xCORE application PLL bit 7 should be set to 0. To use one of the external PLL's bit 7 should be set to 1. 


LEDs, Buttons and Other IO
++++++++++++++++++++++++++

All programmable I/O on the board is configured for 3v3.

For green LED's and three push buttons are provided for general purpose user interfacing. 

The LEDs are connected to PORT 4F and the buttons are connected to bits [0:2] of PORT 4E. Bit 3 of this port is connected to the (currently
unused) ADC interrupt line.

The board also includes support for an AES11 format Word Clock input via 75 ohm BNC. The software does not support this currently and it is
provided for future expansion.

All spare IO and Functional IO brought out on headers for easy connection of expansion boards (via 0.1” headers).

Power
+++++

The board is capable of acting as a USB2.0 self or bus powered device. If bus powered, board takes power from  ``USB DEVICE`` connector (micro-B receptacle). 
If self powered, board takes power from ``EXTERNAL POWER`` input (micro-B receptacle).

A Power Source Select (marked ``PWR SRC``) is used to select between bus and self-powered configuration. 


Debug
+++++

For convenience the board includes an on-board xTAG4 for debugging via JTAG/xSCOPE. This is accessed via the USB (micro-B) receptacle marked ``DEBUG``. 

