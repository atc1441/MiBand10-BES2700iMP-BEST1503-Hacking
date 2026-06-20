#include "assert.h"

void __assert(const char* fmt)
{
    // Avoid CPU instruction fetch blocking the system bus on some chips
    do { volatile int i = 0; i++; } while (1);
}

void abort(void)
{
    // Avoid CPU instruction fetch blocking the system bus on some chips
    do { volatile int i = 0; i++; } while (1);
}

