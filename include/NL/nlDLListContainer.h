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
        : m_Head(NULL)
    {
    }
    DLListContainerBase(const int initial, const int delta)
        : m_Allocator(initial, delta)
        , m_Head(NULL)
    {
    }

    ~DLListContainerBase()
    {
        typedef void (*WalkFn)(DLListEntry<T>*, DLListContainerBase*, void (DLListContainerBase::*)(DLListEntry<T>*));
        void (DLListContainerBase::*func)(DLListEntry<T>*) = &DLListContainerBase::DeleteEntry;
        WalkFn walk = &nlWalkDLRing<DLListEntry<T>, DLListContainerBase>;
        walk(m_Head, this, func);
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
            entry->m_data = localData;
        }
        return entry;
    }

    void DeleteEntry(DLListEntry<T>* entry);

    /* 0x0 */ Adapter m_Allocator;     // offset 0x0, size 0x18
    /* 0x18 */ DLListEntry<T>* m_Head; // offset 0x18, size 0x4
}; // total size: 0x1C

template <typename T, typename Adapter>
void DLListContainerBase<T, Adapter>::DeleteEntry(DLListEntry<T>* entry)
{
    m_Allocator.DeleteEntry(entry);
}

template <typename T>
class nlDLListContainer : public DLListContainerBase<T, NewAdapter<DLListEntry<T> > >
{
public:
}; // total size: 0x8

#endif // _NLDLLISTCONTAINER_H_
