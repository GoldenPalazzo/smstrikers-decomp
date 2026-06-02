#ifndef _POSENODEREPLAYDISPATCH_H_
#define _POSENODEREPLAYDISPATCH_H_

#include "Game/SAnim/pnBlender.h"
#include "Game/SAnim/pnFeather.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "NL/nlDebug.h"

class SaveFrame;
class LoadFrame;

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

template <int N>
void Replayable(LoadFrame& frame, char typeId, cPoseNode*& poseNode)
{
    FORCE_DONT_INLINE;
    if (typeId == 0)
    {
        cPN_Blender* blender = AllocateBlender();
        new ((u8*)blender) cPN_Blender();
        blender->Replay(frame);
        poseNode = blender;
    }
    else if (typeId == 1)
    {
        cPN_Feather* feather = AllocateFeather();
        new ((u8*)feather) cPN_Feather();
        feather->Replay(frame);
        poseNode = feather;
    }
    else if (typeId == 2)
    {
        cPN_SAnimController* controller = AllocateSAnimController();
        new ((u8*)controller) cPN_SAnimController();
        controller->Replay(frame);
        poseNode = controller;
    }
    else if (typeId == 3)
    {
        cPN_SingleAxisBlender* singleAxis = AllocateSingleAxisBlender();
        new ((u8*)singleAxis) cPN_SingleAxisBlender();
        singleAxis->Replay(frame);
        poseNode = singleAxis;
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

template <>
void Replayable<1>(LoadFrame& frame, char typeId, cPoseNode*& poseNode)
{
    if (frame.mInterval == 1)
    {
        if (typeId == 0)
        {
            cPN_Blender* blender = AllocateBlender();
            new ((u8*)blender) cPN_Blender();
            Replayable<0>(frame, (cPoseNode&)*blender);

            struct FloatProxy7
            {
                float* mF;
            } proxy7;
            proxy7.mF = &blender->m_fBlendTime;
            Replayable<0>(frame, (const FloatCompressor<0, 1, 7>&)proxy7);
            poseNode = blender;
        }
        else if (typeId == 1)
        {
            cPN_Feather* feather = AllocateFeather();
            new ((u8*)feather) cPN_Feather();
            Replayable<0>(frame, (cPoseNode&)*feather);
            feather->m_fBlendTime = 0.0f;
            feather->m_pFeatherWeights = NULL;
            poseNode = feather;
        }
        else if (typeId == 2)
        {
            cPN_SAnimController* controller = AllocateSAnimController();
            new ((u8*)controller) cPN_SAnimController();
            Replayable<0>(frame, (cPoseNode&)*controller);

            struct FloatProxy15
            {
                float* mF;
            } proxy15;
            proxy15.mF = &controller->m_fTime;
            Replayable<0>(frame, (const FloatCompressor<0, 1, 15>&)proxy15);

            unsigned int animPtr = 0;
            Replayable<0>(frame, animPtr);
            controller->m_bMirror = (animPtr & 1);
            controller->m_pSAnim = (cSAnim*)(animPtr & ~1);
            Replayable<0>(frame, (unsigned int&)controller->m_pAnimRetarget);
            poseNode = controller;
        }
        else if (typeId == 3)
        {
            cPN_SingleAxisBlender* singleAxis = AllocateSingleAxisBlender();
            new ((u8*)singleAxis) cPN_SingleAxisBlender();
            Replayable<0>(frame, (cPoseNode&)*singleAxis);

            struct FloatProxy7b
            {
                float* mF;
            } proxy7b;
            proxy7b.mF = &singleAxis->m_fSmoothedWeight;
            Replayable<0>(frame, (const FloatCompressor<0, 1, 7>&)proxy7b);
            poseNode = singleAxis;
        }
    }
}

#endif // _POSENODEREPLAYDISPATCH_H_
