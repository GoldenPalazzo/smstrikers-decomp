#ifndef _NLMEMORY_H_
#define _NLMEMORY_H_

#include "dolphin/os.h"
#include <stddef.h>

inline unsigned long KB(unsigned long size)
{
    return size << 10;
}

inline unsigned long MB(unsigned long size)
{
    return KB(KB(size));
}

/*
this code snippets are from Cuyler / discord commuity - no yet integrating it, as I would need to refactors different
files to use it instead of the current new/malloc implementation.

void* operator new(size_t size) {
    return nlMalloc(size, 8, false);
}

void* operator new(size_t size, size_t alignment) {
    return nlMalloc(size, alignment, false);
}
*/

void nlFree(void* ptr);
void* nlMalloc(unsigned long size, unsigned int alignment, bool atEnd);
void* nlMalloc(unsigned long size);
void* operator new(unsigned long size);

inline void* operator new(unsigned long size, unsigned int alignment, bool atEnd)
{
    return nlMalloc(size, alignment, atEnd);
}
inline void* operator new[](unsigned long size, unsigned int alignment, bool atEnd)
{
    return nlMalloc(size, alignment, atEnd);
}
inline void* operator new[](unsigned long size, unsigned int alignment, bool atEnd, const char*)
{
    return nlMalloc(size, alignment, atEnd);
}
unsigned int nlVirtualTotalFree();
unsigned int nlVirtualLargestBlock();
void nlVirtualFree(void* ptr);
void* nlVirtualAlloc(unsigned long size, bool bZero);
void nlInitMemory();

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif // _NLMEMORY_H_
