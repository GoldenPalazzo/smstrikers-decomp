#ifndef _NLSLOTPOOLHIGH_H_
#define _NLSLOTPOOLHIGH_H_

#include "NL/nlMemory.h"
#include "NL/nlSlotPool.h"

template <typename T>
class BasicSlotPoolHigh : public BasicSlotPool<T>
{
public:
    static void* allocFN(unsigned long size)
    {
        return nlMalloc(size, 8, true);
    }

    static void freeFN(void* ptr)
    {
        nlFree(ptr);
    }

    BasicSlotPoolHigh()
        : BasicSlotPool<T>()
    {
        this->m_AllocFn = allocFN;
        this->m_FreeFn = freeFN;
    }

    void DeleteEntry(T* entry)
    {
        SlotPoolEntry* e = (SlotPoolEntry*)entry;
        e->m_next = this->m_FreeList;
        this->m_FreeList = e;
    }
}; // total size: 0x18

#endif // _NLSLOTPOOLHIGH_H_
