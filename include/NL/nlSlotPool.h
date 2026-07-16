#ifndef _NLSLOTPOOL_H_
#define _NLSLOTPOOL_H_

#include "types.h"
#include "NL/nlMemory.h"
#include "NL/nlList.h"

typedef void* (*SlotPoolAllocatorFunc)(unsigned long size);
typedef void (*SlotPoolFreeFunc)(void* data);

// Forward declarations
struct SlotPoolBlock;
struct SlotPoolEntry;

class SlotPoolBase
{
public:
    SlotPoolBase();
    ~SlotPoolBase();

    static void BaseAddNewBlock(SlotPoolBase* pool, unsigned int entrySize);
    static void BaseFreeBlocks(SlotPoolBase* pool, unsigned int entrySize);

    /* 0x00 */ u32 m_Initial;              // Secondary slot count
    /* 0x04 */ u32 m_Delta;                // Primary slot count
    /* 0x08 */ SlotPoolBlock* m_BlockList; // Head of block list
    /* 0x0C */ SlotPoolEntry* m_FreeList;  // Head of entry list
    /* 0x10 */ SlotPoolAllocatorFunc m_AllocFn;
    /* 0x14 */ SlotPoolFreeFunc m_FreeFn;
}; // total size: 0x18

struct SlotPoolBlock
{
    SlotPoolBlock* m_next; // Pointer for linked list functionality
};

struct SlotPoolEntry
{
    SlotPoolEntry* m_next; // Pointer for linked list functionality
};

static void DefaultSlotPoolFree(void*);
static void* DefaultSlotPoolAllocator(unsigned long);

template <typename T>
class BasicSlotPool : public SlotPoolBase
{
public:
    BasicSlotPool()
        : SlotPoolBase()
    {
    }

    BasicSlotPool(const int initial, const int delta)
        : SlotPoolBase()
    {
        this->m_Initial = initial;
        BaseAddNewBlock(this, sizeof(T));
        this->m_Delta = delta;
    }

    ~BasicSlotPool()
    {
        BaseFreeBlocks(this, sizeof(T));
    };

    // Allocate an entry from the free list
    void Allocate(T*& out)
    {
        if (m_FreeList == NULL)
        {
            BaseAddNewBlock(this, sizeof(T));
        }
        if (m_FreeList != NULL)
        {
            out = (T*)m_FreeList;
            m_FreeList = m_FreeList->m_next;
        }
    }

    void AllocateForReturn(T*& out)
    {
        T* entry = NULL;
        if (m_FreeList == NULL)
        {
            BaseAddNewBlock(this, sizeof(T));
        }
        if (m_FreeList != NULL)
        {
            entry = (T*)m_FreeList;
            m_FreeList = m_FreeList->m_next;
        }
        out = entry;
    }

    T* Allocate()
    {
        T* out = NULL;
        AllocateForReturn(out);
        return out;
    }

    // Return an entry back to the free list
    void Free(T* entry)
    {
        SlotPoolEntry* e = (SlotPoolEntry*)entry;
        e->m_next = m_FreeList;
        m_FreeList = e;
    }

    void DeleteEntry(T* entry)
    {
        Free(entry);
    }

    void Delete(T* entry)
    {
        Free(entry);
    }
}; // total size: 0x18

template <typename T>
class SlotPool : public BasicSlotPool<T>
{
public:
    SlotPool()
        : BasicSlotPool<T>()
    {
    }

    SlotPool(int initial, int delta)
    {
        this->m_Initial = initial;
        SlotPoolBase::BaseAddNewBlock(this, sizeof(T));
        this->m_Delta = delta;
    }

#ifdef CHARACTERTEMPLATE_SLOTPOOL_DTOR
    ~SlotPool() {}
#endif
}; // total size: 0x18

#endif // _NLSLOTPOOL_H_
