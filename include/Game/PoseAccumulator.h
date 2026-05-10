#ifndef _POSEACCUMULATOR_H_
#define _POSEACCUMULATOR_H_

#include "NL/nlMath.h"
#include "NL/nlVector.h"

#include "Game/SHierarchy.h"
#include "Game/PoseNode.h"

// class cPoseAccumulator;

typedef void (*BuildNodeMatrixFn)(unsigned int, unsigned int, cPoseAccumulator*, unsigned int, int);

class cBuildNodeMatrixCallbackInfo
{
public:
    cBuildNodeMatrixCallbackInfo()
    {
        funcCallback = NULL;
    }

    /* 0x00 */ BuildNodeMatrixFn funcCallback;
    /* 0x04 */ unsigned int nParam1;
    /* 0x08 */ unsigned int nParam2;
}; // size: 0x0C

struct RotAccum
{
    /* 0x00 */ nlQuaternion q;
    /* 0x10 */ float quatAccumulatedWeight;
    /* 0x14 */ u16 rotAroundZ;
    /* 0x18 */ float rotAroundZAccumulatedWeight;
    /* 0x1C */ bool bIdentity;
}; // size: 0x20

struct ScaleAccum
{
    /* 0x00 */ nlVector3 s;
    /* 0x0C */ float fAccumulatedWeight;
    /* 0x10 */ bool bIdentity;
    // /* 0x00 */ float x;
    // /* 0x04 */ float y;
    // /* 0x08 */ float z;
    // /* 0x0C */ float weight;
    // /* 0x10 */ bool locked;
    // /* 0x11 */ u8 _pad[3];
}; // total size: 0x14

struct TransAccum
{

    // /* 0x00 */ float x;
    // /* 0x04 */ float y;
    // /* 0x08 */ float z;
    // /* 0x0C */ float weight;
    // /* 0x10 */ bool locked;
    // /* 0x11 */ u8 _pad[3];
    /* 0x00 */ nlVector3 t;              // offset 0x0, size 0xC
    /* 0x0C */ float fAccumulatedWeight; // offset 0xC, size 0x4
    /* 0x10 */ bool bIdentity;           // offset 0x10, size 0x1
}; // size: 0x14

class cPoseAccumulator
{
public:
    cPoseAccumulator(const cPoseAccumulator&);
    cPoseAccumulator(cSHierarchy* hierarchy, bool withSecondary);
    void Pose(const cPoseNode&, const nlMatrix4&);
    void InitAccumulators();
    void BuildNodeMatrices(const nlMatrix4&);
    void BlendRot(int, const nlQuaternion*, float, bool);
    void BlendRotAroundZ(int, unsigned short, float);
    void BlendScale(int, const nlVector3*, float, bool);
    void BlendTrans(int, const nlVector3*, float, bool);
    void BlendRotIdentity(int, float);
    void BlendScaleIdentity(int, float);
    void BlendTransIdentity(int, float);
    nlMatrix4& GetNodeMatrix(int) const;
    nlMatrix4& GetNodeMatrixByHashID(unsigned int) const;
    s32 GetNumNodes() const;
    void MultNodeMatrices(const nlMatrix4*);
    void SetBuildNodeMatrixCallback(int, BuildNodeMatrixFn, unsigned int, unsigned int);

    cPoseAccumulator& operator=(const cPoseAccumulator&);

    /* 0x00 */ cSHierarchy* m_BaseSHierarchy;
    /* 0x04 */ Vector<nlMatrix4, DefaultAllocator> m_NodeMatrices;
    /* 0x10 */ Vector<nlMatrix4, DefaultAllocator> m_PrevNodeMatrices;
    /* 0x1C */ Vector<RotAccum, DefaultAllocator> m_rot;
    /* 0x28 */ Vector<ScaleAccum, DefaultAllocator> m_scale;
    /* 0x34 */ Vector<TransAccum, DefaultAllocator> m_trans;
    /* 0x40 */ Vector<cBuildNodeMatrixCallbackInfo, DefaultAllocator> m_cb;
    /* 0x4C */ Vector<float, DefaultAllocator> m_MorphWeights;
}; // total size: 0x58

