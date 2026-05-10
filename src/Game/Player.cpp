#include "Game/Player.h"
#include "Game/Character.h"
#include "Game/Ball.h"
#include "Game/AnimInventory.h"
#include "Game/CharacterAudio.h"
#include "Game/Net.h"

#include "Game/AI/Fielder.h"
#include "Game/AI/ShotMeter.h"
#include "Game/AI/SpaceSearch.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/Scripts/CommonScript.h"

#include "Game/CharacterTemplate.h"
#include "Game/SAnim/pnFeather.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "NL/nlMath.h"
#include "NL/nlString.h"

#include "Game/DB/StatsTracker.h"
#include "Game/Effects/EffectsGroup.h"
#include "Game/CharacterTriggers.h"
#include "Game/Game.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/Physics/PhysicsColumn.h"
#include "Game/FixedUpdateTask.h"
#include "Game/FormationDefines.h"

float g_fPassInterceptNoPickupTimer = 5.0f;

static u8 sbNoBallPickups;

extern int g_nHeadSpinMax;
extern int g_nHeadTiltMax;
extern int g_nWarioHeadSpinMax;
extern int g_nWarioHeadTiltMax;
extern int g_nWaluigiHeadSpinMax;
extern float g_fFixedUpdateTick;
extern u16 g_aOOIConstraint;

/**
 * Offset/Address/Size: 0x0 | 0x80057550 | size: 0x20
 */
s32 cPlayer::GetUniqueID(int nTeamID) const
{
    if (nTeamID == -1)
    {
        nTeamID = *(s32*)m_pTeam;
    }
    nTeamID *= 5;
    return nTeamID + m_ID;
}

/**
 * Offset/Address/Size: 0x20 | 0x80057570 | size: 0x48
 */
void cPlayer::SetNoPickUpTime(float NewNoPickUpTime)
{
    m_pPhysicsCharacter->m_CanCollideWithBall = (NewNoPickUpTime <= 0.0f);
    m_tNoPickupTimer.SetSeconds(NewNoPickUpTime);
}

inline float max_float(float a, float b)
{
    return (a >= b) ? a : b;
}

inline float min_float(float a, float b)
{
    return (a <= b) ? a : b;
}

/**
 * Offset/Address/Size: 0x68 | 0x800575B8 | size: 0xAC
 */
nlVector3 cPlayer::GetAIDefNetLocation(const nlVector3* v3ReferencePos)
{
    nlVector3 v3NetLocation = m_pTeam->m_pNet->m_baseLocation;
    float yCoord = (v3ReferencePos != NULL) ? v3ReferencePos->f.y : m_v3Position.f.y;

    float fNetWidth = 0.5f * cNet::m_fNetWidth;

    if (yCoord < 0.0f)
    {
        yCoord = max_float(yCoord, -1.0f * fNetWidth);
        v3NetLocation.f.y = yCoord;
    }
    else
    {
        yCoord = min_float(yCoord, fNetWidth);
        v3NetLocation.f.y = yCoord;
    }

    return v3NetLocation;
}

/**
 * Offset/Address/Size: 0x114 | 0x80057664 | size: 0xE0
 */
nlVector3 cPlayer::GetAIOffNetLocation(const nlVector3* v3ReferencePos)
{
    nlVector3 v3NetLocation = m_pTeam->GetOtherNet()->m_baseLocation;
    float yCoord = (v3ReferencePos != NULL) ? v3ReferencePos->f.y : m_v3Position.f.y;

    float fNetWidth = 0.5f * cNet::m_fNetWidth;

    if (yCoord < 0.0f)
    {
        yCoord = max_float(yCoord, -1.0f * fNetWidth);
        v3NetLocation.f.y = yCoord;
    }
    else
    {
        yCoord = min_float(yCoord, fNetWidth);
        v3NetLocation.f.y = yCoord;
    }

    return v3NetLocation;
}

/**
 * Offset/Address/Size: 0x1F4 | 0x80057744 | size: 0x90
 */
bool cPlayer::CanPickupBallFromPass(cBall* pBall)
{
    bool result = false;
    if (pBall->m_pOwner == NULL)
    {
        int jointIdx = m_nBallJointIndex;
        result = TestCollision(0.18f, GetPrevJointPosition(jointIdx), GetJointPosition(jointIdx), 0.18f, pBall->m_v3PrevPosition, pBall->m_v3Position);
    }
    return result;
}

/**
 * Offset/Address/Size: 0x284 | 0x800577D4 | size: 0x1B0
 * TODO: 99.44% match - MWCC scheduling keeps one extra li r30,0 after
 * extsh r0,r3 instead of at 0x158 before the TestCollision beq.
 */
bool cPlayer::CanPickupBall(cBall* pBall)
{
    cPlayer* const self = this;

    if (sbNoBallPickups)
    {
        return false;
    }

    if (!g_pGame->IsGameplayOrOvertime())
    {
        return false;
    }

    bool bDoPickUp = false;
    float speedSq = pBall->m_v3Velocity.f.x * pBall->m_v3Velocity.f.x
                  + pBall->m_v3Velocity.f.y * pBall->m_v3Velocity.f.y
                  + pBall->m_v3Velocity.f.z * pBall->m_v3Velocity.f.z;

    if (pBall->m_pOwner == NULL && pBall->m_tNoPickupTimer.m_uPackedTime == 0)
    {
        if (speedSq <= 900.0f || pBall->m_pPassTarget == self)
        {
            if (self->m_tNoPickupPassInterceptTimer.m_uPackedTime == 0
                && self->m_tNoPickupTimer.m_uPackedTime == 0
                && self->m_eAnimID != 0x12 && self->m_eAnimID != 0x13)
            {
                float fPhysicsRadius = 0.0f;
                self->m_pPhysicsCharacter->m_pPlayerPlayerColumn->GetRadius(&fPhysicsRadius);

                nlVector3 v3PrevPlayerPos = self->m_v3PrevPosition;
                nlVector3 v3PlayerPos = self->m_v3Position;
                v3PlayerPos.f.z = 0.0f;
                v3PrevPlayerPos.f.z = 0.0f;

                if (TestCollision(fPhysicsRadius, v3PrevPlayerPos, v3PlayerPos, 0.0f, pBall->m_v3PrevPosition, pBall->m_v3Position))
                {
                    s16 delta;
                    delta = self->GetFacingDeltaToPosition(g_pBall->m_v3Position);
                    bDoPickUp = false;
                    u16 absDelta = (u16)((delta < 0) ? -delta : delta);
                    if (absDelta < 0x4000)
                    {
                        bDoPickUp = true;
                    }
                }
            }
        }
    }

    return bDoPickUp;
}

