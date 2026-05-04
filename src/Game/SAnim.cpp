#include "Game/SAnim.h"
#include "Game/PoseAccumulator.h"

#include "NL/nlMemory.h"
#include "NL/nlList.h"

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
 * Offset/Address/Size: 0xD40 | 0x801E9F54 | size: 0x68C
 * TODO: 83.58% match - 40 register allocation diffs in inlined nlGetChunkData expansions
 * (r3/r5 swap for alignment, r5/r7 swap for chunk pointer). Known -inline deferred
 * scratch limitation - register allocator sees different context vs full TU.
 */
// #pragma inline_depth(smart)
cSAnim* cSAnim::Initialize(nlChunk* pChunk)
{
    nlChunk* chunk = (nlChunk*)((u8*)pChunk + 8);
    nlChunk* end = nlGetNextChunk(pChunk);

    cSAnim* pRetval = (cSAnim*)nlGetChunkData(chunk);
    pRetval->m_pCallbackList = NULL;

    chunk = nlGetNextChunk(chunk);
    pRetval->m_szName = (const char*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    pRetval->m_pRotKeys = nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    pRetval->m_pTransKeys = (PackedTrans**)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    pRetval->m_pScaleKeys = (PackedScale**)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    pRetval->m_pRootRot = (unsigned short*)nlGetChunkData(chunk);

    chunk = nlGetNextChunk(chunk);
    pRetval->m_pRootTrans = (nlVector3*)nlGetChunkData(chunk);

    u32 nodeIndex = 0;
    nlChunk* nodeChunk = nlGetNextChunk(chunk);

    while (nodeChunk != end)
    {
        u32 type = nodeChunk->m_ID & 0x80FFFFFF;
        if (type != 0x80017100 && type != 0x1001)
            break;

        if (type == 0x80017100)
        {
            nlChunk* subChunk = (nlChunk*)((u8*)nodeChunk + 8);
            nlChunk* subEnd = nlGetNextChunk(nodeChunk);

            while (subChunk != subEnd)
            {
                u32 subType = subChunk->m_ID & 0x80FFFFFF;
                if (subType == 0x17101)
                {
                    ((void**)pRetval->m_pRotKeys)[nodeIndex] = nlGetChunkData(subChunk);
                }
                else if (subType == 0x17102)
                {
                    pRetval->m_pTransKeys[nodeIndex] = (PackedTrans*)nlGetChunkData(subChunk);
                }
                else if (subType == 0x17103)
                {
                    pRetval->m_pScaleKeys[nodeIndex] = (PackedScale*)nlGetChunkData(subChunk);
                }
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

        if (numRootKeys == 0)
        {
            v3PosStart.f.x = 0.0f;
            v3PosStart.f.y = 0.0f;
            v3PosStart.f.z = 0.0f;
        }
        else if (numRootKeys == 1)
        {
            v3PosStart = rootTrans[numRootKeys - 1];
        }
        else
        {
            float fRealIndex = 0.0f * (numRootKeys - 1);
            int nIndex = (int)fRealIndex;
            float fFrac = fRealIndex - (float)nIndex;
            float fInvFrac = 1.0f - fFrac;
            nlVector3* pVal0 = &rootTrans[nIndex];
            nlVector3* pVal1 = &rootTrans[nIndex + 1];
            v3PosStart.f.x = fFrac * pVal1->f.x + fInvFrac * pVal0->f.x;
            v3PosStart.f.y = fFrac * pVal1->f.y + fInvFrac * pVal0->f.y;
            v3PosStart.f.z = fFrac * pVal1->f.z + fInvFrac * pVal0->f.z;
        }

        if (numRootKeys == 0)
        {
            v3PosEnd.f.x = 0.0f;
            v3PosEnd.f.y = 0.0f;
            v3PosEnd.f.z = 0.0f;
        }
        else
        {
            v3PosEnd = rootTrans[numRootKeys - 1];
        }

        float dist = nlSqrt(
            (v3PosEnd.f.y - v3PosStart.f.y) * (v3PosEnd.f.y - v3PosStart.f.y) + (v3PosEnd.f.x - v3PosStart.f.x) * (v3PosEnd.f.x - v3PosStart.f.x) + (v3PosEnd.f.z - v3PosStart.f.z) * (v3PosEnd.f.z - v3PosStart.f.z),
            true);

        pRetval->m_fLinearSpeed = dist / ((float)pRetval->m_nNumKeys / 30.0f);
    }
    else
    {
        pRetval->m_fLinearSpeed = 0.0f;
    }

    pRetval->m_pNumMorphKeys = (const unsigned int*)nlGetChunkData(nodeChunk);

    nodeChunk = nlGetNextChunk(nodeChunk);
    pRetval->m_nMorphIds = (unsigned long*)nlGetChunkData(nodeChunk);

    nodeChunk = nlGetNextChunk(nodeChunk);
    pRetval->m_pMorphKeys = (unsigned char*)nlGetChunkData(nodeChunk);

    nodeChunk = nlGetNextChunk(nodeChunk);
    pRetval->m_pNodeProperties = (const unsigned int*)nlGetChunkData(nodeChunk);

    return pRetval;
}
// #pragma inline_depth()

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
 * TODO: 98.66% match - remaining interpolation block register allocation differs
 * (index in r5 vs r7 and val0/val1 base register assignment swap).
 */
void cSAnim::GetRootTrans(float t, nlVector3* out) const
{
    if (m_nNumRootKeys != 0)
    {
        if (t == 1.0f || m_nNumRootKeys == 1)
        {
            *out = m_pRootTrans[m_nNumRootKeys - 1];
            return;
        }

        float fRealIndex = t * (m_nNumRootKeys - 1);
        int nIndex = (int)fRealIndex;
        float fWeight = fRealIndex - nIndex;
        float fInvWeight = 1.0f - fWeight;
        const nlVector3* pRootTrans = m_pRootTrans;
        const nlVector3* pVal0 = &pRootTrans[nIndex];
        const nlVector3* pVal1 = &pRootTrans[nIndex + 1];

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
