#include "Game/SHierarchy.h"
#include "types.h"

/**
 * Offset/Address/Size: 0x0 | 0x801EDFEC | size: 0x18
 */
bool cSHierarchy::PreserveBoneLength(int nodeIndex) const
{
    return m_boneLengthFlags[nodeIndex] ? true : false;
}

/**
 * Offset/Address/Size: 0x18 | 0x801EE004 | size: 0x10
 */
nlVector3* cSHierarchy::GetTranslationOffset(int nodeIndex) const
{
    return &m_translationOffsets[nodeIndex];
}

/**
 * Offset/Address/Size: 0x28 | 0x801EE014 | size: 0x10
 */
s32 cSHierarchy::GetParent(int nodeIndex) const
{
    return m_parentIndices[nodeIndex];
}

/**
 * Offset/Address/Size: 0x38 | 0x801EE024 | size: 0x10
 */
s32 cSHierarchy::GetPushPop(int nodeIndex) const
{
    return m_pushPopFlags[nodeIndex];
}

/**
 * Offset/Address/Size: 0x48 | 0x801EE034 | size: 0x10
 */
s32 cSHierarchy::GetMirroredNode(int nodeIndex) const
{
    return m_mirroredNodeIndices[nodeIndex];
}

/**
 * Offset/Address/Size: 0x58 | 0x801EE044 | size: 0x10
 */
s32 cSHierarchy::GetNumChildren(int nodeIndex) const
{
    return m_childCounts[nodeIndex];
}

/**
 * Offset/Address/Size: 0x68 | 0x801EE054 | size: 0x10
 */
u32 cSHierarchy::GetNodeID(int nodeIndex) const
{
    return m_nodeIDs[nodeIndex];
}

/**
 * Offset/Address/Size: 0x78 | 0x801EE064 | size: 0x40
 */
s32 cSHierarchy::GetNodeIndexByID(unsigned int nodeID) const
{
    int i = 0;
    u32* pNodeID = GetNodeIDs();

    for (int count = GetNodeCount(); count > 0; count--)
    {
        if (nodeID == GetNodeID(i))
        {
            return i;
        }
        pNodeID++;
        i++;
    }
    return -1;
}

/**
 * Offset/Address/Size: 0xB8 | 0x801EE0A4 | size: 0x18
 */
s32 cSHierarchy::GetChild(int parentIndex, int childIndex) const
{
    FORCE_DONT_INLINE;
    return m_childArrays[parentIndex][childIndex];
}

// NONMATCHING - regalloc
/**
 * Offset/Address/Size: 0xD0 | 0x801EE0BC | size: 0x284
 */
void cSHierarchy::BuildPushPopFlags(int nodeIndex, int currentDepth, int& stackDepth)
{
    int newSD1;
    int child;
    int k;
    int cc2;
    int newSD2;
    int l;
    int cc3;
    int ggchild;
    int newSD3;
    int gchild;
    int j;
    int cc1;
    int i;
    int childCount;
    int newSD0;

    if (currentDepth != stackDepth)
    {
        m_pushPopFlags[nodeIndex - 1] = currentDepth - stackDepth;
        stackDepth = currentDepth;
    }

    childCount = m_childCounts[nodeIndex];
    if (childCount != 0)
    {
        m_pushPopFlags[nodeIndex] = 1;
        stackDepth += 1;
        newSD0 = stackDepth;

        for (i = 0; i < childCount; i++)
        {
            child = m_childArrays[nodeIndex][i];

            if (newSD0 != stackDepth)
            {
                m_pushPopFlags[child - 1] = newSD0 - stackDepth;
                stackDepth = newSD0;
            }

            cc1 = m_childCounts[child];
            if (cc1 != 0)
            {
                m_pushPopFlags[child] = 1;
                stackDepth += 1;
                newSD1 = stackDepth;

                for (j = 0; j < cc1; j++)
                {
                    gchild = m_childArrays[child][j];

                    if (newSD1 != stackDepth)
                    {
                        m_pushPopFlags[gchild - 1] = newSD1 - stackDepth;
                        stackDepth = newSD1;
                    }

                    cc2 = m_childCounts[gchild];
                    if (cc2 != 0)
                    {
                        m_pushPopFlags[gchild] = 1;
                        stackDepth += 1;
                        newSD2 = stackDepth;

                        for (k = 0; k < cc2; k++)
                        {
                            ggchild = GetChild(gchild, k);

                            if (newSD2 != stackDepth)
                            {
                                m_pushPopFlags[ggchild - 1] = newSD2 - stackDepth;
                                stackDepth = newSD2;
                            }

                            cc3 = m_childCounts[ggchild];
                            if (cc3 != 0)
                            {
                                m_pushPopFlags[ggchild] = 1;
                                stackDepth += 1;
                                newSD3 = stackDepth;

                                for (l = 0; l < cc3; l++)
                                {
                                    BuildPushPopFlags(GetChild(ggchild, l), newSD3, stackDepth);
                                }
                            }
                            else
                            {
                                m_pushPopFlags[ggchild] = 0;
                            }
                        }
                    }
                    else
                    {
                        m_pushPopFlags[gchild] = 0;
                    }
                }
            }
            else
            {
                m_pushPopFlags[child] = 0;
            }
        }
    }
    else
    {
        m_pushPopFlags[nodeIndex] = 0;
    }
}

static inline void* nlGetChunkData(nlChunk* chunk)
{
    u32 alignField = chunk->m_ID & 0x7F000000;
    if (((-alignField) | alignField) >> 31)
    {
        alignField = 1u << (alignField >> 24);
        u32 result = (u32)chunk + alignField;
        result = (result + 7) & ~(alignField - 1);
        return (void*)result;
    }
    return (void*)((u8*)chunk + 8);
}

static inline nlChunk* nlGetNextChunk(nlChunk* chunk)
{
    return (nlChunk*)((u8*)chunk + chunk->m_Size + 8);
}

// NONMATCHING - regalloc
/**
 * Offset/Address/Size: 0x354 | 0x801EE340 | size: 0x3E0
 */
cSHierarchy* cSHierarchy::Initialize(nlChunk* chunkData)
{
    nlChunk* chunk = (nlChunk*)((u8*)chunkData + 8);
    cSHierarchy* hier = (cSHierarchy*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_szName = (const char*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_nodeIDs = (u32*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_parentIndices = (s32*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_childCounts = (s32*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_childArrays = (s32**)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_pushPopFlags = (s32*)nlGetChunkData(chunk);

    nlChunk* childDataChunk = nlGetNextChunk(chunk);
    s32* childData = (s32*)nlGetChunkData(childDataChunk);

    for (int i = 0; i < hier->m_nodeCount; i++)
    {
        if (hier->m_childCounts[i] > 0)
        {
            hier->m_childArrays[i] = childData;
        }
        else
        {
            hier->m_childArrays[i] = 0;
        }
        childData += hier->m_childCounts[i];
    }

    int stackDepth = 0;
    hier->BuildPushPopFlags(0, 0, stackDepth);

    chunk = nlGetNextChunk(childDataChunk);
    hier->m_mirroredNodeIndices = (s32*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_translationOffsets = (nlVector3*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    hier->m_boneLengthFlags = (u8*)nlGetChunkData(chunk);

    return hier;
}
