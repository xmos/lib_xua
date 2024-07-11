// Copyright 2013-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef QUEUE_H_
#define QUEUE_H_

#include "midioutparse.h"
#include "xassert.h"


typedef struct queue_t {
    /// Read index.
    unsigned rdptr;
    /// Write index.
    unsigned wrptr;
    unsigned size;
    unsigned mask;
} queue_t;

#ifdef __XC__

inline int is_power_of_2(unsigned x) {
    return x != 0 && (x & (x - 1)) == 0;
}

inline void queue_init(queue_t &q, unsigned size) {
    xassert(is_power_of_2(size) && "MIDI FIFO size must be a power of 2"); // Keep this enabled as will be discovered duirng dev time
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

    if(queue_is_full(q)) {
        xassert(0 && "Unexpected push to MIDI queue when full");
        // Silently drop message if asserts not enabled
        return;
    }

    array[q.wrptr++ & q.mask] = data;
}

inline unsigned queue_pop_word(queue_t &q, unsigned array[]) {
    if(queue_is_empty(q)){
        xassert(0 && "Unexpected pop from MIDI queue when empty");
        // Return NULL messaqe if asserts not enabled
        return MIDI_OUT_NULL_MESSAGE;
    }

    return array[q.rdptr++ & q.mask];
}


inline unsigned queue_items(const queue_t &q) {
    return q.wrptr - q.rdptr;
}

inline unsigned queue_space(const queue_t &q) {
    return q.size - queue_items(q);
}

#endif // __XC__

#endif /* QUEUE_H_ */
