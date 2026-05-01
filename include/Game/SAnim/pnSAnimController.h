#ifndef _PNSANIMCONTROLLER_H_
#define _PNSANIMCONTROLLER_H_

#include "Game/SAnim.h"
#include "NL/nlMath.h"
#include "types.h"

#include "Game/SHierarchy.h"
#include "Game/PoseAccumulator.h"
#include "NL/nlSlotPool.h"

// class SlotPool<cPN_SAnimController>
// {
// public:
//     void ~SlotPool();
// };

// class cSAnim
// {
// public:
//     void GetCallbackList() const;
// };

class AnimRetarget;

class cPN_SAnimController : public cPoseNode
{
public:
    cPN_SAnimController()
        : cPoseNode(0)
        , m_pSAnim(NULL)
        , m_fTime(0.0f)
    {
    }
    cPN_SAnimController(cSAnim*, const AnimRetarget*, ePlayMode, void (*)(unsigned int, cPN_SAnimController*), unsigned int, bool);
    /* 0x08 */ virtual ~cPN_SAnimController() { };
    static void operator delete(void* ptr)
    {
        ((SlotPoolEntry*)ptr)->m_next = m_SAnimControllerSlotPool.m_FreeList;
        m_SAnimControllerSlotPool.m_FreeList = (SlotPoolEntry*)ptr;
    }
    /* 0x10 */ virtual void Evaluate(float, cPoseAccumulator*) const;
    /* 0x14 */ virtual void Evaluate(int, float, cPoseAccumulator*) const;
    /* 0x18 */ virtual cPoseNode* Update(float);
    /* 0x1C */ virtual int GetType() { return 0x2; };
    /* 0x20 */ virtual void BlendRootTrans(nlVector3*, float, float*);
    /* 0x24 */ virtual void BlendRootRot(unsigned short*, float, float*);

    template <typename T>
    void Replay(T& frame);

    void UpdateSynchronized(float);
    void SetTime(float time)
    {
        FORCE_DONT_INLINE;
        FORCE_DONT_INLINE;
        FORCE_DONT_INLINE;
        m_fPrevTime = m_fTime;
        m_fTime = time;
    };
    void ProcessCallbacks();
    bool TestTrigger(float) const;
    bool TestFrameTrigger(float);
    int RemapNode(int) const;

    inline const float get_fTime() const
    {
        return m_fTime;
    }

    inline const bool get_bMirror() const
    {
        return m_bMirror;
    }

    // inline cSAnim* get_pSAnim() const
    // {
    //     return m_pSAnim;
    // }

    /* 0x14 */ cSAnim* m_pSAnim;
    /* 0x18 */ float m_fTime;
    /* 0x1C */ bool m_bMirror;
    /* 0x20 */ const AnimRetarget* m_pAnimRetarget;
    /* 0x24 */ float m_fPrevTime;
    /* 0x28 */ ePlayMode m_ePlayMode;
    /* 0x2C */ mutable float m_fWeight;
    /* 0x30 */ bool m_bIgnoreTriggers;
    /* 0x34 */ void (*m_funcPlaybackSpeedCallback)(unsigned int, class cPN_SAnimController*);
    /* 0x38 */ unsigned int m_nPlaybackSpeedCallbackParam;
    /* 0x3C */ float m_fPlaybackSpeedScale;
    /* 0x40 */ bool m_bIsSynchronized;
    /* 0x44 */ cPN_SAnimController* m_pSynchronizedController;
    /* 0x48 */ void (*m_funcSychronizedWeightCallback)(unsigned int, class cPN_SAnimController*);
    /* 0x4C */ unsigned int m_nSynchronizedWeightCallbackParam;
    /* 0x50 */ float m_fSynchronizedWeight;

    static SlotPool<cPN_SAnimController> m_SAnimControllerSlotPool;
}; // total size: 0x54

inline cPN_SAnimController* AllocateSAnimController()
{
    cPN_SAnimController* controller = nullptr;
    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }
    return controller;
}

#endif // _PNSANIMCONTROLLER_H_
