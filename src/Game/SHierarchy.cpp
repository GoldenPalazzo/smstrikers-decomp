#include "Game/SHierarchy.h"
#include "types.h"

/**
 * Offset/Address/Size: 0x0 | 0x801EDFEC | size: 0x18
 */
bool cSHierarchy::PreserveBoneLength(int i) const
{
    return m_pPreserveBoneLength[i] ? true : false;
}

/**
 * Offset/Address/Size: 0x18 | 0x801EE004 | size: 0x10
 */
nlVector3& cSHierarchy::GetTranslationOffset(int i) const
{
    return m_pV3TranslationOffset[i];
}

/**
 * Offset/Address/Size: 0x28 | 0x801EE014 | size: 0x10
 */
s32 cSHierarchy::GetParent(int i) const
{
    return m_pParent[i];
}

/**
 * Offset/Address/Size: 0x38 | 0x801EE024 | size: 0x10
 */
s32 cSHierarchy::GetPushPop(int i) const
{
    return m_pPushPop[i];
}

/**
 * Offset/Address/Size: 0x48 | 0x801EE034 | size: 0x10
 */
s32 cSHierarchy::GetMirroredNode(int i) const
{
    return m_pMirrorTable[i];
}

/**
 * Offset/Address/Size: 0x58 | 0x801EE044 | size: 0x10
 */
s32 cSHierarchy::GetNumChildren(int i) const
{
    return m_pNumChildren[i];
}

/**
 * Offset/Address/Size: 0x68 | 0x801EE054 | size: 0x10
 */
u32 cSHierarchy::GetNodeID(int i) const
{
    return m_pNodeID[i];
}

/**
 * Offset/Address/Size: 0x78 | 0x801EE064 | size: 0x40
 */
s32 cSHierarchy::GetNodeIndexByID(unsigned int id) const
{
    for (int i = 0; i < m_nNumNodes; i++)
    {
        if (id == m_pNodeID[i])
        {
            return i;
        }
    }
    return -1;
}

/**
 * Offset/Address/Size: 0xB8 | 0x801EE0A4 | size: 0x18
 */
s32 cSHierarchy::GetChild(int i, int j) const
{
    return m_pChildren[i][j];
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
        hierarchy->m_pPushPop[nodeIndex - 1] = currentDepth - stackDepth;
        stackDepth = currentDepth;
    }

    childCount = hierarchy->m_pNumChildren[nodeIndex];
    if (childCount != 0)
    {
        hierarchy->m_pPushPop[nodeIndex] = 1;
        stackDepth += 1;
        newSD1 = stackDepth;

        for (j = 0; j < childCount; j++)
        {
            gchild = hierarchy->m_pChildren[nodeIndex][j];

            if (newSD1 != stackDepth)
            {
                hierarchy->m_pPushPop[gchild - 1] = newSD1 - stackDepth;
                stackDepth = newSD1;
            }

            cc2 = hierarchy->m_pNumChildren[gchild];
            if (cc2 != 0)
            {
                hierarchy->m_pPushPop[gchild] = 1;
                stackDepth += 1;
                newSD2 = stackDepth;

                for (k = 0; k < cc2; k++)
                {
                    ggchild = hierarchy->GetChild(gchild, k);

                    if (newSD2 != stackDepth)
                    {
                        hierarchy->m_pPushPop[ggchild - 1] = newSD2 - stackDepth;
                        stackDepth = newSD2;
                    }

                    cc3 = hierarchy->m_pNumChildren[ggchild];
                    if (cc3 != 0)
                    {
                        hierarchy->m_pPushPop[ggchild] = 1;
                        stackDepth += 1;
                        newSD3 = stackDepth;

                        for (l = 0; l < cc3; l++)
                        {
                            hierarchy->BuildPushPopFlags(hierarchy->GetChild(ggchild, l), newSD3, stackDepth);
                        }
                    }
                    else
                    {
                        hierarchy->m_pPushPop[ggchild] = 0;
                    }
                }
            }
            else
            {
                hierarchy->m_pPushPop[gchild] = 0;
            }
        }
    }
    else
    {
        hierarchy->m_pPushPop[nodeIndex] = 0;
    }
}

/**
 * Offset/Address/Size: 0xD0 | 0x801EE0BC | size: 0x284
 */
void cSHierarchy::BuildPushPopFlags(int nNode, int nParentDepth, int& nCurrentDepth)
{
    int child;
    int i;
    int nNumChildren;

    if (nParentDepth != nCurrentDepth)
    {
        m_pPushPop[nNode - 1] = nParentDepth - nCurrentDepth;
        nCurrentDepth = nParentDepth;
    }

    nNumChildren = m_pNumChildren[nNode];
    if (nNumChildren != 0)
    {
        m_pPushPop[nNode] = 1;
        nCurrentDepth += 1;
        nParentDepth = nCurrentDepth;

        for (i = 0; i < nNumChildren; i++)
        {
            child = m_pChildren[nNode][i];
            BuildPushPopFlagsChild(this, child, nParentDepth, nCurrentDepth);
        }
    }
    else
    {
        m_pPushPop[nNode] = 0;
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
    pRetval->m_pNodeID = (u32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pParent = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pNumChildren = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pChildren = (s32**)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pPushPop = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    s32* pChild = (s32*)pChunk->GetData();

    for (int i = 0; i < pRetval->m_nNumNodes; i++)
    {
        if (pRetval->m_pNumChildren[i] > 0)
        {
            pRetval->m_pChildren[i] = pChild;
        }
        else
        {
            pRetval->m_pChildren[i] = 0;
        }
        pChild += pRetval->m_pNumChildren[i];
    }

    int nCurrentDepth = 0;
    pRetval->BuildPushPopFlags(0, 0, nCurrentDepth);

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pMirrorTable = (s32*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pV3TranslationOffset = (nlVector3*)pChunk->GetData();

    pChunk = pChunk->GetNextChunk();
    pRetval->m_pPreserveBoneLength = (u8*)pChunk->GetData();

    return pRetval;
}
