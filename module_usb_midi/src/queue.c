#include <stdio.h>
#include "queue.h"

// Queue implementation
//  Offers no protection against adding when full or dequeueing when empty.
//  Uses read and write counts for pointers to distinguish full and empty cases.
//  Works from c and xc
//  Must allocate the memory outside of this and pass it in to init_queue so can statically allocate
//  Must work for different element sizes

// This presumes that the xc compiler will not re-use the mem passed to init_queue
void init_queue(queue *q, unsigned char arr[], int size, int element_size) {
   q->rdptr = 0;
   q->wrptr = 0;
   q->data = (uintptr_t)arr;
   q->size = size; // in items, presume that size is power of two
   q->element_size = element_size; // The size of each element in bytes
   q->mask = size - 1;
}

extern inline void enqueue(queue *q, unsigned value) {
   switch (q->element_size) {
      case 4:
         ((unsigned *)q->data)[q->wrptr & q->mask] = value;
         break;
      case 1:
         ((unsigned char *)q->data)[q->wrptr & q->mask] = (unsigned char)value;
         break;
      default:
         break;
   }
   q->wrptr++;
}

extern inline unsigned dequeue(queue *q) {
   unsigned retval;
   switch (q->element_size) {
      case 4:
         retval = ((unsigned *)q->data)[q->rdptr & q->mask];
         break;
      case 1:
         retval = ((unsigned char *)q->data)[q->rdptr & q->mask];
         break;
      default:
         break;
   }
   q->rdptr++;
   return retval;
}

extern inline int isempty(queue *q) {
   return (q->rdptr == q->wrptr);
}

extern inline int isfull(queue *q) {
   return ((q->wrptr - q->rdptr) == q->size);
}

extern inline int items(queue *q) {
   int items = q->wrptr - q->rdptr;
   return items;
}

// How to calculate size? Could make it a function call or leave it as a variable within the struct
extern inline int space(queue *q) {
   return q->size - items(q);
}

void dump(queue *q) {
   for (int i = q->rdptr; i != q->wrptr; i++) {
      switch (q->element_size) {
         case 4:
            printf("a[%d] = %d\n", i & q->mask, ((unsigned *)q->data)[i & q->mask]);
            break;
         case 1:
            printf("a[%d] = %d\n", i & q->mask, ((unsigned char *)q->data)[i & q->mask]);
            break;
         default:
            break;
      }
   }
}


