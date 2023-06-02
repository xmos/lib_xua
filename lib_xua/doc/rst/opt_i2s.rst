|newpage|

I2S/TDM
=======

I2S/TDM is typically fundamental to most products and is built into the ``XUA_AudioHub()`` core.

The defines in :ref:`opt_i2s_defines` effect the I2S implementation. 

.. tabularcolumns:: lp{5cm}l
.. _opt_i2s_defines:
.. list-table:: I2S defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``I2S_CHANS_DAC``
     - The desired number of output channels via I2S (0 for disabled)
     - N/A (Must be defined)
   * - ``I2S_CHANS_ADC``
     - The desired number of input channels via I2S (0 for disabled)
     - N/A (Must be defined)
   * - ``XUA_PCM_FORMAT``
     - Enabled either TDM or I2S mode
     - ``XUA_PCM_FORMAT_I2S``
   * - ``CODEC_MASTER``
     - Sets is xCORE is I2S master or slave
     - ``0`` (xCORE is master)

The I2S code expects that the ports required for I2S (master clock, LR-clock, bit-clock and data lines) are be defined in the application XN file in the relevant `Tile``.  
For example::
          
    <Tile Number="0" Reference="tile[0]">
        <Port Location="XS1_PORT_1A"  Name="PORT_MCLK_IN"/>
        <Port Location="XS1_PORT_1B"  Name="PORT_I2S_LRCLK"/>
        <Port Location="XS1_PORT_1C"  Name="PORT_I2S_BCLK"/>
        <Port Location="XS1_PORT_1D"  Name="PORT_I2S_DAC0"/>
        <port Location="XS1_PORT_1E"  Name="PORT_I2S_DAC1"/>
        <Port Location="XS1_PORT_1F"  Name="PORT_I2S_ADC0"/>
        <Port Location="XS1_PORT_1G"  Name="PORT_I2S_ADC1"/>
    </Tile>

All of the I2S related ports must be 1-bit ports.

.. note:: 

    TDM mode allows 8 channels (rather than 2) to be supplied on each dataline.
