#ifndef _PNSINGLEAXISBLENDER_H_
#define _PNSINGLEAXISBLENDER_H_

#include "Game/PoseAccumulator.h"
#include "NL/nlMath.h"
#include "NL/nlSlotPool.h"

class cPN_SingleAxisBlender : public cPoseNode
{
public:
    cPN_SingleAxisBlender() { }
    cPN_SingleAxisBlender(int, void (*)(unsigned int, cPN_SingleAxisBlender*), unsigned int, float);
    /* 0x08 */ virtual ~cPN_SingleAxisBlender() { };
    static void operator delete(void* ptr);
    /* 0x14 */ virtual void Evaluate(int, float, cPoseAccumulator*) const;
    /* 0x10 */ virtual void Evaluate(float, cPoseAccumulator*) const;
    /* 0x18 */ virtual cPoseNode* Update(float);
    /* 0x1C */ virtual int GetType() { return 0x3; };
    /* 0x20 */ virtual void BlendRootTrans(nlVector3*, float, float*);
    /* 0x24 */ virtual void BlendRootRot(unsigned short*, float, float*);

    template <typename T>
    void Replay(T&);

    /* 0x14 */ float m_fSmoothedWeight;
    /* 0x18 */ void (*m_fWeightCallback)(unsigned int, class cPN_SingleAxisBlender*);
    /* 0x1C */ unsigned int m_nCallbackParam1;
    /* 0x20 */ float m_fDesiredWeight;
    /* 0x24 */ float m_fWeightSeek;

    static SlotPool<cPN_SingleAxisBlender> m_SingleAxisBlenderSlotPool;
}; // total size: 0x28

inline cPN_SingleAxisBlender* AllocateSingleAxisBlender()
{
    cPN_SingleAxisBlender* pSAB = NULL;

    if (cPN_SingleAxisBlender::m_SingleAxisBlenderSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SingleAxisBlender::m_SingleAxisBlenderSlotPool, sizeof(cPN_SingleAxisBlender));
    }

    if (cPN_SingleAxisBlender::m_SingleAxisBlenderSlotPool.m_FreeList != NULL)
    {
        pSAB = (cPN_SingleAxisBlender*)cPN_SingleAxisBlender::m_SingleAxisBlenderSlotPool.m_FreeList;
        cPN_SingleAxisBlender::m_SingleAxisBlenderSlotPool.m_FreeList = cPN_SingleAxisBlender::m_SingleAxisBlenderSlotPool.m_FreeList->m_next;
    }

    return pSAB;
}

#endif // _PNSINGLEAXISBLENDER_H_
