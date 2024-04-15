// Copyright 2021-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XUA_UNIT_TESTS_H_
#define XUA_UNIT_TESTS_H_

#include "unity.h"
#include "xua_conf.h"

#ifndef __XC__
void midi_in_parse_wrap(void * mips, unsigned cable_number, unsigned char b, unsigned * valid, unsigned * packed);
void midi_out_parse_wrap(unsigned tx_data, unsigned midi[3], unsigned * size);
void reset_midi_state_wrap(void *mips);
unsigned random(unsigned *x);
#endif

#endif /* XUA_UNIT_TESTS_H_ */
