|newpage|

Mixer
=====

``lib_xua`` supports audio mixing functionality with highly flexible routing options.

Essentially the mixer is capable of performing 8 separate mixes with up to 18 inputs at sample rates
up to 96kHz and 2 mixes with up to 18 inputs at higher sample rates.

Inputs to the mixer can be selected from any device input (USB, S/PDIF, I2S etc) and
outputs from the mixer can be routed to any device output (USB, S/PDIF, I2S etc).

See :ref:`usb_audio_sec_mixer` for full details of the mixer including control.

Basic configuration of mixer functionality is achieved with the the defines
:numref:`opt_mixer_defines`.

.. _opt_mixer_defines:

.. list-table:: Mixer defines
   :header-rows: 1

   * - Define
     - Description
     - Default
   * - ``XUA_MIXER_EN``
     - Enable mixer
     - ``0`` (Disabled)
   * - ``XUA_MAX_MIX_COUNT``
     - Number of separate mix outputs to perform
     - ``8``
   * - ``XUA_MIX_INPUTS``
     - Number of channels input into the mixer
     - ``18``

.. note::

   The mixer configuration defines were renamed from the historical
   unprefixed names ``MIXER``, ``MAX_MIX_COUNT``, and ``MIX_INPUTS``.
   Existing applications using the old names remain supported for
   compatibility, but new configurations should use the ``XUA_``-prefixed
   names.

.. note::

   The mixer threads always run on the tile defined by ``XUA_AUDIO_IO_TILE_NUM``

