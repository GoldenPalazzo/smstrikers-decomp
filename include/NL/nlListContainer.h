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

    ~ListContainerBase()
    {
        nlWalkList(m_Head, this, DeleteEntryFunc());
        m_Head = NULL;
        m_Tail = NULL;
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

#ifdef CHARACTERTEMPLATE_LISTCONTAINER_PC_EXTERN
// Per-TU (CharacterTemplate.cpp only). PARTIAL specialization for T = char*:
// identical to the generic above except DeleteEntry is DECLARED-not-DEFINED, so
// its address-take via DeleteEntryFunc() (which mints the .data PTMF) emits a UND
// import, resolved by AnimInventory.o's weak definition, which links first
// (0x80007478). It stays a TEMPLATE, so its members instantiate lazily -- the
// PTMF anon still mints at instantiation time and keeps its .data position (a
// full explicit class specialization is a non-template: it parses eagerly and
// mints the PTMF at .data offset 0, shifting the whole section).
// The generic is untouched, so cSHierarchy*/AnimRetargetList* keep instantiating
// implicitly, preserving their natural emission order.
template <typename Adapter>
class ListContainerBase<char*, Adapter>
{
public:
    ListContainerBase()
        : m_Head(NULL)
        , m_Tail(NULL)
    {
    }

    ~ListContainerBase()
    {
        nlWalkList(m_Head, this, DeleteEntryFunc());
        m_Head = NULL;
        m_Tail = NULL;
    }

    // DECLARED ONLY -- defined by the AnimInventory.cpp TU.
    void DeleteEntry(ListEntry<char*>* entry);

    typedef void (ListContainerBase::*ENTRY_FUNC)(ListEntry<char*>*);

    static ENTRY_FUNC DeleteEntryFunc()
    {
        return &ListContainerBase::DeleteEntry;
    }

    void AddEntry(ListEntry<char*>* entry)
    {
        nlListAddStart<ListEntry<char*> >(&m_Head, entry, &m_Tail);
    }

    ListEntry<char*>* Allocate(char* const& data)
    {
        ListEntry<char*> localEntry(data);
        ListEntry<char*>* entry = NULL;
        m_Allocator.Allocate(entry);
        if (entry != NULL)
        {
            *entry = localEntry;
        }
        return entry;
    }

    void AddEntry(char* const& data)
    {
        ListEntry<char*>* entry = new (m_Allocator.Allocate()) ListEntry<char*>(data);
        nlListAddStart<ListEntry<char*> >(&m_Head, entry, &m_Tail);
    }

    void AddStart(char* const& data)
    {
        ListEntry<char*>* entry = m_Allocator.New(ListEntry<char*>(data));
        nlListAddStart<ListEntry<char*> >(&m_Head, entry, &m_Tail);
    }

    void RemoveEntry(ListEntry<char*>* entry)
    {
    }

    /* 0x0 */ Adapter m_Allocator;
    ListEntry<char*>* m_Head;
    ListEntry<char*>* m_Tail;
};
#endif

template <typename T>
class nlListContainer : public ListContainerBase<T, NewAdapter<ListEntry<T> > >
{
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
