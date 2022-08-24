|newpage|

Channel Counts and Sample Rates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following defines set the channel counts for the device.

.. _opt_channel_defines:

.. list-table:: Channel count defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``NUM_USB_CHAN_OUT``
     - Number of output channels the device advertises to the usb host 
     - N/A (must be defined) 
   * - ``NUM_USB_CHAN_OUT``
     - Number of output channels the device advertises to the usb host 
     - N/A (must be defined) 

Sample rates ranges are controlled by the following defines. All values are in Hz:

.. list-table:: Sample rate defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``MAX_FREQ``
     - Maximum supported sample rate as reported to the USB host
     - ``192000``
   * - ``MIN_FREQ``
     - Minimum supported sample rate as reported to the USB host
     - ``44100``
   * - ``DEFAULT_FREQ``
     - Starting frequency for the device when it boots
     - ``MIN_FREQ``


The codebase requires knowledge of the two master clock frequencies that will be present on the 
master-clock port(s). One for 44.1kHz, 88.2kHz etc and one for 48kHz, 96kHz etc.  All values are in Hz.

.. list-table:: Master clock rate defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``CLK_441``
     - Master clock defines for 44100 rates (in Hz)
     - ``(256 * 44100)``
   * - ``MCLK_48``
     - Master clock defines for 48000 rates (in Hz)
     - ``(256 * 48000)``
