I2S/TDM
~~~~~~~

I2S/TDM is typically fundamental to most products and is built into the ``XUA_AudioHub()`` core.

The following defines effect the I2S implementation. 

.. _opt_i2s_defines:

.. list-table:: I2S defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - I2S_CHANS_DAC
     - The desired number of output channels via I2S (0 for disabled)
     - N/A (Must be defined)
   * - I2S_CHANS_ADC
     - The desired number of input channels via I2S (0 for disabled)
     - N/A (Must be defined)
   * - XUA_PCM_FORMAT
     - Enabled either TDM or I2S mode
     - XUA_PCM_FORMAT_I2S
   * - CODEC_MASTER
     - Sets is xCORE is I2S master or slave
     - 0 (xCORE is master)



