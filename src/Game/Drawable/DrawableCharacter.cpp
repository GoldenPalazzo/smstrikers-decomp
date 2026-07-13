#include "Game/Drawable/DrawableCharacter.h"
#include "Game/Character.h"
#include "Game/Player.h"
#include "Game/Team.h"
#include "Game/AI/HeadTrack.h"
#include "Game/PoseAccumulator.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim/pnFeather.h"
#include "Game/SAnim/pnBlender.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "Game/SAnim/PoseNodeReplayDispatch.h"
#include "Game/Render/RenderShadow.h"
#include "Game/GameObjectLighting.h"
#include "Game/WorldManager.h"
#include "Game/BasicStadium.h"
#include "Game/GL/GLInventory.h"
#include "NL/gl/glView.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glUserData.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glModify.h"
#include "NL/nlTask.h"
#include "NL/nlTicker.h"
#include "NL/nlString.h"
#include "NL/nlDebug.h"
#include <dolphin/os.h>

cCharacter* DrawableCharacter::spRenderOnlyThisCharacter = nullptr;
bool DrawableCharacter::sbRenderOpposingGoalieToo = false;
bool DrawableCharacter::sCameraRelativeLighting = false;
bool DrawableCharacter::sSTSLighting = false;

unsigned char sShadowRenderingDisabled__17DrawableCharacter;

const u32 GLTT_BumpLocal_bit = 1 << (int)GLTT_BumpLocal;
static unsigned long GLTT_Detail_bit = 1UL << (int)GLTT_Detail;

static unsigned long UnlitProgram = glGetProgram("3d unlit");
static unsigned long LitProgram = glGetProgram("3d pointlit");
static unsigned long LightTexture = glGetTexture("global/lightramp");
static unsigned long BlackTexture = glGetTexture("global/black");
static unsigned long WhiteTexture = glGetTexture("global/white");
static unsigned long CharacterDirtProgram = glGetProgram("3d pointlit dirt");
static unsigned long CharacterProgram = glGetProgram("3d pointlit");

static int g_nShowBones;
static const int g_nOnscreenUpdate[3] = { 4, 3, 2 };
static const int g_nOffscreenUpdate[3] = { 8, 8, 6 };

struct ShadowScale
{
    float fRadius;
    float fHeight;
    float fScalar;
};

ShadowScale shadowScale[] = {
    { 1.5f, 1.0f, 1.0f },
    { 1.75f, 1.25f, 1.25f },
    { 2.8f, 1.25f, 2.0f },
    { 1.5f, 1.0f, 1.0f },
    { 1.5f, 1.0f, 1.0f },
    { 1.5f, 1.0f, 1.0f },
    { 1.5f, 1.0f, 1.0f },
    { 1.75f, 1.25f, 1.25f },
    { 1.5f, 1.0f, 1.0f },
    { 2.0f, 1.25f, 1.5f },
    { 1.5f, 1.0f, 1.0f },
    { 1.5f, 1.0f, 1.0f },
    { 1.75f, 1.25f, 1.25f },
    { 1.75f, 1.25f, 1.25f },
};

int charSizes[] = {
    0,
    1,
    2,
    0,
    0,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    1,
    0,
};

static float g_fRadiusScale = 1.175f;
static unsigned char g_bSloppyBounds = 1;
static u32 debugColourValue = 0xFFFF4050;

template <>
FloatCompressor<0, 1, 15>::FloatCompressor(float& f)
    : mF(f)
{
    FORCE_DONT_INLINE;
}

template <>
FloatCompressor<0, 1, 7>::FloatCompressor(float& f)
    : mF(f)
{
    FORCE_DONT_INLINE;
}

template <>
inline void Replayable<1, LoadFrame, FloatCompressor<-128, 128, 8> >(LoadFrame& frame, const FloatCompressor<-128, 128, 8>& proxy)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        unsigned int value = 0;
        if (frame.mInterval == 1)
        {
            const char* cursor = frame.mStream.mStorage;
            unsigned char mid = (unsigned char)cursor[1];
            unsigned char hi = (unsigned char)cursor[2];
            unsigned char lo = (unsigned char)cursor[0];
            cursor += 3;
            value = (unsigned int)mid << 8;
            value |= (unsigned int)hi << 16;
            frame.mStream.mStorage = cursor;
            value |= (unsigned int)lo;
        }
        proxy.mF = (float)value / (float)(1 << 8);
        proxy.mF += (float)(-128);
    }
}

template <>
inline void Replayable<1, LoadFrame, FloatCompressor<-512, 512, 8> >(LoadFrame& frame, const FloatCompressor<-512, 512, 8>& proxy)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        unsigned int value = 0;
        if (frame.mInterval == 1)
        {
            const char* cursor = frame.mStream.mStorage;
            unsigned char mid = (unsigned char)cursor[1];
            unsigned char hi = (unsigned char)cursor[2];
            unsigned char lo = (unsigned char)cursor[0];
            cursor += 3;
            value = (unsigned int)mid << 8;
            value |= (unsigned int)hi << 16;
            frame.mStream.mStorage = cursor;
            value |= (unsigned int)lo;
        }
        proxy.mF = (float)value / (float)(1 << 8);
        proxy.mF += (float)(-512);
    }
}

template <>
void ReplayablePolymorphic<1, LoadFrame, cPoseNode>(LoadFrame& frame, cPoseNode*& ptr);

template <>
void ReplayablePolymorphic<1, SaveFrame, cPoseNode>(SaveFrame& frame, cPoseNode*& ptr);

/**
 * Offset/Address/Size: 0x2C0 | 0x8011C5EC | size: 0x178
 */
template <>
void DrawableCharacter::Replay<SaveFrame>(SaveFrame& frame)
{
    Replayable<1>(frame, (unsigned char&)mDirt);
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mPosition.f.x));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mPosition.f.y));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mPosition.f.z));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mBip01Position.f.x));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mBip01Position.f.y));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mBip01Position.f.z));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mHeadPosition.f.x));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mHeadPosition.f.y));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mHeadPosition.f.z));
    Replayable<1>(frame, FloatCompressor<-512, 512, 8>(mVelocity.f.x));
    Replayable<1>(frame, FloatCompressor<-512, 512, 8>(mVelocity.f.y));
    Replayable<1>(frame, FloatCompressor<-512, 512, 8>(mVelocity.f.z));
    Replayable<1>(frame, mVisible);
    Replayable<1>(frame, mFacingDirection);
    Replayable<1>(frame, mHeadSpin);
    Replayable<1>(frame, mHeadTilt);
    Replayable<1>(frame, (unsigned long&)mEffectsTexturing);
    ReplayablePolymorphic<1>(frame, mPoseTree);
}

/**
 * Offset/Address/Size: 0x0 | 0x8011C32C | size: 0x2C0
 */
