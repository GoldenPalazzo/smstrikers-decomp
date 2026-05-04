#include "Game/Goalie.h"
#include "Game/AI/GoalieLooseBall.h"
#include "Game/AI/AiUtil.h"
#include "Game/Ball.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/CharacterAudio.h"
#include "Game/SAnim.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "Game/Physics/PhysicsGoalie.h"
#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Field.h"
#include "Game/AnimInventory.h"
#include "Game/CharacterTriggers.h"
#include "Game/Game.h"
#include "Game/GameTweaks.h"
#include "Game/Team.h"
#include "Game/AI/FilteredRandom.h"
#include "Game/FixedUpdateTask.h"
#include "NL/plat/plataudio.h"
#include "types.h"

namespace Audio
{
enum eWorldSFX
{
    WORLDSFX_DUMMY = 0,
};

class cWorldSFX : public cGameSFX
{
public:
    void Stop(eWorldSFX, cGameSFX::StopFlag);
    unsigned long Play(Audio::SoundAttributes&);
};

extern cWorldSFX gCrowdSFX;
extern cWorldSFX gStadGenSFX;
} // namespace Audio

extern cTeam* g_pCurrentlyUpdatingTeam;
extern f32 gfRepositionThreshold;

class Fuzzy
{
public:
    static FuzzyVariant GetBestPassTarget(cPlayer*);
};

float OpenTo(cPlayer*, cPlayer*);

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

/**
 * Offset/Address/Size: 0xB780 | 0x8004E27C | size: 0x2B8
 */
Goalie::Goalie(eCharacterClass charClass, const int* nModelID, cSHierarchy* pHierarchy, cAnimInventory* pAnimInventory, const CharacterPhysicsData* pPhysicsData, GoalieTweaks* pCharTweaks, AnimRetargetList* pAnimRetargetList)
    : cPlayer(0, charClass, nModelID, pHierarchy, pAnimInventory, pPhysicsData, pCharTweaks, pAnimRetargetList, GOALIE)
{
}

/**
 * Offset/Address/Size: 0xB714 | 0x8004E210 | size: 0x6C
 */
Goalie::~Goalie()
{
    GoalieSave::ClearData();
    LooseBallAnims::Destroy();
}

/**
 * Offset/Address/Size: 0xB598 | 0x8004E094 | size: 0x17C
 */
void Goalie::Update(float dt)
{
    cPlayer::Update(dt);

    if (mbDoHeadTrack)
    {
        if (m_pBall == NULL)
        {
            m_pHeadTrack->m_bTrackOOI = true;
            m_pHeadTrack->m_v3OOI = g_pBall->m_v3Position;
        }
        else if (mGoalieActionState == GOALIEACTION_LOOSEBALL_PICKUP && mpLooseBallInfo != NULL && mpLooseBallInfo->mAnimType == LOOSEBALL_ANIM_KICK && mpPassTarget != NULL)
        {
            m_pHeadTrack->m_bTrackOOI = true;
            m_pHeadTrack->m_v3OOI = mpPassTarget->m_v3Position;
        }
        else
        {
            m_pHeadTrack->m_bTrackOOI = false;
        }
    }
    else
    {
        m_pHeadTrack->m_bTrackOOI = false;
    }

    UpdateActionState(dt);
    mFatigue.Update(dt);
    cCharacter::Update(dt);

    if (!mbIsPosed)
    {
        PoseLocalSpace();
        m_pPhysicsCharacter->UpdatePose(m_pPoseAccumulator, m_v3Position.f.z);
        m_pPhysicsCharacter->GetCharacterPositionXY(&m_v3Position);
        CreateWorldMatrix();
        AdjustPoseMatrices();
        mbIsPosed = true;
    }
}

/**
 * Offset/Address/Size: 0xA86C | 0x8004D368 | size: 0xD2C
 */
void Goalie::CollideWithBallCallback(cBall*)
{
}

/**
 * Offset/Address/Size: 0xA178 | 0x8004CC74 | size: 0x6F4
 */
void Goalie::CollideWithCharacterCallback(CollisionPlayerPlayerData*)
{
}

/**
 * Offset/Address/Size: 0xA01C | 0x8004CB18 | size: 0x15C
 * TODO: 97.47% match - r3/r4 register swap for jump table base address (MWCC allocator quirk),
 * causes extra mr r3, r31 before CleanGoalieAction and InitiatePickup calls
 */
