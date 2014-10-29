sc_usb_audio Change Log
=======================

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


Please see changelog in sw_usb_audio for changes prior to 6.8.0 release.
