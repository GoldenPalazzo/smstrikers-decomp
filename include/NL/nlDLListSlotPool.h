#ifndef _NLDLLISTSLOTPOOL_H_
#define _NLDLLISTSLOTPOOL_H_

#include "NL/nlDLListContainer.h"
#include "NL/nlSlotPool.h"

template <typename T>
class nlDLListSlotPool : public DLListContainerBase<T, BasicSlotPool<DLListEntry<T> > >
{
public:
    nlDLListSlotPool()
    {
        this->m_Head = NULL;
        this->m_Allocator.m_Initial = 16;
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&this->m_Allocator, sizeof(DLListEntry<T>));
        this->m_Allocator.m_Delta = 16;
    }

    nlDLListSlotPool(const int initial, const int delta)
    {
        this->m_Head = NULL;
        this->m_Allocator.m_Initial = initial;
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&this->m_Allocator, sizeof(DLListEntry<T>));
        this->m_Allocator.m_Delta = delta;
    }

    ~nlDLListSlotPool()
    {
    }
}; // total size: 0x1C

#endif // _NLDLLISTSLOTPOOL_H_