/**
 * Offset/Address/Size: 0x434 | 0x80057984 | size: 0x20
 */
void cPlayer::PostPhysicsUpdate()
{
    cCharacter::PostPhysicsUpdate();
}

/**
 * Offset/Address/Size: 0x454 | 0x800579A4 | size: 0x34
 */
void cPlayer::PreUpdate(float dt)
{
    cCharacter::PreUpdate(dt);
    m_bCanTestController = true;
}

/**
 * Offset/Address/Size: 0x488 | 0x800579D8 | size: 0x148
 */
void cPlayer::PrePhysicsUpdate(float dt)
{
    m_pPoseAccumulator->SetBuildNodeMatrixCallback(m_nHeadJointIndex, PlayerHeadTrackCallback, (unsigned int)this, 0);

    bool poseLocal = false;
    if (m_eClassType != GOALIE || m_v3Position.f.x * g_pBall->m_v3Position.f.x > 0.0f)
    {
        poseLocal = true;
    }

    if (poseLocal)
    {
        PoseLocalSpace();
    }

    m_pPoseAccumulator->SetBuildNodeMatrixCallback(m_nHeadJointIndex, NULL, 0, 0);

    if (m_pBall != NULL)
    {
        m_pBall->m_pPhysicsBall->EnableCollisions();
        m_pPhysicsCharacter->ContainObject(m_pBall->m_pPhysicsBall);
    }

    if (m_pBall != NULL)
    {
        nlVector3 jointPos = GetJointPosition(m_nBallJointIndex);
        float physZ = m_pPhysicsCharacter->GetPosition().f.z;
        jointPos.f.z -= physZ;
        m_pPhysicsCharacter->m_SubObject.SetSubObjectPosition(jointPos, PhysicsObject::RELATIVE_TO_PARENT);
    }

    if (poseLocal)
    {
        m_pPhysicsCharacter->UpdatePose(m_pPoseAccumulator, m_v3Position.f.z);
    }
}

/**
 * Offset/Address/Size: 0x5D0 | 0x80057B20 | size: 0x148
 */
void cPlayer::PlayerHeadTrackCallback(unsigned int nSelf, unsigned int, cPoseAccumulator* pPoseAccumulator, unsigned int nJointIndex, int)
{
    cPlayer& self = *(cPlayer*)(void*)nSelf;
    nlMatrix4 m4HeadMatrix;

    nlMultMatrices(m4HeadMatrix, pPoseAccumulator->GetNodeMatrix(nJointIndex), self.m_m4WorldMatrix);

    int nHeadSpinMax = g_nHeadSpinMax;
    int nHeadTiltMax = g_nHeadTiltMax;
    eCharacterClass cc = self.m_eCharacterClass;

    if (cc == WARIO)
    {
        nHeadSpinMax = g_nWarioHeadSpinMax;
        nHeadTiltMax = g_nWarioHeadTiltMax;
    }
    else if (cc == WALUIGI)
    {
        nHeadSpinMax = g_nWaluigiHeadSpinMax;
    }

    eClassTypes classType = self.m_eClassType;
    float fSmoothTime;
    if (classType == GOALIE)
    {
        fSmoothTime = 0.1f;
    }
    else
    {
        fSmoothTime = 0.1f;
    }

    if (classType == GOALIE)
    {
        self.m_pHeadTrack->Update(m4HeadMatrix, m4HeadMatrix, g_fFixedUpdateTick, g_aOOIConstraint, nHeadSpinMax, nHeadTiltMax, fSmoothTime);

        cHeadTrack* pHeadTrack = self.m_pHeadTrack;
        u16 headSpin = (u16)(int)pHeadTrack->m_fHeadSpin;
        u16 headTilt = (u16)(int)pHeadTrack->m_fHeadTilt;
        CalcHeadTrackMatrix(headSpin, headTilt, pPoseAccumulator, nJointIndex);
    }
    else
    {
        nlMatrix4 m4ConstraintMatrix;
        nlMultMatrices(m4ConstraintMatrix, pPoseAccumulator->GetNodeMatrix(self.m_nSpine1JointIndex), self.m_m4WorldMatrix);

        self.m_pHeadTrack->Update(m4HeadMatrix, m4HeadMatrix, g_fFixedUpdateTick, g_aOOIConstraint, nHeadSpinMax, nHeadTiltMax, fSmoothTime);
    }
}

/**
 * Offset/Address/Size: 0x718 | 0x80057C68 | size: 0x140
 */
cPN_SingleAxisBlender* cPlayer::CreateSingleAxisBlender(const int* pSABAnims, int nNumSABAnims, int nPrimaryAnim, void (*fWeightCB)(unsigned int, cPN_SingleAxisBlender*), float fWeightSeek, cPN_SAnimController* pSynchingController)
{
    cPN_SAnimController* pNewCurrentAnimController;
    pNewCurrentAnimController = NULL;

    cPN_SingleAxisBlender* pSAB = new (AllocateSingleAxisBlender()) cPN_SingleAxisBlender(nNumSABAnims, fWeightCB, (unsigned int)this, fWeightSeek);

    const int* pAnims = pSABAnims;
    for (int i = 0; i < nNumSABAnims; i++)
    {
        cPN_SAnimController* pNewController = NewAnimController(*pAnims, false, false, NULL, 0);
        if (pSynchingController != NULL)
        {
            pNewController->m_bIsSynchronized = true;
            pSynchingController->m_pSynchronizedController = pNewController;
            pSynchingController = pNewController;
        }
        pSAB->SetChild(i, pNewController);
        if (i == nPrimaryAnim)
        {
            pNewCurrentAnimController = pNewController;
        }
        pAnims++;
    }

    SetAnimID(pSABAnims[nPrimaryAnim]);
    m_pCurrentAnimController = pNewCurrentAnimController;

    return pSAB;
}

/**
 * Offset/Address/Size: 0x858 | 0x80057DA8 | size: 0x38
 */
