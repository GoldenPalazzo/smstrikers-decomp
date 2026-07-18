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
    for (int i = 0; i < m_nodeCount; i++)
    {
        if (nodeID == m_nodeIDs[i])
        {
            return i;
        }
    }
    return -1;
}

/**
 * Offset/Address/Size: 0xB8 | 0x801EE0A4 | size: 0x18
 */
s32 cSHierarchy::GetChild(int parentIndex, int childIndex) const
{
    return m_childArrays[parentIndex][childIndex];
}

static inline void BuildPushPopFlagsChild(cSHierarchy* hierarchy, int nodeIndex, int currentDepth, int& stackDepth)
{
    int gchild;
    int newSD3;
    int ggchild;
    int cc3;
    int l;
    int newSD2;
    int cc2;
    int k;
    int newSD1;
    int childCount;
    int j;

    if (currentDepth != stackDepth)
    {
        hierarchy->m_pushPopFlags[nodeIndex - 1] = currentDepth - stackDepth;
        stackDepth = currentDepth;
    }

    childCount = hierarchy->m_childCounts[nodeIndex];
    if (childCount != 0)
    {
        hierarchy->m_pushPopFlags[nodeIndex] = 1;
        stackDepth += 1;
        newSD1 = stackDepth;

        for (j = 0; j < childCount; j++)
        {
            gchild = hierarchy->m_childArrays[nodeIndex][j];

            if (newSD1 != stackDepth)
            {
                hierarchy->m_pushPopFlags[gchild - 1] = newSD1 - stackDepth;
                stackDepth = newSD1;
            }

            cc2 = hierarchy->m_childCounts[gchild];
            if (cc2 != 0)
            {
                hierarchy->m_pushPopFlags[gchild] = 1;
                stackDepth += 1;
                newSD2 = stackDepth;

                for (k = 0; k < cc2; k++)
                {
                    ggchild = hierarchy->GetChild(gchild, k);

                    if (newSD2 != stackDepth)
                    {
                        hierarchy->m_pushPopFlags[ggchild - 1] = newSD2 - stackDepth;
                        stackDepth = newSD2;
                    }

                    cc3 = hierarchy->m_childCounts[ggchild];
                    if (cc3 != 0)
                    {
                        hierarchy->m_pushPopFlags[ggchild] = 1;
                        stackDepth += 1;
                        newSD3 = stackDepth;

                        for (l = 0; l < cc3; l++)
                        {
                            hierarchy->BuildPushPopFlags(hierarchy->GetChild(ggchild, l), newSD3, stackDepth);
                        }
                    }
                    else
                    {
                        hierarchy->m_pushPopFlags[ggchild] = 0;
                    }
                }
            }
            else
            {
                hierarchy->m_pushPopFlags[gchild] = 0;
            }
        }
    }
    else
    {
        hierarchy->m_pushPopFlags[nodeIndex] = 0;
    }
}

/**
 * Offset/Address/Size: 0xD0 | 0x801EE0BC | size: 0x284
 */
void cSHierarchy::BuildPushPopFlags(int nodeIndex, int currentDepth, int& stackDepth)
{
    int child;
    int i;
    int childCount;

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
        currentDepth = stackDepth;

        for (i = 0; i < childCount; i++)
        {
            child = m_childArrays[nodeIndex][i];
            BuildPushPopFlagsChild(this, child, currentDepth, stackDepth);
        }
    }
    else
    {
        m_pushPopFlags[nodeIndex] = 0;
    }
}

/**
 * Offset/Address/Size: 0x354 | 0x801EE340 | size: 0x3E0
 */
cSHierarchy* cSHierarchy::Initialize(nlChunk* pChunk)
{
    pChunk = pChunk->GetFirstChunk();
    cSHierarchy* pRetval = (cSHierarchy*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_szName = (const char*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_nodeIDs = (u32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_parentIndices = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_childCounts = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_childArrays = (s32**)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pushPopFlags = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    s32* pChild = (s32*)pChunk->GetData();

    for (int i = 0; i < pRetval->m_nodeCount; i++)
    {
        if (pRetval->m_childCounts[i] > 0)
        {
            pRetval->m_childArrays[i] = pChild;
        }
        else
        {
            pRetval->m_childArrays[i] = 0;
        }
        pChild += pRetval->m_childCounts[i];
    }

    int nCurrentDepth = 0;
    pRetval->BuildPushPopFlags(0, 0, nCurrentDepth);

    pChunk = pChunk->GetNextChunk();
    pRetval->m_mirroredNodeIndices = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_translationOffsets = (nlVector3*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_boneLengthFlags = (u8*)pChunk->GetData();

    return pRetval;
}
