#########################################################
# XMOS Compiled Assembly File                           #
#########################################################
#       generated: Mon Jan 09 2012, 15:48               #
#         product: XS1-040                              #
#        compiler: XMOS 32-bit XC Compiler 11.11.0beta1 (build 2136)#
#           input: reboot.xi                            #
#          output: reboot.s                             #
#########################################################
          .file     "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
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
.extern configure_in_port_handshake, "f{0}(w:p,i:p,o:p,ck)"
.extern configure_out_port_handshake, "f{0}(w:p,i:p,o:p,ck,ui)"
.extern configure_in_port_strobed_master, "f{0}(w:p,o:p,:ck)"
.extern configure_out_port_strobed_master, "f{0}(w:p,o:p,:ck,ui)"
.extern configure_in_port_strobed_slave, "f{0}(w:p,i:p,ck)"
.extern configure_out_port_strobed_slave, "f{0}(w:p,i:p,ck,ui)"
.extern configure_in_port, "f{0}(w:p,:ck)"
.extern configure_out_port, "f{0}(w:p,:ck,ui)"
.extern configure_port_clock_output, "f{0}(w:p,:ck)"
.extern start_port, "f{0}(w:p)"
.extern stop_port, "f{0}(w:p)"
.extern configure_clock_src, "f{0}(ck,w:p)"
.extern configure_clock_ref, "f{0}(ck,uc)"
.extern configure_clock_rate, "f{0}(ck,ui,ui)"
.extern configure_clock_rate_at_least, "f{0}(ck,ui,ui)"
.extern configure_clock_rate_at_most, "f{0}(ck,ui,ui)"
.extern set_clock_src, "f{0}(ck,w:p)"
.extern set_clock_ref, "f{0}(ck)"
.extern set_clock_div, "f{0}(ck,uc)"
.extern set_clock_rise_delay, "f{0}(ck,ui)"
.extern set_clock_fall_delay, "f{0}(ck,ui)"
.extern set_port_clock, "f{0}(w:p,:ck)"
.extern set_port_ready_src, "f{0}(w:p,w:p)"
.extern set_clock_ready_src, "f{0}(ck,w:p)"
.extern set_clock_on, "f{0}(ck)"
.extern set_clock_off, "f{0}(ck)"
.extern start_clock, "f{0}(ck)"
.extern stop_clock, "f{0}(ck)"
.extern set_port_use_on, "f{0}(w:p)"
.extern set_port_use_off, "f{0}(w:p)"
.extern set_port_mode_data, "f{0}(w:p)"
.extern set_port_mode_clock, "f{0}(w:p)"
.extern set_port_mode_ready, "f{0}(w:p)"
.extern set_port_drive, "f{0}(w:p)"
.extern set_port_drive_low, "f{0}(w:p)"
.extern set_port_pull_up, "f{0}(w:p)"
.extern set_port_pull_down, "f{0}(w:p)"
.extern set_port_pull_none, "f{0}(w:p)"
.extern set_port_master, "f{0}(w:p)"
.extern set_port_slave, "f{0}(w:p)"
.extern set_port_no_ready, "f{0}(w:p)"
.extern set_port_strobed, "f{0}(w:p)"
.extern set_port_handshake, "f{0}(w:p)"
.extern set_port_no_sample_delay, "f{0}(w:p)"
.extern set_port_sample_delay, "f{0}(w:p)"
.extern set_port_no_inv, "f{0}(w:p)"
.extern set_port_inv, "f{0}(w:p)"
.extern set_port_shift_count, "f{0}(w:p,ui)"
.extern set_pad_delay, "f{0}(w:p,ui)"
.extern set_thread_fast_mode_on, "f{0}(0)"
.extern set_thread_fast_mode_off, "f{0}(0)"
.extern start_streaming_slave, "f{0}(chd)"
.extern start_streaming_master, "f{0}(chd)"
.extern stop_streaming_slave, "f{0}(chd)"
.extern stop_streaming_master, "f{0}(chd)"
.extern outuchar, "f{0}(chd,uc)"
.extern outuint, "f{0}(chd,ui)"
.extern inuchar, "f{uc}(chd)"
.extern inuint, "f{ui}(chd)"
.extern inuchar_byref, "f{0}(chd,&(uc))"
.extern inuint_byref, "f{0}(chd,&(ui))"
.extern sync, "f{0}(w:p)"
.extern peek, "f{ui}(w:p)"
.extern clearbuf, "f{0}(w:p)"
.extern endin, "f{ui}(w:p)"
.extern partin, "f{ui}(w:p,ui)"
.extern partout, "f{0}(w:p,ui,ui)"
.extern partout_timed, "f{ui}(w:p,ui,ui,ui)"
.extern partin_timestamped, "f{ui,ui}(w:p,ui)"
.extern partout_timestamped, "f{ui}(w:p,ui,ui)"
.extern outct, "f{0}(chd,uc)"
.extern chkct, "f{0}(chd,uc)"
.extern inct, "f{uc}(chd)"
.extern inct_byref, "f{0}(chd,&(uc))"
.extern testct, "f{si}(chd)"
.extern testwct, "f{si}(chd)"
.extern soutct, "f{0}(m:chd,uc)"
.extern schkct, "f{0}(m:chd,uc)"
.extern sinct, "f{uc}(m:chd)"
.extern sinct_byref, "f{0}(m:chd,&(uc))"
.extern stestct, "f{si}(m:chd)"
.extern stestwct, "f{si}(m:chd)"
.extern out_char_array, "ft{0}(chd,&(a(:c:uc)),ui)"
.extern in_char_array, "ft{0}(chd,&(a(:uc)),ui)"
.extern crc32, "f{0}(&(ui),ui,ui)"
.extern crc8shr, "f{ui}(&(ui),ui,ui)"
.extern lmul, "f{ui,ui}(ui,ui,ui,ui)"
.extern mac, "f{ui,ui}(ui,ui,ui,ui)"
.extern macs, "f{si,ui}(si,si,si,ui)"
.extern sext, "f{si}(ui,ui)"
.extern zext, "f{ui}(ui,ui)"
.extern pinseq, "f{0}(ui)"
.extern pinsneq, "f{0}(ui)"
.extern pinseq_at, "f{0}(ui,ui)"
.extern pinsneq_at, "f{0}(ui,ui)"
.extern timerafter, "f{0}(ui)"
.extern getps, "f{ui}(ui)"
.extern setps, "f{0}(ui,ui)"
.extern read_pswitch_reg, "f{si}(ui,ui,&(ui))"
.extern read_sswitch_reg, "f{si}(ui,ui,&(ui))"
.extern write_pswitch_reg, "f{si}(ui,ui,ui)"
.extern write_pswitch_reg_no_ack, "f{si}(ui,ui,ui)"
.extern write_sswitch_reg, "f{si}(ui,ui,ui)"
.extern write_sswitch_reg_no_ack, "f{si}(ui,ui,ui)"
.extern read_node_config_reg, "f{si}(cr,ui,&(ui))"
.extern write_node_config_reg, "f{si}(cr,ui,ui)"
.extern write_node_config_reg_no_ack, "f{si}(cr,ui,ui)"
.extern read_periph_8, "f{si}(cr,ui,ui,ui,&(a(:uc)))"
.extern write_periph_8, "f{si}(cr,ui,ui,ui,&(a(:c:uc)))"
.extern write_periph_8_no_ack, "f{si}(cr,ui,ui,ui,&(a(:c:uc)))"
.extern read_periph_32, "f{si}(cr,ui,ui,ui,&(a(:ui)))"
.extern write_periph_32, "f{si}(cr,ui,ui,ui,&(a(:c:ui)))"
.extern write_periph_32_no_ack, "f{si}(cr,ui,ui,ui,&(a(:c:ui)))"
.extern get_core_id, "f{ui}(0)"
.extern get_thread_id, "f{ui}(0)"
.extern __builtin_getid, "f{si}(0)"
.extern printchar, "f{si}(uc)"
.extern printcharln, "f{si}(uc)"
.extern printint, "f{si}(si)"
.extern printintln, "f{si}(si)"
.extern printuint, "f{si}(ui)"
.extern printuintln, "f{si}(ui)"
.extern printhex, "f{si}(ui)"
.extern printhexln, "f{si}(ui)"
.extern printstr, "f{si}(&(a(:c:uc)))"
.extern printstrln, "f{si}(&(a(:c:uc)))"
.extern write_sswitch_reg_blind, "f{si}(ui,ui,ui)"
          .text
