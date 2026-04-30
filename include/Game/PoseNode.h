#ifndef _POSENODE_H_
#define _POSENODE_H_

#include "NL/nlMath.h"

template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, T& drawable);

template <int N, typename FrameType, typename T>
void ReplayablePolymorphic(FrameType& frame, T*& ptr);

class cPoseAccumulator;

class cPoseNode
{
public:
    cPoseNode();
    cPoseNode(int);

    /* 0x08 */ virtual ~cPoseNode();
    /* 0x14 */ virtual void Evaluate(int, float, cPoseAccumulator*) const = 0;
    /* 0x10 */ virtual void Evaluate(float, cPoseAccumulator*) const = 0;
    /* 0x18 */ virtual cPoseNode* Update(float) = 0;
    /* 0x1C */ virtual int GetType() = 0;
    /* 0x20 */ virtual void BlendRootTrans(nlVector3* outBase, float weight, float* scratch) = 0;
    /* 0x24 */ virtual void BlendRootRot(unsigned short* outRot, float weight, float* scratch) = 0;

    // /* 0x0C */ virtual void V_unkC(int) = 0;
    // // /* 0x10 */ virtual void V_unk10(float) const = 0;
    // /* 0x10 */ virtual void V_unk10(cPoseAccumulator* accum, float weight) const = 0;
    // /* 0x14 */ virtual void V_unk14() = 0;
    // /* 0x18 */ virtual void V_unk18() = 0;

    // void V_GetRootTrans(nlVector3* outBase, float weight, float* scratch) = 0;
    // virtual void V_GetRootRot(unsigned short* outRot, float weight, float* scratch) = 0;

    void GetRootRot(unsigned short*);
    void GetRootTrans(nlVector3*, unsigned short);
    void SetChild(int, cPoseNode*);
    cPoseNode** GetChildPtr(int);
    cPoseNode* GetChild(int) const;
    cPoseNode* GetChild(int);

    template <typename T>
    void Replay(T& frame);

    /* 0x4 */ cPoseNode* m_children[3];
    /* 0x10 */ int m_numChildren;
}; // total size: 0x14

template <typename T>
inline void cPoseNode::Replay(T& frame)
{
    Replayable<0>(frame, m_numChildren);
    for (int i = 0; i < m_numChildren; i++)
    {
        ReplayablePolymorphic<0>(frame, m_children[i]);
    }
}

#endif // _POSENODE_H_
