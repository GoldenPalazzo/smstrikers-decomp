#ifndef _NLLISTSLOTPOOLHIGH_H_
#define _NLLISTSLOTPOOLHIGH_H_

#include "NL/nlList.h"
#include "NL/nlSlotPoolHigh.h"

template <typename T>
class nlListSlotPoolHigh : public ListContainerBase<T, BasicSlotPoolHigh<ListEntry<T> > >
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
        SlotPoolBase::BaseAddNewBlock(&this->m_Allocator, sizeof(ListEntry<T>));
        this->m_Allocator.m_Delta = delta;
    }
};

#endif // _NLLISTSLOTPOOLHIGH_H_
