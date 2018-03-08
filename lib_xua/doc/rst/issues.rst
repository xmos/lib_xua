Known Issues
------------

- Quad-SPI DFU will corrupt the factory image with tools version < 14.0.4 due to an issue with libquadflash 

- (#14762) When in DSD mode with S/PDIF output enabled, DSD samples are transmitted over S/PDIF if the DSD and S/PDIF channels are shared, this may or may not be desired

- (#14173) I2S input is completely disabled when DSD output is active - any input stream to the host will contain 0 samples

- (#14780) Operating the design at a sample rate of less than or equal to the SOF rate (i.e. 8kHz at HS, 1kHz at FS) may expose a corner case relating to 0 length packet handling in both the driver and device and should be considered un-supported at this time.

- (#14883) Before DoP mode is detected a small number of DSD samples will be played out as PCM via I2S

- (#14887) Volume control settings currently affect samples in both DSD and PCM modes. This results in invalid DSD output if volume control not set to 0

-  Windows XP volume control very sensitive.  The Audio 1.0 driver built into Windows XP (usbaudio.sys) does not properly support master volume AND channel volume controls, leading to a very sensitive control.  Descriptors can be easily modified to disable master volume control if required (one byte - bmaControls(0) in Feature Unit descriptors)

-  88.2kHz and 176.4kHz sample frequencies are not exposed in Windows control panels.  These are known OS restrictions.