.cc_top device_reboot.function,device_reboot
          .align    4
.LDBG5:
.call device_reboot, __builtin_outct
.call device_reboot, __builtin_inct
.call device_reboot, get_core_id
.call device_reboot, read_sswitch_reg
.call device_reboot, write_sswitch_reg_blind
.call device_reboot, write_sswitch_reg_blind
.set __builtin_outct, 0
.linkset __builtin_outct.locnoside, 0
.linkset __builtin_outct.locnochandec, 1
.set __builtin_inct, 0
.linkset __builtin_inct.locnoside, 0
.linkset __builtin_inct.locnochandec, 1
.globl device_reboot, "f{0}(chd)"
.globl device_reboot.nstackwords
.globl device_reboot.maxthreads
.globl device_reboot.maxtimers
.globl device_reboot.maxchanends
.globl device_reboot.maxsync
.type  device_reboot, @function
.linkset device_reboot.locnoside, 1
.linkset device_reboot.locnochandec, 1
.linkset .LLNK4, get_core_id.nstackwords $M read_sswitch_reg.nstackwords
.linkset .LLNK3, .LLNK4 $M write_sswitch_reg_blind.nstackwords
.linkset .LLNK2, .LLNK3 $M write_sswitch_reg_blind.nstackwords
.linkset .LLNK1, .LLNK2 $M .LLNK2
.linkset .LLNK0, .LLNK1 + 3
.linkset device_reboot.nstackwords, .LLNK0
.LDBG11:
.LDBG8:

