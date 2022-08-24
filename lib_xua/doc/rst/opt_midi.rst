MIDI
~~~~

The codebase supports MIDI input/output over USB as per `Universal Serial Bus Device Class Definition for MIDI Devices <https://www.usb.org/sites/default/files/midi10.pdf>`_.

MIDI functionality is enabled with the following define.

.. _opt_midi_defines:

.. list-table:: MIDI Enable Define
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - MIDI
     - Enable MIDI functionality
     - 0 (Disabled)


The codebase supports MIDI receive on a 4-bit or 1-bit port - the default is using a 1-bit port. MIDI transmit is supported on any width port.
By default the codebase assumes the transmit and receive I/O is connected to bit[0] of the port. This is configurable for the transmit port.

.. _opt_midi_port_defines:

.. list-table:: MIDI Port Defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - MIDI_RX_PORT_WIDTH
     - Port width of the MIDI rx port (1 or 4bit)
     - 1 (1-bit port) 
   * - MIDI_SHIFT_TX
     - MIDI tx bit 
     - 0 (bit[0]) 