void cPlayer::CollideWithWallCallback(const CollisionPlayerWallData*)
{
    if (this != g_pBall->m_pOwner)
    {
        return;
    }

    if (m_eBallRotationMode != BRM_ANIMATED)
    {
        return;
    }

    m_eBallRotationMode = BRM_MATCH_VELOCITY;

    if (m_pBall != NULL)
    {
        m_ResetBaseBallOrientation = true;
    }
}

/**
 * Offset/Address/Size: 0x890 | 0x80057DE0 | size: 0xE8
 */
void cPlayer::SetPowerupAnimState(int animID)
{
    s32 nodeIndex;
    cPN_SAnimController* pController = NewAnimController(animID, false, false, nullptr, 0);
    if (animID == 0x5F || animID == 0x61)
    {
        cSHierarchy* pHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
        nodeIndex = pHierarchy->GetNodeIndexByID(nlStringLowerHash("bip01 l clavicle"));
    }
    else
    {
        cSHierarchy* pHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
        nodeIndex = pHierarchy->GetNodeIndexByID(nlStringLowerHash("bip01 r clavicle"));
    }

    m_pPowerupLayer->ClearNodeWeights();
    m_pPowerupLayer->SetNodeWeight(nodeIndex, 1.0f, 0.2f);
    m_pPowerupLayer->SetChild(1, pController);
    m_pPowerupLayer->BeginBlendIn(0.067f);
}

/**
 * Offset/Address/Size: 0x978 | 0x80057EC8 | size: 0x18
 */
void cPlayer::ClearSwapControllerTimer()
{
    m_tSwapControllerTimer[0].m_uPackedTime = 0;
    m_tSwapControllerTimer[1].m_uPackedTime = 0;
    m_tSwapControllerTimer[2].m_uPackedTime = 0;
    m_tSwapControllerTimer[3].m_uPackedTime = 0;
}

/**
 * Offset/Address/Size: 0x990 | 0x80057EE0 | size: 0x80
 */
void cPlayer::ClearPowerupAnimState(bool bIsEndGame)
{
    bool bRemovePowerup = false;
    if ((bIsEndGame != 0) && (m_pPowerupLayer->GetChild(1) != 0))
    {
        bRemovePowerup = 1;
    }

    m_pPowerupLayer->BeginBlendOut(0.1f);
    if (m_eClassType == FIELDER)
    {
        cFielder* pFielder = (cFielder*)this;
        pFielder->m_nPowerupAnimID = -1;
        if (bRemovePowerup != 0)
        {
            pFielder->m_ePowerup = POWER_UP_NONE;
        }
    }
}

/**
 * Offset/Address/Size: 0xA10 | 0x80057F60 | size: 0xBCC
 */
void cPlayer::DoRegularPassing(cPlayer* pTeammate, bool bVolleyPass, bool bAllowLeadPass, bool unk1, bool unk2)
{
    class cFielder * pPassTarget; // r30
    unsigned char bLeadPass; // r29
    class nlVector3 teammateLeadPassVelocity; // r1+0x50
    class nlVector3 v3PassIntercept; // r1+0x44
    float fPassGroundSpeed; // f30
    class nlVector3 suggestedPassDirection; // r1+0x38
    class nlVector3 suggestedPassTarget; // r1+0x2C
    float fLeadPassSpeed; // f28
    unsigned char calcPassIntercept; // r28
    float speedToSuggestedPassTarget; // f28
    float fNewPassGroundSpeed; // f1
    int nNumSolutions; // r1+0x8
    float pSolutions[2]; // r1+0xC
    float t; // f3
    class nlVector3 v3PassVelocity; // r1+0x20
    float fPassTime; // f30
    unsigned short aContactFacingDirection; // r28
    const struct LooseBallContactAnimInfo * pBestContactAnimInfo; // r27
    class nlVector3 v3ContactOffset; // r1+0x14
    enum eSpinType Spin; // r25
    unsigned char bBadVolleyPass; // r24
    struct PassBallData * pEventData; // r0
    class cFielder * pPasser; // r0

    g_pBall->m_tNoPickupTimer.SetSeconds(0.1f); // g_pBall + 0x0c
                                                //
    if (bAllowLeadPass && ((cFielder*)pTeammate)->ShouldILeadPass()) {
        Fuzzy::GetPassDirection(this, pTeammate);
        // r3 = 0x78
        // r4 = 0x08
        // r5 = 0x00

        SSearchBestPass* bestPass = (SSearchBestPass*) nlMalloc(0x78, 0x8, 0);
        // r24 = r3 = return di nlMalloc
        // r4 = r30 = this
        // r5 = r29 = pTeammate
        // r6 = 
        if (bestPass != nullptr) {
            bestPass = new (bestPass) SSearchBestPass(this, pTeammate, bVolleyPass, unk1);
        }

        eFieldDirection temp_r25;
        float temp_r8 = 6.0f;
        unsigned short temp_r9 = 0xAAAA;
        auto ciao = pTeammate->m_pSpaceSearch->FindBestPosition(suggestedPassTarget, pTeammate->m_v3Position, temp_r25, &(this->m_v3Position), temp_r8, temp_r9);
    }
}

/**
 * Offset/Address/Size: 0x15DC | 0x80058B2C | size: 0x34
 */
void cPlayer::ResetUnPossessionTimer()
{
    if (m_pBall == NULL)
    {
        m_tBallUnPossessionTimer.SetSeconds(0.0f);
    }
}

/**
 * Offset/Address/Size: 0x1610 | 0x80058B60 | size: 0xD4
 */
void cPlayer::ReleaseBall()
{
    m_pPhysicsCharacter->ReleaseObject();
    m_pBall->ClearOwner();
    m_pBall = NULL;

    float fPossessionTime = m_tBallPossessionTimer.GetSeconds();
    StatsTracker::Instance()->TrackStat(STATS_POSSESION_TIME, m_pTeam->m_nSide, m_ID, 100.0f * fPossessionTime, 0, 0, 0);

    if (IsPlayingEffect(fxGetGroup("ball_sts_windup")))
    {
        StopSFX(Audio::CHARSFX_SHOT_WINDUP);
        StopSFX(Audio::CHARSFX_EFFORTS_HEAD_SHAKE);
    }

    KillWindups(this);
    StopSFX(Audio::CHARSFX_KICK_ATTEMPT);
    m_pCharacterSFX->StopMovementLoop();
}

/**
 * Offset/Address/Size: 0x16E4 | 0x80058C34 | size: 0x1C
 */