################
# Entry to function `device_reboot' (Implied thread 0)
################
# STACK
# -------|-workspace------
# -----2-| pllVal
# -----1-| _t0
# -----0-| rsvd
# ------------------------
# r0 = [f:spare, t:8, l:core_id, t:11, t:12, t:13, t:15, t:16, t:17, t:18, t:20, t:22, t:25]
# r1 = [t:9, t:24, t:26, t:27]
# r2 = [t:14, t:19, t:23]
# r4 = [t:10, t:21]
# r5 = [t:1]
# r6 = [t:2]
# r7 = [t:3]
# r8 = [t:4]
# r9 = [t:5]
# r10 = [t:6]
# Code
device_reboot:
          entsp     0x3              # extend sp, save lr (mem[sp]=r15)
.LDBG12:
          stw       r4, sp[0x1]      # cross-thread move to stack location
.LDBG9:
          .file     1 "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .loc      1 9 0

          .loc      1 10 0

.L2_bb_begin:
          .loc      1 10 0

          .loc      1 10 0

.L20_EndPoint:
          outct     res[r0], 0x1     # output ctrl token on `_t0_r0'
          .loc      1 11 0

.L5_bb_begin:
          .loc      1 11 0

          .loc      1 11 0

.L21_EndPoint:
          inct      r1, res[r0]      # input control token on `_t0_r0'
.L7_bb_begin:
          freer res[r0]
.LDBG6:
          .loc      1 18 0

.L22_Call:
          bl        get_core_id 
          mov       r4, r0           # move r4 <-- r0
.L10_bb_begin:
          .loc      1 19 0

.L13_bb_begin:
          .loc      1 19 0

          ldaw      r2, sp[0x2] 
          ldc       r1, 0x6
          .loc      1 19 0

.L23_Call:
          bl        read_sswitch_reg 
          .loc      1 20 0

.L16_bb_begin:
          .loc      1 20 0

          ldc       r0, 0x8000
          xor       r0, r4, r0
          ldw       r2, sp[0x2] 
          ldc       r1, 0x6
          .loc      1 20 0

