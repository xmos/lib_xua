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

static inline unsigned fifo_get_fill_short(volatile mem_fifo_short_t * unsafe fifo) {
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

static inline void fifo_init_short(volatile mem_fifo_short_t * unsafe fifo) {
    unsafe{
        fifo->write_idx = 0;
        fifo->read_idx = (fifo->size * 2) / 4;
        memset(fifo->data_base_ptr , 0, fifo->size * sizeof(short));
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
static inline fifo_ret_t fifo_block_push_short(volatile mem_fifo_short_t * unsafe fifo, short data[], unsigned n) {
    unsafe{
        //check there is a block of space large enough
        unsigned space_remaining = fifo->size - fifo_get_fill_short(fifo) - 1;
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
static inline fifo_ret_t fifo_block_push_short_fast(volatile mem_fifo_short_t * unsafe fifo, short data[], unsigned n) {
    unsafe{
        //check there is a block of space large enough
        unsigned space_remaining = fifo->size - fifo_get_fill_short(fifo) - 1;
        if (n > space_remaining) {
            return FIFO_FULL;
        }
        //We will write either one or two blocks depending on wrap
        unsigned first_block_size = 0;
        unsigned second_block_size = 0;

        //See if we need to wrap during block writes
        unsigned space_left_at_top = fifo->size - fifo->write_idx;
        //printf("space_left_at_top %d\n", space_left_at_top);
        //Yes, we do need to wrap
        if (n > space_left_at_top){
            first_block_size = space_left_at_top;
            second_block_size = n - space_left_at_top;
            memcpy(&fifo->data_base_ptr[fifo->write_idx], &data[0], first_block_size * sizeof(short));
            memcpy(&fifo->data_base_ptr[0], &data[first_block_size], second_block_size * sizeof(short));
            fifo->write_idx = second_block_size;
        }
        //No wrap, do all in one go
        else{
            first_block_size = n;
            second_block_size = 0;
            memcpy(&fifo->data_base_ptr[fifo->write_idx], &data[0], first_block_size * sizeof(short));
            fifo->write_idx += first_block_size;
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
static inline fifo_ret_t fifo_block_pop_short(volatile mem_fifo_short_t * unsafe fifo, short data[], unsigned n) {
    unsafe{
        //Check we have a block big enough to send
        if (n > fifo_get_fill_short(fifo)){
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
static inline fifo_ret_t fifo_block_pop_short_fast(volatile mem_fifo_short_t * unsafe fifo, short data[], unsigned n) {
    unsafe{
        //Check we have a block big enough to send
        if (n > fifo_get_fill_short(fifo)){
            return FIFO_EMPTY;
        }
        //We will read either one or two blocks depending on wrap
        unsigned first_block_size = 0;
        unsigned second_block_size = 0;

        //See if we need to wrap during block read
        unsigned num_read_at_top = fifo->size - fifo->read_idx;
        // printf("num_read_at_top %d\n", num_read_at_top);
        //Yes, we do need to wrap
        if (n > num_read_at_top){
            first_block_size = num_read_at_top;
            second_block_size = n - num_read_at_top;
            memcpy(&data[0], &fifo->data_base_ptr[fifo->read_idx], first_block_size * sizeof(short));
            memcpy( &data[first_block_size], &fifo->data_base_ptr[0], second_block_size * sizeof(short));
            fifo->read_idx = second_block_size;
            // printf("wrap\n");
        }
        //No wrap, do all in one go
        else{
            first_block_size = n;
            second_block_size = 0;
            memcpy(&data[0], &fifo->data_base_ptr[fifo->read_idx], first_block_size * sizeof(short));
            fifo->read_idx += first_block_size;
            // printf("no wrap\n");

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

//Version of above that returns fill level relative to half full
static inline int fifo_get_fill_relative_half_short(volatile mem_fifo_short_t * unsafe fifo){
    unsafe{
        int fifo_fill = (int)fifo_get_fill_short(fifo);
        fifo_fill -= (fifo->size / 2);
        return fifo_fill;
    }
}
#endif