cGlobalPad* cPlayer::GetGlobalPad()
{
    if (m_pController != NULL)
    {
        return m_pController->m_pPad;
    }
    return NULL;
}

static inline cFielder* GetAIOrderedFielder(cTeam* pTeam, s32 i)
{
    return pTeam->m_pAIOrderedFielders[i];
}

/**
 * Offset/Address/Size: 0x1700 | 0x80058C50 | size: 0x230
 */
cPlayer* cPlayer::DoFindBestPassTarget(bool bAllowLeadPass, bool bIsPerfectPass)
{
    f32 fBestScore = 99999.0f;
    cFielder* pTarget;
    cPlayer* pBestTarget = NULL;

    for (s32 i = 0; i < 4; i++)
    {
        pTarget = GetAIOrderedFielder(m_pTeam, i);

        if (pTarget == this)
            continue;

        u16 aDirection = m_aActualFacingDirection;

        if (m_pController != NULL)
        {
            if (m_pController->GetMovementStickMagnitude() > 0.1f)
            {
                aDirection = m_pController->GetMovementStickDirection();
            }
        }

        f32 fAngleWeighting;
        f32 fDistBetween;
        f32 dx;
        f32 dy;
        f32 fMinDistance;
        f32 fMaxDistance;

        if (bAllowLeadPass || bIsPerfectPass)
        {
            fMaxDistance = 100.0f;
            fAngleWeighting = g_pGame->m_pGameTweaks->fVolleyPassAngleWeighting;
            fMinDistance = g_pGame->m_pGameTweaks->fVolleyPassMinDistance;
        }
        else
        {
            fMinDistance = 1.0f;
            fMaxDistance = 50.0f;
            fAngleWeighting = g_pGame->m_pGameTweaks->fPassAngleWeighting;
        }

        dy = pTarget->m_v3Position.f.y - m_v3Position.f.y;
        dx = pTarget->m_v3Position.f.x - m_v3Position.f.x;

        f32 fSqrt = nlSqrt(dx * dx + dy * dy, true);
        fDistBetween = fSqrt;

        if (fSqrt < fMinDistance || fSqrt > fMaxDistance)
        {
            fDistBetween = 0.0f;
        }

        f32 fConverted = 10430.37835f * nlATan2f(dy, dx);
        f32 fInvWeight = 1.0f - fAngleWeighting;
        s16 angleDiff = (s16)(aDirection - (u16)(s32)fConverted);
        s32 angleAbs;
        if (angleDiff < 0)
        {
            angleAbs = -angleDiff;
        }
        else
        {
            angleAbs = angleDiff;
        }

        f32 fScore = fDistBetween * fInvWeight + fAngleWeighting * (f32)(u16)angleAbs;

        if (!pTarget->CanReceivePass())
        {
            fScore += 500.0f;
        }

        if (fScore < fBestScore)
        {
            pBestTarget = pTarget;
            fBestScore = fScore;
        }
    }

    if (pBestTarget == NULL)
    {
        pBestTarget = m_pTeam->GetDefence();
        if (pBestTarget == this)
        {
            pBestTarget = m_pTeam->GetMidfield();
        }
    }

    return pBestTarget;
}

/**
 * Offset/Address/Size: 0x1930 | 0x80058E80 | size: 0x24
 */
bool cPlayer::IsCaptain() const
{
    return ::IsCaptain(m_eCharacterClass);
}

/**
 * Offset/Address/Size: 0x1954 | 0x80058EA4 | size: 0x28
 */
bool cPlayer::IsOnSameTeam(cPlayer* other)
{
    if ((other != NULL) && (other->m_pTeam == m_pTeam))
    {
        return true;
    }
    return false;
}

/**
 * Offset/Address/Size: 0x197C | 0x80058ECC | size: 0x4A4
 * TODO: 97.91% match - 4-byte instruction offset throughout (scratch artifact)
 */
void cPlayer::SetAIPad(cAIPad* pPad)
{
    m_pController = pPad;
    if (m_eClassType != FIELDER)
        return;
    cFielder* pFielder = (cFielder*)this;

    if (pFielder->m_pBall != NULL)
    {
        u8 bShotInProgress = 0;
        eShotMeterState state;
        ShotMeter* pMeter = pFielder->m_pShotMeter;
        state = pMeter->m_eShotMeterState;
        if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
            bShotInProgress = 1;
        if (!bShotInProgress)
        {
            if (pFielder->m_eActionState == ACTION_SHOT)
                goto pad;
            if (pMeter->m_eShotMeterState != SHOT_METER_RELEASED)
                goto pad;
        }
        pMeter->ShotReleased(pFielder);
        pFielder->InitActionShot(false);
        return;
    }
pad:
    if (m_pController != NULL && pFielder->m_eFielderDesireState < FIELDERDESIRE_USER_CONTROLLED)
    {
        if (g_pGame->IsGameplayOrOvertime() && pFielder->m_eActionState == ACTION_SHOOT_TO_SCORE)
        {
            pFielder->InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
            return;
        }
        pFielder->EndDesire(false);
        return;
    }
    if (m_pController == NULL)
    {
        if (g_pGame->IsGameplayOrOvertime() && (pFielder->m_eActionState == ACTION_SLIDE_ATTACK || pFielder->m_eActionState == ACTION_SHOOT_TO_SCORE))
        {
            pFielder->InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
            return;
        }
    }
    pFielder->EndDesire(false);
    if (pFielder->m_eFielderDesireState == FIELDERDESIRE_WAIT && g_pGame->m_eGameState == GS_KICKOFF)
        pFielder->EndDesire(false);
}

/**
 * Offset/Address/Size: 0x1E20 | 0x80059370 | size: 0x18C
 */
