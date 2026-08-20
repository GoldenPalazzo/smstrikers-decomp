#include "Game/PoseAccumulator.h"
#include "NL/nlMemory.h"

#include "math.h"
#include "types.h"

static nlQuaternion qRotIdentity = { 0.0f, 0.0f, 0.0f, 1.0f };
static nlVector3 v3ScaleIdentity = { 1.0f, 1.0f, 1.0f };
static nlVector3 v3TransIdentity = { 0.0f, 0.0f, 0.0f };

cPoseAccumulator::cPoseAccumulator(cSHierarchy* pSHierarchy, bool bStorePrevNodeMatrices)
    : m_BaseSHierarchy(pSHierarchy)
    , m_NodeMatrices(pSHierarchy->m_nodeCount + 1, 0)
    , m_PrevNodeMatrices(bStorePrevNodeMatrices ? pSHierarchy->m_nodeCount + 1 : 0, 0)
    , m_rot(pSHierarchy->m_nodeCount, 0)
    , m_scale(pSHierarchy->m_nodeCount, 0)
    , m_trans(pSHierarchy->m_nodeCount, 0)
    , m_cb(pSHierarchy->m_nodeCount, 0)
    , m_MorphWeights(8, 0)
{
    FORCE_DONT_INLINE;

    int i;

    for (i = 0; i < m_BaseSHierarchy->m_nodeCount; ++i)
    {
        if (m_BaseSHierarchy->PreserveBoneLength(i))
        {
            const nlVector3* t = m_BaseSHierarchy->GetTranslationOffset(i);
            m_trans.mData[i].t = *t;
            m_trans.mData[i].fAccumulatedWeight = 1.0f;
            m_trans.mData[i].bIdentity = false;
        }
    }
}

static inline void PoseAccumulatorInitNodeAccumulators(cPoseAccumulator* pose)
{
    int i;
    for (i = 0; i < pose->m_BaseSHierarchy->m_nodeCount; i++)
    {
        RotAccum& r = pose->m_rot.mData[i];
        r.q.x = 0.0f;
        r.q.y = 0.0f;
        r.q.z = 0.0f;
        r.q.w = 1.0f;
        r.quatAccumulatedWeight = 0.0f;
        r.rotAroundZ = 0;
        r.rotAroundZAccumulatedWeight = 0.0f;
        r.bIdentity = true;

        ScaleAccum& s = pose->m_scale.mData[i];
        s.s.x = 1.0f;
        s.s.y = 1.0f;
        s.s.z = 1.0f;
        s.fAccumulatedWeight = 0.0f;
        s.bIdentity = true;

        if (!pose->m_BaseSHierarchy->PreserveBoneLength(i))
        {
            TransAccum& t = pose->m_trans.mData[i];
            t.t.x = 0.0f;
            t.t.y = 0.0f;
            t.t.z = 0.0f;
            t.fAccumulatedWeight = 0.0f;
            t.bIdentity = true;
        }
    }
}

static inline void PoseAccumulatorClearMorphWeights(cPoseAccumulator* pose)
{
    int i;
    for (i = 0; i < pose->m_MorphWeights.mSize; i++)
    {
        pose->m_MorphWeights.mData[i] = 0.0f;
    }
}

/**
 * Offset/Address/Size: 0xB7C | 0x801EC11C | size: 0x15C
 */
void cPoseAccumulator::Pose(const cPoseNode& node, const nlMatrix4& mat)
{
    PoseAccumulatorInitNodeAccumulators(this);
    PoseAccumulatorClearMorphWeights(this);

    node.Evaluate(1.0f, this);

    BuildNodeMatrices(mat);
}

/**
 * Offset/Address/Size: 0xA38 | 0x801EBFD8 | size: 0x144
 */
void cPoseAccumulator::InitAccumulators()
{
    for (int i = 0; i < m_BaseSHierarchy->m_nodeCount; ++i)
    {
        RotAccum& r = m_rot.mData[i];
        r.q.x = 0.0f;
        r.q.y = 0.0f;
        r.q.z = 0.0f;
        r.q.w = 1.0f;
        r.quatAccumulatedWeight = 0.0f;
        r.rotAroundZ = 0;
        r.rotAroundZAccumulatedWeight = 0.0f;
        r.bIdentity = true;

        ScaleAccum& s = m_scale.mData[i];
        s.s.x = 1.0f;
        s.s.y = 1.0f;
        s.s.z = 1.0f;
        s.fAccumulatedWeight = 0.0f;
        s.bIdentity = true;

        if (!m_BaseSHierarchy->PreserveBoneLength(i))
        {
            TransAccum& t = m_trans.mData[i];
            t.t.x = 0.0f;
            t.t.y = 0.0f;
            t.t.z = 0.0f;
            t.fAccumulatedWeight = 0.0f;
            t.bIdentity = true;
        }
    }

    for (int k = 0; k < m_MorphWeights.mSize; ++k)
    {
        m_MorphWeights.mData[k] = 0.0f;
    }
}

