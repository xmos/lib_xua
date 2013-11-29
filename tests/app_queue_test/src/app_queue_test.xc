#include "queue.h"
#include <stdlib.h>

#define VERIFY(x) do { if (!(x)) _Exit(1); } while(0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

int main()
{
    queue_t q;
    unsigned array[4];
    const unsigned size = ARRAY_SIZE(array);
    queue_init(q, size);
    VERIFY(queue_is_empty(q));
    VERIFY(!queue_is_full(q));
    VERIFY(queue_items(q) == 0);
    VERIFY(queue_space(q) == size);
    queue_push_word(q, array, 1);
    VERIFY(!queue_is_empty(q));
    VERIFY(queue_items(q) == 1);
    VERIFY(queue_space(q) == size - 1);
    for (unsigned i = 1; i < size; i++) {
        queue_push_word(q, array, i + 1);
    }
    VERIFY(queue_is_full(q));
    VERIFY(queue_items(q) == size);
    VERIFY(queue_space(q) == 0);
    for (unsigned i = 0; i < size; i++) {
        VERIFY(queue_pop_word(q, array) == i + 1);
    }
    return 0;
}
