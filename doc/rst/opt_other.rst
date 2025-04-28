|newpage|

Other options
=============

There are a few other, lesser used, options available. These are shown in :numref:`opt_other_defines`.

|beginfullwidth|

.. _opt_other_defines:

.. list-table:: Other defines
   :header-rows: 1
   :widths: 40 80 20

   * - Define
     - Description
     - Default
   * - ``XUA_USB_EN``
     - Allows the use of the audio subsytem without USB
     - ``1`` (enabled)
   * - ``INPUT_VOLUME_CONTROL``
     - Enables volume control on input channels, both descriptors and processing
     - ``1`` (enabled)
   * - ``OUTPUT_VOLUME_CONTROL``
     - Enables volume control on output channels, both descriptors and processing
     - ``1`` (enabled)
   * - ``CHAN_BUFF_CTRL``
     - Enables event based communication between XUA_Buffer_Ep() and XUA_Buffer_Decouple()
       which significantly reduces power consumption (approx 40 mW on xcore.ai) at the cost
       of consuming two extra channel-ends. Consequently this option may not be viable on
       some high end configurations which feature multiple digital interfaces such as SPDIF
       or ADAT.
     - ``0`` (disabled)


|endfullwidth|
