#ifndef __interrupt_h__
#define __interrupt_h__

#define store_args0(c) \
  asm("kentsp 19; stw %0, sp[1]; krestsp 19"::"r"(c));

#define store_args1(c,x) \
  asm("kentsp 20; stw %0, sp[1]; stw %1, sp[2]; krestsp 20"::"r"(c),"r"(x));

#define store_args2(c,x0,x1)       \
  asm("kentsp 21; stw %0, sp[1];" \
      "stw %1, sp[2];" \
      "stw %2, sp[3];"  \
      " krestsp 21"::"r"(c),"r"(x0),"r"(x1));

#define store_args3(c,x0,x1,x2)   \
  asm("kentsp 22; stw %0, sp[1];" \
      "stw %1, sp[2];" \
      "stw %2, sp[3];"  \
      "stw %3, sp[4];"  \
      " krestsp 22"::"r"(c),"r"(x0),"r"(x1),"r"(x2));

#define store_args4(c,x0,x1,x2,x3)  \
  asm("kentsp 23; stw %4, sp[1];" \
      "stw %0, sp[2];" \
      "stw %1, sp[3];" \
      "stw %2, sp[4];"  \
      "stw %3, sp[5];"  \
      " krestsp 23"::"r"(c),"r"(x0),"r"(x1),"r"(x2),"r"(x3));

#define store_args5(c,x0,x1,x2,x3,x4)             \
  asm("kentsp 24;" \
      "stw %4, sp[1];" \
      "stw %5, sp[2];" \
      "stw %0, sp[3];" \
      "stw %1, sp[4];" \
      "stw %2, sp[5];"  \
      "stw %3, sp[6];"  \
      " krestsp 24"::"r"(c),"r"(x0),"r"(x1),"r"(x2),"r"(x3),"r"(x4));

#define store_args6(c,x0,x1,x2,x3,x4,x5)          \
  asm("kentsp 25;" \
      "stw %4, sp[1];" \
      "stw %5, sp[2];" \
      "stw %6, sp[3];" \
      "stw %0, sp[4];" \
      "stw %1, sp[5];" \
      "stw %2, sp[6];"  \
      "stw %3, sp[7];"  \
      " krestsp 25"::"r"(c),"r"(x0),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5));

#define store_args7(c,x0,x1,x2,x3,x4,x5,x6)       \
  asm("kentsp 26;" \
      "stw %4, sp[1];" \
      "stw %5, sp[2];" \
      "stw %6, sp[3];" \
      "stw %7, sp[4];" \
      "stw %0, sp[5];" \
      "stw %1, sp[6];" \
      "stw %2, sp[7];"  \
      "stw %3, sp[8];"  \
      " krestsp 26"::"r"(c),"r"(x0),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5),"r"(x6));

#define store_args8(c,x0,x1,x2,x3,x4,x5,x6,x7)    \
  asm("kentsp 27;" \
      "stw %4, sp[1];" \
      "stw %5, sp[2];" \
      "stw %6, sp[3];" \
      "stw %7, sp[4];" \
      "stw %8, sp[5];" \
      "stw %0, sp[6];" \
      "stw %1, sp[7];" \
      "stw %2, sp[8];"  \
      "stw %3, sp[9];"  \
      " krestsp 27"::"r"(c),"r"(x0),"r"(x1),"r"(x2),"r"(x3),"r"(x4),"r"(x5),"r"(x6),"r"(x7));




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

#define load_argsn(f, args) \
      ".linkset __"#f"_handler_arg0, "#args"-2\n"\
      "ldw r0, sp[" "__"#f"_handler_arg0" "]\n" \
      ".linkset __"#f"_handler_arg1, "#args"-1\n"\
      "ldw r1, sp[" "__"#f"_handler_arg1" "]\n" \
      ".linkset __"#f"_handler_arg2, "#args"-0\n"\
      "ldw r2, sp[" "__"#f"_handler_arg2" "]\n" \
      ".linkset __"#f"_handler_arg3, "#args"+1\n"\
      "ldw r3, sp[" "__"#f"_handler_arg3" "]\n"

#define load_args4(f) load_argsn(f,4)
#define load_args5(f) load_argsn(f,5)
#define load_args6(f) load_argsn(f,6)
#define load_args7(f) load_argsn(f,7)
#define load_args8(f) load_argsn(f,8)

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

#define do_interrupt_handler(f,args)    \
  asm("bu .L__" #f "_handler_skip;\n" \
      "__" #f "_handler:\n"  \
      "kentsp " #args " + 19\n" \
      "__kent:" \
      save_state(f,args)    \
      load_args ## args (f) \
      "bl " #f "\n" \
      restore_state(f,args)       \
      "krestsp " #args " + 19 \n" \
      "__kret:\n" \
      "kret\n" \
      ".L__" #f "_handler_skip:\n");

#define set_interrupt_handler(f, nstackwords, args, c, ...) \
  asm (" .section .dp.data,       \"adw\", @progbits\n" \
       " .align 4\n" \
       "__" #f "_kernel_stack:\n" \
       " .space  " #nstackwords ", 0\n" \
       " .text\n"); \
  asm("mov r10, %0; ldaw r11, dp[__" #f "_kernel_stack];add r11, r11, r10;ldaw r10, sp[0]; "\
      "set sp,r11;stw r10, sp[0]; krestsp 0"::"r"(nstackwords-8):"r10","r11"); \
  store_args ## args(c, __VA_ARGS__) \
  do_interrupt_handler(f, args) \
  asm("ldap r11, __" #f "_handler; setv res[%0],r11"::"r"(c):"r11");    \
  asm("setc res[%0], 0xa; eeu res[%0]"::"r"(c));                        \
  asm("setsr (((0) & ~(((1 << 0x1) - 1) << 0x1)) | (((1) << 0x1) & (((1 << 0x1) - 1) << 0x1)))");



#endif

