// Copyright 2018-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xua.h"

#ifndef MAX_MIX_COUNT
#error
#endif

#ifndef MIX_INPUTS
#error
#endif

#if (MAX_MIX_COUNT > 0)

#define DOMIX_TOP(i) \
.cc_top doMix##i.function,doMix##i; \
          .align    16 ;\
.globl doMix##i ;\
.type doMix##i, @function ;\
.globl doMix##i##.nstackwords ;\
.globl doMix##i##.maxthreads ; \
.globl doMix##i##.maxtimers ; \
.globl doMix##i##.maxchanends ; \
.globl doMix##i##.maxsync ;\
.linkset doMix##i##.locnoside, 1; \
.linkset doMix##i##.locnochandec, 1;\
.linkset doMix##i##.nstackwords, 0 ;\
.linkset doMix##i##.maxchanends, 0 ;\
.linkset doMix##i##.maxtimers, 0 ;\
.linkset doMix##i##.maxthreads, 1; \
doMix##i##: ;\
          ENTSP_lu6 0; \
          set       cp, r0; \
          set       dp, r1; \
          lsub      r0, r1, r0, r0, r0;\
          .label_##i##:

#define DOMIX_BOT(i) \
          ldap      r11, _dp; \
          set       dp, r11;\
          ldap      r11, _cp;\
          set       cp, r11;\
\
          mov       r0, r1;\
          ldc       r2, 0x19;\
          sext      r0, r2;\
          eq        r0, r0, r1;\
          bf        r0, .L20; \
\
          shl       r0, r1, 0x7;\
          retsp     0x0;\
\
\
.size doMix##i, .-doMix##i; \
.cc_bottom doMix##i##.function;

#define N MIX_INPUTS
#define BODY(i) \
          ldw       r2,cp[i]; \
          ldw       r11, dp[i];  \
          maccs     r1, r0, r2, r11;


.text

.L20:\
          lss       r0, r1, r3;\
          bt        r0, .L16; \
          ldw       r0, cp[.LC0];\
          retsp     0x0; \
.L16:\
          ldw       r0, cp[.LC1];\
          retsp     0x0; \


#if(MAX_MIX_COUNT > 0)
DOMIX_TOP(0)
#include "repeat.h"
DOMIX_BOT(0)
#endif


#if(MAX_MIX_COUNT > 1)
DOMIX_TOP(1)
#include "repeat.h"
DOMIX_BOT(1)
#endif

#if(MAX_MIX_COUNT > 2)
DOMIX_TOP(2)
#include "repeat.h"
DOMIX_BOT(2)
#endif

#if(MAX_MIX_COUNT > 3)
DOMIX_TOP(3)
#include "repeat.h"
DOMIX_BOT(3)
#endif

#if(MAX_MIX_COUNT > 4)
DOMIX_TOP(4)
#include "repeat.h"
DOMIX_BOT(4)
#endif

#if(MAX_MIX_COUNT > 5)
DOMIX_TOP(5)
#include "repeat.h"
DOMIX_BOT(5)
#endif

#if(MAX_MIX_COUNT > 6)
DOMIX_TOP(6)
#include "repeat.h"
DOMIX_BOT(6)
#endif

#if(MAX_MIX_COUNT > 7)
DOMIX_TOP(7)
#include "repeat.h"
DOMIX_BOT(7)
#endif

#if(MAX_MIX_COUNT>8)
#error MAX_MIX_COUNT>7
#endif

/* We need MIX_OUTPUT x setPtr functions */

#undef N
#undef BODY
#define N MAX_MIX_COUNT
.cc_top setPtr.function,setPtr;
.align    16 ;
.globl setPtr;
.type setPtr, @function
.globl setPtr.nstackwords;
.globl setPtr.maxthreads;
.globl setPtr.maxtimers;
.globl setPtr.maxchanends;
.globl setPtr.maxsync;
.linkset setPtr.locnoside, 1;
.linkset setPtr.locnochandec, 1;
.linkset setPtr.nstackwords, 0;
.linkset setPtr.maxchanends, 0;
.linkset setPtr.maxtimers, 0;
.linkset setPtr.maxthreads, 1;
setPtr:
  ENTSP_lu6 0
  shl   r2, r2, 1
.xtabranch  .label_0
  bru   r2
.syntax architectural
#define BODY(i) \
  LDAPF_u10  .label_##i; \
  BRFU_u6 setPtr_go
#include "repeat.h"
.syntax default
setPtr_go:
  shl   r0, r0, 3;
  ldc   r2, 0x80;
  add   r1, r1, r2;
  st8   r1, r11[r0];
  retsp 0;
.size setPtr, .-setPtr
.cc_bottom setPtr.function

          .section .cp.const4,     "acM", @progbits, 4
.cc_top .LC0.data
          .align    4
.LC0:
          .int      0x7fffff00
.cc_bottom .LC0.data
.cc_top .LC1.data
          .align    4
.LC1:
          .int      0x80000000
.cc_bottom .LC1.data

#undef N
#undef BODY

#endif

