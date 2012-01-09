          .file     "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
.extern device_reboot_implementation, "f{0}(chd)"
          .text
          .align    2
.cc_top device_reboot.function,device_reboot
          .align    4
.call device_reboot, device_reboot_implementation
.globl device_reboot, "f{0}(chd)"
.globl device_reboot.nstackwords
.globl device_reboot.maxthreads
.globl device_reboot.maxtimers
.globl device_reboot.maxchanends
.globl device_reboot.maxsync
.type  device_reboot, @function
.linkset device_reboot.locnoside, 1
.linkset device_reboot.locnochandec, 1
.linkset .LLNK1, device_reboot_implementation.nstackwords $M device_reboot_implementation.nstackwords
.linkset .LLNK0, .LLNK1 + 1
.linkset device_reboot.nstackwords, .LLNK0
device_reboot:
          entsp     0x1 
.L1:
.L3:
          bl        device_reboot_implementation 
.L2:
          retsp     0x1 
.size device_reboot, .-device_reboot
.cc_bottom device_reboot.function
.linkset device_reboot.maxchanends, 0#device_reboot_implementation.maxchanends
.linkset device_reboot.maxtimers, device_reboot_implementation.maxtimers
.linkset .LLNK4, device_reboot_implementation.maxthreads - 1
.linkset .LLNK3, 1 + .LLNK4
.linkset .LLNK2, 1 $M .LLNK3
.linkset device_reboot.maxthreads, .LLNK2
# Thread names for recovering thread graph in linker
          .section .xtacalltable,       "", @progbits
.L4:
          .int      .L5-.L4
          .int      0x00000000
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .int      0x00000008
          .long    .L3
.cc_bottom device_reboot.function
.L5:
          .section .xtalabeltable,       "", @progbits
.L6:
          .int      .L7-.L6
          .int      0x00000000
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .int      0x00000009
          .int      0x00000009
# line info for line 9 
          .long    .L2
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .int      0x00000008
          .int      0x00000008
# line info for line 8 
          .long    .L1
.cc_bottom device_reboot.function
.L7:
          .section .dp.data,       "adw", @progbits
.align 4
          .align    4
          .section .dp.bss,        "adw", @nobits
.align 4
          .ident    "XMOS 32-bit XC Compiler 11.11.0beta1 (build 2136)"
          .core     "XS1"
          .corerev  "REVB"
