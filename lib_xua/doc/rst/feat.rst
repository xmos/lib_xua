
Features & Options
------------------

The previous sections describes only the basic core set of ``lib_xua`` details on enabling additional features e.g. S/PDIF are discussed in this section.

Where something must be defined, it is recommened this is done in `xua_conf.h` but could also be done in the application Makefile.

For each feature steps are listed for if calling ``lib_xua`` functions manually - if using the "codeless" programming model then these steps informational only. 
Each section also includes a sub-section on enabling the feature using the "codeless" model.

For full details of all options please see the API section

I2S/TDM
~~~~~~~

I2S/TDM is typically fundamental to most products and is built into the ``XUA_AudioHub()`` core.

In order to enable I2S on must declare an array of ports for the data-lines (one for each direction)::

    /* Port declarations. Note, the defines come from the xn file */
    buffered out port:32 p_i2s_dac[]    = {PORT_I2S_DAC0};   /* I2S Data-line(s) */
    buffered in port:32 p_i2s_adc[]    	= {PORT_I2S_ADC0};   /* I2S Data-line(s) */

Ports for the sample and bit clocks are also required::

    buffered out port:32 p_lrclk        = PORT_I2S_LRCLK;    /* I2S Bit-clock */
    buffered out port:32 p_bclk         = PORT_I2S_BCLK;     /* I2S L/R-clock */

.. note::

    All of these ports must be buffered, width 32. Based on whether the xCORE is bus slave/master the ports must be declared as input/output respectively

These ports must then be passed to the ``XUA_AudioHub()`` task appropriately.

I2S functionality also requires two clock-blocks, one for bit and sample clock e.g.::

    /* Clock-block declarations */
    clock clk_audio_bclk                = on tile[0]: XS1_CLKBLK_4;   /* Bit clock */
    clock clk_audio_mclk                = on tile[0]: XS1_CLKBLK_5;   /* Master clock */

These hardware resources must be passed into the call to ``XUA_AudioHub()``::

    /* AudioHub/IO core does most of the audio IO i.e. I2S (also serves as a hub for all audio) */
    on tile[0]: XUA_AudioHub(c_aud, clk_audio_mclk, clk_audio_bclk, p_mclk_in, p_lrclk, p_bclk);


Codeless Programming Model
..........................

All ports and hardware resources are already fully declared, one must simply set the following:

    * `I2S_CHANS_DAC` must be set to the desired number of output channels via I2S
    * `I2S_CHANS_ADC` must be set to the desired number of input channels via I2S
    * `AUDIO_IO_TILE` must be set to the tile where the physical I2S connections reside 
    
For configuration options, master vs slave, TDM etc please see the API section.


|newpage|

S/PDIF Transmit
~~~~~~~~~~~~~~~

``lib_xua`` supports the development of devices with S/PDIF transmit functionality through the use of 
``lib_spdif``. The XMOS S/PDIF transmitted runs in a single core and supports rates up to 192kHz.

The S/PDIF transmitter core takes PCM audio samples via a channel and outputs them
in S/PDIF format to a port. 

Samples are provided to the S/PDIF transmitter task from the ``XUA_AudioHub()`` task.

In order to use the S/PDIF transmmiter with ``lib_xua`` hardware resources must be declared e.g::

    buffered out port:32 p_spdif_tx         = PORT_SPDIF_OUT;     /* SPDIF transmit port */


For further details please see the documentation, application notes and examples provided for ``lib_spdif``.

Codeless Programming Model
..........................

If using the codeless programming method one must simply ensure the following:

    * `PORT_SPDIF_OUT` is correctly defined in the XN file
    * `XUA_SPDIF_TX_EN` should be defined as non-zero
    * `SPDIF_TX_TILE` is correctly defined (note, this defaults to `AUDIO_IO_TILE`)

For further configuration options please see the API section.