void cPlayer::PlayAttackReactionSounds(float fScale)
{
    Audio::cCharacterSFX* pSFX = m_pCharacterSFX;

    Audio::gCrowdSFX.Stop(Audio::CROWDSFX_EVENT_OH_SMALL3, cGameSFX::SFX_STOP_FIRST);

    if (pSFX->IsKeepingTrackOf(Audio::CHARSFX_HIT_BODY, NULL))
    {
        pSFX->Stop(Audio::CHARSFX_HIT_BODY, cGameSFX::SFX_STOP_FIRST);
    }
    else if (pSFX->IsKeepingTrackOf(Audio::CHARSFX_HIT_BODY_BONE, NULL))
    {
        pSFX->Stop(Audio::CHARSFX_HIT_BODY_BONE, cGameSFX::SFX_STOP_FIRST);
    }

    Audio::SoundAttributes attrs;
    attrs.Init();

    if (fScale >= g_pGame->m_pGameTweaks->unk240 && m_eClassType != GOALIE)
    {
        attrs.SetSoundType(Audio::CHARSFX_HIT_BODY_BONE, true);
    }
    else
    {
        attrs.SetSoundType(Audio::CHARSFX_HIT_BODY, true);
    }

    attrs.UseStationaryPosVector(m_v3Position);

    attrs.mf_Volume = fScale;
    if (0.0f != fScale)
    {
        SoundStrToIDNode& node = m_pCharacterSFX->mpSFX[attrs.mu_Type];
        attrs.mf_Volume *= node.fVolume;
    }

    PlaySFX(attrs);

    if (!pSFX->IsPlayingRandomCharDialogue(CHAR_DIALOGUE_ELECTROCUTE))
    {
        pSFX->StopPlayingAllRandomCharDialogue();
        if (m_eCharacterClass != MYSTERY)
        {
            PlayRandomCharDialogue(1, VECTORS, fScale, 1.0f);
        }
    }
}

/**
 * Offset/Address/Size: 0x1FAC | 0x800594FC | size: 0x2EC
 */
void cPlayer::PickupBall(cBall* pBall)
{
    if (m_eClassType == GOALIE && ((Goalie*)this)->mbNoUserControl)
    {
    }
    else
    {
        cGlobalPad* hasPad = (m_pController != NULL) ? m_pController->m_pPad : NULL;
        if (hasPad == NULL)
        {
            cPlayer* closest = NULL;

            cAIPad* goalieCtrl = m_pTeam->GetGoalie()->m_pController;
            cGlobalPad* goaliePad = (goalieCtrl != NULL) ? goalieCtrl->m_pPad : NULL;
            if (goaliePad != NULL)
            {
                closest = m_pTeam->GetGoalie();
            }
            else
            {
                f32 bestDistSq;
                for (s32 i = 0; i < 4; i++)
                {
                    cPlayer* player = m_pTeam->GetPlayer(i);
                    cGlobalPad* playerPad = (player->m_pController != NULL) ? player->m_pController->m_pPad : NULL;
                    if (playerPad == NULL)
                        continue;

                    if (closest != NULL)
                    {
                        f32 dy = player->m_v3Position.f.y - m_v3Position.f.y;
                        f32 dx = player->m_v3Position.f.x - m_v3Position.f.x;
                        cPlayer* passTarget = g_pBall->m_pPassTarget;
                        f32 distSq = dx * dx + dy * dy;

                        if (passTarget == NULL || closest != g_pBall->m_pPrevOwner)
                        {
                            if (distSq < bestDistSq)
                            {
                            }
                            else
                            {
                                continue;
                            }
                        }

                        if (passTarget != NULL && player == g_pBall->m_pPrevOwner)
                        {
                            continue;
                        }

                        bestDistSq = distSq;
                        closest = player;
                    }
                    else
                    {
                        closest = player;
                    }
                }
            }

            if (closest != NULL)
            {
                SetAIPad(closest->m_pController);
                closest->SetAIPad(NULL);
            }
        }
    }

    Event* event = g_pEventManager->CreateValidEvent(0xD, 0x20);
    ReceiveBallData* data = new (&event->m_data) ReceiveBallData();
    data->pReceiver = this;
    data->eResult = RECEIVEBALL_LOOSE_PICKUP;

    cPlayer* prevPassTarget = pBall->m_pPassTarget;
    if (prevPassTarget != NULL)
    {
        if (prevPassTarget == this)
        {
            data->eResult = RECEIVEBALL_PASS_COMPLETE;
        }
        else if (prevPassTarget->m_pTeam != m_pTeam)
        {
            data->eResult = RECEIVEBALL_PASS_INTERCEPT;
        }
    }

    if (data->eResult == RECEIVEBALL_PASS_COMPLETE)
    {
        bool bOneTouchShot = false;
        if (m_eClassType == FIELDER)
        {
            if (((cFielder*)this)->GetOneTouchShotDesire() != 0)
            {
                bOneTouchShot = true;
            }
        }
        if (!bOneTouchShot)
        {
            EmitBallImpact(this, false);
        }
    }

    m_tBallPossessionTimer.m_uPackedTime = 0;
    m_tBallUnPossessionTimer.m_uPackedTime = 0;
    m_pBall = pBall;
    m_pBall->SetOwner(this);

    if (m_pBall != NULL)
    {
        m_pBall->m_pPhysicsBall->EnableCollisions();
        m_pPhysicsCharacter->ContainObject(m_pBall->m_pPhysicsBall);
    }

    if (m_pBall != NULL)
    {
        nlVector3& jointPos = GetJointPosition(m_nBallJointIndex);
        nlVector3 pos = jointPos;
        m_pPhysicsCharacter->m_SubObject.SetSubObjectPosition(pos, PhysicsObject::WORLD_COORDINATES);
    }

    m_ResetBaseBallOrientation = true;
    if (m_eClassType == FIELDER)
    {
        ((cFielder*)this)->ClearVolleyPass();
    }
}

/**
 * Offset/Address/Size: 0x2298 | 0x800597E8 | size: 0x138
 * TODO: 99.23% match - r28/r30 register swap: compiler assigns r30 to pOtherTeam (longer live range) instead of pPosition (DWARF confirms pPosition should be r30). Likely compiler version difference.
 */
cFielder* cPlayer::GetClosestOpponentFielder(nlVector3* pPosition)
{
    cTeam* pOtherTeam = m_pTeam->GetOtherTeam();
    f32 fMinDist = HUGE_VALF;
    cPlayer* pClosest = NULL;

    nlVector3 refPos;
    if (pPosition == NULL)
    {
        refPos = m_v3Position;
    }
    else
    {
        refPos = *pPosition;
    }

    f32 refX = refPos.f.x;
    f32 refY = refPos.f.y;

    for (int i = 0; i < 4; i++)
    {
        cPlayer* pPlayer = pOtherTeam->GetPlayer(i);
        if (pPlayer == this)
        {
            continue;
        }
        f32 dx = refX - pPlayer->m_v3Position.f.x;
        f32 dy = refY - pPlayer->m_v3Position.f.y;
        f32 dist = dx * dx + dy * dy;
        if (dist < fMinDist)
        {
            pClosest = pPlayer;
            fMinDist = dist;
        }
    }

    return (cFielder*)pClosest;
}

