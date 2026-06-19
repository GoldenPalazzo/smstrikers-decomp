#ifndef _NLDLLISTCONTAINER_H_
#define _NLDLLISTCONTAINER_H_

#include "NL/nlDLRing.h"
#include "NL/nlAdapter.h"
#include "NL/nlArrayAllocator.h"

template <typename T, typename Adapter>
class DLListContainerBase
{
public:
    DLListContainerBase()
    {
    }

    DLListContainerBase(int)
        : m_Head(NULL)
    {
    }

    DLListContainerBase(const int initial, const int delta)
        : m_Allocator(initial, delta)
    {
    }

    static void DestroyAllEntries(DLListContainerBase* container)
    {
        void (DLListContainerBase::*func)(DLListEntry<T>*) = &DLListContainerBase::DeleteEntry;
        nlWalkDLRing<DLListEntry<T>, DLListContainerBase>(container->m_Head, container, func);
        container->m_Head = NULL;
    }

    ~DLListContainerBase()
    {
        DestroyAllEntries(this);
    }

    DLListEntry<T>* Allocate(const T& data)
    {
        T localData = data;
        DLListEntry<T>* entry = m_Allocator.m_pFree;
        if (entry != NULL)
        {
            m_Allocator.m_pFree = entry->m_next;
        }
        if (entry != NULL)
        {
            entry->m_next = NULL;
            entry->m_prev = NULL;
            entry->m_data = localData;
        }
        return entry;
    }

    void Deallocate(DLListEntry<T>* entry, T* outData)
    {
        if (outData)
        {
            *outData = entry->m_data;
        }
        entry->m_next = m_Allocator.m_pFree;
        m_Allocator.m_pFree = entry;
    }

    T* AllocateAtEnd(unsigned long* outEntry);

    void DeleteEntry(DLListEntry<T>* entry);

    /* 0x0 */ Adapter m_Allocator;     // offset 0x0, size 0x18
    /* 0x18 */ DLListEntry<T>* m_Head; // offset 0x18, size 0x4
}; // total size: 0x1C

template <typename T, typename Adapter>
T* DLListContainerBase<T, Adapter>::AllocateAtEnd(unsigned long* outEntry)
{
    // TODO: 98.97% match - residual is a pure r27/r28 callee-saved register swap
    // between the allocated entry pointer and the cleanup-tracking pointer.
    DLListEntry<T>* result;
    {
        T data;
        DLListEntry<T> localEntry(data);
        result = NULL;

        m_Allocator.Allocate(result);
        result = new (result) DLListEntry<T>(localEntry);
    }

    nlDLRingAddEnd(&m_Head, result);

    if (outEntry != NULL)
    {
        *outEntry = (unsigned long)result;
    }

    return &result->m_data;
}

template <typename T, typename Adapter>
void DLListContainerBase<T, Adapter>::DeleteEntry(DLListEntry<T>* entry)
{
    if (entry)
    {
        entry->m_data.~T();
    }
    m_Allocator.DeleteEntry(entry);
}

template <typename T>
class nlDLListContainer : public DLListContainerBase<T, NewAdapter<DLListEntry<T> > >
{
public:
}; // total size: 0x8

#endif // _NLDLLISTCONTAINER_H_
