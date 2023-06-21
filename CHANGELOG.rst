lib_xua Change Log
==================

3.5.1
-----

  * FIXED:     Respect I2S_CHANS_PER_FRAME when calculating bit-clock rates

  * Changes to dependencies:

    - lib_spdif: 5.0.0 -> 5.0.1

3.5.0
-----

  * ADDED:     Configurable word-length for I2S/TDM via XUA_I2S_N_BITS
  * ADDED:     Support for statically defined custom HID descriptor
  * CHANGED:   Rearranged main() such that adding custom code that uses lib_xud
    is possible
  * CHANGED:   bNumConfigurations changed from 2 to 1, removing a work-around to
    stop old Windows versions loading the composite driver
  * FIXED:     Memory corruption due to erroneous initialisation of mixer
    weights when not in use (#152)
  * FIXED:     UserHostActive() not being called as expected (#326)
  * FIXED:     Exception when entering DSD mode (#327)

  * Changes to dependencies:

    - lib_spdif: 4.2.1 -> 5.0.0

    - lib_xud: 2.2.2 -> 2.2.3

3.4.0
-----

  * ADDED:     Unit tests for mixer functionality
  * ADDED:     Host mixer control applications (for Win/macOS)
  * CHANGED:   Small tidies to mixer implementation
  * CHANGED:   Improved mixer control channel communication protocol to avoid
    deadlock situations
  * CHANGED:   By default, output volume processing occurs in mixer task, if
    present. Previously occurred in decouple task
  * CHANGED:   Some optimisations in sample transfer from decouple task
  * FIXED:     Exception on startup when USB input disabled
  * FIXED:     Full 32bit volume processing only applied when required
  * FIXED:     Setting OUT_VOLUME_AFTER_MIX to zero now has the expected effect

  * Changes to dependencies:

    - lib_xud: 2.2.1 -> 2.2.2

3.3.1
-----

  * CHANGED:  Documentation updates

  * Changes to dependencies:

    - lib_spdif: 4.1.0 -> 4.2.1

3.3.0
-----

  * CHANGED:   Define ADAT_RX renamed to XUA_ADAT_RX_EN
  * CHANGED:   Define ADAT_TX renamed to XUA_ADAT_TX_EN
  * CHANGED:   Define SPDIF_RX renamed to XUA_SPDIF_RX_EN
  * CHANGED:   Define SELF_POWERED changed to XUA_POWERMODE and associated
    defines
  * CHANGED:   Drive strength of I2S clock lines upped to 8mA on xCORE.ai
  * CHANGED:   ADC datalines sampled on falling edge of clock in TDM mode
  * CHANGED:   Improved startup behaviour of TDM clocks
  * FIXED:     Intermittent underflow at MAX_FREQ on input stream start due to
    insufficient packet buffering
  * FIXED:     Decouple buffer accounting to avoid corruption of samples

  * Changes to dependencies:

    - lib_adat: Added dependency 1.0.1

    - lib_xud: 2.1.0 -> 2.2.1

3.2.0
-----

  * CHANGED:   Updated tests to use lib_locks (was legacy module_locks)
  * CHANGED:   Exclude HID Report functions unless the HID feature is enabled
  * CHANGED:   Explicit feedback EP enabled by default (see
    UAC_FORCE_FEEDBACK_EP)
  * FIXED:     Incorrect conditional compilation of HID report code
  * FIXED:     Input/output descriptors written when input/output not enabled.
    (Audio class 1.0 mode using XUA_USB_DESCRIPTOR_OVERWRITE_RATE_RES)

  * Changes to dependencies:

    - lib_dsp: 5.0.0 -> 6.2.1

    - lib_locks: Added dependency 2.1.0

    - lib_logging: 3.0.0 -> 3.1.1

    - lib_mic_array: 4.0.0 -> 4.5.0

    - lib_spdif: 4.0.0 -> 4.1.0

    - lib_xassert: 4.0.0 -> 4.1.0

    - lib_xud: 2.0.0 -> 2.1.0

3.1.0
-----

  * CHANGED:   Removed logic from HID API functions allowing a Report ID of 0 to
    be used as "all/any" Report

3.0.0
-----

  * ADDED:     Support for HID Report IDs
  * CHANGED:   Renamed the HID API file xua_hid_report_descriptor.h to
    xua_hid_report.h
  * REMOVED:   Support for HID Reports containing controls from mixed usage
    pages

2.1.1
-----

  * CHANGED:   Setting of HID report items

2.1.0
-----

  * CHANGED:   Updated clock blocks to support lib_xud v2.0.0

  * Changes to dependencies:

    - lib_xud: 1.2.0 -> 2.0.0

2.0.0
-----

  * ADDED:     Function to get a Report item description
  * ADDED:     Support for multiple flash specs defined by DFU_FLASH_DEVICE
  * ADDED:     Nullable c_aud_ctl chan-end optimisation for fixed rate devices
  * CHANGED:   Check HID Usage Page when changing a Report item description
  * CHANGED:   HID event ID from list to bit and byte location in HID Report
  * CHANGED:   Interface to UserHIDRecordEvent()

1.3.0
-----

  * ADDED:     Build default HID Report descriptor at boot-time
  * ADDED:     Function to return length of HID Report
  * CHANGED:   Move HID descriptors to ease maintenance
  * CHANGED:   Move legacy tests to separate directory
  * CHANGED:   Replace unused GPI-specific HID event names with generic ones
  * CHANGED:   HID Report to return multiple bytes
  * CHANGED:   NO_USB conditional compilation switch with XUA_USB_EN
  * CHANGED:   Clock blocks used for BCLK and MCLK
  * CHANGED:   Arguments no longer supported by XUD_Main
  * REMOVED:   Support for XS1 based devices

1.2.0
-----

  * ADDED:     Updates for xcore.ai/XS3 compatibility
  * ADDED:     Makefile.Win32 for xmosdfu on Windows
  * CHANGED:   Use XMOS Public Licence Version 1
  * FIXED:     Bump default BCD device number to v1.2.0
  * FIXED:     xmosdfu now fails with an error when given a directory (#119)
  * FIXED:     Compilation errors related to HID code
  * FIXED:     Runtime error when using mic array interface
  * FIXED:     Automate HID Report Descriptor length in AC1 HID Descriptor

1.1.1
-----

  * CHANGED:   Pin Python package versions
  * FIXED:     Zero length input packets generated before enumeration causing
    I2S timing pushout at startup

1.1.0
-----

  * ADDED:     Ability to read or modify serial number string

1.0.1
-----

  * FIXED:     Wrong size of vendor and product strings

1.0.0
-----

  * ADDED:     UAC1 HID support with simulated Voice Command detection reported
    every 10 seconds
  * ADDED:     Support for USB HID Set Idle request
  * ADDED:     Pre-processor symbols to enable single-threaded, dual-PDM
    microphone operation
  * ADDED:     Guards to user_hid.h and xua_hid.h
  * ADDED:     UAC1 HID support for AC Stop (End Call), Volume Increment and
    Volume Decrement
  * CHANGED:   UAC1 HID to report function keys f21 through f24 as specified by
    customer
  * CHANGED:   HID interface for user to set and clear events from global
    variable to function
  * CHANGE     HID report descriptor to use generic events instead of GPI
    events, to report Key-phrase detection as AC Search, and to report end-call
    detection as AC Stop
  * ADDED:     Ability to read or modify vendor and product IDs and strings
  * ADDED:     Ability to read or modify bcdDevice
  * ADDED:     Override USB descriptor with sampling frequency and
    bit-resolution set at boot time.
  * ADDED:     Global pointer to allow external access to masterClockFreq
  * FIXED:     Descriptors for XUA_ADAPTIVE incorrectly defined for IN endpoint

  * Changes to dependencies:

    - lib_spdif: 3.1.0 -> 4.0.0

    - lib_xassert: 3.0.1 -> 4.0.0

0.2.1
-----

  * FIXED:     Fix descriptors for XUA_ADAPTIVE

  * Changes to dependencies:

    - lib_logging: 2.1.1 -> 3.0.0

    - lib_xud: 0.1.0 -> 0.2.0

0.2.0
-----

  * ADDED:     Initial library documentation
  * ADDED:     Application note AN00247: Using lib_xua with lib_spdif (transmit)
  * ADDED:     Separate callbacks for input/output audio stream start/stop
  * CHANGED:   I2S hardware resources no longer used globally and must be passed
    to XUA_AudioHub()
  * CHANGED:   XUA_AudioHub() no longer pars S/PDIF transmitter task
  * CHANGED:   Moved to lib_spdif (from module_spdif_tx & module_spdif_rx)
  * CHANGED:   Define NUM_PDM_MICS renamed to XUA_NUM_PDM_MICS
  * CHANGED:   Define NO_USB renamed to XUA_USB_EN
  * CHANGED:   Build files updated to support new "xcommon" behaviour in xwaf.
  * FIXED:     wChannelConfig in UAC1 descriptor set according to output channel
    count
  * FIXED:     Indexing of ADAT channel strings (#18059)
  * FIXED:     Rebooting device fails when PLL config "not reset" bit is set

  * Changes to dependencies:

    - lib_dsp: Added dependency 5.0.0

    - lib_mic_array: Added dependency 4.0.0

    - lib_spdif: Added dependency 3.1.0

    - lib_xassert: Added dependency 3.0.1

0.1.2
-----

  * ADDED:     Application note AN00246: Simple USB Audio Device using lib_xua
  * CHANGED:   xmosdfu emits warning if empty image read via upload
  * CHANGED:   Simplified mclk port sharing - no longer uses unsafe pointer
  * FIXED:     Runtime exception issues when incorrect feedback calculated
    (introduced in sc_usb_audio 6.13)
  * FIXED:     Output sample counter reset on stream start. Caused playback
    issues on some Linux based hosts

0.1.1
-----

  * FIXED:   Configurations where I2S_CHANS_DAC and I2S_CHANS_ADC are both 0 now
    build
  * FIXED:   Deadlock in mixer when MAX_MIX_COUNT > 0 for larger channel counts

  * Changes to dependencies:

    - lib_logging: Added dependency 2.1.1

    - lib_xud: Added dependency 0.1.0

0.1.0
-----

  * ADDED:     FB_USE_REF_CLOCK to allow feedback generation from xCORE internal
    reference
  * ADDED:     Linux Makefile for xmosdfu host application
  * ADDED:     Raspberry Pi Makefile for xmosdfu host application
  * ADDED:     Documentation of PID argument to xmosdfu
  * ADDED:     Optional build time microphone delay line (MIC_BUFFER_DEPTH)
  * CHANGED:   Removal of audManage_if, users should define their own interfaces
    as required
  * CHANGED:   Vendor specific control interface in UAC1 descriptor now has a
    string descriptor so it shows up with a descriptive name in Windows Device
    Manager
  * CHANGED:   DFU_BCD_DEVICE removed (now uses BCD_DEVICE)
  * CHANGED:   Renaming in descriptors.h to avoid clashes with application
  * CHANGED:   Make device reboot function no-argument (was one channel end)
  * FIXED:     FIR gain compensation for PDM mics set incorrectly for divide of
    8
  * FIXED:     Incorrect xmosdfu DYLD path in test script code
  * FIXED:   xmosdfu cannot find XMOS device on modern MacBook Pro (#17897)
  * FIXED:   Issue when feedback is initially incorrect when two SOF's are not
    yet received
  * FIXED:   AUDIO_TILE and PDM_TILE may now share the same value/tile
  * FIXED:   Cope with out of order interface numbers in xmosdfu
  * FIXED:   DSD playback not functional on xCORE-200 (introduced in
    sc_usb_audio 6.14)
  * FIXED:   Improvements made to clock sync code in TDM slave mode


Legacy release history
----------------------

(Note: Forked from sc_usb_audio at this point)

7.4.1
-----
    - FIXED:   Exception due to null chanend when using NO_USB

7.4.0
-----
    - FIXED:   PID_DFU now based on AUDIO_CLASS. This potentially caused issues
      with UAC1 DFU


7.3.0
-----
    - CHANGED:    Example OSX DFU host app updated to now take PID as runtime
      argument. This enabled multiple XMOS devices to be attached to the host
      during DFU process

7.2.0
-----
    - ADDED:      DFU to UAC1 descriptors (guarded by DFU and FORCE_UAC1_DFU)
    - FIXED:      Removed 'reinterpretation to type of larger alignment' warnings
    - FIXED:      DFU flash code run on tile[0] even if XUD_TILE and AUDIO_IO_TILE are not 0

7.1.0
-----
    - ADDED:      UserBufferManagementInit() to reset any state required in UserBufferManagement()
    - ADDED:      I2S output up-sampling (enabled when AUD_TO_USB_RATIO is > 1)
    - ADDED:      PDM Mic decimator output rate can now be controlled independently (via AUD_TO_MICS_RATIO)
    - CHANGED:    Rename I2S input down-sampling (enabled when AUD_TO_USB_RATIO is > 1, rather than via I2S_DOWNSAMPLE_FACTOR)
    - FIXED:      Crosstalk between input channels when I2S input down-sampling is enabled
    - FIXED:      Mic decimation data tables properly sized when mic sample-rate < USB audio sample-rate

7.0.1
-----
    - FIXED:      PDM microphone decimation issue at some sample rates caused by integration

7.0.0
------
    - ADDED:      I2S down-sampling (I2S_DOWNSAMPLE_FACTOR)
    - ADDED:      I2S resynchronisation when in slave mode (CODEC_MASTER=1)
    - CHANGED:    Various memory optimisations when MAX_FREQ = MIN_FREQ
    - CHANGED:    Memory optimisations in audio buffering
    - CHANGED:    Various memory optimisations in UAC1 mode
    - CHANGED:    user_pdm_process() API change
    - CHANGED:    PDM Mic decimator table now related to MIN_FREQ (memory optimisation)
    - FIXED:      Audio request interrupt handler properly eliminated

6.30.0
------
    - FIXED:   Number of PDM microphone channels configured now based on NUM_PDM_MICS define
    (previously hard-coded)
    - FIXED:   PDM microphone clock divide now based MCLK defines (previously hard-coded)
    - CHANGED: Second microphone decimation core only run if NUM_PDM_MICS > 4

6.20.0
------
    - FIXED:   Intra-frame sample delays of 1/2 samples on input streaming in TDM mode
    - FIXED:   Build issue with NUM_USB_CHAN_OUT set to 0 and MIXER enabled
    - FIXED:   SPDIF_TX_INDEX not defined build warning only emitted when SPDIF_TX defined
    - FIXED:   Failure to enter DFU mode when configured without input volume control

6.19.0
------
    - FIXED:   SPDIF_TX_INDEX not defined build warning only emitted when SPDIF_TX defined
    - FIXED:   Failure to enter DFU mode when configured without input volume control

6.18.1
------
    - ADDED:   Vendor Specific control interface added to UAC1 descriptors to allow control of
                XVSM params from Windows (via lib_usb)

6.18.0
------
    - ADDED:   Call to VendorRequests() and VendorRequests_Init() to Endpoint 0
    - ADDED:   VENDOR_REQUESTS_PARAMS define to allow for custom parameters to VendorRequest calls
    - FIXED:   FIR gain compensation set appropriately in lib_mic_array usage
    - CHANGED: i_dsp interface renamed i_audManage

6.16.0
------
    - ADDED:      Call to UserBufferManagement()
    - ADDED:      PDM_MIC_INDEX in devicedefines.h and usage
    - CHANGED:    pdm_buffer() task now combinable
    - CHANGED:    Audio I/O task now takes i_dsp interface as a parameter
    - CHANGED:    Removed built-in support for A/U series internal ADC
    - CHANGED:    User PDM Microphone processing now uses an interface (previously function call)

6.15.2
------
    - FIXED:   interrupt.h (used in audio buffering) now compatible with xCORE-200 ABI

6.15.1
------
    - FIXED:   DAC data mis-alignment issue in TDM/I2S slave mode
    - CHANGED:    Updates to support API changes in lib_mic_array version 2.0

6.15.0
------

    - FIXED:   UAC 1.0 descriptors now support multi-channel volume control (previously were
                  hard-coded as stereo)
    - CHANGED:    Removed 32kHz sample-rate support when PDM microphones enabled (lib_mic_array
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
    - FIXED:   MIDI data not accepted after "sleep" in OSX 10.11 (El Capitan) - related to sc_xud
                  issue #17092
    - CHANGED:    Asynchronous feedback system re-implemented to allow for the first two ADDED
                  changelog items
    - CHANGED:    Hardware divider used to generate bit-clock from master clock (xCORE-200 only).
                  Allows easy support for greater number of master-clock to sample-rate ratios.
    - CHANGED:    module_queue no longer uses any assert module/lib

6.13.0
------
    - ADDED:      Device now uses implicit feedback when input stream is available (previously explicit
                  feedback pipe always used). This saves chanend/EP resources and means less processing
                  burden for the host. Previous behaviour available by enabling UAC_FORCE_FEEDBACK_EP
    - FIXED:   Exception when SPDIF_TX and ADAT_TX both enabled due to clock-block being configured
                  after already started. Caused by SPDIF_TX define check typo
    - FIXED:   DFU flag address changed to properly conform to memory address range allocated to
                  apps by tools
    - FIXED:   Build failure when DFU disabled
    - FIXED:   Build issue when I2S_CHANS_ADC/DAC set to 0 and CODEC_MASTER enabled
    - FIXED:   Typo in MCLK_441 checking for MIN_FREQ define
    - CHANGED:    Mixer and non-mixer channel comms scheme (decouple <-> audio path) now identical
    - CHANGED:    Input stream buffering modified such that during overflow older samples are removed
                  rather than ignoring most recent samples. Removes any chance of stale input packets
                  being sent to host
    - CHANGED:    module_queue (in sc_usb_audio) now uses lib_xassert rather than module_xassert

6.12.6
------
    - FIXED:   Build error when DFU is disabled
    - FIXED:   Build error when I2S_CHANS_ADC or I2S_CHANS_DAC set to 0 and CODEC_MASTER enabled

6.12.5
------
    - FIXED:   Stream issue when NUM_USB_CHAN_IN < I2S_CHANS_ADC

6.12.4
------
    - FIXED:   DFU fail when DSD enabled and USB library not running on tile[0]

6.12.3
------
    - FIXED:   Method for storing persistent state over a DFU reboot modified to improve resilience
                  against code-base and tools changes

6.12.2
------
    - FIXED:   Reboot code (used for DFU) failure in tools versions > 14.0.2 (xCORE-200 only)
    - FIXED:   Run-time exception in mixer when MAX_MIX_COUNT > 0 (xCORE-200 only)
    - FIXED:   MAX_MIX_COUNT checked properly for mix strings in string table
    - CHANGED:    DFU code re-written to use an XC interface. The flash-part may now be connected
                  to a separate tile to the tile running USB code
    - CHANGED:    DFU code can now use quad-SPI flash
    - CHANGED:    Example xmos_dfu application now uses a list of PIDs to allow adding PIDs easier.
                  --listdevices command also added.
    - CHANGED:    I2S_CHANS_PER_FRAME and I2S_WIRES_xxx defines tidied

6.12.1
------
    - FIXED:   Fixes to TDM input timing/sample-alignment when BCLK=MCLK
    - FIXED:   Various minor fixes to allow ADAT_RX to run on xCORE 200 MC AUDIO hardware
    - CHANGED:    Moved from old SPDIF define to SPDIF_TX

6.12.0
------
    - ADDED:      Checks for XUD_200_SERIES define where required
    - FIXED:   Run-time exception due to decouple interrupt not entering correct issue mode
                  (affects XCORE-200 only)
    - CHANGED:    SPDIF Tx Core may now reside on a different tile from I2S
    - CHANGED:    I2C ports now in structure to match new module_i2c_singleport/shared API.

  * Changes to dependencies:

    - sc_util: 1.0.4rc0 -> 1.0.5alpha0

      + xCORE-200 Compatiblity fixes to module_locks

6.11.3
------
    - FIXED:  (Major) Streaming issue when mixer not enabled (introduced in 6.11.2)

6.11.2
------
    - FIXED:   (Major) Enumeration issue when MAX_MIX_COUNT > 0 only. Introduced in mixer
                  optimisations in 6.11.0. Only affects designs using mixer functionality.
    - FIXED:   (Normal) Audio buffering request system modified such that the mixer output is
                  not silent when in underflow case (i.e. host output stream not active) This issue was
                  introduced with the addition of DSD functionality and only affects designs using
                  mixer functionality.
    - FIXED:   (Minor) Potential build issue due to duplicate labels in inline asm in
                  set_interrupt_handler macro
    - FIXED:   (Minor) BCD_DEVICE define in devicedefines.h now guarded by ifndef (caused issues
                  with DFU test build configs.
    - FIXED:   (Minor) String descriptor for Clock Selector unit incorrectly reported
    - FIXED:   (Minor) BCD_DEVICE in devicedefines.h now guarded by #ifndef (Caused issues with
                  default DFU test build configs.
    - CHANGED:    HID report descriptor defines added to shared user_hid.h
    - CHANGED:    Now uses module_adat_rx from sc_adat (local module_usb_audio_adat removed)

6.11.1
------
    - ADDED:      ADAT transmit functionality, including SMUX. See ADAT_TX and ADAT_TX_INDEX.
    - FIXED:   (Normal) Build issue with CODEC_MASTER (xCore is I2S slave) enabled
    - FIXED:   (Minor) Channel ordering issue in when TDM and CODEC_MASTER mode enabled
    - FIXED:   (Normal) DFU fails when SPDIF_RX enabled due to clock block being shared between SPDIF
                  core and FlashLib

6.11.0
------
    - ADDED:      Basic TDM I2S functionality added. See I2S_CHANS_PER_FRAME and I2S_MODE_TDM
    - CHANGED:    Various optimisations in 'mixer' core to improve performance for higher
                  channel counts including the use of XC unsafe pointers instead of inline ASM
    - CHANGED:    Mixer mapping disabled when MAX_MIX_COUNT is 0 since this is wasted processing.
    - CHANGED:    Descriptor changes to allow for channel input/output channel count up to 32
                  (previous limit was 18)

6.10.0
------
    - CHANGED:    Endpoint management for iAP EA Native Transport now merged into buffer() core.
                  Previously was separate core (as added in 6.8.0).
    - CHANGED:    Minor optimisation to I2S port code for inputs from ADC

6.9.0
-----
    - ADDED:      ADAT S-MUX II functionality (i.e. 2 channels at 192kHz) - Previously only S-MUX
                  supported (4 channels at 96kHz).
    - ADDED:      Explicit build warnings if sample rate/depth & channel combination exceeds
                  available USB bus bandwidth.
    - FIXED:   (Major) Reinstated ADAT input functionality, including descriptors and clock
                  generation/control and stream configuration defines/tables.
    - FIXED:   (Major) S/PDIF/ADAT sample transfer code in audio() (from ClockGen()) moved to
                  aid timing.
    - CHANGED:    Modifying mix map now only affects specified mix, previous was applied to all
                  mixes. CS_XU_MIXSEL control selector now takes values 0 to MAX_MIX_COUNT + 1
                  (with 0 affecting all mixes).
    - CHANGED:    Channel c_dig_rx is no longer nullable, assists with timing due to removal of
                  null checks inserted by compiler.
    - CHANGED:    ADAT SMUX selection now based on device sample frequency rather than selected
                  stream format - Endpoint 0 now configures clockgen() on a sample-rate change
                  rather than stream start.

6.8.0
-----
    - ADDED:      Evaluation support for iAP EA Native Transport endpoints
    - FIXED:   (Minor) Reverted change in 6.5.1 release where sample rate listing in Audio Class
                  1.0 descriptors was trimmed (previously 4 rates were always reported). This change
                  appears to highlight a Windows (only) enumeration issue with the Input & Output
                  configs
    - FIXED:   (Major) Mixer functionality re-instated, including descriptors and various required
                  updates compatibility with 13 tools
    - FIXED:   (Major) Endpoint 0 was requesting an out of bounds channel whilst requesting level data
    - FIXED:   (Major) Fast mix code not operates correctly in 13 tools, assembler inserting long jmp
                  instructions
    - FIXED:   (Minor) LED level meter code now compatible with 13 tools (shared mem access)
    - FIXED:    (Minor) Ordering of level data from the device now matches channel ordering into
                  mixer (previously the device input data and the stream from host were swapped)
    - CHANGED:    Level meter buffer naming now resemble functionality


Legacy release history
----------------------

Please see changelog in sw_usb_audio for changes prior to 6.8.0 release.

