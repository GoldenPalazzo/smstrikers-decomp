#define NL_SINGLETON_NO_DEFINE
#include "Game/Goalie.h"
#include "Game/AI/GoalieLooseBall.h"
#include "Game/AI/AiUtil.h"
#include "Game/Ball.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/CharacterAudio.h"
#include "Game/SAnim.h"
#include "Game/SAnim/pnBlender.h"
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
#include "Game/FE/feHelpFuncs.h"
#include "Game/Camera/CameraMan.h"
#include "Game/ParticleUpdateTask.h"
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

namespace Fuzzy
{
FuzzyVariant GetBestPassTarget(cPlayer*);
}

float OpenTo(cPlayer*, cPlayer*);

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

bool Goalie::mbPosGoalieNetCheck;
bool Goalie::mbNegGoalieNetCheck;
u8 Goalie::mbActionDataSetup;
f32 Goalie::mfGoalieStepDist = -1.0f;
f32 Goalie::mfGoalieStrafeDist = -1.0f;
f32 Goalie::mfGoalieRunDist = -1.0f;
f32 Goalie::mfGoalieUrgentDist = 0.8f;
f32 gfRepositionThreshold = 0.15f;
bool gbEnableBallGoalieSweepTest = true;

/**
 * Offset/Address/Size: 0xB780 | 0x8004E27C | size: 0x2B8
 */
