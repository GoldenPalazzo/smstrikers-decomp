#include "Game/SAnim.h"
#include "Game/PoseAccumulator.h"

#include "NL/nlMemory.h"
#include "NL/nlList.h"

#pragma inline_depth(8)
#pragma inline_max_size(0x10000)
#pragma inline_max_total_size(0x10000)

/**
 * Offset/Address/Size: 0x1420 | 0x801EA634 | size: 0x28
 */
template <>
void nlListAddStart<cSAnimCallback>(cSAnimCallback** head, cSAnimCallback* entry, cSAnimCallback** tail)
{
    if (tail != 0)
    {
        if (*head == 0)
        {
            *tail = entry;
        }
    }

    entry->next = *head;
    *head = entry;
}

#define NL_GET_CHUNK_DATA_AS(chunk, out, type)              \
    do                                                      \
    {                                                       \
        u32 alignField = (chunk)->m_ID & 0x7F000000;        \
        u32 ptr;                                            \
        u32 isAligned = ((-alignField) | alignField) >> 31; \
        if (isAligned != 0)                                 \
        {                                                   \
            u32 alignment = 1u << (alignField >> 24);       \
            ptr = (u32)(chunk) + alignment;                 \
            ptr += 7;                                       \
            ptr &= ~(alignment - 1);                        \
        }                                                   \
        else                                                \
        {                                                   \
            ptr = (u32)(chunk) + 8;                         \
        }                                                   \
        (out) = (type)ptr;                                  \
    } while (0)

#define GET_ROOT_TRANS_FOR_INITIALIZE(anim, numRootKeys, t, out)                  \
    do                                                                            \
    {                                                                             \
        if ((numRootKeys) != 0)                                                   \
        {                                                                         \
            if ((t) == 1.0f || (numRootKeys) == 1)                                \
            {                                                                     \
                const nlVector3* pSrc = &(anim)->m_pRootTrans[(numRootKeys) - 1]; \
                ((u32*)(out))[0] = ((const u32*)pSrc)[0];                         \
                ((u32*)(out))[1] = ((const u32*)pSrc)[1];                         \
                ((u32*)(out))[2] = ((const u32*)pSrc)[2];                         \
            }                                                                     \
            else                                                                  \
            {                                                                     \
                float fRealIndex = (numRootKeys) - 1;                             \
                fRealIndex *= (t);                                                \
                int nIndex0 = (int)fRealIndex;                                    \
                int nIndex1 = nIndex0 + 1;                                        \
                const nlVector3* pRootTrans = (anim)->m_pRootTrans;               \
                const nlVector3* pVal0 = &pRootTrans[nIndex0];                    \
                const nlVector3* pVal1 = &pRootTrans[nIndex1];                    \
                float fWeight = fRealIndex - nIndex0;                             \
                float fInvWeight = 1.0f - fWeight;                                \
                (out)->f.x = (fWeight * pVal1->f.x) + (fInvWeight * pVal0->f.x);  \
                (out)->f.y = (fWeight * pVal1->f.y) + (fInvWeight * pVal0->f.y);  \
                (out)->f.z = (fWeight * pVal1->f.z) + (fInvWeight * pVal0->f.z);  \
            }                                                                     \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            (out)->f.x = 0.0f;                                                    \
            (out)->f.y = 0.0f;                                                    \
            (out)->f.z = 0.0f;                                                    \
        }                                                                         \
    } while (0)

#define nlGetNextChunk(chunk) ((nlChunk*)((u8*)(chunk) + (chunk)->m_Size + 8))

#pragma inline_depth(8)
#pragma inline_max_size(0x10000)
#pragma inline_max_total_size(0x10000)
/**
 * Offset/Address/Size: 0xD40 | 0x801E9F54 | size: 0x68C
 * TODO: 86.16% match - chunk-data helper and inlined root translation still
 * have register-coloring differences in chunk walks and interpolation setup.
 */
