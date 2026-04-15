#ifndef _NLSLOTPOOLHIGH_H_
#define _NLSLOTPOOLHIGH_H_

#include "NL/nlMemory.h"
#include "NL/nlSlotPool.h"

template <typename T>
class BasicSlotPoolHigh : public SlotPoolBase
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
        : SlotPoolBase()
    {
        m_AllocFn = allocFN;
        m_FreeFn = freeFN;
    }

    void DeleteEntry(T* entry)
    {
        SlotPoolEntry* e = (SlotPoolEntry*)entry;
        e->m_next = m_FreeList;
        m_FreeList = e;
    }
}; // total size: 0x18

#endif // _NLSLOTPOOLHIGH_H_
