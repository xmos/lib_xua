// Copyright 2013-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "queue.h"

// Force external definitions of inline functions.
extern inline int is_power_of_2(unsigned x);
extern inline void queue_init(queue_t &q, unsigned size);
extern inline int queue_is_empty(const queue_t &q);
extern inline int queue_is_full(const queue_t &q);
extern inline void queue_push_word(queue_t &q, unsigned array[], unsigned data);
extern inline unsigned queue_pop_word(queue_t &q, unsigned array[]);
extern inline void queue_push_byte(queue_t &q, unsigned char array[], unsigned data);
extern inline unsigned queue_pop_byte(queue_t &q, unsigned char array[]);
extern inline unsigned queue_space(const queue_t &q);
extern inline unsigned queue_items(const queue_t &q);