inline cPoseAccumulator& cPoseAccumulator::operator=(const cPoseAccumulator& other)
{
    m_BaseSHierarchy = other.m_BaseSHierarchy;

    if (m_NodeMatrices.mSize >= other.m_NodeMatrices.mSize)
    {
        for (int i = 0; i < other.m_NodeMatrices.mSize; i++)
        {
            m_NodeMatrices.mData[i] = other.m_NodeMatrices.mData[i];
        }
        m_NodeMatrices.mSize = other.m_NodeMatrices.mSize;
    }
    else
    {
        Vector<nlMatrix4, DefaultAllocator> tmp;
        tmp.mData = (nlMatrix4*)nlMalloc(other.m_NodeMatrices.mSize * sizeof(nlMatrix4), 8, false);
        tmp.mSize = other.m_NodeMatrices.mSize;
        tmp.mCapacity = other.m_NodeMatrices.mSize;

        for (int i = 0; i < other.m_NodeMatrices.mSize; i++)
        {
            tmp.mData[i] = other.m_NodeMatrices.mData[i];
        }

        int oldSize = m_NodeMatrices.mSize;
        int oldCapacity = m_NodeMatrices.mCapacity;
        nlMatrix4* oldData = m_NodeMatrices.mData;

        m_NodeMatrices.mData = tmp.mData;
        m_NodeMatrices.mSize = tmp.mSize;
        m_NodeMatrices.mCapacity = tmp.mCapacity;

        tmp.mData = oldData;
        tmp.mSize = oldSize;
        tmp.mCapacity = oldCapacity;
    }

    if (m_PrevNodeMatrices.mSize >= other.m_PrevNodeMatrices.mSize)
    {
        for (int i = 0; i < other.m_PrevNodeMatrices.mSize; i++)
        {
            m_PrevNodeMatrices.mData[i] = other.m_PrevNodeMatrices.mData[i];
        }
        m_PrevNodeMatrices.mSize = other.m_PrevNodeMatrices.mSize;
    }
    else
    {
        Vector<nlMatrix4, DefaultAllocator> tmp;
        tmp.mData = (nlMatrix4*)nlMalloc(other.m_PrevNodeMatrices.mSize * sizeof(nlMatrix4), 8, false);
        tmp.mSize = other.m_PrevNodeMatrices.mSize;
        tmp.mCapacity = other.m_PrevNodeMatrices.mSize;

        for (int i = 0; i < other.m_PrevNodeMatrices.mSize; i++)
        {
            tmp.mData[i] = other.m_PrevNodeMatrices.mData[i];
        }

        int oldSize = m_PrevNodeMatrices.mSize;
        int oldCapacity = m_PrevNodeMatrices.mCapacity;
        nlMatrix4* oldData = m_PrevNodeMatrices.mData;

        m_PrevNodeMatrices.mData = tmp.mData;
        m_PrevNodeMatrices.mSize = tmp.mSize;
        m_PrevNodeMatrices.mCapacity = tmp.mCapacity;

        tmp.mData = oldData;
        tmp.mSize = oldSize;
        tmp.mCapacity = oldCapacity;
    }

    if (m_rot.mSize >= other.m_rot.mSize)
    {
        for (int i = 0; i < other.m_rot.mSize; i++)
        {
            m_rot.mData[i] = other.m_rot.mData[i];
        }
        m_rot.mSize = other.m_rot.mSize;
    }
    else
    {
        Vector<RotAccum, DefaultAllocator> tmp;
        tmp.mData = (RotAccum*)nlMalloc(other.m_rot.mSize * sizeof(RotAccum), 8, false);
        tmp.mSize = other.m_rot.mSize;
        tmp.mCapacity = other.m_rot.mSize;

        for (int i = 0; i < other.m_rot.mSize; i++)
        {
            tmp.mData[i] = other.m_rot.mData[i];
        }

        int oldSize = m_rot.mSize;
        int oldCapacity = m_rot.mCapacity;
        RotAccum* oldData = m_rot.mData;

        m_rot.mData = tmp.mData;
        m_rot.mSize = tmp.mSize;
        m_rot.mCapacity = tmp.mCapacity;

        tmp.mData = oldData;
        tmp.mSize = oldSize;
        tmp.mCapacity = oldCapacity;
    }

    if (m_scale.mSize >= other.m_scale.mSize)
    {
        for (int i = 0; i < other.m_scale.mSize; i++)
        {
            m_scale.mData[i] = other.m_scale.mData[i];
        }
        m_scale.mSize = other.m_scale.mSize;
    }
    else
    {
        Vector<ScaleAccum, DefaultAllocator> tmp;
        tmp.mData = (ScaleAccum*)nlMalloc(other.m_scale.mSize * sizeof(ScaleAccum), 8, false);
        tmp.mSize = other.m_scale.mSize;
        tmp.mCapacity = other.m_scale.mSize;

        for (int i = 0; i < other.m_scale.mSize; i++)
        {
            tmp.mData[i] = other.m_scale.mData[i];
        }

        int oldSize = m_scale.mSize;
        int oldCapacity = m_scale.mCapacity;
        ScaleAccum* oldData = m_scale.mData;

        m_scale.mData = tmp.mData;
        m_scale.mSize = tmp.mSize;
        m_scale.mCapacity = tmp.mCapacity;

        tmp.mData = oldData;
        tmp.mSize = oldSize;
        tmp.mCapacity = oldCapacity;
    }

    if (m_trans.mSize >= other.m_trans.mSize)
    {
        for (int i = 0; i < other.m_trans.mSize; i++)
        {
            m_trans.mData[i] = other.m_trans.mData[i];
        }
        m_trans.mSize = other.m_trans.mSize;
    }
    else
    {
        Vector<TransAccum, DefaultAllocator> tmp;
        tmp.mData = (TransAccum*)nlMalloc(other.m_trans.mSize * sizeof(TransAccum), 8, false);
        tmp.mSize = other.m_trans.mSize;
        tmp.mCapacity = other.m_trans.mSize;

        for (int i = 0; i < other.m_trans.mSize; i++)
        {
            tmp.mData[i] = other.m_trans.mData[i];
        }

        int oldSize = m_trans.mSize;
        int oldCapacity = m_trans.mCapacity;
        TransAccum* oldData = m_trans.mData;

        m_trans.mData = tmp.mData;
        m_trans.mSize = tmp.mSize;
        m_trans.mCapacity = tmp.mCapacity;

        tmp.mData = oldData;
        tmp.mSize = oldSize;
        tmp.mCapacity = oldCapacity;
    }

    if (m_cb.mSize >= other.m_cb.mSize)
    {
        for (int i = 0; i < other.m_cb.mSize; i++)
        {
            m_cb.mData[i] = other.m_cb.mData[i];
        }
        m_cb.mSize = other.m_cb.mSize;
    }
    else
    {
        int n = other.m_cb.mSize;
        Vector<cBuildNodeMatrixCallbackInfo, DefaultAllocator> tmp;
        tmp.mData = new (nlMalloc(n * sizeof(cBuildNodeMatrixCallbackInfo) + 0x10, 8, false)) cBuildNodeMatrixCallbackInfo[n];
        tmp.mSize = n;
        tmp.mCapacity = n;

        for (int i = 0; i < other.m_cb.mSize; i++)
        {
            tmp.mData[i] = other.m_cb.mData[i];
        }

        int oldSize = m_cb.mSize;
        int oldCapacity = m_cb.mCapacity;
        cBuildNodeMatrixCallbackInfo* oldData = m_cb.mData;

        m_cb.mData = tmp.mData;
        m_cb.mSize = tmp.mSize;
        m_cb.mCapacity = tmp.mCapacity;

        tmp.mData = oldData;
        tmp.mSize = oldSize;
        tmp.mCapacity = oldCapacity;
    }

    if (m_MorphWeights.mSize >= other.m_MorphWeights.mSize)
    {
        for (int i = 0; i < other.m_MorphWeights.mSize; i++)
        {
            m_MorphWeights.mData[i] = other.m_MorphWeights.mData[i];
        }
        m_MorphWeights.mSize = other.m_MorphWeights.mSize;
    }
    else
    {
        Vector<float, DefaultAllocator> tmp;
        tmp.mData = (float*)nlMalloc(other.m_MorphWeights.mSize * sizeof(float), 8, false);
        tmp.mSize = other.m_MorphWeights.mSize;
        tmp.mCapacity = other.m_MorphWeights.mSize;

        for (int i = 0; i < other.m_MorphWeights.mSize; i++)
        {
            tmp.mData[i] = other.m_MorphWeights.mData[i];
        }

        int oldSize = m_MorphWeights.mSize;
        int oldCapacity = m_MorphWeights.mCapacity;
        float* oldData = m_MorphWeights.mData;

        m_MorphWeights.mData = tmp.mData;
        m_MorphWeights.mSize = tmp.mSize;
        m_MorphWeights.mCapacity = tmp.mCapacity;

        tmp.mData = oldData;
        tmp.mSize = oldSize;
        tmp.mCapacity = oldCapacity;
    }

    return *this;
}

#endif // _POSEACCUMULATOR_H_
