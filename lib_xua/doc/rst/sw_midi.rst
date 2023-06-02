|newpage|

MIDI
====

The MIDI core implements a 31250 baud UART for both input and output. On receiving 32=bit USB MIDI events
from the Endpoint Buffer core, it parses these and translates them to 8-bit MIDI messages which are sent
over UART. Similarly, incoming 8-bit MIDI messages are aggregated into 32-bit USB MIDI events and
passed on to the Endpoint Buffer core. The MIDI core is implemented in the file ``usb_midi.xc``.

The Endpoint Buffer core implements the two Bulk endpoints (one In and one Out) as well as interacting 
with small, shared-memory, FIFOs for each endpoint.

