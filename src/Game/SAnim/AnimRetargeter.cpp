#include "Game/SAnim/AnimRetargeter.h"

/**
 * Offset/Address/Size: 0x0 | 0x801EFF90 | size: 0x48
 */
static inline AnimRetarget* GetAnimRetargetWithSignature_ARL(AnimRetargetList* list, const cSAnim* anim)
{
    long offset;
    AnimRetarget* p;
    AnimRetarget* result = NULL;
    offset = (long)result;

    for (long i = list->m_NumAnimRetargets; i > 0; i--)
    {
        p = (AnimRetarget*)((char*)list->m_pAnimRetarget + offset);
        if (anim->m_nHierarchySignature == p->m_TargetHierarchySignature)
        {
            result = p;
            break;
        }
        offset += sizeof(AnimRetarget);
    }

    return result;
}

AnimRetarget* AnimRetargetList::GetAnimRetargetWithSignature(const cSAnim* anim)
{
    return GetAnimRetargetWithSignature_ARL(this, anim);
}

static inline void* GetChunkData_ARL(nlChunk* chunk)
{
    u32 align = chunk->m_ID & 0x7F000000;

    if (((-align | align) >> 31) != 0)
    {
        u32 mask = 1 << (align >> 24);
        u32 ptr = (u32)chunk + mask;
        ptr += 7;
        ptr &= ~(mask - 1);
        return (void*)ptr;
    }

    return (void*)((u8*)chunk + 8);
}

/**
 * Offset/Address/Size: 0x48 | 0x801EFFD8 | size: 0x10C
 * TODO: 96.8% match - first chunk-data result register and loop chunk-pointer
 * setup still differ.
 */
AnimRetargetList* AnimRetargetList::Initialize(nlChunk* chunkData)
{
    AnimRetargetList* data;
    nlChunk* chunk = (nlChunk*)((u8*)chunkData + 8);
    u32 align = chunk->m_ID & 0x7F000000;
    u32 ptr;

    if (((-align | align) >> 31) != 0)
    {
        u32 mask = 1 << (align >> 24);
        ptr = (u32)chunk + mask;
        ptr += 7;
        ptr &= ~(mask - 1);
        data = (AnimRetargetList*)ptr;
    }
    else
    {
        data = (AnimRetargetList*)((u8*)chunk + 8);
    }

    nlChunk* nextChunk = (nlChunk*)((u8*)chunk + chunk->m_Size + 0x10);
    data->m_pAnimRetarget = (AnimRetarget*)GetChunkData_ARL(nextChunk);

    s32 i = 0;
    s32 off = 0;

    while (i < data->m_NumAnimRetargets)
    {
        nlChunk* mapChunk = (nlChunk*)((u8*)nextChunk + nextChunk->m_Size + 8);
        nextChunk = mapChunk;
        signed short* nextMap = (signed short*)GetChunkData_ARL(mapChunk);

        *(signed short**)((u8*)data->m_pAnimRetarget + off + 8) = nextMap;
        off += 0xC;
        i++;
    }

    return data;
}
