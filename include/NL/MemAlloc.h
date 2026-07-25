#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

#include "types.h"
// Load-bearing: nlDLRing.h is not referenced by this header, but it must be
// included HERE, ahead of the class bodies below. Including it from MemAlloc.cpp
// *after* this header instead splits MWCC's weak template block into an extra
// section -- the nlDLRing* helpers stop sharing a section with the nlWalkDLRing
// instantiations -- which reorders the .text contribution and breaks the DOL
// SHA1. objdiff and the per-unit report both still show 100%, so only
// build.sha1 catches it. MemAlloc.cpp is the only TU affected; including
// nlRing.h here instead of nlDLRing.h does NOT work.
#include "NL/nlDLRing.h" // IWYU pragma: keep

struct FreeBlockList
{
    /* 0x0 */ FreeBlockList* m_next;
    /* 0x4 */ FreeBlockList* m_prev;
    /* 0x8 */ u32 m_size;
};

class MemoryAllocator
{
public:
    unsigned int LargestFreeBlock();
    unsigned int TotalFreeMemory();
    void Initialize(void*, unsigned int);
    void* Allocate(unsigned long, unsigned int, bool);
    void Free(void*);

    /* 0x0 */ FreeBlockList* m_free_block_list;
};

extern MemoryAllocator StandardAllocator;
extern MemoryAllocator VirtualAllocator;

template <typename T, typename CallbackType>
void nlWalkDLRing(T* head, CallbackType* callback, void (CallbackType::*callbackFunc)(T*));

#endif // _MEMALLOC_H_