Goalie::Goalie(eCharacterClass charClass, const int* nModelID, cSHierarchy* pHierarchy, cAnimInventory* pAnimInventory, const CharacterPhysicsData* pPhysicsData, GoalieTweaks* pCharTweaks, AnimRetargetList* pAnimRetargetList)
    : cPlayer(4, charClass, nModelID, pHierarchy, pAnimInventory, pPhysicsData, pCharTweaks, pAnimRetargetList, GOALIE)
    , mGoalieActionState(GOALIEACTION_MOVE)
    , mPrevGoalieActionState(GOALIEACTION_MOVE)
    , mpPassTarget(NULL)
    , mpShooter(NULL)
    , mnSubstate(0)
    , mMoveDirection(GOALIEDIR_IDLE)
    , mfSwitchTime(1.0f)
    , mpSaveData(NULL)
    , muSaveType(0xFFFF)
    , mbShouldMiss(false)
    , mbStunEffectActive(false)
    , mbDoIntercept(false)
    , mbDoNavigate(false)
    , mbDoHeadTrack(true)
    , mbBallImpacted(false)
    , mbNoUserControl(false)
    , mbIsPosed(false)
    , mbPickedUp(false)
    , mfSpeedScale(1.0f)
    , mpLooseBallInfo(NULL)
    , muBallChangeCount(0)
    , muBallDeflectCount(0)
    , mnOffplayPending(GOALIE_OFFPLAY_NONE)
{
    CleanGoalieAction();
    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_MOVE;
    mnSubstate = 0;

    SetAnimState(0x08, true, 0.2f, false, false);
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

    GoalieSave::InitData(this);
    LooseBallAnims::Init(this);

    if (!mbActionDataSetup)
    {
        nlVector3 v3JointPos;

        GetJointPositionFuture(&v3JointPos, 0x19, -1, 1.0f, true, true, false);
        mfGoalieStepDist = fabsf(0.6f * v3JointPos.f.y);

        GetJointPositionFuture(&v3JointPos, 0x14, -1, 1.0f, true, true, false);

        mbActionDataSetup = true;
        mfGoalieStrafeDist = fabsf(0.6f * v3JointPos.f.y);
        mfGoalieRunDist = fabsf(1.8f * v3JointPos.f.y);
    }

    mFatigue.Reset();
    mFatigue.mfRecoverRate = pCharTweaks->fFatigueRecoverRate;

    m_pHeadTrack->m_bTrackOOI = true;
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
 * TODO: 98.40% match - remaining r28/r29 fielder copy and f-register order differences in rolling pickup/deflect velocity.
 */
void Goalie::CollideWithBallCallback(cBall* pBall)
{
    cPlayer::CollideWithBallCallback(pBall);

    if (mGoalieActionState != GOALIEACTION_STS || mpShooter == NULL)
    {
        pBall->m_unk_0xA6 = false;
        pBall->mpDamageTarget = NULL;
    }

    cPlayer* pOwner = pBall->m_pOwner;
    cFielder* pFldr;
    if (pOwner != this)
    {
        switch (mGoalieActionState)
        {
        case GOALIEACTION_MOVE:
        case GOALIEACTION_LOOSEBALL_SETUP:
        case GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING:
            if (g_pBall->m_tNoPickupTimer.m_uPackedTime == 0
                && g_pBall->m_pOwner == NULL
                && g_pBall->m_pPassTarget == NULL
                && g_pBall->m_v3Position.f.z > 0.8f)
            {
                CleanGoalieAction();
                mPrevGoalieActionState = mGoalieActionState;
                mGoalieActionState = GOALIEACTION_SNAP_BALL;
                mnSubstate = 0;
                SetAnimState(0x7D, true, 0.2f, false, false);
                InitMovementFromAnim(0, v3Zero, 1.0f, false);
                mfWaitTime = 0.1f;
                SetNoPickUpTime(mfWaitTime);
                mbDoHeadTrack = false;
            }
            break;

        case GOALIEACTION_LOOSEBALL_DESPERATE:
        {
            CleanGoalieAction();
            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_LOOSEBALL_PICKUP;
            mnSubstate = 0;
            SetAnimState(mpLooseBallInfo->mnAnimID, true, 0.2f, false, false);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);
            mMoveDirection = GOALIEDIR_IDLE;
            mfTargetTime = 0.0f;
            mfWaitTime = -1.0f;
            mbPickedUp = false;

            if (0.3f < mpLooseBallInfo->mfPickupDistance)
            {
                float fAdjustedTime = (mpLooseBallInfo->mfPickupDistance - 0.3f) * mpLooseBallInfo->mfPickupTime / mpLooseBallInfo->mfPickupDistance;
                mfTargetTime = fAdjustedTime;
                float fTargetTime = mfTargetTime;
                cPN_SAnimController* pAnim = m_pCurrentAnimController;
                pAnim->m_fPrevTime = pAnim->m_fTime;
                pAnim->m_fTime = fTargetTime;
            }

            InitiatePickup();
            break;
        }

        case GOALIEACTION_LOOSEBALL_PICKUP:
            InitiatePickup();
            break;

        case GOALIEACTION_LOOSEBALL_CATCH:
            if (pOwner == NULL)
            {
                Audio::SoundAttributes sndAtr;
                sndAtr.Init();
                sndAtr.SetSoundType(0xC0, true);
                sndAtr.UseStationaryPosVector(m_v3Position);
                sndAtr.mf_Volume = 0.4f;
                Audio::gStadGenSFX.Play(sndAtr);

                PickupBall(pBall);
                mbPickedUp = true;
                EmitGoalieCatch(this, "goalie_catch", false);
            }
            break;

        case GOALIEACTION_LOOSEBALL_PURSUE_ROLLING:
        {
            pFldr = static_cast<cFielder*>(pOwner);
            if (pOwner != NULL)
            {
                if (IsOnSameTeam(pOwner))
                    break;

                if (!pFldr->IsFallenDown(0.0f) && !pFldr->IsInvincible() && pOwner != NULL && pOwner->m_eClassType == FIELDER)
                {
                    if (!((cFielder*)pOwner)->IsFallenDown(0.0f))
                    {
                        pOwner->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);
                        if (pOwner->m_pBall != NULL)
                        {
                            pOwner->ReleaseBall();
                        }
                        if (IsOnSameTeam(pOwner))
                        {
                            ((cFielder*)pOwner)->EndDesire(false);
                            ((cFielder*)pOwner)->EndAction();
                        }
                        else
                        {
                            ((cFielder*)pOwner)->InitActionSlideAttackReact(this, false);
                        }
                    }
                }
            }

            int animID = mpLooseBallInfo->mnAnimID;
            if (animID != m_eAnimID)
            {
                bool bShouldSetAnim = false;
                if (animID != m_eAnimID || (bShouldSetAnim = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
                {
                    SetAnimState(animID, true, 0.2f, false, false);
                }

                {
                    cPN_SAnimController* pAnim = m_pCurrentAnimController;
                    float targetTime = 0.5f * mpLooseBallInfo->mfPickupTime;
                    float currentTime = pAnim->m_fTime;
                    pAnim->m_fPrevTime = currentTime;
                    pAnim->m_fTime = targetTime;
                }
                InitMovementFromAnim(0, v3Zero, 1.0f, false);
            }

            if (m_pBall == NULL)
            {
                PickupBall(g_pBall);
                mbPickedUp = true;
                g_pBall->ClearShotInProgress();
                EmitGoalieCatch(this, "goalie_catch", false);
            }
            break;
        }

        case GOALIEACTION_MISS_CHIP_SHOT:
            if (m_eAnimID != 0x70 && m_pCurrentAnimController->m_fTime < 0.5555556f)
            {
                SetAnimState(0x70, true, 0.2f, false, false);
                InitMovementFromAnim(0, v3Zero, 1.0f, false);
                EmitGoalieCatch(this, "goalie_deflect", false);

                Audio::SoundAttributes sndAtr;
                sndAtr.Init();
                sndAtr.SetSoundType(0xB7, true);
                sndAtr.UseStationaryPosVector(m_v3Position);
                Audio::gStadGenSFX.Play(sndAtr);
            }
            break;

        case GOALIEACTION_STS:
            HandleSTSContact(pBall);
            break;

        case GOALIEACTION_SAVE:
        {
            if (mbBallImpacted)
                return;

            mbBallImpacted = true;
            u8 bIsPerfect = 0;
            bool bShotPerfect = false;

            if (g_pBall->m_tShotTimer.m_uPackedTime != 0 && g_pBall->m_unk_0xA4)
            {
                bShotPerfect = true;
            }

            if (bShotPerfect)
            {
                EmitGoalieCatch(this, "perfect_shot_catch", false);
                bIsPerfect = true;
            }

            if (!bIsPerfect && mpSaveData != NULL && (mpSaveData->muSaveType & 3) == 0)
            {
                Event* pEvent = g_pEventManager->CreateValidEvent(0x11, 0x38);
                GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();
                pSaveData->saveType = g_pBall->m_uGoalType;
                pSaveData->pShooter = g_pBall->m_pShooter;
                pSaveData->pGoalie = this;
            }

            if (mpSaveData != NULL && (mpSaveData->muSaveType & 3) != 0)
            {
                pFldr = g_pBall->GetOwnerFielder();
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

                MakeSaveEvent(false);
                PickupBall(pBall);

                if (!bIsPerfect)
                {
                    EmitGoalieCatch(this, "goalie_catch", false);
                }
                break;
            }
            else
            {
                if (!mbShouldMiss)
                {
                    MakeSaveEvent(false);

                    pFldr = g_pBall->GetOwnerFielder();
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

                if (!bIsPerfect)
                {
                    EmitGoalieCatch(this, "goalie_deflect", false);
                }

                pBall->m_tNoPickupTimer.SetSeconds(0.08f);

                if (pBall->m_tShotTimer.m_uPackedTime == 0 && mpSaveData != NULL && (mpSaveData->muSaveType & 3) == 0)
                {
                    float fForwardX = m_m4WorldMatrix.m[0][0];
                    float fForwardY = m_m4WorldMatrix.m[0][1];
                    float fForwardZ = m_m4WorldMatrix.m[0][2];
                    float fDotX = pBall->m_v3Position.f.x - m_v3Position.f.x;
                    float fDotY = pBall->m_v3Position.f.y - m_v3Position.f.y;
                    float fDotZ = pBall->m_v3Position.f.z - m_v3Position.f.z;
                    float fDot = fDotX * fForwardX + fDotY * fForwardY + fDotZ * fForwardZ;

                    if (fDot > 0.0f)
                    {
                        static FilteredRandomReal randgenFScale;
                        static FilteredRandomReal randgenVScale;
                        static FilteredRandomReal randgenZVel;

                        float fForwardScale = 1.0f + 1.0f * randgenFScale.genrand();
                        float fVelScale = 0.75f + 0.75f * randgenVScale.genrand();

                        nlVector3 v3NewVel;
                        v3NewVel.f.z = fForwardScale * fForwardZ + fVelScale * m_v3Velocity.f.z;
                        v3NewVel.f.x = fForwardScale * fForwardX + fVelScale * m_v3Velocity.f.x;
                        v3NewVel.f.y = fForwardScale * fForwardY + fVelScale * m_v3Velocity.f.y;

                        float fZVel = 2.0f * randgenZVel.genrand();
                        v3NewVel.f.z = 3.0f + fZVel;

                        nlVector3 v3AngVel;
                        pBall->m_pPhysicsBall->GetAngularVelocity(&v3AngVel);
                        nlVec3Scale(v3AngVel, 0.6f);

                        pBall->SetVelocity(v3NewVel, SPINTYPE_PARAMETER, &v3AngVel);
                    }
                }
            }

            if (mpSaveData != NULL && pBall->m_pPassTarget == NULL)
            {
                float fThreshold = mpSaveData->mfMilestonePercent[1];
                if (fThreshold <= 0.0f)
                {
                    fThreshold = 0.7f * mpSaveData->mfMilestonePercent[2];
                }

                if (mpSaveData->muSaveType != 4 && !(m_pCurrentAnimController->m_fTime < fThreshold))
                    break;

                if (g_pBall->m_uGoalType != 0 && g_pBall->m_uGoalType != 1)
                    break;

                cPlayer* pScorer = g_pGame->m_pScorer;
                bool bStunnedOn = GameInfoManager::GetInstance()->IsStunnedGoaliesOn();

                if (!bStunnedOn)
                {
                    if (pScorer == NULL)
                        break;
                    if (pScorer->IsOnSameTeam(this))
                        break;
                    if (pScorer->m_eClassType != FIELDER)
                        break;
                }

                if (!bStunnedOn)
                {
                    if (g_pBall->m_unk_0xA3)
                        break;
                    if (g_pBall->m_tShotTimer.m_uPackedTime == 0)
                        break;
                    if (pScorer != g_pBall->m_pShooter)
                        break;
                    if (!(*(float*)((u8*)((cFielder*)pScorer)->m_pShotMeter + 0x0C) > 0.8f))
                        break;
                }

                mbDoHeadTrack = false;
                CleanGoalieAction();
                mPrevGoalieActionState = mGoalieActionState;
                mGoalieActionState = GOALIEACTION_STS_RECOVER;
                mnSubstate = 0;
                SetAnimState(0x6F, true, 0.2f, false, false);

                mfWaitTime = ((GoalieTweaks*)m_pTweaks)->fGoalieStunTimeMin;
                float fRange = ((GoalieTweaks*)m_pTweaks)->fGoalieStunTimeMax - mfWaitTime;
                if (fRange > 0.0f)
                {
                    mfWaitTime += nlRandomf(fRange, &nlDefaultSeed);
                }

                InitMovementFromAnim(0, v3Zero, 1.0f, false);
                EmitDaze(this);
                mbStunEffectActive = true;
            }
            break;
        }

        case GOALIEACTION_PURSUE_BALL_CARRIER:
        case GOALIEACTION_PURSUE_BALL_POUNCE:
        {
            cPlayer* pPounceTarget = pBall->GetOwnerFielder();
            if (pPounceTarget != NULL && !IsOnSameTeam(pPounceTarget))
            {
                ExecutePounce(pPounceTarget, true);
            }
            break;
        }

        default:
            break;
        }
    }

    if (pBall->m_pPassTarget != NULL)
    {
        pBall->ClearPassTarget();
    }

    if (pBall->m_tShotTimer.m_uPackedTime != 0)
    {
        pBall->ClearShotInProgress();
    }
}

/**
 * Offset/Address/Size: 0xA178 | 0x8004CC74 | size: 0x6F4
 * TODO: 99.93% match - f0/f2 register swap in loose-ball animation time stores.
 */
void Goalie::CollideWithCharacterCallback(CollisionPlayerPlayerData* pData)
{
    cPlayer* pPlayer = pData->player2;
    cPlayer::CollideWithCharacterCallback(pData);

    bool bHitReactResult;
    nlVector3 v3LocalPos;
    s32 anim;

    switch (mGoalieActionState)
    {
    case GOALIEACTION_MOVE_WB:
    {
        if (IsOnSameTeam(pPlayer))
        {
            return;
        }

        if (pPlayer->m_eClassType != FIELDER)
        {
            return;
        }

        cFielder* pFldr = (cFielder*)pPlayer;
        if (pFldr->IsFallenDown(0.0f))
        {
            return;
        }

        if (m_eAnimID == 0x0B)
        {
            return;
        }
        if (m_eAnimID == 0x0D)
        {
            return;
        }
        if (m_eAnimID == 0x0C)
        {
            return;
        }

        f32 y = pPlayer->m_v3Position.f.y - m_v3Position.f.y;
        f32 x = pPlayer->m_v3Position.f.x - m_v3Position.f.x;
        u16 aHit = (u16)(s32)(10430.378f * nlATan2f(y, x));

        bHitReactResult = pFldr->InitActionHitReact(this, aHit, false);
        mnSubstate = 6;

        GetLocalPoint(v3LocalPos, pPlayer->m_v3Position, m_v3Position, m_aActualFacingDirection);

        if (fabsf(v3LocalPos.f.x / v3LocalPos.f.y) > 1.0f)
        {
            anim = 0x0B;
        }
        else if (v3LocalPos.f.y > 0.0f)
        {
            anim = 0x0D;
        }
        else
        {
            anim = 0x0C;
        }

        bool bShouldSetAnim = false;
        if (anim != m_eAnimID || (bShouldSetAnim = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
        {
            SetAnimState(anim, true, 0.2f, false, false);
        }

        InitMovementFromAnim(0, v3Zero, 1.0f, false);

        if (bHitReactResult)
        {
            PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);
            pPlayer->PlayAttackReactionSounds(100.0f);
        }
        return;
    }

    case GOALIEACTION_STS_ATTACK:
    {
        cFielder* pFldr = g_pBall->GetOwnerFielder();
        bool bDoHit;

        if (pFldr != NULL && !IsOnSameTeam(pFldr) && pFldr->m_eActionState == ACTION_SHOOT_TO_SCORE)
        {
            bDoHit = 1;
        }
        else
        {
            bDoHit = 0;
        }

        if (!bDoHit)
        {
            return;
        }

        if (pPlayer != g_pBall->m_pOwner)
        {
            return;
        }

        if (!(m_pCurrentAnimController->m_fTime > 0.3f))
        {
            return;
        }

        pFldr = g_pBall->GetOwnerFielder();
        if (pFldr == NULL)
        {
            return;
        }

        if (pFldr->m_pBall != NULL)
        {
            pFldr->ReleaseBall();
        }

        pFldr->SetFacingDirection(m_aActualFacingDirection + 0x8000);
        pFldr->InitActionSTSHitReact(this);

        PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);
        pFldr->PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk264);

        nlVector3 v3NewVel;
        nlVec3Scale(v3NewVel, m_v3Position, -((GoalieTweaks*)m_pTweaks)->fSTSAttackBallVelMult);

        f32 yRand = nlRandomf(5.0f, &nlDefaultSeed);
        if ((u32)nlRandom(100, &nlDefaultSeed) > 50)
        {
            yRand *= -1.0f;
        }
        v3NewVel.f.y += yRand;
        v3NewVel.f.z = 4.0f + nlRandomf(2.0f, &nlDefaultSeed);

        g_pBall->SetVelocity(v3NewVel, SPINTYPE_FORWARD, NULL);
        g_pBall->m_tNoPickupTimer.SetSeconds(0.12f);
        return;
    }

    case GOALIEACTION_PURSUE_BALL_CARRIER:
    case GOALIEACTION_PURSUE_BALL_POUNCE:
        if (mbPlayMiss)
        {
            return;
        }

        if (!IsOnSameTeam(pPlayer))
        {
            ExecutePounce(pPlayer, false);
        }
        return;

    case GOALIEACTION_MOVE:
    {
        s32 jointIndex = m_nRightFootJointIndex;
        f32 zCheck = GetJointPosition(m_nHeadJointIndex).f.z + 0.4f;
        if (pPlayer->GetJointPosition(jointIndex).f.z > zCheck)
        {
            return;
        }
    }

    case GOALIEACTION_SAVE:
    case GOALIEACTION_LOOSEBALL_SETUP:
    case GOALIEACTION_LOOSEBALL_CATCH:
    case GOALIEACTION_LOOSEBALL_PICKUP:
    case GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING:
    case GOALIEACTION_LOOSEBALL_PURSUE_ROLLING:
        if (IsOnSameTeam(pPlayer))
        {
            return;
        }
        if (pPlayer == NULL)
        {
            return;
        }
        if (pPlayer->m_eClassType != FIELDER)
        {
            return;
        }
        if (((cFielder*)pPlayer)->IsFallenDown(0.0f))
        {
            return;
        }

        pPlayer->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);

        if (pPlayer->m_pBall != NULL)
        {
            pPlayer->ReleaseBall();
        }

        if (IsOnSameTeam(pPlayer))
        {
            ((cFielder*)pPlayer)->EndDesire(false);
            ((cFielder*)pPlayer)->EndAction();
        }
        else
        {
            ((cFielder*)pPlayer)->InitActionSlideAttackReact(this, false);
        }
        return;

    case GOALIEACTION_LOOSEBALL_DESPERATE:
        if (pPlayer == g_pBall->m_pOwner)
        {
            if (pPlayer != NULL)
            {
                if (IsOnSameTeam(pPlayer))
                {
                    return;
                }

                if (!((cFielder*)pPlayer)->IsFallenDown(0.0f) && !((cFielder*)pPlayer)->IsInvincible())
                {
                    if (pPlayer != NULL && pPlayer->m_eClassType == FIELDER && !((cFielder*)pPlayer)->IsFallenDown(0.0f))
                    {
                        pPlayer->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);

                        if (pPlayer->m_pBall != NULL)
                        {
                            pPlayer->ReleaseBall();
                        }

                        if (IsOnSameTeam(pPlayer))
                        {
                            ((cFielder*)pPlayer)->EndDesire(false);
                            ((cFielder*)pPlayer)->EndAction();
                        }
                        else
                        {
                            ((cFielder*)pPlayer)->InitActionSlideAttackReact(this, false);
                        }
                    }
                }
            }

            anim = mpLooseBallInfo->mnAnimID;
            if (anim != m_eAnimID)
            {
                bool bShouldSetLooseAnim = false;
                if (anim != m_eAnimID || (bShouldSetLooseAnim = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
                {
                    SetAnimState(anim, true, 0.2f, false, false);
                }

                cPN_SAnimController* pAnim = m_pCurrentAnimController;
                f32 curTime = pAnim->m_fTime;
                f32 targetTime = 0.5f * mpLooseBallInfo->mfPickupTime;
                pAnim->m_fPrevTime = curTime;
                pAnim->m_fTime = targetTime;
                InitMovementFromAnim(0, v3Zero, 1.0f, false);
            }

            if (m_pBall == NULL)
            {
                PickupBall(g_pBall);
                mbPickedUp = true;
                g_pBall->ClearShotInProgress();
                EmitGoalieCatch(this, "goalie_catch", false);
            }
            return;
        }

        if (IsOnSameTeam(pPlayer))
        {
            return;
        }
        if (pPlayer == NULL)
        {
            return;
        }
        if (pPlayer->m_eClassType != FIELDER)
        {
            return;
        }
        if (((cFielder*)pPlayer)->IsFallenDown(0.0f))
        {
            return;
        }

        pPlayer->PlayRandomCharDialogue(CHAR_DIALOGUE_HIT, VECTORS, 100.0f, -1.0f);

        if (pPlayer->m_pBall != NULL)
        {
            pPlayer->ReleaseBall();
        }

        if (IsOnSameTeam(pPlayer))
        {
            ((cFielder*)pPlayer)->EndDesire(false);
            ((cFielder*)pPlayer)->EndAction();
        }
        else
        {
            ((cFielder*)pPlayer)->InitActionSlideAttackReact(this, false);
        }
        return;

    default:
        return;
    }
}

/**
 * Offset/Address/Size: 0xA01C | 0x8004CB18 | size: 0x15C
 */
bool Goalie::PreCollideWithBallCallback(const dContact&)
{
    switch (mGoalieActionState)
    {
    case GOALIEACTION_STS:
        if (m_eAnimID == 0x6d)
        {
            return false;
        }
        if (mpSaveData != NULL && mpSaveData->muSaveType == 0x40000)
        {
            return false;
        }
        break;

    case GOALIEACTION_LOOSEBALL_PURSUE_ROLLING:
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

        const LooseBallInfo* pInfo = mpLooseBallInfo;
        f32 pickupDist = pInfo->mfPickupDistance;
        if (0.3f < pickupDist)
        {
            f32 pickupTime = pInfo->mfPickupTime;
            mfTargetTime = (pickupDist - 0.3f) * pickupTime / pickupDist;

            cPN_SAnimController* pAnim = m_pCurrentAnimController;
            f32 targetTime = mfTargetTime;
            pAnim->m_fPrevTime = pAnim->m_fTime;
            pAnim->m_fTime = targetTime;
        }

        InitiatePickup();
        return false;
    }

    case GOALIEACTION_LOOSEBALL_PICKUP:
        if (InitiatePickup())
        {
            return false;
        }
        break;

    case GOALIEACTION_MOVE:
    case GOALIEACTION_MOVE_WB:
    case GOALIEACTION_SAVE_SETUP:
    case GOALIEACTION_SAVE_REPOSITION:
    case GOALIEACTION_SAVE:
    case GOALIEACTION_MISS_CHIP_SHOT:
    case GOALIEACTION_DIVE_RECOVER:
    case GOALIEACTION_STS_SETUP:
    case GOALIEACTION_STS_RECOVER:
    case GOALIEACTION_STS_ATTACK_SETUP:
    case GOALIEACTION_STS_ATTACK:
    case GOALIEACTION_PASS:
    case GOALIEACTION_PASS_INTERCEPT:
    case GOALIEACTION_PRE_CROUCH:
    case GOALIEACTION_PURSUE_BALL_CARRIER:
    case GOALIEACTION_PURSUE_BALL_POUNCE:
    case GOALIEACTION_LOOSEBALL_SETUP:
    case GOALIEACTION_LOOSEBALL_CATCH:
    case GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING:
    case GOALIEACTION_LOOSEBALL_DESPERATE:
    case GOALIEACTION_OFFPLAY:
    case GOALIEACTION_SNAP_BALL:
    case GOALIEACTION_GRAB_BALL:
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
                    else if ((fDistanceSq > fOverhandThrowDistanceSq) || (fOpenTo < 0.85f))
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
                else if ((fDistanceSq > fOverhandThrowDistanceSq) || (fOpenTo < 0.85f))
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
 * TODO: 93.53% match - register allocation still diverges in navigation transition branches and blender setup.
 */
void Goalie::DoNavigation(float fDeltaT, float fIdleDistance, Goalie::eNaviMode naviMode)
{
    int nNewAnim;
    u16 aNavDir;
    int nFinalAnim;
    u16 aFinalDir;
    bool bDoBackward;
    s16 aGoalie2Ball;
    u16 absBallAngleDiff;
    bool bDoSeek;
    u16 desiredAng;

    cPN_SAnimController* pAnimCtrl = m_pCurrentAnimController;
    int eAnimID = m_eAnimID;
    f32 fTime = pAnimCtrl->m_fTime;
    cBall* pBall = g_pBall;

    GetLocalPoint(mv3LocalNavTarget, mv3NavTarget, m_v3Position, m_aActualFacingDirection);

    float distSq = mv3LocalNavTarget.f.x * mv3LocalNavTarget.f.x + mv3LocalNavTarget.f.y * mv3LocalNavTarget.f.y;

    if (naviMode == NAVI_FACE_BALL)
    {
        float dy = pBall->m_v3Position.f.y - m_v3Position.f.y;
        float dx = pBall->m_v3Position.f.x - m_v3Position.f.x;
        m_aDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(dy, dx));
    }

    aGoalie2Ball = (s16)(m_aDesiredFacingDirection - m_aActualFacingDirection);
    absBallAngleDiff = (u16)(aGoalie2Ball < 0 ? -aGoalie2Ball : aGoalie2Ball);

    if (distSq < fIdleDistance * fIdleDistance)
    {
        if (absBallAngleDiff > 0x2AA8)
        {
            int idleTurns[2][2] = {
                { 0x10, 0x12 },
                { 0x0F, 0x11 },
            };
            bool neg = (aGoalie2Ball <= 0);
            bool big = absBallAngleDiff >= 0x5FFA;
            nNewAnim = idleTurns[neg][big];
        }
        else
        {
            nNewAnim = 0x08;
        }

        bool bShouldChange = false;
        if (mnSubstate != 1 || (bShouldChange = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
        {
            bool bForceChange = false;
            if (nNewAnim != m_eAnimID || (bForceChange = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
            {
                SetAnimState(nNewAnim, true, 0.2f, false, false);
            }
        }

        if (nNewAnim == 0x08)
        {
            InitMovementFromAnim(0, v3Zero, 0.0f, false);
        }
        else
        {
            InitMovementFromAnimSeek(32768.0f, 2730.6667f);
        }

        if (mnSubstate != 1)
        {
            mnSubstate = 1;
        }
        mMoveDirection = GOALIEDIR_IDLE;
        return;
    }

    float runDistSq = mfGoalieRunDist * mfGoalieRunDist;
    float urgentDistSq = mfGoalieUrgentDist * mfGoalieUrgentDist;

    desiredAng = (u16)(s32)(10430.378f * nlATan2f(mv3LocalNavTarget.f.y, mv3LocalNavTarget.f.x));

    if (mMoveDirection != GOALIEDIR_SIDE && (distSq > runDistSq || (mUrgency == URGENCY_HIGH && distSq > urgentDistSq)))
    {
        mnSubstate = 4;

        s16 ballDiff = (s16)(desiredAng - (u16)aGoalie2Ball);
        aFinalDir = (u16)(desiredAng + m_aActualFacingDirection);
        u16 deltaToBall = (u16)(ballDiff < 0 ? -ballDiff : ballDiff);
        nFinalAnim = 0x26;
        bDoSeek = true;

        if (naviMode == NAVI_FACE_DESIRED)
        {
            if (absBallAngleDiff < 0x3FFC)
            {
                if (deltaToBall > 0x4E34)
                {
                    aFinalDir = (u16)(aFinalDir + 0x8000);
                    nFinalAnim = 0x27;
                }
                else if (deltaToBall > 0x31C4)
                {
                    if (mv3LocalNavTarget.f.y > 0.0f)
                    {
                        aFinalDir = (u16)(aFinalDir - 0x4000);
                        nFinalAnim = 0x23;
                    }
                    else
                    {
                        aFinalDir = (u16)(aFinalDir + 0x4000);
                        nFinalAnim = 0x22;
                    }
                }
            }
        }
        else if (naviMode == NAVI_FACE_BALL)
        {
            bDoBackward = false;

            if (mMoveDirection == GOALIEDIR_IDLE || mMoveDirection == GOALIEDIR_BACKWARD)
            {
                float dy = pBall->m_v3Position.f.y - mv3NavTarget.f.y;
                float dx = pBall->m_v3Position.f.x - mv3NavTarget.f.x;
                float ballToNavDistSq = dx * dx + dy * dy;

                if (distSq < ballToNavDistSq)
                {
                    u16 ballAng = (u16)(s32)(10430.378f * nlATan2f(dy, dx));
                    s16 diff = (s16)(aFinalDir + (u16)(ballAng + 0x8000));
                    if ((u16)(diff < 0 ? -diff : diff) < 0x1554)
                    {
                        bDoBackward = true;
                    }
                }
            }

            if (mMoveDirection == GOALIEDIR_BACKWARD || mMoveDirection == GOALIEDIR_FRONT2BACK)
            {
                if (deltaToBall > 0x3FFC)
                {
                    bDoBackward = true;
                }
            }
            else
            {
                if (deltaToBall > 0x5550)
                {
                    bDoBackward = true;
                }
            }

            if (bDoBackward)
            {
                if (!mbDoIntercept || (mMoveDirection != GOALIEDIR_FORWARD && mMoveDirection != GOALIEDIR_BACK2FRONT))
                {
                    aFinalDir = (u16)(aFinalDir + 0x8000);
                    nFinalAnim = 0x27;
                }
            }
        }

        s16 finalDiff = (s16)((u16)aFinalDir - m_aActualFacingDirection);
        u16 absFinalDiff = (u16)(finalDiff < 0 ? -finalDiff : finalDiff);

        nNewAnim = nFinalAnim;
        bDoSeek = true;

        if (nFinalAnim == 0x26)
        {
            switch (mMoveDirection)
            {
            case GOALIEDIR_IDLE:
                mMoveDirection = GOALIEDIR_FORWARD;
                if (absFinalDiff < 0x3552)
                {
                    m_aDesiredFacingDirection = aFinalDir;
                    nNewAnim = 0x26;
                }
                else
                {
                    if (absFinalDiff > 0x6388)
                    {
                        nNewAnim = 0x24;
                        if (aGoalie2Ball > 0)
                            nNewAnim = 0x25;
                    }
                    else
                    {
                        nNewAnim = 0x24;
                        if (finalDiff > 0)
                            nNewAnim = 0x25;
                    }
                    m_aDesiredFacingDirection = aFinalDir;
                    mfSwitchTime = 0.5f;
                }
                break;

            case GOALIEDIR_FORWARD:
                m_aDesiredFacingDirection = aFinalDir;
                if (absFinalDiff < 0x3FFC)
                {
                    if ((eAnimID == 0x25 || eAnimID == 0x24) && fTime < mfSwitchTime)
                        break;
                    nNewAnim = 0x26;
                }
                else
                {
                    if (absFinalDiff > 0x6388)
                    {
                        nNewAnim = 0x24;
                        if (aGoalie2Ball > 0)
                            nNewAnim = 0x25;
                    }
                    else
                    {
                        nNewAnim = 0x24;
                        if (finalDiff > 0)
                            nNewAnim = 0x25;
                    }
                    mfSwitchTime = 0.5f;
                }
                break;

            case GOALIEDIR_BACKWARD:
                mMoveDirection = GOALIEDIR_BACK2FRONT;
                m_aDesiredFacingDirection = (u16)((u16)aFinalDir + 0x8000);
                bDoSeek = false;
                if (absFinalDiff < 0x1FFE)
                {
                    nNewAnim = 0x2D;
                    mfSwitchTime = 0.42857143f;
                }
                else
                {
                    nNewAnim = 0x2A;
                    if (finalDiff < 0)
                        nNewAnim = 0x2B;
                    mfSwitchTime = 0.5714286f;
                }
                break;

            case GOALIEDIR_SIDE:
                break;

            case GOALIEDIR_BACK2FRONT:
                bDoSeek = false;
                if (fTime > mfSwitchTime)
                    mMoveDirection = GOALIEDIR_FORWARD;
                break;

            case GOALIEDIR_FRONT2BACK:
                bDoSeek = false;
                if (fTime > mfSwitchTime)
                    bDoSeek = true;
                if (fTime < mfSwitchTime)
                    mMoveDirection = GOALIEDIR_FORWARD;
                else
                    mMoveDirection = GOALIEDIR_BACKWARD;
                break;
            }
        }
        else if (nFinalAnim == 0x27)
        {
            switch (mMoveDirection)
            {
            case GOALIEDIR_IDLE:
                mMoveDirection = GOALIEDIR_BACKWARD;
                if (absFinalDiff < 0x3FFC)
                {
                    m_aDesiredFacingDirection = aFinalDir;
                    nNewAnim = 0x27;
                }
                else
                {
                    if (absFinalDiff < 0x6388)
                    {
                        nNewAnim = 0x28;
                        if (finalDiff > 0)
                            nNewAnim = 0x29;
                    }
                    else
                    {
                        nNewAnim = 0x28;
                        if (aGoalie2Ball > 0)
                            nNewAnim = 0x29;
                    }
                    mMoveDirection = GOALIEDIR_FRONT2BACK;
                    m_aDesiredFacingDirection = (u16)((u16)aFinalDir + 0x8000);
                    bDoSeek = false;
                    mfSwitchTime = 0.71428573f;
                }
                break;

            case GOALIEDIR_FORWARD:
                mMoveDirection = GOALIEDIR_FRONT2BACK;
                m_aDesiredFacingDirection = (u16)((u16)aFinalDir + 0x8000);
                bDoSeek = false;
                if (absFinalDiff < 0x1FFE)
                {
                    nNewAnim = 0x2C;
                    mfSwitchTime = 0.5f;
                }
                else
                {
                    if (absFinalDiff < 0x6388)
                    {
                        nNewAnim = 0x28;
                        if (finalDiff > 0)
                            nNewAnim = 0x29;
                    }
                    else
                    {
                        nNewAnim = 0x28;
                        if (aGoalie2Ball > 0)
                            nNewAnim = 0x29;
                    }
                    mfSwitchTime = 0.71428573f;
                }
                break;

            case GOALIEDIR_BACKWARD:
                m_aDesiredFacingDirection = aFinalDir;
                nNewAnim = 0x27;
                break;

            case GOALIEDIR_SIDE:
                break;

            case GOALIEDIR_BACK2FRONT:
                bDoSeek = false;
                if (fTime < mfSwitchTime)
                    mMoveDirection = GOALIEDIR_BACKWARD;
                else
                    mMoveDirection = GOALIEDIR_FORWARD;
                break;

            case GOALIEDIR_FRONT2BACK:
                bDoSeek = false;
                if (fTime > mfSwitchTime)
                    bDoSeek = true;
                if (fTime > mfSwitchTime)
                    mMoveDirection = GOALIEDIR_BACKWARD;
                break;
            }
        }
        else
        {
            nNewAnim = nFinalAnim;
            mMoveDirection = GOALIEDIR_SIDE;
            m_aDesiredFacingDirection = aFinalDir;
        }

        if (nNewAnim != m_eAnimID || (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f))
        {
            SetAnimState(nNewAnim, true, 0.2f, false, false);
        }

        if (nNewAnim == 0x27)
        {
            m_pCurrentAnimController->m_fPlaybackSpeedScale = ((GoalieTweaks*)m_pTweaks)->fSaveBackRunTimeScale;
        }

        if (bDoSeek)
        {
            InitMovementFromAnimSeek(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff);
        }
        else
        {
            InitMovementFromAnim(0, v3Zero, 1.0f, false);
        }
        return;
    }

    aNavDir = SeekDirection(m_aActualFacingDirection, m_aDesiredFacingDirection, m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff, fDeltaT);
    SetFacingDirection(aNavDir);

    GetLocalPoint(mv3LocalNavTarget, mv3NavTarget, m_v3Position, aNavDir);

    bool bNeedChange = false;
    int* nLeftAnims = 0;
    int* nRightAnims = 0;

    maLocalAngle = (u16)(s32)(10430.378f * nlATan2f(mv3LocalNavTarget.f.y, mv3LocalNavTarget.f.x));

    static int gStepLAnims[3] = { 0x1D, 0x1C, 0x1E };
    static int gStepRAnims[3] = { 0x1A, 0x19, 0x1B };
    static int gStrafeLAnims[3] = { 0x16, 0x17, 0x18 };
    static int gStrafeRAnims[3] = { 0x13, 0x14, 0x15 };

    float strafeDistSq = mfGoalieStrafeDist * mfGoalieStrafeDist;

    if (distSq < strafeDistSq && mUrgency == URGENCY_LOW && mnSubstate != 3)
    {
        if (mnSubstate != 2)
        {
            mnSubstate = 2;
            bNeedChange = true;
            nLeftAnims = gStepLAnims;
            nRightAnims = gStepRAnims;
        }
    }
    else
    {
        bool bDoStrafe = true;
        if (mMoveDirection == GOALIEDIR_SIDE)
        {
            float dy1 = mv3NavTarget.f.y - m_v3PrevPosition.f.y;
            float dy2 = mv3NavTarget.f.y - m_v3Position.f.y;
            float dx1 = mv3NavTarget.f.x - m_v3PrevPosition.f.x;
            float dx2 = mv3NavTarget.f.x - m_v3Position.f.x;
            float distOldSq = dx1 * dx1 + dy1 * dy1;
            float distNewSq = dx2 * dx2 + dy2 * dy2;

            if (distNewSq <= distOldSq)
            {
                if (!(m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f))
                {
                    bDoStrafe = false;
                }
            }
        }

        if (bDoStrafe && mnSubstate != 3)
        {
            mnSubstate = 3;
            bNeedChange = true;
            nLeftAnims = gStrafeLAnims;
            nRightAnims = gStrafeRAnims;
        }
    }

    if (mnSubstate == 3)
    {
        f32 fDist = nlSqrt(distSq, true);
        f32 fSpeedScale = InterpolateRangeClamped(0.25f, 2.0f, mfGoalieStepDist, 2.0f * mfGoalieStrafeDist, fDist);

        if (mFatigue.mfEnergyLevel < ((GoalieTweaks*)m_pTweaks)->fGetupEnergyHigh && fSpeedScale > 0.8f)
        {
            fSpeedScale = InterpolateRangeClamped(0.8f, fSpeedScale, ((GoalieTweaks*)m_pTweaks)->fGetupEnergyLow, ((GoalieTweaks*)m_pTweaks)->fGetupEnergyHigh, mFatigue.mfEnergyLevel);
        }

        mfSpeedScale = SeekSpeed(mfSpeedScale, fSpeedScale, 10.0f, 5.0f, fDeltaT);
    }

    if (!bNeedChange)
    {
        return;
    }

    mMoveDirection = GOALIEDIR_IDLE;

    cPN_SingleAxisBlender* pSAB_L = CreateSingleAxisBlender(nLeftAnims, 3, 1, MoveWeightCB, 0.1f, 0);
    cPN_SingleAxisBlender* pSAB_R = CreateSingleAxisBlender(nRightAnims, 3, 1, MoveWeightCB, 0.1f, 0);

    cPN_SingleAxisBlender* pSAB = pSAB_L;
    cPN_SAnimController* pPrevCtrlr = 0;
    int j;

    for (j = 0; j < 2; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            cPN_SAnimController* pCtrlr = (cPN_SAnimController*)pSAB->GetChild(i);

            if (i != 1 || j > 0)
            {
                pCtrlr->m_bIgnoreTriggers = true;
            }

            if (mnSubstate == 3)
            {
                if (pPrevCtrlr == 0)
                {
                    pCtrlr->m_funcSychronizedWeightCallback = StrafeSynchronizedSpeedCallback;
                    pCtrlr->m_nSynchronizedWeightCallbackParam = (unsigned int)this;
                    pCtrlr->m_fSynchronizedWeight = 0.0f;
                }
                else
                {
                    pCtrlr->m_bIsSynchronized = true;
                    pPrevCtrlr->m_pSynchronizedController = pCtrlr;
                }
                pPrevCtrlr = pCtrlr;
            }
        }
        pSAB = pSAB_R;
    }

    cPN_SingleAxisBlender* pDirBlender = AllocateSingleAxisBlender();
    if (pDirBlender)
    {
        pDirBlender = new (pDirBlender) cPN_SingleAxisBlender(2, MoveDirectionCB, (unsigned int)this, 0.1f);
    }

    pDirBlender->SetChild(0, pSAB_L);
    pDirBlender->SetChild(1, pSAB_R);

    MoveWeightCB((unsigned int)this, pSAB_L);
    MoveWeightCB((unsigned int)this, pSAB_R);
    MoveDirectionCB((unsigned int)this, pDirBlender);

    cPN_Blender* pBlender = AllocateBlender();
    if (pBlender)
    {
        new (pBlender) cPN_Blender(*m_pAILayer, pDirBlender, 0.1f);
    }

    *m_pAILayer = (cPoseNode*)pBlender;
    InitMovementFromAnim(0, v3Zero, 1.0f, true);
}

static inline float goalie_clamp_min(float x, float min)
{
    if (x >= min)
        return x;
    else
        return min;
}

static inline float goalie_clamp_max(float x, float max)
{
    if (x <= max)
        return x;
    else
        return max;
}

static inline float goalie_clamp_positive(float x)
{
    if (0.0f >= x)
        return 0.0f;
    else
        return x;
}

/**
 * Offset/Address/Size: 0x82F0 | 0x8004ADEC | size: 0x588
 * TODO: 97.9% match - floating-point register allocation mismatch in breakaway and desired-position calculations
 */
void Goalie::FindDesiredGoaliePosition(nlVector3& pos, nlVector3& dir, nlVector3& focus, unsigned short& ang, const nlVector3* pThreatPos)
{
    cNet* pNet = m_pTeam->m_pNet;
    nlVector3 targetPos;
    float breakaway;
    float goalX;
    nlVector3 desiredVec;
    float goalY = 0.0f;

    goalX = 0.5f * pNet->m_sideSign + pNet->GetGoalLineX();

    if (pThreatPos == NULL)
    {
        cBall* pBall = g_pBall;
        if (pBall->m_pOwner != NULL)
        {
            cPlayer* pOwner = pBall->m_pOwner;
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
    float limit = goalLineX - 3.0f * absTargetY / fNetY;

    if (targetPos.f.x > limit)
    {
        targetPos.f.x = limit;
    }
    else if (targetPos.f.x < -limit)
    {
        targetPos.f.x = -limit;
    }

    desiredVec.f.x = targetPos.f.x - goalX;
    desiredVec.f.y = targetPos.f.y - goalY;
    desiredVec.f.z = targetPos.f.z - goalY;

    float goalLine = cField::GetGoalLineX(1U) - 0.5f;
    float goalieDist = nlSqrt(desiredVec.f.x * desiredVec.f.x + desiredVec.f.y * desiredVec.f.y + desiredVec.f.z * desiredVec.f.z, true);
    breakaway = 0.0f;
    float targetDist = goalieDist - 0.5f;

    cFielder* ownerFielder2;
    cFielder* ownerFielder = g_pBall->GetOwnerFielder();
    if (ownerFielder != NULL && !IsOnSameTeam(ownerFielder))
    {
        ownerFielder2 = g_pBall->GetOwnerFielder();
        bool isShooting;
        if (ownerFielder2 != NULL && !IsOnSameTeam(ownerFielder2) && ownerFielder2->m_eActionState == ACTION_SHOOT_TO_SCORE)
        {
            isShooting = true;
        }
        else
        {
            isShooting = false;
        }
        if (isShooting)
        {
            breakaway = 0.0f;
        }
        else
        {
            breakaway = OnBreakaway(ownerFielder);
            if (breakaway > 0.8f)
            {
                int i;
                cTeam* opponentTeam = ownerFielder->m_pTeam;
                for (i = 0; i < 4; i++)
                {
                    cFielder* fielder = opponentTeam->GetFielder(i);
                    if (fielder == ownerFielder)
                        continue;
                    cNet* myNet = m_pTeam->m_pNet;
                    const nlVector3& fielderPos = fielder->m_v3Position;
                    float absX = (float)fabs(fielderPos.f.x);
                    if (fielder->m_v3Position.f.x * myNet->m_baseLocation.f.x > 0.0 && absX > 2.0f)
                    {
                        float factor = 1.0f - 0.5f * (absX - 2.0f) / 10.0f;
                        factor = goalie_clamp_positive(factor);
                        breakaway *= factor;
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
    float desiredZ = ratio * desiredVec.f.z + goalY;
    float desiredY = ratio * desiredVec.f.y + goalY;
    float desiredX = ratio * desiredVec.f.x + goalX;

    if ((float)fabs(m_v3Position.f.x) > cField::GetGoalLineX(1U))
    {
        float halfNetMinusOne = 0.5f * cNet::m_fNetWidth - 1.0f;
        desiredY = goalie_clamp_min(desiredY, -halfNetMinusOne);
        desiredY = goalie_clamp_max(desiredY, halfNetMinusOne);
        desiredX = goalLine * pNet->m_sideSign;
        desiredVec.f.x = desiredX - m_v3Position.f.x;
        desiredVec.f.y = desiredY - m_v3Position.f.y;
        desiredVec.f.z = desiredZ - m_v3Position.f.z;
    }

    ang = (unsigned short)(10430.378f * nlATan2f(desiredVec.f.y, desiredVec.f.x));

    desiredX = goalie_clamp_min(desiredX, -goalLine);
    desiredX = goalie_clamp_max(desiredX, goalLine);

    pos.f.x = 0.8f * desiredX + 0.2f * m_v3Position.f.x;
    pos.f.y = 0.8f * desiredY + 0.2f * m_v3Position.f.y;
    pos.f.z = 0.8f * desiredZ + 0.2f * m_v3Position.f.z;
    dir = desiredVec;
    focus = targetPos;
}

/**
 * Offset/Address/Size: 0x8024 | 0x8004AB20 | size: 0x2CC
 * TODO: 99.94% match - remaining f31 nav-target Y delta load/sub order mismatch.
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
                fDropTime = (f32)(fDropTime - m_v3Position.f.x);

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
 * TODO: 99.72% match - r30/r31 register swap in BasicString constructor inlining
 */
void Goalie::HandleSTSContact(cBall* pBall)
{
    if (pBall->m_tShotTimer.m_uPackedTime == 0)
    {
        return;
    }
    if (pBall->m_pOwner != NULL)
    {
        return;
    }
    if (m_eAnimID == 0x6D)
    {
        return;
    }

    if (mpSaveData != NULL)
    {
        u32 saveType = mpSaveData->muSaveType;
        if (saveType == 0x40000 || (mbShouldMiss && (saveType & 0xFFFC) != 0))
        {
            return;
        }
    }

    Audio::SoundAttributes sndAtr;
    sndAtr.Init();
    sndAtr.SetSoundType(0xB7, true);
    sndAtr.UseStationaryPosVector(m_v3Position);
    Audio::gStadGenSFX.Play(sndAtr);

    if (mpSaveData != NULL)
    {
        cPlayer* pPrevOwner = g_pBall->m_pPrevOwner;
        if (pPrevOwner != NULL)
        {
            if (pPrevOwner->m_eClassType == 2)
            {
                BasicString<char, Detail::TempStringAllocator> effectName(
                    GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((s16)pPrevOwner->m_pTeam->m_nSide)));
                effectName.AppendInPlace("_shoot_to_score_catch");
                EmitGoalieCatch(this, effectName.c_str(), true);
            }
        }

        u32 shotFlags = mpSaveData->muSaveType;
        if ((shotFlags & 0x20003) != 0)
        {
            if ((shotFlags & 3) != 0)
            {
                MakeSaveEvent(true);
            }

            PickupBall(pBall);

            cBaseCamera* pCamera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
            if (pCamera->GetType() == eCameraType_MatrixEffect)
            {
                FireCameraRumbleFilter(0.0f, 0.2f);
            }

            if (m_eAnimID == 0x6C)
            {
                cNet* pNet = m_pTeam->m_pNet;
                if (pNet->m_baseLocation.f.x > 0.0f)
                {
                    Goalie::mbPosGoalieNetCheck = true;
                }
                else
                {
                    Goalie::mbNegGoalieNetCheck = true;
                }
            }
        }
        else if (mpSaveData->muSaveType == 0x10000)
        {
            MakeSaveEvent(true);

            EmitDaze(this);
            mbStunEffectActive = true;
        }
    }
    else
    {
        if (mpShooter != NULL)
        {
            MakeSaveEvent(true);

            PickupBall(pBall);

            FixedUpdateTask::mTimeScale = 0.75f;
            ParticleUpdateTask::SetTimeScale(0.75f);
        }
    }

    pBall->ClearShotInProgress();
}

/**
 * Offset/Address/Size: 0x75B4 | 0x8004A0B0 | size: 0x494
 * TODO: 99.80% match - stack-offset diffs remain in FuzzyVariant temporary layout
 * during pass-target selection, plus operand-order register diffs in the 0.5f
 * velocity scaling multiplies.
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
                    mUrgency = URGENCY_LOW;
                    mfSpeedScale = 1.0f;

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

            nlVector3 v3Pos = g_pBall->m_v3Position;

            PickupBall(g_pBall);
            mbPickedUp = true;
            g_pBall->SetPosition(v3Pos);

            if (mpLooseBallInfo->mnAnimID == 5 || mpLooseBallInfo->mnAnimID == 4)
            {
                mpPassTarget = NULL;
            }
            else
            {
                cPlayer* pPassTarget;
                if (GetGlobalPad() != NULL)
                {
                    pPassTarget = DoFindBestPassTarget(false, false);
                }
                else
                {
                    FuzzyVariant fvTarget = Fuzzy::GetBestPassTarget(this);
                    if (fvTarget.Confidence >= 0.5f)
                    {
                        pPassTarget = fvTarget.mData.pPlayer;
                    }
                    else
                    {
                        pPassTarget = DoFindBestPassTarget(false, false);
                    }
                }

                mpPassTarget = pPassTarget;
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
            if (g_pBall->m_tNoPickupTimer.m_uPackedTime == 0)
            {
                mfWaitTime = 0.1f;
                SetNoPickUpTime(mfWaitTime);

                nlVector3 v3BallVel = g_pBall->m_v3Velocity;

                float fSpeedSq = v3BallVel.f.x * v3BallVel.f.x + v3BallVel.f.y * v3BallVel.f.y + v3BallVel.f.z * v3BallVel.f.z;
                if (fSpeedSq > 64.0f)
                {
                    v3BallVel.f.x = 0.5f * v3BallVel.f.x;
                    v3BallVel.f.y = 0.5f * v3BallVel.f.y;
                    v3BallVel.f.z = 0.5f * v3BallVel.f.z;
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
 * Offset/Address/Size: 0x6FF0 | 0x80049AE4 | size: 0x268
 * TODO: 99.94% match - remaining difference is fmadds operand order in net-width margin check
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
        double absX;
        float result = FakeBallWorld::GetPredictedPlaneIntersectTime(plane, v3TargetPosition, localVelocity);

        if (!(result <= 0.0f))
        {
            absX = __fabs(v3TargetPosition.f.x);
            saveIgnoreMargin = ((GoalieTweaks*)m_pTweaks)->fSaveIgnoreMargin;

            bool bInNet;
            if ((float)absX > (cField::GetGoalLineX(1U) - 1.0f)
                && (float)fabs(v3TargetPosition.f.y) < (saveIgnoreMargin + cNet::m_fNetWidth / 2.0f)
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

static inline float clamp_upper(float x, float hi)
{
    if (x <= hi)
        return x;
    else
        return hi;
}

static inline float clamp_lower(float x, float lo)
{
    if (x >= lo)
        return x;
    else
        return lo;
}

/**
 * Offset/Address/Size: 0x69D8 | 0x800494D4 | size: 0x3A8
 * TODO: 98.65% match - FPR allocation in save-margin/goal-distance checks
 * and final target-time clamp branch.
 */
bool Goalie::CheckForSTSAttack()
{
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
        f32 fAnimScale;
        f32 fSaveIgnoreMargin;
        f32 ownerDistSq;
        f32 fCloseDistSq;
        f32 fMaxDistSq;
        f32 fCurrentAnimTime;
        bool bInNetZone;

        cFielder* pOppFielder = g_pBall->GetOwnerFielder();

        cPN_SAnimController* pController = pOppFielder->m_pCurrentAnimController;
        fAnimScale = (f32)pController->m_pSAnim->m_nNumKeys / 30.0f;
        fCurrentAnimTime = fAnimScale * pController->m_fTime;
        f32 fTriggerTime = fAnimScale * GetCurrentAnimTriggerTime(pOppFielder, 0x85181B83, 0);

        f32 fPickupDuration = LooseBallAnims::mAttackSTSInfo.mfPickupTime * LooseBallAnims::mAttackSTSInfo.mfAnimDuration;
        if ((fCurrentAnimTime + fPickupDuration) < fTriggerTime)
        {
            do
            {
                ownerDistSq = pOppFielder->m_v3Position.CalculateDistanceSquared2D(m_v3Position);

                f32 fCloseDist = LooseBallAnims::mAttackSTSInfo.mfPickupDistance + ((GoalieTweaks*)m_pTweaks)->fSTSAttackCloseDistance;
                f32 fMaxDist = LooseBallAnims::mAttackSTSInfo.mfPickupDistance + ((GoalieTweaks*)m_pTweaks)->fSTSAttackMaxDistance;
                fCloseDistSq = fCloseDist * fCloseDist;
                fMaxDistSq = fMaxDist * fMaxDist;

                fSaveIgnoreMargin = ((GoalieTweaks*)m_pTweaks)->fSaveIgnoreMargin;

                if (((float)fabsf(pOppFielder->m_v3Position.f.x) > (cField::GetGoalLineX(1U) - 1.0f))
                    && ((float)fabsf(pOppFielder->m_v3Position.f.y) < (cNet::m_fNetWidth * 0.5f + fSaveIgnoreMargin))
                    && (pOppFielder->m_v3Position.f.z < (fSaveIgnoreMargin + cNet::m_fNetHeight)))
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

                if (pOppFielder->m_v3Position.f.y >= clampedY)
                {
                    clampedY = pOppFielder->m_v3Position.f.y;
                }

                clampedY = clamp_upper(clampedY, halfWidth);

                v3GoalPos.f.y = clampedY;

                f32 distSqFielder = v3GoalPos.CalculateDistanceSquared2D(pOppFielder->m_v3Position);
                f32 distSqGoalie = v3GoalPos.CalculateDistanceSquared2D(m_v3Position);

                static FilteredRandomChance randgenSTS;

                if (!bInNetZone)
                {
                    if (distSqFielder < distSqGoalie)
                    {
                    }
                    else
                    {
                        if (ownerDistSq <= fCloseDistSq)
                        {
                        }
                        else if (ownerDistSq <= fMaxDistSq)
                        {
                            if (!randgenSTS.genrand(((GoalieTweaks*)m_pTweaks)->fSTSAttackChancePerFrame))
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                f32 fTimeToImpact = fAnimScale * GetCurrentAnimTriggerTime(pOppFielder, 0x2C8DABFA, 0);
                fTimeToImpact = fTimeToImpact - fCurrentAnimTime;
                f32 fPickupDuration2 = LooseBallAnims::mAttackSTSInfo.mfPickupTime * LooseBallAnims::mAttackSTSInfo.mfAnimDuration;
                fTimeToImpact = fTimeToImpact - fPickupDuration2;

                mfWaitTime = fTimeToImpact;
                fTimeToImpact = clamp_lower(fTimeToImpact, 0.25f);

                mfTargetTime = fTimeToImpact;
                mpLooseBallInfo = &LooseBallAnims::mAttackSTSInfo;
                CleanGoalieAction();

                mPrevGoalieActionState = mGoalieActionState;
                mGoalieActionState = GOALIEACTION_STS_ATTACK_SETUP;
                mnSubstate = 0;
                mUrgency = URGENCY_LOW;
                ActionSTSAttackSetup(1.0f);

                return true;
            } while (false);
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x659C | 0x80049098 | size: 0x43C
 */
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

        float ballAbsXNew = (float)fabs(pBall->m_v3Position.f.x);
        float ballPosX = pBall->m_v3Position.f.x;

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

        float netSideSign = m_pTeam->m_pNet->m_sideSign;
        cBall* pBallVel = g_pBall;
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
 * TODO: 99.15% match - default-percent block has r3/r5 swapped, and interpolation reuses the milestone offset before NormalizeVal.
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
                float* pDefaultStartPercent = fDefaultStartPercent;
                fStartTime = pDefaultStartPercent[nMilestone] * (mpSaveData->mfMilestonePercent[2] * mpSaveData->mfDuration);
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
                for (int i = 0; i < 4; i++)
                {
                    SaveData* pData = mBlendInfo.mpSaveData[i];
                    if (pData != NULL)
                    {
                        fStartPercent[i] = pData->mfMilestonePercent[nMilestone];
                    }
                }
            }
            else
            {
                float* pDefaultStartPercent = fDefaultStartPercent;

                if (mBlendInfo.mpSaveData[0] != NULL)
                {
                    float fDefaultStart = pDefaultStartPercent[nMilestone];
                    fStartPercent[0] = fDefaultStart * mBlendInfo.mpSaveData[0]->mfMilestonePercent[2];
                }
                if (mBlendInfo.mpSaveData[1] != NULL)
                {
                    float fDefaultStart = pDefaultStartPercent[nMilestone];
                    fStartPercent[1] = fDefaultStart * mBlendInfo.mpSaveData[1]->mfMilestonePercent[2];
                }
                if (mBlendInfo.mpSaveData[2] != NULL)
                {
                    float fDefaultStart = pDefaultStartPercent[nMilestone];
                    fStartPercent[2] = fDefaultStart * mBlendInfo.mpSaveData[2]->mfMilestonePercent[2];
                }
                if (mBlendInfo.mpSaveData[3] != NULL)
                {
                    float fDefaultStart = pDefaultStartPercent[nMilestone];
                    fStartPercent[3] = fDefaultStart * mBlendInfo.mpSaveData[3]->mfMilestonePercent[2];
                }
            }
        }
        else if (fStartTime > 0.0f)
        {
            float fPrevMilestone = 0.0f;

            milestone = 0;
            while (milestone < 4 && fStartTime >= mBlendInfo.mfMilestoneTime[milestone])
            {
                if (mBlendInfo.mfMilestoneTime[milestone] > 0.0f)
                {
                    fPrevMilestone = mBlendInfo.mfMilestoneTime[milestone];
                }
                milestone++;
            }

            fStartTime = NormalizeVal(fStartTime, fPrevMilestone, mBlendInfo.mfMilestoneTime[milestone]);

            int prevMilestone = milestone - 1;

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

                    fStartPercent[i] = Interpolate(fStart, pData->mfMilestonePercent[milestone], fStartTime);
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

        cPoseNode* pNode1 = SetupBlender(true, fStartPercent, nMainAnimID, milestone);
        cPoseNode* pMainNode = pNode1;

        if (mBlendInfo.mfSaveBlendComposite >= 0.001f)
        {
            cPoseNode* pNode2 = SetupBlender(false, fStartPercent, nMainAnimID, milestone);
            cPN_SingleAxisBlender* pBlend = new (AllocateSingleAxisBlender()) cPN_SingleAxisBlender(2, NULL, 0, 0.1f);

            pBlend->m_fDesiredWeight = mBlendInfo.mfSaveBlendComposite;
            pBlend->m_fSmoothedWeight = mBlendInfo.mfSaveBlendComposite;
            pBlend->SetChild(0, pNode1);
            pBlend->SetChild(1, pNode2);

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
 * TODO: 95.33% match - remaining stack-slot and register-order differences in
 * loose-ball intercept, desperation, and pickup branches.
 */
void Goalie::InitActionLooseBallSetup()
{
    if (CheckForLooseBallShotInProgress())
    {
        return;
    }

    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
    if (!IsLooseBallClose(pSkillTweaks->fLooseBallChaseDistance))
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
        ActionMove(0.0f);
        return;
    }

    if (g_pBall->m_uGoalType != 6)
    {
        g_pBall->m_uGoalType = 4;
    }

    m_pPhysicsCharacter->m_CanCollideWithBall = true;
    mbDoHeadTrack = true;
    mbPickedUp = false;

    nlVector3 v3Velocity;
    nlVector3 v3BallPosition;
    cNet* pNet;
    bool bInCone = false;
    const nlVector3* pBallVelocity = &g_pBall->m_v3Velocity;

    v3BallPosition = g_pBall->m_v3Position;
    pNet = m_pTeam->m_pNet;
    muBallChangeCount = g_pBall->m_bBallPathChangeCount;
    muBallDeflectCount = g_pBall->m_bBallDeflectCount;

    if (g_pBall->m_pOwner != NULL)
    {
        bInCone = false;
    }
    else
    {
        v3Velocity = g_pBall->m_v3Velocity;
        f32 fVelMagSq = v3Velocity.f.x * v3Velocity.f.x + v3Velocity.f.y * v3Velocity.f.y + v3Velocity.f.z * v3Velocity.f.z;
        if (fVelMagSq < 0.01f)
        {
            bInCone = false;
        }
        else
        {
            nlVector3 v3Post0;
            nlVector3 v3Post1;
            m_pTeam->m_pNet->GetPostLocation(v3Post0, 0, 0.0f);
            m_pTeam->m_pNet->GetPostLocation(v3Post1, 1, 0.0f);

            nlVec3Add(v3Velocity, v3Velocity, g_pBall->m_v3Position);
            bInCone = IsPointInCone(v3Velocity, g_pBall->m_v3Position, v3Post0, v3Post1);
        }
    }

    f32 fBallSpeed = pBallVelocity->f.x * pBallVelocity->f.x + pBallVelocity->f.y * pBallVelocity->f.y
                   + pBallVelocity->f.z * pBallVelocity->f.z;

    if (fBallSpeed > 100.0f && bInCone)
    {
        f32 fAbsGoalieX = (f32)fabs(m_v3Position.f.x);
        f32 fAbsBallX = (f32)fabs(v3BallPosition.f.x);
        if (fAbsBallX < fAbsGoalieX - 1.5f)
        {
            SetDesiredSaveFacing(g_pBall->m_v3Position);

            u16 aDesiredAngle = m_aDesiredFacingDirection;
            const nlVector3* pSelfPos = &m_v3Position;
            nlVector4 v3Plane;
            MakePerpendicularPlane(*pSelfPos, aDesiredAngle, v3Plane, 0.2f);

            nlVector3 v3ContactVel;
            f32 fTimeTilSave = FakeBallWorld::GetPredictedPlaneIntersectTime(v3Plane, mv3TargetPosition, v3ContactVel);

            f32 fAbsTargetX = (f32)fabs(mv3TargetPosition.f.x);
            f32 fGoalLineX = cField::GetGoalLineX(1U);
            if (fAbsTargetX > fGoalLineX)
            {
                fTimeTilSave = -1.0f;
            }
            else if (fTimeTilSave > 0.0f)
            {
                GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, *pSelfPos, aDesiredAngle);
                GetLocalPoint(mv3LocalContactVelocity, v3ContactVel, v3Zero, aDesiredAngle);
            }

            if (fTimeTilSave > 0.0f && fTimeTilSave < 2.0f)
            {
                f32 fAbsTargetY = (f32)fabs(mv3TargetPosition.f.y);
                f32 fPenaltyBoxY = cField::GetPenaltyBoxY();
                if (fAbsTargetY > fPenaltyBoxY - 1.0f)
                {
                    muSaveType = 0x0000FFFC;
                }
                else
                {
                    muSaveType = 0x0000FFFF;
                }

                mbShouldMiss = false;
                u32 uSaveType = muSaveType;

                if (mbShouldMiss)
                {
                    fTimeTilSave = fTimeTilSave + ((GoalieTweaks*)m_pTweaks)->fSaveMissDelay;
                    mpSaveData = NULL;
                }
                else
                {
                    mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, fTimeTilSave, false, uSaveType, false);
                }

                if (mpSaveData != NULL)
                {
                    mbPlayMiss = false;
                }
                else
                {
                    if (!mbShouldMiss)
                    {
                        fTimeTilSave = -1.0f;
                        goto skip_save_adjust;
                    }
                    else
                    {
                        mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, 5.0f, true, uSaveType & 0x3FFF, false);
                        mbPlayMiss = true;
                    }
                }

                {
                    f32 fLocalX = mv3LocalContactPosition.f.x;
                    f32 fLocalVelX = mv3LocalContactVelocity.f.x;
                    f32 fRatio = (mpSaveData->mv3SavePos.f.x - fLocalX) / fLocalVelX;
                    f32 fLocalVelY = mv3LocalContactVelocity.f.y;
                    f32 fLocalY = mv3LocalContactPosition.f.y;
                    f32 fLocalVelZ = mv3LocalContactVelocity.f.z;
                    f32 fLocalZ = mv3LocalContactPosition.f.z;
                    mv3LocalContactPosition.f.x = fRatio * fLocalVelX + fLocalX;
                    mv3LocalContactPosition.f.y = fRatio * fLocalVelY + fLocalY;
                    fTimeTilSave = fTimeTilSave + fRatio;
                    mv3LocalContactPosition.f.z = fRatio * fLocalVelZ + fLocalZ;
                }

            skip_save_adjust:
                mfTimeTilSave = fTimeTilSave;

                if (mfTimeTilSave > 0.0f)
                {
                    mfWaitTime = mfTimeTilSave - mBlendInfo.mfMilestoneTime[2];
                    if (mfWaitTime <= 0.01f)
                    {
                        InitActionSave();
                        return;
                    }

                    if (ShouldReposition())
                    {
                        mv3NavTarget = mv3TargetPosition;
                        mMoveDirection = GOALIEDIR_IDLE;
                        CleanGoalieAction();
                        mPrevGoalieActionState = mGoalieActionState;
                        mGoalieActionState = GOALIEACTION_SAVE_REPOSITION;
                        mnSubstate = 0;
                        f32 fDy = m_v3Position.f.y - mv3NavTarget.f.y;
                        f32 fDx = m_v3Position.f.x - mv3NavTarget.f.x;
                        mfTargetDist = fDy * fDy + fDx * fDx;
                        mUrgency = URGENCY_HIGH;
                        f32 fBallDy = g_pBall->m_v3Position.f.y - m_v3Position.f.y;
                        f32 fBallDx = g_pBall->m_v3Position.f.x - m_v3Position.f.x;
                        m_aDesiredFacingDirection = (s16)(nlATan2f(fBallDy, fBallDx) * 10430.378f);
                        DoNavigation(0.0f, gfRepositionThreshold, NAVI_FACE_DESIRED);
                        if (mfWaitTime > 0.4f)
                        {
                            mUrgency = URGENCY_MED;
                        }
                        else
                        {
                            mUrgency = URGENCY_HIGH;
                        }
                        return;
                    }
                    else
                    {
                        CleanGoalieAction();
                        mPrevGoalieActionState = mGoalieActionState;
                        mGoalieActionState = GOALIEACTION_SAVE_SETUP;
                        mnSubstate = 0;
                        SetAnimState(10, true, 0.2f, false, false);
                        InitMovementFromAnimSeek(((GoalieTweaks*)m_pTweaks)->fSaveDirectionSeekSpeed, ((GoalieTweaks*)m_pTweaks)->fSaveDirectionSeekFalloff);
                        return;
                    }
                }
            }
        }
    }

    if (pBallVelocity->f.z < 3.0f && v3BallPosition.f.z < 1.5f)
    {
        if (bInCone && fBallSpeed > 0.25f)
        {
            f32 fAbsBallX2 = (f32)fabs(v3BallPosition.f.x);
            f32 fPenaltyBoxX = cField::GetPenaltyBoxX(1U);
            bool bInPenaltyArea = false;
            if (fAbsBallX2 > fPenaltyBoxX - 2.0f)
            {
                if (v3BallPosition.f.x * m_v3Position.f.x > 0.0f)
                {
                    f32 fAbsBallY = (f32)fabs(v3BallPosition.f.y);
                    f32 fPenaltyBoxY = cField::GetPenaltyBoxY();
                    if (fAbsBallY < fPenaltyBoxY - 1.0f)
                    {
                        bInPenaltyArea = true;
                    }
                    else
                    {
                        bInPenaltyArea = false;
                    }
                }
            }

            if (bInPenaltyArea)
            {
                mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(0);
                f32 fGoalLineX = cField::GetGoalLineX(1U);
                f32 fPanicLineX = fGoalLineX - 2.0f;

                f32 fPickupTime = mpLooseBallInfo->mfPickupTime;
                f32 fAnimDuration = mpLooseBallInfo->mfAnimDuration;
                f32 fAnimTime = fPickupTime * fAnimDuration;

                f32 fGVelX = pBallVelocity->f.x;
                f32 fGBallX = v3BallPosition.f.x;
                f32 fGVelY = pBallVelocity->f.y;
                f32 fGBallY = v3BallPosition.f.y;
                f32 fGVelZ = pBallVelocity->f.z;
                f32 fGBallZ = v3BallPosition.f.z;

                f32 fGuessBallX = 0.8f * fAnimTime * fGVelX + fGBallX;
                f32 fGuessBallY = 0.8f * fAnimTime * fGVelY + fGBallY;
                f32 fGuessBallZ = 0.8f * fAnimTime * fGVelZ + fGBallZ;

                nlVector3 v3GuessBallPos;
                v3GuessBallPos.f.x = fGuessBallX;
                v3GuessBallPos.f.y = fGuessBallY;

                f32 fAbsGuessX = (f32)fabs(fGuessBallX);
                v3GuessBallPos.f.z = fGuessBallZ;

                if (fAbsGuessX > fPanicLineX)
                {
                    CleanGoalieAction();
                    mPrevGoalieActionState = mGoalieActionState;
                    mGoalieActionState = GOALIEACTION_LOOSEBALL_DESPERATE;
                    mnSubstate = 0;

                    if ((f32)fabs(v3BallPosition.f.x) >= fPanicLineX)
                    {
                        mv3TargetPosition = v3BallPosition;
                    }
                    else
                    {
                        f32 fGoalLineX2 = cField::GetGoalLineX(1U);
                        if (fAbsGuessX < fGoalLineX2)
                        {
                            mv3TargetPosition = v3GuessBallPos;
                        }
                        else
                        {
                            if (pNet->m_baseLocation.f.x > 0.0f)
                            {
                                mv3TargetPosition.f.x = fPanicLineX;
                            }
                            else
                            {
                                mv3TargetPosition.f.x = -fPanicLineX;
                            }
                            f32 fGuessY = v3GuessBallPos.f.y;
                            f32 fBallPosY = v3BallPosition.f.y;
                            f32 fBallPosX = v3BallPosition.f.x;
                            f32 fTargetX = mv3TargetPosition.f.x;
                            f32 fDiffY = fBallPosY - fGuessY;
                            f32 fGuessX = v3GuessBallPos.f.x;
                            f32 fDiffXTarget = fBallPosX - fTargetX;
                            f32 fDiffXOrig = fBallPosX - fGuessX;
                            mv3TargetPosition.f.y = fBallPosY - fDiffXTarget * fDiffY / fDiffXOrig;
                        }
                    }

                    mv3TargetPosition.f.z = 0.0f;
                    mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(0);

                    f32 fPickupDist = mpLooseBallInfo->mfPickupDistance;
                    f32 fReachDist = fPickupDist + 0.75f;
                    f32 fDy = m_v3Position.f.y - mv3TargetPosition.f.y;
                    f32 fDx = m_v3Position.f.x - mv3TargetPosition.f.x;
                    f32 fDistSq = fDy * fDy + fDx * fDx;
                    f32 fReachDistSq = fReachDist * fReachDist;

                    if (fDistSq > fReachDistSq)
                    {
                        mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(1);

                        f32 fPickupDist1 = mpLooseBallInfo->mfPickupDistance;
                        f32 fReachDist1 = fPickupDist1 + 0.75f;
                        f32 fDy1 = m_v3Position.f.y - mv3TargetPosition.f.y;
                        f32 fDx1 = m_v3Position.f.x - mv3TargetPosition.f.x;
                        f32 fDistSq1 = fDy1 * fDy1 + fDx1 * fDx1;
                        f32 fReachDistSq1 = fReachDist1 * fReachDist1;

                        if (fDistSq1 < fReachDistSq1)
                        {
                            GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, m_v3Position, m_aActualFacingDirection);

                            f32 fAngleRatio;
                            if (mv3LocalContactPosition.f.x > 0.01f)
                            {
                                fAngleRatio = mv3LocalContactPosition.f.y / mv3LocalContactPosition.f.x;
                            }
                            else if (mv3LocalContactPosition.f.y < 0.0f)
                            {
                                fAngleRatio = -10.0f;
                            }
                            else
                            {
                                fAngleRatio = 10.0f;
                            }

                            if ((f32)fabs(fAngleRatio) > 1.5f)
                            {
                                if (fAngleRatio < 0.0f)
                                {
                                    mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(2);
                                }
                                else
                                {
                                    mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(3);
                                }
                            }
                        }
                    }

                    {
                        s32 nAnimID = mpLooseBallInfo->mnAnimID;
                        bool bShouldSetAnim = false;
                        if (nAnimID != m_eAnimID || (bShouldSetAnim = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
                        {
                            SetAnimState(nAnimID, true, 0.2f, false, false);
                        }
                        InitMovementFromAnim(0, v3Zero, 1.0f, false);
                        return;
                    }
                }
                else
                {
                    nlVector3 v3IntVel;
                    f32 fInterceptTime;
                    f32 fClosestDist;
                    bool bFound = FakeBallWorld::FindBallIntercept(m_v3Position, 1.0f, 6.0f, mv3TargetPosition, v3IntVel, fInterceptTime, fClosestDist, 3.0f);

                    if (bFound && mv3TargetPosition.f.z < 1.0f && fClosestDist < 0.75f)
                    {
                        CleanGoalieAction();
                        mPrevGoalieActionState = mGoalieActionState;
                        mGoalieActionState = GOALIEACTION_LOOSEBALL_DESPERATE;
                        mnSubstate = 0;

                        f32 fGoalLineX3 = cField::GetGoalLineX(1U);
                        f32 fGoalieTargetX = mv3TargetPosition.f.x;
                        f32 fAbsTargetX = (f32)fabs(fGoalieTargetX);
                        f32 fLimitX = fGoalLineX3 - 0.5f;

                        if (fAbsTargetX > fLimitX)
                        {
                            if ((f32)fabs(v3BallPosition.f.x) > fLimitX)
                            {
                                mv3TargetPosition = v3BallPosition;
                            }
                            else
                            {
                                f32 fBallPosY = v3BallPosition.f.y;
                                f32 fTargetY = mv3TargetPosition.f.y;
                                f32 fDiffY = fBallPosY - fTargetY;
                                f32 fTargetX = mv3TargetPosition.f.x;
                                f32 fBallPosX = v3BallPosition.f.x;
                                f32 fDiffLimitX = fGoalLineX3 - fLimitX;
                                f32 fDiffX = fBallPosX - fTargetX;
                                mv3TargetPosition.f.y = fBallPosY - fDiffLimitX * fDiffY / fDiffX;

                                if (pNet->m_baseLocation.f.x > 0.0f)
                                {
                                    mv3TargetPosition.f.x = fLimitX;
                                }
                                else
                                {
                                    mv3TargetPosition.f.x = -fLimitX;
                                }
                            }
                        }

                        mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(0);

                        f32 fPickupDist = mpLooseBallInfo->mfPickupDistance;
                        f32 fReachDist = fPickupDist + 0.75f;
                        f32 fDy = m_v3Position.f.y - mv3TargetPosition.f.y;
                        f32 fDx = m_v3Position.f.x - mv3TargetPosition.f.x;
                        f32 fDistSq = fDy * fDy + fDx * fDx;
                        f32 fReachDistSq = fReachDist * fReachDist;

                        if (fDistSq > fReachDistSq)
                        {
                            mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(1);

                            f32 fPickupDist1 = mpLooseBallInfo->mfPickupDistance;
                            f32 fReachDist1 = fPickupDist1 + 0.75f;
                            f32 fDy1 = m_v3Position.f.y - mv3TargetPosition.f.y;
                            f32 fDx1 = m_v3Position.f.x - mv3TargetPosition.f.x;
                            f32 fDistSq1 = fDy1 * fDy1 + fDx1 * fDx1;
                            f32 fReachDistSq1 = fReachDist1 * fReachDist1;

                            if (fDistSq1 < fReachDistSq1)
                            {
                                GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, m_v3Position, m_aActualFacingDirection);

                                f32 fAngleRatio;
                                if (mv3LocalContactPosition.f.x > 0.01f)
                                {
                                    fAngleRatio = mv3LocalContactPosition.f.y / mv3LocalContactPosition.f.x;
                                }
                                else if (mv3LocalContactPosition.f.y < 0.0f)
                                {
                                    fAngleRatio = -10.0f;
                                }
                                else
                                {
                                    fAngleRatio = 10.0f;
                                }

                                if ((f32)fabs(fAngleRatio) > 1.5f)
                                {
                                    if (fAngleRatio < 0.0f)
                                    {
                                        mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(2);
                                    }
                                    else
                                    {
                                        mpLooseBallInfo = LooseBallAnims::GetDesperationInfo(3);
                                    }
                                }
                            }
                        }

                        f32 fTargetTime = fInterceptTime - mpLooseBallInfo->mfPickupTime * mpLooseBallInfo->mfAnimDuration;
                        mfTargetTime = fTargetTime;

                        if (mfTargetTime < 0.02f)
                        {
                            s32 nAnimID = mpLooseBallInfo->mnAnimID;
                            bool bShouldSetAnim = false;
                            if (nAnimID != m_eAnimID || (bShouldSetAnim = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
                            {
                                SetAnimState(nAnimID, true, 0.2f, false, false);
                            }
                            InitMovementFromAnim(0, v3Zero, 1.0f, false);
                            return;
                        }
                        else
                        {
                            mv3NavTarget = mv3TargetPosition;
                            f32 fDyNav = v3BallPosition.f.y - m_v3Position.f.y;
                            f32 fDxNav = v3BallPosition.f.x - m_v3Position.f.x;
                            m_aDesiredFacingDirection = (s16)(nlATan2f(fDyNav, fDxNav) * 10430.378f);

                            if (mfTargetTime > 1.0f)
                            {
                                mUrgency = URGENCY_LOW;
                            }
                            else if (mfTargetTime > 0.5f)
                            {
                                mUrgency = URGENCY_MED;
                            }
                            else
                            {
                                mUrgency = URGENCY_HIGH;
                            }

                            DoNavigation(0.0f, 0.0f, NAVI_FOLLOW_TARGET);
                            return;
                        }
                    }
                }
            }
        }

        {
            bool bDoGrab = false;
            f32 fAbsBallX = (f32)fabs(v3BallPosition.f.x);
            f32 fPenaltyBoxX = cField::GetPenaltyBoxX(1U);
            f32 fPenaltyBoxX2 = cField::GetPenaltyBoxX(1U);
            f32 fGoalLineX = cField::GetGoalLineX(1U);
            f32 fMinKickLine = 0.35f * (fGoalLineX - fPenaltyBoxX2) + fPenaltyBoxX;

            if (!IsLooseBallClose(0.0f))
            {
                cFielder* pOpponent = GetClosestOpponentFielder(&v3BallPosition);

                f32 fOppDy = pOpponent->m_v3Position.f.y - v3BallPosition.f.y;
                f32 fOppDx = pOpponent->m_v3Position.f.x - v3BallPosition.f.x;
                f32 fGoalieDy = m_v3Position.f.y - v3BallPosition.f.y;
                f32 fGoalieDx = m_v3Position.f.x - v3BallPosition.f.x;
                f32 fOppDistSq = fOppDy * fOppDy + fOppDx * fOppDx;
                f32 fGoalieDistSq = fGoalieDy * fGoalieDy + fGoalieDx * fGoalieDx;

                if (fGoalieDistSq > fOppDistSq)
                {
                    if (mGoalieActionState == GOALIEACTION_MOVE)
                    {
                        return;
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
                    ActionMove(0.0f);
                    return;
                }
            }
            else
            {
                if ((f32)fabs(v3BallPosition.f.x) > fAbsBallX - 2.0f || fAbsBallX > fMinKickLine)
                {
                    bDoGrab = true;
                }
                else
                {
                    cPlayer* pPassTarget;
                    if (GetGlobalPad())
                    {
                        pPassTarget = DoFindBestPassTarget(false, false);
                    }
                    else
                    {
                        FuzzyVariant fvResult = Fuzzy::GetBestPassTarget(this);
                        if (fvResult.Confidence >= 0.5f)
                        {
                            pPassTarget = fvResult.mData.pPlayer;
                        }
                        else
                        {
                            pPassTarget = DoFindBestPassTarget(false, false);
                        }
                    }

                    if (pPassTarget != NULL)
                    {
                        f32 fBallDy = v3BallPosition.f.y - m_v3Position.f.y;
                        f32 fBallDx = v3BallPosition.f.x - m_v3Position.f.x;
                        f32 fTargetDy = pPassTarget->m_v3Position.f.y - m_v3Position.f.y;
                        f32 fTargetDx = pPassTarget->m_v3Position.f.x - m_v3Position.f.x;
                        f32 fBallDz = v3BallPosition.f.z - m_v3Position.f.z;
                        f32 fTargetDz = pPassTarget->m_v3Position.f.z - m_v3Position.f.z;
                        f32 fBallDistSq = fBallDy * fBallDy + fBallDx * fBallDx + fBallDz * fBallDz;
                        f32 fBallDist = nlSqrt(fBallDistSq, true);
                        f32 fInvDist = 1.0f / fBallDist;
                        f32 fNormDz = fInvDist * fBallDz;
                        f32 fNormDy = fInvDist * fBallDy;
                        f32 fNormDx = fInvDist * fBallDx;
                        f32 fTargetDistSq = fTargetDy * fTargetDy + fTargetDx * fTargetDx + fTargetDz * fTargetDz;
                        f32 fInvTargetDist = nlRecipSqrt(fTargetDistSq, true);
                        f32 fNormTargetDz = fInvTargetDist * fTargetDz;
                        f32 fNormTargetDy = fInvTargetDist * fTargetDy;
                        f32 fNormTargetDx = fInvTargetDist * fTargetDx;

                        if (fBallDist < 1.2f)
                        {
                            bDoGrab = true;
                        }
                        else
                        {
                            f32 fDotBallTarget = fNormDy * fNormTargetDy + fNormDx * fNormTargetDx + fNormDz * fNormTargetDz;
                            if (fDotBallTarget < 0.7071f)
                            {
                                bDoGrab = true;
                            }
                            else
                            {
                                f32 fDotRight = fNormDy * m_m4WorldMatrix.f.m12 + fNormDx * m_m4WorldMatrix.f.m11 + fNormDz * m_m4WorldMatrix.f.m13;
                                if (fDotRight < 0.0f)
                                {
                                    bDoGrab = true;
                                }
                                else
                                {
                                    cFielder* pOpp = GetClosestOpponentFielder(&v3BallPosition);
                                    f32 fOppDy = pOpp->m_v3Position.f.y - v3BallPosition.f.y;
                                    f32 fOppDx = pOpp->m_v3Position.f.x - v3BallPosition.f.x;
                                    f32 fOppDistSq = fOppDy * fOppDy + fOppDx * fOppDx;
                                    if (fOppDistSq < fBallDist * fBallDist)
                                    {
                                        bDoGrab = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (bDoGrab)
            {
                f32 fPredTime = LooseBallAnims::mpLooseBallInfo->mfPickupTime * LooseBallAnims::mpLooseBallInfo->mfAnimDuration;
                nlVector3 v3PredVel;
                FakeBallWorld::GetPredictedBallPosition(fPredTime, mv3TargetPosition, v3PredVel);

                GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, m_v3Position, m_aActualFacingDirection);

                nlVector3 v3CurLocalPos;
                GetLocalPoint(v3CurLocalPos, v3BallPosition, m_v3Position, m_aActualFacingDirection);

                bool bFacingBall = v3CurLocalPos.f.x >= 0.0f;
                mpLooseBallInfo = LooseBallAnims::FindLooseBallAnim(mv3LocalContactPosition, bFacingBall);
            }
            else
            {
                f32 fAbsGoalieX = (f32)fabs(m_v3Position.f.x);
                f32 fAbsBallXPos = (f32)fabs(v3BallPosition.f.x);
                f32 fAbsBallYDist = (f32)fabs(v3BallPosition.f.y - m_v3Position.f.y);
                f32 fDiffX = fAbsBallXPos - fAbsGoalieX;
                s16 nAngDiff = (s16)(nlATan2f(fAbsBallYDist, fDiffX) * 10430.378f);

                if (fAbsBallX > fMinKickLine && (u16)nAngDiff > 0x4E34)
                {
                    mpLooseBallInfo = &LooseBallAnims::mLooseBallKickInfo[0];
                }
                else if (v3BallPosition.f.y * v3BallPosition.f.x > 0.0f)
                {
                    mpLooseBallInfo = &LooseBallAnims::mLooseBallKickInfo[1];
                }
                else
                {
                    mpLooseBallInfo = &LooseBallAnims::mLooseBallKickInfo[2];
                }
            }

            {
                nlVector3 v3IntVel;
                f32 fInterceptTime;
                f32 fClosestDist;
                bool bFound = FakeBallWorld::FindBallIntercept(m_v3Position, 1.0f, 6.0f, mv3TargetPosition, v3IntVel, fInterceptTime, fClosestDist, 3.0f);

                if (bFound && mv3TargetPosition.f.z < 1.0f && fClosestDist < 0.4f)
                {
                    f32 fDy = mv3TargetPosition.f.y - m_v3Position.f.y;
                    f32 fDx = mv3TargetPosition.f.x - m_v3Position.f.x;
                    f32 fPickupDist = mpLooseBallInfo->mfPickupDistance;
                    f32 fReachDist = 0.4f + fPickupDist;
                    f32 fDistSq = fDy * fDy + fDx * fDx;
                    f32 fReachDistSq = fReachDist * fReachDist;
                    f32 fTargetTime = fInterceptTime - mpLooseBallInfo->mfPickupTime * mpLooseBallInfo->mfAnimDuration;

                    if (fDistSq <= fReachDistSq && fTargetTime < 0.02f)
                    {
                        f32 fDist = nlSqrt(fDistSq, true);

                        CleanGoalieAction();
                        mPrevGoalieActionState = mGoalieActionState;
                        mGoalieActionState = GOALIEACTION_LOOSEBALL_PICKUP;
                        mnSubstate = 0;
                        s32 nAnimID = mpLooseBallInfo->mnAnimID;
                        SetAnimState(nAnimID, true, 0.2f, false, false);
                        InitMovementFromAnim(0, v3Zero, 1.0f, false);
                        mMoveDirection = GOALIEDIR_IDLE;
                        mfTargetTime = 0.0f;
                        mfWaitTime = -1.0f;
                        mbPickedUp = false;

                        if (fDist >= mpLooseBallInfo->mfPickupDistance)
                        {
                            return;
                        }

                        f32 fRemaining = mpLooseBallInfo->mfPickupDistance - fDist;
                        f32 fPickupTime2 = mpLooseBallInfo->mfPickupTime;
                        mfTargetTime = fRemaining * fPickupTime2 / mpLooseBallInfo->mfPickupDistance;

                        m_pCurrentAnimController->m_fPrevTime = m_pCurrentAnimController->m_fTime;
                        m_pCurrentAnimController->m_fTime = mfTargetTime;
                        return;
                    }
                    else
                    {
                        mv3NavTarget = mv3TargetPosition;
                        if (mGoalieActionState != GOALIEACTION_LOOSEBALL_PURSUE_ROLLING)
                        {
                            CleanGoalieAction();
                            mPrevGoalieActionState = mGoalieActionState;
                            mGoalieActionState = GOALIEACTION_LOOSEBALL_PURSUE_ROLLING;
                            mnSubstate = 0;
                        }

                        f32 fDyNav = mv3TargetPosition.f.y - m_v3Position.f.y;
                        f32 fDxNav = mv3TargetPosition.f.x - m_v3Position.f.x;
                        m_aDesiredFacingDirection = (s16)(nlATan2f(fDyNav, fDxNav) * 10430.378f);

                        mv3NavTarget = mv3TargetPosition;
                        mUrgency = URGENCY_MED;
                        return;
                    }
                }
                else
                {
                    if (mGoalieActionState == GOALIEACTION_MOVE)
                    {
                        return;
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
                    ActionMove(0.0f);
                    return;
                }
            }
        }

        return;
    }

    {
        int nNumSolutions;
        f32 pSolutions[2];

        CalcInterceptXY(m_v3Position, 0.85f * ((GoalieTweaks*)m_pTweaks)->fRunningSpeed, 0.5f, v3BallPosition, *pBallVelocity, nNumSolutions, pSolutions);

        if (nNumSolutions > 0)
        {
            f32 fBestTime;
            if (nNumSolutions == 2)
            {
                fBestTime = (pSolutions[0] < pSolutions[1]) ? pSolutions[0] : pSolutions[1];
            }
            else
            {
                fBestTime = pSolutions[0];
            }

            if (fBestTime < 5.0f)
            {
                nlVector3 v3IntPos;
                nlVector3 v3IntVel;
                f32 fHeightTime = FakeBallWorld::GetPredictedHeightLimitTime(3.0f, fBestTime, v3IntPos, v3IntVel, false);

                if (fHeightTime >= 0.0f)
                {
                    f32 fIntX = v3IntPos.f.x;
                    f32 fAbsIntX = (f32)fabs(fIntX);
                    f32 fPenaltyBoxX = cField::GetPenaltyBoxX(1U);
                    bool bInPenaltyArea = false;
                    if (fAbsIntX > fPenaltyBoxX - 0.0f)
                    {
                        if (fIntX * m_v3Position.f.x > 0.0f)
                        {
                            f32 fAbsIntY = (f32)fabs(v3IntPos.f.y);
                            f32 fPenaltyBoxY = cField::GetPenaltyBoxY();
                            if (fAbsIntY < 0.0f + fPenaltyBoxY)
                            {
                                bInPenaltyArea = true;
                            }
                            else
                            {
                                bInPenaltyArea = false;
                            }
                        }
                    }

                    if (bInPenaltyArea)
                    {
                        CleanGoalieAction();
                        mPrevGoalieActionState = mGoalieActionState;
                        mGoalieActionState = GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING;
                        mnSubstate = 0;

                        mv3TargetPosition = v3IntPos;
                        mfTargetTime = fHeightTime;
                        mfTargetDist = 1.4f;

                        f32 fDy = v3IntPos.f.y - m_v3Position.f.y;
                        f32 fDx = v3IntPos.f.x - m_v3Position.f.x;
                        m_aDesiredFacingDirection = (s16)(nlATan2f(fDy, fDx) * 10430.378f);

                        f32 fDistSq = fDy * fDy + fDx * fDx;
                        s16 nAngDiff = m_aDesiredFacingDirection - m_aActualFacingDirection;
                        s32 nAnimID = m_eAnimID;

                        if (fDistSq < 1.0f)
                        {
                            mMoveDirection = GOALIEDIR_IDLE;
                            nAnimID = 0x08;
                        }
                        else
                        {
                            s16 nAbsAngDiff = nAngDiff < 0 ? -nAngDiff : nAngDiff;
                            mMoveDirection = GOALIEDIR_FORWARD;
                            u16 uAbsAngDiff = (u16)nAbsAngDiff;

                            if ((nAnimID == 0x24 || nAnimID == 0x25) && m_pCurrentAnimController->m_fTime < 0.92f)
                            {
                            }
                            else if (uAbsAngDiff <= 0x2AF8 || nAnimID == 0x26)
                            {
                                nAnimID = 0x26;
                            }
                            else if (nAngDiff > 0)
                            {
                                nAnimID = 0x25;
                            }
                            else
                            {
                                nAnimID = 0x24;
                            }
                        }

                        bool bShouldSetAnim = false;
                        if (nAnimID != m_eAnimID || (bShouldSetAnim = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
                        {
                            SetAnimState(nAnimID, true, 0.2f, false, false);
                        }
                        InitMovementFromAnimSeek(((GoalieTweaks*)m_pTweaks)->fRunningDirectionSeekSpeed, ((GoalieTweaks*)m_pTweaks)->fRunningDirectionSeekFalloff);
                        return;
                    }

                    if (mGoalieActionState == GOALIEACTION_MOVE)
                    {
                        return;
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
                    ActionMove(0.0f);
                    return;
                }
            }
        }

        CleanGoalieAction();
        mPrevGoalieActionState = mGoalieActionState;
        mGoalieActionState = GOALIEACTION_LOOSEBALL_SETUP;
        mnSubstate = 0;

        f32 fDyFace = v3BallPosition.f.y - m_v3Position.f.y;
        f32 fDxFace = v3BallPosition.f.x - m_v3Position.f.x;
        m_aDesiredFacingDirection = (s16)(nlATan2f(fDyFace, fDxFace) * 10430.378f);

        f32 fDyRun = v3BallPosition.f.y - m_v3Position.f.y;
        f32 fDxRun = v3BallPosition.f.x - m_v3Position.f.x;
        s16 nAngDiff = m_aDesiredFacingDirection - m_aActualFacingDirection;
        s32 nAnimID = m_eAnimID;
        f32 fDistSq = fDyRun * fDyRun + fDxRun * fDxRun;

        if (fDistSq < 1.0f)
        {
            mMoveDirection = GOALIEDIR_IDLE;
            nAnimID = 0x08;
        }
        else
        {
            s16 nAbsAngDiff = nAngDiff < 0 ? -nAngDiff : nAngDiff;
            mMoveDirection = GOALIEDIR_FORWARD;
            u16 uAbsAngDiff = (u16)nAbsAngDiff;

            if ((nAnimID == 0x24 || nAnimID == 0x25) && m_pCurrentAnimController->m_fTime < 0.92f)
            {
            }
            else if (uAbsAngDiff <= 0x2AF8 || nAnimID == 0x26)
            {
                nAnimID = 0x26;
            }
            else if (nAngDiff > 0)
            {
                nAnimID = 0x25;
            }
            else
            {
                nAnimID = 0x24;
            }
        }

        bool bShouldSetAnim = false;
        if (nAnimID != m_eAnimID || (bShouldSetAnim = (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)))
        {
            SetAnimState(nAnimID, true, 0.2f, false, false);
        }
        InitMovementFromAnimSeek(((GoalieTweaks*)m_pTweaks)->fRunningDirectionSeekSpeed, ((GoalieTweaks*)m_pTweaks)->fRunningDirectionSeekFalloff);
    }
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
 * TODO: 98.20% match - remaining f-register order differences in blend updates, milestone clamps, and reposition distance loads.
 */
void Goalie::InitActionSaveSetup(bool bCanReposition)
{
    if (mGoalieActionState == GOALIEACTION_STS || mGoalieActionState == GOALIEACTION_STS_RECOVER)
    {
        return;
    }

    if ((mGoalieActionState == GOALIEACTION_PURSUE_BALL_POUNCE || mGoalieActionState == GOALIEACTION_LOOSEBALL_PICKUP)
        && m_pCurrentAnimController->m_fTime > (0.6f * mpLooseBallInfo->mfPickupTime))
    {
        return;
    }

    if (mGoalieActionState == GOALIEACTION_SAVE)
    {
        return;
    }

    muBallDeflectCount = g_pBall->m_bBallDeflectCount;
    mbDoHeadTrack = true;
    mbBallImpacted = false;
    mnOffplayPending = GOALIE_OFFPLAY_NONE;

    CleanGoalieAction();

    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_SAVE_SETUP;
    mnSubstate = 0;

    m_pPhysicsCharacter->m_CanCollideWithBall = true;

    SetDesiredSaveFacing(g_pBall->m_v3Position);

    u16 desiredFacingDirection = m_aDesiredFacingDirection;
    const nlVector3& pPosition = m_v3Position;

    nlVector4 plane;
    MakePerpendicularPlane(pPosition, desiredFacingDirection, plane, 0.2f);

    nlVector3 localVelocity;
    float fTimeToContact = FakeBallWorld::GetPredictedPlaneIntersectTime(plane, mv3TargetPosition, localVelocity);
    double absX = __fabs(mv3TargetPosition.f.x);

    if ((float)absX > cField::GetGoalLineX(1U))
    {
        fTimeToContact = -1.0f;
    }
    else if (fTimeToContact > 0.0f)
    {
        GetLocalPoint(mv3LocalContactPosition, mv3TargetPosition, pPosition, desiredFacingDirection);
        GetLocalPoint(mv3LocalContactVelocity, localVelocity, v3Zero, desiredFacingDirection);
    }

    if (fTimeToContact > 0.0f)
    {
        bool bFromTakeoff = false;

        if (mPrevGoalieActionState == GOALIEACTION_PRE_CROUCH || mPrevGoalieActionState == GOALIEACTION_PURSUE_BALL_CARRIER)
        {
            bFromTakeoff = true;
        }

        if (mbShouldMiss)
        {
            if (g_pBall->m_unk_0xA3)
            {
                float dY = m_v3Position.f.y - g_pBall->m_v3ShotTarget.f.y;
                float dX = m_v3Position.f.x - g_pBall->m_v3ShotTarget.f.x;

                if ((dX * dX + dY * dY) > 6.25f)
                {
                    static FilteredRandomChance randgenStumble;
                    if (randgenStumble.genrand(((GoalieTweaks*)m_pTweaks)->fLobShotStumbleChance))
                    {
                        InitActionChipShotStumble();
                        return;
                    }
                }
            }
        }
        else if (mUrgency == URGENCY_HIGH)
        {
            bFromTakeoff = true;
        }

        float fTimeTilSave = fTimeToContact;
        u32 saveType = muSaveType;

        if (mbShouldMiss)
        {
            fTimeTilSave = fTimeToContact + ((GoalieTweaks*)m_pTweaks)->fSaveMissDelay;
            mpSaveData = NULL;
        }
        else
        {
            mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, fTimeToContact, false, saveType, bFromTakeoff);
        }

        if (mpSaveData != NULL)
        {
            mbPlayMiss = false;
            goto apply_blend_1;
        }

        if (!mbShouldMiss)
        {
            fTimeTilSave = -1.0f;
            goto set_time_1;
        }

        mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, 5.0f, true, saveType & 0xFFFC, false);
        mbPlayMiss = true;

    apply_blend_1:
    {
        float fLocalX = mv3LocalContactPosition.f.x;
        float fLocalVelX = mv3LocalContactVelocity.f.x;
        float fBlendFactor = (mpSaveData->mv3SavePos.f.x - fLocalX) / fLocalVelX;
        fTimeTilSave += fBlendFactor;

        nlVec3Set(mv3LocalContactPosition,
            fBlendFactor * fLocalVelX + fLocalX,
            fBlendFactor * mv3LocalContactVelocity.f.y + mv3LocalContactPosition.f.y,
            fBlendFactor * mv3LocalContactVelocity.f.z + mv3LocalContactPosition.f.z);
    }

    set_time_1:
        mfTimeTilSave = fTimeTilSave;

        if (mfTimeTilSave < 0.0f)
        {
            saveType = muSaveType;

            if (mbShouldMiss)
            {
                fTimeToContact += ((GoalieTweaks*)m_pTweaks)->fSaveMissDelay;
                mpSaveData = NULL;
            }
            else
            {
                mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, fTimeToContact, false, saveType, true);
            }

            if (mpSaveData != NULL)
            {
                mbPlayMiss = false;
            }
            else
            {
                mpSaveData = GoalieSave::FindBestSave(mBlendInfo, mv3LocalContactPosition, 5.0f, true, saveType & 0xFFFC, false);
                mbPlayMiss = true;
            }

            float fLocalX = mv3LocalContactPosition.f.x;
            float fLocalVelX = mv3LocalContactVelocity.f.x;
            float fBlendFactor = (mpSaveData->mv3SavePos.f.x - fLocalX) / fLocalVelX;

            nlVec3Set(mv3LocalContactPosition,
                fBlendFactor * fLocalVelX + fLocalX,
                fBlendFactor * mv3LocalContactVelocity.f.y + mv3LocalContactPosition.f.y,
                fBlendFactor * mv3LocalContactVelocity.f.z + mv3LocalContactPosition.f.z);

            fTimeToContact += fBlendFactor;
            mfTimeTilSave = fTimeToContact;
        }

        float fMilestone2 = mBlendInfo.mfMilestoneTime[2];
        if (mbShouldMiss)
        {
            if (bFromTakeoff)
            {
                mBlendInfo.mfStartTime = mBlendInfo.mfMilestoneTime[0];
            }
            else
            {
                mBlendInfo.mfStartTime = 0.0f;
            }
        }
        else if (bFromTakeoff)
        {
            if ((fMilestone2 - mBlendInfo.mfMilestoneTime[0]) <= mfTimeTilSave)
            {
                mBlendInfo.mfStartTime = mBlendInfo.mfMilestoneTime[0];
            }
            else
            {
                float diff = fMilestone2 - mfTimeTilSave;
                if (diff <= mBlendInfo.mfMilestoneTime[1])
                {
                    mBlendInfo.mfStartTime = diff;
                }
                else
                {
                    mBlendInfo.mfStartTime = mBlendInfo.mfMilestoneTime[1];
                }
            }
        }
        else
        {
            if (fMilestone2 <= mfTimeTilSave)
            {
                mBlendInfo.mfStartTime = 0.0f;
            }
            else
            {
                float diff = fMilestone2 - mfTimeTilSave;
                if (diff <= mBlendInfo.mfMilestoneTime[1])
                {
                    mBlendInfo.mfStartTime = diff;
                }
                else
                {
                    mBlendInfo.mfStartTime = mBlendInfo.mfMilestoneTime[1];
                }
            }
        }

        mfWaitTime = mBlendInfo.mfStartTime + (fTimeTilSave - fMilestone2);

        if (mfWaitTime <= 0.01f)
        {
            InitActionSave();
            return;
        }

        if (bCanReposition && ShouldReposition())
        {
            mMoveDirection = GOALIEDIR_IDLE;

            CleanGoalieAction();

            mPrevGoalieActionState = mGoalieActionState;
            mGoalieActionState = GOALIEACTION_SAVE_REPOSITION;
            mnSubstate = 0;

            float dY = m_v3Position.f.y - mv3NavTarget.f.y;
            float dX = m_v3Position.f.x - mv3NavTarget.f.x;
            mfTargetDist = dY * dY + dX * dX;

            mUrgency = URGENCY_HIGH;

            cBall* pBall = g_pBall;
            m_aDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(pBall->m_v3Position.f.y - m_v3Position.f.y, pBall->m_v3Position.f.x - m_v3Position.f.x));

            DoNavigation(0.0f, gfRepositionThreshold, NAVI_FACE_DESIRED);
            return;
        }

        SetAnimState(10, true, 0.2f, false, false);
        GoalieTweaks* pGoalieTweaks = (GoalieTweaks*)m_pTweaks;
        InitMovementFromAnimSeek(pGoalieTweaks->fSaveDirectionSeekSpeed, pGoalieTweaks->fSaveDirectionSeekFalloff);

        return;
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
 * TODO: 98.07% match - register allocation diffs remain in the ball-target
 * vector delta/scale block and final net-Y clamp temporary flow.
 */
void Goalie::InitActionChipShotStumble()
{
    CleanGoalieAction();
    mPrevGoalieActionState = mGoalieActionState;
    mGoalieActionState = GOALIEACTION_MISS_CHIP_SHOT;
    mnSubstate = 0;

    float dx = m_v3Position.f.x - g_pBall->m_v3Position.f.x;
    float dy = m_v3Position.f.y - g_pBall->m_v3Position.f.y;
    bool bFar = (dx * dx + dy * dy) > 42.25f;
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
        float distPlusOne = 1.5f + dist;
        float sx = dirX;
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
            pushX = 1.5f;
        else
            pushX = -1.5f;
        mv3NavTarget.f.x += pushX;
    }

    float clampedY = mv3NavTarget.f.y;
    float maxY = 0.5f * cNet::m_fNetWidth - 1.0f;
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
            f32 threshold = (f32)fabs(myX) - 0.5f;

            if ((f32)fabs(pOwnerVolatile->m_v3Position.f.x) > threshold || (f32)fabs(pBall->m_v3Position.f.x) > threshold)
            {
                f32 distThresh = 1.6899998f;

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
 * TODO: 97.99% match - r4/r5 pBall register swap, missing addi r3 hoisting,
 * cascading register diffs in distance sections, milestone else f0/f1 swap
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
                f32 dx = mBlendInfo.mv3BlendedSavePos.f.x - mv3LocalContactPosition.f.x;
                f32 dy = mBlendInfo.mv3BlendedSavePos.f.y - mv3LocalContactPosition.f.y;
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
                f32 dz = 0.0f;
                f32 dot = dy * g_pBall->m_v3Velocity.f.x + dx * g_pBall->m_v3Velocity.f.y + dz * g_pBall->m_v3Velocity.f.z;

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
                    f32 dx2 = mBlendInfo.mv3BlendedSavePos.f.x - mv3LocalContactPosition.f.x;
                    f32 dy2 = mBlendInfo.mv3BlendedSavePos.f.y - mv3LocalContactPosition.f.y;
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
            f32 milestone2 = mBlendInfo.mfMilestoneTime[2];
            if (milestone2 <= mfTimeTilSave)
            {
                mBlendInfo.mfStartTime = 0.0f;
                mfWaitTime = mfTimeTilSave - milestone2;
            }
            else
            {
                f32 diff = milestone2 - mfTimeTilSave;
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

static inline cPlayer* DoGoalieFindOpenPassTarget(Goalie* pGoalie)
{
    cPlayer* pPassTarget;
    if (pGoalie->GetGlobalPad() != NULL)
    {
        pPassTarget = pGoalie->DoFindBestPassTarget(false, false);
    }
    else
    {
        FuzzyVariant vBestPassTarget = Fuzzy::GetBestPassTarget(pGoalie);
        if (vBestPassTarget.Confidence >= 0.5f)
        {
            pPassTarget = vBestPassTarget.mData.pPlayer;
        }
        else
        {
            pPassTarget = pGoalie->DoFindBestPassTarget(false, false);
        }
    }

    return pPassTarget;
}

/**
 * Offset/Address/Size: 0xF30 | 0x80043A2C | size: 0x8A4
 */
void Goalie::DoPassRelease()
{
    extern float g_fSimulationTick;

    if (m_pBall == NULL && mGoalieActionState == GOALIEACTION_LOOSEBALL_PICKUP && m_eAnimID == 3)
    {
        nlVector3 v3AnimBallPos = GetJointPosition(m_nBallJointIndex);

        float dy = v3AnimBallPos.f.y - g_pBall->m_v3Position.f.y;
        float dx = v3AnimBallPos.f.x - g_pBall->m_v3Position.f.x;
        float dz = v3AnimBallPos.f.z - g_pBall->m_v3Position.f.z;
        float distanceSq = (dx * dx) + (dy * dy) + (dz * dz);

        if (distanceSq < 0.36f)
        {
            InitiatePickup();
        }
        else
        {
            return;
        }
    }

    if (m_pBall == NULL)
    {
        return;
    }

    if (mGoalieActionState == GOALIEACTION_LOOSEBALL_PICKUP)
    {
        if (m_eAnimID == 5 || m_eAnimID == 4)
        {
            mpPassTarget = NULL;
        }
        else
        {
            mpPassTarget = DoGoalieFindOpenPassTarget(this);

            if (mpPassTarget != NULL)
            {
                float dy = mpPassTarget->m_v3Position.f.y - m_v3Position.f.y;
                float dx = mpPassTarget->m_v3Position.f.x - m_v3Position.f.x;
                u16 aTarget = (u16)(s32)(10430.378f * nlATan2f(dy, dx));

                u16 aFacing = m_aActualFacingDirection;
                if (m_v3Position.f.x < 0.0f)
                {
                    aFacing += 0x8000;
                }

                if (aFacing < 0x5550)
                {
                    aFacing = 0x5550;
                }
                else if (aFacing > 0xAAB0)
                {
                    aFacing = (u16)-0x5550;
                }

                if (m_v3Position.f.x < 0.0f)
                {
                    aFacing += 0x8000;
                }

                s16 aDiff = (s16)(aTarget - aFacing);
                int aAbsDiff = aDiff;
                if (aDiff < 0)
                {
                    aAbsDiff = -aDiff;
                }

                bool bValidPassTarget;
                if ((u16)aAbsDiff > 0x2AA8)
                {
                    bValidPassTarget = false;
                }
                else if ((float)fabs(mpPassTarget->m_v3Position.f.x) > (float)fabs(m_v3Position.f.x)
                         && (float)fabs(mpPassTarget->m_v3Position.f.y) < cField::GetPenaltyBoxY())
                {
                    bValidPassTarget = false;
                }
                else
                {
                    bValidPassTarget = true;
                }

                if (!bValidPassTarget)
                {
                    mpPassTarget = NULL;
                }
            }
        }
    }
    else if (mGoalieActionState == GOALIEACTION_STS)
    {
        if (mpSaveData != NULL)
        {
            int nBallJoint = m_nBallJointIndex;
            const nlVector3* pPrevBallPos = &GetPrevJointPosition(nBallJoint);
            const nlVector3* pCurBallPos = &GetJointPosition(nBallJoint);

            float invTick = 1.0f / g_fSimulationTick;
            nlVector3 v3BallVel;
            nlVec3Sub(v3BallVel, *pCurBallPos, *pPrevBallPos);
            nlVec3Scale(v3BallVel, invTick);

            ReleaseBall();
            g_pBall->SetVelocity(v3BallVel, SPINTYPE_NONE, NULL);
            SetNoPickUpTime(2.0f);
            return;
        }

        if (mpShooter != NULL)
        {
            if (m_pBall == NULL)
                return;

            nlVector3 v3BallVel;
            nlVector3 v3BallTarget = mpShooter->m_v3Position;

            float dy = m_v3Position.f.y - v3BallTarget.f.y;
            float dx = m_v3Position.f.x - v3BallTarget.f.x;
            float fDistance = nlSqrt((dx * dx) + (dy * dy), true);
            float fShotSpeed = InterpolateRangeClamped(22.0f, 28.0f, 5.0f, 15.0f, fDistance);
            v3BallTarget.f.z += 0.5f;

            ReleaseBall();
            g_pBall->ShootAtFast(v3BallVel, v3BallTarget, fDistance / fShotSpeed);
            g_pBall->ShootRelease(v3BallVel, SPINTYPE_FORWARD);

            g_pBall->m_unk_0xA6 = true;
            g_pBall->m_tNoPickupTimer.SetSeconds(0.2f);
            g_pBall->mpDamageTarget = mpShooter;

            EmitBallShot(mpShooter, BALL_EFFECT_S2S_SHOT, NULL, false);
            g_pBall->InitiateBallBlur(BALL_EFFECT_S2S_SHOT, mpShooter);

            Audio::SoundAttributes sndAtr;
            sndAtr.Init();
            sndAtr.SetSoundType(0xBD, true);
            sndAtr.UsePhysObj(g_pBall->m_pPhysicsBall);
            Audio::gStadGenSFX.Play(sndAtr);

            SetNoPickUpTime(0.08f);
            Audio::FadeFilterFromCurrentToZero();
            FixedUpdateTask::mTimeScale = 1.0f;
            ParticleUpdateTask::SetTimeScale(1.0f);
            return;
        }
    }

    eSpinType spinType;
    bool bIsKick = false;
    if (m_eAnimID < 6 && m_eAnimID >= 2)
    {
        bIsKick = true;
        Event* pEvent = g_pEventManager->CreateValidEvent(0x10, 0x38);
        GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();
        pSaveData->saveType = g_pBall->m_uGoalType;
        pSaveData->pShooter = g_pBall->m_pShooter;
        pSaveData->pGoalie = this;
    }

    if (mpPassTarget != NULL)
    {
        float fIsOpen = OpenTo(this, mpPassTarget);

        if ((bIsKick
                && (m_v3Position.f.x - mpPassTarget->m_v3Position.f.x) * (m_v3Position.f.x - mpPassTarget->m_v3Position.f.x)
                           + (m_v3Position.f.y - mpPassTarget->m_v3Position.f.y) * (m_v3Position.f.y - mpPassTarget->m_v3Position.f.y)
                       > 64.0f)
            || (fIsOpen < 0.85f && m_eAnimID != 1))
        {
            DoRegularPassing(mpPassTarget, true, true, false, false);
        }
        else
        {
            DoRegularPassing(mpPassTarget, false, true, false, false);
        }

        if (bIsKick)
        {
            EmitBallPass(this);
        }
        return;
    }

    GoalieTweaks* pTweaks = (GoalieTweaks*)m_pTweaks;

    if (nlRandom(2, &nlDefaultSeed) != 0)
    {
        spinType = SPINTYPE_FORWARD;
    }
    else
    {
        spinType = SPINTYPE_BACK;
    }

    float fPercent = nlRandomf(1.0f, &nlDefaultSeed);
    float fShotSpeed = Interpolate(pTweaks->fKickVelocityMin, pTweaks->fKickVelocityMax, fPercent);
    float fShotAng = Interpolate(pTweaks->fKickAngleMin, pTweaks->fKickAngleMax, fPercent);

    nlVector3 v3Desired = v3Zero;
    float fYPos = cField::GetPenaltyBoxY() + nlRandomf(1.0f, &nlDefaultSeed);
    if (m_v3Position.f.y > 0.0f)
    {
        v3Desired.f.y = fYPos;
    }
    else
    {
        v3Desired.f.y = -fYPos;
    }

    float dy = v3Desired.f.y - m_v3Position.f.y;
    float dx = v3Desired.f.x - m_v3Position.f.x;
    u16 aDesired = (u16)(s32)(10430.378f * nlATan2f(dy, dx));

    s16 aDiff = (s16)(aDesired - m_aActualFacingDirection);
    s16 aAbsDiff = (aDiff < 0) ? -aDiff : aDiff;
    if ((u16)aAbsDiff > 0x1554)
    {
        if (aDiff > 0)
        {
            aDesired = m_aActualFacingDirection + 0x1554;
        }
        else
        {
            aDesired = m_aActualFacingDirection - 0x1554;
        }
    }

    if (m_v3Position.f.x < 0.0f)
    {
        aDesired += 0x8000;
    }

    if (aDesired < 0x5550)
    {
        aDesired = 0x5550;
    }
    else if (aDesired > 0xAAB0)
    {
        aDesired = (u16)-0x5550;
    }

    if (m_v3Position.f.x < 0.0f)
    {
        aDesired += 0x8000;
    }

    float fSin;
    float fCos;
    nlSinCos(&fSin, &fCos, aDesired);
    float fBaseCos = fCos;
    float fBaseSin = fSin;

    u16 aShot = (u16)(s32)(10430.378f * ((3.1415927f * fShotAng) / 180.0f));
    nlSinCos(&fSin, &fCos, aShot);

    float fXYMag = fCos * fShotSpeed;
    nlVector3 v3Velocity;
    v3Velocity.f.x = fBaseCos * fXYMag;
    v3Velocity.f.y = fBaseSin * fXYMag;
    v3Velocity.f.z = fSin * fShotSpeed;

    ReleaseBall();
    g_pBall->ShootRelease(v3Velocity, spinType);
    SetNoPickUpTime(0.25f);
    g_pBall->m_tNoPickupTimer.SetSeconds(0.15f);

    if (bIsKick)
    {
        EmitBallImpact(this, false);
    }
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
 * TODO: 96.53% match - callee-saved float register swap (f27/f29) for fTfY and fBtgX in normalize section
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

    nlVector3 v3Facing;
    nlVector3 v3G2Ball;
    nlVector3 v3G2Post1;
    nlVector3 v3G2Post2;

    nlVec3Sub(v3G2Ball, v3BallPosition, m_v3Position);

    m_pTeam->m_pNet->GetPostLocation(v3G2Post1, 0, 0.5f);
    m_pTeam->m_pNet->GetPostLocation(v3G2Post2, 1, 0.5f);

    nlVec3Sub(v3G2Post1, v3G2Post1, m_v3Position);
    nlVec3Sub(v3G2Post2, v3G2Post2, m_v3Position);

    float fLeftDot = nlVec3DotProduct(v3G2Ball, v3G2Post1);
    float fRightDot = nlVec3DotProduct(v3G2Ball, v3G2Post2);

    if ((fLeftDot > 0.0f) || (fRightDot > 0.0f))
    {
        if (fLeftDot > fRightDot)
        {
            v3Facing.f.x = v3G2Post1.f.y;
            v3Facing.f.y = -v3G2Post1.f.x;
            v3Facing.f.z = 0.0f;

            if (nlVec3DotProduct(v3Facing, v3G2Post2) > 0.0f)
            {
                v3Facing.f.x = -1.0f * v3Facing.f.x;
                v3Facing.f.y = -1.0f * v3Facing.f.y;
                v3Facing.f.z = -1.0f * v3Facing.f.z;
            }
        }
        else
        {
            v3Facing.f.x = v3G2Post2.f.y;
            v3Facing.f.y = -v3G2Post2.f.x;
            v3Facing.f.z = 0.0f;

            if (nlVec3DotProduct(v3Facing, v3G2Post1) > 0.0f)
            {
                v3Facing.f.x = -1.0f * v3Facing.f.x;
                v3Facing.f.y = -1.0f * v3Facing.f.y;
                v3Facing.f.z = -1.0f * v3Facing.f.z;
            }
        }
    }
    else
    {
        v3Facing = v3G2Ball;
    }

    float fBallOffMagSq = nlVec3DotProduct(v3G2Ball, v3G2Ball);

    if (fBallOffMagSq < 1.44f)
    {
        nlVector3 v3BallToGoal;
        float fLengthSq = nlVec3LengthSquared(v3Facing);
        nlVec3Sub(v3BallToGoal, v3BallPosition, m_pTeam->m_pNet->m_baseLocation);

        float fRecip = nlRecipSqrt(fLengthSq, true);
        nlVec3Scale(v3Facing, fRecip);

        float fRecip2 = nlRecipSqrt(nlVec3LengthSquared(v3BallToGoal), true);
        nlVec3Scale(v3BallToGoal, fRecip2);

        nlVec3WeightedSum(v3Facing, 0.5f, v3Facing, 0.5f, v3BallToGoal);
    }

    m_aDesiredFacingDirection = (s16)(nlATan2f(v3Facing.f.y, v3Facing.f.x) * (32768.0f / 3.14159265f));
}

/**
 * Offset/Address/Size: 0x3C4 | 0x80042EC0 | size: 0x1F4
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

    s16 aDiff = (s16)((u16)(s32)(10430.378f * fAngleToTarget)
                      - (u16)(s32)(10430.378f * nlATan2f(v3FutureBallPos.f.y - m_v3Position.f.y, v3FutureBallPos.f.x - m_v3Position.f.x)));
    s32 iRatio = (s32)(1024.0f * fRatio);
    s32 iTurn = (iRatio * aDiff) / 1024;
    SetFacingDirection((u16)(iTurn + m_aActualFacingDirection));

    nlVector3 v3Velocity;
    float fZero = 0.0f;
    v3Velocity.f.z = fRatio * fZero;
    v3Velocity.f.x = fRatio * fDeltaX;
    v3Velocity.f.y = fRatio * fDeltaY;

    v3Velocity.f.x = v3Velocity.f.x >= -0.12f ? v3Velocity.f.x : -0.12f;
    v3Velocity.f.x = v3Velocity.f.x <= 0.12f ? v3Velocity.f.x : 0.12f;
    v3Velocity.f.y = v3Velocity.f.y >= -0.12f ? v3Velocity.f.y : -0.12f;
    v3Velocity.f.y = v3Velocity.f.y <= 0.12f ? v3Velocity.f.y : 0.12f;

    nlVec3Set(v3FuturePos, v3Velocity.f.x + m_v3Position.f.x, v3Velocity.f.y + m_v3Position.f.y, v3Velocity.f.z + m_v3Position.f.z);

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

    float yRand = nlRandomf(5.0f, &nlDefaultSeed);
    if ((u32)nlRandom(100, &nlDefaultSeed) > 50)
    {
        yRand *= -1.0f;
    }
    v3BallVel.f.y += yRand;

    float zRand = nlRandomf(2.0f, &nlDefaultSeed);
    v3BallVel.f.z = 4.0f + zRand;

    g_pBall->SetVelocity(v3BallVel, SPINTYPE_FORWARD, NULL);
    g_pBall->m_tNoPickupTimer.SetSeconds(0.12f);
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