cSAnim* cSAnim::Initialize(nlChunk* pChunk)
{
    nlChunk* chunkA = (nlChunk*)((u8*)pChunk + 8);
    nlChunk* end = nlGetNextChunk(pChunk);
    nlChunk* chunkB;
    nlChunk* chunkC;

    cSAnim* pRetval;
    NL_GET_CHUNK_DATA_AS(chunkA, pRetval, cSAnim*);
    pRetval->m_pCallbackList = NULL;

    chunkB = (nlChunk*)((u8*)chunkA + chunkA->m_Size + 8);
    NL_GET_CHUNK_DATA_AS(chunkB, pRetval->m_szName, const char*);

    chunkC = (nlChunk*)((u8*)chunkB + chunkB->m_Size + 8);
    NL_GET_CHUNK_DATA_AS(chunkC, pRetval->m_pRotKeys, void*);

    chunkA = (nlChunk*)((u8*)chunkC + chunkC->m_Size + 8);
    NL_GET_CHUNK_DATA_AS(chunkA, pRetval->m_pTransKeys, PackedTrans**);

    chunkC = (nlChunk*)((u8*)chunkA + chunkA->m_Size + 8);
    NL_GET_CHUNK_DATA_AS(chunkC, pRetval->m_pScaleKeys, PackedScale**);

    chunkA = (nlChunk*)((u8*)chunkC + chunkC->m_Size + 8);
    NL_GET_CHUNK_DATA_AS(chunkA, pRetval->m_pRootRot, unsigned short*);

    chunkC = (nlChunk*)((u8*)chunkA + chunkA->m_Size + 8);
    NL_GET_CHUNK_DATA_AS(chunkC, pRetval->m_pRootTrans, nlVector3*);

    u32 nodeIndex = 0;
    u32 type;
    nlChunk* nodeChunk = nlGetNextChunk(chunkC);

    while (nodeChunk != end && ((type = nodeChunk->m_ID & 0x80FFFFFF) == 0x80017100 || type == 0x1001))
    {
        if (type == 0x80017100)
        {
            nlChunk* subChunk = (nlChunk*)((u8*)nodeChunk + 8);
            nlChunk* subEnd = nlGetNextChunk(nodeChunk);

            while (subChunk != subEnd)
            {
                u32 subType = subChunk->m_ID & 0x80FFFFFF;
                if (subType == 0x17101)
                    NL_GET_CHUNK_DATA_AS(subChunk, ((void**)pRetval->m_pRotKeys)[nodeIndex], void*);
                else if (subType == 0x17102)
                    NL_GET_CHUNK_DATA_AS(subChunk, pRetval->m_pTransKeys[nodeIndex], PackedTrans*);
                else if (subType == 0x17103)
                    NL_GET_CHUNK_DATA_AS(subChunk, pRetval->m_pScaleKeys[nodeIndex], PackedScale*);

                subChunk = nlGetNextChunk(subChunk);
            }

            nodeIndex++;
        }

        nodeChunk = nlGetNextChunk(nodeChunk);
    }

    nlVector3* rootTrans = pRetval->m_pRootTrans;
    nlVector3 v3PosStart;
    nlVector3 v3PosEnd;

    if (rootTrans != NULL)
    {
        u32 numRootKeys = pRetval->m_nNumRootKeys;
        GET_ROOT_TRANS_FOR_INITIALIZE(pRetval, numRootKeys, 0.0f, &v3PosStart);
        GET_ROOT_TRANS_FOR_INITIALIZE(pRetval, numRootKeys, 1.0f, &v3PosEnd);

        float dx = v3PosEnd.f.x - v3PosStart.f.x;
        float dy = v3PosEnd.f.y - v3PosStart.f.y;
        float dz = v3PosEnd.f.z - v3PosStart.f.z;
        float dist = nlSqrt(dz * dz + (dx * dx + (dy * dy)), true);

        pRetval->m_fLinearSpeed = dist / ((float)pRetval->m_nNumKeys / 30.0f);
    }
    else
    {
        pRetval->m_fLinearSpeed = 0.0f;
    }

    NL_GET_CHUNK_DATA_AS(nodeChunk, pRetval->m_pNumMorphKeys, const unsigned int*);

    nodeChunk = nlGetNextChunk(nodeChunk);
    NL_GET_CHUNK_DATA_AS(nodeChunk, pRetval->m_nMorphIds, unsigned long*);

    nodeChunk = nlGetNextChunk(nodeChunk);
    NL_GET_CHUNK_DATA_AS(nodeChunk, pRetval->m_pMorphKeys, unsigned char*);

    nodeChunk = nlGetNextChunk(nodeChunk);
    NL_GET_CHUNK_DATA_AS(nodeChunk, pRetval->m_pNodeProperties, const unsigned int*);

    return pRetval;
}
#pragma inline_depth()

