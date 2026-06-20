#ifndef GBA_FUNCTIONS_H
#define GBA_FUNCTIONS_H

#include <string.h>
#include "doomtype.h"
#include "m_fixed.h"


//***********************************************************************
//The following math functions were taken from the Jaguar Port of Doom
//here: https://github.com/Arc0re/jaguardoom/blob/master/jagonly.c
//
//There may be a licence incompatibility with the iD release
//and the GPL that prBoom (and this as derived work) is under.
//***********************************************************************

static CONSTFUNC unsigned UDiv32 (unsigned aa, unsigned bb)
{
    if(bb == 0)
        return UINT_MAX;

    return aa / bb;
}

inline static CONSTFUNC int IDiv32 (int a, int b)
{
    return a / b;
}

inline static void BlockCopy(void* dest, const void* src, const unsigned int len)
{
    memcpy(dest, src, len & 0xfffffffc);
}

inline static void CpuBlockCopy(void* dest, const void* src, const unsigned int len)
{
    BlockCopy(dest, src, len);
}

inline static void BlockSet(void* dest, volatile unsigned int val, const unsigned int len)
{
    memset(dest, val, len & 0xfffffffc);
}

inline static void ByteCopy(byte* dest, const byte* src, unsigned int count)
{
    do
    {
        *dest++ = *src++;
    } while(--count);
}

inline static void ByteSet(byte* dest, byte val, unsigned int count)
{
    do
    {
        *dest++ = val;
    } while(--count);
}

inline static void* ByteFind(byte* mem, byte val, unsigned int count)
{
    do
    {
        if(*mem == val)
            return mem;

        mem++;
    } while(--count);

    return NULL;
}

inline static void SaveSRAM(const byte* eeprom, unsigned int size, unsigned int offset)
{
}

inline static void LoadSRAM(byte* eeprom, unsigned int size, unsigned int offset)
{
}


#endif // GBA_FUNCTIONS_H