.L24_Call:
          bl        write_sswitch_reg_blind 
          .loc      1 21 0

.L18_bb_begin:
          .loc      1 21 0

          ldw       r2, sp[0x2] 
          mov       r0, r4           # move r0 <-- r4
          ldc       r1, 0x6
          .loc      1 21 0

.L25_Call:
          bl        write_sswitch_reg_blind 
.LDBG7:
          .loc      1 23 0

.LDBG13:
          ldw       r4, sp[0x1] 
.LDBG14:
.L19_bb_begin:
          retsp     0x3              # return: dealloc and link (pc=r15=mem[sp])
.LDBG10:
.LDBG15:
.LDBG16:
.size device_reboot, .-device_reboot
.cc_bottom device_reboot.function
          .section  .debug_frame, "",     @progbits
.cc_top device_reboot.function,device_reboot
          .align    4
          .int      .LDBG18-.LDBG17
.LDBG17:
          .long     .LDBG2           # offset in .debug_frame
          .int      .LDBG11
          .int      .LDBG16-.LDBG11
          .byte     0x01
          .int      .LDBG12
          .byte     0x0e
          .uleb128  0xc
          .byte     0x14
          .uleb128  0xe
          .uleb128  0x0
          .byte     0x8f
          .uleb128  0x0
          .byte     0x01
          .int      .LDBG13
          .byte     0x0a
          .byte     0x01
          .int      .LDBG14
          .byte     0xc4
          .byte     0x01
          .int      .LDBG15
          .byte     0x0b
          .align    4, 0
.LDBG18:
.cc_bottom device_reboot.function

# PROFILE
.linkset .LLNK7, get_core_id.maxchanends $M read_sswitch_reg.maxchanends
.linkset .LLNK6, .LLNK7 $M write_sswitch_reg_blind.maxchanends
.linkset .LLNK5, .LLNK6 $M write_sswitch_reg_blind.maxchanends
.linkset device_reboot.maxchanends, 0#.LLNK5
.linkset .LLNK10, get_core_id.maxtimers $M read_sswitch_reg.maxtimers
.linkset .LLNK9, .LLNK10 $M write_sswitch_reg_blind.maxtimers
.linkset .LLNK8, .LLNK9 $M write_sswitch_reg_blind.maxtimers
.linkset device_reboot.maxtimers, .LLNK8
.linkset .LLNK16, get_core_id.maxthreads - 1
.linkset .LLNK15, 1 + .LLNK16
.linkset .LLNK14, 1 $M .LLNK15
.linkset .LLNK18, read_sswitch_reg.maxthreads - 1
.linkset .LLNK17, 1 + .LLNK18
.linkset .LLNK13, .LLNK14 $M .LLNK17
.linkset .LLNK20, write_sswitch_reg_blind.maxthreads - 1
.linkset .LLNK19, 1 + .LLNK20
.linkset .LLNK12, .LLNK13 $M .LLNK19
.linkset .LLNK22, write_sswitch_reg_blind.maxthreads - 1
.linkset .LLNK21, 1 + .LLNK22
.linkset .LLNK11, .LLNK12 $M .LLNK21
.linkset device_reboot.maxthreads, .LLNK11
###############

          .text
.LDBG19:
# Thread names for recovering thread graph in linker
.LDBG20:
.extern __builtin_getid, "f{si}(0)"
.extern __builtin_getid, "f{si}(0)"
          .section  .debug_info,   "",    @progbits
.LDBG22:
          .int      .LDBG24-.LDBG23
.LDBG23:
          .short    0x0003
          .long     .LDBG21          # offset in .debug_abbrev
          .byte     0x04
          .uleb128  0x1
          .long     .LDBG0           # low address
          .long     .LDBG20          # high address
          .asciiz   "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
          .short    0xc000
          .asciiz   "XMOS Dwarf Symbolic Debug Generator"
          .long     .LDBG1           # offset in .debug_lineprog
