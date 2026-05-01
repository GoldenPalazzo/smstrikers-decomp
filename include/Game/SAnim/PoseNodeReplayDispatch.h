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

    if (typeId == 0) {
        ((cPN_Blender*)poseNode)->Replay(frame);
    } else if (typeId == 1) {
        ((cPN_Feather*)poseNode)->Replay(frame);
    } else if (typeId == 2) {
        ((cPN_SAnimController*)poseNode)->Replay(frame);
    } else if (typeId == 3) {
        ((cPN_SingleAxisBlender*)poseNode)->Replay(frame);
    }
}
#pragma inline_depth()

#endif // _POSENODEREPLAYDISPATCH_H_
