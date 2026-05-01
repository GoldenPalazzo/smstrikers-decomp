#ifndef _PNFEATHER_H_
#define _PNFEATHER_H_

#include "Game/SHierarchy.h"
#include "Game/PoseAccumulator.h"
#include "NL/nlSlotPool.h"

enum eFeatherBlendMode
{
    eFEATHER_BLEND_IN = 0,
    eFEATHER_BLEND_OUT = 1,
};

class cPN_Feather : public cPoseNode
{
public:
    cPN_Feather() { }
    cPN_Feather(cSHierarchy*, void (*)(unsigned int, cPN_Feather*), unsigned int);
    /* 0x08 */ virtual ~cPN_Feather();
    /* 0x14 */ virtual void Evaluate(int, float, cPoseAccumulator*) const;
    /* 0x10 */ virtual void Evaluate(float, cPoseAccumulator*) const;
    /* 0x18 */ virtual cPoseNode* Update(float);
    /* 0x1C */ virtual int GetType() { return 0x1; };
    /* 0x20 */ virtual void BlendRootTrans(nlVector3*, float, float*);
    /* 0x24 */ virtual void BlendRootRot(unsigned short*, float, float*);

    template <typename T>
    void Replay(T& frame);

    void ClearNodeWeights();
    void SetNodeWeight(int, float, float);
    void SetChildFeatherWeight(int, float);
    void SetNodeWeight(int, float);
    void BeginBlendIn(float);
    void BeginBlendOut(float);

    inline void operator delete(void* ptr);

    /* 0x14 */ float* m_pFeatherWeights;
    /* 0x18 */ float m_fBlendTime;
    /* 0x1C */ void (*m_fWeightTableCallback)(unsigned int, class cPN_Feather*);
    /* 0x20 */ unsigned int m_nCallbackParam1;
    /* 0x24 */ float m_fBlendDuration;
    /* 0x28 */ cSHierarchy* m_pBaseHierarchy;
    /* 0x2C */ eFeatherBlendMode m_eFeatherBlendMode;

    static SlotPool<cPN_Feather> m_FeatherSlotPool;
}; // total size: 0x30

// class SlotPool<cPN_Feather>
// {
// public:
//     void ~SlotPool();
// };

inline cPN_Feather* AllocateFeather()
{
    cPN_Feather* feather = nullptr;

    if (cPN_Feather::m_FeatherSlotPool.m_FreeList == nullptr)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_Feather::m_FeatherSlotPool, sizeof(cPN_Feather));
    }

    if (cPN_Feather::m_FeatherSlotPool.m_FreeList != nullptr)
    {
        feather = (cPN_Feather*)cPN_Feather::m_FeatherSlotPool.m_FreeList;
        cPN_Feather::m_FeatherSlotPool.m_FreeList = cPN_Feather::m_FeatherSlotPool.m_FreeList->m_next;
    }

    return feather;
}

#endif // _PNFEATHER_H_