template <>
void DrawableCharacter::Replay<LoadFrame>(LoadFrame& frame)
{
    Replayable<1>(frame, (unsigned char&)mDirt);
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mPosition.f.x));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mPosition.f.y));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mPosition.f.z));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mBip01Position.f.x));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mBip01Position.f.y));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mBip01Position.f.z));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mHeadPosition.f.x));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mHeadPosition.f.y));
    Replayable<1>(frame, FloatCompressor<-128, 128, 8>(mHeadPosition.f.z));
    Replayable<1>(frame, FloatCompressor<-512, 512, 8>(mVelocity.f.x));
    Replayable<1>(frame, FloatCompressor<-512, 512, 8>(mVelocity.f.y));
    Replayable<1>(frame, FloatCompressor<-512, 512, 8>(mVelocity.f.z));
    Replayable<1>(frame, mVisible);
    Replayable<1>(frame, mFacingDirection);
    Replayable<1>(frame, mHeadSpin);
    Replayable<1>(frame, mHeadTilt);
    Replayable<1>(frame, (unsigned long&)mEffectsTexturing);
    ReplayablePolymorphic<1>(frame, mPoseTree);

    if (frame.mInterval == 1)
    {
        mPoseAccumulator->InitAccumulators();
        mPoseTree->Evaluate(1.0f, mPoseAccumulator);

        nlMatrix4 worldMatrix;
        float angle = 0.0000958738f * (float)mFacingDirection;
        nlMakeRotationMatrixZ(worldMatrix, angle);
        worldMatrix.SetRow_(3, mPosition);

        if (mCharacter != nullptr)
        {
            mPoseAccumulator->SetBuildNodeMatrixCallback(mCharacter->m_nHeadJointIndex, DrawableCharacterHeadTrackCallback, (unsigned int)this, 0);
        }
        else if (mBowser != nullptr)
        {
            mPoseAccumulator->SetBuildNodeMatrixCallback(mBowser->mnHeadJointIndex, DrawableBowserHeadTrackCallback, (unsigned int)this, 0);
        }

        mPoseAccumulator->BuildNodeMatrices(worldMatrix);

        if (mCharacter != nullptr)
        {
            mPoseAccumulator->SetBuildNodeMatrixCallback(mCharacter->m_nHeadJointIndex, nullptr, 0, 0);
        }
        else if (mBowser != nullptr)
        {
            mPoseAccumulator->SetBuildNodeMatrixCallback(mBowser->mnHeadJointIndex, nullptr, 0, 0);
        }

        delete mPoseTree;
        mPoseTree = nullptr;
    }
}

/**
 * Offset/Address/Size: 0x3DC | 0x8011D3F4 | size: 0x20
 */
template <>
void ReplayablePolymorphic<0, LoadFrame, cPoseNode>(LoadFrame& frame, cPoseNode*& ptr)
{
    frame.ReplayablePolymorphicPtr<0, cPoseNode>(ptr);
}

/**
 * Address/Size: 0x8011CBE4 | size: 0x20
 */
template <>
void ReplayablePolymorphic<1, LoadFrame, cPoseNode>(LoadFrame& frame, cPoseNode*& ptr)
{
    FORCE_DONT_INLINE;
    frame.ReplayablePolymorphicPtr<1, cPoseNode>(ptr);
}

/**
 * Offset/Address/Size: 0x438 | 0x8011C764 | size: 0x45C
 * TODO: 97.0% match - first nlMatrix4 copy loop initializes byte offset with li r8,0 instead of mr r8,r7
 */
cPoseAccumulator::cPoseAccumulator(const cPoseAccumulator& other)
{
    m_BaseSHierarchy = other.m_BaseSHierarchy;

    int i;
    int byteOffset;

    m_NodeMatrices.mData = (nlMatrix4*)nlMalloc(other.m_NodeMatrices.mSize * sizeof(nlMatrix4), 8, 0);
    m_NodeMatrices.mSize = other.m_NodeMatrices.mSize;
    m_NodeMatrices.mCapacity = other.m_NodeMatrices.mSize;
    for (i = 0, byteOffset = i; i < m_NodeMatrices.mSize; i++, byteOffset += sizeof(nlMatrix4))
    {
        *(nlMatrix4*)((char*)m_NodeMatrices.mData + byteOffset) = *(nlMatrix4*)((char*)other.m_NodeMatrices.mData + byteOffset);
    }

    m_PrevNodeMatrices.mData = (nlMatrix4*)nlMalloc(other.m_PrevNodeMatrices.mSize * sizeof(nlMatrix4), 8, 0);
    m_PrevNodeMatrices.mSize = other.m_PrevNodeMatrices.mSize;
    m_PrevNodeMatrices.mCapacity = other.m_PrevNodeMatrices.mSize;
    for (i = 0, byteOffset = i; i < m_PrevNodeMatrices.mSize; i++, byteOffset += sizeof(nlMatrix4))
    {
        *(nlMatrix4*)((char*)m_PrevNodeMatrices.mData + byteOffset) = *(nlMatrix4*)((char*)other.m_PrevNodeMatrices.mData + byteOffset);
    }

    m_rot.mData = (RotAccum*)nlMalloc(other.m_rot.mSize * sizeof(RotAccum), 8, 0);
    m_rot.mSize = other.m_rot.mSize;
    m_rot.mCapacity = other.m_rot.mSize;
    for (i = 0; i < m_rot.mSize; i++)
    {
        m_rot.mData[i] = other.m_rot.mData[i];
    }

    m_scale.mData = (ScaleAccum*)nlMalloc(other.m_scale.mSize * sizeof(ScaleAccum), 8, 0);
    m_scale.mSize = other.m_scale.mSize;
    m_scale.mCapacity = other.m_scale.mSize;
    for (i = 0; i < m_scale.mSize; i++)
    {
        m_scale.mData[i] = other.m_scale.mData[i];
    }

    m_trans.mData = (TransAccum*)nlMalloc(other.m_trans.mSize * sizeof(TransAccum), 8, 0);
    m_trans.mSize = other.m_trans.mSize;
    m_trans.mCapacity = other.m_trans.mSize;
    for (i = 0; i < m_trans.mSize; i++)
    {
        m_trans.mData[i] = other.m_trans.mData[i];
    }

    {
        int n = other.m_cb.mSize;
        m_cb.mData = new (nlMalloc(n * sizeof(cBuildNodeMatrixCallbackInfo) + 0x10, 8, 0)) cBuildNodeMatrixCallbackInfo[n];
        m_cb.mSize = n;
        m_cb.mCapacity = n;
        for (i = 0; i < m_cb.mSize; i++)
        {
            m_cb.mData[i] = other.m_cb.mData[i];
        }
    }

    m_MorphWeights.mData = (float*)nlMalloc(other.m_MorphWeights.mSize * sizeof(float), 8, 0);
    m_MorphWeights.mSize = other.m_MorphWeights.mSize;
    m_MorphWeights.mCapacity = other.m_MorphWeights.mSize;
    for (i = 0; i < m_MorphWeights.mSize; i++)
    {
        m_MorphWeights.mData[i] = other.m_MorphWeights.mData[i];
    }
}

/**
 * Offset/Address/Size: 0x2D50 | 0x8011BC00 | size: 0x4C
 */
DrawableCharacter::DrawableCharacter()
{
    mVisible = true;

    mPoseTree = nullptr;
    mPoseAccumulator = nullptr;
    mEffectsTexturing = nullptr;
    mCharacter = nullptr;
    mBowser = nullptr;

    mPosition.f.x = 0.0f;
    mPosition.f.y = 0.0f;
    mPosition.f.z = 0.0f;
    mBip01Position.f.x = 0.0f;
    mBip01Position.f.y = 0.0f;
    mBip01Position.f.z = 0.0f;
    mHeadPosition.f.x = 0.0f;
    mHeadPosition.f.y = 0.0f;
    mHeadPosition.f.z = 0.0f;
}

/**
 * Offset/Address/Size: 0x2C6C | 0x8011BB1C | size: 0xE4
 */
DrawableCharacter::~DrawableCharacter()
{
    delete mPoseAccumulator;
}

/**
 * Offset/Address/Size: 0x2BA4 | 0x8011BA54 | size: 0xC8
 */
void DrawableCharacter::Free()
{
    delete mPoseAccumulator;
    mPoseAccumulator = NULL;
}

/**
 * Offset/Address/Size: 0x2B98 | 0x8011BA48 | size: 0xC
 */
cPN_SAnimController& DrawableCharacter::GetAnimController() const
{
    return *mCharacter->m_pCurrentAnimController;
}

/**
 * Offset/Address/Size: 0x2A1C | 0x8011B8CC | size: 0x17C
 */
