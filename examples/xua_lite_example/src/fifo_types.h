#ifndef __ASRC_FIFO_TYPES__
#define __ASRC_FIFO_TYPES__
#include <stdint.h>

//Shared FIFO return types
typedef enum fifo_ret_t {
  FIFO_SUCCESS = 0,
  FIFO_FULL,
  FIFO_EMPTY
} fifo_ret_t;

/////////////////////////////////////////////////////////////////////////
//Shared memory FIFO (sample by sample or block)
//Can be any size
//
//Note that the actual storage for the FIFO is declared externally
//and a reference to the base address of the storage is passed in along
//with the size of the storage. This way, multiple instances may be 
//different sizes.
//
/////////////////////////////////////////////////////////////////////////

typedef struct mem_fifo_t {
    const unsigned size;              //Size in INTs
    int * const unsafe data_base_ptr; //Base of the data array - declared externally so we can have differnt sized FIFOs
    unsigned write_idx;
    unsigned read_idx;
} mem_fifo_t;

#endif