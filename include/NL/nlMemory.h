#ifndef _NLMEMORY_H_
#define _NLMEMORY_H_

#include "dolphin/os.h"
#include <stddef.h>

inline void* operator new(unsigned long, void* p)
{
    return p;
}

inline void* operator new[](unsigned long, void* p)
{
    return p;
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
void* nlMalloc(unsigned long);
void* operator new(unsigned long);

inline void* operator new(unsigned long size, unsigned int alignment, bool atEnd)
{
    return nlMalloc(size, alignment, atEnd);
}
inline void* operator new[](unsigned long size, unsigned int alignment, bool atEnd)
{
    return nlMalloc(size, alignment, atEnd);
}
void operator delete[](void* ptr);
void operator delete(void* ptr);
unsigned int nlVirtualTotalFree();
unsigned int nlVirtualLargestBlock();
void nlVirtualFree(void*);
void* nlVirtualAlloc(unsigned long, bool);
void nlInitMemory();

#ifdef __cplusplus
extern "C"
{
#endif

// TODO: a bit messy include structure
#ifndef memcpy
    void* memcpy(void* dest, const void* src, size_t num);
#endif
#ifndef memmove
    void* memmove(void* dest, const void* src, size_t num);
#endif
#ifndef memset
    void* memset(void* dest, int ch, size_t count);
#endif
#ifndef memcmp
    int memcmp(const void* ptr1, const void* ptr2, size_t num);
#endif

    class DefaultAllocator
    {
    public:
        static void* allocate(size_t size)
        {
            return nlMalloc(size, 8, false); // 8-byte alignment, not at end
        }

        static void deallocate(void* ptr)
        {
            nlFree(ptr);
        }
    };

#ifdef __cplusplus
}
#endif

#endif // _NLMEMORY_H_
