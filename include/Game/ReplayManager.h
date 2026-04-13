#ifndef _REPLAYMANAGER_H_
#define _REPLAYMANAGER_H_

#define BASECAMERA_FOV_IMPL

#include "Game/Sys/eventman.h"
#include "Game/Camera/FollowCam.h"
#include "Game/RenderSnapshot.h"
#include "Game/Replay.h"

template <typename T>
void Blend(const float* blendFactors, const T& lhs, const T& rhs, T& result);

// void Replayable<0, SaveFrame, RenderSnapshot>(SaveFrame&, RenderSnapshot&);
// void Replayable<0, LoadFrame, RenderSnapshot>(LoadFrame&, RenderSnapshot&);

// class RenderSnapshot
// {
// public:
//     RenderSnapshot() { };
//     ~RenderSnapshot() { };

//     int NumDrawableObjects() const;
//     // /* 0x0000 */ char pad0[4];
//     // /* 0x0004 */ u8 unk4;                           /* inferred */
//     // /* 0x0005 */ char pad5[3];                      /* maybe part of unk4[4]? */
//     // /* 0x0008 */ RenderSnapshot *unk8;              /* inferred */
//     // /* 0x000C */ s8 unkC;                           /* inferred */
//     // /* 0x000D */ char padD[0x3F];                   /* maybe part of unkC[0x40]? */
//     // /* 0x004C */ ReplayManager unk4C;               /* inferred */
//     // /* 0x004C */ char pad4C[0x19A0];
//     // /* 0x19EC */ DrawableCharacter unk19EC;         /* inferred */
//     // /* 0x19EC */ char pad19EC[0x58];
//     // /* 0x1A44 */ DrawableCharacter unk1A44;         /* inferred */
//     // /* 0x1A44 */ char pad1A44[0x58];
//     // /* 0x1A9C */ s32 unk1A9C;                       /* inferred */
//     // /* 0x1AA0 */ s32 unk1AA0;                       /* inferred */
//     // /* 0x1AA4 */ char pad1AA4[0x18];                /* maybe part of unk1AA0[7]? */
//     // /* 0x1ABC */ f32 unk1ABC;                       /* inferred */
//     // /* 0x1AC0 */ ? unk1AC0;                         /* inferred */

//     /* 0x00 */ char pad1AC0[0x1AC0];
// }; // SIZE: 0x1AC0

// class Replay;

class ReplayManager
{
public:
    ~ReplayManager();
    static ReplayManager* Instance();
    void Initialize();
    void InitializeSnapshots();
    void Uninitialize();
    void GrabSnapshot();
    RenderSnapshot& GetMutableRenderSnapshot();
    void Flush();
    void ResetSnapshots();
    void PrepareForRecording();
    void SetCurrentTime(float);
    void EventHandler(Event*);
    static void EventHandler(Event* event, void* userData) { ((ReplayManager*)userData)->EventHandler(event); }
    void RenderSnapshotAt(float);

    /* 0x0000 */ RenderSnapshot mSnapshots[3]; // size: 0x5040
    /* 0x5040 */ RenderSnapshot* mCurrent;
    /* 0x5044 */ RenderSnapshot* mPrevious;
    /* 0x5048 */ RenderSnapshot* mRender;
    /* 0x504C */ cFollowCamera mDebugCamera;
    /* 0x50EC */ u32 mEvents;
    /* 0x50F0 */ f32 mSpeed;
    /* 0x50F4 */ f32 mSpeedUp;
    /* 0x50F8 */ f32 mDeltaTime;
    /* 0x50FC */ f32 mTime;
    /* 0x5100 */ f32 mBlend[3];
    /* 0x510C */ Replay* mReplay;
    /* 0x5110 */ u8* mMemory;
}; // total size: 0x5114

// class Replay
// {
// public:
//     void Play<RenderSnapshot>(float, RenderSnapshot&, RenderSnapshot&, float*) const;
//     void Record<RenderSnapshot>(float, RenderSnapshot&, unsigned int);
// };

#endif // _REPLAYMANAGER_H_
