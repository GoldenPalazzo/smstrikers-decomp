#ifndef _POSENODEREPLAYDISPATCH_H_
#define _POSENODEREPLAYDISPATCH_H_

#include "Game/SAnim/pnBlender.h"
#include "Game/SAnim/pnFeather.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "NL/nlDebug.h"

class SaveFrame;

#pragma inline_depth(0)
template <int N>
void Replayable(SaveFrame& frame, char typeId, cPoseNode*& poseNode)
{
    if (typeId < 0 || typeId > 3)
        nlBreak();

    if (typeId == 0)
    {
        ((cPN_Blender*)poseNode)->Replay(frame);
    }
    else if (typeId == 1)
    {
        ((cPN_Feather*)poseNode)->Replay(frame);
    }
    else if (typeId == 2)
    {
        ((cPN_SAnimController*)poseNode)->Replay(frame);
    }
    else if (typeId == 3)
    {
        ((cPN_SingleAxisBlender*)poseNode)->Replay(frame);
    }
}

template <>
void Replayable<1>(SaveFrame& frame, char typeId, cPoseNode*& poseNode)
{
    if (frame.mInterval == 1)
    {
        if (typeId < 0 || typeId > 3)
            nlBreak();

        if (typeId == 0)
        {
            cPN_Blender* blender = (cPN_Blender*)poseNode;
            Replayable<0>(frame, (cPoseNode&)*blender);

            struct FloatProxy7
            {
                float* mF;
            } proxy7;
            proxy7.mF = &blender->m_fBlendTime;
            Replayable<0>(frame, (const FloatCompressor<0, 1, 7>&)proxy7);
        }
        else if (typeId == 1)
        {
            Replayable<0>(frame, (cPoseNode&)*poseNode);
        }
        else if (typeId == 2)
        {
            cPN_SAnimController* controller = (cPN_SAnimController*)poseNode;
            Replayable<0>(frame, (cPoseNode&)*controller);

            struct FloatProxy15
            {
                float* mF;
            } proxy15;
            proxy15.mF = &controller->m_fTime;
            Replayable<0>(frame, (const FloatCompressor<0, 1, 15>&)proxy15);

            unsigned int animPtr = 0;
            animPtr = (unsigned int)controller->m_pSAnim;
            if (controller->m_bMirror)
                animPtr |= 1;

            Replayable<0>(frame, animPtr);
            Replayable<0>(frame, (unsigned int&)controller->m_pAnimRetarget);
        }
        else if (typeId == 3)
        {
            cPN_SingleAxisBlender* singleAxis = (cPN_SingleAxisBlender*)poseNode;
            Replayable<0>(frame, (cPoseNode&)*singleAxis);

            struct FloatProxy7b
            {
                float* mF;
            } proxy7b;
            proxy7b.mF = &singleAxis->m_fSmoothedWeight;
            Replayable<0>(frame, (const FloatCompressor<0, 1, 7>&)proxy7b);
        }
    }
}
#pragma inline_depth()

#endif // _POSENODEREPLAYDISPATCH_H_
