#ifndef _NLLISTSLOTPOOL_H_
#define _NLLISTSLOTPOOL_H_

#include "NL/nlList.h"
#include "NL/nlSlotPool.h"

// The ListContainerBase<T, BasicSlotPool<ListEntry<T> > > specialization
// lives in NL/nlListContainer.h (its original home header - linkonce
// grouping is keyed by the body's file). Only the derived nlListSlotPool
// class belongs here, so ~nlListSlotPool buckets separately from
// ListContainerBase::DeleteEntry, as in the target.

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

    ~nlListSlotPool();
};

// Dtor body in its own header: `template class nlListSlotPool<T>;` emits all
// members; the phantom inline ctors bucket to THIS file's section (droppable
// at link) while the kept dtor buckets to nlListSlotPoolDtor.h.
#include "NL/nlListSlotPoolDtor.h"

// Phantom trigger helper (never called; instantiated only by an explicit
// directive in AudioScriptEventMgr.cpp): its body holds that TU's only
// out-of-line reference to ~nlListSlotPool, forcing the weak dtor to emit
// BEFORE __sinit instead of in the post-__sinit instantiation wave. Housed
// in THIS header (not nlListSlotPoolDtor.h) so the helper's own linkonce
// section stays isolated from the dtor's and is dropped at link.
template <typename T>
void nlListSlotPoolReap(nlListSlotPool<T>* p)
{
    FORCE_DONT_INLINE;
    p->~nlListSlotPool();
}

#endif // _NLLISTSLOTPOOL_H_
