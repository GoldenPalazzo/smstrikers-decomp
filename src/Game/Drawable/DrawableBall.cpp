#include "Game/Drawable/DrawableBall.h"
#include "Game/Drawable/DrawableCharacter.h"
#include "Game/RenderSnapshot.h"
#include "Game/Ball.h"
#include "Game/CharacterTemplate.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glState.h"
#include "NL/gl/glUserData.h"

extern int g_nMotionBlurDivs;
extern float g_fMotionBlurAlpha0;
extern float g_fMotionBlurAlphaScale;

static float g_fBallBlur;

extern const unsigned long eOC_NO_LIGHT;
extern unsigned char sbBallShadowDisabled__13DrawableModel;

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
 * TODO: 99.19% match on scratch - epilogue scheduling diff (LR restore order)
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

/**
 * Offset/Address/Size: 0x1C0 | 0x8011DF10 | size: 0x47C
 * TODO: 98.87% match - remaining diffs are integer register allocation (r18 vs r24
 * shift for savedWorldMatrix and blurMatrix word copies in GetWorldMatrix copies)
 */
void DrawableBall::Render() const
{
    unsigned long savedBlurCreationFlags;
    int i;
    DrawableObject* pDrawableBall;
    glModel* (*savedBlurCB)(glModel*, eGLView&, unsigned long&);
    u8 savedBallShadowDisabled;
    unsigned long savedLightingFlags;
    glModel* (*savedLightingCB)(glModel*, eGLView&, unsigned long&);
    u8 savedBallShadowDisabled2;

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

    pDrawableBall = g_pBall->m_pDrawableBall;
    pDrawableBall->m_orientation = mOrientation;
    pDrawableBall->m_worldMatrixUpToDate = 0;
    pDrawableBall->m_translation = mPosition;
    pDrawableBall->m_worldMatrixUpToDate = 0;

    unsigned long savedCreationFlags = pDrawableBall->m_uObjectCreationFlags;
    pDrawableBall->m_uObjectCreationFlags &= ~0x80;
    pDrawableBall->Draw();
    pDrawableBall->m_uObjectCreationFlags = savedCreationFlags;

    nlMatrix4 savedWorldMatrix;
    nlMatrix4 blurMatrix;
    nlMatrix4 viewMatrix;
    glViewGetViewMatrix(GLV_Unshadowed, viewMatrix);

    const float blurOffsetScale = 0.0078125f;
    float blurOffsetX, blurOffsetY, blurOffsetZ;
    blurOffsetZ = blurOffsetScale * viewMatrix.f.m33;
    blurOffsetY = blurOffsetScale * viewMatrix.f.m23;
    blurOffsetX = blurOffsetScale * viewMatrix.f.m13;

    savedBallShadowDisabled = sbBallShadowDisabled__13DrawableModel;
    sbBallShadowDisabled__13DrawableModel = 1;

    savedWorldMatrix = pDrawableBall->GetWorldMatrix();
    savedBlurCB = pDrawableBall->m_CB;
    pDrawableBall->m_CB = BallBlurCB;

    savedBlurCreationFlags = pDrawableBall->m_uObjectCreationFlags;
    pDrawableBall->m_uObjectCreationFlags = savedBlurCreationFlags | eOC_NO_LIGHT;

    for (i = 0; i < g_nMotionBlurDivs; i++)
    {
        float alpha = (float)i / (float)g_nMotionBlurDivs;
        g_fBallBlur = g_fMotionBlurAlphaScale * (1.0f - alpha) + g_fMotionBlurAlpha0;

        nlQuaternion orientation;
        nlQuatNLerp(orientation, mPrevOrientation, mOrientation, alpha);
        pDrawableBall->m_orientation = orientation;
        pDrawableBall->m_worldMatrixUpToDate = 0;

        blurMatrix = pDrawableBall->GetWorldMatrix();
        blurMatrix.f.m41 += blurOffsetX;
        blurMatrix.f.m42 += blurOffsetY;
        blurMatrix.f.m43 += blurOffsetZ;
        pDrawableBall->m_worldMatrix = blurMatrix;

        pDrawableBall->Draw();
    }

    sbBallShadowDisabled__13DrawableModel = savedBallShadowDisabled;
    pDrawableBall->m_worldMatrix = savedWorldMatrix;
    pDrawableBall->m_CB = savedBlurCB;
    pDrawableBall->m_uObjectCreationFlags = savedBlurCreationFlags;

    savedBallShadowDisabled2 = sbBallShadowDisabled__13DrawableModel;
    sbBallShadowDisabled__13DrawableModel = 1;

    savedLightingCB = pDrawableBall->m_CB;
    pDrawableBall->m_CB = BallLightingCB;

    savedLightingFlags = pDrawableBall->m_uObjectCreationFlags;
    pDrawableBall->m_uObjectCreationFlags &= ~0x80;
    pDrawableBall->Draw();

    sbBallShadowDisabled__13DrawableModel = savedBallShadowDisabled2;
    pDrawableBall->m_CB = savedLightingCB;
    pDrawableBall->m_uObjectCreationFlags = savedLightingFlags;
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
 * Offset/Address/Size: 0x0 | 0x8011E8AC | size: 0x50
 */
// void Replayable<1, SaveFrame, char>(SaveFrame&, char&)
// {
// }

/**
 * Offset/Address/Size: 0x50 | 0x8011E8FC | size: 0x54
 */
// void Replayable<1, LoadFrame, char>(LoadFrame&, char&)
// {
// }

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
 * Offset/Address/Size: 0x26C | 0x8011EB18 | size: 0x74
 */
// void Replayable<1, LoadFrame, FloatCompressor<-127, 127, 7>>(LoadFrame&, const FloatCompressor<-127, 127, 7>&)
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
