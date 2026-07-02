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

    typedef void (ListContainerBase::*ENTRY_FUNC)(ListEntry<T>*);

    // Single mint site for the DeleteEntry PTMF const (.data): every caller
    // (dtor, DestroyAllEntries, WalkDeleteEntries) materializes the anon
    // const here, so MWCC pools one copy per TU.
    static ENTRY_FUNC DeleteEntryFunc()
    {
        return &ListContainerBase::DeleteEntry;
    }

    // Head is a separate arg so a caller can evaluate it straight off a
    // global (keeps the container address in one register lifetime at the
    // call site).
    static void WalkDeleteEntries(ListEntry<T>* head, ListContainerBase* container)
    {
        ENTRY_FUNC func = DeleteEntryFunc();
        nlWalkList(head, container, func);
    }

    static void DestroyAllEntries(ListContainerBase* container)
    {
        ENTRY_FUNC func = DeleteEntryFunc();
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
