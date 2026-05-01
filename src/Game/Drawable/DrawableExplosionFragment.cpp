#include "Game/Drawable/DrawableExplosionFragment.h"

#include "Game/Game.h"
#include "Game/Render/SidelineExplodable.h"
#include "Game/World.h"
#include "Game/WorldManager.h"
#include "Game/Drawable/DrawableObj.h"

/**
 * Offset/Address/Size: 0x0 | 0x8011F7A0 | size: 0xF8
 */
void DrawableExplosionFragment::Blend(const float* w, const DrawableExplosionFragment& a, const DrawableExplosionFragment& b)
{
    mVisible = a.mVisible && b.mVisible && (a.mFragmentModelHash == b.mFragmentModelHash);

    if (!mVisible)
        return;

    const float t = w[2];
    const float it = 1.0f - t;

    mPosition.f.x = it * a.mPosition.f.x + t * b.mPosition.f.x;
    mPosition.f.y = it * a.mPosition.f.y + t * b.mPosition.f.y;
    mPosition.f.z = it * a.mPosition.f.z + t * b.mPosition.f.z;

    nlQuatSlerp(mOrientation, a.mOrientation, b.mOrientation, t);

    mFragmentModelHash = a.mFragmentModelHash;
    float opacity = (a.mOpacity + b.mOpacity);
    float factor = 0.5f;
    mOpacity = opacity * factor; // otherwise it does not match the asm
}

/**
 * Offset/Address/Size: 0xF8 | 0x8011F898 | size: 0xDC
 */
void DrawableExplosionFragment::Render() const
{
    if (g_pGame->mbCaptainShotToScoreOn == 0)
    {
        DrawableObject* obj = WorldManager::s_World->FindDrawableObject(mFragmentModelHash);
        if (obj != NULL)
        {
            obj->m_translation = mPosition;
            obj->m_worldMatrixUpToDate = false;
            obj->m_orientation = mOrientation;
            obj->m_worldMatrixUpToDate = false;

            obj->m_translucency = mOpacity;
            if (obj->m_translucency < 0.0f)
            {
                obj->m_translucency = 0.0f;
            }
            if (obj->m_translucency > 1.0f)
            {
                obj->m_translucency = 1.0f;
            }

            if (mVisible)
            {
                obj->Draw();
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1D4 | 0x8011F974 | size: 0xBC
 */
void DrawableExplosionFragment::Grab()
{
    ExplosionFragment* frag = SidelineExplodableManager::GetFragmentFromHandle(mID);
    if (frag == NULL)
    {
        mVisible = false;
        return;
    }

    mVisible = true;
    mFragmentModelHash = frag->mFragmentModelHash;

    mPosition = frag->ExplosionFragment::GetPosition();

    static nlMatrix4 temp;
    frag->ExplosionFragment::GetRotation(&temp);
    nlMatrixToQuat(mOrientation, temp);

    if (frag->mfRemainingLifespan > ExplosionFragment::sfFadeOutTime)
    {
        mOpacity = 1.0f;
    }
    else
    {
        float ratio = frag->mfRemainingLifespan;
        ratio /= ExplosionFragment::sfFadeOutTime;
        mOpacity = ratio;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8011FA30 | size: 0x7C
 */
template <>
void DrawableExplosionFragment::Replay<LoadFrame>(LoadFrame& frame)
{
    Replayable<3, LoadFrame, bool>(frame, mVisible);
    if (mVisible)
    {
        Replayable<3, LoadFrame, unsigned long>(frame, mFragmentModelHash);
        Replayable<3, LoadFrame, nlVector3>(frame, mPosition);
        Replayable<3, LoadFrame, nlQuaternion>(frame, mOrientation);
        Replayable<3, LoadFrame, float>(frame, mOpacity);
    }
}

/**
 * Offset/Address/Size: 0x7C | 0x8011FAAC | size: 0x7C
 */
template <>
void DrawableExplosionFragment::Replay<SaveFrame>(SaveFrame& frame)
{
    Replayable<3, SaveFrame, bool>(frame, mVisible);
    if (mVisible)
    {
        Replayable<3, SaveFrame, unsigned long>(frame, mFragmentModelHash);
        Replayable<3, SaveFrame, nlVector3>(frame, mPosition);
        Replayable<3, SaveFrame, nlQuaternion>(frame, mOrientation);
        Replayable<3, SaveFrame, float>(frame, mOpacity);
    }
}

#include "Game/ReplaySpecializations.h"

/**
 * Offset/Address/Size: 0x0 | 0x8011FB28 | size: 0x50
 */
// void Replayable<3, SaveFrame, unsigned long>(SaveFrame&, unsigned long&)
// {
// }

/**
 * Offset/Address/Size: 0x50 | 0x8011FB78 | size: 0x50
 */
// Replayable<3, SaveFrame, nlQuaternion> defined in ReplaySpecializations.h

/**
 * Offset/Address/Size: 0xA0 | 0x8011FBC8 | size: 0x54
 */
// void Replayable<3, LoadFrame, unsigned long>(LoadFrame&, unsigned long&)
// {
// }

/**
 * Offset/Address/Size: 0xF4 | 0x8011FC1C | size: 0x54
 */
// void Replayable<3, LoadFrame, nlQuaternion>(LoadFrame&, nlQuaternion&)
// {
// }
