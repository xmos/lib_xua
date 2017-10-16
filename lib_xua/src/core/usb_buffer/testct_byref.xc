#include <xs1.h>

/* TODO Currently complier does not support inline select functions, hense this is in a seperate file to ensure this is not the case */
#pragma select handler
void testct_byrefnot(chanend c, unsigned &isCt)
{
    if (testct(c))
    {
        isCt = 1;
    }
    else
    {
        isCt = 0;
    }
}