.LDBG25:
          .uleb128  0x2
          .asciiz   "long"
          .byte     0x05
          .byte     0x04
.LDBG26:
          .uleb128  0x2
          .asciiz   "unsigned long"
          .byte     0x07
          .byte     0x04
.LDBG27:
          .uleb128  0x2
          .asciiz   "int"
          .byte     0x05
          .byte     0x04
.LDBG28:
          .uleb128  0x2
          .asciiz   "unsigned int"
          .byte     0x07
          .byte     0x04
.LDBG29:
          .uleb128  0x2
          .asciiz   "short"
          .byte     0x05
          .byte     0x02
.LDBG30:
          .uleb128  0x2
          .asciiz   "unsigned short"
          .byte     0x07
          .byte     0x02
.LDBG31:
          .uleb128  0x2
          .asciiz   "char"
          .byte     0x06
          .byte     0x01
.LDBG32:
          .uleb128  0x2
          .asciiz   "unsigned char"
          .byte     0x08
          .byte     0x01
.LDBG33:
          .uleb128  0x2
          .asciiz   "chanend"
          .byte     0x07
          .byte     0x04
.LDBG34:
          .uleb128  0x2
          .asciiz   "timer"
          .byte     0x07
          .byte     0x04
.LDBG35:
          .uleb128  0x2
          .asciiz   "clock"
          .byte     0x07
          .byte     0x04
.LDBG36:
          .uleb128  0x2
          .asciiz   "port"
          .byte     0x07
          .byte     0x04
.LDBG37:
          .uleb128  0x2
          .asciiz   "buffered port:1"
          .byte     0x07
          .byte     0x04
.LDBG38:
          .uleb128  0x2
          .asciiz   "buffered port:4"
          .byte     0x07
          .byte     0x04
.LDBG39:
          .uleb128  0x2
          .asciiz   "buffered port:8"
          .byte     0x07
          .byte     0x04
.LDBG40:
          .uleb128  0x2
          .asciiz   "buffered port:16"
          .byte     0x07
          .byte     0x04
.LDBG41:
          .uleb128  0x2
          .asciiz   "buffered port:32"
          .byte     0x07
          .byte     0x04
.cc_top device_reboot.function,device_reboot
.LDBG42:
          .uleb128  0x3
          .asciiz   "device_reboot"
          .byte     0x01
          .byte     0x09
          .byte     0x01
          .byte     0x01
          .long     .LDBG5           # low address
          .long     .LDBG19          # high address
          .uleb128  0x4
          .asciiz   "spare"
          .byte     0x01
          .short    0x0008
          .int      .LDBG33-.LDBG22
          .int      .LDBG43
          .section  .debug_loc,    "",    @progbits
.cc_top device_reboot.function,device_reboot
.LDBG43:
          .int      .LDBG9-.LDBG0
          .int      .LDBG10-.LDBG0
          .short    .LDBG45-.LDBG44
.LDBG44:
          .byte     0x50
.LDBG45:
          .int      0x00000000
          .int      0x00000000
.cc_bottom device_reboot.function
          .section  .debug_info,   "",    @progbits
.LDBG46:
          .uleb128  0x5
          .long     .LDBG6           # low address
          .long     .LDBG7           # high address
          .uleb128  0x6
          .asciiz   "pllVal"
          .byte     0x01
          .short    0x0011
          .short    .LDBG47-.LDBG46
          .int      .LDBG28-.LDBG22
          .int      .LDBG48
          .section  .debug_loc,    "",    @progbits
.cc_top device_reboot.function,device_reboot
.LDBG48:
          .int      .LDBG6-.LDBG0
          .int      .LDBG7-.LDBG0
          .short    .LDBG50-.LDBG49
.LDBG49:
          .byte     0x7e
          .sleb128  0x8
.LDBG50:
          .int      0x00000000
          .int      0x00000000
.cc_bottom device_reboot.function
          .section  .debug_info,   "",    @progbits
