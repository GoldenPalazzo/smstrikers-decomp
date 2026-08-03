#include "Game/Drawable/DrawableBall.h"
#include "Game/Drawable/DrawableCharacter.h"
#include "Game/Drawable/DrawableModel.h"
#include "Game/RenderSnapshot.h"
#include "Game/Ball.h"
#include "Game/CharacterTemplate.h"
#include "Game/Player.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glState.h"
#include "NL/gl/glUserData.h"

static int g_nMotionBlurDivs = 5;
static float g_fMotionBlurAlpha0 = 0.1f;
static float g_fMotionBlurAlphaScale = 0.22f;

static float g_fBallBlur;

namespace
{
extern "C"
{
extern const unsigned long eOC_NO_LIGHT;
}
}

static glModel* BallLightingCB(glModel* pModel, eGLView& view, unsigned long& uLayer);
static glModel* BallBlurCB(glModel* pModel, eGLView& view, unsigned long& uLayer);

/**
 * Offset/Address/Size: 0x0 | 0x8011DD50 | size: 0x80
 */
void DrawableBall::EvaluateFrom(DrawableCharacter& character)
{
    mPosition = character.GetBallPosition();
    mOrientation = character.GetBallOrientation();
}

/**
 * Offset/Address/Size: 0x80 | 0x8011DDD0 | size: 0x140
 */
void DrawableBall::Blend(const float* alpha, const DrawableBall& a, const DrawableBall& b)
{
    mPrevOrientation = mOrientation;
    nlQuatNLerp(mOrientation, a.mOrientation, b.mOrientation, *alpha);

    const f32 t = *alpha;

    mPosition.f.x = (1.0f - t) * a.mPosition.f.x + t * b.mPosition.f.x;
    mPosition.f.y = (1.0f - t) * a.mPosition.f.y + t * b.mPosition.f.y;
    mPosition.f.z = (1.0f - t) * a.mPosition.f.z + t * b.mPosition.f.z;

    mVelocity.f.x = (1.0f - t) * a.mVelocity.f.x + t * b.mVelocity.f.x;
    mVelocity.f.y = (1.0f - t) * a.mVelocity.f.y + t * b.mVelocity.f.y;
    mVelocity.f.z = (1.0f - t) * a.mVelocity.f.z + t * b.mVelocity.f.z;

    mVisible = a.mVisible && b.mVisible;

    mOwnerIndex = b.mOwnerIndex;
    mPrevOwnerIndex = b.mPrevOwnerIndex;
    mPassTargetIndex = b.mPassTargetIndex;
}

inline void DrawableBall::RenderMotionBlur(DrawableObject& obj) const
{
    int i;
    float t;
    nlQuaternion q;
    nlMatrix4 mView;
    nlMatrix4 mWorld;
    nlMatrix4 mSaved;
    unsigned long uSavedFlags;

    glViewGetViewMatrix(GLV_Unshadowed, mView);

    const float blurOffsetScale = 0.0078125f;
    float blurOffsetX = blurOffsetScale * mView.f.m13;
    float blurOffsetY = blurOffsetScale * mView.f.m23;
    float blurOffsetZ = blurOffsetScale * mView.f.m33;

    u8 savedBallShadowDisabled = DrawableModel::sbBallShadowDisabled;
    DrawableModel::sbBallShadowDisabled = 1;

    mSaved = obj.GetWorldMatrix();
    glModel* (*savedBlurCB)(glModel*, eGLView&, unsigned long&) = obj.m_CB;
    obj.m_CB = BallBlurCB;

    uSavedFlags = obj.m_uObjectCreationFlags;
    obj.m_uObjectCreationFlags = uSavedFlags | eOC_NO_LIGHT;

    for (i = 0; i < g_nMotionBlurDivs; i++)
    {
        t = (float)i / (float)g_nMotionBlurDivs;
        g_fBallBlur = g_fMotionBlurAlphaScale * (1.0f - t) + g_fMotionBlurAlpha0;

        nlQuatNLerp(q, mPrevOrientation, mOrientation, t);
        obj.m_orientation = q;
        obj.m_worldMatrixUpToDate = 0;

        mWorld = obj.GetWorldMatrix();
        mWorld.f.m41 += blurOffsetX;
        mWorld.f.m42 += blurOffsetY;
        mWorld.f.m43 += blurOffsetZ;
        obj.m_worldMatrix = mWorld;

        obj.Draw();
    }

    DrawableModel::sbBallShadowDisabled = savedBallShadowDisabled;
    obj.m_worldMatrix = mSaved;
    obj.m_CB = savedBlurCB;
    obj.m_uObjectCreationFlags = uSavedFlags;
}

