
|newpage|

MIDI
====

The codebase supports MIDI input/output over USB as per `Universal Serial Bus Device Class Definition for MIDI Devices <https://www.usb.org/sites/default/files/midi10.pdf>`_.

MIDI functionality is enabled with the define in :ref:`opt_midi_defines`.

.. _opt_midi_defines:

.. list-table:: MIDI enable define
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``MIDI``
     - Enable MIDI functionality
     - ``0`` (Disabled)


The codebase supports MIDI receive on a 4-bit or 1-bit port, defaulting to using a 1-bit port. 
MIDI transmit is supported  port of any bit-width.  By default the codebase assumes the transmit
and receive I/O is connected to bit[0] of the port. This is configurable for the transmit port.
:ref:`opt_midi_defines` provides information on the configuring these parameters.

.. _opt_midi_port_defines:

.. list-table:: MIDI port defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``MIDI_RX_PORT_WIDTH``
     - Port width of the MIDI rx port (1 or 4bit)
     - ``1`` (1-bit port) 
   * - ``MIDI_SHIFT_TX``
     - MIDI tx bit 
     - ``0`` (bit[0]) 

The MIDI code expects that the ports for receive and transmit are defined in the application XN file in the relevant Tile.  
The expected names for the ports are ``PORT_MIDI_IN`` and ``PORT_MIDI_OUT``, for example::
          
    <Tile Number="0" Reference="tile[0]">
        <!-- MIDI -->
        <Port Location="XS1_PORT_1F"  Name="PORT_MIDI_IN"/>
        <Port Location="XS1_PORT_4C"  Name="PORT_MIDI_OUT"/> 
    </Tile>
    
