#ifndef _NLLISTCONTAINER_H_
#define _NLLISTCONTAINER_H_

// Home header for ListContainerBase and nlListContainer. The original compiler
// grouped weak instantiations into
// linkonce sections keyed by the BODY's file; DWARF attributes
// ListContainerBase::DeleteEntry and ~ListContainerBase to
// core/nlListContainer.h, so their bodies live here rather than nlList.h for
// a TU's .text section grouping to match the target.
//
// Include via NL/nlList.h (which includes this at its end) - ListEntry and
// nlWalkList must be visible first.

template <typename T, typename Adapter>
class ListContainerBase
{
public:
    ListContainerBase()
        : m_Head(NULL)
        , m_Tail(NULL)
    {
    }

    typedef void (ListContainerBase::*ENTRY_FUNC)(ListEntry<T>*);

    ~ListContainerBase()
    {
        Clear();
    }

    void Clear()
    {
        ENTRY_FUNC func = &ListContainerBase::DeleteEntry;
        nlWalkList(m_Head, this, func);
        m_Head = NULL;
        m_Tail = NULL;
    }

    void DeleteEntry(ListEntry<T>* entry);

    // Add more list operations as needed
    void AddEntry(ListEntry<T>* entry)
    {
        nlListAddStart<ListEntry<T> >(&m_Head, entry, &m_Tail);
    }

    ListEntry<T>* Allocate(const T& data)
    {
        ListEntry<T> localEntry(data);
        ListEntry<T>* entry = NULL;
        m_Allocator.Allocate(entry);
        if (entry != NULL)
        {
            *entry = localEntry;
        }
        return entry;
    }

    void AddEntry(const T& data)
    {
        ListEntry<T>* entry = new (m_Allocator.Allocate()) ListEntry<T>(data);
        nlListAddStart<ListEntry<T> >(&m_Head, entry, &m_Tail);
    }

    void AddStart(const T& data)
    {
        ListEntry<T>* entry = m_Allocator.New(ListEntry<T>(data));
        nlListAddStart<ListEntry<T> >(&m_Head, entry, &m_Tail);
    }

    ListEntry<T>* RemoveStart()
    {
        return nlListRemoveStart<ListEntry<T> >(&m_Head, &m_Tail);
    }

    void RemoveEntry(ListEntry<T>* entry)
    {
        // Implementation for removing entries
    }

    // offsets and sizes are dependent on the adapter
    /* 0x0 */ Adapter m_Allocator;
    ListEntry<T>* m_Head;
    ListEntry<T>* m_Tail;
};

template <typename T, typename Adapter>
void ListContainerBase<T, Adapter>::DeleteEntry(ListEntry<T>* entry)
{
    m_Allocator.DeleteEntry(entry);
}

template <typename T>
class nlListContainer : public ListContainerBase<T, NewAdapter<ListEntry<T> > >
{
}; // total size: 0xC

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
};

template <typename T>
class nlListSlotPoolHigh
    : public ListContainerBase<T, BasicSlotPoolHigh<ListEntry<T> > >
{
public:
    nlListSlotPoolHigh()
        : ListContainerBase<T, BasicSlotPoolHigh<ListEntry<T> > >()
    {
    }

    nlListSlotPoolHigh(int initial, int delta)
        : ListContainerBase<T, BasicSlotPoolHigh<ListEntry<T> > >()
    {
        this->m_Allocator.m_Initial = initial;
        SlotPoolBase::BaseAddNewBlock(
            &this->m_Allocator, sizeof(ListEntry<T>));
        this->m_Allocator.m_Delta = delta;
    }
};

#endif // _NLLISTCONTAINER_H_