inline void DrawableBall::RenderLighting(DrawableObject& obj)
{
    u8 savedBallShadowDisabled = DrawableModel::sbBallShadowDisabled;
    DrawableModel::sbBallShadowDisabled = 1;

    glModel* (*savedLightingCB)(glModel*, eGLView&, unsigned long&) = obj.m_CB;
    obj.m_CB = BallLightingCB;

    unsigned long uSavedFlags = obj.m_uObjectCreationFlags;
    obj.m_uObjectCreationFlags &= ~0x80;
    obj.Draw();

    DrawableModel::sbBallShadowDisabled = savedBallShadowDisabled;
    obj.m_CB = savedLightingCB;
    obj.m_uObjectCreationFlags = uSavedFlags;
}

namespace
{
extern "C"
{
const unsigned long eOC_NO_LIGHT = 0x80;
}
}

/**
 * Offset/Address/Size: 0x1C0 | 0x8011DF10 | size: 0x47C
 * TODO: 98.87% match - remaining diffs are integer register allocation (r18 vs r24
 * shift for savedWorldMatrix and blurMatrix word copies in GetWorldMatrix copies)
 */
void DrawableBall::Render() const
{
    DrawableObject* drawable = g_pBall->m_pDrawableBall;
    if (mVisible)
    {
        drawable->m_uObjectFlags |= 1;
    }
    else
    {
        drawable->m_uObjectFlags &= ~1;
    }
    if (!mVisible)
    {
        return;
    }

    DrawableObject* pDrawableBall = g_pBall->m_pDrawableBall;
    pDrawableBall->m_orientation = mOrientation;
    pDrawableBall->m_worldMatrixUpToDate = 0;
    pDrawableBall->m_translation = mPosition;
    pDrawableBall->m_worldMatrixUpToDate = 0;

    unsigned long uSavedFlags = pDrawableBall->m_uObjectCreationFlags;
    pDrawableBall->m_uObjectCreationFlags &= ~0x80;
    pDrawableBall->Draw();
    pDrawableBall->m_uObjectCreationFlags = uSavedFlags;

    RenderMotionBlur(*pDrawableBall);
    RenderLighting(*pDrawableBall);
}

/**
 * Offset/Address/Size: 0x63C | 0x8011E38C | size: 0xB4
 */
static glModel* BallLightingCB(glModel* pModel, eGLView& view, unsigned long& uLayer)
{
    u32 tex;
    glModelPacket* pPacket;

    static u32 WhiteTexture = glGetTexture("global/white");

    pPacket = pModel->packets;
    tex = WhiteTexture;
    while (pPacket < &pModel->packets[pModel->numPackets])
    {
        pPacket->state.texture[0] = tex;
        glSetRasterState(pPacket->state.raster, GLS_AlphaBlend, 4);
        pPacket++;
    }

    uLayer += 2;
    return pModel;
}

/**
 * Offset/Address/Size: 0x6F0 | 0x8011E440 | size: 0x10C
 */
glModel* BallBlurCB(glModel* pModel, eGLView& view, unsigned long& uLayer)
{
    glModelPacket* pPacket;
    if (g_fBallBlur == 0.0f)
    {
        return nullptr;
    }

    void* pUserDataHandle = glUserAlloc(GLUD_ConstantColour, 4, false);
    u8* pColorData = (u8*)glUserGetData(pUserDataHandle);

    const float alphaFloat = 255.0f * g_fBallBlur;

    pColorData[0] = 0xC8;
    pColorData[1] = 0xC8;
    pColorData[2] = 0xC8;
    pColorData[3] = (u8)(int)alphaFloat;

    pPacket = pModel->packets;
    while (pPacket < &pModel->packets[pModel->numPackets])
    {
        if (g_fBallBlur != 1.0f)
        {
            glSetRasterState(pPacket->state.raster, GLS_AlphaBlend, 1);
            glSetRasterState(pPacket->state.raster, GLS_DepthWrite, 0);
            glUserAttach(pUserDataHandle, pPacket, false);
        }
        pPacket++;
    }

    return pModel;
}

/**
 * Offset/Address/Size: 0x7FC | 0x8011E54C | size: 0xCC
 */
void DrawableBall::Grab()
{
    mOrientation = g_pBall->m_qOrientation;
    mPosition = g_pBall->m_v3Position;
    mVelocity = g_pBall->m_v3Velocity;
    mOwnerIndex = GetCharacterIndex(g_pBall->m_pOwner);
    mPrevOwnerIndex = GetCharacterIndex(g_pBall->m_pPrevOwner);
    mPassTargetIndex = GetCharacterIndex(g_pBall->m_pPassTarget);
    mLastTouchIndex = GetCharacterIndex(g_pBall->m_pLastTouch);
    mVisible = true;
}

/**
 * Offset/Address/Size: 0x8C8 | 0x8011E618 | size: 0x24
 */
DrawableCharacter* DrawableBall::IndexToPlayer(int index) const
{
    if (index == -1)
    {
        return nullptr;
    }
    return &mRenderSnapshot->mCharacters[index];
}

/**
 * Offset/Address/Size: 0x138 | 0x8011E774 | size: 0x138
 */