bool Goalie::PreCollideWithBallCallback(const dContact& contact)
{
    switch (mGoalieActionState)
    {
    case GOALIEACTION_SAVE_SETUP:
    case GOALIEACTION_SAVE_REPOSITION:
    case GOALIEACTION_SAVE:
    case GOALIEACTION_MISS_CHIP_SHOT:
    case GOALIEACTION_DIVE_RECOVER:
        if (m_eAnimID == 0x6d)
        {
            return false;
        }
        if (mpSaveData != NULL && mpSaveData->muSaveType == 0x40000)
        {
            return false;
        }
        break;

    case GOALIEACTION_LOOSEBALL_SETUP:
    case GOALIEACTION_LOOSEBALL_CATCH:
    case GOALIEACTION_LOOSEBALL_PICKUP:
    case GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING:
    case GOALIEACTION_LOOSEBALL_PURSUE_ROLLING:
    case GOALIEACTION_LOOSEBALL_DESPERATE:
    case GOALIEACTION_GRAB_BALL:
    {
        CleanGoalieAction();

        mPrevGoalieActionState = mGoalieActionState;
        mGoalieActionState = GOALIEACTION_LOOSEBALL_PICKUP;
        mnSubstate = 0;

        SetAnimState(mpLooseBallInfo->mnAnimID, true, 0.2f, false, false);
        InitMovementFromAnim(0, v3Zero, 1.0f, false);

        mMoveDirection = (eGoalieMoveDirection)0;
        mfTargetTime = 0.0f;
        mfWaitTime = 0.5f;
        mbPickedUp = false;

        {
            const LooseBallInfo* pInfo = mpLooseBallInfo;
            f32 pickupDist = pInfo->mfPickupDistance;
            if (0.3f < pickupDist)
            {
                f32 pickupTime = pInfo->mfPickupTime;
                mfTargetTime = (pickupDist - 0.3f) * pickupTime / pickupDist;

                cPN_SAnimController* pAnim = m_pCurrentAnimController;
                f32 targetTime = mfTargetTime;
                f32 curTime = pAnim->m_fTime;
                pAnim->m_fPrevTime = curTime;
                pAnim->m_fTime = targetTime;
            }
        }

        InitiatePickup();
        return false;
    }

    case GOALIEACTION_MOVE:
    case GOALIEACTION_MOVE_WB:
    case GOALIEACTION_PRE_CROUCH:
    case GOALIEACTION_OFFPLAY:
    case GOALIEACTION_SNAP_BALL:
        if (InitiatePickup())
        {
            return false;
        }
        break;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x9D18 | 0x8004C814 | size: 0x304
 * TODO: 97.78% match - remaining diffs are in pounce hit-distance float register
 * ordering and action-transition tail register allocation around the 0x33 anim gate.
 */
void Goalie::ExecutePounce(cPlayer* pPlayer, bool bCheckHitDistance)
{
    cFielder* pFldr = static_cast<cFielder*>(pPlayer);
    bool bDoHit = false;

    if (!pFldr->IsFallenDown(0.0f) && !pFldr->IsInvincible())
    {
        bDoHit = true;
    }

    if (bDoHit && bCheckHitDistance)
    {
        float fPlayerRadius;
        float fGoalieRadius;

        m_pPhysicsCharacter->GetRadius(&fGoalieRadius);
        pFldr->m_pPhysicsCharacter->GetRadius(&fPlayerRadius);

        float fDeltaX = m_v3Position.f.x - pFldr->m_v3Position.f.x;
        float fMinHitDistance = fGoalieRadius + fPlayerRadius + 0.5f;
        float fDeltaY = m_v3Position.f.y - pFldr->m_v3Position.f.y;

        bDoHit = ((fDeltaX * fDeltaX) + (fDeltaY * fDeltaY)) < (fMinHitDistance * fMinHitDistance);
    }

    if (pFldr->m_eActionState == ACTION_SHOOT_TO_SCORE)
    {
        bDoHit = true;
    }

    bool bGetBall = false;
    if (pPlayer->m_pBall != NULL && g_pBall->m_v3Position.f.z < 1.0f)
    {
        Audio::SoundAttributes sndAtr;
        sndAtr.Init();
        sndAtr.SetSoundType(0xB7, true);
        sndAtr.UseStationaryPosVector(m_v3Position);
        sndAtr.mf_Volume = 1.0f;
        Audio::gStadGenSFX.Play(sndAtr);
        bGetBall = true;
    }

    if (bDoHit)
    {
        if (pFldr != NULL && pFldr->m_eClassType == FIELDER && !pFldr->IsFallenDown(0.0f))
        {
            pFldr->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);

            if (pFldr->m_pBall != NULL)
            {
                pFldr->ReleaseBall();
            }

            if (IsOnSameTeam(pFldr))
            {
                pFldr->EndDesire(false);
                pFldr->EndAction();
            }
            else
            {
                pFldr->InitActionSlideAttackReact(this, false);
            }
        }
    }
    else if (pFldr != NULL && pFldr->m_eClassType == FIELDER && pFldr->m_pBall != NULL)
    {
        pFldr->ReleaseBall();

        if (pFldr->m_eFielderDesireState != FIELDERDESIRE_FINISH_ACTION)
        {
            pFldr->EndDesire(false);
            pFldr->EndAction();
        }
    }

    if (bGetBall)
    {
        PickupBall(g_pBall);
        mbPickedUp = true;
        g_pBall->ClearShotInProgress();
        EmitGoalieCatch(this, "goalie_catch", false);

        if (mGoalieActionState != GOALIEACTION_PURSUE_BALL_POUNCE)
        {
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_PURSUE_BALL_POUNCE;
            mnSubstate = 0;

            bool bShouldSetAnim = false;
            if (m_eAnimID != 0x33 || (bShouldSetAnim = (m_pCurrentAnimController->m_ePlayMode == 1 && m_pCurrentAnimController->m_fTime == 1.0f)))
            {
                SetAnimState(0x33, true, 0.2f, false, false);
            }

            InitMovementFromAnimSeek(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff);
        }
    }
}

/**
 * Offset/Address/Size: 0x99F0 | 0x8004C4EC | size: 0x328
 * TODO: 93.94% match - FuzzyVariant copy ctor stack layout: named local vs return temp at swapped offsets (sp+0x08 vs sp+0x38). Compiler-internal allocation order issue. fOpenTo threshold uses 0.65f (separate SDA entry from GetPressure's 0.5f).
 */
void Goalie::InitActionPass(bool useTarget)
{
    int animID;

    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_PASS;
    mnSubstate = 0;
    mpPassTarget = NULL;

    if (useTarget)
    {
        cPlayer* pPassTarget;

        if (GetGlobalPad() != NULL)
        {
            pPassTarget = DoFindBestPassTarget(false, false);
        }
        else
        {
            FuzzyVariant passTarget = Fuzzy::GetBestPassTarget(this);
            if (passTarget.Confidence >= 0.0f)
            {
                pPassTarget = passTarget.mData.pPlayer;
            }
            else
            {
                pPassTarget = DoFindBestPassTarget(false, false);
            }
        }

        mpPassTarget = pPassTarget;

        if (mpPassTarget != NULL)
        {
            bool isValidPassTarget;

            if ((float)fabs(mpPassTarget->m_v3Position.f.x) > (float)fabs(m_v3Position.f.x)
                && (float)fabs(mpPassTarget->m_v3Position.f.y) < cField::GetPenaltyBoxY())
            {
                isValidPassTarget = false;
            }
            else
            {
                isValidPassTarget = true;
            }

            if (isValidPassTarget)
            {
                GoalieTweaks* pTweaks = static_cast<GoalieTweaks*>(m_pTweaks);

                float dy = m_v3Position.f.y - mpPassTarget->m_v3Position.f.y;
                float dx = m_v3Position.f.x - mpPassTarget->m_v3Position.f.x;
                float fDistanceSq = (dx * dx) + (dy * dy);
                float fKickDistanceSq = pTweaks->fKickDistanceMin * pTweaks->fKickDistanceMin;
                float fOverhandThrowDistanceSq = pTweaks->fOverhandThrowDistanceMin * pTweaks->fOverhandThrowDistanceMin;
                float fOpenTo = OpenTo(this, mpPassTarget);

                if (GetGlobalPad() != NULL)
                {
                    if (GetGlobalPad()->GetPressure(0x15, true) > 0.5f)
                    {
                        animID = 2;
                    }
                    else if ((fDistanceSq > fOverhandThrowDistanceSq) || (fOpenTo < 0.65f))
                    {
                        animID = 0;
                    }
                    else
                    {
                        animID = 1;
                    }
                }
                else if (fDistanceSq > fKickDistanceSq)
                {
                    animID = 2;
                }
                else if ((fDistanceSq > fOverhandThrowDistanceSq) || (fOpenTo < 0.65f))
                {
                    animID = 0;
                }
                else
                {
                    animID = 1;
                }
            }
            else
            {
                mpPassTarget = NULL;
            }
        }
    }

    if (mpPassTarget == NULL)
    {
        animID = 2;
    }

    SetAnimState(animID, true, 0.2f, false, false);
    InitMovementFromAnimSeek(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff);
}

/**
 * Offset/Address/Size: 0x987C | 0x8004C378 | size: 0x174
 */
void Goalie::InitActionPassIntercept()
{
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_PASS_INTERCEPT;
    mnSubstate = 0;
    muBallDeflectCount = g_pBall->m_bBallDeflectCount;

    if (mfWaitTime <= 0.0f)
    {
        CleanGoalieAction();

        mPrevGoalieActionState = mGoalieActionState;
        mGoalieActionState = GOALIEACTION_SAVE;
        mnSubstate = 0;

        PlayBlendedAnims(mBlendInfo.mfStartTime, -1);

        m_pPhysicsCharacter->m_CanCollideWithBall = 1;

        mnOffplayPending = GOALIE_OFFPLAY_NONE;
        mbBallImpacted = false;

        Event* pEvent = g_pEventManager->CreateValidEvent(0x13, 0x38);
        GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();

        pSaveData->pGoalie = this;
        pSaveData->v3BallVelocity = v3Zero;
        pSaveData->fWowFactor = 0.0f;
        pSaveData->isSTS = 0;

        pSaveData->saveType = g_pBall->m_uGoalType;
        pSaveData->pShooter = g_pBall->m_pShooter;

        if (mpSaveData != NULL)
        {
            pSaveData->padding = mpSaveData->muSaveType;
        }
        else
        {
            pSaveData->padding = 3;
        }
    }
    else
    {
        mnSubstate = 4;
    }
}

/**
 * Offset/Address/Size: 0x9754 | 0x8004C250 | size: 0x128
 */
void Goalie::InitActionPassInterceptSave()
{
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_SAVE;
    mnSubstate = 0;

    PlayBlendedAnims(mBlendInfo.mfStartTime, -1);

    m_pPhysicsCharacter->m_CanCollideWithBall = 1;

    mnOffplayPending = GOALIE_OFFPLAY_NONE;
    mbBallImpacted = false;

    Event* pEvent = g_pEventManager->CreateValidEvent(0x13, 0x38);
    GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();

    pSaveData->pGoalie = this;
    pSaveData->v3BallVelocity = v3Zero;
    pSaveData->fWowFactor = 0.0f;
    pSaveData->isSTS = 0;

    pSaveData->saveType = g_pBall->m_uGoalType;
    pSaveData->pShooter = g_pBall->m_pShooter;

    if (mpSaveData != NULL)
    {
        pSaveData->padding = mpSaveData->muSaveType;
    }
    else
    {
        pSaveData->padding = 3;
    }
}

/**
 * Offset/Address/Size: 0x9684 | 0x8004C180 | size: 0xD0
 */
void Goalie::InitActionPreCrouch(eGoalieCrouchType crouchType)
{
    if (mGoalieActionState == GOALIEACTION_STS || mGoalieActionState == GOALIEACTION_STS_RECOVER)
    {
        return;
    }

    mCrouchType = crouchType;
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_PRE_CROUCH;
    mnSubstate = 0;

    if (m_eAnimID == 0x2e)
    {
        bool bShouldSetAnim = false;
        if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
        {
            bShouldSetAnim = true;
        }
        if (bShouldSetAnim)
        {
            goto check;
        }
    }
    else
    {
    check:
        SetAnimState(0x2e, true, 0.2f, false, false);
    }

    InitMovementFromAnim(0, v3Zero, 0.0f, false);
}

/**
 * Offset/Address/Size: 0x95FC | 0x8004C0F8 | size: 0x88
 */
void Goalie::InitActionPursueBallCarrier()
{
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_PURSUE_BALL_CARRIER;
    mnSubstate = 0;

    mpLooseBallInfo = &LooseBallAnims::mTrapBallInfo;

    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
    float random = nlRandomf(1.0f, &nlDefaultSeed);
    mbPlayMiss = (random < pSkillTweaks->fGoalieDekeChance);
}

/**
 * Offset/Address/Size: 0x957C | 0x8004C078 | size: 0x80
 */
void Goalie::InitActionPursueBallPounce()
{
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_PURSUE_BALL_POUNCE;
    mnSubstate = 0;

    SetAnimState(mpLooseBallInfo->mnAnimID, true, 0.2f, false, false);
    InitMovementFromAnimSeek(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff);

    mbPickedUp = false;
}

/**
 * Offset/Address/Size: 0x94E8 | 0x8004BFE4 | size: 0x94
 */
void Goalie::InitActionPursueRecover()
{
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_DIVE_RECOVER;
    mnSubstate = 0;

    int animID = 0x8f;
    if (m_pBall == NULL)
    {
        animID = 0x8e;
    }

    SetAnimState(animID, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    mbPickedUp = false;
}

/**
 * Offset/Address/Size: 0x8878 | 0x8004B374 | size: 0xC70
 */
void Goalie::DoNavigation(float, float, Goalie::eNaviMode)
{
}

/**
 * Offset/Address/Size: 0x82F0 | 0x8004ADEC | size: 0x588
 */
void Goalie::FindDesiredGoaliePosition(nlVector3& pos, nlVector3& dir, nlVector3& focus, unsigned short& ang, const nlVector3* pThreatPos)
{
    cNet* pNet = m_pTeam->m_pNet;
    nlVector3 targetPos;
    float goalX;
    nlVector3 desiredVec;

    goalX = 0.5f * pNet->m_sideSign + pNet->GetGoalLineX();

    if (pThreatPos == NULL)
    {
        cBall* pBall = g_pBall;
        cPlayer* pOwner = pBall->m_pOwner;
        if (pOwner != NULL)
        {
            targetPos.f.x = 0.18f * pOwner->m_v3Velocity.f.x + pBall->m_v3Position.f.x;
            targetPos.f.y = 0.18f * pOwner->m_v3Velocity.f.y + pBall->m_v3Position.f.y;
            targetPos.f.z = 0.18f * pOwner->m_v3Velocity.f.z + pBall->m_v3Position.f.z;
        }
        else
        {
            targetPos.f.x = 0.18f * pBall->m_v3Velocity.f.x + pBall->m_v3Position.f.x;
            targetPos.f.y = 0.18f * pBall->m_v3Velocity.f.y + pBall->m_v3Position.f.y;
            targetPos.f.z = 0.18f * pBall->m_v3Velocity.f.z + pBall->m_v3Position.f.z;
        }
        targetPos.f.z = 0.0f;
    }
    else
    {
        targetPos = *pThreatPos;
    }

    float goalLineX = cField::GetGoalLineX(1U);
    float absTargetY = (float)fabs(targetPos.f.y);
    float fNetY = cField::GetSidelineY(1U);
    float limit = goalLineX - absTargetY * 3.0f / fNetY;

    if (targetPos.f.x > limit)
    {
        targetPos.f.x = limit;
    }
    else if (targetPos.f.x < -limit)
    {
        targetPos.f.x = -limit;
    }

    desiredVec.f.x = targetPos.f.x - goalX;
    desiredVec.f.y = targetPos.f.y - 0.0f;
    desiredVec.f.z = targetPos.f.z - 0.0f;

    float goalLine = cField::GetGoalLineX(1U) - 0.5f;
    float goalieDist = nlSqrt(desiredVec.f.x * desiredVec.f.x + desiredVec.f.y * desiredVec.f.y + desiredVec.f.z * desiredVec.f.z, true);
    float breakaway = 0.0f;
    float targetDist = goalieDist - 0.5f;

    cFielder* ownerFielder = g_pBall->GetOwnerFielder();
    if (ownerFielder != NULL && !IsOnSameTeam(ownerFielder))
    {
        cFielder* ownerFielder2 = g_pBall->GetOwnerFielder();
        bool isShooting = (ownerFielder2 != NULL && !IsOnSameTeam(ownerFielder2) && ownerFielder2->m_eActionState == ACTION_SHOOT_TO_SCORE);
        if (isShooting)
        {
            breakaway = 0.0f;
        }
        else
        {
            breakaway = OnBreakaway(ownerFielder);
            if (breakaway > 0.8f)
            {
                cTeam* opponentTeam = ownerFielder->m_pTeam;
                for (int i = 0; i < 4; i++)
                {
                    cFielder* fielder = opponentTeam->GetFielder(i);
                    if (fielder == ownerFielder)
                        continue;
                    cNet* myNet = m_pTeam->m_pNet;
                    float absX = (float)fabs(fielder->m_v3Position.f.x);
                    if (fielder->m_v3Position.f.x * myNet->m_baseLocation.f.x > 0.0 && absX > 2.0f)
                    {
                        float factor = 1.0f - 0.5f * (absX - 2.0f) / 10.0f;
                        if (0.0f >= factor)
                        {
                        }
                        else
                        {
                            breakaway *= factor;
                        }
                    }
                }
            }
        }
    }

    if (breakaway > 0.8f)
    {
        goalieDist = Interpolate(2.5f, 8.0f, 5.0000005f * (breakaway - 0.8f));
        mUrgency = URGENCY_MED;
    }
    else if (targetDist > 23.0f)
    {
        goalieDist = 8.0f;
    }
    else if (targetDist > 19.0f)
    {
        goalieDist = 5.5f * (targetDist - 19.0f) * 0.25f + 2.5f;
    }
    else if (targetDist > 12.0f)
    {
        goalieDist = 2.5f;
    }
    else if (targetDist > 7.0f)
    {
        goalieDist = 4.0f + (-1.5f * (targetDist - 7.0f) / 5.0f);
    }
    else
    {
        goalieDist = targetDist - 3.0f;
    }

    if (goalieDist > targetDist - 3.0f)
    {
        goalieDist = targetDist - 3.0f;
    }
    if (goalieDist < 0.5f)
    {
        goalieDist = 0.5f;
    }

    float ratio = (goalieDist + 0.5f) / (targetDist + 0.5f);
    float desiredX = ratio * desiredVec.f.x + goalX;
    float desiredY = ratio * desiredVec.f.y + 0.0f;
    float desiredZ = ratio * desiredVec.f.z + 0.0f;

    if ((float)fabs(m_v3Position.f.x) > cField::GetGoalLineX(1U))
    {
        float halfNetMinusOne = 0.5f * cNet::m_fNetWidth - 1.0f;
        if (desiredY >= -halfNetMinusOne)
        {
        }
        else
        {
            desiredY = -halfNetMinusOne;
        }
        if (desiredY <= halfNetMinusOne)
        {
        }
        else
        {
            desiredY = halfNetMinusOne;
        }
        desiredX = goalLine * pNet->m_sideSign;
        desiredVec.f.y = desiredY - m_v3Position.f.y;
        desiredVec.f.x = desiredX - m_v3Position.f.x;
        desiredVec.f.z = desiredZ - m_v3Position.f.z;
    }

    ang = (unsigned short)(10430.378f * nlATan2f(desiredVec.f.y, desiredVec.f.x));

    if (desiredX >= -goalLine)
    {
    }
    else
    {
        desiredX = -goalLine;
    }
    if (desiredX <= goalLine)
    {
    }
    else
    {
        desiredX = goalLine;
    }

    pos.f.x = 0.2f * m_v3Position.f.x + 0.8f * desiredX;
    pos.f.y = 0.2f * m_v3Position.f.y + 0.8f * desiredY;
    pos.f.z = 0.2f * m_v3Position.f.z + 0.8f * desiredZ;
    dir = desiredVec;
    focus = targetPos;
}

/**
 * Offset/Address/Size: 0x8024 | 0x8004AB20 | size: 0x2CC
 * TODO: 99.89% match - FP register allocation mismatch in nav-target delta setup (f31/f30 direct load/sub form)
 */
bool Goalie::ShouldReposition()
{
    if (mfWaitTime < 0.07f)
    {
        return false;
    }

    bool bDesiredDirSet = false;

    if (!mbShouldMiss)
    {
        cBall* pBall = g_pBall;

        if (!pBall->m_unk_0xA6)
        {
            if ((0.3f + mBlendInfo.mv3BlendedSavePos.f.z) < mv3LocalContactPosition.f.z)
            {
                nlVector3 v3ContactVel;
                float fDropTime = FakeBallWorld::GetPredictedHeightLimitTime(2.0f, 0.04f, mv3NavTarget, v3ContactVel, true);
                float fGoalGapDist = cField::GetGoalLineX(1U) - 0.5f;
                float fBoxGapDist = 0.5f + cField::GetPenaltyBoxX(1U);
                float fTargetX;
                bool bCalcIntersect = false;
                float navAbsX = (float)fabs(mv3NavTarget.f.x);

                if (navAbsX > fGoalGapDist)
                {
                    if (m_v3Position.f.x > 0.0f)
                    {
                        fTargetX = fGoalGapDist;
                    }
                    else
                    {
                        fTargetX = -fGoalGapDist;
                    }

                    bCalcIntersect = true;
                }
                else if (navAbsX < fBoxGapDist)
                {
                    FakeBallWorld::GetPredictedHeightLimitTime(2.0f, 0.25f + fDropTime, mv3NavTarget, v3ContactVel, true);

                    if ((float)fabs(mv3NavTarget.f.x) < fBoxGapDist)
                    {
                        if (m_v3Position.f.x > 0.0f)
                        {
                            fTargetX = fBoxGapDist;
                        }
                        else
                        {
                            fTargetX = -fBoxGapDist;
                        }
                    }

                    bCalcIntersect = true;
                }

                if (bCalcIntersect)
                {
                    if ((float)fabs(v3ContactVel.f.x) > 0.5f)
                    {
                        float ballX = pBall->m_v3Position.f.x;
                        float ballY = pBall->m_v3Position.f.y;

                        mv3NavTarget.f.y = ballY + ((fTargetX - ballX) * (mv3NavTarget.f.y - ballY) / (mv3NavTarget.f.x - ballX));
                    }

                    mv3NavTarget.f.x = fTargetX;
                }

                fTargetX = mv3NavTarget.f.y;
                fDropTime = mv3NavTarget.f.x;
                fTargetX -= m_v3Position.f.y;
                fDropTime -= m_v3Position.f.x;

                m_aDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(pBall->m_v3Position.f.y - m_v3Position.f.y, pBall->m_v3Position.f.x - m_v3Position.f.x));

                if (((fDropTime * fDropTime) + (fTargetX * fTargetX)) > 0.25f)
                {
                    mUrgency = URGENCY_MED;
                    return true;
                }

                bDesiredDirSet = true;
            }
        }
    }

    if ((float)fabs(mBlendInfo.mv3BlendedSavePos.f.y) > gfRepositionThreshold)
    {
        if (!bDesiredDirSet)
        {
            cBall* pBall = g_pBall;
            m_aDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(pBall->m_v3Position.f.y - m_v3Position.f.y, pBall->m_v3Position.f.x - m_v3Position.f.x));
        }

        GetWorldPoint(mv3NavTarget, mBlendInfo.mv3BlendedSavePos, m_v3Position, m_aDesiredFacingDirection);
        mv3NavTarget.f.z = 0.0f;

        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x7A48 | 0x8004A544 | size: 0x5DC
 */
void Goalie::HandleSTSContact(cBall*)
{
}

/**
 * Offset/Address/Size: 0x75B4 | 0x8004A0B0 | size: 0x494
 * TODO: 83.86% match - 9 branch offset diffs from 1 extra instruction (4 bytes) in
 * if(m_pBall==NULL) wrapping pattern for KICK block. Target uses flat bne- far branch
 * but MWCC generates beq-+inline return for if(m_pBall!=NULL)return false, requiring
 * wrapping which adds a trailing branch instruction shifting all subsequent branch targets.
 */
bool Goalie::InitiatePickup()
{
    if (mpLooseBallInfo->mAnimType == LOOSEBALL_ANIM_KICK)
    {
        if (mbPickedUp)
        {
            return false;
        }

        if (m_pBall == NULL)
        {
            mbNoUserControl = true;

            cFielder* pFldr = g_pBall->GetOwnerFielder();
            if (pFldr != NULL)
            {
                if (IsOnSameTeam(pFldr))
                {
                    CleanGoalieAction();

                    mPrevGoalieActionState = mGoalieActionState;
                    mGoalieActionState = GOALIEACTION_MOVE;
                    mnSubstate = 0;

                    SetAnimState(8, true, 0.2f, false, false);
                    InitMovementFromAnim(0, v3Zero, 1.0f, false);

                    mnSubstate = 1;
                    mMoveDirection = GOALIEDIR_IDLE;

                    m_pPhysicsCharacter->m_CanCollideWithBall = true;
                    mbShouldMiss = false;
                    mbDoNavigate = false;

                    m_pPhysicsCharacter->m_CanCollidedWithGoalLine = true;
                    m_pPhysicsCharacter->m_CanCollideWithWall = true;

                    if (mbStunEffectActive)
                    {
                        KillDaze(this);
                        mbStunEffectActive = false;
                    }

                    mpShooter = NULL;
                    mfSpeedScale = 1.0f;
                    mUrgency = URGENCY_LOW;

                    mbPosGoalieNetCheck = false;
                    mbNegGoalieNetCheck = false;
                    mbDoHeadTrack = true;
                    mbBallImpacted = false;
                    mbNoUserControl = false;
                    mbPickedUp = false;

                    return false;
                }

                if (pFldr != NULL && pFldr->m_eClassType == FIELDER)
                {
                    if (!pFldr->IsFallenDown(0.0f))
                    {
                        pFldr->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);

                        if (pFldr->m_pBall != NULL)
                        {
                            pFldr->ReleaseBall();
                        }

                        if (IsOnSameTeam(pFldr))
                        {
                            pFldr->EndDesire(false);
                            pFldr->EndAction();
                        }
                        else
                        {
                            pFldr->InitActionSlideAttackReact(this, false);
                        }
                    }
                }
            }
            else
            {
                if (g_pBall->m_tNoPickupTimer.m_uPackedTime != 0)
                {
                    return false;
                }
            }

            nlVector3 v3Pos;
            v3Pos.f.x = g_pBall->m_v3Position.f.x;
            v3Pos.f.y = g_pBall->m_v3Position.f.y;
            v3Pos.f.z = g_pBall->m_v3Position.f.z;

            PickupBall(g_pBall);
            mbPickedUp = true;
            g_pBall->SetPosition(v3Pos);

            if (mpLooseBallInfo->mnAnimID == 5 || mpLooseBallInfo->mnAnimID == 4)
            {
                mpPassTarget = NULL;
            }
            else
            {
                if (GetGlobalPad() != NULL)
                {
                    mpPassTarget = DoFindBestPassTarget(false, false);
                }
                else
                {
                    FuzzyVariant fvTarget = Fuzzy::GetBestPassTarget(this);
                    if (fvTarget.Confidence >= 0.5f)
                    {
                        mpPassTarget = fvTarget.mData.pPlayer;
                    }
                    else
                    {
                        mpPassTarget = DoFindBestPassTarget(false, false);
                    }
                }
            }

            if (mpPassTarget != NULL)
            {
                mbDoHeadTrack = true;
            }
            else
            {
                mbDoHeadTrack = false;
            }

            if (g_pBall->m_tShotTimer.m_uPackedTime != 0)
            {
                g_pBall->ClearShotInProgress();
            }

            return true;
        }
    }
    else
    {
        if (mfWaitTime <= 0.0f)
        {
            if (g_pBall->m_tNoPickupTimer.m_uPackedTime != 0)
            {
                return false;
            }

            mfWaitTime = 0.1f;
            SetNoPickUpTime(mfWaitTime);

            nlVector3 v3BallVel;
            v3BallVel.f.x = g_pBall->m_v3Velocity.f.x;
            v3BallVel.f.y = g_pBall->m_v3Velocity.f.y;
            v3BallVel.f.z = g_pBall->m_v3Velocity.f.z;

            float fSpeedSq = v3BallVel.f.x * v3BallVel.f.x + v3BallVel.f.y * v3BallVel.f.y + v3BallVel.f.z * v3BallVel.f.z;
            if (fSpeedSq > 64.0f)
            {
                v3BallVel.f.x *= 0.5f;
                v3BallVel.f.y *= 0.5f;
                v3BallVel.f.z *= 0.5f;
                g_pBall->SetVelocity(v3BallVel, SPINTYPE_NONE, NULL);
            }

            mbDoHeadTrack = false;

            if (g_pBall->m_tShotTimer.m_uPackedTime != 0)
            {
                g_pBall->ClearShotInProgress();
            }

            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x73E4 | 0x80049EE0 | size: 0x1D0
 */
void Goalie::InitiatePanicGrab(cPlayer* pPlayer)
{
    if (pPlayer != NULL)
    {
        if (IsOnSameTeam(pPlayer))
        {
            return;
        }

        cFielder* pFielder = static_cast<cFielder*>(pPlayer);

        if (!pFielder->IsFallenDown(0.0f) && !pFielder->IsInvincible() && pPlayer != NULL && pPlayer->m_eClassType == FIELDER && !pFielder->IsFallenDown(0.0f))
        {
            pPlayer->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);

            if (pPlayer->m_pBall != NULL)
            {
                pPlayer->ReleaseBall();
            }

            if (IsOnSameTeam(pFielder))
            {
                pFielder->EndDesire(false);
                pFielder->EndAction();
            }
            else
            {
                pFielder->InitActionSlideAttackReact(this, false);
            }
        }
    }

    s32 nAnimID = mpLooseBallInfo->mnAnimID;
    if (nAnimID != m_eAnimID)
    {
        do
        {
            if (nAnimID == m_eAnimID)
            {
                cPN_SAnimController* pController = m_pCurrentAnimController;
                u8 bShouldSetAnim = false;
                if (pController->m_ePlayMode == 1 && pController->m_fTime == 1.0f)
                {
                    bShouldSetAnim = true;
                }

                if (!bShouldSetAnim)
                {
                    break;
                }
            }

            SetAnimState(nAnimID, true, 0.2f, false, false);
        } while (0);

        cPN_SAnimController* pController = m_pCurrentAnimController;
        f32 fPickupTime = mpLooseBallInfo->mfPickupTime;
        pController->m_fPrevTime = pController->m_fTime;
        pController->m_fTime = 0.5f * fPickupTime;
        InitMovementFromAnim(0, v3Zero, 1.0f, false);
    }

    if (m_pBall == NULL)
    {
        PickupBall(g_pBall);
        mbPickedUp = true;
        g_pBall->ClearShotInProgress();
        EmitGoalieCatch(this, "goalie_catch", false);
    }
}

/**
 * Offset/Address/Size: 0x7328 | 0x80049E24 | size: 0xBC
 */
bool Goalie::IsCloseToPlane(const nlVector3& rPos1, const nlVector3& rPos2, float fThreshold)
{
    nlVector3 v3Dir;
    nlVector4 plane;

    nlVector3* pBallPos = &g_pBall->m_v3Position;

    v3Dir.f.x = pBallPos->f.y - rPos1.f.y;
    v3Dir.f.y = rPos1.f.x - pBallPos->f.x;
    v3Dir.f.z = 0.0f;

    MakePerpendicularPlane(*pBallPos, v3Dir, plane, 0.0f);

    float distance = (rPos2.f.x * plane.f.x) + (rPos2.f.y * plane.f.y) + (rPos2.f.z * plane.f.z) - plane.f.w;
    float absDistance = (float)fabsf(distance);

    return absDistance <= fThreshold;
}

/**
 * Offset/Address/Size: 0x7250 | 0x80049D4C | size: 0xD8
 */
bool Goalie::IsInsideGoalieBox(const nlVector3& rPos, float fXOffset, float fYOffset)
{
    float x = rPos.f.x;
    if (((float)fabs(x) > (cField::GetPenaltyBoxX(1U) - fXOffset)) && ((x * m_v3Position.f.x) > 0.0f))
    {
        float absPosY = (float)fabs(rPos.f.y);
        if (absPosY < (fYOffset + cField::GetPenaltyBoxY()))
        {
            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x6FE8 | 0x80049AE4 | size: 0x268
 */
/**
 * Offset/Address/Size: 0x6FF0 | 0x80049AE4 | size: 0x268
 * TODO: 99.74% match - f30/f31 register swap between absX and saveIgnoreMargin
 */
float Goalie::CheckForDelflectAwayFromNet()
{
    if (muBallDeflectCount != g_pBall->m_bBallDeflectCount)
    {
        nlVector3 v3TargetPosition;
        nlVector4 plane;
        nlVector3 localVelocity;

        float netX = m_pTeam->m_pNet->m_baseLocation.f.x;

        if (netX < 0.0f)
        {
            plane.f.y = 0.0f;
            plane.f.x = 1.0f;
            plane.f.z = 0.0f;
            plane.f.w = -netX;
        }
        else
        {
            plane.f.y = 0.0f;
            plane.f.x = -1.0f;
            plane.f.z = 0.0f;
            plane.f.w = -netX;
        }

        float saveIgnoreMargin;
        float absX;
        float result = FakeBallWorld::GetPredictedPlaneIntersectTime(plane, v3TargetPosition, localVelocity);

        if (!(result <= 0.0f))
        {
            absX = (float)fabs(v3TargetPosition.f.x);
            saveIgnoreMargin = ((GoalieTweaks*)m_pTweaks)->fSaveIgnoreMargin;

            bool bInNet;
            if (absX > (cField::GetGoalLineX(1U) - 1.0f)
                && (float)fabs(v3TargetPosition.f.y) < (saveIgnoreMargin + cNet::m_fNetWidth * 0.5f)
                && v3TargetPosition.f.z < (saveIgnoreMargin + cNet::m_fNetHeight))
            {
                bInNet = true;
            }
            else
            {
                bInNet = false;
            }

            if (bInNet)
                goto shotTarget;
        }

        CleanGoalieAction();

        mPrevGoalieActionState = mGoalieActionState;
        mGoalieActionState = GOALIEACTION_MOVE;
        mnSubstate = 0;

        SetAnimState(8, true, 0.2f, false, false);
        InitMovementFromAnim(0, v3Zero, 1.0f, false);

        mnSubstate = 1;
        mMoveDirection = GOALIEDIR_IDLE;

        m_pPhysicsCharacter->m_CanCollideWithBall = true;
        mbShouldMiss = false;
        mbDoNavigate = false;
        m_pPhysicsCharacter->m_CanCollidedWithGoalLine = true;
        m_pPhysicsCharacter->m_CanCollideWithWall = true;

        if (mbStunEffectActive)
        {
            KillDaze(this);
            mbStunEffectActive = false;
        }

        mpShooter = NULL;
        mUrgency = URGENCY_LOW;
        mfSpeedScale = 1.0f;
        mbPosGoalieNetCheck = false;
        mbNegGoalieNetCheck = false;
        mbDoHeadTrack = true;
        mbBallImpacted = false;
        mbNoUserControl = false;
        mbPickedUp = false;

        result = -1.0f;
        goto done;

    shotTarget:
        g_pBall->m_v3ShotTarget = v3TargetPosition;

    done:
        return result;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x6D80 | 0x8004987C | size: 0x268
 * TODO: 98.38% match - remaining differences are in animation-gate bool/register allocation
 * and resulting branch target offsets in the PRE_CROUCH transition tail.
 */
bool Goalie::CheckForLooseBallShotInProgress()
{
    cBall* pBall = g_pBall;
    if (pBall->m_pOwner == NULL)
    {
        cNet* pNet = m_pTeam->m_pNet;
        f32 looseBallShotDistance = ((GoalieTweaks*)m_pTweaks)->fLooseBallShotDistance;
        f32 d0 = pBall->m_v3Position.f.x - pNet->m_baseLocation.f.x;
        f32 d1 = pBall->m_v3Position.f.y - pNet->m_baseLocation.f.y;
        f32 distSq = (d0 * d0) + (d1 * d1);

        if (distSq < (looseBallShotDistance * looseBallShotDistance))
        {
            cTeam* pOtherTeam = m_pTeam->GetOtherTeam();
            cFielder* pShooter = NULL;
            f32 closestDistSq = 0.0f;

            for (s32 i = 0; i < 4; i++)
            {
                cFielder* pFielder = pOtherTeam->GetFielder(i);
                if (pFielder->m_eActionState == 7)
                {
                    f32 d0 = pBall->m_v3Position.f.x - pFielder->m_v3Position.f.x;
                    f32 d1 = pBall->m_v3Position.f.y - pFielder->m_v3Position.f.y;
                    f32 shooterDistSq = (d0 * d0) + (d1 * d1);

                    if (pShooter == NULL || shooterDistSq < closestDistSq)
                    {
                        pShooter = pFielder;
                        closestDistSq = shooterDistSq;
                    }
                }
            }

            mpShooter = pShooter;
            if (pShooter != NULL)
            {
                f32 d0 = pBall->m_v3Position.f.x - m_v3Position.f.x;
                f32 d1 = pBall->m_v3Position.f.y - m_v3Position.f.y;
                f32 goalieDistSq = (d0 * d0) + (d1 * d1);

                if (goalieDistSq > closestDistSq)
                {
                    nlVector4 plane;
                    nlVector3 v3Dir;
                    nlVector3* pBallPos = &g_pBall->m_v3Position;

                    v3Dir.f.x = pBallPos->f.y - mv3TargetPosition.f.y;
                    v3Dir.f.y = mv3TargetPosition.f.x - pBallPos->f.x;
                    v3Dir.f.z = 0.0f;

                    MakePerpendicularPlane(*pBallPos, v3Dir, plane, 0.0f);

                    float distance = (m_v3Position.f.x * plane.f.x) + (m_v3Position.f.y * plane.f.y) + (m_v3Position.f.z * plane.f.z) - plane.f.w;
                    float absDistance = (float)fabsf(distance);

                    if (absDistance <= 2.0f)
                    {
                        if (mGoalieActionState != GOALIEACTION_STS && mGoalieActionState != GOALIEACTION_STS_RECOVER)
                        {
                            mCrouchType = GOALIECROUCH_LOOSEBALL;
                            CleanGoalieAction();

                            mPrevGoalieActionState = mGoalieActionState;
                            mGoalieActionState = GOALIEACTION_PRE_CROUCH;
                            mnSubstate = 0;

                            bool bShouldSetAnim = (m_eAnimID != 0x2E) || (m_pCurrentAnimController->m_ePlayMode == 1 && m_pCurrentAnimController->m_fTime == 1.0f);

                            if (bShouldSetAnim)
                            {
                                SetAnimState(0x2E, true, 0.2f, false, false);
                            }

                            InitMovementFromAnim(0, v3Zero, 0.0f, false);
                        }

                        return true;
                    }

                    mUrgency = URGENCY_HIGH;
                }
            }
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x69D8 | 0x800494D4 | size: 0x3A8
 * TODO: 92.18% match - r29/r30 GPR swap (pFielder/bInNetZone), volatile FPR naming,
 * cror+bne+b vs ble branch patterns, return-false block duplication
 */
bool Goalie::CheckForSTSAttack()
{
    bool bInNetZone;
    cFielder* pFielder;
    bool bCanAttack;

    pFielder = g_pBall->GetOwnerFielder();

    if ((pFielder != NULL) && !IsOnSameTeam(pFielder) && (pFielder->m_eActionState == ACTION_SHOOT_TO_SCORE))
    {
        bCanAttack = true;
    }
    else
    {
        bCanAttack = false;
    }

    if (bCanAttack)
    {
        f32 fSaveIgnoreMargin;
        f32 fAnimScale;
        f32 ownerDistSq;
        f32 fCloseDistSq;
        f32 fMaxDistSq;
        f32 fCurrentAnimTime;

        pFielder = g_pBall->GetOwnerFielder();

        cPN_SAnimController* pController = pFielder->m_pCurrentAnimController;
        fAnimScale = (f32)pController->m_pSAnim->m_nNumKeys / 30.0f;
        fCurrentAnimTime = fAnimScale * pController->m_fTime;
        f32 fTriggerTime = fAnimScale * GetCurrentAnimTriggerTime(pFielder, 0x85181B83, 0);

        f32 fPickupDuration = LooseBallAnims::mAttackSTSInfo.mfPickupTime * LooseBallAnims::mAttackSTSInfo.mfAnimDuration;
        if ((fCurrentAnimTime + fPickupDuration) < fTriggerTime)
        {
            f32 dy = pFielder->m_v3Position.f.y - m_v3Position.f.y;
            f32 dx = pFielder->m_v3Position.f.x - m_v3Position.f.x;
            ownerDistSq = dx * dx + dy * dy;

            f32 fCloseDist = LooseBallAnims::mAttackSTSInfo.mfPickupDistance + ((GoalieTweaks*)m_pTweaks)->fSTSAttackCloseDistance;
            f32 fMaxDist = LooseBallAnims::mAttackSTSInfo.mfPickupDistance + ((GoalieTweaks*)m_pTweaks)->fSTSAttackMaxDistance;
            fCloseDistSq = fCloseDist * fCloseDist;
            fMaxDistSq = fMaxDist * fMaxDist;

            fSaveIgnoreMargin = ((GoalieTweaks*)m_pTweaks)->fSaveIgnoreMargin;

            if (((float)fabsf(pFielder->m_v3Position.f.x) > (cField::GetGoalLineX(1U) - 1.0f))
                && ((float)fabsf(pFielder->m_v3Position.f.y) < (cNet::m_fNetWidth * 0.5f + fSaveIgnoreMargin))
                && (pFielder->m_v3Position.f.z < (fSaveIgnoreMargin + cNet::m_fNetHeight)))
            {
                bInNetZone = true;
            }
            else
            {
                bInNetZone = false;
            }

            nlVector3 v3GoalPos = m_pTeam->m_pNet->m_baseLocation;

            f32 halfWidth = 0.5f * cNet::m_fNetWidth;
            f32 clampedY = -halfWidth;

            if (pFielder->m_v3Position.f.y >= clampedY)
            {
                clampedY = pFielder->m_v3Position.f.y;
            }

            if (clampedY > halfWidth)
            {
                clampedY = halfWidth;
            }

            v3GoalPos.f.y = clampedY;

            f32 dfx = v3GoalPos.f.x - pFielder->m_v3Position.f.x;
            f32 dfy = v3GoalPos.f.y - pFielder->m_v3Position.f.y;
            f32 dgx = v3GoalPos.f.x - m_v3Position.f.x;
            f32 dgy = v3GoalPos.f.y - m_v3Position.f.y;
            f32 distSqFielder = dfx * dfx + dfy * dfy;
            f32 distSqGoalie = dgx * dgx + dgy * dgy;

            static FilteredRandomChance randgenSTS;

            if (!bInNetZone)
            {
                if (distSqFielder >= distSqGoalie)
                {
                    if (ownerDistSq <= fCloseDistSq)
                    {
                    }
                    else if (ownerDistSq <= fMaxDistSq)
                    {
                        if (!randgenSTS.genrand(((GoalieTweaks*)m_pTweaks)->fSTSAttackChancePerFrame))
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            f32 fTimeToImpact = fAnimScale * GetCurrentAnimTriggerTime(pFielder, 0x2C8DABFA, 0);
            fTimeToImpact = fTimeToImpact - fCurrentAnimTime;
            f32 fPickupDuration2 = LooseBallAnims::mAttackSTSInfo.mfPickupTime * LooseBallAnims::mAttackSTSInfo.mfAnimDuration;
            fTimeToImpact = fTimeToImpact - fPickupDuration2;

            mfWaitTime = fTimeToImpact;
            if (fTimeToImpact < 0.0f)
            {
                fTimeToImpact = 0.0f;
            }

            mfTargetTime = fTimeToImpact;
            mpLooseBallInfo = &LooseBallAnims::mAttackSTSInfo;
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_STS_ATTACK_SETUP;
            mnSubstate = 0;
            mUrgency = URGENCY_LOW;
            ActionSTSAttackSetup(1.0f);

            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x659C | 0x80049098 | size: 0x43C
 * TODO: 99.65% match - register allocation diffs: 8 FPR in pass-target
 * distance math and 6 FPR in inner ballPosX/fabs block.
 */
extern float GonnaGetBall(cTeam*);

bool Goalie::IsLooseBallClose(float fDistFromBox)
{
    bool bBallIsLoose = true;
    cBall* pBall = g_pBall;
    if (pBall->m_pPassTarget != NULL)
    {
        nlVector3 leftPost;
        nlVector3 rightPost;
        bool coneResult;
        if (pBall->m_pOwner != NULL)
        {
            coneResult = false;
        }
        else
        {
            nlVector3 ballVelocity = pBall->m_v3Velocity;
            float velMagSq = (ballVelocity.f.x * ballVelocity.f.x) + (ballVelocity.f.y * ballVelocity.f.y) + (ballVelocity.f.z * ballVelocity.f.z);
            if (velMagSq < 0.01f)
            {
                coneResult = false;
            }
            else
            {
                m_pTeam->m_pNet->GetPostLocation(rightPost, 0, 0.0f);
                m_pTeam->m_pNet->GetPostLocation(leftPost, 1, 0.0f);
                const nlVector3& ballPos = g_pBall->m_v3Position;
                nlVec3Add(ballVelocity, ballVelocity, ballPos);
                coneResult = IsPointInCone(ballVelocity, ballPos, rightPost, leftPost);
            }
        }
        do
        {
            if (coneResult)
            {
                cPlayer* pPassTarget = g_pBall->m_pPassTarget;
                float dxToTarget = m_v3Position.f.x - pPassTarget->m_v3Position.f.x;
                float dyToTarget = m_v3Position.f.y - pPassTarget->m_v3Position.f.y;
                float dyToBall = m_v3Position.f.y - pBall->m_v3Position.f.y;
                float dxToBall = m_v3Position.f.x - pBall->m_v3Position.f.x;
                float distToPassTargetSq = (dxToTarget * dxToTarget) + (dyToTarget * dyToTarget);
                float distToBallSq = (dxToBall * dxToBall) + (dyToBall * dyToBall);
                if (distToBallSq > distToPassTargetSq)
                {
                }
                else
                {
                    break;
                }
            }
            bBallIsLoose = false;
        } while (false);
    }
    pBall = g_pBall;
    if ((pBall->m_pOwner == NULL) && bBallIsLoose)
    {
        float sideSign = m_pTeam->m_pNet->m_baseLocation.f.x;
        if (sideSign * pBall->m_v3Position.f.x < 0.0f)
        {
            return false;
        }
        float goalLineX = cField::GetGoalLineX(1U);
        float penaltyY = cField::GetPenaltyBoxY();
        float absBallX = (float)fabs(pBall->m_v3Position.f.x);
        float absBallY = (float)fabs(pBall->m_v3Position.f.y);
        if ((absBallX > (goalLineX - 2.0f)) && (absBallY < penaltyY))
        {
            return true;
        }
        if ((GonnaGetBall(m_pTeam) > 0.75f) && (absBallX < (goalLineX - 3.0f)) && (absBallY > penaltyY))
        {
            return false;
        }
        {
            float ballPosX = pBall->m_v3Position.f.x;
            float ballAbsXNew = (float)fabs(ballPosX);
            bool innerCheck;
            do
            {
                if (ballAbsXNew > cField::GetPenaltyBoxX(1U) - fDistFromBox)
                {
                    if (ballPosX * m_v3Position.f.x > 0.0f)
                    {
                        float ballAbsYNew = (float)fabs(pBall->m_v3Position.f.y);
                        if (ballAbsYNew < fDistFromBox + cField::GetPenaltyBoxY())
                        {
                            innerCheck = true;
                            break;
                        }
                    }
                }
                innerCheck = false;
            } while (false);
            if (innerCheck)
            {
                return true;
            }
        }
        float netSideSign = m_pTeam->m_pNet->m_sideSign;
        cBall* pBallVel = g_pBall;
        {
            float t = cField::GetPenaltyBoxX(1U) - fDistFromBox;
            float penaltyBoxXLimit = (0.0f >= t) ? 0.0f : t;
            float t2 = penaltyY + fDistFromBox;
            float penaltyBoxYLimit = (0.0f >= t2) ? 0.0f : t2;
            if (absBallX < penaltyBoxXLimit)
            {
                float fForwardVelX = pBallVel->m_v3Velocity.f.x * netSideSign;
                if (fForwardVelX < 1.0f)
                {
                    return false;
                }
                float fXtime = (penaltyBoxXLimit - absBallX) / fForwardVelX;
                if (fXtime > 0.3f)
                {
                    return false;
                }
                float projectedY = fXtime * pBallVel->m_v3Velocity.f.y + pBall->m_v3Position.f.y;
                if ((float)fabs(projectedY) < penaltyBoxYLimit)
                {
                    return true;
                }
            }
            if (absBallY > penaltyBoxYLimit)
            {
                float ballY = pBall->m_v3Position.f.y;
                float ySign;
                if (ballY > 0.0f)
                {
                    ySign = 1.0f;
                }
                else
                {
                    ySign = -1.0f;
                }
                float fForwardVelY = pBallVel->m_v3Velocity.f.y * ySign;
                if (fForwardVelY > -1.0f)
                {
                    return false;
                }
                float fYtime = (penaltyBoxYLimit - absBallY) / fForwardVelY;
                if (fYtime > 0.3f)
                {
                    return false;
                }
                float projectedX = fYtime * pBallVel->m_v3Velocity.f.x + pBall->m_v3Position.f.x;
                if (projectedX * netSideSign > penaltyBoxXLimit)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

static inline f32 DistSq(f32 a, f32 b)
{
    return (b * b) + (a * a);
}

/**
 * Offset/Address/Size: 0x64C0 | 0x80048FBC | size: 0xDC
 */
bool Goalie::IsWithinPounceRange()
{
    cFielder* pFielder = g_pBall->GetOwnerFielder();
    if ((pFielder != NULL) && !IsOnSameTeam(pFielder))
    {
        if (pFielder->m_eActionState == ACTION_SHOOT_TO_SCORE)
        {
            return false;
        }

        f32 dy = m_v3Position.f.y - pFielder->m_v3Position.f.y;
        f32 range = LooseBallAnims::mTrapBallInfo.mfPickupDistance + ((GoalieTweaks*)m_pTweaks)->fPounceRange;
        f32 dx = m_v3Position.f.x - pFielder->m_v3Position.f.x;
        f32 rangeSq = range * range;

        // TODO: 99.0% match - f2/f4 float register swap (selfY/rangeSq). Tested 14+ approaches, 3 compilers - unresolvable MWCC quirk.
        if ((rangeSq > DistSq(dy, dx))
            || (rangeSq > DistSq(m_v3Position.f.y - g_pBall->m_v3Position.f.y, m_v3Position.f.x - g_pBall->m_v3Position.f.x)))
        {
            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x6388 | 0x80048E84 | size: 0x138
 */
bool Goalie::IsOpponentBallCarrierInRange()
{
    cFielder* pFielder = g_pBall->GetOwnerFielder();
    if ((pFielder != NULL) && !IsOnSameTeam(pFielder))
    {
        if (pFielder->m_eActionState == ACTION_SHOOT_TO_SCORE)
        {
            return false;
        }

        if (IsInsideGoalieBox(pFielder->m_v3Position, 0.0f, 0.0f))
        {
            SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            f32 dy = m_v3Position.f.y - pFielder->m_v3Position.f.y;
            f32 dx = m_v3Position.f.x - pFielder->m_v3Position.f.x;
            f32 distSq = pSkillTweaks->fAttackCarrierDistance * pSkillTweaks->fAttackCarrierDistance;
            if ((dx * dx + dy * dy) < distSq)
            {
                return true;
            }
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x631C | 0x80048E18 | size: 0x6C
 */
bool Goalie::IsOpponentInSTS()
{
    cFielder* pFielder = g_pBall->GetOwnerFielder();
    if ((pFielder != NULL) && !IsOnSameTeam(pFielder) && (pFielder->m_eActionState == ACTION_SHOOT_TO_SCORE))
    {
        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x6218 | 0x80048D14 | size: 0x104
 */
bool Goalie::IsPassThreat()
{
    bool var_r0;

    cPlayer* pPassTarget = g_pBall->m_pPassTarget;
    if (pPassTarget != NULL)
    {
        if (!IsOnSameTeam(pPassTarget))
        {
            cBall* pBall = g_pBall;
            float absPassX = (float)fabs(pBall->m_v3PassIntercept.f.x);
            float ballPassX = pBall->m_v3PassIntercept.f.x;

            if ((absPassX > (cField::GetPenaltyBoxX(1U) - 2.0f))
                && (ballPassX * m_v3Position.f.x > 0.0f)
                && ((float)fabs(pBall->m_v3PassIntercept.f.y) < (cField::GetPenaltyBoxY() + 2.0f)))
            {
                var_r0 = true;
            }
            else
            {
                var_r0 = false;
            }

            if (var_r0)
            {
                mpPassTarget = pPassTarget;
                muBallDeflectCount = g_pBall->m_bBallDeflectCount;
                return true;
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0x6104 | 0x80048C00 | size: 0x114
 */
void Goalie::MakeSaveEvent(bool bIsSTS)
{
    Event* pEvent = g_pEventManager->CreateValidEvent(0xF, 0x38);
    GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();

    pSaveData->pGoalie = this;
    pSaveData->v3BallVelocity = g_pBall->m_v3Velocity;

    GoalieTweaks* pTweaks = (GoalieTweaks*)m_pTweaks;
    pSaveData->fWowFactor = 1.0f / pTweaks->fShotFatigueMax;

    pSaveData->isSTS = bIsSTS;

    pSaveData->saveType = g_pBall->m_uGoalType;
    pSaveData->pShooter = g_pBall->m_pShooter;

    if (mpSaveData != NULL)
    {
        pSaveData->padding = mpSaveData->muSaveType;
        pSaveData->fWowFactor *= mpSaveData->mfFatigueValue;
    }
    else
    {
        pSaveData->padding = 3;
        pSaveData->fWowFactor *= ((GoalieTweaks*)m_pTweaks)->fShotFatigueDefault;
    }
}

/**
 * Offset/Address/Size: 0x5FF8 | 0x80048AF4 | size: 0x10C
 */
void Goalie::UpdateActionState(float fDeltaTime)
{
    switch (mGoalieActionState)
    {
    case GOALIEACTION_MOVE:
        ActionMove(fDeltaTime);
        break;
    case GOALIEACTION_MOVE_WB:
        ActionMoveWB(fDeltaTime);
        break;
    case GOALIEACTION_SAVE_SETUP:
        ActionSaveSetup(fDeltaTime);
        break;
    case GOALIEACTION_SAVE_REPOSITION:
        ActionSaveReposition(fDeltaTime);
        break;
    case GOALIEACTION_SAVE:
        ActionSave(fDeltaTime);
        break;
    case GOALIEACTION_MISS_CHIP_SHOT:
        ActionChipShotStumble(fDeltaTime);
        break;
    case GOALIEACTION_DIVE_RECOVER:
        ActionDiveRecover(fDeltaTime);
        break;
    case GOALIEACTION_STS_SETUP:
        ActionSTSSetup(fDeltaTime);
        break;
    case GOALIEACTION_STS:
        ActionSTS(fDeltaTime);
        break;
    case GOALIEACTION_STS_RECOVER:
        ActionSTSRecover(fDeltaTime);
        break;
    case GOALIEACTION_PASS:
        ActionPass(fDeltaTime);
        break;
    case GOALIEACTION_PASS_INTERCEPT:
        ActionPassIntercept(fDeltaTime);
        break;
    case GOALIEACTION_PRE_CROUCH:
        ActionPreCrouch(fDeltaTime);
        break;
    case GOALIEACTION_PURSUE_BALL_CARRIER:
        ActionPursueBallCarrier(fDeltaTime);
        break;
    case GOALIEACTION_PURSUE_BALL_POUNCE:
        ActionPursueBallPounce(fDeltaTime);
        break;
    case GOALIEACTION_LOOSEBALL_SETUP:
        ActionLooseBallSetup(fDeltaTime);
        break;
    case GOALIEACTION_LOOSEBALL_CATCH:
        ActionLooseBallCatch(fDeltaTime);
        break;
    case GOALIEACTION_LOOSEBALL_PICKUP:
        ActionLooseBallPickup(fDeltaTime);
        break;
    case GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING:
        ActionLooseBallPursueBouncing(fDeltaTime);
        break;
    case GOALIEACTION_LOOSEBALL_PURSUE_ROLLING:
        ActionLooseBallPursueRolling(fDeltaTime);
        break;
    case GOALIEACTION_LOOSEBALL_DESPERATE:
        ActionLooseBallDesperate(fDeltaTime);
        break;
    case GOALIEACTION_OFFPLAY:
        ActionOffplay(fDeltaTime);
        break;
    case GOALIEACTION_SNAP_BALL:
        ActionSnapBall(fDeltaTime);
        break;
    case GOALIEACTION_GRAB_BALL:
        ActionGrabBall(fDeltaTime);
        break;
    case GOALIEACTION_STS_ATTACK_SETUP:
        ActionSTSAttackSetup(fDeltaTime);
        break;
    case GOALIEACTION_STS_ATTACK:
        ActionSTSAttack(fDeltaTime);
        break;
    }
}

/**
 * Offset/Address/Size: 0x5FA4 | 0x80048AA0 | size: 0x54
 */
void Goalie::SetGoalieAction(eGoalieActionState newGoalieState, int newSubstate)
{
    CleanGoalieAction();
    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = newGoalieState;
    mnSubstate = newSubstate;
}

/**
 * Offset/Address/Size: 0x5F40 | 0x80048A3C | size: 0x64
 */
void Goalie::SaveBlendCallback(unsigned int nParam, cPN_SAnimController* pAnimCtrl)
{
    Goalie* pThis = reinterpret_cast<Goalie*>(nParam & ~3U);
    unsigned int saveDataIndex = nParam & 3U;

    SaveData* pSaveData = pThis->mBlendInfo.mpSaveData[saveDataIndex];
    if (pSaveData == NULL)
    {
        return;
    }

    float fTime = pAnimCtrl->m_fTime;

    int milestoneIndex = 0;

    while (milestoneIndex < 4 && fTime >= pSaveData->mfMilestonePercent[milestoneIndex])
    {
        milestoneIndex++;
    }

    pAnimCtrl->m_fPlaybackSpeedScale = pThis->mBlendInfo.mfMilestoneScale[saveDataIndex][milestoneIndex];
}

static inline int GetAnimID(SaveBlendInfo& blend, int index)
{
    return blend.mpSaveData[index]->mnAnimID;
}

/**
 * Offset/Address/Size: 0x5D3C | 0x80048838 | size: 0x204
 */
cPoseNode* Goalie::SetupBlender(bool bPrimary, const float* fStartPercent, int nMainAnimID, int nMilestone)
{
    float fBlend;
    int index1;
    cPN_SAnimController* pSaveController1;
    int index2;

    if (bPrimary)
    {
        fBlend = mBlendInfo.mfSaveBlendPrimary;
        index1 = 0;
        index2 = 1;
    }
    else
    {
        fBlend = mBlendInfo.mfSaveBlendSecondary;
        index1 = 2;
        index2 = 3;
    }
    int animID = GetAnimID(mBlendInfo, index1);
    pSaveController1 = NewAnimController(animID, false, false, SaveBlendCallback, index1 + (unsigned int)this);
    pSaveController1->m_fPlaybackSpeedScale = mBlendInfo.mfMilestoneScale[index1][nMilestone];
    if (fStartPercent[index1] > 0.0f)
    {
        pSaveController1->m_fPrevTime = pSaveController1->m_fTime;
        pSaveController1->m_fTime = fStartPercent[index1];
    }
    cPoseNode* result = pSaveController1;
    if (nMainAnimID == animID)
    {
        m_pCurrentAnimController = pSaveController1;
    }
    else
    {
        pSaveController1->m_bIgnoreTriggers = true;
    }
    if (fBlend >= 0.0f)
    {
        animID = GetAnimID(mBlendInfo, index2);
        cPN_SAnimController* pSaveController2 = NewAnimController(animID, false, false, SaveBlendCallback, index2 + (unsigned int)this);
        pSaveController2->m_fPlaybackSpeedScale = mBlendInfo.mfMilestoneScale[index2][nMilestone];
        if (fStartPercent[index2] > 0.0f)
        {
            pSaveController2->m_fPrevTime = pSaveController2->m_fTime;
            pSaveController2->m_fTime = fStartPercent[index2];
        }
        if (nMainAnimID == animID)
        {
            m_pCurrentAnimController = pSaveController2;
        }
        else
        {
            pSaveController2->m_bIgnoreTriggers = true;
        }

        cPN_SingleAxisBlender* pPoseNode = new (AllocateSingleAxisBlender()) cPN_SingleAxisBlender(2, NULL, 0, 1.0f);
        pPoseNode->m_fDesiredWeight = fBlend;
        pPoseNode->m_fSmoothedWeight = fBlend;
        pPoseNode->SetChild(0, pSaveController1);
        pPoseNode->SetChild(1, pSaveController2);
        result = pPoseNode;
    }
    return result;
}

/**
 * Offset/Address/Size: 0x58E4 | 0x800483E0 | size: 0x458
 * TODO: 87.30% match - this-pointer register allocation remains shifted (r28 vs r29),
 * and milestone array loads still compile as add+lfs instead of addi+lfsx in several blocks.
 */
class cPN_Blender : public cPoseNode
{
public:
    cPN_Blender(cPoseNode*, cPoseNode*, float);
    virtual ~cPN_Blender() { };
    virtual void Evaluate(float, cPoseAccumulator*) const;
    virtual void Evaluate(int, float, cPoseAccumulator*) const;
    virtual cPoseNode* Update(float);
    virtual int GetType() { return 0x0; };
    virtual void BlendRootTrans(nlVector3*, float, float*);
    virtual void BlendRootRot(unsigned short*, float, float*);

    static SlotPool<cPN_Blender> m_BlenderSlotPool;
};

inline cPN_Blender* AllocateBlender()
{
    cPN_Blender* pBlender = NULL;
    if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, 0x1C);
    }
    if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
    {
        pBlender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
        cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
    }
    return pBlender;
}

/**
 * Offset/Address/Size: 0x58E4 | 0x800483E0 | size: 0x458
 * TODO: 87.63% match - this=r28 vs r29 (dual pool alloc shifts register), slwi r0 vs r4 (prevents lfsx pattern)
 */
void Goalie::PlayBlendedAnims(float fStartTime, int nMilestone)
{
    static float fDefaultStartPercent[] = { 0.4f, 0.7f };

    float fStartPercent[4];
    int nMainAnimID;
    int milestone;

    if (mBlendInfo.mfSaveBlendComposite <= 0.001f && mBlendInfo.mfSaveBlendPrimary <= 0.001f)
    {
        SetAnimState(mpSaveData->mnAnimID, true, 0.2f, false, false);

        if (nMilestone >= 0 && nMilestone < 2)
        {
            fStartTime = mBlendInfo.mfMilestoneTime[nMilestone];
            if (fStartTime <= 0.0f)
            {
                fStartTime = fDefaultStartPercent[nMilestone] * (mpSaveData->mfMilestonePercent[2] * mpSaveData->mfDuration);
            }
        }

        if (fStartTime > 0.0f && fStartTime < mpSaveData->mfDuration)
        {
            cPN_SAnimController* pController = m_pCurrentAnimController;
            f32 fAnimTime = fStartTime / mpSaveData->mfDuration;

            pController->m_fPrevTime = pController->m_fTime;
            pController->m_fTime = fAnimTime;
        }
    }
    else
    {
        nMainAnimID = mpSaveData->mnAnimID;

        if (nMilestone >= 0)
        {
            milestone = nMilestone;

            if (mBlendInfo.mfMilestoneTime[nMilestone] > 0.0f)
            {
                if (mBlendInfo.mpSaveData[0] != NULL)
                {
                    fStartPercent[0] = mBlendInfo.mpSaveData[0]->mfMilestonePercent[nMilestone];
                }
                if (mBlendInfo.mpSaveData[1] != NULL)
                {
                    fStartPercent[1] = mBlendInfo.mpSaveData[1]->mfMilestonePercent[nMilestone];
                }
                if (mBlendInfo.mpSaveData[2] != NULL)
                {
                    fStartPercent[2] = mBlendInfo.mpSaveData[2]->mfMilestonePercent[nMilestone];
                }
                if (mBlendInfo.mpSaveData[3] != NULL)
                {
                    fStartPercent[3] = mBlendInfo.mpSaveData[3]->mfMilestonePercent[nMilestone];
                }
            }
            else
            {
                if (mBlendInfo.mpSaveData[0] != NULL)
                {
                    fStartPercent[0] = fDefaultStartPercent[nMilestone] * mBlendInfo.mpSaveData[0]->mfMilestonePercent[2];
                }
                if (mBlendInfo.mpSaveData[1] != NULL)
                {
                    fStartPercent[1] = fDefaultStartPercent[nMilestone] * mBlendInfo.mpSaveData[1]->mfMilestonePercent[2];
                }
                if (mBlendInfo.mpSaveData[2] != NULL)
                {
                    fStartPercent[2] = fDefaultStartPercent[nMilestone] * mBlendInfo.mpSaveData[2]->mfMilestonePercent[2];
                }
                if (mBlendInfo.mpSaveData[3] != NULL)
                {
                    fStartPercent[3] = fDefaultStartPercent[nMilestone] * mBlendInfo.mpSaveData[3]->mfMilestonePercent[2];
                }
            }
        }
        else if (fStartTime > 0.0f)
        {
            float fPrevMilestone = 0.0f;

            milestone = 0;
            while (milestone < 4 && fStartTime >= mBlendInfo.mfMilestoneTime[milestone])
            {
                if (mBlendInfo.mfMilestoneTime[milestone] > fPrevMilestone)
                {
                    fPrevMilestone = mBlendInfo.mfMilestoneTime[milestone];
                }
                milestone++;
            }

            fStartTime = NormalizeVal(fStartTime, fPrevMilestone, mBlendInfo.mfMilestoneTime[milestone]);

            int prevMilestone = milestone - 1;
            int currMilestone = milestone;

            for (int i = 0; i < 4; i++)
            {
                SaveData* pData = mBlendInfo.mpSaveData[i];
                if (pData != NULL)
                {
                    float fStart = 0.0f;
                    if (milestone > 0)
                    {
                        fStart = pData->mfMilestonePercent[prevMilestone];
                    }

                    fStartPercent[i] = Interpolate(fStart, pData->mfMilestonePercent[currMilestone], fStartTime);
                }
            }
        }
        else
        {
            fStartPercent[0] = 0.0f;
            fStartPercent[1] = 0.0f;
            fStartPercent[2] = 0.0f;
            fStartPercent[3] = 0.0f;
            milestone = 0;
        }

        cPoseNode* pMainNode = SetupBlender(true, fStartPercent, nMainAnimID, milestone);

        if (mBlendInfo.mfSaveBlendComposite >= 0.001f)
        {
            cPoseNode* pOther = SetupBlender(false, fStartPercent, nMainAnimID, milestone);
            cPN_SingleAxisBlender* pBlend = new (AllocateSingleAxisBlender()) cPN_SingleAxisBlender(2, NULL, 0, 0.1f);

            pBlend->m_fDesiredWeight = mBlendInfo.mfSaveBlendComposite;
            pBlend->m_fSmoothedWeight = mBlendInfo.mfSaveBlendComposite;
            pBlend->SetChild(0, pMainNode);
            pBlend->SetChild(1, pOther);

            pMainNode = pBlend;
        }

        cPN_Blender* pBlender = new (AllocateBlender()) cPN_Blender(m_pAILayer[0], pMainNode, 0.1f);

        m_pAILayer[0] = pBlender;
        SetAnimID(nMainAnimID);
    }

    InitMovementFromAnim(0, v3Zero, 1.0f, true);
}

/**
 * Offset/Address/Size: 0x5878 | 0x80048374 | size: 0x6C
 */
void Goalie::PlayNewAnim(int nAnimID)
{
    if (nAnimID == m_eAnimID)
    {
        cPN_SAnimController* pController = m_pCurrentAnimController;
        bool bSkipSetAnimState = false;

        if (pController->m_ePlayMode == PM_HOLD && pController->m_fTime == 1.0f)
        {
            bSkipSetAnimState = true;
        }

        if (!bSkipSetAnimState)
        {
            return;
        }
    }

    SetAnimState(nAnimID, true, 0.2f, false, false);
}

/**
 * Offset/Address/Size: 0x5760 | 0x8004825C | size: 0x118
 */
void Goalie::CleanGoalieAction()
{
    switch (mGoalieActionState)
    {
    case GOALIEACTION_MOVE:
        mbDoNavigate = false;
        break;

    case GOALIEACTION_SAVE_SETUP:
    case GOALIEACTION_SAVE_REPOSITION:
    case GOALIEACTION_SAVE:
        if (mpSaveData != NULL)
        {
            if (mpSaveData->mnRecoverAnimID < 0)
            {
                mpSaveData = NULL;
            }
        }
        muSaveType = 0xFFFF;
        mbShouldMiss = false;
        mpPassTarget = NULL;
        mbBallImpacted = false;
        break;

    case GOALIEACTION_MISS_CHIP_SHOT:
        mpSaveData = NULL;
        break;

    case GOALIEACTION_DIVE_RECOVER:
        mbPosGoalieNetCheck = false;
        mbNegGoalieNetCheck = false;
        break;

    case GOALIEACTION_STS_SETUP:
    case GOALIEACTION_STS:
        mpShooter = NULL;
        break;

    case GOALIEACTION_STS_RECOVER:
        mpSaveData = NULL;
        if (mbStunEffectActive != false)
        {
            KillDaze(this);
            mbStunEffectActive = false;
        }
        break;

    case GOALIEACTION_STS_ATTACK_SETUP:
    case GOALIEACTION_STS_ATTACK:
        mbPlayMiss = false;
        break;

    case GOALIEACTION_LOOSEBALL_PICKUP:
        mbPlayMiss = false;
        mbNoUserControl = false;
        mbPickedUp = false;
        break;

    case GOALIEACTION_OFFPLAY:
        mnOffplayPending = GOALIE_OFFPLAY_NONE;
        break;

    case GOALIEACTION_SNAP_BALL:
        mpShooter = NULL;
        break;

    case GOALIEACTION_GRAB_BALL:
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x56D4 | 0x800481D0 | size: 0x8C
 */
void Goalie::InitActionLooseBallCatch()
{
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_LOOSEBALL_CATCH;
    mnSubstate = 0;

    mv3LocalContactPosition.f.x = 0.2f;

    mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, mfTargetTime, false, 1, true);

    mpLooseBallInfo = NULL;
    mMoveDirection = GOALIEDIR_IDLE;

    if (mpSaveData == NULL)
    {
        InitActionLooseBallSetup();
    }
}

/**
 * Offset/Address/Size: 0x3BD0 | 0x800466CC | size: 0x1B04
 */
void Goalie::InitActionLooseBallSetup()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x3AA4 | 0x800465A0 | size: 0x12C
 */
void Goalie::InitActionMove(bool bParam)
{
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_MOVE;
    mnSubstate = 0;

    SetAnimState(8, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    mnSubstate = 1;
    mMoveDirection = GOALIEDIR_IDLE;

    m_pPhysicsCharacter->m_CanCollideWithBall = 1;
    mbShouldMiss = false;
    mbDoNavigate = false;
    m_pPhysicsCharacter->m_CanCollidedWithGoalLine = 1;
    m_pPhysicsCharacter->m_CanCollideWithWall = 1;

    if (mbStunEffectActive)
    {
        KillDaze(this);
        mbStunEffectActive = false;
    }

    mpShooter = NULL;
    mUrgency = URGENCY_LOW;
    mfSpeedScale = 1.0f;
    mbPosGoalieNetCheck = false;
    mbNegGoalieNetCheck = false;
    mbDoHeadTrack = true;
    mbBallImpacted = false;
    mbNoUserControl = false;
    mbPickedUp = false;

    if (bParam)
    {
        ActionMove(0.0f);
    }
}

/**
 * Offset/Address/Size: 0x39F4 | 0x800464F0 | size: 0xB0
 */
void Goalie::InitActionMoveWB()
{
    if (m_pBall == NULL)
    {
        PickupBall(g_pBall);
    }

    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_MOVE_WB;
    mnSubstate = 0;

    SetAnimState(9, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    mfWaitTime = ((GoalieTweaks*)m_pTweaks)->fGoalieBallTime;
    mfTargetTime = 0.0f;
    mpPassTarget = NULL;
}

/**
 * Offset/Address/Size: 0x32E8 | 0x80045DE4 | size: 0x70C
 */
void Goalie::InitActionSaveSetup(bool)
{
}

/**
 * Offset/Address/Size: 0x2F9C | 0x80045A98 | size: 0x34C
 * TODO: 99.95% match - fmadds frA/frC operand swap for 0.5f * m_fNetWidth (commutative, same result)
 */
void Goalie::InitActionSave()
{
    float absX = (float)fabs(g_pBall->m_v3ShotTarget.f.x);
    if (absX > (cField::GetGoalLineX(1U) - 0.2f))
    {
        cBall* pBall = g_pBall;
        float saveIgnoreMargin = ((GoalieTweaks*)m_pTweaks)->fSaveIgnoreMargin;
        double shotAbsX = __fabs(pBall->m_v3ShotTarget.f.x);

        bool bInNet;
        if ((float)shotAbsX > (cField::GetGoalLineX(1U) - 1.0f)
            && (float)fabs(pBall->m_v3ShotTarget.f.y) < (cNet::m_fNetWidth / 2.0f + saveIgnoreMargin)
            && pBall->m_v3ShotTarget.f.z < (saveIgnoreMargin + cNet::m_fNetHeight))
        {
            bInNet = true;
        }
        else
        {
            bInNet = false;
        }

        if (bInNet == false)
        {
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_MOVE;
            mnSubstate = 0;

            SetAnimState(8, true, 0.2f, false, false);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);

            mnSubstate = 1;
            mMoveDirection = GOALIEDIR_IDLE;

            m_pPhysicsCharacter->m_CanCollideWithBall = true;
            mbShouldMiss = false;
            mbDoNavigate = false;
            m_pPhysicsCharacter->m_CanCollidedWithGoalLine = true;
            m_pPhysicsCharacter->m_CanCollideWithWall = true;

            if (mbStunEffectActive)
            {
                KillDaze(this);
                mbStunEffectActive = false;
            }

            mpShooter = NULL;
            mUrgency = URGENCY_LOW;
            mfSpeedScale = 1.0f;
            mbPosGoalieNetCheck = false;
            mbNegGoalieNetCheck = false;
            mbDoHeadTrack = true;
            mbBallImpacted = false;
            mbNoUserControl = false;
            mbPickedUp = false;

            return;
        }
    }

    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_SAVE;
    mnSubstate = 0;

    mFatigue.RegisterShot(mpSaveData->mfFatigueValue);
    mbBallImpacted = false;

    if (mbShouldMiss)
    {
        if (mpSaveData->mpFailAnimData)
        {
            mpSaveData = mpSaveData->mpFailAnimData;

            SetAnimState(mpSaveData->mnAnimID, true, 0.2f, false, false);
            InitMovementFromAnim(0, v3Zero, 0.0f, false);
        }
        else
        {
            PlayBlendedAnims(0.0f, -1);
        }
    }
    else
    {
        PlayBlendedAnims(mBlendInfo.mfStartTime, -1);
    }

    Event* pEvent = g_pEventManager->CreateValidEvent(0x13, 0x38);
    GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();

    pSaveData->pGoalie = this;
    pSaveData->v3BallVelocity = v3Zero;
    pSaveData->fWowFactor = 0.0f;
    pSaveData->isSTS = 0;

    pSaveData->saveType = g_pBall->m_uGoalType;
    pSaveData->pShooter = g_pBall->m_pShooter;

    if (mpSaveData)
    {
        pSaveData->padding = mpSaveData->muSaveType;
    }
    else
    {
        pSaveData->padding = 3;
    }
}

/**
 * Offset/Address/Size: 0x2D4C | 0x80045848 | size: 0x250
 */
void Goalie::InitActionSTSSetup()
{
    if (mGoalieActionState == GOALIEACTION_PURSUE_BALL_POUNCE || mGoalieActionState == GOALIEACTION_DIVE_RECOVER)
    {
        return;
    }

    mnOffplayPending = GOALIE_OFFPLAY_NONE;

    if (mGoalieActionState != GOALIEACTION_STS_RECOVER)
    {
        CleanGoalieAction();
        mPrevGoalieActionState = mGoalieActionState;
        mGoalieActionState = GOALIEACTION_STS_SETUP;
        mnSubstate = 0;
    }
    else
    {
        SaveData* pSavedSaveData = mpSaveData;
        CleanGoalieAction();
        mPrevGoalieActionState = mGoalieActionState;
        mGoalieActionState = GOALIEACTION_STS_SETUP;
        mnSubstate = 0;
        mpSaveData = pSavedSaveData;
    }

    SetDesiredSaveFacing(g_pBall->m_v3Position);

    nlVector4 plane;
    const u16 desiredFacingDirection = m_aDesiredFacingDirection;
    const nlVector3& pPosition = m_v3Position;

    MakePerpendicularPlane(pPosition, desiredFacingDirection, plane, 0.2f);

    nlVector3 localVelocity;
    float time = FakeBallWorld::GetPredictedPlaneIntersectTime(plane, mv3TargetPosition, localVelocity);
    double absX = __fabs(mv3TargetPosition.f.x);

    if ((float)absX > cField::GetGoalLineX(1U))
    {
        time = -1.0f;
    }
    else if (time > 0.0f)
    {
        GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, pPosition, desiredFacingDirection);
        GetLocalPoint(mv3LocalContactVelocity, localVelocity, v3Zero, desiredFacingDirection);
    }

    mfTimeTilSave = time;
    if (mfTimeTilSave < 0.0f)
    {
        mfTimeTilSave = 0.0f;
    }

    if (g_pBall->m_unk_0xA6 && mpLooseBallInfo != NULL)
    {
        mpShooter = static_cast<cFielder*>(g_pBall->m_pPrevOwner);
        mfWaitTime = mfTimeTilSave - (mpLooseBallInfo->mfPickupTime * mpLooseBallInfo->mfAnimDuration);
    }
    else
    {
        mpShooter = NULL;
        cBall* pBall = g_pBall;
        pBall->m_unk_0xA6 = false;
        pBall->mpDamageTarget = NULL;
        mfWaitTime = mfTimeTilSave - mBlendInfo.mfMilestoneTime[2];

        if (mbShouldMiss)
        {
            mfWaitTime += 0.11f;
        }
    }

    if (mfWaitTime <= 0.01f)
    {
        InitActionSTS();
        return;
    }

    SetAnimState(0xA, true, 0.2f, false, false);
    GoalieTweaks* pGoalieTweaks = static_cast<GoalieTweaks*>(m_pTweaks);
    InitMovementFromAnimSeek(pGoalieTweaks->fSaveDirectionSeekSpeed, pGoalieTweaks->fSaveDirectionSeekFalloff);
}

/**
 * Offset/Address/Size: 0x29A4 | 0x800454A0 | size: 0x3A8
 * TODO: 99.47% match - register allocation diffs remain in STS trajectory setup:
 * f30/f31 swap around time/absX path and f1/f2/f4/f5 allocation in
 * fFinalYPos/fNetYLimit math.
 */
void Goalie::InitActionSTS()
{
    mbDoHeadTrack = false;
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_STS;
    mnSubstate = 0;

    cBall* pBall;
    if (mpShooter != NULL)
    {
        SetDesiredSaveFacing(g_pBall->m_v3Position);

        nlVector4 plane;
        const u16 desiredFacingDirection = m_aDesiredFacingDirection;
        const nlVector3& pPosition = m_v3Position;

        MakePerpendicularPlane(pPosition, desiredFacingDirection, plane, 0.2f);

        nlVector3 localVelocity;
        float time = FakeBallWorld::GetPredictedPlaneIntersectTime(plane, mv3TargetPosition, localVelocity);
        double absX = __fabs(mv3TargetPosition.f.x);

        if ((float)absX > cField::GetGoalLineX(1U))
        {
            time = -1.0f;
        }
        else if (time > 0.0f)
        {
            GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, pPosition, desiredFacingDirection);
            GetLocalPoint(mv3LocalContactVelocity, localVelocity, v3Zero, desiredFacingDirection);
        }

        mfTimeTilSave = time;

        SetAnimState(mpLooseBallInfo->mnAnimID, true, 0.2f, false, false);
        m_pCurrentAnimController->m_fPlaybackSpeedScale = (mpLooseBallInfo->mfPickupTime * mpLooseBallInfo->mfAnimDuration) / mfTimeTilSave;
    }
    else
    {
        if (mpSaveData->muSaveType == 0x20000)
        {
            m_pPhysicsCharacter->m_CanCollidedWithGoalLine = false;
            m_pPhysicsCharacter->m_CanCollideWithWall = false;

            PlayBlendedAnims(-mfWaitTime, -1);
            mFatigue.RegisterShot(mpSaveData->mfFatigueValue);

            pBall = g_pBall;
            float fFinalXPos = cField::GetGoalLineX(1U) + 1.5f;
            if (m_pTeam->m_pNet->m_baseLocation.f.x < 0.0f)
            {
                fFinalXPos = -fFinalXPos;
            }

            mv3NavTarget.f.x = fFinalXPos;

            float fNetYLimit = (0.5f * cNet::m_fNetWidth) - 1.3f;
            float fFinalYPos = m_v3Position.f.y
                             + (((m_v3Position.f.y - pBall->m_v3Position.f.y) * (fFinalXPos - m_v3Position.f.x))
                                 / (m_v3Position.f.x - pBall->m_v3Position.f.x));
            fFinalYPos = (fFinalYPos >= -fNetYLimit) ? fFinalYPos : -fNetYLimit;
            fFinalYPos = (fFinalYPos <= fNetYLimit) ? fFinalYPos : fNetYLimit;

            mv3NavTarget.f.y = fFinalYPos;
            mv3NavTarget.f.z = 0.0f;
            mfTargetTime = mpSaveData->mfMilestonePercent[2];
        }
        else if (mpSaveData->muSaveType & 0x50000)
        {
            PlayBlendedAnims(-mfWaitTime, -1);
            mFatigue.RegisterShot(mpSaveData->mfFatigueValue);
        }
        else
        {
            if (mbShouldMiss)
            {
                mfWaitTime = 0.0f;
            }

            if (mfWaitTime < -mBlendInfo.mfMilestoneTime[1])
            {
                mfWaitTime = -mBlendInfo.mfMilestoneTime[1];
            }

            PlayBlendedAnims(-mfWaitTime, -1);
            mFatigue.RegisterShot(((GoalieTweaks*)m_pTweaks)->fShotFatigueSTSSave);
        }
    }

    Event* pEvent = g_pEventManager->CreateValidEvent(0x13, 0x38);
    GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();

    pSaveData->pGoalie = this;
    pSaveData->v3BallVelocity = v3Zero;
    pSaveData->fWowFactor = 0.0f;
    pSaveData->isSTS = 0;

    pSaveData->saveType = g_pBall->m_uGoalType;
    pSaveData->pShooter = g_pBall->m_pShooter;

    if (mpSaveData)
    {
        pSaveData->padding = mpSaveData->muSaveType;
    }
    else
    {
        pSaveData->padding = 3;
    }
}

/**
 * Offset/Address/Size: 0x2824 | 0x80045320 | size: 0x180
 */
void Goalie::InitActionSTSRecover()
{
    if ((mpSaveData != NULL) && (mpSaveData->mnRecoverAnimID >= 0))
    {
        if (mpSaveData->muSaveType & 0x70000)
        {
            mbDoHeadTrack = false;
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_STS_RECOVER;
            mnSubstate = 0;

            SetAnimState(mpSaveData->mnRecoverAnimID, true, 0.2f, false, false);

            if ((mpSaveData->muSaveType + 0xFFFF0000) == 0)
            {

                mfWaitTime = ((GoalieTweaks*)m_pTweaks)->fGoalieStunTimeMin;

                float timeRange = ((GoalieTweaks*)m_pTweaks)->fGoalieStunTimeMax - mfWaitTime;
                if (timeRange > 0.0f)
                {
                    static FilteredRandomReal randgenTimeRange;
                    float randomValue = timeRange * randgenTimeRange.genrand();
                    mfWaitTime += randomValue;
                }

                InitMovementFromAnim(0, v3Zero, 1.0f, false);
                g_pBall->m_uGoalType = 4;
            }
            return;
        }
        else
        {
            if ((mpSaveData->muSaveType & 0xFFFC) && (mnOffplayPending == GOALIE_OFFPLAY_NONE))
            {
                g_pBall->m_uGoalType = 4;
            }
        }
    }
    InitActionDiveRecover();
}

/**
 * Offset/Address/Size: 0x2600 | 0x800450FC | size: 0x224
 * TODO: 97.01% match - register allocation diffs in delta block (f3/f4 swap for
 * y/x/z deltas) and scale block (cascading register differences). All remaining
 * diffs are register-only (r markers), functionally equivalent.
 */
void Goalie::InitActionChipShotStumble()
{
    CleanGoalieAction();
    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_MISS_CHIP_SHOT;
    mnSubstate = 0;

    float dx = m_v3Position.f.x - g_pBall->m_v3Position.f.x;
    float dy = m_v3Position.f.y - g_pBall->m_v3Position.f.y;
    bool bFar = (dx * dx + dy * dy) > 100.0f;
    bool bContactLow;
    if (mv3LocalContactPosition.f.y > 0.0f)
        bContactLow = false;
    else
        bContactLow = true;
    mpSaveData = GoalieSave::GetMissChipSaveData(bContactLow, bFar);

    mpLooseBallInfo = NULL;
    SetAnimState(mpSaveData->mnAnimID, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 0.5f, false);
    m_pPhysicsCharacter->m_CanCollidedWithGoalLine = false;

    cBall* pBall = g_pBall;
    float y = pBall->m_v3ShotTarget.f.y - pBall->m_v3Position.f.y;
    float x = pBall->m_v3ShotTarget.f.x - pBall->m_v3Position.f.x;
    float yy = y * y;
    float z = pBall->m_v3ShotTarget.f.z - pBall->m_v3Position.f.z;
    volatile float dirZ, dirY, dirX;
    dirY = y;
    float xx = x * x;
    dirX = x;
    dirZ = z;
    float dist = nlSqrt(xx + yy, true);

    if (dist > 0.5f)
    {
        float sx = dirX;
        float distPlusOne = 1.0f + dist;
        float sy = dirY;
        float sz = dirZ;
        float scale = distPlusOne / dist;
        nlVec3Set(mv3NavTarget,
            pBall->m_v3ShotTarget.f.x + scale * sx,
            pBall->m_v3ShotTarget.f.y + scale * sy,
            pBall->m_v3ShotTarget.f.z + scale * sz);
    }
    else
    {
        mv3NavTarget = pBall->m_v3ShotTarget;
        float pushX;
        if (mv3NavTarget.f.x > 0.0f)
            pushX = 1.0f;
        else
            pushX = -1.0f;
        mv3NavTarget.f.x += pushX;
    }

    float clampedY = mv3NavTarget.f.y;
    float maxY = 0.5f * cNet::m_fNetWidth - 0.5f;
    clampedY = (clampedY >= -maxY) ? clampedY : -maxY;
    clampedY = (clampedY <= maxY) ? clampedY : maxY;
    mv3NavTarget.f.y = clampedY;

    mv3NavTarget.f.z = 0.0f;
    mbDoHeadTrack = false;
}

/**
 * Offset/Address/Size: 0x2324 | 0x80044E20 | size: 0x2DC
 */
void Goalie::InitActionDiveRecover()
{
    if (mpSaveData != NULL && mpSaveData->mnRecoverAnimID >= 0)
    {
        mbDoHeadTrack = false;

        if (mnOffplayPending != GOALIE_OFFPLAY_NONE)
        {
            int randomValue = nlRandom(2, &nlDefaultSeed);
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_OFFPLAY;
            mnSubstate = 0;

            int animID;
            if (m_pAnimInventory->GetMirrored(m_eAnimID))
            {
                animID = (randomValue == 0) ? 0x89 : 0x8B;
            }
            else
            {
                animID = (randomValue == 0) ? 0x88 : 0x8A;
            }

            SetAnimState(animID, true, 0.2f, false, false);
            mnOffplayPending = GOALIE_OFFPLAY_NONE;
        }
        else
        {
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_DIVE_RECOVER;
            mnSubstate = 0;

            SetAnimState(mpSaveData->mnRecoverAnimID, true, 0.2f, false, false);
        }

        InitMovementFromAnim(0, v3Zero, 1.0f, false);
    }
    else
    {
        if (m_pBall == NULL)
        {
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_MOVE;
            mnSubstate = 0;

            SetAnimState(8, true, 0.2f, false, false);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);

            mnSubstate = 1;
            mMoveDirection = GOALIEDIR_IDLE;

            m_pPhysicsCharacter->m_CanCollideWithBall = 1;
            mbShouldMiss = false;
            mbDoNavigate = false;
            m_pPhysicsCharacter->m_CanCollidedWithGoalLine = 1;
            m_pPhysicsCharacter->m_CanCollideWithWall = 1;

            if (mbStunEffectActive)
            {
                KillDaze(this);
                mbStunEffectActive = false;
            }

            mpShooter = NULL;
            mUrgency = URGENCY_LOW;
            mfSpeedScale = 1.0f;
            mbPosGoalieNetCheck = false;
            mbNegGoalieNetCheck = false;
            mbDoHeadTrack = true;
            mbBallImpacted = false;
            mbNoUserControl = false;
            mbPickedUp = false;

            ActionMove(0.0f); // TODO: ActionMove needs to be declared in Goalie.h
        }
        else
        {
            if (m_pBall == NULL)
            {
                PickupBall(g_pBall);
            }
            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_MOVE_WB;
            mnSubstate = 0;

            SetAnimState(9, true, 0.2f, false, false);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);

            mfWaitTime = ((GoalieTweaks*)m_pTweaks)->fGoalieBallTime;
            mfTargetTime = 0.0f;
            mpPassTarget = NULL;
        }
    }

    mbPickedUp = false;
}

/**
 * Offset/Address/Size: 0x224C | 0x80044D48 | size: 0xD8
 */
void Goalie::InitActionSTSAttack()
{
    mbDoHeadTrack = false;
    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_STS_ATTACK;
    mnSubstate = 0;

    SetAnimState(mpLooseBallInfo->mnAnimID, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    cFielder* pFielder = g_pBall->GetOwnerFielder();
    float deltaX = pFielder->m_v3Position.f.x - m_v3Position.f.x;
    float deltaY = pFielder->m_v3Position.f.y - m_v3Position.f.y;
    float distance = nlSqrt((deltaX * deltaX) + (deltaY * deltaY), true);
    mfTargetDist = distance - mpLooseBallInfo->mfPickupDistance;
    mpShooter = pFielder;
}

/**
 * Offset/Address/Size: 0x214C | 0x80044C48 | size: 0x100
 */
bool Goalie::IsTeammateHoardingBall()
{
    cFielder* pOwner = g_pBall->GetOwnerFielder();
    if (pOwner != NULL && IsOnSameTeam(pOwner))
    {
        f32 myX = m_v3Position.f.x;
        f32 ownerX = pOwner->m_v3Position.f.x;
        volatile cFielder* pOwnerVolatile = pOwner;
        cBall* pBall = g_pBall;
        if (myX * ownerX > 0.0f)
        {
            f32 threshold = (f32)fabs(myX) - 2.7f;

            if ((f32)fabs(pOwnerVolatile->m_v3Position.f.x) > threshold || (f32)fabs(pBall->m_v3Position.f.x) > threshold)
            {
                f32 distThresh = 100.0f;

                if (nlGetLengthSquared2D(m_v3Position.f.x - pOwner->m_v3Position.f.x, m_v3Position.f.y - pOwner->m_v3Position.f.y) < distThresh
                    || m_v3Position.CalculateDistanceSquared2D(pBall->m_v3Position) < distThresh)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0x2084 | 0x80044B80 | size: 0xC8
 */
void Goalie::InitActionGrabBall()
{
    cFielder* pOwnerFielder = g_pBall->GetOwnerFielder();
    if (pOwnerFielder == NULL)
    {
        return;
    }

    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_GRAB_BALL;
    mnSubstate = 0;

    GetLocalPoint(mv3LocalContactPosition, g_pBall->m_v3Position, m_v3Position, m_aActualFacingDirection);

    mpLooseBallInfo = LooseBallAnims::FindLooseBallAnim(mv3LocalContactPosition, true);

    SetAnimState(mpLooseBallInfo->mnAnimID, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    mbDoHeadTrack = false;
    mbPickedUp = false;
}

/**
 * Offset/Address/Size: 0x1FA0 | 0x80044A9C | size: 0xE4
 */
unsigned short Goalie::CalcBestSave(float fTime, const nlVector3& rTargetPos, const nlVector3& rContactPos, unsigned int uSaveType, bool bParam)
{
    SetDesiredSaveFacing(rTargetPos);

    muSaveType = uSaveType;
    mpLooseBallInfo = NULL;

    GetLocalPoint(mv3LocalContactPosition, rContactPos, m_v3Position, m_aDesiredFacingDirection);

    if (uSaveType == 0 || fTime <= 0.0f)
    {
        mpSaveData = NULL;
        return m_aDesiredFacingDirection;
    }

    mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, fTime, bParam, uSaveType, false);

    if (mpSaveData != NULL)
    {
        mbPlayMiss = false;
    }

    return m_aDesiredFacingDirection;
}

/**
 * Offset/Address/Size: 0x1E68 | 0x80044964 | size: 0x138
 */
float Goalie::CalcSaveParameters(float fTimeToContact, unsigned int uSaveType, bool bFromTakeoff, bool bFindFailSave)
{
    if (mbShouldMiss)
    {
        fTimeToContact += ((GoalieTweaks*)m_pTweaks)->fSaveMissDelay;
        mpSaveData = NULL;
    }
    else
    {
        mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, fTimeToContact, false, uSaveType, bFromTakeoff);
    }

    if (mpSaveData != NULL)
    {
        mbPlayMiss = false;
    }
    else
    {
        if (!mbShouldMiss && !bFindFailSave)
        {
            return -1.0f;
        }

        mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, 0.0f, true, uSaveType & 0xFFFC, false);
        mbPlayMiss = true;
    }

    const float blendFactor = (mpSaveData->mv3SavePos.f.x - mv3LocalContactPosition.f.x) / mv3LocalContactVelocity.f.x;

    fTimeToContact += blendFactor;

    nlVec3Set(mv3LocalContactPosition,
        (blendFactor * mv3LocalContactVelocity.f.x) + mv3LocalContactPosition.f.x,
        (blendFactor * mv3LocalContactVelocity.f.y) + mv3LocalContactPosition.f.y,
        (blendFactor * mv3LocalContactVelocity.f.z) + mv3LocalContactPosition.f.z);

    return fTimeToContact;
}

/**
 * Offset/Address/Size: 0x1D74 | 0x80044870 | size: 0xF4
 */
float Goalie::CalcTimeToPlane()
{
    Goalie* self = this;
    nlVector3 localVelocity;
    nlVector4 plane;
    float time;
    unsigned short desiredFacing;

    self->SetDesiredSaveFacing(g_pBall->m_v3Position);

    desiredFacing = self->m_aDesiredFacingDirection;
    nlVector3& pos = self->m_v3Position;
    MakePerpendicularPlane(pos, desiredFacing, plane, 0.2f);

    time = FakeBallWorld::GetPredictedPlaneIntersectTime(plane, self->mv3TargetPosition, localVelocity);

    if ((float)fabsf(self->mv3TargetPosition.f.x) > cField::GetGoalLineX(1U))
    {
        return -1.0f;
    }

    if (time > 0.0f)
    {
        GetLocalPoint(self->mv3LocalContactPosition, self->mv3TargetPosition, pos, desiredFacing);
        GetLocalPoint(self->mv3LocalContactVelocity, localVelocity, v3Zero, desiredFacing);
    }

    return time;
}

/**
 * Offset/Address/Size: 0x18C8 | 0x800443C4 | size: 0x4AC
 */
/**
 * TODO: 96.51% match - r4/r5 pBall register swap, missing addi r3 hoisting,
 * cascading float register diffs due to -inline deferred vs -inline auto
 */
bool Goalie::CanInterceptPass()
{
    GoalieTweaks* pTweaks = (GoalieTweaks*)m_pTweaks;
    f32 fInterceptRangeSq = pTweaks->fInterceptSaveTolerance * pTweaks->fInterceptSaveTolerance;

    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
    if (pSkillTweaks->fGoalieCanInterceptPass < 0.5f)
    {
        return false;
    }

    do
    {
        if (mpPassTarget == NULL)
        {
            break;
        }
        if (IsOnSameTeam(mpPassTarget))
        {
            break;
        }

        cBall* pBall = g_pBall;
        if (pBall->m_bBallDeflectCount != muBallDeflectCount)
        {
            break;
        }

        mpSaveData = NULL;

        f32 dyBallToTarget = pBall->m_v3Position.f.y - mpPassTarget->m_v3Position.f.y;
        f32 dyBallToGoalie = pBall->m_v3Position.f.y - m_v3Position.f.y;
        f32 dxBallToTarget = pBall->m_v3Position.f.x - mpPassTarget->m_v3Position.f.x;
        f32 dxBallToGoalie = pBall->m_v3Position.f.x - m_v3Position.f.x;

        f32 distTargetSq = dxBallToTarget * dxBallToTarget + dyBallToTarget * dyBallToTarget;
        f32 distGoalieSq = dxBallToGoalie * dxBallToGoalie + dyBallToGoalie * dyBallToGoalie;

        u16 saveAngle;
        u32 uSaveType;

        if (distGoalieSq < distTargetSq)
        {
            nlVector3 v3PassNorm;
            nlVec3Scale(v3PassNorm, g_pBall->m_v3Velocity, -1.0f);
            v3PassNorm.f.z = 0.0f;

            nlVector4 v4Plane;
            MakePerpendicularPlane(m_v3Position, v3PassNorm, v4Plane, 0.2f);

            nlVector3 contactVel;
            mfTimeTilSave = FakeBallWorld::GetPredictedPlaneIntersectTime(v4Plane, mv3TargetPosition, contactVel);

            saveAngle = (u16)(s32)(10430.378f * nlATan2f(v3PassNorm.f.y, v3PassNorm.f.x));

            uSaveType = 0xFFFF;

            f32 fAbsTargetX = (f32)fabs(mv3TargetPosition.f.x);
            f32 fTargetX = mv3TargetPosition.f.x;
            bool bInPenaltyBox;
            if (fAbsTargetX > cField::GetPenaltyBoxX(1) - (-2.0f)
                && fTargetX * m_v3Position.f.x > 0.0f
                && (f32)fabs(mv3TargetPosition.f.y) < (-2.0f) + cField::GetPenaltyBoxY())
            {
                bInPenaltyBox = true;
            }
            else
            {
                bInPenaltyBox = false;
            }

            if (!bInPenaltyBox)
            {
                uSaveType = 0xFFFC;
            }

            GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, m_v3Position, saveAngle);

            mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, mfTimeTilSave, false, uSaveType, false);

            if (mpSaveData != NULL)
            {
                f32 dy = mBlendInfo.mv3BlendedSavePos.f.y - mv3LocalContactPosition.f.y;
                f32 dx = mBlendInfo.mv3BlendedSavePos.f.x - mv3LocalContactPosition.f.x;
                f32 dz = mBlendInfo.mv3BlendedSavePos.f.z - mv3LocalContactPosition.f.z;
                if (dx * dx + dy * dy + dz * dz > fInterceptRangeSq)
                {
                    mpSaveData = NULL;
                }
            }
        }

        if (mpSaveData == NULL)
        {
            f32 fLeadTime = 2.0f * FixedUpdateTask::GetPhysicsUpdateTick();
            mfTimeTilSave = g_pBall->m_tPassTargetTimer.GetSeconds() - fLeadTime;

            if (mfTimeTilSave > 0.05f)
            {
                nlVector3 v3BallVel;
                FakeBallWorld::GetPredictedBallPosition(mfTimeTilSave, mv3TargetPosition, v3BallVel);

                f32 dx = mv3TargetPosition.f.x - m_v3Position.f.x;
                f32 dy = m_v3Position.f.y - mv3TargetPosition.f.y;
                f32 dot = dx * g_pBall->m_v3Velocity.f.y + dy * g_pBall->m_v3Velocity.f.x + 0.0f * g_pBall->m_v3Velocity.f.z;

                if (dot > 0.0f)
                {
                    dy = -1.0f * dy;
                    dx = -1.0f * dx;
                }

                saveAngle = (u16)(s32)(10430.378f * nlATan2f(dx, dy));

                uSaveType = 0xFFFF;

                f32 fAbsTargetX = (f32)fabs(mv3TargetPosition.f.x);
                f32 fTargetX = mv3TargetPosition.f.x;
                bool bInPenaltyBox;
                if (fAbsTargetX > cField::GetPenaltyBoxX(1) - (-2.0f)
                    && fTargetX * m_v3Position.f.x > 0.0f
                    && (f32)fabs(mv3TargetPosition.f.y) < (-2.0f) + cField::GetPenaltyBoxY())
                {
                    bInPenaltyBox = true;
                }
                else
                {
                    bInPenaltyBox = false;
                }

                if (!bInPenaltyBox)
                {
                    uSaveType = 0xFFFC;
                }

                GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, m_v3Position, saveAngle);

                mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, mfTimeTilSave, false, uSaveType, false);

                if (mpSaveData != NULL)
                {
                    f32 dy2 = mBlendInfo.mv3BlendedSavePos.f.y - mv3LocalContactPosition.f.y;
                    f32 dx2 = mBlendInfo.mv3BlendedSavePos.f.x - mv3LocalContactPosition.f.x;
                    f32 dz2 = mBlendInfo.mv3BlendedSavePos.f.z - mv3LocalContactPosition.f.z;
                    if (dx2 * dx2 + dy2 * dy2 + dz2 * dz2 > fInterceptRangeSq)
                    {
                        mpSaveData = NULL;
                    }
                }
            }
        }

        if (mpSaveData != NULL)
        {
            if (mBlendInfo.mfMilestoneTime[2] <= mfTimeTilSave)
            {
                mBlendInfo.mfStartTime = 0.0f;
                mfWaitTime = mfTimeTilSave - mBlendInfo.mfMilestoneTime[2];
            }
            else
            {
                f32 diff = mBlendInfo.mfMilestoneTime[2] - mfTimeTilSave;
                if (diff <= mBlendInfo.mfMilestoneTime[1])
                {
                    mBlendInfo.mfStartTime = diff;
                }
                else
                {
                    mBlendInfo.mfStartTime = mBlendInfo.mfMilestoneTime[1];
                }
                mfWaitTime = 0.0f;
            }
            m_aDesiredFacingDirection = saveAngle;
            return true;
        }
    } while (0);

    return false;
}

/**
 * Offset/Address/Size: 0x1810 | 0x8004430C | size: 0xB8
 */
int Goalie::ChooseRunAnim(short nAngle, const nlVector3& rTargetPos, float fThreshold)
{
    float fDeltaX = rTargetPos.f.x - m_v3Position.f.x;
    float fDeltaY = rTargetPos.f.y - m_v3Position.f.y;
    unsigned short nAbsAngle;
    int nCurrentAnimID = m_eAnimID;

    if (((fDeltaX * fDeltaX) + (fDeltaY * fDeltaY)) < (fThreshold * fThreshold))
    {
        mMoveDirection = GOALIEDIR_IDLE;
        return 8;
    }

    nAbsAngle = (nAngle < 0 ? -nAngle : nAngle);

    mMoveDirection = GOALIEDIR_FORWARD;

    int id;

    if (((nCurrentAnimID == 0x24) || (nCurrentAnimID == 0x25)) && (m_pCurrentAnimController->m_fTime < 0.92f))
    {
        id = nCurrentAnimID;
    }
    else if (nAbsAngle <= 0x2AF8 || nCurrentAnimID == 0x26)
    {
        id = 0x26;
    }
    else if (nAngle > 0)
    {
        id = 0x25;
    }
    else
    {
        id = 0x24;
    }

    return id;
}

/**
 * Offset/Address/Size: 0x17D4 | 0x800442D0 | size: 0x3C
 */
void Goalie::ChooseSwatAnim(int nParam)
{
    mpLooseBallInfo = LooseBallAnims::GetSwatSTSInfo(nParam);
    mpSaveData = NULL;
}

/**
 * Offset/Address/Size: 0xF30 | 0x80043A2C | size: 0x8A4
 */
void Goalie::DoPassRelease()
{
}

/**
 * Offset/Address/Size: 0xAD0 | 0x800435CC | size: 0x460
 * TODO: 99.46% match - case 3 keeps owner pointer in r31 instead of r30 and uses
 * local float labels (@1320/@1321/@1322) instead of target SDA labels
 * (@3669/@3289/@3290).
 */
void Goalie::EventHandler(Event* event, void*)
{
    extern cCharacter* g_pCharacters[10];

    switch (event->m_uEventID)
    {
    case 3:
    {
        register cPlayer* pPlayer = g_pBall->m_pOwner;
        g_pBall->m_tNoPickupTimer.SetSeconds(3.0f);

        if (pPlayer != NULL)
        {
            pPlayer->ReleaseBall();

            if (pPlayer->m_eClassType == GOALIE)
            {
                Goalie* pGoalie = (Goalie*)pPlayer;

                pGoalie->CleanGoalieAction();

                pGoalie->mPrevGoalieActionState = pGoalie->mGoalieActionState;
                pGoalie->mGoalieActionState = GOALIEACTION_MOVE;
                pGoalie->mnSubstate = 0;

                pGoalie->SetAnimState(8, true, 0.2f, false, false);
                pGoalie->InitMovementFromAnim(0, v3Zero, 1.0f, false);

                pGoalie->mnSubstate = 1;
                pGoalie->mMoveDirection = GOALIEDIR_IDLE;

                pGoalie->m_pPhysicsCharacter->m_CanCollideWithBall = true;
                pGoalie->mbShouldMiss = false;
                pGoalie->mbDoNavigate = false;
                pGoalie->m_pPhysicsCharacter->m_CanCollidedWithGoalLine = true;
                pGoalie->m_pPhysicsCharacter->m_CanCollideWithWall = true;

                if (pGoalie->mbStunEffectActive)
                {
                    KillDaze(pGoalie);
                    pGoalie->mbStunEffectActive = false;
                }

                pGoalie->mpShooter = NULL;
                pGoalie->mUrgency = URGENCY_LOW;
                pGoalie->mfSpeedScale = 1.0f;
                mbPosGoalieNetCheck = false;
                mbNegGoalieNetCheck = false;
                pGoalie->mbDoHeadTrack = true;
                pGoalie->mbBallImpacted = false;
                pGoalie->mbNoUserControl = false;
                pGoalie->mbPickedUp = false;
            }
            else if (pPlayer->m_eClassType == FIELDER)
            {
                cFielder* pFielder = (cFielder*)pPlayer;
                pFielder->EndDesire(false);
                pFielder->EndAction();
            }
        }

        Goalie* pHomeGoalie = (Goalie*)g_pCharacters[8];

        pHomeGoalie->CleanGoalieAction();

        pHomeGoalie->mPrevGoalieActionState = pHomeGoalie->mGoalieActionState;
        pHomeGoalie->mGoalieActionState = GOALIEACTION_MOVE;
        pHomeGoalie->mnSubstate = 0;

        pHomeGoalie->SetAnimState(8, true, 0.2f, false, false);
        pHomeGoalie->InitMovementFromAnim(0, v3Zero, 1.0f, false);

        pHomeGoalie->mnSubstate = 1;
        pHomeGoalie->mMoveDirection = GOALIEDIR_IDLE;

        pHomeGoalie->m_pPhysicsCharacter->m_CanCollideWithBall = true;
        pHomeGoalie->mbShouldMiss = false;
        pHomeGoalie->mbDoNavigate = false;
        pHomeGoalie->m_pPhysicsCharacter->m_CanCollidedWithGoalLine = true;
        pHomeGoalie->m_pPhysicsCharacter->m_CanCollideWithWall = true;

        if (pHomeGoalie->mbStunEffectActive)
        {
            KillDaze(pHomeGoalie);
            pHomeGoalie->mbStunEffectActive = false;
        }

        pHomeGoalie->mpShooter = NULL;
        pHomeGoalie->mUrgency = URGENCY_LOW;
        pHomeGoalie->mfSpeedScale = 1.0f;
        mbPosGoalieNetCheck = false;
        mbNegGoalieNetCheck = false;
        pHomeGoalie->mbDoHeadTrack = true;
        pHomeGoalie->mbBallImpacted = false;
        pHomeGoalie->mbNoUserControl = false;
        pHomeGoalie->mbPickedUp = false;

        Goalie* pAwayGoalie = (Goalie*)g_pCharacters[9];

        pAwayGoalie->CleanGoalieAction();

        pAwayGoalie->mPrevGoalieActionState = pAwayGoalie->mGoalieActionState;
        pAwayGoalie->mGoalieActionState = GOALIEACTION_MOVE;
        pAwayGoalie->mnSubstate = 0;

        pAwayGoalie->SetAnimState(8, true, 0.2f, false, false);
        pAwayGoalie->InitMovementFromAnim(0, v3Zero, 1.0f, false);

        pAwayGoalie->mnSubstate = 1;
        pAwayGoalie->mMoveDirection = GOALIEDIR_IDLE;

        pAwayGoalie->m_pPhysicsCharacter->m_CanCollideWithBall = true;
        pAwayGoalie->mbShouldMiss = false;
        pAwayGoalie->mbDoNavigate = false;
        pAwayGoalie->m_pPhysicsCharacter->m_CanCollidedWithGoalLine = true;
        pAwayGoalie->m_pPhysicsCharacter->m_CanCollideWithWall = true;

        if (pAwayGoalie->mbStunEffectActive)
        {
            KillDaze(pAwayGoalie);
            pAwayGoalie->mbStunEffectActive = false;
        }

        pAwayGoalie->mpShooter = NULL;
        pAwayGoalie->mUrgency = URGENCY_LOW;
        pAwayGoalie->mfSpeedScale = 1.0f;
        mbPosGoalieNetCheck = false;
        mbNegGoalieNetCheck = false;
        pAwayGoalie->mbDoHeadTrack = true;
        pAwayGoalie->mbBallImpacted = false;
        pAwayGoalie->mbNoUserControl = false;
        pAwayGoalie->mbPickedUp = false;

        break;
    }

    case 5:
    {
        GoalScoredData* data;

        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();

            if (id != 0x18A)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (GoalScoredData*)&event->m_data;
            }
        }

        Goalie* pGoalie = g_pTeams[data->uTeamIndex]->GetGoalie();
        Goalie* pOtherGoalie = g_pTeams[1 - data->uTeamIndex]->GetGoalie();

        if (g_pGame->m_eGameState != GS_KICKOFF)
        {
            pGoalie->mnOffplayPending = GOALIE_OFFPLAY_GOAL_FOR;
            pGoalie->mbPickedUp = false;
            pOtherGoalie->mnOffplayPending = GOALIE_OFFPLAY_GOAL_AGAINST;
            pOtherGoalie->mbPickedUp = false;
        }

        pGoalie->mFatigue.Reset();
        pOtherGoalie->mFatigue.Reset();
        break;
    }

    case 10:
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0xA84 | 0x80043580 | size: 0x4C
 */
void Goalie::FindSTSStunData()
{
    mpSaveData = GoalieSave::GetRandomSTSSaveData();
    GoalieSave::GetClosestBlendedPos(mBlendInfo, v3Zero, mpSaveData);
    mpLooseBallInfo = NULL;
}

/**
 * Offset/Address/Size: 0x994 | 0x80043490 | size: 0xF0
 */
bool Goalie::FindSTSMissData(const nlVector3& rPos)
{
    nlVector3 localPos = rPos;

    float goalLineX = cField::GetGoalLineX(1U);
    float absY = (float)fabsf(localPos.f.y);
    float sidelineY = cField::GetSidelineY(1U);

    float scale = 6.0f;
    float threshold = goalLineX - ((absY * scale) / sidelineY);

    bool bIsOutsideRange;
    if (localPos.f.x > threshold)
    {
        bIsOutsideRange = true;
    }
    else if (localPos.f.x < -threshold)
    {
        bIsOutsideRange = true;
    }
    else
    {
        bIsOutsideRange = false;
    }

    mpSaveData = GoalieSave::GetRandomSTSMissData(bIsOutsideRange);
    GoalieSave::GetClosestBlendedPos(mBlendInfo, v3Zero, mpSaveData);

    mpLooseBallInfo = NULL;

    return bIsOutsideRange;
}

/**
 * Offset/Address/Size: 0x944 | 0x80043440 | size: 0x50
 */
void Goalie::FindSTSSpinData(bool bParam)
{
    mpSaveData = GoalieSave::GetSTSSpinMissData(bParam);

    GoalieSave::GetClosestBlendedPos(mBlendInfo, v3Zero, mpSaveData);

    mpLooseBallInfo = NULL;
}

/**
 * Offset/Address/Size: 0x93C | 0x80043438 | size: 0x8
 */
PhysicsGoalie* Goalie::GetPhysicsGoalie()
{
    return static_cast<PhysicsGoalie*>(m_pPhysicsCharacter);
}

/**
 * Offset/Address/Size: 0x5B8 | 0x800430B4 | size: 0x384
 * TODO: 92.18% match - MWCC loads struct members by address (X before Y) regardless of source
 * declaration order, causing register swap (f9/f7) in offset section. Target has 2 extra frsp
 * per branch (for values read from v3TargetFacing struct members) that current compiler elides
 * because fsubs results are already single-precision. Cascading register diffs in normalize.
 */
void Goalie::SetDesiredSaveFacing(const nlVector3& v3BallPosition)
{
    if (m_v3Position.f.x > (cField::GetGoalLineX(1U) - 0.1f))
    {
        m_aDesiredFacingDirection = 0x8000;
        return;
    }

    if (m_v3Position.f.x < (0.1f - cField::GetGoalLineX(1U)))
    {
        m_aDesiredFacingDirection = 0;
        return;
    }

    nlVector3 v3TargetFacing;
    nlVector3 v3BallOffset;
    nlVector3 v3LeftPost;
    nlVector3 v3RightPost;

    v3BallOffset.f.x = v3BallPosition.f.x - m_v3Position.f.x;
    v3BallOffset.f.y = v3BallPosition.f.y - m_v3Position.f.y;
    v3BallOffset.f.z = v3BallPosition.f.z - m_v3Position.f.z;

    m_pTeam->m_pNet->GetPostLocation(v3LeftPost, 0, 0.5f);
    m_pTeam->m_pNet->GetPostLocation(v3RightPost, 1, 0.5f);

    float fLeftY = v3LeftPost.f.y - m_v3Position.f.y;
    float fRightY = v3RightPost.f.y - m_v3Position.f.y;
    float fLeftX = v3LeftPost.f.x - m_v3Position.f.x;
    float fLeftZ = v3LeftPost.f.z - m_v3Position.f.z;
    float fRightX = v3RightPost.f.x - m_v3Position.f.x;
    float fRightZ = v3RightPost.f.z - m_v3Position.f.z;

    float fLeftDot = v3BallOffset.f.y * fLeftY + v3BallOffset.f.x * fLeftX + v3BallOffset.f.z * fLeftZ;
    float fRightDot = v3BallOffset.f.y * fRightY + v3BallOffset.f.x * fRightX + v3BallOffset.f.z * fRightZ;

    v3LeftPost.f.x = fLeftX;
    v3LeftPost.f.y = fLeftY;
    v3LeftPost.f.z = fLeftZ;
    v3RightPost.f.x = fRightX;
    v3RightPost.f.y = fRightY;
    v3RightPost.f.z = fRightZ;

    if ((fLeftDot > -1.0f) || (fRightDot > -1.0f))
    {
        if (fLeftDot > fRightDot)
        {
            v3TargetFacing.f.x = fLeftY;
            v3TargetFacing.f.y = -fLeftX;
            v3TargetFacing.f.z = -1.0f;

            if (((v3TargetFacing.f.y * fRightY) + (v3TargetFacing.f.x * fRightX) + (v3TargetFacing.f.z * fRightZ)) > -1.0f)
            {
                v3TargetFacing.f.x *= 0.5f;
                v3TargetFacing.f.y *= 0.5f;
                v3TargetFacing.f.z *= 0.5f;
            }
        }
        else
        {
            v3TargetFacing.f.x = fRightY;
            v3TargetFacing.f.y = -fRightX;
            v3TargetFacing.f.z = -1.0f;

            if (((v3TargetFacing.f.y * fLeftY) + (v3TargetFacing.f.x * fLeftX) + (v3TargetFacing.f.z * fLeftZ)) > -1.0f)
            {
                v3TargetFacing.f.x *= 0.5f;
                v3TargetFacing.f.y *= 0.5f;
                v3TargetFacing.f.z *= 0.5f;
            }
        }
    }
    else
    {
        v3TargetFacing = v3BallOffset;
    }

    float fBallOffMagSq = v3BallOffset.f.x * v3BallOffset.f.x + v3BallOffset.f.y * v3BallOffset.f.y + v3BallOffset.f.z * v3BallOffset.f.z;

    if (fBallOffMagSq < 0.0001f)
    {
        float fSqX = v3TargetFacing.f.x * v3TargetFacing.f.x;
        float fTfY = v3TargetFacing.f.y;
        float fTfZ = v3TargetFacing.f.z;
        float fSqY = fTfY * fTfY;
        float fSqZ = fTfZ * fTfZ;

        float fBtgZ = v3BallPosition.f.z - m_pTeam->m_pNet->m_baseLocation.f.z;
        float fBtgY = v3BallPosition.f.y - m_pTeam->m_pNet->m_baseLocation.f.y;
        float fBtgX = v3BallPosition.f.x - m_pTeam->m_pNet->m_baseLocation.f.x;

        float fRecip = nlRecipSqrt(fSqX + fSqY + fSqZ, true);

        v3TargetFacing.f.y = fRecip * fTfY;
        v3TargetFacing.f.x = fRecip * v3TargetFacing.f.x;
        v3TargetFacing.f.z = fRecip * fTfZ;

        float fRecip2 = nlRecipSqrt(fBtgY * fBtgY + fBtgX * fBtgX + fBtgZ * fBtgZ, true);

        v3TargetFacing.f.x = 0.5f * (fRecip2 * fBtgX) + 0.5f * v3TargetFacing.f.x;
        v3TargetFacing.f.y = 0.5f * (fRecip2 * fBtgY) + 0.5f * v3TargetFacing.f.y;
        v3TargetFacing.f.z = 0.5f * (fRecip2 * fBtgZ) + 0.5f * v3TargetFacing.f.z;
    }

    m_aDesiredFacingDirection = (s16)(nlATan2f(v3TargetFacing.f.y, v3TargetFacing.f.x) * (32768.0f / 3.14159265f));
}

/**
 * Offset/Address/Size: 0x3C4 | 0x80042EC0 | size: 0x1F4
 * TODO: 96.26% match - volatile FP register allocation differs: 10430.378f->f0 instead of f3,
 * cascading through angle section (r4/r5 swap) and velocity clamp section (fVelX/fVelY in
 * f0/f1 instead of f4/f5, missing 2 branch instructions in ternary codegen).
 */
void Goalie::TrackTarget(const nlVector3& v3Target, float fRatio)
{
    nlVector3 v3FutureBallPos;
    nlVector3 v3FuturePos;
    unsigned short aRot;

    GetCurrentAnimFuture(m_nBallJointIndex, mpLooseBallInfo->mfPickupTime, v3FutureBallPos, v3FuturePos, aRot);

    float fDeltaY = v3Target.f.y - v3FutureBallPos.f.y;
    float fDeltaX = v3Target.f.x - v3FutureBallPos.f.x;

    float fAngleToTarget = nlATan2f(v3Target.f.y - m_v3Position.f.y, v3Target.f.x - m_v3Position.f.x);

    u16 aFutureAngle = (u16)(s32)(10430.378f * nlATan2f(v3FutureBallPos.f.y - m_v3Position.f.y, v3FutureBallPos.f.x - m_v3Position.f.x));
    u16 aTargetAngle = (u16)(s32)(10430.378f * fAngleToTarget);
    s32 iRatio = (s32)(1024.0f * fRatio);
    s16 aDiff = (s16)(aTargetAngle - aFutureAngle);
    iRatio = (iRatio * aDiff) / 1024;
    SetFacingDirection((u16)(m_aActualFacingDirection + iRatio));

    float fVelX = fRatio * fDeltaX;
    float fVelY = fRatio * fDeltaY;
    float fVelZ = 0.0f;
    fVelZ *= fRatio;

    fVelX = fVelX >= -0.12f ? fVelX : -0.12f;
    fVelX = fVelX <= 0.5f ? fVelX : 0.5f;
    fVelY = fVelY >= -0.12f ? fVelY : -0.12f;
    fVelY = fVelY <= 0.5f ? fVelY : 0.5f;

    nlVec3Set(v3FuturePos, fVelX + m_v3Position.f.x, fVelY + m_v3Position.f.y, fVelZ + m_v3Position.f.z);

    SetPosition(v3FuturePos);
}

/**
 * Offset/Address/Size: 0x304 | 0x80042E00 | size: 0xC0
 */
void Goalie::TacklePlayer(cPlayer* pPlayer)
{
    cFielder* pFielder = static_cast<cFielder*>(pPlayer);
    if ((pPlayer != NULL) && (pPlayer->m_eClassType == FIELDER) && (pFielder->IsFallenDown(0.0f) == 0))
    {
        pPlayer->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);
        if (pPlayer->m_pBall != 0)
        {
            pPlayer->ReleaseBall();
        }
        if (IsOnSameTeam(pFielder) != 0)
        {
            pFielder->EndDesire(false);
            pFielder->EndAction();
            return;
        }
        pFielder->InitActionSlideAttackReact(this, false);
    }
}

/**
 * Offset/Address/Size: 0x298 | 0x80042D94 | size: 0x6C
 */
void Goalie::StealBall(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return;
    }

    if (pPlayer->m_eClassType != FIELDER)
    {
        return;
    }

    if (pPlayer->m_pBall == NULL)
    {
        return;
    }

    pPlayer->ReleaseBall();

    cFielder* pFielder = static_cast<cFielder*>(pPlayer);
    if (pFielder->m_eFielderDesireState == FIELDERDESIRE_FINISH_ACTION)
    {
        return;
    }

    pFielder->EndDesire(false);
    pFielder->EndAction();
}

/**
 * Offset/Address/Size: 0x148 | 0x80042C44 | size: 0x150
 */
void Goalie::WhackSTSPlayer(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return;
    }

    if (pFielder->m_pBall != NULL)
    {
        pFielder->ReleaseBall();
    }

    pFielder->SetFacingDirection(m_aActualFacingDirection + 0x8000);
    pFielder->InitActionSTSHitReact(this);
    PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);
    pFielder->PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk264);

    nlVector3 v3BallVel;
    float fBallVelMult = ((GoalieTweaks*)m_pTweaks)->fSTSAttackBallVelMult;
    float vx = (-fBallVelMult) * m_v3Position.f.x;
    float vz = (-fBallVelMult) * m_v3Position.f.z;
    float vy = (-fBallVelMult) * m_v3Position.f.y;
    v3BallVel.f.x = vx;
    v3BallVel.f.z = vz;
    v3BallVel.f.y = vy;

    float yRand = nlRandomf(1.0f, &nlDefaultSeed);
    if ((u32)nlRandom(100, &nlDefaultSeed) > 50)
    {
        yRand *= -1.0f;
    }
    v3BallVel.f.y += yRand;

    float zRand = nlRandomf(3.0f, &nlDefaultSeed);
    v3BallVel.f.z = 2.0f + zRand;

    g_pBall->SetVelocity(v3BallVel, SPINTYPE_FORWARD, NULL);
    g_pBall->m_tNoPickupTimer.SetSeconds(0.5f);
}

/**
 * Offset/Address/Size: 0x0 | 0x80042AFC | size: 0x148
 */
void Goalie::InitActionPostWhistle()
{
    if (m_pBall != NULL)
    {
        ReleaseBall();
    }

    mnOffplayPending = GOALIE_OFFPLAY_NONE;
    mbPickedUp = false;
    SetAnimState(8, false, 0.0f, false, false);

    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_MOVE;
    mnSubstate = 0;

    SetAnimState(8, true, 0.2f, false, false);

    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    mnSubstate = 1;
    mMoveDirection = GOALIEDIR_IDLE;

    m_pPhysicsCharacter->m_CanCollideWithBall = 1;
    mbShouldMiss = false;
    mbDoNavigate = false;

    m_pPhysicsCharacter->m_CanCollidedWithGoalLine = 1;

    m_pPhysicsCharacter->m_CanCollideWithWall = 1;

    if (mbStunEffectActive)
    {
        KillDaze(this);
        mbStunEffectActive = false;
    }

    mpShooter = NULL;
    mUrgency = URGENCY_LOW;
    mfSpeedScale = 1.0f;
    mbPosGoalieNetCheck = false;
    mbNegGoalieNetCheck = false;
    mbDoHeadTrack = true;
    mbBallImpacted = false;
    mbNoUserControl = false;
    mbPickedUp = false;
}