void DrawableCharacter::Grab(cCharacter& character)
{
    mCharacter = &character;
    mBowser = NULL;
    mPosition = character.m_v3Position;
    mBip01Position = character.GetJointPosition(character.m_nBip01JointIndex_0xA4);
    mHeadPosition = character.GetJointPosition(character.m_nHeadJointIndex);
    mHeight = mBip01Position.f.z;
    mVelocity = character.m_v3Velocity;
    mFacingDirection = character.m_aActualFacingDirection;
    mHeadSpin = (unsigned short)character.m_pHeadTrack->m_fHeadSpin;
    mHeadTilt = (unsigned short)character.m_pHeadTrack->m_fHeadTilt;
    mPoseTree = character.m_pPoseTree;
    mVisible = true;
    mDirt = (unsigned char)(255.0f * character.m_Dirt);

    if (mPoseAccumulator == NULL)
    {
        cPoseAccumulator* p = (cPoseAccumulator*)nlMalloc(sizeof(cPoseAccumulator), 8, false);
        p = new (p) cPoseAccumulator(*character.m_pPoseAccumulator);
        mPoseAccumulator = p;
    }
    else
    {
        *mPoseAccumulator = *character.m_pPoseAccumulator;
    }

    EffectsTexturing* tex = character.m_pEffectsTexturing;
    if (tex == NULL)
    {
        tex = fxGetTexturing(eFXTex_Nothing);
    }
    mEffectsTexturing = tex;
}

/**
 * Offset/Address/Size: 0x29F0 | 0x8011B8A0 | size: 0x2C
 */
void DrawableCharacterHeadTrackCallback(unsigned int ctx, unsigned int, cPoseAccumulator* poseAccumulator, unsigned int headNodeIndex, int unused)
{
    DrawableCharacter* pDrawableCharacter = (DrawableCharacter*)ctx;
    CalcHeadTrackMatrix(pDrawableCharacter->mHeadSpin, pDrawableCharacter->mHeadTilt, poseAccumulator, headNodeIndex);
}

/**
 * Offset/Address/Size: 0x290C | 0x8011B7BC | size: 0xE4
 */
void DrawableCharacter::DrawableBowserHeadTrackCallback(unsigned int ctx, unsigned int, cPoseAccumulator* poseAccumulator, unsigned int headNodeIndex, int)
{
    DrawableCharacter* pDrawableCharacter = (DrawableCharacter*)ctx;
    nlMatrix4& nodeMatrix = poseAccumulator->GetNodeMatrix(headNodeIndex);
    pDrawableCharacter->mBowser->mLastHeadMatrix = nodeMatrix;
    CalcHeadTrackMatrix(pDrawableCharacter->mHeadSpin, pDrawableCharacter->mHeadTilt, poseAccumulator, headNodeIndex);
}

/**
 * Offset/Address/Size: 0x27F4 | 0x8011B6A4 | size: 0x118
 */
void DrawableCharacter::BuildNodeMatrices()
{
    nlMatrix4 worldMatrix;
    float angle = 0.0000958738f * (float)mFacingDirection;
    nlMakeRotationMatrixZ(worldMatrix, angle);

    worldMatrix.SetRow_(3, mPosition);

    if (mCharacter != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(mCharacter->m_nHeadJointIndex, DrawableCharacterHeadTrackCallback, (unsigned int)this, 0);
    }
    else
    {
        if (mBowser != nullptr)
        {
            mPoseAccumulator->SetBuildNodeMatrixCallback(mBowser->mnHeadJointIndex, DrawableBowserHeadTrackCallback, (unsigned int)this, 0);
        }
    }

    mPoseAccumulator->BuildNodeMatrices(worldMatrix);

    if (mCharacter != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(
            mCharacter->m_nHeadJointIndex,
            nullptr,
            0,
            0);
    }
    else if (mBowser != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(
            mBowser->mnHeadJointIndex,
            nullptr,
            0,
            0);
    }
}

/**
 * Offset/Address/Size: 0x26C4 | 0x8011B574 | size: 0x130
 */
void DrawableCharacter::Render(cCharacter& character) const
{
    if (!mVisible)
    {
        return;
    }

    character.PoseSkinMesh(mPoseAccumulator);

    if (mCharacter->m_pPropModel != NULL)
    {
        nlMatrix4& nodeMatrix = mPoseAccumulator->GetNodeMatrix(character.m_nPropJointIndex);
        mCharacter->m_pPropModel->m_worldMatrix = nodeMatrix;
    }

    cCharacter* renderOnly = spRenderOnlyThisCharacter;
    if (renderOnly != NULL && renderOnly != &character)
    {
        if (!sbRenderOpposingGoalieToo)
        {
            return;
        }
        cTeam* otherTeam = ((cPlayer*)renderOnly)->m_pTeam->GetOtherTeam();
        if (&character != (cCharacter*)otherTeam->GetGoalie())
        {
            return;
        }
    }

    SendToGl(character);
}

static const float kBigFloat = 1e38f;

/**
 * Offset/Address/Size: 0x22EC | 0x8011B19C | size: 0x3D8
 */
static void FindBoundingSphereAccurate(nlVector3* pOutSphere, float* pOutRadius, int numVertices, const nlVector3* pVertices)
{
    nlVector3 minXPt, maxXPt, minYPt, maxYPt, minZPt, maxZPt;
    nlVector3 span1, span2;
    const nlVector3* p;
    int i;
    float radiusSq;

    minZPt.f.z = kBigFloat;
    minYPt.f.y = kBigFloat;
    minXPt.f.x = kBigFloat;
    maxZPt.f.z = -kBigFloat;
    maxYPt.f.y = -kBigFloat;
    maxXPt.f.x = -kBigFloat;

    p = pVertices;
    for (i = 0; i < numVertices; i++, p++)
    {
        if (p->f.x < minXPt.f.x)
        {
            minXPt = *p;
        }
        if (p->f.x > maxXPt.f.x)
        {
            maxXPt = *p;
        }
        if (p->f.y < minYPt.f.y)
        {
            minYPt = *p;
        }
        if (p->f.y > maxYPt.f.y)
        {
            maxYPt = *p;
        }
        if (p->f.z < minZPt.f.z)
        {
            minZPt = *p;
        }
        if (p->f.z > maxZPt.f.z)
        {
            maxZPt = *p;
        }
    }

    float xSpanDistSq = nlGetLengthSquared3D(maxXPt.f.x - minXPt.f.x, maxXPt.f.y - minXPt.f.y, maxXPt.f.z - minXPt.f.z);
    float ySpanDistSq = nlGetLengthSquared3D(maxYPt.f.x - minYPt.f.x, maxYPt.f.y - minYPt.f.y, maxYPt.f.z - minYPt.f.z);
    float zSpanDistSq = nlGetLengthSquared3D(maxZPt.f.x - minZPt.f.x, maxZPt.f.y - minZPt.f.y, maxZPt.f.z - minZPt.f.z);

    span1 = minXPt;
    span2 = maxXPt;
    if (ySpanDistSq > xSpanDistSq)
    {
        xSpanDistSq = ySpanDistSq;
        span1 = minYPt;
        span2 = maxYPt;
    }
    if (zSpanDistSq > xSpanDistSq)
    {
        span1 = minZPt;
        span2 = maxZPt;
    }

    nlVec3Set(*pOutSphere, 0.5f * (span1.f.x + span2.f.x), 0.5f * (span1.f.y + span2.f.y), 0.5f * (span1.f.z + span2.f.z));

    nlVector3 result;
    nlVec3Sub(result, span2, *pOutSphere);
    radiusSq = nlGetLengthSquared3D(result.f.x, result.f.y, result.f.z);

    *pOutRadius = nlSqrt(radiusSq, false);

    for (i = 0; i < numVertices; pVertices++, i++)
    {
        float distSq = nlGetLengthSquared3D(pVertices->f.x - pOutSphere->f.x, pVertices->f.y - pOutSphere->f.y, pVertices->f.z - pOutSphere->f.z);
        if (distSq > radiusSq)
        {
            float dist = nlSqrt(distSq, false);
            *pOutRadius = 0.5f * (*pOutRadius + dist);
            float d = dist - *pOutRadius;
            radiusSq = *pOutRadius * *pOutRadius;
            pOutSphere->f.x = (*pOutRadius * pOutSphere->f.x + d * pVertices->f.x) / dist;
            pOutSphere->f.y = (*pOutRadius * pOutSphere->f.y + d * pVertices->f.y) / dist;
            pOutSphere->f.z = (*pOutRadius * pOutSphere->f.z + d * pVertices->f.z) / dist;
        }
    }
}

