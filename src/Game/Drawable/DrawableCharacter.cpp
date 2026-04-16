#include "Game/Drawable/DrawableCharacter.h"
#include "Game/Character.h"
#include "Game/Player.h"
#include "Game/Team.h"
#include "Game/AI/HeadTrack.h"
#include "Game/PoseAccumulator.h"

cCharacter* DrawableCharacter::spRenderOnlyThisCharacter = nullptr;
bool DrawableCharacter::sbRenderOpposingGoalieToo = false;

template <>
void ReplayablePolymorphic<1, LoadFrame, cPoseNode>(LoadFrame& frame, cPoseNode*& ptr);

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
    frame.ReplayablePolymorphicPtr<1, cPoseNode>(ptr);
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

/**
 * Offset/Address/Size: 0x22EC | 0x8011B19C | size: 0x3D8
 * TODO: 98.29% match - register allocation diff in center computation (span2.x reload)
 */
void FindBoundingSphereAccurate(nlVector3* pOutSphere, float* pOutRadius, int numVertices, const nlVector3* pVertices)
{
    nlVector3 minXPt, maxXPt, minYPt, maxYPt, minZPt, maxZPt;
    nlVector3 span1, span2;
    const nlVector3* p;
    int i;
    float radiusSq;

    minZPt.f.z = 1e38f;
    minYPt.f.y = 1e38f;
    minXPt.f.x = 1e38f;
    maxZPt.f.z = -1e38f;
    maxYPt.f.y = -1e38f;
    maxXPt.f.x = -1e38f;

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

    pOutSphere->f.x = 0.5f * (span1.f.x + span2.f.x);
    pOutSphere->f.y = 0.5f * (span1.f.y + span2.f.y);
    pOutSphere->f.z = 0.5f * (span1.f.z + span2.f.z);
    radiusSq = nlGetLengthSquared3D(span2.f.x - pOutSphere->f.x, span2.f.y - pOutSphere->f.y, span2.f.z - pOutSphere->f.z);
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
 */
void DrawableCharacter::SendToGl(const cCharacter&) const
{
    FORCE_DONT_INLINE;
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
 */
void DrawableCharacter::Blend(const float*, const DrawableCharacter&, const DrawableCharacter&)
{
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