/**
 * Offset/Address/Size: 0x23D0 | 0x80059920 | size: 0x4
 */
void cPlayer::CollideWithCharacterCallback(CollisionPlayerPlayerData*)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x23D4 | 0x80059924 | size: 0x64
 */
void cPlayer::CollideWithBallCallback(cBall* pBall)
{
    if (pBall->m_pPassTarget != NULL)
    {
        if (pBall->m_pPassTarget != this)
        {
            m_tNoPickupPassInterceptTimer.SetSeconds(g_fPassInterceptNoPickupTimer);
        }
    }

    if (pBall->m_pOwner == NULL)
    {
        pBall->m_pLastTouch = this;
    }
}

/**
 * Offset/Address/Size: 0x2438 | 0x80059988 | size: 0x134
 */
float cPlayer::DoFlashLight(const nlVector3& Position, unsigned short aDirection, float fAngleWeighting, float fIgnoreObjectCloserThanThis, float fIgnoreObjectFartherThanThis)
{
    float fDistBetween;
    float dx;
    float dy = Position.f.y - m_v3Position.f.y;
    dx = Position.f.x - m_v3Position.f.x;

    float fSqrt = nlSqrt(dx * dx + dy * dy, true);
    fDistBetween = fSqrt;

    if (fSqrt < fIgnoreObjectCloserThanThis || fSqrt > fIgnoreObjectFartherThanThis)
    {
        fDistBetween = 0.0f;
    }

    float fConverted = 10430.37835f * nlATan2f(dy, dx);
    float fInvWeight = 1.0f - fAngleWeighting;
    s16 angleDiff = (s16)(aDirection - (u16)(s32)fConverted);
    s32 angleAbs;
    if (angleDiff < 0)
    {
        angleAbs = -angleDiff;
    }
    else
    {
        angleAbs = angleDiff;
    }

    return fDistBetween * fInvWeight + fAngleWeighting * (f32)(u16)angleAbs;
}

/**
 * Offset/Address/Size: 0x256C | 0x80059ABC | size: 0x134
 */
float cPlayer::DoFlashLight(const nlVector3& Position1, const nlVector3& Position2, unsigned short aDirection, float fAngleWeighting, float fIgnoreObjectCloserThanThis, float fIgnoreObjectFartherThanThis)
{
    float dx;
    float dy = Position2.f.y - Position1.f.y;
    dx = Position2.f.x - Position1.f.x;
    float fDistBetween;
    float dist = nlSqrt(dx * dx + dy * dy, true);
    fDistBetween = dist;
    if (dist < fIgnoreObjectCloserThanThis || dist > fIgnoreObjectFartherThanThis)
    {
        fDistBetween = 0.0f;
    }

    u16 targetAngle = (u16)(s32)(10430.378f * nlATan2f(dy, dx));
    float fDistWeight = 1.0f - fAngleWeighting;
    s16 angleDiff = aDirection - targetAngle;
    u16 absAngle = (angleDiff < 0) ? -angleDiff : angleDiff;

    return fDistBetween * fDistWeight + fAngleWeighting * absAngle;
}

/**
 * Offset/Address/Size: 0x26A0 | 0x80059BF0 | size: 0x5C
 */
void cPlayer::SetAnimID(int animID)
{
    cCharacter::SetAnimID(animID);
    m_eBallRotationMode = (eBallRotationMode)m_pAnimInventory->GetBallRotationMode(animID);
    if (m_pBall != nullptr)
    {
        m_ResetBaseBallOrientation = true;
    }
}

/**
 * Offset/Address/Size: 0x26FC | 0x80059C4C | size: 0x190
 */
void cPlayer::GetAnimatedBallOrientation(nlQuaternion& qRetval)
{
    cSAnim* pSAnim = m_pCurrentAnimController->m_pSAnim;
    float time = m_pCurrentAnimController->m_fTime;
    int sanimBallNode = m_pCurrentAnimController->RemapNode(m_nBallJointIndex);

    RotAccum* rot = &m_pPoseAccumulator->m_rot.mData[m_nBallJointIndex];
    rot->q.f.x = 0.0f;
    rot->q.f.y = 0.0f;
    rot->q.f.z = 0.0f;
    rot->q.f.w = 1.0f;
    rot->quatAccumulatedWeight = 0.0f;
    rot->rotAroundZ = 0;
    rot->rotAroundZAccumulatedWeight = 0.0f;
    rot->bIdentity = true;

    pSAnim->BlendRot(m_nBallJointIndex, sanimBallNode, time, 1.0f, m_pPoseAccumulator, m_pCurrentAnimController->m_bMirror);

    nlQuaternion qAnimated = rot->q;
    nlQuaternion qFacing;
    nlVector3 v3Up = { 0.0f, 0.0f, 1.0f };
    nlQuaternion qOrient;
    nlQuaternion startInverse;

    nlMakeQuat(qFacing, v3Up, 0.0000958738f * (float)m_aActualFacingDirection);
    nlMultQuat(qOrient, qFacing, qAnimated);

    if (m_ResetBaseBallOrientation)
    {
        nlQuatInverse(startInverse, qOrient);
        nlMultQuat(m_BaseBallOrientation, startInverse, g_pBall->m_qOrientation);
        m_ResetBaseBallOrientation = false;
    }

    nlMultQuat(qRetval, qOrient, m_BaseBallOrientation);
}

static inline void UpdateTimers(cPlayer* self, float fDeltaT)
{
    if (self->m_pBall != NULL)
    {
        self->m_tBallPossessionTimer.Countup(fDeltaT, 100000.0f);
    }
    else
    {
        self->m_tBallUnPossessionTimer.Countup(fDeltaT, 100000.0f);
    }

    self->m_tSlideAttackTimer.Countdown(fDeltaT, 0.0f);
    self->m_tSwapFacingTimer.Countdown(fDeltaT, 0.0f);
    self->m_tNoPickupPassInterceptTimer.Countdown(fDeltaT, 0.0f);

    if (self->m_tNoPickupTimer.m_uPackedTime != 0)
    {
        bool bCanCollideWithBall = self->m_tNoPickupTimer.Countdown(fDeltaT, 0.0f);
        self->m_pPhysicsCharacter->m_CanCollideWithBall = bCanCollideWithBall;
    }
    else
    {
        self->m_pPhysicsCharacter->m_CanCollideWithBall = true;
    }

    float fScaledDt = fDeltaT * (1.0f / FixedUpdateTask::mTimeScale);
    for (int iPadID = 0; iPadID < 4; iPadID++)
    {
        self->m_tSwapControllerTimer[iPadID].Countdown(fScaledDt, 0.0f);
    }

    if (self->m_fActualSpeed < 0.1f)
    {
        self->m_tInactivityTimer.Countup(fDeltaT, 100000.0f);
    }
    else
    {
        self->m_tInactivityTimer.m_uPackedTime = 0;
    }
}