/**
 * Offset/Address/Size: 0x1AE4 | 0x8011A994 | size: 0x808
 * TODO: 99.42% match - detail texture load/register order and debug sphere model save.
 */
void DrawableCharacter::SendToGl(const cCharacter& character) const
{
    extern GLInventory glInventory;
    extern unsigned long ResolvedWhiteTexture;

    ProjectedShadowParams params;
    nlMatrix4 mWorld;
    nlMatrix4 sphereWorldMatrix;
    EffectsTexturing* fxtex = mEffectsTexturing;
    eCharacterClass ec = character.m_eCharacterClass;

    if (fxtex != nullptr && fxtex->m_uTexture == 0xFFFFFFFF)
    {
        fxtex = nullptr;
    }

    u8 hasDetail = 0;
    if (fxtex != nullptr && fxtex->m_bDetail)
    {
        hasDetail = 1;
    }

    unsigned long program = CharacterDirtProgram;
    if (hasDetail != 0)
    {
        program = CharacterProgram;
    }

    GLSkinMesh* skinMesh = character.GetSkinMesh();
    skinMesh->PrepareToRender(program, nullptr);

    if (ec == DONKEYKONG || ec == MYSTERY)
    {
        for (glModelPacket* pPacket = skinMesh->pModel->packets; pPacket < skinMesh->pModel->packets + skinMesh->pModel->numPackets; pPacket++)
        {
            glSetRasterState(pPacket->state.raster, GLS_Culling, 0);
        }
    }

    u32 lightTexture;
    World* world = WorldManager::s_World;
    if (DrawableCharacter::sSTSLighting != 0)
    {
        lightTexture = world->m_GlobalLightRampSTSTex;
    }
    else
    {
        lightTexture = GetGameObjectLightRamp();
    }

    void* pSpecularData;
    void* pLightData;
    if (DrawableCharacter::sSTSLighting != 0)
    {
        pLightData = world->m_pSpecularData;
    }
    else if (DrawableCharacter::sCameraRelativeLighting || AlwaysUseCameraRelativeCharacterLighting())
    {
        pLightData = GetCameraRelativeLightData();
    }
    else
    {
        pLightData = GetInGameLightData();
    }

    void* pEnviroData = nullptr;
    if (fxtex != nullptr && fxtex->m_bEnviro)
    {
        pEnviroData = glUserAlloc((eGLUserData)0xE, 0, false);
    }

    pSpecularData = WorldManager::s_World->m_pSTSIntensity;
    glModel* pModel = glModelDup(skinMesh->pModel, true);

    bool isVisible;
    if (nlTaskManager::m_pInstance->m_CurrState == 0x100)
    {
        isVisible = 1;
    }
    else if (WorldManager::s_World != nullptr)
    {
        float fRadius;
        if (ec == DONKEYKONG)
        {
            fRadius = 3.5f;
        }
        else
        {
            fRadius = 2.5f;
        }

        mWorld.SetIdentity();
        mWorld.f.m41 = mBip01Position.f.x;
        mWorld.f.m42 = mBip01Position.f.y;
        mWorld.f.m43 = mBip01Position.f.z;
        mWorld.f.m44 = 1.0f;

        isVisible = WorldManager::s_World->IsSphereInFrustum(mWorld, fRadius);
    }
    else
    {
        isVisible = 1;
    }

    if (isVisible)
    {
        glModelPacket* pPacket = pModel->packets;
        u32 dirtValue = (u32)(u8)(int)(63.0f * (1.0f - ((float)mDirt / 255.0f)));

        for (; pPacket < pModel->packets + pModel->numPackets; pPacket++)
        {
            if (pLightData != nullptr)
            {
                glUserAttach(pLightData, pPacket, false);
            }

            if (pPacket->state.texconfig & 0x10)
            {
                nlBreak();
                glUserAttach(pSpecularData, pPacket, false);
            }

            pPacket->state.texture[GLTT_BumpLocal] = lightTexture;
            pPacket->state.texconfig |= GLTT_BumpLocal_bit;

            if (fxtex != nullptr)
            {
                if (fxtex->m_eBlendMode != GLB_None)
                {
                    glSetRasterState(pPacket->state.raster, GLS_AlphaBlend, fxtex->m_eBlendMode);
                }

                if (fxtex->m_bDetail)
                {
                    if (ec == MYSTERY)
                    {
                        pPacket->state.texture[GLTT_Diffuse] = fxtex->m_uTexture;
                    }
                    else
                    {
                        u32 detailTexture = fxtex->m_uTexture;
                        u32 detailBit = GLTT_Detail_bit;
                        pPacket->state.texture[GLTT_Detail] = detailTexture;
                        pPacket->state.texconfig = detailBit | pPacket->state.texconfig;
                        glSetTextureState(pPacket->state.texturestate, (eGLTextureState)0xC, 0xF);
                    }
                }
                else
                {
                    pPacket->state.texture[GLTT_Diffuse] = fxtex->m_uTexture;
                }

                if (pEnviroData != nullptr)
                {
                    glUserAttach(pEnviroData, pPacket, false);
                }
            }

            if ((pPacket->state.texconfig & 0x2) && (fxtex == nullptr || !fxtex->m_bDetail))
            {
                glSetTextureState(pPacket->state.texturestate, (eGLTextureState)0xC, dirtValue);
            }
        }

        u8 isMapped = 0;
        if (character.m_uNormalTextureID != character.m_uSwapTextureID)
        {
            gl_ModifyAddMapping(eGLModifier_1, character.m_uNormalTextureID, character.m_uSwapTextureID);
            isMapped = 1;
        }

        if (fxtex == nullptr)
        {
            character.PerformBlinking(skinMesh, pModel);
        }

        glViewAttachModel(GLV_Characters, pModel);

        if (isMapped != 0)
        {
            gl_ModifyClearLastMapping();
        }
    }

    if (g_nShowBones > 0)
    {
        static u32 tDiff;
        {
            static s8 init;
            if (!init)
            {
                tDiff = 0;
                init = 1;
            }
        }
        static u32 counter;
        {
            static s8 init;
            if (!init)
            {
                counter = 0;
                init = 1;
            }
        }

        bool endpointBounds = (g_nShowBones == 1);
        PhysicsCharacterBase* pPhysicsCharacter = character.m_pPhysicsCharacter;
        int numBoneVolumePoints = pPhysicsCharacter->GetNumBoneVolumePoints(endpointBounds);

        if (numBoneVolumePoints <= 0xC0)
        {
            u32 startTick = nlGetTicker();

            nlVector3 points[0xC0];
            pPhysicsCharacter->GetBoneVolumePoints(points, endpointBounds);

            nlVector3 vCenter;
            float fRadius;
            if (g_bSloppyBounds != 0)
            {
                vCenter = mBip01Position;
                float maxDistSq = 0.0f;

                for (int i = 0; i < numBoneVolumePoints; i++)
                {
                    float distSq = nlGetLengthSquared3D(
                        points[i].f.x - vCenter.f.x,
                        points[i].f.y - vCenter.f.y,
                        points[i].f.z - vCenter.f.z);
                    maxDistSq = (maxDistSq >= distSq) ? maxDistSq : distSq;
                }

                fRadius = nlSqrt(maxDistSq, false);
            }
            else
            {
                FindBoundingSphereAccurate(&vCenter, &fRadius, numBoneVolumePoints, points);
            }

            u32 endTick = nlGetTicker();
            u32 tickDiff = nlSubtractTicks(startTick, endTick);

            tDiff += tickDiff;
            if (++counter >= 0x1E0)
            {
                float ms = nlGetTickerDifference(0, tDiff);
                ms = ms / (float)counter;
                ms = 8.0f * ms;
                u32 avgTicks = tDiff / counter;
                tDiff = avgTicks;
                OSReport("%u avg ticks (%0.3fms for 8 chars) to find bounding sphere\n", avgTicks, ms);
                tDiff = 0;
                counter = 0;
            }

            float sphereRadius = fRadius;
            u32 debugColour = debugColourValue;
            void* pConstantColour;
            glModelPacket* pSpherePacket;
            unsigned long sphereHash = nlStringHash("debug/sphere");
            glModel* pSphereModel = glModelDup(glInventory.GetModel(sphereHash), true);

            sphereWorldMatrix.SetIdentity();
            sphereWorldMatrix.f.m41 = vCenter.f.x;
            sphereWorldMatrix.f.m42 = vCenter.f.y;
            sphereWorldMatrix.f.m43 = vCenter.f.z;
            sphereWorldMatrix.f.m44 = 1.0f;
            sphereWorldMatrix.f.m11 = sphereRadius;
            sphereWorldMatrix.f.m22 = sphereRadius;
            sphereWorldMatrix.f.m33 = sphereRadius;

            unsigned long matrix = glAllocMatrix();
            if (matrix != 0xFFFFFFFF)
            {
                glSetMatrix(matrix, sphereWorldMatrix);
            }

            pConstantColour = glUserAlloc(GLUD_ConstantColour, 4, false);
            u32* pDebugColour = (u32*)glUserGetData(pConstantColour);
            u8 alpha = ((u8*)&debugColour)[3];
            *pDebugColour = debugColour;
            for (pSpherePacket = pSphereModel->packets; pSpherePacket < pSphereModel->packets + pSphereModel->numPackets; pSpherePacket++)
            {
                pSpherePacket->state.matrix = matrix;
                pSpherePacket->state.texture[GLTT_Diffuse] = ResolvedWhiteTexture;
                if (alpha != 0xFF)
                {
                    glSetRasterState(pSpherePacket->state.raster, GLS_AlphaBlend, GLB_Standard);
                }
                glUserAttach(pConstantColour, pSpherePacket, false);
            }

            glViewAttachModel(GLV_Characters, 6, pSphereModel);
        }
    }

    if (sShadowRenderingDisabled__17DrawableCharacter == 0)
    {
        LightObject* pLight = ((BasicStadium*)WorldManager::s_World)->m_pShadowLight;
        if (pLight != nullptr)
        {
            static float s_fHeightFudge;
            {
                static s8 init;
                if (!init)
                {
                    s_fHeightFudge = 1.125f;
                    init = 1;
                }
            }

            params.fScalar = 1.0f;
            float fRadius;
            float fHeight;
            int characterSizeIndex;

            if (ec < NUM_FIELDER_CLASSES)
            {
                const ShadowScale& ss = shadowScale[ec];
                fRadius = ss.fRadius;
                fHeight = ss.fHeight;
                params.fScalar = ss.fScalar;
                characterSizeIndex = charSizes[ec];
            }
            else
            {
                const ShadowScale& ss = shadowScale[NUM_FIELDER_CLASSES];
                fRadius = ss.fRadius;
                fHeight = ss.fHeight;
                params.fScalar = ss.fScalar;
                characterSizeIndex = charSizes[NUM_FIELDER_CLASSES];
            }

            nlVec4Set(params.vLight, pLight->m_worldPosition.f.x, pLight->m_worldPosition.f.y, pLight->m_worldPosition.f.z, 1.0f);
            params.vPosition = mBip01Position;
            params.fRadius = g_fRadiusScale * fRadius;
            params.fHeight = s_fHeightFudge * fHeight;
            params.fWidth = params.fHeight;
            params.pModel = pModel;
            params.nPartitionIndex = ((int (*)())GetShadowPartitionIndex)();
            params.nVisibleInterval = g_nOnscreenUpdate[characterSizeIndex];
            params.nInvisibleInterval = g_nOffscreenUpdate[characterSizeIndex];

            if (ShouldShadowBeUpdated(params))
            {
                params.pModel = glModelDupNoStreams(pModel, true, false);
                RenderCharacterIntoTexture(params);
            }

            RenderProjectedShadow(params);
        }
    }
}
// /**
//  * Offset/Address/Size: 0xA6C | 0x8011991C | size: 0x1078
//  */
// void cPoseAccumulator::operator=(const cPoseAccumulator&)
// {
// }

