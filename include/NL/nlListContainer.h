#ifndef _NLLISTCONTAINER_H_
#define _NLLISTCONTAINER_H_

// Home header for ListContainerBase (generic + BasicSlotPool specialization)
// and nlListContainer. The original compiler grouped weak instantiations into
// linkonce sections keyed by the BODY's file; DWARF attributes
// ListContainerBase::DeleteEntry and ~ListContainerBase to
// core/nlListContainer.h, so their bodies live here (NOT in nlList.h /
// nlListSlotPool.h) for a TU's .text section grouping to match the target.
//
// Include via NL/nlList.h (which includes this at its end) - ListEntry and
// nlWalkList must be visible first.

template <typename T>
class BasicSlotPool;

template <typename T, typename Adapter>
class ListContainerBase
{
public:
    ListContainerBase()
        : m_Head(NULL)
        , m_Tail(NULL)
    {
    }

    void DeleteEntry(ListEntry<T>* entry);

    typedef void (ListContainerBase::*ENTRY_FUNC)(ListEntry<T>*);

    // Single mint site for the DeleteEntry PTMF const (.data) -- same trick
    // as the BasicSlotPool specialization below: every caller materializes
    // the anon const here, so MWCC pools one copy per TU.
    static ENTRY_FUNC DeleteEntryFunc()
    {
        return &ListContainerBase::DeleteEntry;
    }

    // Add more list operations as needed
    void AddEntry(ListEntry<T>* entry)
    {
        nlListAddStart<ListEntry<T> >(&m_Head, entry, &m_Tail);
    }

    void AddEntry(const T& data)
    {
        ListEntry<T>* entry = new (m_Allocator.Allocate()) ListEntry<T>(data);
        nlListAddStart<ListEntry<T> >(&m_Head, entry, &m_Tail);
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
public:
    ~nlListContainer()
    {
        if (this != NULL)
        {
            nlWalkList(this->m_Head, static_cast<ListContainerBase<T, NewAdapter<ListEntry<T> > >*>(this), ListContainerBase<T, NewAdapter<ListEntry<T> > >::DeleteEntryFunc());
            this->m_Head = NULL;
            this->m_Tail = NULL;
        }
    }
}; // total size: 0xC

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

    // Dtor DECLARED here but DEFINED at the end of NL/nlSlotPool.h: its weak
    // __dt__90 emission is a phantom (not in the target DOL) and must bucket
    // with the equally-phantom __dt__48BasicSlotPool (same body file) so the
    // linker can drop the whole section, instead of fusing into DeleteEntry's
    // kept section here.
    ~ListContainerBase();

    void DeleteEntry(ListEntry<T>* entry)
    {
        m_Allocator.DeleteEntry(entry);
    }

    void AddEntry(ListEntry<T>* entry)
    {
        nlListAddStart<ListEntry<T> >(&m_Head, entry, &m_Tail);
    }

    void RemoveEntry(ListEntry<T>* entry)
    {
    }

    /* 0x0 */ BasicSlotPool<ListEntry<T> > m_Allocator;
    ListEntry<T>* m_Head;
    ListEntry<T>* m_Tail;
};

#endif // _NLLISTCONTAINER_H_
