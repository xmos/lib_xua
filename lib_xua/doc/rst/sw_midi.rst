|newpage|

MIDI
====

The MIDI core implements a 31250 baud UART (8-N-1) for both input and output. It uses a single dedicated thread which performs multiple functions:

    - UART Tx peripheral.
    - UART Tx FIFO of 1024 bytes (may be configured by the user).
    - Decoding of USB MIDI message to bytes.
    - UART Rx peripheral.
    - Packing of received MIDI bytes into USB MIDI messages/events.

It is connected via a channel to the Endpoint Buffer core meaning that it can be placed on any XCORE tile in the system subject to resource availability.

The Endpoint Buffer core implements the two Bulk endpoints (one In and one Out) as well as interacting with small, shared-memory, FIFOs for each endpoint.

On receiving 32-bit USB MIDI events from the Endpoint Buffer core, the MIDI core parses these and translates them to 8-bit MIDI messages which are sent
out over the UART. Up to 1024 bytes may be buffered by the MIDI task for outgoing messages in the default configuration. If the outgoing buffer is full then it will cause the USB endpoint to be NACKed which provides flow control in the case that the host application sends messages too fast. This is important because the USB bandwidth far exceeds the MIDI UART bandwidth by many orders of magnitude. The combination of buffering and flow control ensures outgoing messages are not dropped during normal operation.

Incoming 8-bit MIDI messages from the UART are packed into 32-bit USB MIDI events and passed on to the Endpoint Buffer core. Since the rate of ingress
to the MIDI port is tiny in comparison to the host USB bandwidth, no buffering is required and the MIDI events are always forwarded on directly to USB immediately.

All MIDI message types are supported including `Sysex` (MIDI System Exclusive) strings allowing custom function such as bank updates and patches, backup and device firmware upgrade (DFU) where supported by the MIDI device.

The MIDI core is implemented in the file ``usb_midi.xc`` and the USB buffering is handled in the file ``ep_buffer.xc``.


