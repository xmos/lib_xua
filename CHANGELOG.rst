sc_usb_audio Change Log
=======================

6.15.2
------
    - RESOLVED:  interrupt.h (used in audio buffering) now compatible with xCORE-200 ABI

6.15.1
------
    - RESOLVED:   DAC data mis-alignment issue in TDM slave mode
    - CHANGE:     Updates to support API changes in lib_mic_array version 2.0

6.15.0
------
   
    - RESOLVED:   UAC 1.0 descriptors now support multi-channel volume control (previously were 
                  hard-coded as stereo)
    - CHANGE:     Removed 32kHz sample-rate support when PDM microphones enabled (lib_mic_array
                  currently does not support non-integer decimation factors)

6.14.0
------
    - ADDED:      Support for for master-clock/sample-rate divides that are not a power of 2 
                  (i.e. 32kHz from 24.567MHz) 
    - ADDED:      Extended available sample-rate/master-clock ratios. Previous restriction was <= 
                  512x (i.e. could not support 1024x and above e.g. 49.152MHz MCLK for Sample Rates 
                  below 96kHz) (#13893)
    - ADDED:      Support for various "low" sample rates (i.e. < 44100) into UAC 2.0 sample rate
                  list and UAC 1.0 descriptors
    - ADDED:      Support for the use and integration of PDM microphones (including PDM to PCM 
                  conversion) via lib_mic_array 
    - RESOLVED:   MIDI data not accepted after "sleep" in OSX 10.11 (El Capitan) - related to sc_xud
                  issue #17092
    - CHANGE:     Asynchronous feedback system re-implemented to allow for the first two ADDED
                  changelog items 
    - CHANGE:     Hardware divider used to generate bit-clock from master clock (xCORE-200 only). 
                  Allows easy support for greater number of master-clock to sample-rate ratios.
    - CHANGE:     module_queue no longer uses any assert module/lib

6.13.0
------
    - ADDED:      Device now uses implicit feedback when input stream is available (previously explicit
                  feedback pipe always used). This saves chanend/EP resources and means less processing 
                  burden for the host. Previous behaviour available by enabling UAC_FORCE_FEEDBACK_EP
    - RESOLVED:   Exception when SPDIF_TX and ADAT_TX both enabled due to clock-block being configured
                  after already started. Caused by SPDIF_TX define check typo
    - RESOLVED:   DFU flag address changed to properly conform to memory address range allocated to 
                  apps by tools
    - RESOLVED:   Build failure when DFU disabled
    - RESOLVED:   Build issue when I2S_CHANS_ADC/DAC set to 0 and CODEC_MASTER enabled
    - RESOLVED:   Typo in MCLK_441 checking for MIN_FREQ define
    - CHANGE:     Mixer and non-mixer channel comms scheme (decouple <-> audio path) now identical
    - CHANGE:     Input stream buffering modified such that during overflow older samples are removed 
                  rather than ignoring most recent samples. Removes any chance of stale input packets
                  being sent to host
    - CHANGE:     module_queue (in sc_usb_audio) now uses lib_xassert rather than module_xassert

6.12.6
------
    - RESOLVED:   Build error when DFU is disabled
    - RESOLVED:   Build error when I2S_CHANS_ADC or I2S_CHANS_DAC set to 0 and CODEC_MASTER enabled

6.12.5
------   
    - RESOLVED:   Stream issue when NUM_USB_CHAN_IN < I2S_CHANS_ADC

6.12.4
------   
    - RESOLVED:   DFU fail when DSD enabled and USB library not running on tile[0]

6.12.3
------
    - RESOLVED:   Method for storing persistent state over a DFU reboot modified to improve resilience 
                  against code-base and tools changes  

6.12.2
------
    - RESOLVED:   Reboot code (used for DFU) failure in tools versions > 14.0.2 (xCORE-200 only)
    - RESOLVED:   Run-time exception in mixer when MAX_MIX_COUNT > 0 (xCORE-200 only)
    - RESOLVED:   MAX_MIX_COUNT checked properly for mix strings in string table
    - CHANGE:     DFU code re-written to use an XC interface. The flash-part may now be connected
                  to a separate tile to the tile running USB code
    - CHANGE:     DFU code can now use quad-SPI flash
    - CHANGE:     Example xmos_dfu application now uses a list of PIDs to allow adding PIDs easier.
                  --listdevices command also added.
    - CHANGE:     I2S_CHANS_PER_FRAME and I2S_WIRES_xxx defines tidied

6.12.1
------
    - RESOLVED:   Fixes to TDM input timing/sample-alignment when BCLK=MCLK
    - RESOLVED:   Various minor fixes to allow ADAT_RX to run on xCORE 200 MC AUDIO hardware
    - CHANGE:     Moved from old SPDIF define to SPDIF_TX

6.12.0
------
    - ADDED:      Checks for XUD_200_SERIES define where required
    - RESOLVED:   Run-time exception due to decouple interrupt not entering correct issue mode
                  (affects XCORE-200 only)
    - CHANGE:     SPDIF Tx Core may now reside on a different tile from I2S
    - CHANGE:     I2C ports now in structure to match new module_i2c_singleport/shared API.

  * Changes to dependencies:

    - sc_util: 1.0.4rc0 -> 1.0.5alpha0

      + xCORE-200 Compatiblity fixes to module_locks

6.11.3
------
    - RESOLVED:  (Major) Streaming issue when mixer not enabled (introduced in 6.11.2)

6.11.2
------
    - RESOLVED:   (Major) Enumeration issue when MAX_MIX_COUNT > 0 only. Introduced in mixer
                  optimisations in 6.11.0. Only affects designs using mixer functionality.
    - RESOLVED:   (Normal) Audio buffering request system modified such that the mixer output is
                  not silent when in underflow case (i.e. host output stream not active) This issue was
                  introduced with the addition of DSD functionality and only affects designs using
                  mixer functionality.
    - RESOLVED:   (Minor) Potential build issue due to duplicate labels in inline asm in
                  set_interrupt_handler macro
    - RESOLVED:   (Minor) BCD_DEVICE define in devicedefines.h now guarded by ifndef (caused issues
                  with DFU test build configs.
    - RESOLVED:   (Minor) String descriptor for Clock Selector unit incorrectly reported
    - RESOLVED:   (Minor) BCD_DEVICE in devicedefines.h now guarded by #ifndef (Caused issues with
                  default DFU test build configs.
    - CHANGE:     HID report descriptor defines added to shared user_hid.h
    - CHANGE:     Now uses module_adat_rx from sc_adat (local module_usb_audio_adat removed)

6.11.1
------
    - ADDED:      ADAT transmit functionality, including SMUX. See ADAT_TX and ADAT_TX_INDEX.
    - RESOLVED:   (Normal) Build issue with CODEC_MASTER (xCore is I2S slave) enabled
    - RESOLVED:   (Minor) Channel ordering issue in when TDM and CODEC_MASTER mode enabled
    - RESOLVED:   (Normal) DFU fails when SPDIF_RX enabled due to clock block being shared between SPDIF
                  core and FlashLib

6.11.0
------
    - ADDED:      Basic TDM I2S functionality added. See I2S_CHANS_PER_FRAME and I2S_MODE_TDM
    - CHANGE:     Various optimisations in 'mixer' core to improve performance for higher
                  channel counts including the use of XC unsafe pointers instead of inline ASM
    - CHANGE:     Mixer mapping disabled when MAX_MIX_COUNT is 0 since this is wasted processing.
    - CHANGE:     Descriptor changes to allow for channel input/output channel count up to 32
                  (previous limit was 18)

6.10.0
------
    - CHANGE:     Endpoint management for iAP EA Native Transport now merged into buffer() core.
                  Previously was separate core (as added in 6.8.0).
    - CHANGE:     Minor optimisation to I2S port code for inputs from ADC

6.9.0
-----
    - ADDED:      ADAT S-MUX II functionality (i.e. 2 channels at 192kHz) - Previously only S-MUX
                  supported (4 channels at 96kHz).
    - ADDED:      Explicit build warnings if sample rate/depth & channel combination exceeds
                  available USB bus bandwidth.
    - RESOLVED:   (Major) Reinstated ADAT input functionality, including descriptors and clock
                  generation/control and stream configuration defines/tables.
    - RESOLVED:   (Major) S/PDIF/ADAT sample transfer code in audio() (from ClockGen()) moved to
                  aid timing.
    - CHANGE:     Modifying mix map now only affects specified mix, previous was applied to all
                  mixes. CS_XU_MIXSEL control selector now takes values 0 to MAX_MIX_COUNT + 1
                  (with 0 affecting all mixes).
    - CHANGE:     Channel c_dig_rx is no longer nullable, assists with timing due to removal of
                  null checks inserted by compiler.
    - CHANGE:     ADAT SMUX selection now based on device sample frequency rather than selected
                  stream format - Endpoint 0 now configures clockgen() on a sample-rate change
                  rather than stream start.

6.8.0
-----
    - ADDED:      Evaluation support for iAP EA Native Transport endpoints
    - RESOLVED:   (Minor) Reverted change in 6.5.1 release where sample rate listing in Audio Class
                  1.0 descriptors was trimmed (previously 4 rates were always reported). This change
                  appears to highlight a Windows (only) enumeration issue with the Input & Output
                  configs
    - RESOLVED:   (Major) Mixer functionality re-instated, including descriptors and various required
                  updates compatibility with 13 tools
    - RESOLVED:   (Major) Endpoint 0 was requesting an out of bounds channel whilst requesting level data
    - RESOLVED:   (Major) Fast mix code not operates correctly in 13 tools, assembler inserting long jmp
                  instructions
    - RESOLVED:   (Minor) LED level meter code now compatible with 13 tools (shared mem access)
    - RESOLVED    (Minor) Ordering of level data from the device now matches channel ordering into
                  mixer (previously the device input data and the stream from host were swapped)
    - CHANGE:     Level meter buffer naming now resemble functionality


Legacy release history
----------------------

Please see changelog in sw_usb_audio for changes prior to 6.8.0 release.
