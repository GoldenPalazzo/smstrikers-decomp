#include "Game/Physics/CharacterPhysicsElement.h"

#include "NL/nlFile.h"
#include "NL/nlMemory.h"

#include "Game/SAnim.h"

static inline void* nlGetChunkData(nlChunk* chunk)
{
    u32 alignField = chunk->m_ID & 0x7F000000;
    u32 isAligned = ((-alignField) | alignField) >> 31;
    if (isAligned != 0)
    {
        u32 alignment = 1u << (alignField >> 24);
        u32 ptr = (u32)chunk + alignment;
        ptr += 7;
        ptr &= ~(alignment - 1);
        return (void*)ptr;
    }
    return (void*)((u8*)chunk + 8);
}

static inline nlChunk* nlGetNextChunk(nlChunk* chunk)
{
    return (nlChunk*)((u8*)chunk + chunk->m_Size + 8);
}

/**
 * Offset/Address/Size: 0x0 | 0x801FE13C | size: 0x2AC
 * TODO: 96.84% match - nlGetChunkData return-via-r3 forces extra mr r5,r3
 * in case 2, and copy-loop counter n stays in r4 instead of r3.
 */
bool LoadCharacterPhysicsElements(const char* pFileData, CharacterPhysicsData* pPhysicsData)
{
    nlChunk* var_r30;
    nlChunk* temp_r29;

    u32 dataSize;
    u8* rawData = (u8*)nlLoadEntireFile(pFileData, &dataSize, 0x20, AllocateStart);
    if (rawData == 0)
    {
        return false;
    }

    u32 temp_r4 = *((u32*)(rawData + 4));
    var_r30 = (nlChunk*)(rawData + 8);
    temp_r29 = (nlChunk*)(rawData + temp_r4 + 8);

    while (var_r30 < temp_r29)
    {
        s32 temp_r5 = (s32)var_r30->m_ID;
        s32 temp_r4_2 = temp_r5 & 0x80FFFFFF;

        switch (temp_r4_2)
        {
        case 0x0001D001:
        {
            pPhysicsData->physicsElementCount = *(u32*)nlGetChunkData(var_r30);
            pPhysicsData->pPhysicsElements = (CharacterPhysicsElement*)nlMalloc(pPhysicsData->physicsElementCount * sizeof(CharacterPhysicsElement), 8, false);
            break;
        }

        case 0x0001D002:
        {
            u32 n;
            u8* var_r5 = (u8*)nlGetChunkData(var_r30);

            n = 0;
            unsigned long i = n;
            while (n < pPhysicsData->physicsElementCount)
            {
                *(CharacterPhysicsElement*)((u8*)pPhysicsData->pPhysicsElements + i) = *(CharacterPhysicsElement*)var_r5;
                n++;
                i += sizeof(CharacterPhysicsElement);
                var_r5 += sizeof(CharacterPhysicsElement);
            }
            break;
        }
        }

        var_r30 = nlGetNextChunk(var_r30);
    }

    delete rawData;
    return true;
}