template <>
void DrawableBall::Replay<SaveFrame>(SaveFrame& frame)
{
    Replayable<1, SaveFrame, FloatCompressor<-127, 127, 7> >(frame, FloatCompressor<-127, 127, 7>(mPosition.f.x));
    Replayable<1, SaveFrame, FloatCompressor<-127, 127, 7> >(frame, FloatCompressor<-127, 127, 7>(mPosition.f.y));
    Replayable<1, SaveFrame, FloatCompressor<-127, 127, 7> >(frame, FloatCompressor<-127, 127, 7>(mPosition.f.z));

    Replayable<1, SaveFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.x));
    Replayable<1, SaveFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.y));
    Replayable<1, SaveFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.z));
    Replayable<1, SaveFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.w));

    Replayable<1, SaveFrame, FloatCompressor<-127, 127, 5> >(frame, FloatCompressor<-127, 127, 5>(mVelocity.f.x));
    Replayable<1, SaveFrame, FloatCompressor<-127, 127, 5> >(frame, FloatCompressor<-127, 127, 5>(mVelocity.f.y));
    Replayable<1, SaveFrame, FloatCompressor<-127, 127, 5> >(frame, FloatCompressor<-127, 127, 5>(mVelocity.f.z));

    Replayable<1, SaveFrame, bool>(frame, mVisible);
    Replayable<1, SaveFrame, char>(frame, (char&)mOwnerIndex);
    Replayable<1, SaveFrame, char>(frame, (char&)mPrevOwnerIndex);
    Replayable<1, SaveFrame, char>(frame, (char&)mPassTargetIndex);
    Replayable<1, SaveFrame, char>(frame, (char&)mLastTouchIndex);
}

/**
 * Offset/Address/Size: 0x0 | 0x8011E63C | size: 0x138
 */
template <>
void DrawableBall::Replay<LoadFrame>(LoadFrame& frame)
{
    Replayable<1, LoadFrame, FloatCompressor<-127, 127, 7> >(frame, FloatCompressor<-127, 127, 7>(mPosition.f.x));
    Replayable<1, LoadFrame, FloatCompressor<-127, 127, 7> >(frame, FloatCompressor<-127, 127, 7>(mPosition.f.y));
    Replayable<1, LoadFrame, FloatCompressor<-127, 127, 7> >(frame, FloatCompressor<-127, 127, 7>(mPosition.f.z));

    Replayable<1, LoadFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.x));
    Replayable<1, LoadFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.y));
    Replayable<1, LoadFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.z));
    Replayable<1, LoadFrame, FloatCompressor<-1, 1, 13> >(frame, FloatCompressor<-1, 1, 13>(mOrientation.f.w));

    Replayable<1, LoadFrame, FloatCompressor<-127, 127, 5> >(frame, FloatCompressor<-127, 127, 5>(mVelocity.f.x));
    Replayable<1, LoadFrame, FloatCompressor<-127, 127, 5> >(frame, FloatCompressor<-127, 127, 5>(mVelocity.f.y));
    Replayable<1, LoadFrame, FloatCompressor<-127, 127, 5> >(frame, FloatCompressor<-127, 127, 5>(mVelocity.f.z));

    Replayable<1, LoadFrame, bool>(frame, mVisible);
    Replayable<1, LoadFrame, char>(frame, (char&)mOwnerIndex);
    Replayable<1, LoadFrame, char>(frame, (char&)mPrevOwnerIndex);
    Replayable<1, LoadFrame, char>(frame, (char&)mPassTargetIndex);
    Replayable<1, LoadFrame, char>(frame, (char&)mLastTouchIndex);
}

/**
 * Offset/Address/Size: 0xA4 | 0x8011E950 | size: 0x98
 */
// void Replayable<1, SaveFrame, FloatCompressor<-127, 127, 7>>(SaveFrame&, const FloatCompressor<-127, 127, 7>&)
// {
// }

/**
 * Offset/Address/Size: 0x13C | 0x8011E9E8 | size: 0x98
 */
// void Replayable<1, SaveFrame, FloatCompressor<-1, 1, 13>>(SaveFrame&, const FloatCompressor<-1, 1, 13>&)
// {
// }

/**
 * Offset/Address/Size: 0x1D4 | 0x8011EA80 | size: 0x98
 */
// void Replayable<1, SaveFrame, FloatCompressor<-127, 127, 5>>(SaveFrame&, const FloatCompressor<-127, 127, 5>&)
// {
// }

/**
 * Offset/Address/Size: 0x2E0 | 0x8011EB8C | size: 0x74
 */
// void Replayable<1, LoadFrame, FloatCompressor<-1, 1, 13>>(LoadFrame&, const FloatCompressor<-1, 1, 13>&)
// {
// }

/**
 * Offset/Address/Size: 0x354 | 0x8011EC00 | size: 0x74
 */
// void Replayable<1, LoadFrame, FloatCompressor<-127, 127, 5>>(LoadFrame&, const FloatCompressor<-127, 127, 5>&)
// {
// }