/**
 * Offset/Address/Size: 0x91C | 0x801E9B30 | size: 0x424
 */
void cSAnim::BlendRot(int nodeIndex, int remappedNodeIndex, float tNorm, float weight, cPoseAccumulator* acc, bool additive) const
{
    void* pRawKeys = ((void**)m_pRotKeys)[remappedNodeIndex];
    if (pRawKeys != NULL && (unsigned int)remappedNodeIndex < m_nNumNodes)
    {
        unsigned int props = m_pNodeProperties[remappedNodeIndex];

        if (props & 0x2)
        {
            if (props & 0x1)
            {
                acc->BlendRotAroundZ(nodeIndex, ((unsigned short*)pRawKeys)[0], weight);
                return;
            }

            nlQuaternion q;
            q.f.x = 0.000061035156f * ((signed short*)pRawKeys)[0];
            q.f.y = 0.000061035156f * ((signed short*)pRawKeys)[1];
            q.f.z = 0.000061035156f * ((signed short*)pRawKeys)[2];
            q.f.w = 0.000061035156f * ((signed short*)pRawKeys)[3];
            acc->BlendRot(nodeIndex, &q, weight, additive);
            return;
        }

        if (1.0f == tNorm)
        {
            int lastIndex = m_nNumKeys - 1;

            if (props & 0x1)
            {
                acc->BlendRotAroundZ(nodeIndex, ((unsigned short*)pRawKeys)[lastIndex], weight);
                return;
            }

            signed short* pLast = ((signed short*)pRawKeys) + (lastIndex * 4);
            nlQuaternion q;
            q.f.x = 0.000061035156f * pLast[0];
            q.f.y = 0.000061035156f * pLast[1];
            q.f.z = 0.000061035156f * pLast[2];
            q.f.w = 0.000061035156f * pLast[3];
            acc->BlendRot(nodeIndex, &q, weight, additive);
            return;
        }

        float fRealIndex = tNorm * (m_nNumKeys - 1);
        int nKeyIndex = (int)fRealIndex;
        float fFrac = fRealIndex - nKeyIndex;
        float fWeight2 = weight * fFrac;
        float fWeight1 = weight - fWeight2;

        if (props & 0x1)
        {
            unsigned short* pKeys = (unsigned short*)pRawKeys;
            acc->BlendRotAroundZ(nodeIndex, pKeys[nKeyIndex], fWeight1);
        }
        else
        {
            signed short* pKey = ((signed short*)pRawKeys) + (nKeyIndex * 4);
            nlQuaternion q1;
            q1.f.x = 0.000061035156f * pKey[0];
            q1.f.y = 0.000061035156f * pKey[1];
            q1.f.z = 0.000061035156f * pKey[2];
            q1.f.w = 0.000061035156f * pKey[3];
            acc->BlendRot(nodeIndex, &q1, fWeight1, additive);
        }

        if (m_pNodeProperties[remappedNodeIndex] & 0x1)
        {
            unsigned short* pKeys = (unsigned short*)(((void**)m_pRotKeys)[remappedNodeIndex]);
            unsigned short* pKey = &pKeys[nKeyIndex];
            acc->BlendRotAroundZ(nodeIndex, pKey[1], fWeight2);
            return;
        }

        signed short* pKey = ((signed short*)(((void**)m_pRotKeys)[remappedNodeIndex])) + ((nKeyIndex + 1) * 4);
        nlQuaternion q2;
        q2.f.x = 0.000061035156f * pKey[0];
        q2.f.y = 0.000061035156f * pKey[1];
        q2.f.z = 0.000061035156f * pKey[2];
        q2.f.w = 0.000061035156f * pKey[3];
        acc->BlendRot(nodeIndex, &q2, fWeight2, additive);
        return;
    }

    acc->BlendRotIdentity(nodeIndex, weight);
}

/**
 * Offset/Address/Size: 0x608 | 0x801E981C | size: 0x314
 */
