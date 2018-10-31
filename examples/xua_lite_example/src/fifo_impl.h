#ifndef __FIFO__
#define __FIFO__
#include <string.h> //memcpy
#include "fifo_types.h"

//Asynch FIFO implementaion
//Note these are in the include file to allow the compiler to inline for performance

///////////////////////////////////////
//Shared memory FIFO (sample by sample)
//Can be any size
///////////////////////////////////////


static inline unsigned fifo_get_fill(volatile mem_fifo_t * unsafe fifo) {
    unsafe{
        unsigned fifo_fill = 0;
        if (fifo->write_idx >= fifo->read_idx){
            fifo_fill = fifo->write_idx - fifo->read_idx;
        }
        else{
            fifo_fill = (fifo->size + fifo->write_idx) - fifo->read_idx;
        }
        return fifo_fill;
    }
}

#pragma unsafe arrays
static inline fifo_ret_t fifo_block_push(volatile mem_fifo_t * unsafe fifo, int data[], unsigned n) {
    unsafe{
        //check there is a block of space large enough
        unsigned space_remaining = fifo->size - fifo_get_fill(fifo) - 1;
        if (n > space_remaining) {
            return FIFO_FULL;
        }
        for (int i = 0; i < n; i++){
            unsigned next_idx = fifo->write_idx + 1;
            if (next_idx == fifo->size) next_idx = 0; //Check for wrap
            fifo->data_base_ptr[fifo->write_idx] = data[i];
            fifo->write_idx = next_idx;
        }
        return FIFO_SUCCESS;
    }
}

#pragma unsafe arrays
static inline fifo_ret_t fifo_block_push_short_pairs(volatile mem_fifo_t * unsafe fifo, short data[], unsigned n) {
    unsafe{
        //check there is a block of space large enough
        unsigned space_remaining = fifo->size - fifo_get_fill(fifo) - 1;
        if (n > space_remaining) {
            return FIFO_FULL;
        }
        for (int i = 0; i < n; i++){
            unsigned next_idx = fifo->write_idx + 1;
            if (next_idx == fifo->size) next_idx = 0; //Check for wrap
            fifo->data_base_ptr[fifo->write_idx] = data[i] << 16;
            fifo->write_idx = next_idx;
        }
        return FIFO_SUCCESS;
    }
}

#pragma unsafe arrays
static inline fifo_ret_t fifo_block_pop(volatile mem_fifo_t * unsafe fifo, int data[], unsigned n) {
    unsafe{
        //Check we have a block big enough to send
        if (n > fifo_get_fill(fifo)){
            return FIFO_EMPTY;
        }
        for (int i = 0; i < n; i++){
            data[i] = fifo->data_base_ptr[fifo->read_idx];
            fifo->read_idx++;
            if (fifo->read_idx == fifo->size) fifo->read_idx = 0; //Check for wrap
        }
        return FIFO_SUCCESS;
    }
}

#pragma unsafe arrays
static inline fifo_ret_t fifo_block_pop_short_pairs(volatile mem_fifo_t * unsafe fifo, short data[], unsigned n) {
    unsafe{
        //Check we have a block big enough to send
        if (n > fifo_get_fill(fifo)){
            return FIFO_EMPTY;
        }
        for (int i = 0; i < n; i++){
            data[i] = fifo->data_base_ptr[fifo->read_idx] >> 16;
            fifo->read_idx++;
            if (fifo->read_idx == fifo->size) fifo->read_idx = 0; //Check for wrap
        }
        return FIFO_SUCCESS;
    }
}

//Version of above that returns fill level relative to half full
static inline int fifo_get_fill_relative_half(volatile mem_fifo_t * unsafe fifo){
    unsafe{
        int fifo_fill = (int)fifo_get_fill(fifo);
        fifo_fill -= (fifo->size / 2);
        return fifo_fill;
    }
}

#endif