/**
 * Offset/Address/Size: 0x924 | 0x801197D4 | size: 0x148
 */
void DrawableCharacter::Grab(SkinAnimatedMovableNPC& character)
{
    mCharacter = nullptr;

    if (static_cast<SkinAnimatedNPC&>(character).GetSkinAnimatedNPC_Type() == SkinAnimatedNPC_BOWSER)
    {
        mBowser = (Bowser*)&character;
    }

    mPosition = character.mv3Position;

    nlMatrix4& nodeMatrix = character.mpPoseAccumulator->GetNodeMatrix(0);
    mHeight = nodeMatrix.f.m43;

    mVelocity.f.x = 0.0f;
    mVelocity.f.y = 0.0f;
    mVelocity.f.z = 0.0f;

    mFacingDirection = character.maFacingDirection;

    float headSpin = character.GetHeadSpin();
    mHeadSpin = (unsigned short)(int)headSpin;

    float headTilt = character.GetHeadTilt();
    mHeadTilt = (unsigned short)(int)headTilt;

    mPoseTree = character.mpPoseTree;
    mVisible = character.mbIsVisible;
    mDirt = false;

    if (mPoseAccumulator == nullptr)
    {
        mPoseAccumulator = (cPoseAccumulator*)new (nlMalloc(0x58, 8, false)) cPoseAccumulator(*character.mpPoseAccumulator);
    }
    else
    {
        *mPoseAccumulator = *character.mpPoseAccumulator;
    }

    mEffectsTexturing = nullptr;
}

/**
 * Offset/Address/Size: 0x87C | 0x8011972C | size: 0xA8
 */
void DrawableCharacter::Render(SkinAnimatedMovableNPC& character) const
{
    if (!mVisible)
    {
        return;
    }

    nlMatrix4 rotMatrix;
    float angle = 0.0000958738f * (float)mFacingDirection;
    nlMakeRotationMatrixZ(rotMatrix, angle);

    rotMatrix.SetRow_(3, mPosition);

    character.mbIsVisible = mVisible;
    character.RenderFromReplay(*mPoseAccumulator, &rotMatrix);
}

/**
 * Offset/Address/Size: 0x2B8 | 0x80119168 | size: 0x5C4
 * TODO: 96.25% match - pose accumulator/stride register ordering and morph-weight index scheduling remain.
 */
