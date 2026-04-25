#ifndef _RENDERSNAPSHOT_H_
#define _RENDERSNAPSHOT_H_

#include "types.h"
#include "NL/nlMath.h"

#include "Game/Replay.h"

#include "Game/Drawable/DrawableBall.h"
#include "Game/Drawable/DrawableCharacter.h"
#include "Game/Drawable/DrawablePowerup.h"
#include "Game/Drawable/DrawableExplosionFragment.h"

class DrawableNetMesh;

class RenderSnapshot
{
public:
    template <typename T>
    void Replay(T&);

    RenderSnapshot()
        : mValid(false)
        , mBall(this)
        , mpNetMeshPositiveX(NULL)
        , mpNetMeshNegativeX(NULL)
        , mFrameBlendPercent(0.0f)
    {
    }

    void Initialize();
    void Free();
    void Grab();
    int NumDrawableObjects() const;
    nlVector3 GetPositionForDrawableObject(int) const;
    void Invalidate();
    void Render(float) const;
    void RenderDebugInfo(const RenderSnapshot&, const RenderSnapshot&, float) const;
    void Blend(const float*, const RenderSnapshot&, const RenderSnapshot&);
    RenderSnapshot& GetMutable();

    /*  0x0000 */ unsigned int mEvents;
    /*  0x0004 */ bool mValid;
    /*  0x0005 */ bool mGoalLight;
    /*  0x0008 */ DrawableBall mBall;
    /*  0x004C */ DrawableCharacter mCharacters[10];
    /*  0x03BC */ DrawablePowerup mPowerups[150];
    /*  0x167C */ DrawableExplosionFragment mExplosionFragments[20];
    /*  0x19EC */ DrawableCharacter mChainChomp;
    /*  0x1A44 */ DrawableCharacter mBowser;
    /*  0x1A9C */ DrawableNetMesh* mpNetMeshPositiveX;
    /*  0x1AA0 */ DrawableNetMesh* mpNetMeshNegativeX;
    /*  0x1AA4 */ bool mDoGoalieNetTestPosX;
    /*  0x1AA5 */ bool mDoGoalieNetTestNegX;
    /*  0x1AA8 */ unsigned char* mpExplodableVisibilityRecords;
    /*  0x1AAC */ int mNumExplodables;
    /*  0x1AB0 */ nlVector3 mCameraUp;
    /*  0x1ABC */ float mFrameBlendPercent;
}; // total size: 0x1AC0

template <>
void RenderSnapshot::Replay<SaveFrame>(SaveFrame&);

template <>
void RenderSnapshot::Replay<LoadFrame>(LoadFrame&);

#endif // _RENDERSNAPSHOT_H_
