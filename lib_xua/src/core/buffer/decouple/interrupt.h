// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __interrupt_h__
#define __interrupt_h__

#define store_args0(c) \
  asm("kentsp 20; stw %0, sp[1]; krestsp 20"::"r"(c));

#define store_args1(c,x) \
  asm("kentsp 20; stw %0, sp[1]; stw %1, sp[2]; krestsp 20"::"r"(c),"r"(x));

#define store_args2(c,x0,x1)       \
  asm("kentsp 22; stw %0, sp[1];" \
      "stw %1, sp[2];" \
      "stw %2, sp[3];"  \
      " krestsp 22"::"r"(c),"r"(x0),"r"(x1));

#define store_args3(c,x0,x1,x2)   \
  asm("kentsp 22; stw %0, sp[1];" \
      "stw %1, sp[2];" \
      "stw %2, sp[3];"  \
      "stw %3, sp[4];"  \
      " krestsp 22"::"r"(c),"r"(x0),"r"(x1),"r"(x2));

#define load_args0(f) \
  "ldw r0, sp[1]\n"

#define load_args1(f)\
      "ldw r0, sp[1]\n" \
      "ldw r1, sp[2]\n"

#define load_args2(f)\
      "ldw r0, sp[1]\n" \
      "ldw r1, sp[2]\n" \
      "ldw r2, sp[3]\n"

#define load_args3(f)\
      "ldw r0, sp[1]\n" \
      "ldw r1, sp[2]\n" \
      "ldw r2, sp[3]\n" \
      "ldw r3, sp[4]\n"

#define save_state(f,args)                         \
  ".linkset __"#f"_handler_r0_save, "#args"+12\n"   \
  "stw r0, sp[" "__"#f"_handler_r0_save" "]\n" \
  ".linkset __"#f"_handler_r1_save, "#args"+13\n"   \
  "stw r1, sp[" "__"#f"_handler_r1_save" "]\n" \
  ".linkset __"#f"_handler_r2_save, "#args"+2\n"   \
  "stw r2, sp[" "__"#f"_handler_r2_save" "]\n" \
  ".linkset __"#f"_handler_r3_save, "#args"+3\n"   \
  "stw r3, sp[" "__"#f"_handler_r3_save" "]\n" \
  ".linkset __"#f"_handler_r11_save, "#args"+11\n"   \
  "stw r11, sp[" "__"#f"_handler_r11_save" "]\n" \
  ".linkset __"#f"_handler_lr_save, "#args"+14\n"   \
  "stw lr, sp[" "__"#f"_handler_lr_save" "]\n"

#define restore_state(f,args)                  \
  "ldw r0, sp[" "__"#f"_handler_r0_save" "]\n" \
  "ldw r1, sp[" "__"#f"_handler_r1_save" "]\n" \
  "ldw r2, sp[" "__"#f"_handler_r2_save" "]\n" \
  "ldw r3, sp[" "__"#f"_handler_r3_save" "]\n" \
  "ldw r11, sp[" "__"#f"_handler_r11_save" "]\n" \
  "ldw lr, sp[" "__"#f"_handler_lr_save" "]\n"


#define STRINGIFY0(x) #x
#define STRINGIFY(x) STRINGIFY0(x)

#define ENABLE_INTERRUPTS() asm("setsr " STRINGIFY(XS1_SR_IEBLE_SET(0, 1)))

#define DISABLE_INTERRUPTS() asm("clrsr " STRINGIFY(XS1_SR_IEBLE_SET(0, 1)))

//int ksp_enter, ksp_exit, r11_store;

#define ISSUE_MODE_SINGLE ".issue_mode single\n"
#define ISSUE_MODE_DUAL ".issue_mode dual\n"

#define do_interrupt_handler(f,args)    \
  asm(ISSUE_MODE_SINGLE\
      ".align 4\n" \
      ".cc_top __"#f"_handler.function,__"#f"_handler\n" \
      "__" #f "_handler:\n"  \
      "ENTSP_lu6 0\n" \
      "kentsp " #args "/2*2 + 20\n" \
      "__kent:" \
      save_state(f,args)    \
      load_args ## args (f) \
      "bl " #f "\n" \
      restore_state(f,args)       \
      "krestsp " #args "/2*2 + 20 \n" \
      "__kret:\n" \
      "kret\n" \
      ".cc_bottom __"#f"_handler.function");

#define register_interrupt_handler(f, args, nstackwords) \
  asm (" .section .dp.data,       \"adw\", @progbits\n"  \
       " .globl __" #f "_kernel_stack_end\n" \
       " .globl __" #f "_handler\n" \
       " .align 8\n"                                    \
       "__" #f "_kernel_stack:\n" \
       " .space "  #nstackwords ", 0\n" \
       "__" #f "_kernel_stack_end:\n" \
       " .space 4\n"\
       " .text\n");                             \
      do_interrupt_handler(f, args)

#define set_interrupt_handler(f, args, c, ...) \
  asm("ldaw r11, dp[__" #f "_kernel_stack_end];ldaw r10, sp[0]; " \
      "set sp,r11;stw r10, sp[0]; krestsp 0":::"r10","r11");      \
  store_args ## args(c, __VA_ARGS__) \
  asm("ldap r11, __" #f "_handler; setv res[%0],r11"::"r"(c):"r11");    \
  asm("setc res[%0], 0xa; eeu res[%0]"::"r"(c));                        \
  asm("setsr (((0) & ~(((1 << 0x1) - 1) << 0x1)) | (((1) << 0x1) & (((1 << 0x1) - 1) << 0x1)))");

#endif