void DrawableCharacter::Blend(const float* blendFactors, const DrawableCharacter& lhs, const DrawableCharacter& rhs)
{
    mVisible = lhs.mVisible && rhs.mVisible;
    mCharacter = lhs.mCharacter;
    mBowser = lhs.mBowser;
    mDirt = lhs.mDirt;
    mPosition.f.x = (1.0f - *blendFactors) * lhs.mPosition.f.x + *blendFactors * rhs.mPosition.f.x;
    mPosition.f.y = (1.0f - *blendFactors) * lhs.mPosition.f.y + *blendFactors * rhs.mPosition.f.y;
    mPosition.f.z = (1.0f - *blendFactors) * lhs.mPosition.f.z + *blendFactors * rhs.mPosition.f.z;
    mBip01Position.f.x = (1.0f - *blendFactors) * lhs.mBip01Position.f.x + *blendFactors * rhs.mBip01Position.f.x;
    mBip01Position.f.y = (1.0f - *blendFactors) * lhs.mBip01Position.f.y + *blendFactors * rhs.mBip01Position.f.y;
    mBip01Position.f.z = (1.0f - *blendFactors) * lhs.mBip01Position.f.z + *blendFactors * rhs.mBip01Position.f.z;
    mHeadPosition.f.x = (1.0f - *blendFactors) * lhs.mHeadPosition.f.x + *blendFactors * rhs.mHeadPosition.f.x;
    mHeadPosition.f.y = (1.0f - *blendFactors) * lhs.mHeadPosition.f.y + *blendFactors * rhs.mHeadPosition.f.y;
    mHeadPosition.f.z = (1.0f - *blendFactors) * lhs.mHeadPosition.f.z + *blendFactors * rhs.mHeadPosition.f.z;
    float t = *blendFactors;
    mFacingDirection = lhs.mFacingDirection + (short)(t * (float)(short)(rhs.mFacingDirection - lhs.mFacingDirection));
    mHeadSpin = lhs.mHeadSpin + (short)(t * (float)(short)(rhs.mHeadSpin - lhs.mHeadSpin));
    mHeadTilt = lhs.mHeadTilt + (short)(t * (float)(short)(rhs.mHeadTilt - lhs.mHeadTilt));
    mPoseTree = nullptr;
    if (mPoseAccumulator == nullptr)
    {
        mPoseAccumulator = new (nlMalloc(sizeof(cPoseAccumulator), 8, false)) cPoseAccumulator(lhs.mPoseAccumulator->m_BaseSHierarchy, false);
    }
    mPoseAccumulator->InitAccumulators();
    float oneMinusT;
    const float rhsWeight = *blendFactors;
    const float one = 1.0f;
    oneMinusT = one - rhsWeight;
    RotAccum* lhsRot;
    RotAccum* rhsRot;
    cPoseAccumulator* lhsAccumulator = lhs.mPoseAccumulator;
    cPoseAccumulator* rhsAccumulator = rhs.mPoseAccumulator;
    for (int i = 0; i < mPoseAccumulator->GetNumNodes(); i++)
    {
        lhsRot = &lhsAccumulator->m_rot.mData[i];
        rhsRot = &rhsAccumulator->m_rot.mData[i];
        float rhsRotAroundZWeight = rhsRot->rotAroundZAccumulatedWeight * rhsWeight;
        mPoseAccumulator->BlendRotAroundZ(i, lhsRot->rotAroundZ, lhsRot->rotAroundZAccumulatedWeight * oneMinusT);
        mPoseAccumulator->BlendRotAroundZ(i, rhsRot->rotAroundZ, rhsRotAroundZWeight);
        float rhsQuatWeight = rhsRot->quatAccumulatedWeight * rhsWeight;
        mPoseAccumulator->BlendRot(i, &lhsRot->q, lhsRot->quatAccumulatedWeight * oneMinusT, false);
        mPoseAccumulator->BlendRot(i, &rhsRot->q, rhsQuatWeight, false);
        mPoseAccumulator->BlendTrans(i, &lhsAccumulator->m_trans.mData[i].t, 1.0f - *blendFactors, false);
        mPoseAccumulator->BlendTrans(i, &rhsAccumulator->m_trans.mData[i].t, *blendFactors, false);
        mPoseAccumulator->BlendScale(i, &lhsAccumulator->m_scale.mData[i].s, 1.0f - *blendFactors, false);
        mPoseAccumulator->BlendScale(i, &rhsAccumulator->m_scale.mData[i].s, *blendFactors, false);
    }
    oneMinusT = one - *blendFactors;
    t = *blendFactors;
    cPoseAccumulator* lhsPoseAccum = lhs.mPoseAccumulator;
    cPoseAccumulator* rhsPoseAccum = rhs.mPoseAccumulator;
    int j = 0;
    for (int i = 0; i < 2; i++)
    {
        mPoseAccumulator->m_MorphWeights.mData[j] += lhsPoseAccum->m_MorphWeights.mData[j] * oneMinusT;
        mPoseAccumulator->m_MorphWeights.mData[j] += rhsPoseAccum->m_MorphWeights.mData[j] * t;
        mPoseAccumulator->m_MorphWeights.mData[j + 1] += lhsPoseAccum->m_MorphWeights.mData[j + 1] * oneMinusT;
        mPoseAccumulator->m_MorphWeights.mData[j + 1] += rhsPoseAccum->m_MorphWeights.mData[j + 1] * t;
        mPoseAccumulator->m_MorphWeights.mData[j + 2] += lhsPoseAccum->m_MorphWeights.mData[j + 2] * oneMinusT;
        mPoseAccumulator->m_MorphWeights.mData[j + 2] += rhsPoseAccum->m_MorphWeights.mData[j + 2] * t;
        mPoseAccumulator->m_MorphWeights.mData[j + 3] += lhsPoseAccum->m_MorphWeights.mData[j + 3] * oneMinusT;
        mPoseAccumulator->m_MorphWeights.mData[j + 3] += rhsPoseAccum->m_MorphWeights.mData[j + 3] * t;
        j += 4;
    }
    nlMatrix4 rotMatrix;
    nlMakeRotationMatrixZ(rotMatrix, 0.0000958738f * (float)mFacingDirection);
    rotMatrix.m[3][0] = mPosition.f.x;
    rotMatrix.m[3][1] = mPosition.f.y;
    rotMatrix.m[3][2] = mPosition.f.z;
    if (mCharacter != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(mCharacter->m_nHeadJointIndex, DrawableCharacterHeadTrackCallback, (unsigned int)this, 0);
    }
    else if (mBowser != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(mBowser->mnHeadJointIndex, DrawableBowserHeadTrackCallback, (unsigned int)this, 0);
    }
    mPoseAccumulator->BuildNodeMatrices(rotMatrix);
    if (mCharacter != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(mCharacter->m_nHeadJointIndex, nullptr, 0, 0);
    }
    else if (mBowser != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(mBowser->mnHeadJointIndex, nullptr, 0, 0);
    }
    mEffectsTexturing = lhs.mEffectsTexturing;
}

/**
 * Offset/Address/Size: 0xD8 | 0x80118F88 | size: 0x1E0
 */
