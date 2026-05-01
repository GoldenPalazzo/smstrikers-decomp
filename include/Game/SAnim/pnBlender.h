#ifndef _PNBLENDER_H_
#define _PNBLENDER_H_

#include "NL/nlSlotPool.h"
#include "Game/PoseAccumulator.h"

// static void operator delete(void* ptr)
// {
//     m_BlenderSlotPool.Free(static_cast<cPN_Blender*>(ptr));
// }

class cPN_Blender : public cPoseNode
{
public:
    cPN_Blender() { }
    cPN_Blender(cPoseNode*, cPoseNode*, float);
    /* 0x08 */ virtual ~cPN_Blender() { };
    /* 0x10 */ virtual void Evaluate(float, cPoseAccumulator*) const;
    /* 0x14 */ virtual void Evaluate(int, float, cPoseAccumulator*) const;
    /* 0x18 */ virtual cPoseNode* Update(float);
    /* 0x1C */ virtual int GetType() { return 0x0; };
    /* 0x20 */ virtual void BlendRootTrans(nlVector3*, float, float*);
    /* 0x24 */ virtual void BlendRootRot(unsigned short*, float, float*);

    template <typename T>
    void Replay(T&);

    inline void operator delete(void* ptr);

    /* 0x14 */ float m_fBlendTime;
    /* 0x18 */ float m_fBlendDuration;

    static SlotPool<cPN_Blender> m_BlenderSlotPool;
}; // total size: 0x1C

inline cPN_Blender* AllocateBlender()
{
    cPN_Blender* blender = nullptr;

    if (cPN_Blender::m_BlenderSlotPool.m_FreeList == nullptr)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
    }

    if (cPN_Blender::m_BlenderSlotPool.m_FreeList != nullptr)
    {
        blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
        cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
    }

    return blender;
}

inline cPN_Blender* CreateAndAssignBlender(const cPoseNode* child0, const cPoseNode* child1, float blendDuration)
{
    cPN_Blender* blender = nullptr;

    if (cPN_Blender::m_BlenderSlotPool.m_FreeList == nullptr)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
    }

    if (cPN_Blender::m_BlenderSlotPool.m_FreeList != nullptr)
    {
        blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
        cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
    }
    return new ((u8*)blender) cPN_Blender((cPoseNode*)child0, (cPoseNode*)child1, blendDuration);
}

#endif // _PNBLENDER_H_
