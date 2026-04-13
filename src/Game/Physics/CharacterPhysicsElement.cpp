#include "Game/Physics/CharacterPhysicsElement.h"

#include "NL/nlFile.h"
#include "NL/nlMemory.h"

#include "Game/SAnim.h"

/**
 * Offset/Address/Size: 0x0 | 0x801FE13C | size: 0x2AC
 * TODO: 93.68% match - alignment address formation add folding (both cases)
 * and copy-loop r3/r4 register swap (-inline deferred scratch limitation).
 */
bool LoadCharacterPhysicsElements(const char* pFileData, CharacterPhysicsData* pPhysicsData)
{
    u8* var_r30;
    u8* temp_r29;

    u32 dataSize;
    u8* rawData = (u8*)nlLoadEntireFile(pFileData, &dataSize, 0x20, AllocateStart);
    if (rawData == 0)
    {
        return false;
    }

    u32 temp_r4 = *((u32*)(rawData + 4));
    var_r30 = rawData + 8;
    temp_r29 = rawData + temp_r4 + 8;

    while (var_r30 < temp_r29)
    {
        s32 temp_r5 = *(s32*)var_r30;
        s32 temp_r4_2 = temp_r5 & 0x80FFFFFF;

        switch (temp_r4_2)
        {
        case 0x0001D001:
        {
            s32 temp_r3 = temp_r5 & 0x7F000000;
            u8* var_r3;
            if ((((u32)(-temp_r3 | temp_r3)) >> 31) != 0)
            {
                s32 temp_r4_3 = 1 << ((u32)temp_r3 >> 24);
                u8* ptr = var_r30 + temp_r4_3;
                ptr += 7;
                var_r3 = (u8*)((u32)ptr & ~(u32)(temp_r4_3 - 1));
            }
            else
            {
                var_r3 = var_r30 + 8;
            }

            pPhysicsData->physicsElementCount = *(u32*)var_r3;
            pPhysicsData->pPhysicsElements = (CharacterPhysicsElement*)nlMalloc(pPhysicsData->physicsElementCount * sizeof(CharacterPhysicsElement), 8, false);
            break;
        }

        case 0x0001D002:
        {
            u32 n;
            s32 temp_r3 = temp_r5 & 0x7F000000;
            u8* var_r5;
            if ((((u32)(-temp_r3 | temp_r3)) >> 31) != 0)
            {
                s32 temp_r4_3 = 1 << ((u32)temp_r3 >> 24);
                u8* ptr = var_r30 + temp_r4_3;
                ptr += 7;
                var_r5 = (u8*)((u32)ptr & ~(u32)(temp_r4_3 - 1));
            }
            else
            {
                var_r5 = var_r30 + 8;
            }

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

        u32 chunkSize = *(u32*)(var_r30 + 4);
        var_r30 += chunkSize + 8;
    }

    delete rawData;
    return true;
}
