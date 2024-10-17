
|newpage|

.. _usb_audio_sec_clock_recovery:

External Clock Recovery (Clock Gen)
===================================

To provide an audio master clock an application may use selectable oscillators, clock
generation IC or, in the case of xCORE.ai devices, an integrated secondary PLL, to generate fixed
master clock frequencies.

It may also use an external PLL/Clock Multiplier to generate a master clock based on a reference from
the xCORE.

Using an external PLL/Clock Multiplier allows an Asynchronous mode design to lock to an external
clock source from a digital stream (e.g. S/PDIF or ADAT input).  The codebase supports the Cirrus
Logic CS2100 device or use of lib_sw_pll (xcore.ai only) for this purpose. Other devices may be
supported via code modification.

The Clock Recovery core (Clock Gen) is responsible for either generating the reference frequency
to the CS2100 device or driving lib_sw_pll from time measurements based on the local master clock
and the time of received samples. Clock Gen (via CS2100 or lib_sw_pll) generates the master clock
used over the whole design. This core also serves as a smaller buffer between ADAT and S/PDIF
receiving cores and the Audio Hub core.

When using lib_sw_pll (xcore.ai only) a further core is instantiated which performs the sigma-delta
modulation of the xCORE PLL to ensure the lowest jitter over the audio band. See lib_sw_pll
documentation for further details.

When running in *Internal Clock* mode this core simply generates this clock using a local
timer, based on the XMOS reference clock.

When running in an external clock mode (i.e. S/PDIF Clock" or "ADAT Clock" mode) samples are
received from the S/PDIF and/or ADAT receive core. The external frequency is calculated through
counting samples in a given period. Either the reference clock to the CS2100 is then generated based on
the reception of these samples or the timing information is provided to lib_sw_pll to generate
the phase-locked clock on-chip (xcore.ai only).

If an external stream becomes invalid, the *Internal Clock* timer event will fire to ensure that
valid master clock generation continues regardless of cable unplugs etc. Efforts are made to
ensure that the transitions between these clocks are relatively seamless. Additionally efforts are also
made to try and keep the jitter on the reference clock as low as possible, regardless of activity
level of the Clock Gen core. The is achieved though the use of port times to schedule pin toggling
rather than directly outputting to the port in the case of using the CS2100. For lib_sw_pll cases the
last setting is kept for the sigma-delta modulator ensuring clock continuity.

The Clock Gen core gets clock selection Get/Set commands from Endpoint 0 via the ``c_clk_ctl``
channel.  This core also records the validity of external clocks, which is also queried
through the same channel from Endpoint 0. Note, the *Internal Clock* is always reported as being
valid. It should be noted that the device always reports the current device sample rate regardless
of the clock being interrogated. This results in improved user experience for most driver/operating
system combinations

To inform the host of any status change, the Clock Gen core can also cause the Decouple core to
request an interrupt packet on change of clock validity.  This functionality is based on the Audio
Class 2.0 status/interrupt endpoint feature.

.. note::

   When running in Synchronous mode external digital input streams are currently not supported.
   Such a feature would require sample-rate conversion to covert from the S/PDIF or ADAT clock
   domain to the USB host clock domain. As such this core is not used in a Synchronous mode device.
