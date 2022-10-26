|newpage|

Mixer
=====

The codebase supports audio mixing functionality with highly flexible routing options. 

Essentially the mixer is capable of performing 8 separate mixes with up to 18 inputs at sample rates 
up to 96kHz and 2 mixes with up to 18 inputs at higher sample rates. 

Inputs to the mixer can be selected from any device input (USB, S/PDIF, I2S etc) and 
outputs from the mixer can be routed to any device output (USB, S/PDIF, I2S etc).

See :ref:`usb_audio_sec_mixer` for full details of the mixer including control.

Basic configuration of mixer functionality is achieved with the defines in :ref:`opt_mixer_defines`.

.. _opt_mixer_defines:

.. list-table:: Mixer defines
   :header-rows: 1
   :widths: 20 80 20

   * - Define
     - Description
     - Default
   * - ``MIXER``
     - Enable mixer
     - ``0`` (Disabled)
   * - ``MAX_MIX_COUNT``
     - Number of separate mix outputs to perform
     - ``8``
   * - ``MIX_INPUTS``
     - Number of channels input into the mixer
     - ``18``

.. note::

   The mixer cores always run on the tile defined by ``AUDIO_IO_TILE``


