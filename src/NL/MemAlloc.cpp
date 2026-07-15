#include "NL/MemAlloc.h"

extern void nlPrintf(const char*, ...);
extern void nlBreak();

// Built with `-inline deferred`: MWCC emits .text in REVERSE source order, so the
// functions below are declared last-to-first relative to their addresses. Parse-time
// .data (the ptmf constants) still follows source order, which is what pins
// TotalFreeMemCallback's ptmf ahead of LargestFreeBlockCallback's.

/**
 * Offset/Address/Size: 0x534 | 0x801CDC80 | size: 0x130
 */
void MemoryAllocator::Free(void* arg0)
{
    FreeBlockList* block;
    MemoryAllocator* self;
    FreeBlockList* start;
    FreeBlockList* iter;
    FreeBlockList* next;
    FreeBlockList* prev;
    s32 size;
    s32 header;
    s32 offset;

    if (arg0 == NULL)
    {
        return;
    }

    block = (FreeBlockList*)((char*)arg0 - 4);
    self = this;
    header = *(u32*)block;
    size = header & 0x3FFFFFFF;
    size += 3;
    size &= 0xFFFFFFFC;
    if (header & 0x40000000)
    {
        size += *(u32*)((char*)arg0 + size);
    }

    size += 4;
    if (header & 0x80000000)
    {
        offset = *(u32*)((char*)block - 4);
        block = (FreeBlockList*)((char*)block - offset);
        size += offset;
    }

    block->m_unk_0x08 = size;
    start = nlDLRingGetStart<FreeBlockList>(self->m_free_block_list);
    if ((start > block) || (start == NULL))
    {
        nlDLRingAddStart<FreeBlockList>(&self->m_free_block_list, block);
    }
    else
    {
        iter = start->m_next;
        while (iter != start)
        {
            if (iter > block)
            {
                break;
            }
            iter = iter->m_next;
        }
        nlDLRingInsert<FreeBlockList>(&self->m_free_block_list, iter->m_prev, block);
    }

    next = block->m_next;
    if (next > block)
    {
        size = block->m_unk_0x08;
        if (((char*)block + size) == (char*)next)
        {
            block->m_unk_0x08 = size + next->m_unk_0x08;
            nlDLRingRemove<FreeBlockList>(&self->m_free_block_list, next);
        }
    }

    prev = block->m_prev;
    if (prev < block)
    {
        size = prev->m_unk_0x08;
        if (((char*)prev + size) == (char*)block)
        {
            prev->m_unk_0x08 = size + block->m_unk_0x08;
            nlDLRingRemove<FreeBlockList>(&self->m_free_block_list, block);
        }
    }
}

/**
 * Offset/Address/Size: 0x1D8 | 0x801CD924 | size: 0x35C
 */
void* MemoryAllocator::Allocate(unsigned long size, unsigned int alignment, bool fromEnd)
{
    void* result;

    if (alignment < 4)
        alignment = 4;
    if (size < 0xC)
        size = 0xC;

    if (fromEnd)
    {
        FreeBlockList* cur;
        FreeBlockList* end = nlDLRingGetEnd<FreeBlockList>(m_free_block_list);
        cur = end;
        u32 alignedSize = (size + 3) & ~3u;
        u32 alignMask = ~(alignment - 1);
        u32 savedSize = size;
        u32 blockSize;
        u32 offset;

        do
        {
            blockSize = cur->m_unk_0x08;
            if (blockSize > alignedSize)
            {
                u32 endAddr = (u32)cur + blockSize;
                offset = (endAddr - alignedSize) & alignMask;
                size = (endAddr - offset) + 4;
                if (size <= blockSize)
                    break;
            }
            cur = cur->m_next;
            if (cur == end)
            {
                nlPrintf("Total Free Memory: %d\n", TotalFreeMemory());
                nlPrintf("Largest Free Block: %d\n", LargestFreeBlock());
                nlBreak();
            }
        } while (true);

        {
            u32 remaining = blockSize - size;
            alignment = 4;
            if (remaining > 0xC)
            {
                cur->m_unk_0x08 = remaining;
            }
            else
            {
                alignment = remaining + 4;
                nlDLRingRemove<FreeBlockList>(&m_free_block_list, cur);
            }

            u32 suffixBase = size - alignedSize;
            u32 header = savedSize;
            void* allocPtr;
            u32 suffixGap;
            suffixGap = suffixBase - 4;
            allocPtr = (char*)(offset - alignment) + alignment;
            if (alignment > 4)
            {
                header = savedSize | 0x80000000;
                *(u32*)((char*)allocPtr - 8) = alignment - 4;
            }
            u8* blockEnd = (u8*)allocPtr + savedSize;
            if (suffixGap != 0)
            {
                header |= 0x40000000;
                *(u32*)(((u32)blockEnd + 3) & ~3u) = suffixGap;
            }
            *(u32*)((char*)allocPtr - 4) = header;
            result = allocPtr;
        }
    }
    else
    {
        u32 savedSize;
        u32 alignMask;
        u32 alignedSize;
        FreeBlockList* cur;
        FreeBlockList* start;
        u32 alignedStart;
        u32 blockSize;
        u32 offset;

        start = nlDLRingGetStart<FreeBlockList>(m_free_block_list);
        cur = start;
        alignedSize = (size + 3) & ~3u;
        alignMask = ~(alignment - 1);
        savedSize = size;

        do
        {
            blockSize = cur->m_unk_0x08;
            if (blockSize > alignedSize)
            {
                alignedStart = (u32)cur + alignment;
                alignedStart = alignMask & (alignedStart + 3);
                size = alignedStart - (u32)cur;
                offset = size + alignedSize;
                if (offset <= blockSize)
                    break;
            }
            cur = cur->m_next;
            if (cur == start)
            {
                nlPrintf("Total Free Memory: %d\n", TotalFreeMemory());
                nlPrintf("Largest Free Block: %d\n", LargestFreeBlock());
                nlBreak();
            }
        } while (true);

        {
            FreeBlockList* prev = cur->m_prev;
            nlDLRingRemove<FreeBlockList>(&m_free_block_list, cur);
            u32 remaining = cur->m_unk_0x08 - offset;
            if (remaining > 0xC)
            {
                FreeBlockList* newFree = (FreeBlockList*)((char*)cur + offset);
                newFree->m_unk_0x08 = remaining;
                if (m_free_block_list == NULL || cur == start)
                {
                    nlDLRingAddStart<FreeBlockList>(&m_free_block_list, newFree);
                }
                else
                {
                    nlDLRingInsert<FreeBlockList>(&m_free_block_list, prev, newFree);
                }
                cur->m_unk_0x08 = offset;
            }

            u32 currentBlockSize = cur->m_unk_0x08;
            u32 header = savedSize;
            void* allocPtr = (void*)((char*)cur + size);
            u32 suffixSize = currentBlockSize - offset;
            if (size > 4)
            {
                header = savedSize | 0x80000000;
                *(u32*)((char*)allocPtr - 8) = size - 4;
            }
            u8* blockEnd = (u8*)allocPtr + savedSize;
            if (suffixSize != 0)
            {
                header |= 0x40000000;
                *(u32*)(((u32)blockEnd + 3) & ~3u) = suffixSize;
            }
            *(u32*)((char*)allocPtr - 4) = header;
            result = allocPtr;
        }
    }

    return result;
}

