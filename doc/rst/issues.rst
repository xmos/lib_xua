
|newpage|

************
Known Issues
************

- When in DSD mode with S/PDIF output enabled, DSD samples are transmitted over S/PDIF if the DSD and S/PDIF channels are shared, this may or may not be desired (#14762)

- I2S input is completely disabled when DSD output is active - any input stream to the host will contain 0 samples (#14173)

- Operating the design at a sample rate of less than or equal to the SOF rate (i.e. 8kHz at HS, 1kHz at FS) may expose a corner case relating to 0 length packet handling in both the driver and device and should be considered unsupported at this time (#14780)

- Before DoP mode is detected a small number of DSD samples will be played out as PCM via I2S (lib_xua #162)

- Volume control settings currently affect samples in both DSD and PCM modes. This results in invalid DSD output if volume control not set to 0 (#14887)

- 88.2kHz and 176.4kHz sample frequencies are not exposed in Windows control panels.  These are known OS restrictions.

- When DFU flash access fails the device NAKS the host indefinitely (sw_usb_audio #54)

- In synchronous mode there is no nice transition of the reference signal when moving between internal and SOF clocks (lib_xua #275)

- Binary images exceeding FLASH_MAX_UPGRADE_SIZE fail silently on DFU download (lib_xua #165)

- Input does not come out of underflow for USB Audio Class 2 when sample rate is 16kHz and channel count is 2. (lib_xua #434). This will result in silence being streamed to the host. Please use USB Audio Class 1 for low channel count and sample rates.
