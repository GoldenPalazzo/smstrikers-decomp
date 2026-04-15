#ifndef _NLARRAYALLOCATOR_H_
#define _NLARRAYALLOCATOR_H_

#include "NL/nlDLRing.h"

template <typename T>
class nlArrayAllocator
{
public:
    void DeleteEntry(T* entry);

    /* 0x0 */ T* m_pFree; // size 0x4
}; // total size: 0x4

template <typename T>
void nlArrayAllocator<T>::DeleteEntry(T* entry)
{
    entry->m_next = m_pFree;
    m_pFree = entry;
}

template <typename T, int N>
class nlStaticArrayAllocator : public nlArrayAllocator<T>
{
public:
    /* 0x4 */ unsigned char m_Memory[sizeof(T) * N]; // offset 0x4
}; // total size: 0x4 + sizeof(T) * N

#endif // _NLARRAYALLOCATOR_H_
