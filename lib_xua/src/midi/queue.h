// Copyright 2013-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef QUEUE_H_
#define QUEUE_H_

#define assert(x) asm("ecallf %0"::"r"(x));

typedef struct queue_t {
    /// Read index.
    unsigned rdptr;
    /// Write index.
    unsigned wrptr;
    unsigned size;
    unsigned mask;
} queue_t;

inline int is_power_of_2(unsigned x) {
    return x != 0 && (x & (x - 1)) == 0;
}

inline void queue_init(queue_t &q, unsigned size) {
    assert(is_power_of_2(size));
    q.rdptr = 0;
    q.wrptr = 0;
    q.size = size;
    q.mask = size - 1; // Assumes power of two.
}

inline int queue_is_empty(const queue_t &q) {
    return q.wrptr == q.rdptr;
}

inline int queue_is_full(const queue_t &q) {
    return q.wrptr - q.rdptr == q.size;
}

inline void queue_push_word(queue_t &q, unsigned array[], unsigned data)
{
    assert(!queue_is_full(q));
    array[q.wrptr++ & q.mask] = data;
}

inline unsigned queue_pop_word(queue_t &q, unsigned array[]) {
    assert(!queue_is_empty(q));
    return array[q.rdptr++ & q.mask];
}

inline void queue_push_byte(queue_t &q, unsigned char array[], unsigned data)
{
    assert(!queue_is_full(q));
    array[q.wrptr++ & q.mask] = data;
}

inline unsigned queue_pop_byte(queue_t &q, unsigned char array[]) {
    assert(!queue_is_empty(q));
    return array[q.rdptr++ & q.mask];
}

inline unsigned queue_items(const queue_t &q) {
    return q.wrptr - q.rdptr;
}

inline unsigned queue_space(const queue_t &q) {
    return q.size - queue_items(q);
}

#endif /* QUEUE_H_ */