void cSAnim::BlendScale(int nodeIndex, int remappedNodeIndex, float tNorm, float weight, cPoseAccumulator* acc, bool additive) const
{
    PackedScale* pKeys = m_pScaleKeys[remappedNodeIndex];
    if (pKeys != NULL && (unsigned int)remappedNodeIndex < m_nNumNodes)
    {
        if (m_pNodeProperties[remappedNodeIndex] & 0x8)
        {
            nlVector3 v;
            v.f.x = 0.01f * pKeys[0].x;
            v.f.y = 0.01f * pKeys[0].y;
            v.f.z = 0.01f * pKeys[0].z;
            acc->BlendScale(nodeIndex, &v, weight, additive);
            return;
        }

        if (1.0f == tNorm)
        {
            PackedScale* pLastKey = &pKeys[m_nNumKeys - 1];
            nlVector3 v;
            v.f.x = 0.01f * pLastKey->x;
            v.f.y = 0.01f * pLastKey->y;
            v.f.z = 0.01f * pLastKey->z;
            acc->BlendScale(nodeIndex, &v, weight, additive);
            return;
        }

        float fRealIndex = tNorm * (m_nNumKeys - 1);
        int nKeyIndex = (int)fRealIndex;
        float fFrac = fRealIndex - nKeyIndex;
        float fWeight2 = weight * fFrac;
        float fWeight1 = weight - fWeight2;

        PackedScale* pKey = &pKeys[nKeyIndex];
        nlVector3 v1;
        v1.f.x = 0.01f * pKey->x;
        v1.f.y = 0.01f * pKey->y;
        v1.f.z = 0.01f * pKey->z;
        acc->BlendScale(nodeIndex, &v1, fWeight1, additive);

        PackedScale* pNextKey = &m_pScaleKeys[remappedNodeIndex][nKeyIndex + 1];
        nlVector3 v2;
        v2.f.x = 0.01f * pNextKey->x;
        v2.f.y = 0.01f * pNextKey->y;
        v2.f.z = 0.01f * pNextKey->z;
        acc->BlendScale(nodeIndex, &v2, fWeight2, additive);
    }
    else
    {
        acc->BlendScaleIdentity(nodeIndex, weight);
    }
}

/**
 * Offset/Address/Size: 0x404 | 0x801E9618 | size: 0x204
 */
void cSAnim::BlendTrans(int nAccumulatorNode, int nSAnimNode, float fTime, float fWeight, cPoseAccumulator* pAccumulator, bool bMirror) const
{
    if (pAccumulator->m_BaseSHierarchy->PreserveBoneLength(nAccumulatorNode))
    {
        return;
    }

    PackedTrans* pKeys = m_pTransKeys[nSAnimNode];
    if (pKeys != NULL && (unsigned int)nSAnimNode < m_nNumNodes)
    {
        if (m_pNodeProperties[nSAnimNode] & 0x4)
        {
            nlVector3 v;
            v.f.x = pKeys[0].x;
            v.f.y = pKeys[0].y;
            v.f.z = pKeys[0].z;
            pAccumulator->BlendTrans(nAccumulatorNode, &v, fWeight, bMirror);
            return;
        }

        if (1.0f == fTime)
        {
            PackedTrans* pLastKey = &pKeys[m_nNumKeys - 1];
            nlVector3 v;
            v.f.x = pLastKey->x;
            v.f.y = pLastKey->y;
            v.f.z = pLastKey->z;
            pAccumulator->BlendTrans(nAccumulatorNode, &v, fWeight, bMirror);
            return;
        }

        int nKeyIndex = (int)(fTime * (float)(m_nNumKeys - 1));
        float fFrac = fTime * (float)(m_nNumKeys - 1) - (float)nKeyIndex;
        float fWeight2 = fWeight * fFrac;
        float fWeight1 = fWeight - fWeight2;

        PackedTrans* pKey = &pKeys[nKeyIndex];
        nlVector3 v1;
        v1.f.x = pKey->x;
        v1.f.y = pKey->y;
        v1.f.z = pKey->z;
        pAccumulator->BlendTrans(nAccumulatorNode, &v1, fWeight1, bMirror);

        PackedTrans* pNextKey = &m_pTransKeys[nSAnimNode][nKeyIndex + 1];
        nlVector3 v2;
        v2.f.x = pNextKey->x;
        v2.f.y = pNextKey->y;
        v2.f.z = pNextKey->z;
        pAccumulator->BlendTrans(nAccumulatorNode, &v2, fWeight2, bMirror);
    }
    else
    {
        pAccumulator->BlendTransIdentity(nAccumulatorNode, fWeight);
    }
}

/**
 * Offset/Address/Size: 0x3CC | 0x801E95E0 | size: 0x38
 */
void cSAnim::Destroy()
{
    nlDeleteList<cSAnimCallback>(&m_pCallbackList);
    m_pCallbackList = 0;
}

