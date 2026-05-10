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

    void operator=(const cPoseAccumulator&);

    /* 0x00 */ cSHierarchy* m_BaseSHierarchy;
    /* 0x04 */ Vector<nlMatrix4, DefaultAllocator> m_NodeMatrices;
    /* 0x10 */ Vector<nlMatrix4, DefaultAllocator> m_PrevNodeMatrices;
    /* 0x1C */ Vector<RotAccum, DefaultAllocator> m_rot;
    /* 0x28 */ Vector<ScaleAccum, DefaultAllocator> m_scale;
    /* 0x34 */ Vector<TransAccum, DefaultAllocator> m_trans;
    /* 0x40 */ Vector<cBuildNodeMatrixCallbackInfo, DefaultAllocator> m_cb;
    /* 0x4C */ Vector<float, DefaultAllocator> m_MorphWeights;
}; // total size: 0x58

inline void cPoseAccumulator::operator=(const cPoseAccumulator& other)
{
    if (this != &other)
    {
        this->~cPoseAccumulator();
        new (this) cPoseAccumulator(other);
    }
}

#endif // _POSEACCUMULATOR_H_
