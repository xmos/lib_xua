lib_xua Change Log
==================

1.0.0
-----
    - ADDED:      FB_USE_REF_CLOCK to allow feedback generation from xCORE internal reference
    - ADDED:      Linux Makefile for xmosdfu host application
    - ADDED:      Raspberry Pi Makefile for xmosdfu host application
    - ADDED:      Documentation of PID argument to xmosdfu
    - ADDED:      Optional build time microphone delay line (MIC_BUFFER_DEPTH) 
    - CHANGE:     Removal of audManage_if, users should define their own interfaces as required
    - CHANGE:     Vendor specific control interface in UAC1 descriptor now has a string descriptor
                  so it shows up with a descriptive name in Windows Device Manager
    - CHANGE:     DFU_BCD_DEVICE removed (now uses BCD_DEVICE)
    - CHANGE:     Renaming in descriptors.h to avoid clashes with application
    - RESOLVED:   FIR gain compensation for PDM mics set incorrectly for divide of 8
    - RESOLVED:   Incorrect xmosdfu DYLD path in test script code
    - RESOLVED:   xmosdfu cannot find XMOS device on modern MacBook Pro (#17897)
    - RESOLVED:   Issue when feedback is initially incorrect when two SOF's are not yet received 
    - RESOLVED:   AUDIO_TILE and PDM_TILE may now share the same value/tile      
    - RESOLVED:   Cope with out of order interface numbers in xmosdfu
    
Legacy release history
----------------------

(Note: Forked from sc_usb_audio at this point)

7.4.1
-----
    - RESOLVED:   Exception due to null chanend when using NO_USB

7.4.0
-----
    - RESOLVED:   PID_DFU now based on AUDIO_CLASS. This potentially caused issues
      with UAC1 DFU


7.3.0
-----
    - CHANGE:     Example OSX DFU host app updated to now take PID as runtime 
      argument. This enabled multiple XMOS devices to be attached to the host
      during DFU process

7.2.0
-----
    - ADDED:      DFU to UAC1 descriptors (guarded by DFU and FORCE_UAC1_DFU)
    - RESOLVED:   Removed 'reinterpretation to type of larger alignment' warnings
    - RESOLVED:   DFU flash code run on tile[0] even if XUD_TILE and AUDIO_IO_TILE are not 0

7.1.0
-----
    - ADDED:      UserBufferManagementInit() to reset any state required in UserBufferManagement()
    - ADDED:      I2S output up-sampling (enabled when AUD_TO_USB_RATIO is > 1)
    - ADDED:      PDM Mic decimator output rate can now be controlled independently (via AUD_TO_MICS_RATIO)
    - CHANGE:     Rename I2S input down-sampling (enabled when AUD_TO_USB_RATIO is > 1, rather than via I2S_DOWNSAMPLE_FACTOR)
    - RESOLVED:   Crosstalk between input channels when I2S input down-sampling is enabled
    - RESOLVED:   Mic decimation data tables properly sized when mic sample-rate < USB audio sample-rate

7.0.1
-----
    - RESOLVED:   PDM microphone decimation issue at some sample rates caused by integration

7.0.0
------
    - ADDED:      I2S down-sampling (I2S_DOWNSAMPLE_FACTOR)
    - ADDED:      I2S resynchronisation when in slave mode (CODEC_MASTER=1)
    - CHANGE:     Various memory optimisations when MAX_FREQ = MIN_FREQ
    - CHANGE:     Memory optimisations in audio buffering
    - CHANGE:     Various memory optimisations in UAC1 mode
    - CHANGE:     user_pdm_process() API change
    - CHANGE:     PDM Mic decimator table now related to MIN_FREQ (memory optimisation)
    - RESOLVED:   Audio request interrupt handler properly eliminated

6.30.0
------
    - RESOLVED:   Number of PDM microphone channels configured now based on NUM_PDM_MICS define
                  (previously hard-coded)
    - RESOLVED:   PDM microphone clock divide now based MCLK defines (previously hard-coded)
    - CHANGE:     Second microphone decimation core only run if NUM_PDM_MICS > 4

6.20.0
------
    - RESOLVED:   Intra-frame sample delays of 1/2 samples on input streaming in TDM mode
    - RESOLVED:   Build issue with NUM_USB_CHAN_OUT set to 0 and MIXER enabled
    - RESOLVED:   SPDIF_TX_INDEX not defined build warning only emitted when SPDIF_TX defined
    - RESOLVED:   Failure to enter DFU mode when configured without input volume control

6.19.0
------
    - RESOLVED:   SPDIF_TX_INDEX not defined build warning only emitted when SPDIF_TX defined
    - RESOLVED:   Failure to enter DFU mode when configured without input volume control

6.18.1
------
    - ADDED:      Vendor Specific control interface added to UAC1 descriptors to allow control of
                  XVSM params from Windows (via lib_usb)

6.18.0
------
    - ADDED:      Call to VendorRequests() and VendorRequests_Init() to Endpoint 0
    - ADDED:      VENDOR_REQUESTS_PARAMS define to allow for custom parameters to VendorRequest calls
    - RESOLVED:   FIR gain compensation set appropriately in lib_mic_array usage
    - CHANGE:     i_dsp interface renamed i_audManage

6.16.0
------
    - ADDED:      Call to UserBufferManagement()
    - ADDED:      PDM_MIC_INDEX in devicedefines.h and usage
    - CHANGE:     pdm_buffer() task now combinable
    - CHANGE:     Audio I/O task now takes i_dsp interface as a parameter
    - CHANGE:     Removed built-in support for A/U series internal ADC
    - CHANGE:     User PDM Microphone processing now uses an interface (previously function call)

6.15.2
------
    - RESOLVED:   interrupt.h (used in audio buffering) now compatible with xCORE-200 ABI

6.15.1
------
    - RESOLVED:   DAC data mis-alignment issue in TDM/I2S slave mode
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
