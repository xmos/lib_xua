#########################################################
# XMOS Compiled Assembly File                           #
#########################################################
#       generated: Mon Jan 09 2012, 16:14               #
#         product: XS1-040                              #
#        compiler: XMOS 32-bit XC Compiler 11.11.0beta1 (build 2136)#
#           input: rebootwrapper.xi                     #
#          output: rebootwrapper.s                      #
#########################################################
          .file     "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .text
          .align    2

.LDBG0:
          .section  .debug_line,   "",    @progbits
.LDBG1:
          .section  .debug_frame, "",     @progbits
          .align    4
.LDBG2:
          .int      .LDBG4-.LDBG3
.LDBG3:
          .int      0xffffffff
          .byte     0x03
          .byte     0x00
          .uleb128  0x2
          .sleb128  0xfffffffc
          .uleb128  0xf
          .byte     0x0c
          .uleb128  0xe
          .uleb128  0x0
          .byte     0x07
          .uleb128  0x0
          .byte     0x07
          .uleb128  0x1
          .byte     0x07
          .uleb128  0x2
          .byte     0x07
          .uleb128  0x3
          .byte     0x08
          .uleb128  0x4
          .byte     0x08
          .uleb128  0x5
          .byte     0x08
          .uleb128  0x6
          .byte     0x08
          .uleb128  0x7
          .byte     0x08
          .uleb128  0x8
          .byte     0x08
          .uleb128  0x9
          .byte     0x08
          .uleb128  0xa
          .byte     0x07
          .uleb128  0xb
          .byte     0x08
          .uleb128  0xc
          .byte     0x08
          .uleb128  0xd
          .byte     0x08
          .uleb128  0xe
          .byte     0x08
          .uleb128  0xf
          .align    4, 0
.LDBG4:
.extern device_reboot_implementation, "f{0}(chd)"
          .text
.cc_top device_reboot.function,device_reboot
          .align    4
.LDBG5:
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
.LDBG9:
.LDBG6:

################
# Entry to function `device_reboot' (Implied thread 0)
################
# STACK
# -----0-| rsvd
# ------------------------
# r0 = [f:spare, t:7]
# r4 = [t:0]
# r5 = [t:1]
# r6 = [t:2]
# r7 = [t:3]
# r8 = [t:4]
# r9 = [t:5]
# r10 = [t:6]
# Code
device_reboot:
          entsp     0x1              # extend sp, save lr (mem[sp]=r15)
.LDBG10:
.LDBG7:
          .file     1 "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .loc      1 7 0

          .loc      1 8 0

.L1_bb_begin:
          .loc      1 8 0

          .loc      1 8 0

.L3_Call:
          bl        device_reboot_implementation 
          .loc      1 9 0

.LDBG11:
.L2_bb_begin:
          retsp     0x1              # return: dealloc and link (pc=r15=mem[sp])
.LDBG8:
.LDBG12:
.LDBG13:
.size device_reboot, .-device_reboot
.cc_bottom device_reboot.function
          .section  .debug_frame, "",     @progbits
.cc_top device_reboot.function,device_reboot
          .align    4
          .int      .LDBG15-.LDBG14
.LDBG14:
          .long     .LDBG2           # offset in .debug_frame
          .int      .LDBG9
          .int      .LDBG13-.LDBG9
          .byte     0x01
          .int      .LDBG10
          .byte     0x0e
          .uleb128  0x4
          .byte     0x14
          .uleb128  0xe
          .uleb128  0x0
          .byte     0x8f
          .uleb128  0x0
          .byte     0x01
          .int      .LDBG11
          .byte     0x0a
          .byte     0x01
          .int      .LDBG12
          .byte     0x0b
          .align    4, 0
.LDBG15:
.cc_bottom device_reboot.function

# PROFILE
.linkset device_reboot.maxchanends, 0#device_reboot_implementation.maxchanends
.linkset device_reboot.maxtimers, device_reboot_implementation.maxtimers
.linkset .LLNK4, device_reboot_implementation.maxthreads - 1
.linkset .LLNK3, 1 + .LLNK4
.linkset .LLNK2, 1 $M .LLNK3
.linkset device_reboot.maxthreads, .LLNK2
###############

          .text
.LDBG16:
# Thread names for recovering thread graph in linker
.LDBG17:
          .section  .debug_info,   "",    @progbits
.LDBG19:
          .int      .LDBG21-.LDBG20
.LDBG20:
          .short    0x0003
          .long     .LDBG18          # offset in .debug_abbrev
          .byte     0x04
          .uleb128  0x1
          .long     .LDBG0           # low address
          .long     .LDBG17          # high address
          .asciiz   "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
          .short    0xc000
          .asciiz   "XMOS Dwarf Symbolic Debug Generator"
          .long     .LDBG1           # offset in .debug_lineprog
.LDBG22:
          .uleb128  0x2
          .asciiz   "long"
          .byte     0x05
          .byte     0x04
.LDBG23:
          .uleb128  0x2
          .asciiz   "unsigned long"
          .byte     0x07
          .byte     0x04
.LDBG24:
          .uleb128  0x2
          .asciiz   "int"
          .byte     0x05
          .byte     0x04
.LDBG25:
          .uleb128  0x2
          .asciiz   "unsigned int"
          .byte     0x07
          .byte     0x04
.LDBG26:
          .uleb128  0x2
          .asciiz   "short"
          .byte     0x05
          .byte     0x02
.LDBG27:
          .uleb128  0x2
          .asciiz   "unsigned short"
          .byte     0x07
          .byte     0x02
