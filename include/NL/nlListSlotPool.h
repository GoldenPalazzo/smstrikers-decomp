#ifndef _NLLISTSLOTPOOL_H_
#define _NLLISTSLOTPOOL_H_

#include "NL/nlList.h"
#include "NL/nlSlotPool.h"

template <typename T>
class ListContainerBase<T, BasicSlotPool<ListEntry<T> > >
{
public:
    ListContainerBase()
        : m_Head(NULL)
        , m_Tail(NULL)
    {
    }

    static void DestroyAllEntries(ListContainerBase* container)
    {
        void (ListContainerBase::*func)(ListEntry<T>*) = &ListContainerBase::DeleteEntry;
        nlWalkList(container->m_Head, container, func);
        container->m_Head = NULL;
        container->m_Tail = NULL;
    }

    ~ListContainerBase()
    {
        DestroyAllEntries(this);
    }

    void DeleteEntry(ListEntry<T>* entry)
    {
        m_Allocator.DeleteEntry(entry);
    }

    void AddEntry(ListEntry<T>* entry)
    {
    }

    void RemoveEntry(ListEntry<T>* entry)
    {
    }

    /* 0x0 */ BasicSlotPool<ListEntry<T> > m_Allocator;
    ListEntry<T>* m_Head;
    ListEntry<T>* m_Tail;
};

/**
 * Offset/Address/Size: 0xE0 | 0x8014B590 | size: 0xCC
 */
template <typename T>
class nlListSlotPool : public ListContainerBase<T, BasicSlotPool<ListEntry<T> > >
{
public:
    nlListSlotPool()
        : ListContainerBase<T, BasicSlotPool<ListEntry<T> > >()
    {
    }

    nlListSlotPool(int initial, int delta)
        : ListContainerBase<T, BasicSlotPool<ListEntry<T> > >()
    {
        this->m_Allocator.m_Initial = initial;
        SlotPoolBase::BaseAddNewBlock(&this->m_Allocator, sizeof(ListEntry<T>));
        this->m_Allocator.m_Delta = delta;
    }

    ~nlListSlotPool()
    {
    }
};

#endif // _NLLISTSLOTPOOL_H_
