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
     - Allows the use of the audio subsystem without USB
     - ``1`` (enabled)
   * - ``INPUT_VOLUME_CONTROL``
     - Enables volume control on input channels, both descriptors and processing
     - ``1`` (enabled)
   * - ``OUTPUT_VOLUME_CONTROL``
     - Enables volume control on output channels, both descriptors and processing
     - ``1`` (enabled)
   * - ``XUA_CHAN_BUFF_CTRL``
     - Enables event based communication between XUA_Buffer_Ep() and XUA_Buffer_Decouple()
       which significantly reduces power consumption (approx 40 mW on xcore.ai) at the cost
       of consuming two extra channel-ends. Consequently this option may not be viable on
       some high end configurations which feature multiple digital interfaces such as SPDIF
       or ADAT.
     - ``0`` (disabled)
   * - ``XUA_USER_IN_ENDPOINTS``
     - Allows additional input USB endpoints to be declared. Endpoints must be initialised using code
       from the ``xua_user_endpoint_init.h`` include file.
     - Undefined
   * - ``XUA_USER_OUT_ENDPOINTS``
     - Allows additional output USB endpoints to be declared. Endpoints must be initialised using code
       from the ``xua_user_endpoint_init.h`` include file.
     - Undefined
   * - ``XUA_USER_INTERFACES``
     - Used for inserting interfaces into descriptor for composite devices. Note - 
       This define must be able to be compiled under ``C``.
     - Undefined


.. note:: When extending the descriptors to add user interfaces, the descriptors must also be extended to 
          tell the host what kind of device to expect. There are three optional include files which may
          be used to insert code into user descriptors. The expected files in your
          project are ``xua_user_descriptors_incl.h`` to add any additional include files,
          ``xua_user_descriptors_decl.h`` to make the descriptor declarations and ``xua_user_descriptors_content.h``
          to populate them. The include files must be able to be compiled under ``C``.

.. note:: In addition to adding user interfaces and descriptors, endpoint 0 handlers and endpoint initialisation must
          be supplied. The expected files in your project are ``xua_user_endpoint0_decl.h`` to make declarations
          (for example external function prototype), ``xua_user_endpoint0_handler.h`` to provide handling code for
          the extensions to endpoint 0 and ``xua_user_endpoint_init.h`` where the additionally declared endpoints
          are initialised. The include files for endpoint 0 must be able to be compiled under ``C``.

|endfullwidth|