/**
 * Offset/Address/Size: 0x644 | 0x801EBBE4 | size: 0x3F4
 */
void cPoseAccumulator::BuildNodeMatrices(const nlMatrix4& world)
{
    if (m_PrevNodeMatrices.mSize == m_NodeMatrices.mSize)
    {
        s32 tmp = m_PrevNodeMatrices.mSize;
        m_PrevNodeMatrices.mSize = m_NodeMatrices.mSize;
        m_NodeMatrices.mSize = tmp;
        tmp = m_PrevNodeMatrices.mCapacity;
        m_PrevNodeMatrices.mCapacity = m_NodeMatrices.mCapacity;
        m_NodeMatrices.mCapacity = tmp;
        nlMatrix4* tmp_mat = m_PrevNodeMatrices.mData;
        m_PrevNodeMatrices.mData = m_NodeMatrices.mData;
        m_NodeMatrices.mData = tmp_mat;
    }
    nlMatrix4* pLocalMatrix;
    int ParentStack[32];
    int nStackIndex = -1;
    for (int i = 0; i < m_BaseSHierarchy->m_nodeCount; i++)
    {
        pLocalMatrix = &GetNodeMatrix(i + 1);
        if (!m_rot.mData[i].bIdentity)
        {
            if (m_rot.mData[i].quatAccumulatedWeight == 0.0f)
                nlMakeRotationMatrixZ(*pLocalMatrix, 0.0000958738f * m_rot.mData[i].rotAroundZ);
            else
            {
                if (m_rot.mData[i].rotAroundZAccumulatedWeight != 0.0f)
                {
                    float sin, cos;
                    nlSinCos(&sin, &cos, (u16)((u32)m_rot.mData[i].rotAroundZ >> 1));
                    nlQuaternion quatAroundZ;
                    quatAroundZ.x = 0.0f;
                    quatAroundZ.y = 0.0f;
                    quatAroundZ.z = sin;
                    quatAroundZ.w = cos;
                    float t = m_rot.mData[i].rotAroundZAccumulatedWeight / (m_rot.mData[i].rotAroundZAccumulatedWeight + m_rot.mData[i].quatAccumulatedWeight);
                    nlQuatNLerp(m_rot.mData[i].q, m_rot.mData[i].q, quatAroundZ, t);
                }
                nlQuatToMatrix(*pLocalMatrix, m_rot.mData[i].q);
            }
        }
        else
            pLocalMatrix->SetIdentity();

        if (!m_trans.mData[i].bIdentity)
        {
            pLocalMatrix->m41 = m_trans.mData[i].t.x;
            pLocalMatrix->m42 = m_trans.mData[i].t.y;
            pLocalMatrix->m43 = m_trans.mData[i].t.z;
        }

        if (i > 0)
        {
            int parentIdx = ParentStack[nStackIndex];
            pLocalMatrix->m41 *= m_scale.mData[parentIdx].s.x;
            pLocalMatrix->m42 *= m_scale.mData[parentIdx].s.y;
            pLocalMatrix->m43 *= m_scale.mData[parentIdx].s.z;
        }

        int nParentIndex = -1;
        if (i > 0)
        {
            nParentIndex = ParentStack[nStackIndex];
            nlMatrix4* pNodeMatrices;
            nlMatrix4* pParentMatrix = (pNodeMatrices = m_NodeMatrices.mData) + nParentIndex;
            nlMultMatrices(pNodeMatrices[i], *pLocalMatrix, *pParentMatrix);
        }
        else
            nlMultMatrices(m_NodeMatrices.mData[i], *pLocalMatrix, world);
        int nPushPop = m_BaseSHierarchy->GetPushPop(i);
        nStackIndex += nPushPop;

        if (nPushPop > 0)
            ParentStack[nStackIndex] = i;

        cBuildNodeMatrixCallbackInfo* pCallback = &m_cb.mData[i];

        if (pCallback->funcCallback)
            pCallback->funcCallback(pCallback->nParam1, pCallback->nParam2, this, i, nParentIndex);
    }

    for (int i = 0; i < m_BaseSHierarchy->m_nodeCount; i++)
    {
        if (!m_scale.mData[i].bIdentity)
        {
            m_NodeMatrices.mData[i].m11 *= m_scale.mData[i].s.x;
            m_NodeMatrices.mData[i].m12 *= m_scale.mData[i].s.x;
            m_NodeMatrices.mData[i].m13 *= m_scale.mData[i].s.x;
            m_NodeMatrices.mData[i].m21 *= m_scale.mData[i].s.y;
            m_NodeMatrices.mData[i].m22 *= m_scale.mData[i].s.y;
            m_NodeMatrices.mData[i].m23 *= m_scale.mData[i].s.y;
            m_NodeMatrices.mData[i].m31 *= m_scale.mData[i].s.z;
            m_NodeMatrices.mData[i].m32 *= m_scale.mData[i].s.z;
            m_NodeMatrices.mData[i].m33 *= m_scale.mData[i].s.z;
        }
    }
}

