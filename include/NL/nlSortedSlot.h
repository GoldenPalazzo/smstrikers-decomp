#ifndef _NLSORTEDSLOT_H_
#define _NLSORTEDSLOT_H_

#include "NL/nlArrayAllocator.h"

template <typename T, int N = 0>
class nlSortedSlot
{
public:
    template <typename U>
    struct EntryLookup
    {
        /* 0x0 */ unsigned long hash;
        /* 0x4 */ U* pEntry;
    }; // total size: 0x8

    virtual T* GetNewEntry() = 0;
    virtual void FreeEntry(T*) = 0;
    virtual void ExpandLookup() = 0;
    virtual void FreeLookup() = 0;

    /* 0x04 */ EntryLookup<T>* m_pEntryLookup;
    /* 0x08 */ unsigned long m_EntryCount;
    /* 0x0C */ unsigned long m_LookupAllocated;
}; // total size: 0x10

template <typename T, int N>
class nlStaticSortedSlot : public nlSortedSlot<T, N>
{
public:
    virtual T* GetNewEntry();
    virtual void FreeEntry(T*);
    virtual void ExpandLookup();
    virtual void FreeLookup();

    /* 0x10 */ unsigned char m_EntryData[sizeof(T) * N];
    /* 0x2C */ typename nlSortedSlot<T, N>::template EntryLookup<T> m_LookupData[N];
    /* 0x64 */ nlArrayAllocator<T> m_ArrayAllocator;
}; // total size: depends on T, N

template <typename T, int N>
void nlStaticSortedSlot<T, N>::ExpandLookup()
{
    this->m_pEntryLookup = m_LookupData;
    this->m_LookupAllocated = N;
}

template <typename T, int N>
void nlStaticSortedSlot<T, N>::FreeLookup()
{
    this->m_LookupAllocated = 0;
}

template <typename T, int N>
void nlStaticSortedSlot<T, N>::FreeEntry(T* entry)
{
    *(T**)entry = m_ArrayAllocator.m_pFree;
    m_ArrayAllocator.m_pFree = entry;
}

template <typename T, int N>
T* nlStaticSortedSlot<T, N>::GetNewEntry()
{
    T* pEntry = m_ArrayAllocator.m_pFree;
    if (pEntry == 0)
    {
        pEntry = 0;
    }
    else
    {
        m_ArrayAllocator.m_pFree = *(T**)pEntry;
    }
    return pEntry;
}

#endif // _NLSORTEDSLOT_H_
