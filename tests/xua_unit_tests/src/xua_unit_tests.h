// Copyright 2021-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XUA_UNIT_TESTS_H_
#define XUA_UNIT_TESTS_H_

#include "unity.h"
#include "xua_conf.h"
#include "../../../lib_xua/src/midi/queue.h"

#ifndef __XC__
void midi_in_parse_c_wrapper(void * mips, unsigned cable_number, unsigned char b, unsigned * valid, unsigned * packed);
void midi_out_parse_c_wrapper(unsigned tx_data, unsigned midi[3], unsigned * size);
void reset_midi_state_c_wrapper(void *mips);
unsigned random(unsigned *x);

void queue_init_c_wrapper(queue_t *q, unsigned size);
int queue_is_empty_c_wrapper(const queue_t *q);
int queue_is_full_c_wrapper(const queue_t *q);
void queue_push_word_c_wrapper(queue_t *q, unsigned array[], unsigned data);
unsigned queue_pop_word_c_wrapper(queue_t *q, unsigned array[]);
void queue_push_byte_c_wrapper(queue_t *q, unsigned char array[], unsigned data);
unsigned queue_pop_byte_c_wrapper(queue_t *q, unsigned char array[]);
unsigned queue_items_c_wrapper(const queue_t *q);
unsigned queue_space_c_wrapper(const queue_t *q);

#endif

#endif /* XUA_UNIT_TESTS_H_ */
