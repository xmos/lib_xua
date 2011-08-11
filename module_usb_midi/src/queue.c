#include <stdio.h>
#include "queue.h"

// This presumes that the xc compiler will not re-use the mem passed to init_queue
void init_queue(queue *q, int arr[], int size) {
   q->rdptr = 0;
   q->wrptr = 0;
   q->data = (intptr_t)arr;
   q->size = size; // presume that size is power of two
   q->mask = size - 1;
}

extern inline void enqueue(queue *q, int value) {
   ((int *)q->data)[q->wrptr & q->mask] = value;
   q->wrptr++;
}

extern inline int dequeue(queue *q) {
   int retval = ((int *)q->data)[q->rdptr & q->mask];
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
      printf("a[%d] = %d\n", i & q->mask, ((int *)q->data)[i & q->mask]);
   }
}


