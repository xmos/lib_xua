sc_usb_audio Change Log
=======================

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
    - CHANGE:     Level meter buffer naming now resemble functionality

                  

Please see changelog in sw_usb_audio for changes prior to 6.8.0 release.
