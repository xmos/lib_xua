#ifndef __xc_ptr__
#define __xc_ptr__

typedef unsigned int xc_ptr;

inline xc_ptr array_to_xc_ptr(unsigned a[]) {
  xc_ptr x;
  asm("mov %0, %1":"=r"(x):"r"(a));
  return x;
}

#define write_via_xc_ptr(p,x)   asm("stw %0, %1[0]"::"r"(x),"r"(p))

#define write_via_xc_ptr_indexed(p,i,x)   asm("stw %0, %1[%2]"::"r"(x),"r"(p),"r"(i))
#define write_byte_via_xc_ptr_indexed(p,i,x)   asm("st8 %0, %1[%2]"::"r"(x),"r"(p),"r"(i))

#define read_via_xc_ptr(x,p)  asm("ldw %0, %1[0]":"=r"(x):"r"(p));

#define read_via_xc_ptr_indexed(x,p,i)  asm("ldw %0, %1[%2]":"=r"(x):"r"(p),"r"(i));
#define read_byte_via_xc_ptr_indexed(x,p,i)  asm("ld8u %0, %1[%2]":"=r"(x):"r"(p),"r"(i));

#define GET_SHARED_GLOBAL(x, g) asm("ldw %0, dp[" #g "]":"=r"(x))
#define SET_SHARED_GLOBAL(g, v) asm("stw %0, dp[" #g "]"::"r"(v))

#endif 