/**
 * Offset/Address/Size: 0x288C | 0x80059DDC | size: 0x1C4
 */
void cPlayer::Update(float fDeltaT)
{
    if (m_pController != NULL)
    {
        m_UserControlledTime += fDeltaT;
    }
    else
    {
        m_UserControlledTime = 0.0f;
    }

    if (g_pGame->IsGameplayOrOvertime())
    {
        UpdateTimers(this, fDeltaT);
    }

    FieldLocToAILoc(m_v3AIPosition, m_v3Position, (eTeamSide)m_pTeam->m_nSide);
}

/**
 * Offset/Address/Size: 0x2A50 | 0x80059FA0 | size: 0x27C
 */
u8 cPlayer::SwapController()
{
    cPlayer* pSwapPlayer;
    s32 i;
    s32 iPadID;
    cPlayer* pPotentialSwapPlayer;

    if (m_bCanTestController != 0)
    {
        if (g_pBall->m_pPassTarget == NULL || g_pBall->m_pPassTarget != this)
        {
            pSwapPlayer = NULL;
            iPadID = ((m_pController != NULL) ? m_pController->m_pPad : NULL)->m_padIndex;

            for (i = 0; i < 4; i++)
            {
                pPotentialSwapPlayer = m_pTeam->GetPlayer(i);
                if (pPotentialSwapPlayer != this)
                {
                    cGlobalPad* pPotentialSwapPad = (pPotentialSwapPlayer->m_pController != NULL) ? pPotentialSwapPlayer->m_pController->m_pPad : NULL;
                    if (pPotentialSwapPad == NULL)
                    {
                        if (pSwapPlayer == NULL || (pPotentialSwapPlayer->m_tSwapControllerTimer[iPadID].GetSeconds() == 0.0f && pSwapPlayer->m_tSwapControllerTimer[iPadID].GetSeconds() > 0.0f))
                        {
                            pSwapPlayer = pPotentialSwapPlayer;
                            continue;
                        }

                        float fSwapPlayerTime = pSwapPlayer->m_tSwapControllerTimer[iPadID].GetSeconds();
                        if (pPotentialSwapPlayer->m_tSwapControllerTimer[iPadID].GetSeconds() <= fSwapPlayerTime)
                        {
                            float fPotentialScore = Fuzzy::GetSwapControllerScore(pPotentialSwapPlayer).mData.f;
                            float fSwapScore = Fuzzy::GetSwapControllerScore(pSwapPlayer).mData.f;
                            if (fPotentialScore > fSwapScore)
                            {
                                pSwapPlayer = pPotentialSwapPlayer;
                            }
                        }
                    }
                }
            }

            if (pSwapPlayer == NULL)
            {
                float fBestDist = 999999.9f;
                s32 iFielder;
                cFielder* pCandidate;

                for (iFielder = 0; iFielder < 4; iFielder++)
                {
                    pCandidate = m_pTeam->GetFielder(iFielder);

                    float dx, dy, dz;
                    dy = pCandidate->m_v3Position.f.y - g_pBall->m_v3Position.f.y;
                    dx = pCandidate->m_v3Position.f.x - g_pBall->m_v3Position.f.x;
                    dz = pCandidate->m_v3Position.f.z - g_pBall->m_v3Position.f.z;
                    float fDist = nlSqrt(dx * dx + dy * dy + dz * dz, true);

                    if (pCandidate != this)
                    {
                        cGlobalPad* pCandidatePad = (pCandidate->m_pController != NULL) ? pCandidate->m_pController->m_pPad : NULL;
                        if (pCandidatePad == NULL && fDist < fBestDist)
                        {
                            pSwapPlayer = pCandidate;
                            fBestDist = fDist;
                        }
                    }
                }

                if (pSwapPlayer == NULL)
                {
                    return 0;
                }
            }

            if (m_fActualSpeed > 1.0f)
            {
                pSwapPlayer->m_tSwapFacingTimer.SetSeconds(g_pGame->m_pGameTweaks->fSwapFacingTime);
                pSwapPlayer->m_aSwapFacingDirection = pSwapPlayer->m_aActualMovementDirection;
            }

            m_tSwapControllerTimer[iPadID].SetSeconds(g_pGame->m_pGameTweaks->fSwapControllerTime);
            pSwapPlayer->SetAIPad(m_pController);
            pSwapPlayer->m_bCanTestController = false;
            SetAIPad(NULL);

            return 1;
        }
    }

    return 0;
}

/**
 * Offset/Address/Size: 0x2CCC | 0x8005A21C | size: 0x114
 */
void cPlayer::SetDesiredFacingDirection()
{
    if (m_pController != NULL)
    {
        if (m_pController->GetMovementStickMagnitude() > 0.1f)
        {
            if (m_tSwapFacingTimer.m_uPackedTime != 0)
            {
                GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;
                f32 t = m_tSwapFacingTimer.GetSeconds() / pGameTweaks->fSwapFacingTime;
                u16 aStickDirection = m_pController->GetMovementStickDirection();
                s16 diff = m_aSwapFacingDirection - aStickDirection;
                m_aDesiredFacingDirection = (u16)((f32)aStickDirection + (f32)diff * t);
            }
            else
            {
                m_aDesiredFacingDirection = m_pController->GetMovementStickDirection();
            }
        }
        else
        {
            m_aDesiredFacingDirection = m_aActualFacingDirection;
        }
    }

    if (m_eAnimID >= 0x5C || m_eAnimID < 0x56)
    {
        m_aDesiredMovementDirection = m_aDesiredFacingDirection;
    }
}

/**
 * Offset/Address/Size: 0x2DE0 | 0x8005A330 | size: 0x10
 */