void DrawableCharacter::EvaluateFrom(const cPoseNode& poseNode, const nlVector3& position, unsigned short facingDirection)
{
    mPosition = position;

    mVelocity.f.x = 0.0f;
    mVelocity.f.y = 0.0f;
    mVelocity.f.z = 0.0f;

    mFacingDirection = facingDirection;
    mHeadSpin = 0;
    mHeadTilt = 0;
    mHeight = 0.0f;

    mPoseAccumulator->InitAccumulators();

    poseNode.Evaluate(1.0f, mPoseAccumulator);

    mEffectsTexturing = fxGetTexturing(eFXTex_Nothing);

    nlMatrix4 rotMatrix;
    float angle = 0.0000958738f * (float)mFacingDirection;
    nlMakeRotationMatrixZ(rotMatrix, angle);
    rotMatrix.SetRow_(3, mPosition);

    if (mCharacter != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(mCharacter->m_nHeadJointIndex, DrawableCharacterHeadTrackCallback, (unsigned int)this, 0);
    }
    else
    {
        if (mBowser != nullptr)
        {
            mPoseAccumulator->SetBuildNodeMatrixCallback(mBowser->mnHeadJointIndex, DrawableBowserHeadTrackCallback, (unsigned int)this, 0);
        }
    }

    mPoseAccumulator->BuildNodeMatrices(rotMatrix);

    if (mCharacter != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(
            mCharacter->m_nHeadJointIndex,
            nullptr,
            0,
            0);
    }
    else if (mBowser != nullptr)
    {
        mPoseAccumulator->SetBuildNodeMatrixCallback(
            mBowser->mnHeadJointIndex,
            nullptr,
            0,
            0);
    }

    nlMatrix4& bip01Matrix = mPoseAccumulator->GetNodeMatrix(mCharacter->m_nBip01JointIndex_0xA4);
    mBip01Position = bip01Matrix.GetTranslation();

    nlMatrix4& headMatrix = mPoseAccumulator->GetNodeMatrix(mCharacter->m_nHeadJointIndex);
    mHeadPosition = headMatrix.GetTranslation();
}

/**
 * Offset/Address/Size: 0x88 | 0x80118F38 | size: 0x50
 */
nlVector3 DrawableCharacter::GetBallPosition() const
{
    cPlayer* pPlayer = (cPlayer*)mCharacter;
    nlMatrix4& matrix = mPoseAccumulator->GetNodeMatrix(pPlayer->m_nBallJointIndex);
    return matrix.GetTranslation();
}

/**
 * Offset/Address/Size: 0x24 | 0x80118ED4 | size: 0x64
 */
nlQuaternion DrawableCharacter::GetBallOrientation() const
{
    nlQuaternion quat;
    nlMatrixToQuat(quat, mPoseAccumulator->GetNodeMatrix(((cPlayer*)mCharacter)->m_nBallJointIndex));
    return quat;
}

/**
 * Offset/Address/Size: 0x18 | 0x80118EC8 | size: 0xC
 */
void DrawableCharacter::RenderOnlyOneCharacter(const cCharacter& character, bool renderOpposingGoalieToo)
{
    spRenderOnlyThisCharacter = (cCharacter*)&character;
    sbRenderOpposingGoalieToo = renderOpposingGoalieToo;
}

/**
 * Offset/Address/Size: 0x8 | 0x80118EB8 | size: 0x10
 */
void DrawableCharacter::RenderAllCharacters()
{
    spRenderOnlyThisCharacter = nullptr;
    sbRenderOpposingGoalieToo = false;
}

/**
 * Offset/Address/Size: 0x0 | 0x80118EB0 | size: 0x8
 */
cCharacter* DrawableCharacter::OnlyRenderingOneCharacter()
{
    return spRenderOnlyThisCharacter;
}

/**
 * Address/Size: 0x8011CC04 | size: 0x50
 */
template <>
void Replayable<1, SaveFrame, unsigned char>(SaveFrame& frame, unsigned char& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(frame.mStream.mStorage, &value, 1);
            frame.mStream.mStorage += 1;
        }
    }
}

/**
 * Address/Size: 0x8011CC54 | size: 0x50
 */
template <>
void Replayable<1, SaveFrame, unsigned short>(SaveFrame& frame, unsigned short& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(unsigned short));
            frame.mStream.mStorage += sizeof(unsigned short);
        }
    }
}

/**
 * Address/Size: 0x8011CCA4 | size: 0x50
 */
template <>
void Replayable<1, SaveFrame, unsigned long>(SaveFrame& frame, unsigned long& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(unsigned long));
            frame.mStream.mStorage += sizeof(unsigned long);
        }
    }
}

/**
 * Address/Size: 0x8011CCF4 | size: 0x54
 */
template <>
void Replayable<1, LoadFrame, unsigned char>(LoadFrame& frame, unsigned char& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(unsigned char));
            frame.mStream.mStorage += sizeof(unsigned char);
        }
    }
}

/**
 * Address/Size: 0x8011CD48 | size: 0x54
 */
template <>
void Replayable<1, LoadFrame, unsigned short>(LoadFrame& frame, unsigned short& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(unsigned short));
            frame.mStream.mStorage += sizeof(unsigned short);
        }
    }
}

/**
 * Address/Size: 0x8011CD9C | size: 0x54
 */
template <>
void Replayable<1, LoadFrame, unsigned long>(LoadFrame& frame, unsigned long& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(unsigned long));
            frame.mStream.mStorage += sizeof(unsigned long);
        }
    }
}

/**
 * Offset/Address/Size: 0x44CC | 0x8011D17C | size: 0x44
 */
#pragma dont_inline on
template <>
void Replayable<0, LoadFrame, unsigned int>(LoadFrame& frame, unsigned int& value)
{
    memcpy(&value, frame.mStream.mStorage, sizeof(unsigned int));
    frame.mStream.mStorage += sizeof(unsigned int);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x4538 | 0x8011D238 | size: 0x40
 */
#pragma dont_inline on
template <>
void Replayable<0, SaveFrame, unsigned int>(SaveFrame& frame, unsigned int& value)
{
    memcpy(frame.mStream.mStorage, &value, sizeof(unsigned int));
    frame.mStream.mStorage += sizeof(unsigned int);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x4564 | 0x8011D414 | size: 0x44
 */
#pragma dont_inline on
template <>
void Replayable<0, LoadFrame, int>(LoadFrame& frame, int& value)
{
    memcpy(&value, frame.mStream.mStorage, sizeof(int));
    frame.mStream.mStorage += sizeof(int);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x45A8 | 0x8011D458 | size: 0x40
 */
#pragma dont_inline on
template <>
void Replayable<0, SaveFrame, int>(SaveFrame& frame, int& value)
{
    memcpy(frame.mStream.mStorage, &value, sizeof(int));
    frame.mStream.mStorage += sizeof(int);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x482C | 0x8011D6EC | size: 0x74
 */
template <>
void cPN_SingleAxisBlender::Replay<LoadFrame>(LoadFrame& frame)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (cPoseNode&)*this);
    const char* cursor = frame.mStream.mStorage;
    float scale = 1.0f / 128.0f;
    unsigned char value = *cursor++;
    frame.mStream.mStorage = cursor;
    m_fSmoothedWeight = (float)value * scale;
}

/**
 * Offset/Address/Size: 0x48A0 | 0x8011D760 | size: 0x80
 */
template <>
void cPN_SingleAxisBlender::Replay<SaveFrame>(SaveFrame& frame)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (cPoseNode&)*this);

    float weight = m_fSmoothedWeight;
    if (weight > 1.0f)
        weight = 1.0f;
    if (weight < 0.0f)
        weight = 0.0f;
    weight *= 128.0f;

    char* cursor = frame.mStream.mStorage;
    *cursor++ = (char)(unsigned int)weight;
    frame.mStream.mStorage = cursor;
}

/**
 * Offset/Address/Size: 0x4920 | 0x8011D7E0 | size: 0xD8
 * TODO: 98.61% match - stream cursor/result registers differ in compressed time byte read.
 */
