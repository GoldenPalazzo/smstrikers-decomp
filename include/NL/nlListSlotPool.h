#ifndef _NLLISTSLOTPOOL_H_
#define _NLLISTSLOTPOOL_H_

#include "NL/nlList.h"
#include "NL/nlSlotPool.h"

/**
 * Offset/Address/Size: 0xE0 | 0x8014B590 | size: 0xCC
 * TODO: 97.94% match - extra beq from two-level implicit member destruction (BasicSlotPool+SlotPoolBase)
 */
template <typename T>
class nlListSlotPool : public ListContainerBase<T, BasicSlotPool<ListEntry<T> > >
{
public:
    nlListSlotPool() : ListContainerBase<T, BasicSlotPool<ListEntry<T> > >() {}

    ~nlListSlotPool()
    {
        if (this != NULL)
        {
            typedef void (ListContainerBase<T, BasicSlotPool<ListEntry<T> > >::*DeleteFn)(ListEntry<T>*);
            DeleteFn cb = &ListContainerBase<T, BasicSlotPool<ListEntry<T> > >::DeleteEntry;
            ListEntry<T>* list = this->m_Head;
            while (list != NULL)
            {
                ListEntry<T>* next = list->next;
                (static_cast<ListContainerBase<T, BasicSlotPool<ListEntry<T> > >*>(this)->*cb)(list);
                list = next;
            }
        }
        this->m_Head = NULL;
        this->m_Tail = NULL;
    }
};

#endif // _NLLISTSLOTPOOL_H_
