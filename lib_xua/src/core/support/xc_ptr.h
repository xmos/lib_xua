// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __xc_ptr__
#define __xc_ptr__

typedef unsigned int xc_ptr;

// Note that this function is marked as const to avoid the XC
// parallel usage checks, this is only really going to work if this
// is the *only* way the array a is accessed (and everything else uses
// the xc_ptr)
inline xc_ptr array_to_xc_ptr(const unsigned a[])
{
    xc_ptr x;
    asm("mov %0, %1":"=r"(x):"r"(a));
    return x;
}

#define write_via_xc_ptr_indexed(p,i,x)         asm volatile("stw %0, %1[%2]"::"r"(x),"r"(p),"r"(i))
#define write_byte_via_xc_ptr_indexed(p,i,x)    asm volatile("st8 %0, %1[%2]"::"r"(x),"r"(p),"r"(i))
#define write_byte_via_xc_ptr_indexed(p,i,x)    asm volatile("st8 %0, %1[%2]"::"r"(x),"r"(p),"r"(i))
#define write_short_via_xc_ptr_indexed(p,i,x)   asm volatile("st16 %0, %1[%2]"::"r"(x),"r"(p),"r"(i))

#define write_via_xc_ptr(p,x)                   asm volatile("stw %0, %1[0]"::"r"(x),"r"(p))
// No immediate st8 format
#define write_byte_via_xc_ptr(p,x)              write_byte_via_xc_ptr_indexed(p, 0, x)
#define write_short_via_xc_ptr(p,x)             write_short_via_xc_ptr_indexed(p, 0, x)

#define read_via_xc_ptr_indexed(x,p,i)          asm("ldw %0, %1[%2]":"=r"(x):"r"(p),"r"(i));
#define read_byte_via_xc_ptr_indexed(x,p,i)     asm("ld8u %0, %1[%2]":"=r"(x):"r"(p),"r"(i));
#define read_short_via_xc_ptr_indexed(x,p,i)    asm("ld16s %0, %1[%2]":"=r"(x):"r"(p),"r"(i));

#define read_via_xc_ptr(x,p)                    asm("ldw %0, %1[0]":"=r"(x):"r"(p));
// No immediate ld8u format
#define read_byte_via_xc_ptr(x,p)               read_byte_via_xc_ptr_indexed(x, p, 0)
#define read_short_via_xc_ptr(x,p)              read_short_via_xc_ptr_indexed(x, p, 0)

#define GET_SHARED_GLOBAL(x, g) asm volatile("ldw %0, dp[" #g "]":"=r"(x)::"memory")
#define SET_SHARED_GLOBAL(g, v) asm volatile("stw %0, dp[" #g "]"::"r"(v):"memory")

#endif