template <>
void cPN_SAnimController::Replay<LoadFrame>(LoadFrame& frame)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (cPoseNode&)*this);

    const char* cursor = frame.mStream.mStorage;
    unsigned int value = (unsigned int)(unsigned char)cursor[1] << 8;
    value = (value & 0xFF00) | (unsigned int)(unsigned char)cursor[0];
    frame.mStream.mStorage = cursor + 2;
    m_fTime = (float)value / 32768.0f;

    unsigned int animPtr = 0;
    memcpy(&animPtr, frame.mStream.mStorage, sizeof(unsigned int));
    frame.mStream.mStorage += sizeof(unsigned int);
    m_bMirror = animPtr & 1;
    m_pSAnim = (cSAnim*)(animPtr & ~1);
    memcpy(&m_pAnimRetarget, frame.mStream.mStorage, sizeof(unsigned int));
    frame.mStream.mStorage += sizeof(unsigned int);
}

/**
 * Offset/Address/Size: 0x49F8 | 0x8011D8B8 | size: 0xE4
 */
template <>
void cPN_SAnimController::Replay<SaveFrame>(SaveFrame& frame)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (cPoseNode&)*this);

    float time = m_fTime;
    if (time > 1.0f)
        time = 1.0f;
    if (time < 0.0f)
        time = 0.0f;
    time *= 32768.0f;

    unsigned int value = (unsigned int)time;
    char* cursor = frame.mStream.mStorage;
    *cursor++ = (char)value;
    *cursor++ = (char)((value >> 8) & 0xFF);
    frame.mStream.mStorage = cursor;

    unsigned int animPtr = 0;
    animPtr = (unsigned int)m_pSAnim;
    if (m_bMirror)
        animPtr |= 1;
    memcpy(frame.mStream.mStorage, &animPtr, sizeof(unsigned int));
    frame.mStream.mStorage += sizeof(unsigned int);
    memcpy(frame.mStream.mStorage, &m_pAnimRetarget, sizeof(unsigned int));
    frame.mStream.mStorage += sizeof(unsigned int);
}

/**
 * Offset/Address/Size: 0x4AEC | 0x8011D99C | size: 0x44
 */
template <>
void cPN_Feather::Replay<LoadFrame>(LoadFrame& frame)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (cPoseNode&)*this);
    m_fBlendTime = 0.0f;
    m_pFeatherWeights = NULL;
}

/**
 * Offset/Address/Size: 0x4B30 | 0x8011D9E0 | size: 0x2C
 */
template <>
void cPN_Feather::Replay<SaveFrame>(SaveFrame& frame)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (cPoseNode&)*this);
}

/**
 * Offset/Address/Size: 0x4B5C | 0x8011DA0C | size: 0x74
 */
template <>
void cPN_Blender::Replay<LoadFrame>(LoadFrame& frame)
{
    FORCE_DONT_INLINE;
    Replayable<0>(frame, (cPoseNode&)*this);
    const char* cursor = frame.mStream.mStorage;
    float scale = 1.0f / 128.0f;
    unsigned char value = *cursor++;
    frame.mStream.mStorage = cursor;
    m_fBlendTime = (float)value * scale;
}

/**
 * Offset/Address/Size: 0x4CC0 | 0x8011DA80 | size: 0x234
 * TODO: 87.8% match - MWCC inlines FloatCompressor<0,1,7> and <0,1,15>
 * constructors despite explicit specialization declarations in Replay.h;
 * target calls __ct__ externally. This cascades into r26 not allocated for
 * type and r29/r31 swap.
 */
template <>
void cPN_Blender::Replay<SaveFrame>(SaveFrame& frame)
{
    int i;
    cPoseNode* child;
    char type;

    memcpy(frame.mStream.mStorage, &m_numChildren, sizeof(int));
    frame.mStream.mStorage += sizeof(int);

    for (i = 0; i < m_numChildren; i++)
    {
        child = m_children[i];
        bool hasChild = (child != 0);
        memcpy(frame.mStream.mStorage, &hasChild, 1);
        frame.mStream.mStorage += 1;
        if (hasChild)
        {
            type = (char)child->GetType();
            if (type < 0 || type > 4)
                nlBreak();
            memcpy(frame.mStream.mStorage, &type, 1);
            frame.mStream.mStorage += 1;
            if (type < 0 || type > 3)
                nlBreak();
            if (type == 0)
            {
                Replayable<0>(frame, (cPoseNode&)*child);
                Replayable<0>(frame, FloatCompressor<0, 1, 7>(((cPN_Blender*)child)->m_fBlendTime));
            }
            else if (type == 1)
            {
                Replayable<0>(frame, (cPoseNode&)*child);
            }
            else if (type == 2)
            {
                Replayable<0>(frame, (cPoseNode&)*child);
                Replayable<0>(frame, FloatCompressor<0, 1, 15>(((cPN_SAnimController*)child)->m_fTime));
                unsigned int animPtr = 0;
                animPtr = (unsigned int)((cPN_SAnimController*)child)->m_pSAnim;
                if (((cPN_SAnimController*)child)->m_bMirror)
                    animPtr |= 1;
                Replayable<0>(frame, animPtr);
                Replayable<0>(frame, (unsigned int&)((cPN_SAnimController*)child)->m_pAnimRetarget);
            }
            else if (type == 3)
            {
                Replayable<0>(frame, (cPoseNode&)*child);
                Replayable<0>(frame, FloatCompressor<0, 1, 7>(((cPN_SingleAxisBlender*)child)->m_fSmoothedWeight));
            }
        }
    }

    float blendTime = m_fBlendTime;
    if (blendTime > 1.0f)
        blendTime = 1.0f;
    if (blendTime < 0.0f)
        blendTime = 0.0f;
    blendTime *= 128.0f;

    char* cursor = frame.mStream.mStorage;
    *cursor++ = (char)(unsigned int)blendTime;
    frame.mStream.mStorage = cursor;
}

#pragma dont_inline on
template <>
void ReplayablePolymorphic<1, SaveFrame, cPoseNode>(SaveFrame& frame, cPoseNode*& ptr)
{
    // FORCE_DONT_INLINE;
    char typeId;
    cPoseNode* current = ptr;
    if (frame.mInterval == 1)
    {
        unsigned char notNull = (current != 0);
        memcpy(frame.mStream.mStorage, &notNull, 1);
        frame.mStream.mStorage++;

        if (notNull)
        {
            typeId = (char)current->GetType();
            if (typeId < 0 || typeId > 4)
                nlBreak();

            memcpy(frame.mStream.mStorage, &typeId, 1);
            frame.mStream.mStorage++;

            Replayable<1>(frame, typeId, current);
        }
    }
}

template <>
void ReplayablePolymorphic<0, SaveFrame, cPoseNode>(SaveFrame& frame, cPoseNode*& ptr)
{
    char typeId;
    cPoseNode* current = ptr;
    unsigned char notNull = (current != 0);
    memcpy(frame.mStream.mStorage, &notNull, 1);
    frame.mStream.mStorage++;

    if (notNull)
    {
        typeId = (char)current->GetType();
        if (typeId < 0 || typeId > 4)
            nlBreak();

        memcpy(frame.mStream.mStorage, &typeId, 1);
        frame.mStream.mStorage++;

        Replayable<0>(frame, typeId, current);
    }
}
#pragma dont_inline reset

#pragma force_active on
void DrawableCharacter_stub()
{
    float x;
    SaveFrame sf;
    LoadFrame lf;
    cPoseNode* pn = NULL;
    Replayable<0>(sf, FloatCompressor<0, 1, 7>(x));
    Replayable<0>(sf, FloatCompressor<0, 1, 15>(x));
    Replayable<0>(lf, FloatCompressor<0, 1, 7>(x));
    Replayable<0>(lf, FloatCompressor<0, 1, 15>(x));
    Replayable<0>(lf, 0, pn);
    ReplayablePolymorphic<1, SaveFrame, cPoseNode>(sf, pn);
    cPoseAccumulator& (cPoseAccumulator::*fn)(const cPoseAccumulator&) = &cPoseAccumulator::operator=;
    (void)fn;
}
#pragma force_active reset