/**
 * Offset/Address/Size: 0x4FC | 0x801EBA9C | size: 0x148
 */
void cPoseAccumulator::BlendRot(int idx, const nlQuaternion* q, float w, bool flip)
{
    RotAccum* e = m_rot.mData + idx;

    if ((float)fabsf(w) < 0.001f)
        return;

    nlQuaternion qtemp;

    if (flip)
    {
        cSHierarchy* h = m_BaseSHierarchy;

        if (idx == h->m_nSpineNodeIndex || idx == h->m_nPelvisNodeIndex)
        {
            qtemp.x = -q->w;
            qtemp.y = q->z;
            qtemp.z = q->y;
            qtemp.w = -q->x;
        }
        else if (idx < h->m_nPelvisNodeIndex)
        {
            qtemp.x = -q->x;
            qtemp.y = q->y;
            qtemp.z = -q->z;
            qtemp.w = q->w;
        }
        else
        {
            qtemp.x = -q->x;
            qtemp.y = -q->y;
            qtemp.z = q->z;
            qtemp.w = q->w;
        }

        q = &qtemp;
    }

    e->quatAccumulatedWeight += w;

    float t = w / e->quatAccumulatedWeight;

    nlQuaternion tmp = e->q;
    nlQuatNLerp(e->q, tmp, *q, t);

    e = m_rot.mData + idx;
    e->bIdentity = false;
}

static inline RotAccum* GetRotAccum(RotAccum* data, int idx)
{
    return data + idx;
}

/**
 * Offset/Address/Size: 0x468 | 0x801EBA08 | size: 0x94
 */
void cPoseAccumulator::BlendRotAroundZ(int idx, unsigned short angle, float w)
{
    if (fabsf(w) < 0.001f)
        return;

    RotAccum* e = GetRotAccum(m_rot.mData, idx);

    e->rotAroundZAccumulatedWeight += w;
    float t = w / e->rotAroundZAccumulatedWeight;

    short delta = (short)(angle - e->rotAroundZ);
    delta = (short)(t * delta);

    e->rotAroundZ = e->rotAroundZ + (short)delta;

    e = GetRotAccum(m_rot.mData, idx);
    e->bIdentity = false;
}

static inline ScaleAccum* GetScaleAccum(ScaleAccum* data, int idx)
{
    return data + idx;
}

static inline TransAccum* GetTransAccum(TransAccum* data, int idx)
{
    return data + idx;
}

/**
 * Offset/Address/Size: 0x3DC | 0x801EB97C | size: 0x8C
 */
void cPoseAccumulator::BlendScale(int idx, const nlVector3* v, float w, bool)
{
    if (fabsf(w) < 0.001f)
        return;

    ScaleAccum* e = GetScaleAccum(m_scale.mData, idx);
    e->fAccumulatedWeight += w;

    float t = w / e->fAccumulatedWeight;
    float inv = 1.0f - t;

    e->s.x = inv * e->s.x + t * v->x;
    e->s.y = inv * e->s.y + t * v->y;
    e->s.z = inv * e->s.z + t * v->z;

    e = GetScaleAccum(m_scale.mData, idx);
    e->bIdentity = false;
}

/**
 * Offset/Address/Size: 0x2E4 | 0x801EB884 | size: 0xF8
 */
void cPoseAccumulator::BlendTrans(int idx, const nlVector3* v, float w, bool flip)
{
    if (fabsf(w) < 0.001f)
        return;

    if (flip)
    {
        cSHierarchy* h = m_BaseSHierarchy;

        nlVector3 vtemp;
        if (idx <= h->m_nPelvisNodeIndex || idx == h->m_nSpineNodeIndex)
        {
            vtemp.x = v->x;
            vtemp.y = -v->y;
            vtemp.z = v->z;
        }
        else
        {
            vtemp.x = v->x;
            vtemp.y = v->y;
            vtemp.z = -v->z;
        }

        v = &vtemp;
    }

    TransAccum* e = GetTransAccum(m_trans.mData, idx);
    e->fAccumulatedWeight += w;

    float t = w / e->fAccumulatedWeight;
    float inv = 1.0f - t;

    e->t.x = inv * e->t.x + t * v->x;
    e->t.y = inv * e->t.y + t * v->y;
    e->t.z = inv * e->t.z + t * v->z;

    e = GetTransAccum(m_trans.mData, idx);
    e->bIdentity = false;
}