/**
 * Offset/Address/Size: 0x2EC | 0x801E9500 | size: 0xE0
 * TODO: 98.61% match - remaining diffs are float conversion stack slot ordering
 * and f0/f1 register assignment in the interpolation tail.
 */
void cSAnim::GetRootRot(float fTime, unsigned short* pRootRot) const
{
    float fRealIndex;
    int nIndex;

    if (m_nNumRootKeys != 0)
    {
        if (fTime == 0.0f || m_nNumRootKeys == 1)
        {
            *pRootRot = m_pRootRot[m_nNumRootKeys - 1];
            return;
        }

        fRealIndex = fTime * (m_nNumRootKeys - 1);
        nIndex = (int)fRealIndex;
        unsigned short* pRoots = m_pRootRot;
        unsigned short val0 = pRoots[nIndex];
        s16 diff = (s16)(pRoots[nIndex + 1] - val0);
        *pRootRot = val0 + (int)((fRealIndex - (float)nIndex) * diff);
        return;
    }
    *pRootRot = 0;
}

/**
 * Offset/Address/Size: 0x1E0 | 0x801E93F4 | size: 0x10C
 * TODO: 99.00% match - first two word-copy loads are swapped and
 * interpolation setup keeps index in r5 instead of r7.
 */
// #pragma inline_depth(8)
void cSAnim::GetRootTrans(float t, nlVector3* out) const
{
    if (m_nNumRootKeys != 0)
    {
        if (t == 1.0f || m_nNumRootKeys == 1)
        {
            const nlVector3* pSrc = &m_pRootTrans[m_nNumRootKeys - 1];
            out->e[0] = pSrc->e[0];
            out->e[1] = pSrc->e[1];
            out->e[2] = pSrc->e[2];
            // *out = *(nlVector3*)&m_pRootTrans[m_nNumRootKeys - 1];
            return;
        }

        float fRealIndex = t * (m_nNumRootKeys - 1);
        int nIndex0 = (int)fRealIndex;
        int nIndex1 = nIndex0 + 1;
        const nlVector3* pRootTrans = m_pRootTrans;
        const nlVector3* pVal0 = &pRootTrans[nIndex0];
        const nlVector3* pVal1 = &pRootTrans[nIndex1];
        float fWeight = fRealIndex - nIndex0;
        float fInvWeight = 1.0f - fWeight;

        out->f.x = (fWeight * pVal1->f.x) + (fInvWeight * pVal0->f.x);
        out->f.y = (fWeight * pVal1->f.y) + (fInvWeight * pVal0->f.y);
        out->f.z = (fWeight * pVal1->f.z) + (fInvWeight * pVal0->f.z);

        return;
    }
    out->f.x = 0.0f;
    out->f.y = 0.0f;
    out->f.z = 0.0f;
}

/**
 * Offset/Address/Size: 0x160 | 0x801E9374 | size: 0x80
 */
#pragma inline_depth(0)
void cSAnim::CreateCallback(float time, unsigned int param1, void (*funcCallback)(unsigned int))
{
    cSAnimCallback* temp_r3;
    temp_r3 = (cSAnimCallback*)nlMalloc(0x10, 8, 0);

    if (temp_r3 != NULL)
    {
        temp_r3->m_fTime = time;
        temp_r3->m_nParam1 = param1;
        temp_r3->m_funcCallback = funcCallback;
    }

    nlListAddStart<cSAnimCallback>(&m_pCallbackList, temp_r3, NULL);
}

/**
 * Offset/Address/Size: 0x0 | 0x801E9214 | size: 0x160
 */
float cSAnim::GetMorphWeight(int channel, float fTime) const
{
    const u8* keys = m_pMorphKeys;
    int numKeys = m_pNumMorphKeys[channel];
    int i = 0;

    for (i = 0; i < channel; i++)
    {
        keys += m_pNumMorphKeys[channel];
    }

    if (numKeys == 1 || fTime == 1.0f)
    {
        float weight = (float)keys[numKeys - 1] / 255.0f;
        return weight;
    }

    float fRealIndex = fTime * (float)(numKeys - 1);
    int nIndex = (int)fRealIndex;
    float fWeightB = fRealIndex - (float)nIndex;
    return (1.0f - fWeightB) * ((float)keys[nIndex] / 255.0f) + fWeightB * ((float)keys[nIndex + 1] / 255.0f);
}
