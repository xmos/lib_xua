|newpage|

I2S/TDM
=======

I2S/TDM is typically fundamental to most products and is built into the ``XUA_AudioHub()`` core.

In order to enable I2S/TDM on must declare an array of ports for the data-lines (one for each direction)::

    /* Port declarations. Note, the defines come from the XN file */
    buffered out port:32 p_i2s_dac[] = {PORT_I2S_DAC0}; /* I2S Data-line(s) */
    buffered in port:32 p_i2s_adc[]  = {PORT_I2S_ADC0}; /* I2S Data-line(s) */

Ports for the sample and bit clocks are also required::

    buffered out port:32 p_lrclk        = PORT_I2S_LRCLK; /* I2S Bit-clock */
    buffered out port:32 p_bclk         = PORT_I2S_BCLK;  /* I2S L/R-clock */

.. note::

    All of these ports must be 1-bit ports, 32-bit buffed. Based on whether the xCORE is bus slave/master the ports must be declared as input/output respectively

These ports must then be passed to the ``XUA_AudioHub()`` task appropriately.

I2S/TDM functionality also requires two clock-blocks, one for bit-clock and another for the master clock e.g.::

    /* Clock-block declarations */
    clock clk_audio_bclk = on tile[0]: XS1_CLKBLK_4; /* Bit clock */
    clock clk_audio_mclk = on tile[0]: XS1_CLKBLK_5; /* Master clock */

These hardware resources must be passed into the call to ``XUA_AudioHub()``::

    /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves 
     * as a hub for all audio) */

    on tile[0]: XUA_AudioHub(c_aud, clk_audio_mclk, clk_audio_bclk, p_mclk_in, 
        p_lrclk, p_bclk, p_i2s_dac, p_i2s_adc);