/**
 * Offset/Address/Size: 0x258 | 0x801EB7F8 | size: 0x8C
 */
void cPoseAccumulator::BlendRotIdentity(int idx, float w)
{
    if (fabsf(w) < 0.001f)
        return;

    RotAccum* a = &m_rot.mData[idx];
    a->quatAccumulatedWeight += w;

    if (a->bIdentity)
        return;

    const float t = w / a->quatAccumulatedWeight;

    nlQuaternion tmp = a->q;
    nlQuatNLerp(a->q, tmp, qRotIdentity, t);
}

/**
 * Offset/Address/Size: 0x1CC | 0x801EB76C | size: 0x8C
 */
void cPoseAccumulator::BlendScaleIdentity(int idx, float w)
{
    if (fabsf(w) < 0.001f)
        return;

    ScaleAccum* e = &m_scale.mData[idx];
    e->fAccumulatedWeight += w;

    if (e->bIdentity)
        return;

    float f3 = w / e->fAccumulatedWeight;
    float f2 = 1.0f - f3;

    e->s.x = f2 * e->s.x + f3 * v3ScaleIdentity.x;
    e->s.y = f2 * e->s.y + f3 * v3ScaleIdentity.y;
    e->s.z = f2 * e->s.z + f3 * v3ScaleIdentity.z;
}

/**
 * Offset/Address/Size: 0x140 | 0x801EB6E0 | size: 0x8C
 */
void cPoseAccumulator::BlendTransIdentity(int idx, float w)
{
    if (fabsf(w) < 0.001f)
        return;

    TransAccum* e = &m_trans.mData[idx];
    e->fAccumulatedWeight += w;

    if (e->bIdentity)
        return;

    const float f3 = w / e->fAccumulatedWeight;
    const float f2 = 1.0f - f3;

    e->t.x = f2 * e->t.x + f3 * v3TransIdentity.x;
    e->t.y = f2 * e->t.y + f3 * v3TransIdentity.y;
    e->t.z = f2 * e->t.z + f3 * v3TransIdentity.z;
}

/**
 * Offset/Address/Size: 0x130 | 0x801EB6D0 | size: 0x10
 */
nlMatrix4& cPoseAccumulator::GetNodeMatrix(int i) const
{
    return m_NodeMatrices.mData[i];
}

/**
 * Offset/Address/Size: 0xB0 | 0x801EB650 | size: 0x80
 */
nlMatrix4& cPoseAccumulator::GetNodeMatrixByHashID(unsigned int hash) const
{
    cSHierarchy* hierarchy = m_BaseSHierarchy; // r3->0x00
    int index = 0;                             // r30 = 0

    while (index < hierarchy->m_nodeCount)
    {
        unsigned int nodeID = hierarchy->GetNodeID(index);
        if (hash == nodeID)
        {
            break;
        }
        index++;
    }

    return m_NodeMatrices.mData[index];
}

/**
 * Offset/Address/Size: 0xA4 | 0x801EB644 | size: 0xC
 */
s32 cPoseAccumulator::GetNumNodes() const
{
    return m_BaseSHierarchy->m_nodeCount;
}

/**
 * Offset/Address/Size: 0x28 | 0x801EB5C8 | size: 0x7C
 */
void cPoseAccumulator::MultNodeMatrices(const nlMatrix4* arg0)
{
    for (int i = 0; i < m_BaseSHierarchy->m_nodeCount; i++)
    {
        nlMatrix4* temp_r3 = &m_NodeMatrices.mData[i];
        nlMultMatrices(*temp_r3, *temp_r3, *arg0);
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801EB5A0 | size: 0x28
 */
void cPoseAccumulator::SetBuildNodeMatrixCallback(int idx, BuildNodeMatrixFn fn, unsigned int a, unsigned int b)
{
    int offset = idx * (int)sizeof(cBuildNodeMatrixCallbackInfo);
    *(BuildNodeMatrixFn*)((char*)m_cb.mData + offset) = fn;
    *(unsigned int*)((char*)m_cb.mData + offset + 4) = a;
    *(unsigned int*)((char*)m_cb.mData + offset + 8) = b;
}