void cPlayer::ResetDesiredDirections(unsigned short direction)
{
    m_aDesiredFacingDirection = direction;
    m_aDesiredMovementDirection = m_aDesiredFacingDirection;
}

/**
 * Offset/Address/Size: 0x2DF0 | 0x8005A340 | size: 0x5C
 */
void cPlayer::SetSpaceSearch(SpaceSearch* pSpaceSearch)
{
    if (m_pSpaceSearch != NULL)
    {
        delete m_pSpaceSearch;
    }
    m_pSpaceSearch = pSpaceSearch;
}

/**
 * Offset/Address/Size: 0x2E4C | 0x8005A39C | size: 0x160
 * Almost identical match, emits redundant beq and a single mr
 */
cPlayer::~cPlayer()
{
    PlayerTweaks *piVar1;
    EffectsGroup *pEVar2;
    char cVar3;
    double dVar4;

    this->EndBlur();
    piVar1 = this->m_pTweaks;
    if (piVar1 != nullptr) {
        delete piVar1;
    }
    if (this->m_pBall != nullptr) {
        this->m_pPhysicsCharacter->ReleaseObject();
        this->m_pBall->ClearOwner();
        this->m_pBall = nullptr;
        dVar4 = this->m_tBallPossessionTimer.GetSeconds();
        StatsTracker::s_pInstance->TrackStat((ePlayerStats)0xf, m_pTeam->m_nSide,this->m_ID , 100.f*dVar4, 0, 0, 0);
        pEVar2 = fxGetGroup("ball_sts_windup");
        cVar3 = this->IsPlayingEffect(pEVar2);
        if (cVar3 != 0) {
            this->StopSFX((Audio::eCharSFX)0x14);
            this->StopSFX((Audio::eCharSFX)0x39);
        }
        KillWindups(this);
        this->StopSFX((Audio::eCharSFX)0x12);
        this->m_pCharacterSFX->StopMovementLoop();
    }
    SpaceSearch *temp_piVar1 = this->m_pSpaceSearch;
    if (temp_piVar1 != nullptr)
        delete temp_piVar1;
}

/**
 * Offset/Address/Size: 0x2FAC | 0x8005A4FC | size: 0x380
 * TODO: 97.23% match - remaining mismatch in cPN_Feather allocation control flow
 * for receive-pass and powerup layer construction.
 */
cPlayer::cPlayer(int arg0, eCharacterClass characterClass, const int* arg2, cSHierarchy* hierarchy, cAnimInventory* animInventory, const CharacterPhysicsData* physData, PlayerTweaks* playerTweaks, AnimRetargetList* animRetargetList, eClassTypes classType)
    : cCharacter(characterClass, arg2, hierarchy, animInventory, physData, playerTweaks->fPhysCapsuleHeight, playerTweaks->fPhysCapsuleRadius, animRetargetList, classType)
    , m_ID(arg0)
    , m_bIsContactingWall(false)
    , m_ePositionSeekState(PSS_ARRIVED)
    , m_eBallRotationMode(BRM_MATCH_VELOCITY)
    , m_ResetBaseBallOrientation(false)
    , m_tBallPossessionTimer(0.0f)
    , m_tBallUnPossessionTimer(0.0f)
    , m_tNoPickupTimer(0.0f)
    , m_tNoPickupPassInterceptTimer(0.0f)
    , m_fShotStrengthTime(0.0f)
    , m_tSlideAttackTimer(0.0f)
    , m_tLooseBallPassTimer(0.0f)
    , m_tInactivityTimer(0.0f)
    , m_bCanTestController(true)
    , m_eLastPadAction(PAD_NONE)
    , m_aSwapFacingDirection(0)
    , m_tSwapFacingTimer(0.0f)
    , m_UserControlledTime(0.0f)
    , m_pController(NULL)
    , m_pTweaks(playerTweaks)
    , m_pBall(NULL)
    , m_pTeam(NULL)
{
    cSHierarchy* baseHierarchy;

    baseHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
    m_nBallJointIndex = baseHierarchy->GetNodeIndexByID(nlStringLowerHash("ball"));

    baseHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
    m_nRightFootJointIndex = baseHierarchy->GetNodeIndexByID(nlStringLowerHash("bip01 r foot"));

    baseHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
    m_nLeftFootJointIndex = baseHierarchy->GetNodeIndexByID(nlStringLowerHash("bip01 l foot"));

    baseHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
    m_nLeftHandJointIndex = baseHierarchy->GetNodeIndexByID(nlStringLowerHash("bip01 l hand"));

    baseHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
    m_nRightHandJointIndex = baseHierarchy->GetNodeIndexByID(nlStringLowerHash("bip01 r hand"));

    cPN_Feather* receivePassLayer = AllocateFeather();
    if (receivePassLayer != NULL)
    {
        receivePassLayer = new (receivePassLayer) cPN_Feather(m_pPoseAccumulator->m_BaseSHierarchy, NULL, 0);
    }
    m_pReceivePassLayer = receivePassLayer;

    baseHierarchy = m_pPoseAccumulator->m_BaseSHierarchy;
    int nodeIndex = baseHierarchy->GetNodeIndexByID(nlStringLowerHash("bip01 spine1"));
    m_pReceivePassLayer->SetNodeWeight(nodeIndex, 1.0f, 0.2f);
    m_pReceivePassLayer->SetNodeWeight(m_nBallJointIndex, 1.0f);
    m_pAILayer = m_pReceivePassLayer->GetChildPtr(0);

    cPN_Feather* powerupLayer = AllocateFeather();
    if (powerupLayer != NULL)
    {
        powerupLayer = new (powerupLayer) cPN_Feather(m_pPoseAccumulator->m_BaseSHierarchy, NULL, 0);
    }
    m_pPowerupLayer = powerupLayer;
    m_pPowerupLayer->SetChild(0, m_pReceivePassLayer);
    m_pPoseTree = m_pPowerupLayer;
    m_pSpaceSearch = NULL;

    SetAnimState(0, true, 0.2f, false, false);
    PoseLocalSpace();
    AdjustPoseMatrices();

    m_tSwapFacingTimer.SetSeconds(0.0f);

    m_v3AIPosition.f.x = 0.0f;
    m_v3AIPosition.f.y = 0.0f;
    m_v3AIPosition.f.z = 0.0f;
}