/**
 * Offset/Address/Size: 0xE0 | 0x801CD82C | size: 0xF8
 */
void MemoryAllocator::Initialize(void* arg0, unsigned int arg1)
{
    FreeBlockList* start;
    FreeBlockList* iter;
    FreeBlockList* next;
    FreeBlockList* prev;
    u32 size;

    m_free_block_list = NULL;
    ((FreeBlockList*)arg0)->m_unk_0x08 = arg1;
    start = nlDLRingGetStart<FreeBlockList>(m_free_block_list);
    if ((start > (FreeBlockList*)arg0) || (start == NULL))
    {
        nlDLRingAddStart<FreeBlockList>(&m_free_block_list, (FreeBlockList*)arg0);
    }
    else
    {
        iter = start->m_next;
        while (iter != start)
        {
            if (iter > (FreeBlockList*)arg0)
            {
                break;
            }

            iter = iter->m_next;
        }

        nlDLRingInsert<FreeBlockList>(&m_free_block_list, iter->m_prev, (FreeBlockList*)arg0);
    }

    next = ((FreeBlockList*)arg0)->m_next;
    if (next > (FreeBlockList*)arg0)
    {
        size = ((FreeBlockList*)arg0)->m_unk_0x08;
        if ((FreeBlockList*)((u8*)arg0 + size) == next)
        {
            ((FreeBlockList*)arg0)->m_unk_0x08 = size + next->m_unk_0x08;
            nlDLRingRemove<FreeBlockList>(&m_free_block_list, next);
        }
    }

    prev = ((FreeBlockList*)arg0)->m_prev;
    if (prev < (FreeBlockList*)arg0)
    {
        size = prev->m_unk_0x08;
        if ((FreeBlockList*)((u8*)prev + size) == (FreeBlockList*)arg0)
        {
            prev->m_unk_0x08 = size + ((FreeBlockList*)arg0)->m_unk_0x08;
            nlDLRingRemove<FreeBlockList>(&m_free_block_list, (FreeBlockList*)arg0);
        }
    }
}

/**
 * Offset/Address/Size: 0xCC | 0x801CD818 | size: 0x14
 */
void TotalFreeMemCallback::Callback(FreeBlockList* fbl)
{
    m_unk_0x00 = m_unk_0x00 + fbl->m_unk_0x08;
}

/**
 * Offset/Address/Size: 0x74 | 0x801CD7C0 | size: 0x58
 */
unsigned int MemoryAllocator::TotalFreeMemory()
{
    TotalFreeMemCallback callback;
    callback.m_unk_0x00 = 0;
    nlWalkDLRing<FreeBlockList, TotalFreeMemCallback>(m_free_block_list, &callback, &TotalFreeMemCallback::Callback);
    return callback.m_unk_0x00;
}

/**
 * Offset/Address/Size: 0x58 | 0x801CD7A4 | size: 0x1C
 */
void LargestFreeBlockCallback::Callback(FreeBlockList* fbl)
{
    u32 temp_r0;
    u32 var_r5;

    var_r5 = m_unk_0x00;
    temp_r0 = fbl->m_unk_0x08;
    if (temp_r0 >= var_r5)
    {
        var_r5 = temp_r0;
    }
    m_unk_0x00 = var_r5;
}

/**
 * Offset/Address/Size: 0x0 | 0x801CD74C | size: 0x58
 */
unsigned int MemoryAllocator::LargestFreeBlock()
{
    LargestFreeBlockCallback callback;
    callback.m_unk_0x00 = 0;
    nlWalkDLRing<FreeBlockList, LargestFreeBlockCallback>(m_free_block_list, &callback, &LargestFreeBlockCallback::Callback);
    return callback.m_unk_0x00;
}