.LDBG28:
          .uleb128  0x2
          .asciiz   "char"
          .byte     0x06
          .byte     0x01
.LDBG29:
          .uleb128  0x2
          .asciiz   "unsigned char"
          .byte     0x08
          .byte     0x01
.LDBG30:
          .uleb128  0x2
          .asciiz   "chanend"
          .byte     0x07
          .byte     0x04
.LDBG31:
          .uleb128  0x2
          .asciiz   "timer"
          .byte     0x07
          .byte     0x04
.LDBG32:
          .uleb128  0x2
          .asciiz   "clock"
          .byte     0x07
          .byte     0x04
.LDBG33:
          .uleb128  0x2
          .asciiz   "port"
          .byte     0x07
          .byte     0x04
.LDBG34:
          .uleb128  0x2
          .asciiz   "buffered port:1"
          .byte     0x07
          .byte     0x04
.LDBG35:
          .uleb128  0x2
          .asciiz   "buffered port:4"
          .byte     0x07
          .byte     0x04
.LDBG36:
          .uleb128  0x2
          .asciiz   "buffered port:8"
          .byte     0x07
          .byte     0x04
.LDBG37:
          .uleb128  0x2
          .asciiz   "buffered port:16"
          .byte     0x07
          .byte     0x04
.LDBG38:
          .uleb128  0x2
          .asciiz   "buffered port:32"
          .byte     0x07
          .byte     0x04
.cc_top device_reboot.function,device_reboot
.LDBG39:
          .uleb128  0x3
          .asciiz   "device_reboot"
          .byte     0x01
          .byte     0x07
          .byte     0x01
          .byte     0x01
          .long     .LDBG5           # low address
          .long     .LDBG16          # high address
          .uleb128  0x4
          .asciiz   "spare"
          .byte     0x01
          .short    0x0006
          .int      .LDBG30-.LDBG19
          .int      .LDBG40
          .section  .debug_loc,    "",    @progbits
.cc_top device_reboot.function,device_reboot
.LDBG40:
          .int      .LDBG7-.LDBG0
          .int      .LDBG8-.LDBG0
          .short    .LDBG42-.LDBG41
.LDBG41:
          .byte     0x50
.LDBG42:
          .int      0x00000000
          .int      0x00000000
.cc_bottom device_reboot.function
          .section  .debug_info,   "",    @progbits
          .byte     0x00
.cc_bottom device_reboot.function
          .byte     0x00
.LDBG21:
          .section  .debug_pubnames, "",  @progbits
          .int      .LDBG44-.LDBG43
.LDBG43:
          .short    0x0002
          .long     .LDBG19          # offset in .debug_info
          .int      .LDBG21-.LDBG19
.cc_top device_reboot.function,device_reboot
          .int      .LDBG39-.LDBG19
          .asciiz   "device_reboot"
.cc_bottom device_reboot.function
          .int      0x00000000
.LDBG44:
          .section  .debug_abbrev, "",    @progbits
.LDBG18:
          .uleb128  0x1
          .byte     0x11
          .byte     0x01
          .byte     0x11
          .byte     0x01
          .byte     0x12
          .byte     0x01
          .byte     0x03
          .byte     0x08
          .byte     0x1b
          .byte     0x08
          .byte     0x13
          .byte     0x05
          .byte     0x25
          .byte     0x08
          .byte     0x10
          .byte     0x06
          .byte     0x00
          .byte     0x00
          .uleb128  0x2
          .byte     0x24
          .byte     0x00
          .byte     0x03
          .byte     0x08
          .byte     0x3e
          .byte     0x0b
          .byte     0x0b
          .byte     0x0b
          .byte     0x00
          .byte     0x00
          .uleb128  0x3
          .byte     0x2e
          .byte     0x01
          .byte     0x03
          .byte     0x08
          .byte     0x3a
          .byte     0x0b
          .byte     0x3b
          .byte     0x0b
          .byte     0x3f
          .byte     0x0c
          .byte     0x27
          .byte     0x0c
          .byte     0x11
          .byte     0x01
          .byte     0x12
          .byte     0x01
          .byte     0x00
          .byte     0x00
          .uleb128  0x4
          .byte     0x05
          .byte     0x00
          .byte     0x03
          .byte     0x08
          .byte     0x3a
          .byte     0x0b
          .byte     0x3b
          .byte     0x05
          .byte     0x49
          .byte     0x13
          .byte     0x02
          .byte     0x06
          .byte     0x00
          .byte     0x00

          .byte     0x00
          .section .xtacalltable,       "", @progbits
.L4_xta_begin:
          .int      .L5_xta_end-.L4_xta_begin
          .int      0x00000000
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .int      0x00000008
          .long    .L3_Call
.cc_bottom device_reboot.function
.L5_xta_end:
          .section .xtalabeltable,       "", @progbits
.L6_xta_begin:
          .int      .L7_xta_end-.L6_xta_begin
          .int      0x00000000
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .int      0x00000009
          .int      0x00000009
# line info for line 9 
          .long    .L2_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/rebootwrapper.xc"
          .int      0x00000008
          .int      0x00000008
# line info for line 8 
          .long    .L1_bb_begin
.cc_bottom device_reboot.function
.L7_xta_end:
          .section .dp.data,       "adw", @progbits
.align 4
          .align    4
          .section .dp.bss,        "adw", @nobits
.align 4
          .ident    "XMOS 32-bit XC Compiler 11.11.0beta1 (build 2136)"
          .core     "XS1"
          .corerev  "REVB"

# memory access instructions: 2
# total instructions: 3
########################################
