|newpage|

Channel Counts and Sample Rates
===============================

The codebase is fully configurable in relation to channel counts and sample rates.
Practical limitations of these are normally based on USB packet size restrictions and I/O
availablity.

For example, the maximum packet size for high-speed USB is 1024 bytes, limiting the channel count 
to 10 channels for a device running at 192kHz with 32bit sample depth.

The defines in :ref:`opt_channel_defines` set the channel counts exposed to the USB host.

.. tabularcolumns:: lp{5cm}l
.. _opt_channel_defines:
.. list-table:: Channel count defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``NUM_USB_CHAN_OUT``
     - Number of output channels the device advertises to the USB host 
     - N/A (must be defined) 
   * - ``NUM_USB_CHAN_IN``
     - Number of input channels the device advertises to the USB host 
     - N/A (must be defined) 

Sample rates ranges are set by the defines in :ref:`opt_channel_sr_defines`. The codebase will 
automatically populate the device sample rate list with popular frequencies between the min and 
max values. All values are in Hz:

.. tabularcolumns:: lp{5cm}l
.. _opt_channel_sr_defines:
.. list-table:: Sample rate defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``MAX_FREQ``
     - Maximum supported sample rate (Hz)
     - ``192000``
   * - ``MIN_FREQ``
     - Minimum supported sample rate (Hz)
     - ``44100``
   * - ``DEFAULT_FREQ``
     - Starting frequency for the device after boot
     - ``MIN_FREQ``


The codebase requires knowledge of the two master clock frequencies that will be present on the 
master-clock port(s). One for 44.1kHz, 88.2kHz etc and one for 48kHz, 96kHz etc.  These are set
using defines in :ref:`opt_channel_mc_defines`. All values are in Hz.

.. tabularcolumns:: lp{5cm}l
.. _opt_channel_mc_defines:
.. list-table:: Master clock rate defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``CLK_441``
     - Master clock defines for 44100 rates (Hz)
     - ``(256 * 44100)``
   * - ``MCLK_48``
     - Master clock defines for 48000 rates (Hz)
     - ``(256 * 48000)``
