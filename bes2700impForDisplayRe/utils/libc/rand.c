#include "stdlib.h"
#include "plat_types.h"

static unsigned int seed = 1;

WEAK
void srand(unsigned int init)
{
    seed = init;
}

WEAK
int rand()
{
    //Based on Knuth "The Art of Computer Programming"
    seed = seed * 1103515245 + 12345;
    return ( (unsigned int) (seed / 65536) % (RAND_MAX+1) );
}
