#ifndef _SANIM_H_
#define _SANIM_H_

#include "types.h"

#include "NL/nlMath.h"
// #include "NL/nlList.h"

class cPoseAccumulator;

struct PackedScale
{
    signed short x; // offset 0x0, size 0x2
    signed short y; // offset 0x2, size 0x2
    signed short z; // offset 0x4, size 0x2
}; // total size: 0x6

struct PackedTrans
{
    float x; // offset 0x0, size 0x4
    float y; // offset 0x4, size 0x4
    float z; // offset 0x8, size 0x4
}; // total size: 0xC

enum ePlayMode
{
    PM_CYCLIC = 0,
    PM_HOLD = 1,
};

// typedef struct cSAnimCallback
// {
//     /* 0x00 */ f32 time;
//     /* 0x04 */ u32 mask;
//     /* 0x08 */ void (*fn)(unsigned int);
//     /* 0x0C */ cSAnimCallback* next;
// } cSAnimCallback; // size: 0x10

class cSAnimCallback
{
public:
    /* 0x0 */ float m_fTime;
    /* 0x4 */ unsigned int m_nParam1;
    /* 0x8 */ void (*m_funcCallback)(unsigned int);
    /* 0xC */ class cSAnimCallback* next;
}; // total size: 0x10

class nlChunk
{
public:
    /* 0x00 */ u32 m_ID;
    /* 0x04 */ u32 m_Size;
}; // size: 0x8

class cIdentifier
{
public:
    /* 0x0 */ const char* m_szName;
    /* 0x4 */ unsigned int m_uHashID;
}; // total size: 0x8

class cSAnim : public cIdentifier
{
public:
    static cSAnim* Initialize(nlChunk*);

    void BlendRot(int, int, float, float, cPoseAccumulator*, bool) const;
    void BlendScale(int, int, float, float, cPoseAccumulator*, bool) const;
    void BlendTrans(int, int, float, float, cPoseAccumulator*, bool) const;
    void Destroy();
    void GetRootRot(float, unsigned short*) const;
    void GetRootTrans(float, nlVector3*) const;
    void CreateCallback(float time, unsigned int param1, void (*funcCallback)(unsigned int));
    float GetMorphWeight(int index, float t) const;

    cSAnimCallback* GetCallbackList() const;

    /* 0x08 */ unsigned int m_nNumKeys;
    /* 0x0C */ unsigned int m_nNumNodes;
    /* 0x10 */ unsigned int m_nNumMorphChannels;
    /* 0x14 */ const unsigned int* m_pNodeProperties;
    /* 0x18 */ void* m_pRotKeys;
    /* 0x1C */ PackedScale** m_pScaleKeys;
    /* 0x20 */ PackedTrans** m_pTransKeys;
    /* 0x24 */ unsigned int m_nNumRootKeys;
    /* 0x28 */ unsigned short* m_pRootRot;
    /* 0x2C */ nlVector3* m_pRootTrans;
    /* 0x30 */ unsigned long* m_nMorphIds;
    /* 0x34 */ const unsigned int* m_pNumMorphKeys;
    /* 0x38 */ unsigned char* m_pMorphKeys;
    /* 0x3C */ cSAnimCallback* m_pCallbackList;
    /* 0x40 */ float m_fLinearSpeed;
    /* 0x44 */ unsigned long m_nHierarchySignature;
}; // total size: 0x48

#endif // _SANIM_H_
