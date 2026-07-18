#ifndef _NLDLLISTCONTAINER_H_
#define _NLDLLISTCONTAINER_H_

#include "NL/nlDLRing.h"
#include "NL/NewAdapter.h"
#include "NL/nlArrayAllocator.h"
#include "NL/nlSlotPool.h"

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

    ~DLListContainerBase()
    {
        Clear();
    }

    void Clear()
    {
        void (DLListContainerBase::*func)(DLListEntry<T>*) = &DLListContainerBase::DeleteEntry;
        nlWalkDLRing<DLListEntry<T>, DLListContainerBase>(m_Head, this, func);
        m_Head = NULL;
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
            entry->entry = localData;
        }
        return entry;
    }

    void Deallocate(DLListEntry<T>* entry, T* outData)
    {
        if (outData)
        {
            *outData = entry->entry;
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
    DLListEntry<T>* result;
    {
        T data;
        DLListEntry<T> localEntry(data);
        result = m_Allocator.Allocate();
        result = new (result) DLListEntry<T>(localEntry);
    }

    nlDLRingAddEnd(&m_Head, result);

    if (outEntry != NULL)
    {
        *outEntry = (unsigned long)result;
    }

    return &result->entry;
}

template <typename T>
class nlDLListContainer : public DLListContainerBase<T, NewAdapter<DLListEntry<T> > >
{
public:
}; // total size: 0x8

template <typename T>
class nlDLListSlotPool : public DLListContainerBase<T, BasicSlotPool<DLListEntry<T> > >
{
public:
    nlDLListSlotPool()
        : DLListContainerBase<T, BasicSlotPool<DLListEntry<T> > >(0)
    {
        this->m_Allocator.m_Initial = 16;
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&this->m_Allocator, sizeof(DLListEntry<T>));
        this->m_Allocator.m_Delta = 16;
    }

    nlDLListSlotPool(const int initial)
        : DLListContainerBase<T, BasicSlotPool<DLListEntry<T> > >(0)
    {
        this->m_Allocator.m_Initial = initial;
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&this->m_Allocator, sizeof(DLListEntry<T>));
        this->m_Allocator.m_Delta = 0;
    }

    nlDLListSlotPool(const int initial, const int delta)
        : DLListContainerBase<T, BasicSlotPool<DLListEntry<T> > >(0)
    {
        this->m_Allocator.m_Initial = initial;
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&this->m_Allocator, sizeof(DLListEntry<T>));
        this->m_Allocator.m_Delta = delta;
    }

}; // total size: 0x1C

template <typename T, typename Adapter>
void DLListContainerBase<T, Adapter>::DeleteEntry(DLListEntry<T>* entry)
{
    if (entry)
    {
        entry->entry.~T();
    }
    m_Allocator.DeleteEntry(entry);
}

#endif // _NLDLLISTCONTAINER_H_