.LDBG47:
          .uleb128  0x6
          .asciiz   "core_id"
          .byte     0x01
          .short    0x0012
          .short    0x0000
          .int      .LDBG28-.LDBG22
          .int      .LDBG51
          .section  .debug_loc,    "",    @progbits
.cc_top device_reboot.function,device_reboot
.LDBG51:
          .int      0x00000000
          .int      0x00000000
.cc_bottom device_reboot.function
          .section  .debug_info,   "",    @progbits
          .byte     0x00
          .byte     0x00
.cc_bottom device_reboot.function
          .byte     0x00
.LDBG24:
          .section  .debug_pubnames, "",  @progbits
          .int      .LDBG53-.LDBG52
.LDBG52:
          .short    0x0002
          .long     .LDBG22          # offset in .debug_info
          .int      .LDBG24-.LDBG22
.cc_top device_reboot.function,device_reboot
          .int      .LDBG42-.LDBG22
          .asciiz   "device_reboot"
.cc_bottom device_reboot.function
          .int      0x00000000
.LDBG53:
          .section  .debug_abbrev, "",    @progbits
.LDBG21:
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
          .uleb128  0x6
          .byte     0x34
          .byte     0x00
          .byte     0x03
          .byte     0x08
          .byte     0x3a
          .byte     0x0b
          .byte     0x3b
          .byte     0x05
          .byte     0x2c
          .byte     0x05
          .byte     0x49
          .byte     0x13
          .byte     0x02
          .byte     0x06
          .byte     0x00
          .byte     0x00
          .uleb128  0x5
          .byte     0x0b
          .byte     0x01
          .byte     0x11
          .byte     0x01
          .byte     0x12
          .byte     0x01
          .byte     0x00
          .byte     0x00

          .byte     0x00
          .section .xtaendpointtable,       "", @progbits
.L26_xta_begin:
          .int      .L27_xta_end-.L26_xta_begin
          .int      0x00000000
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x0000000b
          .long    .L21_EndPoint
.cc_bottom device_reboot.function
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x0000000a
          .long    .L20_EndPoint
.cc_bottom device_reboot.function
.L27_xta_end:
          .section .xtacalltable,       "", @progbits
.L28_xta_begin:
          .int      .L29_xta_end-.L28_xta_begin
          .int      0x00000000
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000015
          .long    .L25_Call
.cc_bottom device_reboot.function
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000014
          .long    .L24_Call
.cc_bottom device_reboot.function
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000013
          .long    .L23_Call
.cc_bottom device_reboot.function
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000012
          .long    .L22_Call
.cc_bottom device_reboot.function
.L29_xta_end:
          .section .xtalabeltable,       "", @progbits
.L30_xta_begin:
          .int      .L31_xta_end-.L30_xta_begin
          .int      0x00000000
          .asciiz   "/local/USBAudio/sw_usb_aud_l1_ios/app_usb_aud_l1/.build"
.cc_top device_reboot.function, device_reboot
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000017
          .int      0x00000017
# line info for line 23 
          .long    .L19_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000015
          .int      0x00000015
# line info for line 21 
          .long    .L18_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000014
          .int      0x00000014
# line info for line 20 
          .long    .L16_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000013
          .int      0x00000013
# line info for line 19 
          .long    .L13_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x00000012
          .int      0x00000012
# line info for line 18 
          .long    .L10_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x0000000d
          .int      0x0000000d
# line info for line 13 
          .long    .L7_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x0000000b
          .int      0x0000000b
# line info for line 11 
          .long    .L5_bb_begin
          .asciiz  "../../../sc_usb_audio/module_usb_aud_shared/reboot.xc"
          .int      0x0000000a
          .int      0x0000000a
# line info for line 10 
          .long    .L2_bb_begin
.cc_bottom device_reboot.function
.L31_xta_end:
          .section .dp.data,       "adw", @progbits
.align 4
          .align    4
          .section .dp.bss,        "adw", @nobits
.align 4
          .ident    "XMOS 32-bit XC Compiler 11.11.0beta1 (build 2136)"
          .core     "XS1"
          .corerev  "REVB"

# memory access instructions: 6
# total instructions: 20
########################################
