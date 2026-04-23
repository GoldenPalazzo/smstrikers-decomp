#ifndef _DRAWABLEBALL_H_
#define _DRAWABLEBALL_H_

#include "Game/Drawable/DrawableObj.h"
#include "NL/nlMath.h"

// void Replayable<1, SaveFrame, char>(SaveFrame&, char&);
// void Replayable<1, LoadFrame, char>(LoadFrame&, char&);
// void Replayable<1, SaveFrame, FloatCompressor<-127, 127, 7>>(SaveFrame&, const FloatCompressor<-127, 127, 7>&);
// void Replayable<1, SaveFrame, FloatCompressor<-1, 1, 13>>(SaveFrame&, const FloatCompressor<-1, 1, 13>&);
// void Replayable<1, SaveFrame, FloatCompressor<-127, 127, 5>>(SaveFrame&, const FloatCompressor<-127, 127, 5>&);
// void Replayable<1, LoadFrame, FloatCompressor<-127, 127, 7>>(LoadFrame&, const FloatCompressor<-127, 127, 7>&);
// void Replayable<1, LoadFrame, FloatCompressor<-1, 1, 13>>(LoadFrame&, const FloatCompressor<-1, 1, 13>&);
// void Replayable<1, LoadFrame, FloatCompressor<-127, 127, 5>>(LoadFrame&, const FloatCompressor<-127, 127, 5>&);

class LoadFrame;
class SaveFrame;

class RenderSnapshot;
class DrawableCharacter;

class DrawableBall
{
public:
    template <typename T>
    void Replay(T&);

    void EvaluateFrom(DrawableCharacter&);
    void Blend(const float*, const DrawableBall&, const DrawableBall&);
    void Render() const;
    void Grab();
    DrawableCharacter* IndexToPlayer(int) const;

    /* 0x00 */ RenderSnapshot* mRenderSnapshot; // offset 0x0, size 0x4
    /* 0x04 */ bool mVisible;                   // offset 0x4, size 0x1
    /* 0x05 */ s8 mLastTouchIndex;              // offset 0x5, size 0x1
    /* 0x06 */ s8 mOwnerIndex;                  // offset 0x6, size 0x1
    /* 0x07 */ s8 mPrevOwnerIndex;              // offset 0x7, size 0x1
    /* 0x08 */ s8 mPassTargetIndex;             // offset 0x8, size 0x1
    /* 0x0C */ nlVector3 mVelocity;             // offset 0xC, size 0xC
    /* 0x18 */ nlVector3 mPosition;             // offset 0x18, size 0xC
    /* 0x24 */ nlQuaternion mOrientation;       // offset 0x24, size 0x10
    /* 0x34 */ nlQuaternion mPrevOrientation;   // offset 0x34, size 0x10
}; // total size: 0x44

template <>
void DrawableBall::Replay<LoadFrame>(LoadFrame&);

template <>
void DrawableBall::Replay<SaveFrame>(SaveFrame&);

#endif // _DRAWABLEBALL_H_
