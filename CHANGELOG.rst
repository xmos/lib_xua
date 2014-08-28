sc_usb_audio Change Log
=======================

6.8.0
-----
    - ADDED:      Support for iAP EA Native Transport endpoints  
    - RESOLVED:   Mixer functionality re-instated, including descriptors and various required 
                  updates compatibility with 13 tools
    - RESOLVED:   Endpoint 0 was requesting an out of bounds channel whilst requesting level data
    - RESOLVED:   Fast mix code not operates correctly in 13 tools, assembler inserting long jmp
                  instructions
    - RESOLVED:   Level meter buffer naming now resemble functionality

                  

Please see changelog in sw_usb_audio for changes prior to 6.8.0 release.
