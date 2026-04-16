#include "Game/AI/Fielder.h"
#include "Game/AI/FielderActions.h"
#include "Game/AI/AIPlay.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/AvoidController.h"
#include "Game/AI/FielderDesires.h"
#include "Game/AI/Fuzzy.h"
#include "Game/AI/ShotMeter.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/AnimInventory.h"
#include "Game/Ball.h"
#include "Game/BasicStadium.h"
#include "Game/CharacterTriggers.h"
#include "Game/FixedUpdateTask.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/Render/Bowser.h"
#include "Game/Render/ChainChomp.h"
#include "Game/DB/StatsTracker.h"
#include "Game/RumbleActions.h"
#include "Game/SAnim/pnBlender.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim.h"
#include "Game/Sys/eventman.h"
#include "Game/Net.h"
#include "NL/nlSlotPool.h"
#include "math.h"

static f32 CANT_COLLIDE = *(f32*)__float_max;

namespace Audio
{
enum eWorldSFX
{
    WORLDSFX_DUMMY = 0,
};

class cWorldSFX : public cGameSFX
{
public:
    unsigned long Play(Audio::SoundAttributes&);
    void Stop(eWorldSFX, cGameSFX::StopFlag);
};

extern cWorldSFX gCrowdSFX;
extern cWorldSFX gStadGenSFX;
} // namespace Audio

extern bool g_e3_Build;
void FireCameraRumbleFilter(float, float);
class FakeBallWorld
{
public:
    static void ResetBallIterator();
    static void GetNextBallPosition(nlVector3&);
};
extern float g_fSimulationTick;

class Fuzzy
{
public:
    static FuzzyVariant ShouldIStrafeBall(cFielder*);
    static FuzzyVariant ShouldIStrafeMark(cFielder*);
    static FuzzyVariant GetBestHitTarget(cFielder*);
};

const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

static const float fBallAwayFromCarrierMinTime = 0.0f; // @4959
static const float fBallAwayFromCarrierMaxTime = 1.0f; // @4960

static LooseBallContactAnimInfo gOneTimerIdleGroundContactAnims[4];
static LooseBallContactAnimInfo gOneTimerIdleVolleyContactAnims[4];
static LooseBallContactAnimInfo gOneTimerLeadGroundContactAnims[4];
static LooseBallContactAnimInfo gOneTimerLeadVolleyContactAnims[4];

// Static arrays for GetReceivePassBallContactAnimInfo
static LooseBallContactAnimInfo IdleGroundContactAnims[1] = {
    { 0x37, 3.0f, 0xFFFF, 0xFFFF }
};

static LooseBallContactAnimInfo IdleVolleyContactAnims[1] = {
    { 0x38, 9.0f, 0xFFFF, 0xFFFF }
};

static LooseBallContactAnimInfo LeadGroundContactAnims[4] = {
    { 0x39, 7.0f, 0xE000, 0x2000 },
    { 0x3A, 7.0f, 0xA000, 0xE000 },
    { 0x3C, 7.0f, 0x6000, 0xA000 },
    { 0x3B, 7.0f, 0x2000, 0x6000 }
};

static LooseBallContactAnimInfo LeadVolleyContactAnims[3] = {
    { 0x3D, 10.0f, 0xE000, 0x2000 },
    { 0x3F, 9.0f, 0x2000, 0x8000 },
    { 0x3E, 9.0f, 0x8000, 0xE000 }
};

// static inline bool IsTurboingAnimID(s32 animID)
// {
//     bool result = false;
//     if (animID == 0x1D || animID == 0x1E || animID == 0x1F || animID == 0x20 || animID == 0x21 || animID == 0x22)
//     {
//         result = true;
//     }
//     return result;
// }

// static inline bool IsTurboingWithoutBallAnimID(s32 animID)
// {
//     bool result = false;
//     if (animID == 0x0F || animID == 0x10 || animID == 0x11)
//     {
//         result = true;
//     }
//     return result;
// }

/**
 * Offset/Address/Size: 0x0 | 0x80026B20 | size: 0x8
 */
u32 PlayerAttackData::GetID()
{
    return 0x19a;
}

// /**
//  * Offset/Address/Size: 0x8 | 0x80026B18 | size: 0x8
//  */
// void cNet::GetNetWidth()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80026B10 | size: 0x8
//  */
// void cNet::GetNetHeight()
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x80026AD4 | size: 0x3C
//  */
// FilteredRandomReal::~FilteredRandomReal()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80026A98 | size: 0x3C
//  */
// FilteredRandomChance::~FilteredRandomChance()
// {
// }

// /**
//  * Offset/Address/Size: 0xD2E8 | 0x80026624 | size: 0x474
//  */
// cFielder::cFielder(int, int, eCharacterClass, const int*, cSHierarchy*, cAnimInventory*, const CharacterPhysicsData*, FielderTweaks*, AnimRetargetList*)
// {
// }

/**
 * Offset/Address/Size: 0xD264 | 0x800265A0 | size: 0x84
 */
cFielder::~cFielder()
{
    CleanUpAction();
    delete m_pAvoidance;
    delete m_pShotMeter;
    delete m_pCurrentPlay;
}

/**
 * Offset/Address/Size: 0xD240 | 0x8002657C | size: 0x24
 */
void cFielder::AbortPlay()
{
    m_pCurrentPlay->ClearPlay();
}

/**
 * Offset/Address/Size: 0xD1D4 | 0x80026510 | size: 0x6C
 */
void cFielder::AbortPendingThoughts()
{
    g_pGame->AbortPendingThought(mThoughtHashCalcDesire);
    g_pGame->AbortPendingThought(mThoughtHashInitRunToNet);
    g_pGame->AbortPendingThought(mThoughtHashInitGetOpen);
    g_pGame->AbortPendingThought(mThoughtHashInitWindupPass);
    g_pGame->AbortPendingThought(mThoughtHashInitCutAndBreak);
    ClearQueuedDesire();
}

/**
 * Offset/Address/Size: 0xC890 | 0x80025BCC | size: 0x944
 */
void cFielder::CalculateNewDesire()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0xC814 | 0x80025B50 | size: 0x7C
 */
bool cFielder::CanDoCaptainShootToScore()
{
    if (nlSingleton<GameInfoManager>::Instance()->GetGameplayOptions().Shoot2Score
        && (IsCaptain() || nlSingleton<GameInfoManager>::Instance()->GetTeam(g_pBall->m_pOwner->m_pTeam->m_nSide) == TEAM_MYSTERY))
    {
        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0xC648 | 0x80025984 | size: 0x1CC
 * TODO: 99.91% match - remaining contact-frame source register in the first
 * ratio divide (`lfs f1` target vs `lfs f9` current).
 */
bool cFielder::CanLooseBallShoot()
{
    if ((g_pBall->m_pOwner == NULL)
        && (g_pBall->m_pPassTarget == NULL)
        && (g_pBall->m_tShotTimer.m_uPackedTime == 0)
        && (g_pBall->m_unk_0xA6 == 0))
    {
        u32 nNumKeys = m_pAnimInventory->GetAnim(gOneTimerLeadGroundContactAnims[0].nAnimID)->m_nNumKeys;
        const cBall* pBall = g_pBall;
        float ratio = gOneTimerLeadGroundContactAnims[0].fAnimContactFrame;
        ratio /= (float)nNumKeys;
        float frames = (float)nNumKeys / 30.0f;
        float contactTime = ratio * frames;

        nlVector3 v3PredictedPos;
        nlVec3Set(v3PredictedPos,
            (contactTime * pBall->m_v3Velocity.f.x) + pBall->m_v3Position.f.x,
            (contactTime * pBall->m_v3Velocity.f.y) + pBall->m_v3Position.f.y,
            (contactTime * pBall->m_v3Velocity.f.z) + pBall->m_v3Position.f.z);

        s16 facingDelta = GetFacingDeltaToPosition(v3PredictedPos);
        u16 uFacingDelta = (facingDelta < 0) ? -facingDelta : facingDelta;

        float fInterpolatedValue = InterpolateRangeClamped(1.75f, 2.75f, 32768.0f, 16384.0f, uFacingDelta);

        if (v3PredictedPos.f.z < 1.0f)
        {
            float fDeltaX = v3PredictedPos.f.x - m_v3Position.f.x;
            float fDeltaY = v3PredictedPos.f.y - m_v3Position.f.y;
            float fDistanceSquared = fDeltaX * fDeltaX + fDeltaY * fDeltaY;

            if (fDistanceSquared < fInterpolatedValue * fInterpolatedValue)
            {
                float fMyInterceptAbility = AbleToInterceptBall(this);
                float fOtherInterceptAbility = AbleToInterceptBall(m_pTeam->GetOtherTeam()->GetStriker());

                if (((float)fabs(fOtherInterceptAbility - fMyInterceptAbility) > 0.03f) && (fMyInterceptAbility > fOtherInterceptAbility))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0xC468 | 0x800257A4 | size: 0x1E0
 * TODO: 99.92% match - remaining contact-frame source register in the first
 * ratio divide (`lfs f1` target vs `lfs f9` current).
 */
bool cFielder::CanLooseBallPass()
{
    if ((g_pBall->m_pOwner == NULL)
        && (g_pBall->m_pPassTarget == NULL)
        && (g_pBall->m_tShotTimer.m_uPackedTime == 0)
        && (g_pBall->m_unk_0xA6 == 0))
    {
        u32 nNumKeys = m_pAnimInventory->GetAnim(gOneTimerLeadGroundContactAnims[0].nAnimID)->m_nNumKeys;
        const cBall* pBall = g_pBall;
        float ratio = gOneTimerLeadGroundContactAnims[0].fAnimContactFrame;
        ratio /= (float)nNumKeys;
        float frames = (float)nNumKeys / 30.0f;
        float contactTime = ratio * frames;

        nlVector3 v3PredictedPos;
        nlVec3Set(v3PredictedPos,
            (contactTime * pBall->m_v3Velocity.f.x) + pBall->m_v3Position.f.x,
            (contactTime * pBall->m_v3Velocity.f.y) + pBall->m_v3Position.f.y,
            (contactTime * pBall->m_v3Velocity.f.z) + pBall->m_v3Position.f.z);

        s16 facingDelta = GetFacingDeltaToPosition(v3PredictedPos);
        u16 uFacingDelta = (facingDelta < 0) ? -facingDelta : facingDelta;

        float fLooseBallRadius = InterpolateRangeClamped(1.75f, 2.75f, 32768.0f, 16384.0f, uFacingDelta);

        if (g_pBall->m_v3Velocity.f.z < 0.0f)
        {
            if (v3PredictedPos.f.z < 1.0f)
            {
                float fDeltaX = v3PredictedPos.f.x - m_v3Position.f.x;
                float fDeltaY = v3PredictedPos.f.y - m_v3Position.f.y;
                float fDistanceSquared = fDeltaX * fDeltaX + fDeltaY * fDeltaY;

                if (fDistanceSquared < fLooseBallRadius * fLooseBallRadius)
                {
                    float fMyScore = AbleToInterceptBall(this);
                    float fHisScore = AbleToInterceptBall(m_pTeam->GetOtherTeam()->GetStriker());

                    if (((float)fabs(fHisScore - fMyScore) > 0.03f) && (fMyScore > fHisScore))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0xC410 | 0x8002574C | size: 0x58
 */
bool cFielder::CanReceivePass()
{
    bool bCanReceivePass = false;
    if (!IsFallenDown(0.0f) && (m_tFrozenTimer.m_uPackedTime == 0))
    {
        bCanReceivePass = true;
    }

    return bCanReceivePass;
}

/**
 * Offset/Address/Size: 0xC3D8 | 0x80025714 | size: 0x38
 */
void cFielder::SetMark(cFielder* pMark)
{
    if (m_pMark != nullptr && m_pMark->m_pMarker == this)
    {
        m_pMark->m_pMarker = nullptr;
    }

    m_pMark = pMark;

    if (m_pMark != nullptr)
    {
        pMark->m_pMarker = this;
    }
}

/**
 * Offset/Address/Size: 0xAEBC | 0x800241F8 | size: 0x151C
 */
void cFielder::CollideWithCharacterCallback(CollisionPlayerPlayerData*)
{
}

/**
 * Offset/Address/Size: 0xAD6C | 0x800240A8 | size: 0x150
 */
bool cFielder::CollideWithShellCallback(ePowerupSize eSize, bool bUnknown, const nlVector3& rv3Pos1, const nlVector3& rv3Pos2)
{
    // Check if player is fallen down
    if (!IsFallenDown(0.0f) && (m_eActionState != ACTION_POST_WHISTLE))
    {
        bool bShouldSkip = false;
        if ((m_ePowerup == POWER_UP_STAR && m_tPowerupEffectTime.m_uPackedTime != 0) || (mActionShootToScoreVars.isCurrentlyInvincible != 0))
        {
            bShouldSkip = true;
        }

        if (!bShouldSkip)
        {
            if (eSize == POWERUPSIZE_LARGE)
            {
                InitActionSquishReact(rv3Pos2);
            }
            else
            {
                InitActionShellReact(rv3Pos1, rv3Pos2);

                ePowerupSize eSoundSize = eSize;
                if (bUnknown)
                {
                    eSoundSize = POWERUPSIZE_LARGE;
                }

                switch (eSoundSize)
                {
                case POWERUPSIZE_SMALL:
                    PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk254);
                    break;
                case POWERUPSIZE_MEDIUM:
                    PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk258);
                    break;
                case POWERUPSIZE_LARGE:
                    PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk24C);
                    break;
                }
            }

            // Clear pass target if this fielder is the current pass target
            if (g_pBall->m_pPassTarget != nullptr && g_pBall->m_pPassTarget == this)
            {
                g_pBall->ClearPassTarget();
            }

            return true;
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0xAB90 | 0x80023ECC | size: 0x1DC
 */
bool cFielder::CollideWithFreezeCallback()
{
    bool bShouldSkip = false;

    if ((m_ePowerup == POWER_UP_STAR && m_tPowerupEffectTime.m_uPackedTime != 0) || mActionShootToScoreVars.isCurrentlyInvincible != 0)
    {
        bShouldSkip = true;
    }

    // If invincible, don't freeze
    if ((!bShouldSkip) && (m_tFrozenTimer.m_uPackedTime == 0))
    {
        if (m_eAnimID != 0x74 && m_eAnimID != 0x75)
        {

            // If player has the ball, release it and shoot it
            if (m_pBall != nullptr)
            {
                ReleaseBall();
                ShootBallDueToContact(m_v3Velocity);

                switch (m_eActionState)
                {
                case ACTION_SHOT:
                    break;
                case ACTION_RUNNING_WB:
                case ACTION_RUNNING_WB_TURBO:
                case ACTION_RUNNING_WB_TURBO_TURN:
                case ACTION_SHOOT_TO_SCORE:
                    InitActionRunning();
                    break;
                }

                m_pShotMeter->Abort(this);
            }

            float fFreezeDuration = g_pGame->m_pGameTweaks->fFreezeShellFrozenTime;
            m_tFrozenTimer.SetSeconds(fFreezeDuration);

            m_fDesiredSpeed = 0.0f;
            m_fActualSpeed = m_fDesiredSpeed;
            SetVelocity(v3Zero);

            if ((fFreezeDuration > 0.0f) && (m_eFielderDesireState != FIELDERDESIRE_FINISH_ACTION))
            {
                g_pGame->AbortPendingThought(mThoughtHashCalcDesire);
                g_pGame->AbortPendingThought(mThoughtHashInitRunToNet);
                g_pGame->AbortPendingThought(mThoughtHashInitGetOpen);
                g_pGame->AbortPendingThought(mThoughtHashInitWindupPass);
                g_pGame->AbortPendingThought(mThoughtHashInitCutAndBreak);
                ClearQueuedDesire();
                EndDesire(false);
            }

            SetVelocity(v3Zero);
            m_fDesiredSpeed = 0.0f;
            m_fActualSpeed = m_fDesiredSpeed;

            KillDaze(this);
            EmitFreeze(this);

            if (g_pBall->m_pPassTarget != nullptr && g_pBall->m_pPassTarget == this)
            {
                g_pBall->ClearPassTarget();
            }
            return true;
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0xAACC | 0x80023E08 | size: 0xC4
 */
bool cFielder::CollideWithBananaCallback(const nlVector3& rv3BananaPosition)
{
    if (!IsFallenDown(0.0f) && (m_eActionState != ACTION_POST_WHISTLE))
    {
        bool bShouldSkip = false;
        if ((m_ePowerup == POWER_UP_STAR && m_tPowerupEffectTime.m_uPackedTime != 0) || mActionShootToScoreVars.isCurrentlyInvincible != 0)
        {
            bShouldSkip = true;
        }

        if ((!bShouldSkip) && (m_tFrozenTimer.m_uPackedTime == 0))
        {
            if ((g_pBall->m_pPassTarget != nullptr) && (g_pBall->m_pPassTarget == this))
            {
                g_pBall->ClearPassTarget();
            }
            InitActionBananaReact(rv3BananaPosition);
            return true;
        }
    }
    return false;
}

static bool IsGameplayOrOvertime(eGameState state)
{
    bool result = false;
    if (state == GS_GAMEPLAY || state == GS_OVERTIME)
    {
        result = true;
    }
    return result;
}

/**
 * Offset/Address/Size: 0xA910 | 0x80023C4C | size: 0x1BC
 */
void cFielder::CollideWithChainCallback(ChainChomp* pChainChomp)
{
    if (mActionShootToScoreVars.isInUnbreakablePart == 0)
    {
        // Clear pass target if this fielder is the current pass target
        if (g_pBall->m_pPassTarget != nullptr && g_pBall->m_pPassTarget == this)
        {
            g_pBall->ClearPassTarget();
        }

        // Play powerup sound if not fallen down
        if (!IsFallenDown(0.0f))
        {
            PowerupBase::PlayPowerupSound(POWER_UP_CHAIN_CHOMP, PowerupBase::PWRUP_SOUND_HIT, pChainChomp->mpPhysObj, 100.0f);
        }

        float offsetZ = 0.0f;

        float rightFootZ;
        float leftFootZ;
        float ballJointZ;

        leftFootZ = offsetZ + GetJointPosition(m_nLeftFootJointIndex).f.z;
        rightFootZ = offsetZ + GetJointPosition(m_nRightFootJointIndex).f.z;
        ballJointZ = offsetZ + GetJointPosition(m_nHeadJointIndex).f.z;

        // Determine if player is in air (all joints above ground)
        bool bInAir;
        if (leftFootZ > 1.0f && rightFootZ > 1.0f && ballJointZ > 1.0f)
        {
            bInAir = true;
        }
        else
        {
            bInAir = false;
        }

        if (bInAir)
        {
            // Player is in air - use bomb react
            InitActionBombReact(m_v3Position, 0.0f);
            EmitTackleImpact(this);
        }
        else
        {
            // Player is on ground - use squish react
            InitActionSquishReact(pChainChomp->mv3Velocity);
            EmitChainBite(this);
        }

        if (pChainChomp->mpThrower != nullptr)
        {
            if (g_pGame->IsGameplayOrOvertime() && !IsOnSameTeam(pChainChomp->mpThrower))
            {
                cFielder* pThrower = pChainChomp->mpThrower;
                nlSingleton<StatsTracker>::Instance()->TrackStat(
                    STATS_POWERUPS_HIT,
                    pThrower->m_pTeam->m_nSide,
                    pThrower->m_ID,
                    pChainChomp->mnThrowerPadID,
                    0,
                    0,
                    0);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xA804 | 0x80023B40 | size: 0x10C
 */
void cFielder::CollideWithBowserCallback(Bowser* pBowser)
{
    if (mActionShootToScoreVars.isInUnbreakablePart == 0)
    {
        if (g_pBall->m_pPassTarget != nullptr && g_pBall->m_pPassTarget == this)
        {
            g_pBall->ClearPassTarget();
        }

        // If this fielder owns the ball, release and shoot it
        if (g_pBall->m_pOwner == this)
        {
            ReleaseBall();

            nlVector3 v3Direction;
            nlVec3Set(v3Direction,
                g_pBall->m_v3Position.f.x - pBowser->mv3Position.f.x,
                g_pBall->m_v3Position.f.y - pBowser->mv3Position.f.y,
                g_pBall->m_v3Position.f.z - pBowser->mv3Position.f.z);
            nlVec3Scale(v3Direction, v3Direction, 2.0f);

            v3Direction.f.z = 3.0f + nlRandomf(4.0f, &nlDefaultSeed);

            g_pBall->ShootRelease(v3Direction, SPINTYPE_NONE);
        }

        if (!IsFallenDown(0.0f))
        {
            InitActionBombHitReact(pBowser->mv3Position);
        }
    }
}

// static inline bool ShouldElectrocute(eFielderActionState state,
//     const CollisionPlayerWallData* eventData,
//     cFielder* pFielder)
// {
//     if (state == ACTION_HIT_REACT || state == ACTION_BOMB_REACT || state == ACTION_STS_HIT_REACT)
//     {
//         f32 netHalfWidth = cNet::GetNetWidth() / 2.0f;
//         f32 netHeight = cNet::GetNetHeight();
//         nlVector3 jointPos = pFielder->GetJointPosition(pFielder->m_nBip01JointIndex_0xA4);
//         if ((f32)fabs(eventData->contactPoint.f.y) > netHalfWidth || (f32)fabs(jointPos.f.z) > netHeight)
//         {
//             return true;
//         }
//     }
//     return false;
// }

// void cFielder::CollideWithWallCallback(const CollisionPlayerWallData* eventData)
// {
//     cPlayer::CollideWithWallCallback(eventData);
//     if (ShouldElectrocute(m_eActionState, eventData, this))
//     {
//         InitActionElectrocution(eventData->contactPoint, eventData->wallNormal);
//     }
// }

/**
 * Offset/Address/Size: 0xA70C | 0x80023A48 | size: 0xF8
 * TODO: check inline version - problem with getter inlining
 */
void cFielder::CollideWithWallCallback(const CollisionPlayerWallData* eventData)
{
    cPlayer::CollideWithWallCallback(eventData);

    eFielderActionState state = m_eActionState;
    if (state != ACTION_HIT_REACT && state != ACTION_BOMB_REACT && state != ACTION_STS_HIT_REACT)
    {
        goto skip_check;
    }

    f32 netHalfWidth = cNet::GetNetWidth() / 2.0f;
    f32 netHeight = cNet::GetNetHeight();

    nlVector3 jointPos = GetJointPosition(m_nBip01JointIndex_0xA4);

    bool shouldElectrocute;
    if ((f32)fabs(eventData->contactPoint.f.y) > netHalfWidth || (f32)fabs(jointPos.f.z) > netHeight)
    {
        shouldElectrocute = true;
    }
    else
    {
    skip_check:
        shouldElectrocute = false;
    }

    if (shouldElectrocute)
    {
        InitActionElectrocution(eventData->contactPoint, eventData->wallNormal);
    }
}

/**
 * Offset/Address/Size: 0xA6D0 | 0x80023A0C | size: 0x3C
 */
void cFielder::ClearPassTargetIfAmThePassTarget()
{
    cBall* pBall = g_pBall;
    if (pBall->m_pPassTarget != nullptr)
    {
        if (pBall->m_pPassTarget == this)
        {
            pBall->ClearPassTarget();
        }
    }
}

/**
 * Offset/Address/Size: 0xA6B8 | 0x800239F4 | size: 0x18
 */
bool cFielder::UsePerfectPass()
{
    return g_pBall->mbHyperSTS != 0;
}

/**
 * Offset/Address/Size: 0xA6A8 | 0x800239E4 | size: 0x10
 */
bool cFielder::IsPlayingPowerupAnim()
{
    return m_nPowerupAnimID >= 0;
}

/**
 * Offset/Address/Size: 0xA5DC | 0x80023918 | size: 0xCC
 */
bool cFielder::IsCharacterInAir(bool bUseOffset) const
{
    f32 offsetZ = 0.0f;
    if (bUseOffset)
    {
        offsetZ = m_v3Position.f.z;
    }

    f32 leftFootZ = offsetZ + GetJointPosition(m_nLeftFootJointIndex).f.z;
    f32 rightFootZ = offsetZ + GetJointPosition(m_nRightFootJointIndex).f.z;
    f32 headZ = offsetZ + GetJointPosition(m_nHeadJointIndex).f.z;

    if (leftFootZ > 1.0f && rightFootZ > 1.0f && headZ > 1.0f)
    {
        return true;
    }
    return false;
}

/**
 * Offset/Address/Size: 0xA548 | 0x80023884 | size: 0x94
 */
bool cFielder::IsTurboing()
{
    if (m_pBall != nullptr)
    {
        bool bResult = true;
        bool bIsTurboing = false;
        s32 animID = m_eAnimID;

        if (animID == 0x1D)
            goto setTrue1;
        if (animID == 0x1E)
            goto setTrue1;
        if (animID == 0x1F)
            goto setTrue1;
        if (animID == 0x20)
            goto setTrue1;
        if (animID == 0x21)
            goto setTrue1;
        if (animID == 0x22)
            goto setTrue1;
        goto check1;
    setTrue1:
        bIsTurboing = true;
    check1:
        if (!bIsTurboing)
        {
            bResult = false;
        }
        return bResult;
    }
    else
    {
        bool bResult = true;
        bool bIsTurboing = false;
        s32 animID = m_eAnimID;

        if (animID == 0x10)
            goto setTrue2;
        if (animID == 0x0F)
            goto setTrue2;
        if (animID == 0x11)
            goto setTrue2;
        goto check2;
    setTrue2:
        bIsTurboing = true;
    check2:
        if (!bIsTurboing)
        {
            bResult = false;
        }
        return bResult;
    }
}

/**
 * Offset/Address/Size: 0xA50C | 0x80023848 | size: 0x3C
 * TODO: 80% match - register allocation (r5 vs r4) and branch scheduling differ
 */
bool cFielder::IsRunning() const
{
    bool result;
    bool isRunningWithBall;
    int state;

    result = false;
    state = m_eActionState;
    if (state != ACTION_RUNNING)
    {
        isRunningWithBall = result;
        if (((u32)(state - ACTION_RUNNING_WB) <= 1) || (state == ACTION_RUNNING_WB_TURBO_TURN))
        {
            isRunningWithBall = true;
        }
        if (isRunningWithBall)
        {
            goto set_result;
        }
    }
    else
    {
    set_result:
        result = true;
    }
    return result;
}

/**
 * Offset/Address/Size: 0xA4D8 | 0x80023814 | size: 0x34
 */
bool cFielder::IsInvincible() const
{
    bool result = false;

    if (((m_ePowerup == POWER_UP_STAR) && (m_tPowerupEffectTime.m_uPackedTime != 0)) || (mActionShootToScoreVars.isCurrentlyInvincible != 0))
    {
        result = true;
    }

    return result;
}

/**
 * Offset/Address/Size: 0xA49C | 0x800237D8 | size: 0x3C
 */
bool cFielder::IsBallAwayFromCarrier() const
{
    if (m_eActionState == ACTION_RUNNING_WB_TURBO)
    {
        if ((m_pCurrentAnimController->m_fTime > 0.2f) && (m_pCurrentAnimController->m_fTime < 0.975f))
        {
            return true;
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0xA468 | 0x800237A4 | size: 0x34
 */
bool cFielder::IsReceivingVolleyPass() const
{
    eFielderDesireState desireState = m_eFielderDesireState;
    bool result = false;

    if (desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE || desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_RUN)
    {
        result = m_DesireReceivePassSharedVars.bVolleyPassReceive;
    }
    else if (desireState == FIELDERDESIRE_ONETIMER)
    {
        result = m_DesireOneTimerVars.bVolleyPassReceive;
    }

    return result;
}

/**
 * Offset/Address/Size: 0xA444 | 0x80023780 | size: 0x24
 */
bool cFielder::IsPreparingForOneTimer() const
{
    bool result = false;
    if (m_eFielderDesireState == FIELDERDESIRE_ONETIMER)
    {
        result = m_eDesireSubState == 0;
    }
    return result;
}

/**
 * Offset/Address/Size: 0xA2D0 | 0x8002360C | size: 0x174
 */
void cFielder::CleanUpAction()
{
    switch (m_eActionState)
    {
    case ACTION_DEKE:
        Audio::gCrowdSFX.Stop((Audio::eWorldSFX)0x9F, cGameSFX::SFX_STOP_FIRST);
        break;

    case ACTION_ELECTROCUTION:
        m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.0f;
        break;

    case ACTION_HIT:
        m_v3Position.f.z = 0.0f;
        m_v3Velocity.f.z = 0.0f;
        break;

    case ACTION_LOOSE_BALL_PASS:
        m_pPhysicsCharacter->m_CanCollideWithWall = true;
        SetNoPickUpTime(0.0f);
        mActionLooseBallPassVars.bVolleyPass = false;
        break;

    case ACTION_LOOSE_BALL_SHOT:
        m_pPhysicsCharacter->m_CanCollideWithWall = true;
        SetNoPickUpTime(0.0f);
        mActionLooseBallShotVars.bIsChipShot = false;
        break;

    case ACTION_SHOT:
        EndBlur();
        mActionShotVars.bIsChipShot = false;
        break;

    case ACTION_PASS:
        mActionPassingVars.pPassTarget = NULL;
        mActionPassingVars.bVolleyPass = false;
        break;

    case ACTION_RUNNING:
        mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
        break;

    case ACTION_RUNNING_WB:
        m_eLastPadAction = PAD_NONE;
        break;

    case ACTION_RUNNING_WB_TURBO:
        m_eLastPadAction = PAD_NONE;
        if (m_ePowerup != POWER_UP_MUSHROOM)
        {
            EndBlur();
        }
        break;

    case ACTION_SHOOT_TO_SCORE:
        CleanActionShootToScore();
        break;

    case ACTION_LATE_ONETIMER_FROM_VOLLEY:
        mActionShotVars.bIsChipShot = false;
        break;

    case ACTION_SLIDE_ATTACK:
        Audio::gCrowdSFX.Stop((Audio::eWorldSFX)0x9F, cGameSFX::SFX_STOP_FIRST);
        KillSlideTackleTrail(this);
        StopSFX(Audio::CHARSFX_SLIDE);
        break;

    case ACTION_SLIDE_ATTACK_REACT:
        KillDaze(this);
        break;

    case ACTION_BOMB_REACT:
    case ACTION_SHELL_REACT:
    case ACTION_BANANA_REACT:
    case ACTION_STS_HIT_REACT:
    case ACTION_SQUISH_REACT:
    default:
        break;
    }

    m_eActionState = ACTION_NEED_ACTION;
}

/**
 * Offset/Address/Size: 0xA258 | 0x80023594 | size: 0x78
 */
void cFielder::CleanUpPowerupEffect()
{
    switch (m_ePowerup)
    {
    case POWER_UP_STAR:
        KillStar(this);
        m_ePowerup = POWER_UP_NONE;
        mnNumPowerups = 0;
        m_tPowerupEffectTime.m_uPackedTime = 0;
        break;

    case POWER_UP_MUSHROOM:
        KillMushroom(this);
        m_ePowerup = POWER_UP_NONE;
        mnNumPowerups = 0;
        m_tPowerupEffectTime.m_uPackedTime = 0;
        break;

    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x9DFC | 0x80023138 | size: 0x45C
 * TODO: 99.82% match - remaining f0/f1 register coloring diff in Y/Z accuracy
 * updates caused by -inline deferred flag (scratch matches 100% with -inline auto)
 */
void cFielder::CalcRegularShot(nlVector3& rv3Vel, nlVector3& rv3Target)
{
    u32 uSaveType = 0x10000 - 4;
    float fShotValue = m_pShotMeter->m_fScoreValue;
    Goalie* pGoalie = m_pTeam->GetOtherTeam()->GetGoalie();
    float fGoalieEnergy = pGoalie->mFatigue.mfEnergyLevel;
    GoalieTweaks* pGoalieTweaks = (GoalieTweaks*)pGoalie->m_pTweaks;

    bool bIsChipShot = false;
    if (mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot)
    {
        bIsChipShot = true;
    }

    if (bIsChipShot
        || ((fGoalieEnergy >= pGoalieTweaks->fFatigueCatchThreshold)
            && (m_pShotMeter->m_fSpeedValue < pGoalieTweaks->fCatchSaveMaxSpeed)))
    {
        uSaveType = 0x10000 - 1;
    }

    pGoalie->muSaveType = uSaveType;

    cBall* pBall = g_pBall;
    float fShotTime;

    extern void DoFindBestShotTarget__8cFielderFR9nlVector3Rfb(cFielder*, nlVector3&, float&, bool);
    DoFindBestShotTarget__8cFielderFR9nlVector3Rfb(this, rv3Target, fShotTime, false);

    float fDesiredTime = 1.0f / fShotTime;
    float fDeltaX = pBall->m_v3Position.f.x - rv3Target.f.x;
    float fDeltaY = pBall->m_v3Position.f.y - rv3Target.f.y;
    float fDistance = nlSqrt((fDeltaX * fDeltaX) + (fDeltaY * fDeltaY), true);
    fDesiredTime = fDistance * fDesiredTime;

    bool bIsChipShot2 = false;
    if (mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot)
    {
        bIsChipShot2 = true;
    }

    float fAccuracyScale;
    if (bIsChipShot2)
    {
        fAccuracyScale = (1.1f - fShotValue) * (1.5f * fDistance);
    }
    else
    {
        fAccuracyScale = fShotTime * (1.1f - fShotValue);
    }

    GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;
    float fYAccuracy = fAccuracyScale * pGameTweaks->unk2FC;
    float fZAccuracy = fAccuracyScale * pGameTweaks->unk300;

    static FilteredRandomReal randgenYAccuracy;
    static FilteredRandomReal randgenZAccuracy;

    float fYRand = fYAccuracy * randgenYAccuracy.genrand();
    rv3Target.f.y += 0.5f * fYAccuracy - fYRand;
    float fZOffset = fZAccuracy * randgenZAccuracy.genrand();
    rv3Target.f.z += fZOffset;

    g_pBall->ShootAtFast(rv3Vel, rv3Target, fDesiredTime);

    extern cTeam* g_pCurrentlyUpdatingTeam;

    if (fShotValue < SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue1)
    {
        fShotValue = InterpolateRangeClamped(
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance0,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance1,
            0.0f,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue1,
            fShotValue);
    }
    else if (fShotValue < SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue2)
    {
        fShotValue = InterpolateRangeClamped(
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance1,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance2,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue1,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue2,
            fShotValue);
    }
    else if (fShotValue < SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue3)
    {
        fShotValue = InterpolateRangeClamped(
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance2,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance3,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue2,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue3,
            fShotValue);
    }
    else
    {
        fShotValue = InterpolateRangeClamped(
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance3,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotChance4,
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fShotValue3,
            1.0f,
            fShotValue);
    }

    static FilteredRandomReal randgenMiss;

    if ((100.0f * randgenMiss.genrand()) < fShotValue)
    {
        pGoalie->mbShouldMiss = true;
    }
    else
    {
        pGoalie->mbShouldMiss = false;
    }
}

/**
 * Offset/Address/Size: 0x95C8 | 0x80022904 | size: 0x834
 */
void cFielder::CalcShootToScoreShot(nlVector3&, nlVector3&)
{
}

/**
 * Offset/Address/Size: 0x9594 | 0x800228D0 | size: 0x34
 */
void cFielder::SetSlideAttackSuccessFlag()
{
    mActionSlideAttackVars.bAttackSucceeded = true;
    BeginRumbleAction(RUMBLE_MEDIUM_CONTACT, GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x9564 | 0x800228A0 | size: 0x30
 */
void cFielder::SetKickOffWaitTime()
{
    mbCanKickoff = true;
    mtKickOffWaitTimer.SetSeconds(2.0f);
}

/**
 * Offset/Address/Size: 0x94B8 | 0x800227F4 | size: 0xAC
 */
void cFielder::SetBombImpactTime(const nlVector3& v3BombImpactLocation, float fBombImpactRadius)
{
    float fDeltaX = m_v3Position.f.x - v3BombImpactLocation.f.x;
    float fDeltaY = m_v3Position.f.y - v3BombImpactLocation.f.y;
    float fDeltaZ = m_v3Position.f.z - v3BombImpactLocation.f.z;

    float fDistanceSquared = fDeltaX * fDeltaX + fDeltaY * fDeltaY + fDeltaZ * fDeltaZ;
    float fTime = nlSqrt(fDistanceSquared, true) / 25.f;

    mtBombImpactTime.SetSeconds(fTime);
    mv3BombImpactLocation = v3BombImpactLocation;
    mfBombImpactRadius = fBombImpactRadius;
}

/**
 * Offset/Address/Size: 0x93F4 | 0x80022730 | size: 0xC4
 */
void cFielder::SetDesireDuration(float duration, bool randomize)
{
    if (duration > 0.0f && randomize)
    {
        static FilteredRandomReal randgen;

        // Add random variation
        float rand = randgen.genrand();
        float temp = 0.6f * rand;
        float variation = temp - 0.3f;
        duration += variation;
        if (0.0f >= duration)
        {
            duration = 0.0f;
        }
    }

    m_tDesireDuration.SetSeconds(duration);
}

/**
 * Offset/Address/Size: 0x921C | 0x80022558 | size: 0x1D8
 */
void cFielder::ShootBallDueToContact(const nlVector3& aShootDirection)
{
    FORCE_DONT_INLINE;
    bool bBallAwayFromCarrier;
    if ((m_eActionState == ACTION_RUNNING_WB_TURBO) && ((m_pCurrentAnimController->m_fTime > 0.2f) && (m_pCurrentAnimController->m_fTime < 0.975f)))
    {
        bBallAwayFromCarrier = true;
    }
    else
    {
        bBallAwayFromCarrier = false;
    }
    if (bBallAwayFromCarrier)
    {
        nlVector3 v3Direction;
        nlPolarToCartesian(v3Direction.f.x, v3Direction.f.y, m_aActualFacingDirection, m_fActualSpeed);
        v3Direction.f.z = 0.5f;
        g_pBall->ShootRelease(v3Direction, SPINTYPE_NONE);
        return;
    }
    if (m_eActionState == ACTION_SHOOT_TO_SCORE)
    {
        g_pBall->ShootRelease(v3Zero, SPINTYPE_NONE);
        return;
    }
    nlVector3 v3ReleaseVelocity;
    nlVec3Set(v3ReleaseVelocity, aShootDirection.f.x + m_v3Velocity.f.x, aShootDirection.f.y + m_v3Velocity.f.y, aShootDirection.f.z + m_v3Velocity.f.z);
    if ((aShootDirection.f.x * aShootDirection.f.x + aShootDirection.f.y * aShootDirection.f.y + aShootDirection.f.z * aShootDirection.f.z < 0.001f) || (m_v3Velocity.f.x * m_v3Velocity.f.x + m_v3Velocity.f.y * m_v3Velocity.f.y + m_v3Velocity.f.z * m_v3Velocity.f.z < 0.001f) || (v3ReleaseVelocity.f.x * v3ReleaseVelocity.f.x + v3ReleaseVelocity.f.y * v3ReleaseVelocity.f.y + v3ReleaseVelocity.f.z * v3ReleaseVelocity.f.z < 0.001f))
    {
        nlVector3 v3FallbackDirection;
        nlPolarToCartesian(v3FallbackDirection.f.x, v3FallbackDirection.f.y, m_aActualFacingDirection, 2.0f);
        v3FallbackDirection.f.z = 0.5f;
        g_pBall->ShootRelease(v3FallbackDirection, SPINTYPE_NONE);
        return;
    }
    float fRecip = nlRecipSqrt(v3ReleaseVelocity.f.x * v3ReleaseVelocity.f.x + v3ReleaseVelocity.f.y * v3ReleaseVelocity.f.y + v3ReleaseVelocity.f.z * v3ReleaseVelocity.f.z, true);
    nlVec3Scale(v3ReleaseVelocity, v3ReleaseVelocity, fRecip);

    float fSpeed = 2.0f + m_fActualSpeed;
    nlVec3Set(v3ReleaseVelocity, fSpeed * v3ReleaseVelocity.f.x, fSpeed * v3ReleaseVelocity.f.y, fSpeed * v3ReleaseVelocity.f.z);
    v3ReleaseVelocity.f.z = 0.5f;
    g_pBall->ShootRelease(v3ReleaseVelocity, SPINTYPE_NONE);
}

// void cFielder::ShootBallDueToContact(const nlVector3& aShootDirection)
// {
//     bool bBallAwayFromCarrier;
//     if ((m_eActionState == ACTION_RUNNING_WB_TURBO) && ((m_pCurrentAnimController->m_fTime > 0.2f) && (m_pCurrentAnimController->m_fTime < 0.975f)))
//     {
//         bBallAwayFromCarrier = true;
//     }
//     else
//     {
//         bBallAwayFromCarrier = false;
//     }
//     if (bBallAwayFromCarrier)
//     {
//         nlVector3 v3Direction;
//         nlPolarToCartesian(v3Direction.f.x, v3Direction.f.y, m_aActualFacingDirection, m_fActualSpeed);
//         v3Direction.f.z = 0.5f;
//         g_pBall->ShootRelease(v3Direction, SPINTYPE_NONE);
//         return;
//     }
//     if (m_eActionState == ACTION_SHOOT_TO_SCORE)
//     {
//         g_pBall->ShootRelease(v3Zero, SPINTYPE_NONE);
//         return;
//     }
//     nlVector3 v3ReleaseVelocity;
//     v3ReleaseVelocity.f.x = aShootDirection.f.x + m_v3Velocity.f.x;
//     v3ReleaseVelocity.f.y = aShootDirection.f.y + m_v3Velocity.f.y;
//     v3ReleaseVelocity.f.z = aShootDirection.f.z + m_v3Velocity.f.z;
//     if ((aShootDirection.f.x * aShootDirection.f.x + aShootDirection.f.y * aShootDirection.f.y + aShootDirection.f.z * aShootDirection.f.z < 0.001f) || (m_v3Velocity.f.x * m_v3Velocity.f.x + m_v3Velocity.f.y * m_v3Velocity.f.y + m_v3Velocity.f.z * m_v3Velocity.f.z < 0.001f) || (v3ReleaseVelocity.f.x * v3ReleaseVelocity.f.x + v3ReleaseVelocity.f.y * v3ReleaseVelocity.f.y + v3ReleaseVelocity.f.z * v3ReleaseVelocity.f.z < 0.001f))
//     {
//         nlVector3 v3FallbackDirection;
//         nlPolarToCartesian(v3FallbackDirection.f.x, v3FallbackDirection.f.y, m_aActualFacingDirection, 2.0f);
//         v3FallbackDirection.f.z = 0.5f;
//         g_pBall->ShootRelease(v3FallbackDirection, SPINTYPE_NONE);
//         return;
//     }
//     float fRecip = nlRecipSqrt(v3ReleaseVelocity.f.x * v3ReleaseVelocity.f.x + v3ReleaseVelocity.f.y * v3ReleaseVelocity.f.y + v3ReleaseVelocity.f.z * v3ReleaseVelocity.f.z, true);
//     nlVec3Scale(v3ReleaseVelocity, v3ReleaseVelocity, fRecip);
//     // v3ReleaseVelocity.f.z = fRecip * v3ReleaseVelocity.f.z;
//     // v3ReleaseVelocity.f.y = fRecip * v3ReleaseVelocity.f.y;
//     // v3ReleaseVelocity.f.x = fRecip * v3ReleaseVelocity.f.x;

//     float fSpeed = 2.0f + m_fActualSpeed;
//     v3ReleaseVelocity.f.z = fSpeed * v3ReleaseVelocity.f.z;
//     v3ReleaseVelocity.f.y = fSpeed * v3ReleaseVelocity.f.y;
//     v3ReleaseVelocity.f.x = fSpeed * v3ReleaseVelocity.f.x;
//     v3ReleaseVelocity.f.z = 0.5f;
//     g_pBall->ShootRelease(v3ReleaseVelocity, SPINTYPE_NONE);
// }

/**
 * Offset/Address/Size: 0x918C | 0x800224C8 | size: 0x90
 */
void cFielder::ShootBallDueToContact(unsigned short aAngle)
{
    bool bBallAwayFromCarrier;

    if ((m_eActionState == ACTION_RUNNING_WB_TURBO) && ((m_pCurrentAnimController->m_fTime > 0.2f) && (m_pCurrentAnimController->m_fTime < 0.975f)))
    {
        bBallAwayFromCarrier = true;
    }
    else
    {
        bBallAwayFromCarrier = false;
    }

    if (!bBallAwayFromCarrier)
    {
        nlVector3 v3Direction;
        nlPolarToCartesian(v3Direction.f.x, v3Direction.f.y, aAngle, 2.0f + m_fActualSpeed);
        v3Direction.f.z = 0.5f;

        g_pBall->ShootRelease(v3Direction, SPINTYPE_NONE);
    }
}

/**
 * Offset/Address/Size: 0x8F5C | 0x80022298 | size: 0x230
 * TODO: 99.43% match - remaining gap is in DoClearBall angle-selection register allocation:
 *       r6/r7 swap for m_pShotMeter vs pClearingBottomAngle.a, plus r3/r4 and cmplw operand order drift.
 */
void cFielder::DoClearBall()
{
    nlPolar pClearingTopAngle;
    nlPolar pClearingBottomAngle;
    u16 aClearingAngle;

    float fRandomDistance = nlRandomf(5.0f, &nlDefaultSeed);
    float fGoalline = cField::GetGoalLineX(m_pTeam->GetOtherNet()->m_sideSign);

    float fTopY = cField::GetSidelineY(1) - fRandomDistance;
    nlCartesianToPolar(pClearingTopAngle, fGoalline - m_v3Position.f.x, fTopY - m_v3Position.f.y);

    float fBottomY = fRandomDistance + cField::GetSidelineY(0);
    nlCartesianToPolar(pClearingBottomAngle, fGoalline - m_v3Position.f.x, fBottomY - m_v3Position.f.y);

    ShotMeter* pShotMeter = m_pShotMeter;
    s16 nBottomDelta = (int)(s16)(m_aActualFacingDirection - pClearingBottomAngle.a);
    s16 nTopDelta = (int)(s16)(m_aActualFacingDirection - pClearingTopAngle.a);

    if (pShotMeter->mfSShotAimValue > 0.5f)
    {
        aClearingAngle = pClearingTopAngle.a;
    }
    else if (pShotMeter->mfSShotAimValue < -0.5f)
    {
        aClearingAngle = pClearingBottomAngle.a;
    }
    else
    {
        u16 nAbsBottomDelta = (u16)((nBottomDelta < 0) ? -nBottomDelta : nBottomDelta);
        u16 nAbsTopDelta = (u16)((nTopDelta < 0) ? -nTopDelta : nTopDelta);

        if (nAbsBottomDelta < nAbsTopDelta)
        {
            aClearingAngle = pClearingBottomAngle.a;
        }
        else
        {
            aClearingAngle = pClearingTopAngle.a;
        }
    }

    float fClearSpeed = InterpolateRangeClamped(
        g_pGame->m_pGameTweaks->unk2BC, g_pGame->m_pGameTweaks->unk2C0, 0.0f, 1.0f, pShotMeter->m_fSpeedValue);

    GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
    float fZSpeed = pTweaks->unk2C4;
    bool bIsChipShot = mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot;
    if (bIsChipShot)
    {
        fZSpeed = pTweaks->unk2C8;
    }

    nlVector3 v3ClearBallVelocity;
    nlPolarToCartesian(v3ClearBallVelocity.f.x, v3ClearBallVelocity.f.y, aClearingAngle, fClearSpeed);
    v3ClearBallVelocity.f.z = fZSpeed;

    if (m_pBall != NULL)
    {
        ReleaseBall();
    }

    g_pBall->ShootRelease(v3ClearBallVelocity,
        nlRandom(2, &nlDefaultSeed) != 0 ? SPINTYPE_FORWARD : SPINTYPE_BACK);
    SetNoPickUpTime(0.25f);

    Audio::SoundAttributes sndAtr;
    sndAtr.Init();
    sndAtr.SetSoundType(0xB3, true);
    sndAtr.UseStationaryPosVector(m_v3Position);
    Audio::gStadGenSFX.Play(sndAtr);
    EmitBallImpact(this, true);
}

/**
 * Offset/Address/Size: 0x8CA8 | 0x80021FE4 | size: 0x2B4
 * TODO: 93.09% match - remaining gap in ACTION_SLIDE_FAIL_REACT jump-table
 * lowering and early ShotMeter register assignment (r4/r3 order).
 */
void cFielder::DoHandleActiveShotMeter()
{
    if (GetGlobalPad() == NULL)
    {
        return;
    }

    if ((u32)m_eActionState <= ACTION_SLIDE_FAIL_REACT)
    {
        if (m_eActionState == ACTION_SLIDE_FAIL_REACT)
        {
            m_pShotMeter->Abort(this);
            return;
        }
    }

    if (m_pBall == NULL)
    {
        m_pShotMeter->Abort(this);
        return;
    }

    bool bReturn;
    if (m_eActionState == ACTION_RUNNING_WB_TURBO && m_pCurrentAnimController->m_fTime > 0.2f && m_pCurrentAnimController->m_fTime < 1.0f)
    {
        bReturn = true;
    }
    else
    {
        bReturn = false;
    }

    if (bReturn)
    {
        return;
    }

    if (m_eActionState == ACTION_DEKE)
    {
        return;
    }

    ShotMeter* pShotMeter = m_pShotMeter;
    bool bIsActive = false;
    eShotMeterState meterState = pShotMeter->m_eShotMeterState;
    if (meterState == SHOT_METER_ACTIVE || meterState == SHOT_METER_STS_ACTIVE || meterState == SHOT_METER_STS_TRANSISTION)
    {
        bIsActive = true;
    }
    if (bIsActive)
    {
        if (pShotMeter->m_eShotMeterState == SHOT_METER_STS_TRANSISTION)
        {
            KillWindup(this, "ball_shot_windup", false);
            EmitWindupAtCharacter(this, "ball_sts_windup");
        }
    }

    bool bIsChipShot = false;
    if (GetGlobalPad() != NULL)
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        if (GetGlobalPad()->GetPressure(PAD_AIM, true) > pTweaks->unk2B0)
        {
            bIsChipShot = true;
        }
    }

    pShotMeter = m_pShotMeter;
    bIsActive = false;
    meterState = pShotMeter->m_eShotMeterState;
    if (meterState == SHOT_METER_ACTIVE || meterState == SHOT_METER_STS_ACTIVE || meterState == SHOT_METER_STS_TRANSISTION)
    {
        bIsActive = true;
    }
    if (bIsActive)
    {
        bool bCanShootToScore = false;
        if (pShotMeter->m_eShotMeterState == SHOT_METER_STS_TRANSISTION)
        {
            cNet* pOtherNet = m_pTeam->GetOtherNet();
            if ((m_v3Position.f.x * pOtherNet->m_sideSign) <= 0.0f)
            {
                bCanShootToScore = true;
            }
        }

        if (GetGlobalPad()->IsPressed(PAD_SHOOT, true))
        {
            if (!bCanShootToScore)
            {
                return;
            }
        }

        mActionShotVars.bIsChipShot = bIsChipShot;
        m_pShotMeter->ShotReleased(this);
        InitActionShot(mActionShotVars.bIsChipShot);
    }
    else
    {
        eFielderActionState actionState = m_eActionState;
        if (actionState != ACTION_SHOT && pShotMeter->m_eShotMeterState == SHOT_METER_RELEASED)
        {
            mActionShotVars.bIsChipShot = bIsChipShot;
            m_pShotMeter->ShotReleased(this);

            if (GetGlobalPad() != NULL)
            {
                InitActionShot(mActionShotVars.bIsChipShot);
            }
            else
            {
                InitActionShot(false);
            }
        }
        else if (actionState != ACTION_SHOT && pShotMeter->m_eShotMeterState == SHOT_METER_STS_RELEASED)
        {
            InitActionShootToScore();
        }
    }
}

/**
 * Offset/Address/Size: 0x8A18 | 0x80021D54 | size: 0x290
 */
bool cFielder::DoLooseBallContactFromIdle(nlVector3& v3AnimStartPosition, float& fAnimStartTime, nlVector3& v3BallContactPosition, float& fBallContactTime,
    unsigned short aFutureFacingDirection, const LooseBallContactAnimInfo* pBestBallContactAnimInfo)
{
    const cSAnim* pGuessContactAnim = m_pAnimInventory->GetAnim(pBestBallContactAnimInfo->nAnimID);
    float fAnimTimeToContact = pBestBallContactAnimInfo->fAnimContactFrame / (float)pGuessContactAnim->m_nNumKeys;

    nlVector3 v3ContactOffsetLocal;
    GetJointPositionFuture(&v3ContactOffsetLocal, pBestBallContactAnimInfo->nAnimID, m_nBallJointIndex, fAnimTimeToContact, true, true, false);

    float fCos;
    float fSin;
    nlSinCos(&fSin, &fCos, aFutureFacingDirection);

    nlVector3 v3ContactOffsetWorld;
    nlVector3* pContactOffsetWorld = &v3ContactOffsetWorld;
    float ySin = v3ContactOffsetLocal.f.y * fSin;
    float xSin = v3ContactOffsetLocal.f.x * fSin;

    pContactOffsetWorld->f.x = (v3ContactOffsetLocal.f.x * fCos) - ySin;
    pContactOffsetWorld->f.y = (v3ContactOffsetLocal.f.y * fCos) + xSin;
    pContactOffsetWorld->f.z = v3ContactOffsetLocal.f.z;

    float fPrevBestDistToContactSquared;
    float fBallContactTargetX;
    float fBallContactTargetY;
    float fBallContactTargetZ;
    float fBestDistToContactSquared;
    float fSimulatedTime;

    fBallContactTargetZ = m_v3Position.f.z + pContactOffsetWorld->f.z;
    fBallContactTargetY = m_v3Position.f.y + pContactOffsetWorld->f.y;
    fBallContactTargetX = m_v3Position.f.x + pContactOffsetWorld->f.x;

    FakeBallWorld::ResetBallIterator();

    fBestDistToContactSquared = 0.0f;
    bool bDone = false;
    fSimulatedTime = fBestDistToContactSquared;
    fPrevBestDistToContactSquared = fBestDistToContactSquared;

    while (!bDone || fSimulatedTime > 2.0f)
    {
        nlVector3 v3SimulatedBallPos;
        FakeBallWorld::GetNextBallPosition(v3SimulatedBallPos);

        float distSq;
        float deltaX;
        float deltaY;
        float deltaZ;

        deltaY = fBallContactTargetY - v3SimulatedBallPos.f.y;
        deltaZ = fBallContactTargetZ - v3SimulatedBallPos.f.z;
        deltaX = fBallContactTargetX - v3SimulatedBallPos.f.x;
        distSq = deltaY * deltaY;
        distSq += deltaX * deltaX;
        distSq += deltaZ * deltaZ;

        if (fSimulatedTime > fPrevBestDistToContactSquared)
        {
            if (distSq > fBestDistToContactSquared)
            {
                bDone = true;
            }
        }

        if (!bDone)
        {
            v3BallContactPosition = v3SimulatedBallPos;
            fBestDistToContactSquared = distSq;
            fSimulatedTime += FixedUpdateTask::GetPhysicsUpdateTick();
            if (fSimulatedTime > 2.0f)
            {
                return false;
            }
        }
    }

    float startX;
    float startY;
    float startZ;

    startX = v3BallContactPosition.f.x - pContactOffsetWorld->f.x;
    startZ = v3BallContactPosition.f.z - pContactOffsetWorld->f.z;
    startY = v3BallContactPosition.f.y - pContactOffsetWorld->f.y;
    v3AnimStartPosition.f.x = startX;
    v3AnimStartPosition.f.y = startY;
    v3AnimStartPosition.f.z = startZ;

    unsigned int nNumKeys = pGuessContactAnim->m_nNumKeys;
    float fContactTimeNorm = pBestBallContactAnimInfo->fAnimContactFrame / (float)nNumKeys;
    float fAnimLength = (float)nNumKeys / 30.0f;
    fAnimStartTime = fSimulatedTime - (fContactTimeNorm * fAnimLength);
    fBallContactTime = fSimulatedTime;

    if (!m_bHasBeenUpdated)
    {
        fAnimStartTime += g_fSimulationTick;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x87A8 | 0x80021AE4 | size: 0x270
 * TODO: 99.07% match - f30/f31 register swap for fMaxSimulatedTime/bestTime (MWCC allocator quirk)
 */
bool cFielder::DoLooseBallContactFromRun(nlVector3& v3AnimStartPosition, float& fAnimStartTime, nlVector3& v3BallContactPosition, float& fBallContactTime,
    const LooseBallContactAnimInfo* pBestBallContactAnimInfo, const nlVector3& v3PassIntercept)
{
    float bestTime;
    float fMaxSimulatedTime;
    float passInterceptY;
    float passInterceptX;
    float distSq;
    float fSimulatedTime;
    nlVector3 bestIntercept;
    float bestDistToPassInterceptSquared;
    nlVector3 v3SimulatedBallPos;
    const cSAnim* guessContactAnim;
    nlVector3 v3ContactOffsetWorld;
    nlVector3 v3ContactOffsetLocal;
    float fAnimTimeToContact;
    float fCos;
    float fSin;

    FakeBallWorld::ResetBallIterator();

    fSimulatedTime = 0.0f;
    passInterceptX = v3PassIntercept.f.x;
    bestDistToPassInterceptSquared = 0.0f;
    passInterceptY = v3PassIntercept.f.y;
    fMaxSimulatedTime = 2.0f;

    while (fSimulatedTime < fMaxSimulatedTime)
    {
        FakeBallWorld::GetNextBallPosition(v3SimulatedBallPos);
        fSimulatedTime += FixedUpdateTask::GetPhysicsUpdateTick();

        float deltaX = v3SimulatedBallPos.f.x - passInterceptX;
        float deltaY = v3SimulatedBallPos.f.y - passInterceptY;
        distSq = deltaX * deltaX + deltaY * deltaY;

        if (distSq < bestDistToPassInterceptSquared)
        {
        }
        else if (fSimulatedTime != FixedUpdateTask::GetPhysicsUpdateTick())
        {
            break;
        }

        bestDistToPassInterceptSquared = distSq;
        bestTime = fSimulatedTime;
        bestIntercept = v3SimulatedBallPos;
    }

    if (fSimulatedTime >= fMaxSimulatedTime)
    {
        return false;
    }

    guessContactAnim = m_pAnimInventory->GetAnim(pBestBallContactAnimInfo->nAnimID);
    u32 nNumKeys = guessContactAnim->m_nNumKeys;
    fAnimTimeToContact = pBestBallContactAnimInfo->fAnimContactFrame / (float)nNumKeys;
    GetJointPositionFuture(&v3ContactOffsetLocal, pBestBallContactAnimInfo->nAnimID, m_nBallJointIndex, fAnimTimeToContact, true, true, false);

    nlSinCos(&fSin, &fCos, m_aActualFacingDirection);

    nlVector3* pContactOffsetWorld = &v3ContactOffsetWorld;
    float ySin = v3ContactOffsetLocal.f.y * fSin;
    float xSin = v3ContactOffsetLocal.f.x * fSin;
    pContactOffsetWorld->f.z = v3ContactOffsetLocal.f.z;
    pContactOffsetWorld->f.x = (v3ContactOffsetLocal.f.x * fCos) - ySin;
    pContactOffsetWorld->f.y = (v3ContactOffsetLocal.f.y * fCos) + xSin;

    float fDesiredSpeedToAnimStart = (float)nNumKeys / 30.0f;
    float fBestSpeedToAnimStartDelta = fAnimTimeToContact * fDesiredSpeedToAnimStart;

    nlVec3Set(v3AnimStartPosition,
        bestIntercept.f.x - pContactOffsetWorld->f.x,
        bestIntercept.f.y - pContactOffsetWorld->f.y,
        bestIntercept.f.z - pContactOffsetWorld->f.z);
    v3AnimStartPosition.f.z = 0.0f;

    fAnimStartTime = bestTime - fBestSpeedToAnimStartDelta;

    v3BallContactPosition = bestIntercept;
    fBallContactTime = bestTime;

    if (!m_bHasBeenUpdated)
    {
        fAnimStartTime += g_fSimulationTick;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x84AC | 0x800217E8 | size: 0x2FC
 * TODO: 65.09% match - stack/register allocation and loop branch shape still differ in Z filtering and final timing math.
 */
bool cFielder::DoLooseBallContactFromRunVolley(nlVector3& v3AnimStartPosition, float& fAnimStartTime, nlVector3& v3BallContactPosition, float& fBallContactTime,
    const LooseBallContactAnimInfo* pBestBallContactAnimInfo, const nlVector3& v3PassIntercept)
{
    const cSAnim* pGuessContactAnim = m_pAnimInventory->GetAnim(pBestBallContactAnimInfo->nAnimID);
    float fAnimTimeToContact = pBestBallContactAnimInfo->fAnimContactFrame / (float)pGuessContactAnim->m_nNumKeys;

    nlVector3 v3ContactOffsetLocal;
    GetJointPositionFuture(&v3ContactOffsetLocal, pBestBallContactAnimInfo->nAnimID, m_nBallJointIndex, fAnimTimeToContact, true, true, false);

    float fCos;
    float fSin;
    nlSinCos(&fSin, &fCos, m_aActualFacingDirection);

    nlVector3 v3ContactOffsetWorld;
    float ySin = v3ContactOffsetLocal.f.y * fSin;
    float xSin = v3ContactOffsetLocal.f.x * fSin;
    v3ContactOffsetWorld.f.x = (v3ContactOffsetLocal.f.x * fCos) - ySin;
    v3ContactOffsetWorld.f.y = (v3ContactOffsetLocal.f.y * fCos) + xSin;
    v3ContactOffsetWorld.f.z = v3ContactOffsetLocal.f.z;

    FakeBallWorld::ResetBallIterator();

    float fSimulatedTime = 0.0f;
    float fPrevBallZ = 0.0f;
    float fContactZ = v3ContactOffsetWorld.f.z;
    float bestTime;
    nlVector3 bestIntercept;

    while (fSimulatedTime < 5.0f)
    {
        nlVector3 v3SimulatedBallPos;
        FakeBallWorld::GetNextBallPosition(v3SimulatedBallPos);
        fSimulatedTime += FixedUpdateTask::GetPhysicsUpdateTick();

        float prevDistZ = (float)fabs(fPrevBallZ - fContactZ);
        float currDistZ = (float)fabs(v3SimulatedBallPos.f.z - fContactZ);

        if (fSimulatedTime > FixedUpdateTask::GetPhysicsUpdateTick())
        {
            bool bIsCrossingZ = ((v3SimulatedBallPos.f.z >= fContactZ) && (fPrevBallZ < fContactZ)) || ((v3SimulatedBallPos.f.z < fContactZ) && (fPrevBallZ >= fContactZ));

            if (bIsCrossingZ || (currDistZ <= prevDistZ))
            {
                float deltaY = v3SimulatedBallPos.f.y - v3PassIntercept.f.y;
                float deltaX = v3SimulatedBallPos.f.x - v3PassIntercept.f.x;
                if ((deltaY * deltaY + deltaX * deltaX) < 1.0f)
                {
                    bestTime = fSimulatedTime;
                    bestIntercept = v3SimulatedBallPos;
                    break;
                }
            }
        }

        fPrevBallZ = v3SimulatedBallPos.f.z;
    }

    if (fSimulatedTime >= 5.0f)
    {
        return false;
    }

    float fDesiredSpeedToAnimStart = (float)pGuessContactAnim->m_nNumKeys / 30.0f;
    float fBestSpeedToAnimStartDelta = fAnimTimeToContact * fDesiredSpeedToAnimStart;

    v3AnimStartPosition.f.x = bestIntercept.f.x - v3ContactOffsetWorld.f.x;
    v3AnimStartPosition.f.y = bestIntercept.f.y - v3ContactOffsetWorld.f.y;
    v3AnimStartPosition.f.z = bestIntercept.f.z - v3ContactOffsetWorld.f.z;
    v3AnimStartPosition.f.z = 0.0f;

    fAnimStartTime = bestTime - fBestSpeedToAnimStartDelta;

    v3BallContactPosition = bestIntercept;
    fBallContactTime = bestTime;

    if (!m_bHasBeenUpdated)
    {
        fAnimStartTime += g_fSimulationTick;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x82B0 | 0x800215EC | size: 0x1FC
 */
void cFielder::DoPenaltyCardBooking(cFielder* pVictim, ePenaltyType ePenaltyType)
{
    float fMaxAmount;
    float fMinAmount;

    int ePenaltyTypeAdjusted = ePenaltyType;
    if (pVictim->m_tBallUnPossessionTimer.GetSeconds() < 0.5f)
    {
        if (ePenaltyTypeAdjusted == 1)
        {
            ePenaltyTypeAdjusted = 0;
        }
        else if (ePenaltyTypeAdjusted == 3)
        {
            ePenaltyTypeAdjusted = 2;
        }
    }

    // Update penalty card status
    switch (m_ePenaltyCardStatus)
    {
    case PENALTY_CARD_NONE:
        m_ePenaltyCardStatus = PENALTY_CARD_YELLOW_1;
        break;
    case PENALTY_CARD_YELLOW_1:
        m_ePenaltyCardStatus = PENALTY_CARD_YELLOW_2;
        break;
    case PENALTY_CARD_RED: // ??
        break;
    case PENALTY_CARD_YELLOW_2:
        m_ePenaltyCardStatus = PENALTY_CARD_RED;
        break;
    }
    PenaltyData* pData;
    if (ePenaltyTypeAdjusted == PEN_TYPE_HIT_NO_BALL)
    {
        Event* pEvent = g_pEventManager->CreateValidEvent(0x3D, 0x24);
        pData = new ((u8*)pEvent + 0x10) PenaltyData();
    }
    else
    {
        Event* pEvent = g_pEventManager->CreateValidEvent(0x3C, 0x24);
        pData = new ((u8*)pEvent + 0x10) PenaltyData();
    }

    fMinAmount = 0.0f;
    fMaxAmount = 0.0f;

    switch (ePenaltyTypeAdjusted)
    {
    case PEN_TYPE_HIT_WITH_BALL:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMinAmount = pTweaks->fPowerupHitWithBallMinAmount;
        fMaxAmount = pTweaks->fPowerupHitWithBallMaxAmount;
        break;
    }
    case PEN_TYPE_HIT_NO_BALL:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMinAmount = pTweaks->fPowerupHitNoBallMinAmount;
        fMaxAmount = pTweaks->fPowerupHitNoBallMaxAmount;
        break;
    }
    case PEN_TYPE_SLIDE_WITH_BALL:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMinAmount = pTweaks->fPowerupSlideWithBallMinAmount;
        fMaxAmount = pTweaks->fPowerupSlideWithBallMaxAmount;
        break;
    }
    case PEN_TYPE_SLIDE_NO_BALL:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMinAmount = pTweaks->fPowerupSlideNoBallMinAmount;
        fMaxAmount = pTweaks->fPowerupSlideNoBallMaxAmount;
        break;
    }
    }

    pData->fPenaltyWorth = InterpolateRangeClamped(fMinAmount, fMaxAmount, 0.0f, 1.0f, nlRandomf(1.0f, &nlDefaultSeed));
    pData->pFouler = this;
    pData->pFoulee = pVictim;
}

/**
 * Offset/Address/Size: 0x8184 | 0x800214C0 | size: 0x12C
 */
void cFielder::DoPositioningInterceptBall()
{
    int nInterceptResult;
    float fInterceptTimes[2];
    nlVector3* pPos = &m_v3Position;

    if (nlGetLengthSquared2D(g_pBall->m_v3Position.f.x - pPos->f.x, g_pBall->m_v3Position.f.y - pPos->f.y) <= 4.0f)
    {
        CalcInterceptXY(*pPos, m_pTweaks->fRunningSpeed, 0.f, g_pBall->m_v3Position, g_pBall->m_v3Velocity, nInterceptResult, fInterceptTimes);

        if (nInterceptResult != 0)
        {
            float fTime;

            if (nInterceptResult == 2)
            {
                fTime = (fInterceptTimes[0] < fInterceptTimes[1]) ? fInterceptTimes[0] : fInterceptTimes[1];
            }
            else
            {
                fTime = fInterceptTimes[0];
            }

            float fInterceptX = g_pBall->m_v3Position.f.x + fTime * g_pBall->m_v3Velocity.f.x;
            float fInterceptY = g_pBall->m_v3Position.f.y + fTime * g_pBall->m_v3Velocity.f.y;

            float dyToIntercept = fInterceptY - m_v3Position.f.y;
            float dxToIntercept = fInterceptX - m_v3Position.f.x;
            float angleRad = nlATan2f(dyToIntercept, dxToIntercept);

            float angle16 = 10430.378f * angleRad;
            u16 targetAngle = (u16)(s32)angle16;

            s16 angleDelta = (s16)(targetAngle - m_aDesiredFacingDirection);
            angleDelta = (angleDelta < 0) ? -angleDelta : angleDelta;

            if ((u16)angleDelta <= 0x3000)
            {
                m_aDesiredFacingDirection = targetAngle;
                s16 tmp = m_aDesiredFacingDirection;
                m_aDesiredMovementDirection = tmp;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x8024 | 0x80021360 | size: 0x160
 */
void cFielder::DoAwardPowerupStuff(eAwardPowerupType ePowerupType, float fParam)
{
    Event* pEvent = g_pEventManager->CreateValidEvent(0x3E, 0x20);
    PowerupData* pData = new ((u8*)pEvent + 0x10) PowerupData();

    float fMinAmount = 0.0f;
    float fMaxAmount = 0.0f;

    switch (ePowerupType)
    {
    case AWARD_POWERUP_POWER_SHOT:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMaxAmount = InterpolateRangeClamped(pTweaks->fPowerupPowerShotMinAmount, pTweaks->fPowerupPowerShotMaxAmount, 0.0f, 0.9f, fParam);
        fMinAmount = fMaxAmount;
        break;
    }
    case AWARD_POWERUP_INTERCEPT_PASS:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMinAmount = pTweaks->fPowerupInterceptPassMinAmount;
        fMaxAmount = pTweaks->fPowerupInterceptPassMaxAmount;
        break;
    }
    case AWARD_POWERUP_PERFECT_PASS:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMinAmount = pTweaks->fPowerupPerfectPassMinAmount;
        fMaxAmount = pTweaks->fPowerupPerfectPassMaxAmount;
        break;
    }
    case AWARD_POWERUP_CONTEXT_DEKE:
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        fMinAmount = pTweaks->fPowerupContextDekeMinAmount;
        fMaxAmount = pTweaks->fPowerupContextDekeMaxAmount;
        break;
    }
    }

    pData->fAwardWorth = InterpolateRangeClamped(fMinAmount, fMaxAmount, 0.0f, 1.0f, nlRandomf(1.0f, &nlDefaultSeed));
    pData->pFielder = this;
}

/**
 * Offset/Address/Size: 0x7E78 | 0x800211B4 | size: 0x1AC
 */
void cFielder::DoCalcShootToScoreResult(float f1, float f2, float f3, float f4, float f5)
{
    eShootToScoreResult result = S2S_SAVED;
    float fAbsDiff = (float)fabs(f2 - f4);

    if (f1 == f3 && fAbsDiff < f5)
    {
        result = S2S_SUPER_SHOT;

        Event* pEvent = g_pEventManager->CreateValidEvent(0x43, 0x1C);
        ShotAtGoalData* pData = new ((u8*)pEvent + 0x10) ShotAtGoalData();
        pData->pShooter = this;

        if (g_e3_Build)
        {
            result = S2S_SCORE;
        }
    }
    else
    {
        g_pEventManager->CreateValidEvent(0x44, 0x14);

        GameInfoManager* pGameInfoManager = nlSingleton<GameInfoManager>::Instance();
        if (pGameInfoManager->IsPerfectStrikesOn())
        {
            result = S2S_SUPER_SHOT;

            Event* pEvent = g_pEventManager->CreateValidEvent(0x43, 0x1C);
            ShotAtGoalData* pData = new ((u8*)pEvent + 0x10) ShotAtGoalData();
            pData->pShooter = this;
        }
        else if (fAbsDiff < mActionShootToScoreVars.fCaptainYellowWidth)
        {
            if (fAbsDiff < f5 && f5 > 0.0f)
            {
                result = S2S_SCORE;
            }
            else
            {
                float fValue = (float)fabs(fAbsDiff - f5) / (mActionShootToScoreVars.fCaptainYellowWidth - f5);

                if (f1 == f3)
                {
                    m_pShotMeter->m_fSTSValue = 0.01f;
                }
                else
                {
                    m_pShotMeter->m_fSTSValue = fValue;
                }

                result = S2S_SAVED_YELLOW;
            }
        }
    }

    meS2SResult = result;
}

/**
 * Offset/Address/Size: 0x7C34 | 0x80020F70 | size: 0x244
 */
cFielder* cFielder::DoFindBestHitTarget()
{
    if (GetGlobalPad() == NULL)
    {
        FuzzyVariant vBestTarget = Fuzzy::GetBestHitTarget(this);
        return (cFielder*)vBestTarget.mData.pPlayer;
    }

    float fBestScore = 99999.0f;
    cFielder* pBestCandidate = NULL;
    cTeam* pTeam = m_pTeam->GetOtherTeam();
    u16 aDirection = m_aActualFacingDirection;

    if (m_pController->GetMovementStickMagnitude())
    {
        aDirection = m_pController->GetMovementStickDirection();
    }

    for (int i = 0; i < 4; i++)
    {
        float fTempScore = 99999.0f;
        cFielder* pCandidate = pTeam->GetFielder(i);

        if (!pCandidate->IsFallenDown(0.0f) && pCandidate->m_tFrozenTimer.m_uPackedTime == 0)
        {
            bool bInvalidTarget = ((pCandidate->m_ePowerup == POWER_UP_STAR)
                                      && (pCandidate->m_tPowerupEffectTime.m_uPackedTime != 0))
                               || pCandidate->mActionShootToScoreVars.isCurrentlyInvincible;

            if (!bInvalidTarget)
            {
                fTempScore = DoFlashLight(
                    pCandidate->m_v3Position,
                    aDirection,
                    g_pGame->m_pGameTweaks->fAngleWeighting,
                    0.0f,
                    100.0f);
            }
        }

        if ((g_pBall->GetOwnerFielder() == pCandidate) || (g_pBall->GetPassTargetFielder() == pCandidate))
        {
            fTempScore *= g_pGame->m_pGameTweaks->fHitCarrierWeighting;
        }

        if (fTempScore < fBestScore)
        {
            pBestCandidate = pCandidate;
            fBestScore = fTempScore;
        }
    }

    if (pBestCandidate == NULL)
    {
        pBestCandidate = GetClosestOpponentFielder(NULL);
    }

    return pBestCandidate;
}

/**
 * Offset/Address/Size: 0x73B8 | 0x800206F4 | size: 0x87C
 */
void cFielder::DoFindBestShotTarget(nlVector3& v3PositionOut, float& fShotSpeed, bool bIsSTS)
{
    cBall* pBall = g_pBall;
    Goalie* pGoalie = m_pTeam->GetOtherTeam()->GetGoalie();

    float kBallAllowance = 0.18f + cNet::m_fNetPostRadius;
    bool bIsChipShot = false;
    if (mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot)
    {
        bIsChipShot = true;
    }
    if (bIsChipShot)
    {
        kBallAllowance += g_pGame->m_pGameTweaks->unk2F4;
    }
    else
    {
        kBallAllowance += g_pGame->m_pGameTweaks->unk2F0;
    }

    float fDist2NetSide = 0.5f * cNet::m_fNetWidth - kBallAllowance;
    cNet* pNet = m_pTeam->GetOtherNet();
    float fNetBaseX = pNet->m_baseLocation.f.x;

    float fBallYClamped = (pBall->m_v3Position.f.y >= -fDist2NetSide) ? pBall->m_v3Position.f.y : -fDist2NetSide;
    fBallYClamped = (fBallYClamped <= fDist2NetSide) ? fBallYClamped : fDist2NetSide;

    float fBallZClamped;
    if (bIsSTS)
    {
        fBallZClamped = pBall->m_v3Position.f.z;
        fBallZClamped = (fBallZClamped >= 0.18f) ? fBallZClamped : 0.18f;
        fBallZClamped = (fBallZClamped <= kBallAllowance) ? fBallZClamped : kBallAllowance;
    }
    else
    {
        float fHeightLimit = cNet::m_fNetHeight - kBallAllowance;
        fBallZClamped = pBall->m_v3Position.f.z;
        fBallZClamped = (fBallZClamped >= 0.18f) ? fBallZClamped : 0.18f;
        fBallZClamped = (fBallZClamped <= fHeightLimit) ? fBallZClamped : fHeightLimit;
    }

    float fDY = pBall->m_v3Position.f.y - fBallYClamped;
    float fDX = pBall->m_v3Position.f.x - fNetBaseX;
    float fShotDist = nlSqrt((fDX * fDX) + (fDY * fDY), true);

    bool bIsChipShot2 = false;
    if (mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot)
    {
        bIsChipShot2 = true;
    }

    float speedFactor;
    float fShotMinSpeed;
    float fShotMaxSpeed;
    if (bIsChipShot2)
    {
        speedFactor = fShotDist;
        fShotMinSpeed = ((FielderTweaks*)m_pTweaks)->fShotChipMinSpeed;
        fShotMaxSpeed = ((FielderTweaks*)m_pTweaks)->fShotChipMaxSpeed;
    }
    else
    {
        speedFactor = fShotDist * (0.3f + m_pShotMeter->m_fSpeedValue);
        fShotMinSpeed = ((FielderTweaks*)m_pTweaks)->fShotMinSpeed;
        fShotMaxSpeed = ((FielderTweaks*)m_pTweaks)->fShotMaxSpeed;
    }

    fShotSpeed = InterpolateRangeClamped(fShotMinSpeed, fShotMaxSpeed, 3.0f, 18.0f, speedFactor);

    float fAbsBallY = (f32)fabs(pBall->m_v3Position.f.y);
    float fAbsBallX = (f32)fabs(pBall->m_v3Position.f.x);
    float fAimValue = m_pShotMeter->mfSShotAimValue;
    float fAbsAimValue = (f32)fabs(fAimValue);

    if (fAbsBallY < 1.5f + fDist2NetSide
        && (fAbsBallX > (f32)fabs(pGoalie->m_v3Position.f.x)
            || fAbsBallX > cField::GetGoalLineX(1u) - 1.5f))
    {
        v3PositionOut.f.x = 1.005f * pNet->m_baseLocation.f.x;
        v3PositionOut.f.y = 0.9f * fBallYClamped;
        v3PositionOut.f.z = fBallZClamped + nlRandomf(0.2f, &nlDefaultSeed);
        if (fShotDist < 2.0f)
        {
            fShotSpeed = 10.0f;
        }
    }
    else
    {
        float fNetBaseY = pNet->m_baseLocation.f.y;
        nlVector3 v3Post1 = pNet->m_baseLocation;
        nlVector3 v3Post2 = pNet->m_baseLocation;
        v3Post1.f.y = fNetBaseY - fDist2NetSide;
        v3Post2.f.y = fNetBaseY + fDist2NetSide;

        float fPost1DY = v3Post1.f.y - pBall->m_v3Position.f.y;
        float fPost2DY = v3Post2.f.y - pBall->m_v3Position.f.y;
        float fPost1DX = v3Post1.f.x - pBall->m_v3Position.f.x;
        float fPost2DX = v3Post2.f.x - pBall->m_v3Position.f.x;
        float fPost1DZ = v3Post1.f.z - pBall->m_v3Position.f.z;
        float fPost2DZ = v3Post2.f.z - pBall->m_v3Position.f.z;
        float fGoalieDY = pGoalie->m_v3Position.f.y - pBall->m_v3Position.f.y;
        float fGoalieDX = pGoalie->m_v3Position.f.x - pBall->m_v3Position.f.x;

        float fAngPost1 = nlATan2f(fPost1DY, fPost1DX);
        float fAngPost2 = nlATan2f(fPost2DY, fPost2DX);
        u16 aAngPost2 = (u16)(s32)(10430.378f * fAngPost2);
        float fAngGoalie = nlATan2f(fGoalieDY, fGoalieDX);
        u16 aAngGoalie = (u16)(s32)(10430.378f * fAngGoalie);
        u16 aAngPost1 = (u16)(s32)(10430.378f * fAngPost1);

        s16 sDiffP1G = (s16)(aAngPost1 - aAngGoalie);
        if (sDiffP1G < 0)
        {
            sDiffP1G = -sDiffP1G;
        }
        u16 uAbsP1G = (u16)sDiffP1G;

        s16 sDiffP2G = (s16)(aAngPost2 - aAngGoalie);
        if (sDiffP2G < 0)
        {
            sDiffP2G = -sDiffP2G;
        }
        u16 uAbsP2G = (u16)sDiffP2G;

        s16 sDiffP1P2 = (s16)(aAngPost1 - aAngPost2);
        if (sDiffP1P2 < 0)
        {
            sDiffP1P2 = -sDiffP1P2;
        }
        u16 uAbsP1P2 = (u16)sDiffP1P2;

        v3PositionOut.f.x = 1.005f * pNet->m_baseLocation.f.x;

        float fProbability;
        if (fAbsAimValue > 0.01f)
        {
            fProbability = 0.5f - 0.5f * fAimValue;
        }
        else if (uAbsP1G >= uAbsP1P2)
        {
            fProbability = 1.0f;
        }
        else if (uAbsP2G >= uAbsP1P2)
        {
            fProbability = 0.0f;
        }
        else
        {
            float fAngToNet = nlATan2f(pNet->m_baseLocation.f.y - pBall->m_v3Position.f.y,
                pNet->m_baseLocation.f.x - pBall->m_v3Position.f.x);
            u16 angle2Net = (u16)(s32)(10430.378f * fAngToNet);
            if (pNet->m_baseLocation.f.x < 0.0f)
            {
                angle2Net += 0x8000;
            }
            s16 sAng2Net = (s16)angle2Net;
            if (sAng2Net < 0)
            {
                sAng2Net = -sAng2Net;
            }

            if ((u16)sAng2Net > 0x2000)
            {
                float fGD1X = pGoalie->m_v3Position.f.x - v3Post1.f.x;
                float fGD1Y = pGoalie->m_v3Position.f.y - v3Post1.f.y;
                float fGD2X = pGoalie->m_v3Position.f.x - v3Post2.f.x;
                float fGD2Y = pGoalie->m_v3Position.f.y - v3Post2.f.y;
                float fGD1Sq = (fGD1X * fGD1X) + (fGD1Y * fGD1Y);
                float fGD2Sq = (fGD2X * fGD2X) + (fGD2Y * fGD2Y);
                float fRatio = fGD1Sq / (fGD1Sq + fGD2Sq);
                fRatio = (fRatio >= 0.03f) ? fRatio : 0.03f;
                fProbability = (fRatio <= 0.97f) ? fRatio : 0.97f;
            }
            else if (3 * uAbsP1G < uAbsP2G || 3 * uAbsP2G < uAbsP1G)
            {
                if (uAbsP1G < uAbsP2G)
                {
                    float fRatio = 0.5f * (int)(3 * uAbsP1G - uAbsP2G) / (int)(uAbsP1G + uAbsP2G);
                    fProbability = (0.05f >= fRatio) ? 0.05f : fRatio;
                }
                else
                {
                    float fRatio = 1.0f - 0.5f * (int)(3 * uAbsP2G - uAbsP1G) / (int)(uAbsP1G + uAbsP2G);
                    fProbability = (0.95f <= fRatio) ? 0.95f : fRatio;
                }
            }
            else
            {
                fProbability = InterpolateRangeClamped(0.15f, 0.85f, -8192.0f, 8192.0f, -(float)(s32)sAng2Net);
            }
        }

        static FilteredRandomChance randgenPost;

        if (randgenPost.genrand(fProbability))
        {
            v3PositionOut.f.y = pBall->m_v3Position.f.y + fPost1DY;
            float fDistPost1Sq = (fPost1DX * fPost1DX) + (fPost1DY * fPost1DY) + (fPost1DZ * fPost1DZ);
            float fDistPost2Sq = (fPost2DX * fPost2DX) + (fPost2DY * fPost2DY) + (fPost2DZ * fPost2DZ);
            if (fDistPost1Sq < fDistPost2Sq)
            {
                v3PositionOut.f.x = 0.985f * pNet->m_baseLocation.f.x;
            }
        }
        else
        {
            v3PositionOut.f.y = pBall->m_v3Position.f.y + fPost2DY;
            float fDistPost2Sq = (fPost2DX * fPost2DX) + (fPost2DY * fPost2DY) + (fPost2DZ * fPost2DZ);
            float fDistPost1Sq = (fPost1DX * fPost1DX) + (fPost1DY * fPost1DY) + (fPost1DZ * fPost1DZ);
            if (fDistPost2Sq < fDistPost1Sq)
            {
                v3PositionOut.f.x = 0.985f * pNet->m_baseLocation.f.x;
            }
        }

        if (bIsSTS)
        {
            v3PositionOut.f.z = kBallAllowance + nlRandomf(0.5f, &nlDefaultSeed);
        }
        else if (fShotDist > g_pGame->m_pGameTweaks->unk2F8)
        {
            v3PositionOut.f.z = cNet::m_fNetHeight - kBallAllowance;
        }
        else
        {
            bool bIsChip = false;
            if (mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot)
            {
                bIsChip = true;
            }
            if (bIsChip)
            {
                float fAllowableHeight = cNet::m_fNetHeight - kBallAllowance;
                v3PositionOut.f.z = 0.9f * fAllowableHeight + nlRandomf(0.100000024f * fAllowableHeight, &nlDefaultSeed);
            }
            else
            {
                v3PositionOut.f.z = kBallAllowance + nlRandomf(cNet::m_fNetHeight - 2.0f * kBallAllowance, &nlDefaultSeed);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x6E28 | 0x80020164 | size: 0x590
 */
void cFielder::DoRegularShooting()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x6DE4 | 0x80020120 | size: 0x44
 */
void cFielder::DoResetShotMeter(float fTime)
{
    m_pShotMeter->Reset();
    m_pShotMeter->m_fTime = fTime;
}

/**
 * Offset/Address/Size: 0x6DD0 | 0x8002010C | size: 0x14
 */
bool cFielder::IsActionDone() const
{
    return (u8)(m_eActionState == ACTION_NEED_ACTION);
}

/**
 * Offset/Address/Size: 0x6D94 | 0x800200D0 | size: 0x3C
 */
void cFielder::SetAction(eFielderActionState actionState)
{
    CleanUpAction();
    m_eActionState = actionState;
}

/**
 * Offset/Address/Size: 0x6D58 | 0x80020094 | size: 0x3C
 */
bool cFielder::GetFormationPosition(nlVector3& v3DestPosition, float fBallPosFormationWeight)
{
    if (fBallPosFormationWeight < 0.0f)
    {
        fBallPosFormationWeight = 1.0f;
    }

    return m_pTeam->CalculateFormationPosition(v3DestPosition, this, m_DesireCommonVars.bInPosition, fBallPosFormationWeight);
}

/**
 * Offset/Address/Size: 0x6D4C | 0x80020088 | size: 0xC
 */
const LooseBallContactAnimInfo* GetOneTimerLeadGroundContactAnims()
{
    return gOneTimerLeadGroundContactAnims;
}

/**
 * Offset/Address/Size: 0x6C1C | 0x8001FF58 | size: 0x130
 */
LooseBallContactAnimInfo* cFielder::GetOneTimerBallContactAnimInfo(unsigned short aFutureFacingDirection, const nlVector3& v3FuturePosition, const nlVector3& v3OneTimerTarget, bool bLeadPass, bool bVolleyPass)
{
    const LooseBallContactAnimInfo* pBallContactAnimInfo;
    int nNumContactAnims;

    if (bLeadPass)
    {
        if (bVolleyPass)
        {
            nNumContactAnims = 4;
            pBallContactAnimInfo = gOneTimerLeadVolleyContactAnims;
        }
        else
        {
            nNumContactAnims = 4;
            pBallContactAnimInfo = gOneTimerLeadGroundContactAnims;
        }
    }
    else
    {
        if (bVolleyPass)
        {
            nNumContactAnims = 4;
            pBallContactAnimInfo = gOneTimerIdleVolleyContactAnims;
        }
        else
        {
            nNumContactAnims = 4;
            pBallContactAnimInfo = gOneTimerIdleGroundContactAnims;
        }
    }

    u16 aNetAngle = (u16)(s32)(10430.378f * nlATan2f(v3OneTimerTarget.f.y - v3FuturePosition.f.y, v3OneTimerTarget.f.x - v3FuturePosition.f.x)) - aFutureFacingDirection;

    LooseBallContactAnimInfo* pBestBallContactAnimInfo = NULL;

    for (int i = 0; i < nNumContactAnims; i++)
    {
        if (pBallContactAnimInfo[i].aIncomingAngleMin < pBallContactAnimInfo[i].aIncomingAngleMax)
        {
            if ((aNetAngle >= pBallContactAnimInfo[i].aIncomingAngleMin) && (aNetAngle <= pBallContactAnimInfo[i].aIncomingAngleMax))
            {
                pBestBallContactAnimInfo = (LooseBallContactAnimInfo*)&pBallContactAnimInfo[i];
            }
        }
        else if ((aNetAngle >= pBallContactAnimInfo[i].aIncomingAngleMin) || (aNetAngle <= pBallContactAnimInfo[i].aIncomingAngleMax))
        {
            pBestBallContactAnimInfo = (LooseBallContactAnimInfo*)&pBallContactAnimInfo[i];
        }
    }
    return pBestBallContactAnimInfo;
}

/**
 * Offset/Address/Size: 0x6A00 | 0x8001FD3C | size: 0xEC
 */
void cFielder::GetReceivePassBallContactOffset(nlVector3& v3Offset, unsigned short aFacingDirection, const LooseBallContactAnimInfo* pBestBallContactAnimInfo)
{
    nlVector3 v3ContactOffsetLocal;
    const cSAnim* guessContactAnim = m_pAnimInventory->GetAnim(pBestBallContactAnimInfo->nAnimID);

    GetJointPositionFuture(&v3ContactOffsetLocal, pBestBallContactAnimInfo->nAnimID, m_nBallJointIndex, pBestBallContactAnimInfo->fAnimContactFrame / (float)guessContactAnim->m_nNumKeys, false, true, false);

    float cos, sin;
    nlSinCos(&sin, &cos, aFacingDirection);

    v3Offset.f.x = v3ContactOffsetLocal.f.x * cos - v3ContactOffsetLocal.f.y * sin;
    v3Offset.f.y = v3ContactOffsetLocal.f.y * cos + v3ContactOffsetLocal.f.x * sin;
    v3Offset.f.z = v3ContactOffsetLocal.f.z;
}

/**
 * Offset/Address/Size: 0x6AEC | 0x8001FE28 | size: 0x130
 * TODO: 99.47% match - immediate-only diffs remain for contact anim table symbols and the angle scale float label
 */
const LooseBallContactAnimInfo* cFielder::GetReceivePassBallContactAnimInfo(cBall* pBall, const nlVector3& rv3Pos, unsigned short aAngle, bool bLeadPass, bool bVolleyPass)
{
    const LooseBallContactAnimInfo* pBallContactAnimInfo;
    int nNumContactAnims;

    if (bLeadPass)
    {
        if (bVolleyPass)
        {
            nNumContactAnims = 3;
            pBallContactAnimInfo = LeadVolleyContactAnims;
        }
        else
        {
            nNumContactAnims = 4;
            pBallContactAnimInfo = LeadGroundContactAnims;
        }
    }
    else
    {
        if (bVolleyPass)
        {
            nNumContactAnims = 1;
            pBallContactAnimInfo = IdleVolleyContactAnims;
        }
        else
        {
            nNumContactAnims = 1;
            pBallContactAnimInfo = IdleGroundContactAnims;
        }
    }

    u16 aNetAngle = (u16)(s32)(10430.378f * nlATan2f(pBall->m_v3Position.f.y - rv3Pos.f.y, pBall->m_v3Position.f.x - rv3Pos.f.x)) - aAngle;
    const LooseBallContactAnimInfo* pBestBallContactAnimInfo = NULL;
    for (int i = 0; i < nNumContactAnims; i++)
    {
        if (pBallContactAnimInfo[i].aIncomingAngleMin < pBallContactAnimInfo[i].aIncomingAngleMax)
        {
            if ((aNetAngle >= pBallContactAnimInfo[i].aIncomingAngleMin) && (aNetAngle <= pBallContactAnimInfo[i].aIncomingAngleMax))
            {
                pBestBallContactAnimInfo = &pBallContactAnimInfo[i];
            }
        }
        else if ((aNetAngle >= pBallContactAnimInfo[i].aIncomingAngleMin) || (aNetAngle <= pBallContactAnimInfo[i].aIncomingAngleMax))
        {
            pBestBallContactAnimInfo = &pBallContactAnimInfo[i];
        }
    }
    return pBestBallContactAnimInfo;
}
/**
 * Offset/Address/Size: 0x68F0 | 0x8001FC2C | size: 0x110
 */
bool cFielder::IsFallenDown(float fThreshold) const
{
    s32 nAnimID = m_eAnimID;
    u32 nIndex = (u32)(nAnimID - 0x5c);
    float fThresholdValue = 0.0f;

    switch (nIndex)
    {
    case 0:
        fThresholdValue = 67.0f;
        break;
    case 1:
        fThresholdValue = 64.0f;
        break;
    case 2:
        fThresholdValue = 34.0f;
        break;
    case 3:
        fThresholdValue = 32.0f;
        break;
    case 4:
        fThresholdValue = 42.0f;
        break;
    case 5:
        fThresholdValue = 46.0f;
        break;
    case 6:
        fThresholdValue = 43.0f;
        break;
    case 7:
        fThresholdValue = 30.0f;
        break;
    case 8:
        fThresholdValue = 20.0f;
        if (fThresholdValue < fThreshold)
        {
            return false;
        }
        break;
    case 9:
        fThresholdValue = 43.0f;
        break;
    case 10:
        fThresholdValue = 46.0f;
        break;
    case 11:
        fThresholdValue = 61.0f;
        break;
    case 12:
        fThresholdValue = 108.0f;
        break;
    case 13:
    {
        fThresholdValue = (float)(m_pCurrentAnimController->m_pSAnim->m_nNumKeys);
        break;
    }
    case 14:
        fThresholdValue = 29.0f;
        break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
        break;
    }

    return m_pCurrentAnimController->m_fTime < (fThresholdValue / m_pCurrentAnimController->m_pSAnim->m_nNumKeys);
}

/**
 * Offset/Address/Size: 0x6868 | 0x8001FBA4 | size: 0x88
 */
bool cFielder::IsHitting() const
{
    const cPN_SAnimController* pAnimController = m_pCurrentAnimController;
    const float fAnimTime = 30.0f * pAnimController->m_fTime * (pAnimController->m_pSAnim->m_nNumKeys / 30.0f);
    bool isHitting = ((m_tFrozenTimer.m_uPackedTime == 0) && (m_eActionState == ACTION_HIT) && (fAnimTime >= 4.0f) && (fAnimTime <= 14.0f));

    return isHitting;
}

/**
 * Offset/Address/Size: 0x682C | 0x8001FB68 | size: 0x3C
 */
bool cFielder::IsSlideTackling() const
{
    if ((m_tFrozenTimer.m_uPackedTime == 0) && (m_eActionState == ACTION_SLIDE_ATTACK) && ((mActionSlideAttackVars.eSlideAttackState == SLIDE_ATTACK_DOWN || mActionSlideAttackVars.eSlideAttackState == SLIDE_ATTACK_DECELERATE)))
    {
        return true;
    }
    return false;
}

/**
 * Offset/Address/Size: 0x681C | 0x8001FB58 | size: 0x10
 */
bool cFielder::IsStriker() const
{
    return m_eRole == ROLE_STRIKER;
}

/**
 * Offset/Address/Size: 0x6808 | 0x8001FB44 | size: 0x14
 */
bool cFielder::IsWinger() const
{
    return m_eRole == ROLE_WINGER;
}

/**
 * Offset/Address/Size: 0x67F4 | 0x8001FB30 | size: 0x14
 */
bool cFielder::IsMidField() const
{
    return m_eRole == ROLE_MIDFIELD;
}

/**
 * Offset/Address/Size: 0x67E0 | 0x8001FB1C | size: 0x14
 */
bool cFielder::IsDefense() const
{
    return m_eRole == ROLE_DEFENCE;
}

/**
 * Offset/Address/Size: 0x67CC | 0x8001FB08 | size: 0x14
 */
bool cFielder::IsFrozen() const
{
    return m_tFrozenTimer.m_uPackedTime != 0;
}

/**
 * Offset/Address/Size: 0x6700 | 0x8001FA3C | size: 0xCC
 */
void cFielder::SetFrozen(float seconds)
{
    static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

    m_tFrozenTimer.SetSeconds(seconds);
    m_fDesiredSpeed = 0.0f;
    m_fActualSpeed = m_fDesiredSpeed;
    SetVelocity(v3Zero);

    if (seconds > 0.0f)
    {
        if (m_eFielderDesireState != FIELDERDESIRE_FINISH_ACTION)
        {
            g_pGame->AbortPendingThought(mThoughtHashCalcDesire);
            g_pGame->AbortPendingThought(mThoughtHashInitRunToNet);
            g_pGame->AbortPendingThought(mThoughtHashInitGetOpen);
            g_pGame->AbortPendingThought(mThoughtHashInitWindupPass);
            g_pGame->AbortPendingThought(mThoughtHashInitCutAndBreak);
            ClearQueuedDesire();
            EndDesire(false);
        }
    }
}

/**
 * Offset/Address/Size: 0x6310 | 0x8001F64C | size: 0x3F0
 */
float cFielder::DoFindBestSlideAttackTarget(nlVector3& v3PositionOut, nlVector3& v3VelocityOut)
{
    int nNumSolutions;
    float pSolutions[2];
    nlVector3 v3BallVelocity;
    nlVector3 v3BallPosition;
    nlVector3 v3LandingSpot;

    v3BallVelocity = g_pBall->m_v3Velocity;
    v3BallPosition = g_pBall->m_v3Position;

    if (g_pBall->GetOwnerFielder() != NULL && g_pBall->GetOwnerFielder()->m_eActionState == ACTION_DEKE)
    {
        v3BallVelocity = v3Zero;
    }
    else if (g_pBall->GetOwnerFielder() != NULL && g_pBall->GetOwnerFielder()->m_eActionState == ACTION_SHOOT_TO_SCORE)
    {
        v3BallVelocity = v3Zero;
        v3BallPosition = g_pBall->GetOwnerFielder()->m_v3Position;
    }

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    const nlVector3& myPos = m_v3Position;
    float fSpeed = 1.0f;
    float fSlideSpeed = pTweaks->fRunningWBTurboSpeedLevel2;

    if (fSlideSpeed >= 0.0f)
    {
        switch (m_ePowerup)
        {
        case POWER_UP_MUSHROOM:
            fSpeed *= g_pGame->m_pGameTweaks->fMushroomSpeed;
            break;
        case POWER_UP_STAR:
            fSpeed *= g_pGame->m_pGameTweaks->fStarSpeed;
            break;
        }
    }

    fSpeed *= fSlideSpeed;

    switch (m_ePowerup)
    {
    case POWER_UP_MUSHROOM:
    case POWER_UP_STAR:
        fSpeed *= 1.4f;
        break;
    }

    CalcInterceptXY(myPos, fSpeed, pTweaks->fPhysCapsuleRadius, v3BallPosition, v3BallVelocity, nNumSolutions, pSolutions);

    float t;

    if (nNumSolutions != 0)
    {
        if (nNumSolutions == 2)
        {
            t = (pSolutions[0] < pSolutions[1]) ? pSolutions[0] : pSolutions[1];
        }
        else
        {
            t = pSolutions[0];
        }

        if (g_pBall->m_pPassTarget != NULL)
        {
            if (g_pBall->m_pPrevOwner->m_tBallUnPossessionTimer.GetSeconds() > 0.5f)
            {
                float dx1 = g_pBall->m_v3Position.f.x - g_pBall->m_v3PassIntercept.f.x;
                float dy1 = g_pBall->m_v3Position.f.y - g_pBall->m_v3PassIntercept.f.y;
                float fDistToPassIntercept = nlSqrt(dx1 * dx1 + dy1 * dy1, true);

                float predX = g_pBall->m_v3Position.f.x + t * g_pBall->m_v3Velocity.f.x;
                float predY = g_pBall->m_v3Position.f.y + t * g_pBall->m_v3Velocity.f.y;
                float dx2 = g_pBall->m_v3Position.f.x - predX;
                float dy2 = g_pBall->m_v3Position.f.y - predY;
                float fDistToPredicted = nlSqrt(dx2 * dx2 + dy2 * dy2, true);

                if (fDistToPassIntercept < fDistToPredicted || g_pBall->m_v3Position.f.z > 1.0f)
                {
                    v3PositionOut = g_pBall->m_v3PassIntercept;
                    v3VelocityOut = v3Zero;
                    return 0.0f;
                }
            }
        }

        if (v3BallPosition.f.z > 1.0f)
        {
            float fLandingTime = g_pBall->PredictLandingSpotAndTime(v3LandingSpot);
            v3PositionOut = v3LandingSpot;
            v3VelocityOut = v3Zero;
            return fLandingTime;
        }
    }
    else
    {
        GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;
        cPlayer* pPassTarget = g_pBall->m_pPassTarget;
        t = pGameTweaks->unk2A4 + pGameTweaks->unk2A8;

        if (pPassTarget != NULL)
        {
            v3PositionOut = pPassTarget->m_v3Position;
            v3VelocityOut = g_pBall->m_pPassTarget->m_v3Velocity;
            return t;
        }
    }

    v3PositionOut = v3BallPosition;
    v3VelocityOut = v3BallVelocity;
    return t;
}

/**
 * Offset/Address/Size: 0x62DC | 0x8001F618 | size: 0x34
 */
bool cFielder::CanPickupBall(cBall* pBall)
{
    if (m_tFrozenTimer.m_uPackedTime != 0)
    {
        return false;
    }
    return cPlayer::CanPickupBall(pBall);
}

/**
 * Offset/Address/Size: 0x6284 | 0x8001F5C0 | size: 0x58
 */
bool cFielder::CanBeBlownUp()
{
    switch (m_eActionState)
    {
    case ACTION_ELECTROCUTION:
        // Check if anim ID is 0x75 or 0x76
        if (m_eAnimID == 0x76 || m_eAnimID == 0x75)
        {
            return true;
        }
        return false;

    case ACTION_STS_HIT_REACT:
    case ACTION_SQUISH_REACT:
        return false;

    default:
        return true;
    }
}

/**
 * Offset/Address/Size: 0x6230 | 0x8001F56C | size: 0x54
 */
void cFielder::CanBreakOutOfSlideTackle()
{
    if (m_eActionState == ACTION_SLIDE_ATTACK)
    {
        if (mActionSlideAttackVars.bAttackSucceeded != 0)
        {
            m_tSlideAttackTimer.SetSeconds(0.0f);
            mActionSlideAttackVars.eSlideAttackState = SLIDE_ATTACK_DECELERATE;
        }
    }
}

/**
 * Offset/Address/Size: 0x5F54 | 0x8001F290 | size: 0x2DC
 */
eStrafeDirection cFielder::CalculateStrafeDirection(unsigned short aDesiredFacingDir, unsigned short aDesiredMovementDir)
{
    s32 lastStrafeDirection = mActionRunningVars.eLastStrafeDirection;
    s16 angleDelta = (s16)(aDesiredMovementDir - aDesiredFacingDir);
    float strafeToRunDelta;
    float backwardsToStrafeRunDelta;

    switch (lastStrafeDirection)
    {
    case STRAFE_RIGHT:
    case STRAFE_LEFT:
        strafeToRunDelta = (float)g_pGame->m_pGameTweaks->nStrafeToRunOutDirectionDelta;
        backwardsToStrafeRunDelta = (float)g_pGame->m_pGameTweaks->nBackwardsToStrafeRunOutDirectionDelta;
        break;

    default:
        strafeToRunDelta = (float)g_pGame->m_pGameTweaks->nStrafeToRunInDirectionDelta;
        backwardsToStrafeRunDelta = (float)g_pGame->m_pGameTweaks->nBackwardsToStrafeRunInDirectionDelta;
        break;
    }

    float zero = 0.0f;
    float desiredSpeed = m_fDesiredSpeed;

    if (!(desiredSpeed > zero))
    {
        return STRAFE_IDLE;
    }

    float runningSpeed = zero + m_pTweaks->fRunningSpeed;
    if (desiredSpeed > runningSpeed)
    {
        float swapFacingSeconds = m_tSwapFacingTimer.GetSeconds();
        if (swapFacingSeconds == zero)
        {
            return STRAFE_FORWARD;
        }

        if ((float)(u16)((angleDelta < 0) ? -angleDelta : angleDelta) < strafeToRunDelta)
        {
            return STRAFE_FORWARD;
        }

        if ((float)angleDelta > -backwardsToStrafeRunDelta)
        {
            if ((float)angleDelta <= strafeToRunDelta)
            {
                return STRAFE_RIGHT;
            }
        }

        if ((float)angleDelta < backwardsToStrafeRunDelta)
        {
            if ((float)angleDelta >= strafeToRunDelta)
            {
                return STRAFE_LEFT;
            }
        }

        return STRAFE_BACK;
    }

    if ((float)(u16)((angleDelta < 0) ? -angleDelta : angleDelta) < strafeToRunDelta)
    {
        return STRAFE_FORWARD;
    }

    if ((float)angleDelta > -backwardsToStrafeRunDelta)
    {
        if ((float)angleDelta <= strafeToRunDelta)
        {
            return STRAFE_RIGHT;
        }
    }

    if ((float)angleDelta < backwardsToStrafeRunDelta)
    {
        if ((float)angleDelta >= strafeToRunDelta)
        {
            return STRAFE_LEFT;
        }
    }

    return STRAFE_BACK;
}

/**
 * Offset/Address/Size: 0x5DF0 | 0x8001F12C | size: 0x164
 */
void cFielder::CalcPointOnPerimeter(nlVector3& dest, const nlVector3& fromPoint, float fFutureTimeDelta)
{
    float fMinDistance = 1.0f + m_pTweaks->fPhysCapsuleRadius;
    nlVector3 v3Position;

    if (fFutureTimeDelta > 0.0f)
    {
        v3Position.f.x = m_v3Position.f.x + fFutureTimeDelta * m_v3Velocity.f.x;
        v3Position.f.y = m_v3Position.f.y + fFutureTimeDelta * m_v3Velocity.f.y;
        v3Position.f.z = m_v3Position.f.z + fFutureTimeDelta * m_v3Velocity.f.z;
    }
    else
    {
        v3Position = m_v3Position;
    }

    float dx = fromPoint.f.x - v3Position.f.x;
    float dy = fromPoint.f.y - v3Position.f.y;
    float dz = fromPoint.f.z - v3Position.f.z;
    dest.f.x = dx;
    dest.f.y = dy;
    dest.f.z = dz;

    float fInvLen = nlRecipSqrt(dest.f.x * dest.f.x + dest.f.y * dest.f.y + dest.f.z * dest.f.z, true);

    float nx = fInvLen * dest.f.x;
    float nz = fInvLen * dest.f.z;
    float ny = fInvLen * dest.f.y;
    dest.f.x = nx;
    dest.f.y = ny;
    dest.f.z = nz;

    float rx = v3Position.f.x + fMinDistance * dest.f.x;
    float rz = v3Position.f.z + fMinDistance * dest.f.z;
    float ry = v3Position.f.y + fMinDistance * dest.f.y;
    dest.f.x = rx;
    dest.f.y = ry;
    dest.f.z = rz;
}

/**
 * Offset/Address/Size: 0x5DA4 | 0x8001F0E0 | size: 0x4C
 */
void cFielder::ClearTimers()
{
    m_tPowerupEffectTime.SetSeconds(0.0f);
    mtBombImpactTime.SetSeconds(0.0f);
    m_tFrozenTimer.SetSeconds(0.0f);
}

/**
 * Offset/Address/Size: 0x5DA0 | 0x8001F0DC | size: 0x4
 */
void cFielder::ClearVolleyPass()
{
}

/**
 * Offset/Address/Size: 0x5B34 | 0x8001EE70 | size: 0x26C
 * TODO: 98.00% match - r29/r31 register swap (this vs team-name str pointer)
 * through BasicString construction/copy loop
 */
void cFielder::CleanActionShootToScore()
{
    FORCE_DONT_INLINE;
    extern void SetTimeScale__18ParticleUpdateTaskFf(float);
    extern void* Instance__14WorldDarkeningFv();
    extern void Fade__14WorldDarkeningFff(void*, float, float);
    extern void RenderAllCharacters__17DrawableCharacterFv();
    extern char* GetTeamName__F7eTeamID(eTeamID);
    extern void Remove__14cCameraManagerFRC11cBaseCamera(void*);
    extern void AppendInPlace__45BasicString_c_Q26Detail19TempStringAllocator_FPCc(BasicString<char, Detail::TempStringAllocator>*, const char*);

    extern unsigned char instance__17ShootToScoreMeter[];
    extern unsigned char sSTSLighting__17DrawableCharacter;
    extern unsigned char sbIsHyperShootToScoreRenderingEnabled__5World;

    cFielder* pSelf = this;

    Audio::FadeFilterFromCurrentToZero();
    FixedUpdateTask::mTimeScale = 1.0f;
    SetTimeScale__18ParticleUpdateTaskFf(1.0f);
    g_pEventManager->CreateValidEvent(0x41, 0x14);

    pSelf->mActionShootToScoreVars.isCurrentlyInvincible = false;
    pSelf->mActionShootToScoreVars.isInUnbreakablePart = false;

    Fade__14WorldDarkeningFff(Instance__14WorldDarkeningFv(), 1.0f, 0.0f);
    instance__17ShootToScoreMeter[0x28] = 0;
    RenderAllCharacters__17DrawableCharacterFv();
    sSTSLighting__17DrawableCharacter = 0;
    g_pGame->mbCaptainShotToScoreOn = false;

    g_pBall->m_pDrawableBall->m_uObjectFlags &= ~0x40;

    BasicString<char, Detail::TempStringAllocator> effectName(GetTeamName__F7eTeamID(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((s16)pSelf->m_pTeam->m_nSide)));
    AppendInPlace__45BasicString_c_Q26Detail19TempStringAllocator_FPCc(&effectName, "shoot_to_score_shot");
    EffectsGroup* pGroup = fxGetGroup(effectName.c_str());
    pSelf->KillEffect(pGroup);
    pSelf->KillEffect(fxGetGroup("shoot_to_score_hyper"));

    if (pSelf->mActionShootToScoreVars.captainStsCamera != NULL)
    {
        Remove__14cCameraManagerFRC11cBaseCamera(pSelf->mActionShootToScoreVars.captainStsCamera);
        delete pSelf->mActionShootToScoreVars.captainStsCamera;
        pSelf->mActionShootToScoreVars.captainStsCamera = NULL;
    }

    SetTimeScale__18ParticleUpdateTaskFf(1.0f);
    sbIsHyperShootToScoreRenderingEnabled__5World = 0;
}

/**
 * Offset/Address/Size: 0x5A64 | 0x8001EDA0 | size: 0xD0
 */
void cFielder::SetAttemptOneTouchPass()
{
    bool shouldAttempt = false;

    cGlobalPad* pad = GetGlobalPad();
    if (pad != NULL)
    {
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        float pressure = GetGlobalPad()->GetPressure(0x15, true);
        if (pressure > tweaks->unk2B0)
        {
            shouldAttempt = true;
        }
    }

    m_DesireReceivePassSharedVars.iAttemptOneTouchPass = shouldAttempt ? 2 : 1;

    m_DesireReceivePassSharedVars.pOneTouchPassTarget = DoFindBestPassTarget(ReceivingVolleyPass(this), shouldAttempt);
    m_DesireReceivePassSharedVars.iAttemptOneTouchShot = 0;
}

/**
 * Offset/Address/Size: 0x59C0 | 0x8001ECFC | size: 0xA4
 */
void cFielder::SetAttemptOneTouchShot()
{
    bool shouldAttempt = false;

    cGlobalPad* pad = GetGlobalPad();
    if (pad != NULL)
    {
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        float pressure = GetGlobalPad()->GetPressure(0x15, true);
        if (pressure > tweaks->unk2B0)
        {
            shouldAttempt = true;
        }
    }

    m_DesireReceivePassSharedVars.iAttemptOneTouchShot = shouldAttempt ? 2 : 1;
    m_DesireReceivePassSharedVars.iAttemptOneTouchPass = 0;
    m_DesireReceivePassSharedVars.pOneTouchPassTarget = NULL;
}

/**
 * Offset/Address/Size: 0x595C | 0x8001EC98 | size: 0x64
 */
s16 cFielder::GetOneTouchShotDesire()
{
    float fZero = 0.f;
    float fResult = ReceivingPass(this);
    if (fResult != fZero)
    {
        return ((m_DesireReceivePassSharedVars.iAttemptOneTouchShot != 0) || (m_DesireReceivePassSharedVars.iAttemptOneTouchPass != 0));
    }

    return 0;
}

/**
 * Offset/Address/Size: 0x573C | 0x8001EA78 | size: 0x220
 * Match: 99.85% - all i diffs (SDA offsets / static symbol naming only)
 */
void cFielder::SetStartAnimState(int animState)
{
    static int RunStartAnims[4] = { 0, 0x10, 0x0F, 0x11 };

    if (animState != -1)
    {
        SetAnimState(RunStartAnims[animState], true, 0.2f, false, false);

        s16 turnAdjust = CalcAnimTurnAdjust(m_aActualFacingDirection, m_aDesiredFacingDirection, m_eAnimID);
        InitMovementFromAnim(turnAdjust, v3Zero, 1.0f, false);

        m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.5f;
    }
    else
    {
        int nAnimState = ((m_aDesiredFacingDirection - m_aActualFacingDirection + 0x2000) >> 14) & 3;

        if (nAnimState != 0)
        {
            SetAnimState(RunStartAnims[nAnimState], true, 0.2f, false, false);

            s16 turnAdjust = CalcAnimTurnAdjust(m_aActualFacingDirection, m_aDesiredFacingDirection, m_eAnimID);
            InitMovementFromAnim(turnAdjust, v3Zero, 1.0f, false);

            m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.5f;
        }
        else
        {
            int RunningAnims[3] = { 0x1B, 0x1A, 0x1C };

            SetRunLeanSAB(RunningAnims, 3, 1);

            FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
            InitMovementRunning(pTweaks->fRunningDirectionSeekSpeed, pTweaks->fRunningDirectionSeekFalloff, pTweaks->fRunningAccel, pTweaks->fRunningDecel);
        }
    }
}

/**
 * Offset/Address/Size: 0x5618 | 0x8001E954 | size: 0x124
 */
void cFielder::SetWindupWBAnimState()
{
    cNet* pOtherNet = m_pTeam->GetOtherNet();
    s16 facingDelta = GetFacingDeltaToPosition(pOtherNet->m_baseLocation);

    if (facingDelta < 0)
    {
        SetAnimState(0x57, true, 0.2f, false, false);
    }
    else
    {
        SetAnimState(0x56, true, 0.2f, false, false);
    }

    m_fDesiredSpeed = 0.0f;
    if (m_fActualSpeed > ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed)
    {
        m_fActualSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed;
    }

    InitMovementRunningNoTurn(0.0f, ((FielderTweaks*)m_pTweaks)->fShotWindupDecel);

    Event* pEvent = g_pEventManager->CreateValidEvent(0x15, 0x1c);
    ShotAtGoalData* pData = new ((u8*)pEvent + 0x10) ShotAtGoalData();
    pData->pShooter = this;

    Play3DSFX(Audio::CHARSFX_SHOT_WINDUP, PHYSOBJ, 100.0f);
    Play3DSFX(Audio::CHARSFX_EFFORTS_HEAD_SHAKE, PHYSOBJ, 100.0f);

    EmitWindupAtBall(this, "ball_shot_windup");
}

/**
 * Offset/Address/Size: 0x5464 | 0x8001E7A0 | size: 0x1B4
 * Match: 99.91% - all i diffs (SDA offsets / static symbol naming only)
 */
void cFielder::SetStartWBAnimState()
{
    static int RunWBStartAnims[4] = { 0, 0x37, 0x38, 0x39 };

    int nAnimState = ((m_aDesiredFacingDirection - m_aActualFacingDirection + 0x2000) >> 14) & 3;

    if (nAnimState != 0)
    {
        SetAnimState(RunWBStartAnims[nAnimState], true, 0.15f, false, false);

        s16 turnAdjust = CalcAnimTurnAdjust(m_aActualFacingDirection, m_aDesiredFacingDirection, m_eAnimID);
        InitMovementFromAnim(turnAdjust, v3Zero, 1.0f, false);

        m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.5f;
    }
    else
    {
        int RunningWBAnims[3] = { 0x1B, 0x1A, 0x1C };

        SetRunLeanSAB(RunningWBAnims, 3, 1);

        FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
        InitMovementRunning(pTweaks->fRunningWBDirectionSeekSpeed, pTweaks->fRunningWBDirectionSeekFalloff, pTweaks->fRunningWBAccel, pTweaks->fRunningWBDecel);
    }
}

/**
 * Offset/Address/Size: 0x53E0 | 0x8001E71C | size: 0x84
 */
void cFielder::SetRunTurboAnimState(int animID, bool bForceMirrorSwap)
{
    if (animID == 0x1D)
    {
        EmitTurbo(this, "footy_burst");
    }

    SetAnimState(animID, false, 0.067f, false, bForceMirrorSwap);

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    InitMovementRunningNoTurn(pTweaks->fRunningWBTurboAccel, pTweaks->fRunningWBTurboDecel);
}

/**
 * Offset/Address/Size: 0x5350 | 0x8001E68C | size: 0x90
 */
void cFielder::SetHardStopAnimState()
{
    if (m_pBall != nullptr)
    {
        SetAnimState(0x24, true, 0.2f, false, false);
    }
    else
    {
        SetAnimState(0x12, true, 0.2f, false, false);
    }
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.4f;
}

/**
 * Offset/Address/Size: 0x52C0 | 0x8001E5FC | size: 0x90
 */
void cFielder::SetHardStopRecoverAnimState()
{
    if (m_pBall != nullptr)
    {
        SetAnimState(0x26, false, 0.03f, false, false);
    }
    else
    {
        SetAnimState(0x14, false, 0.03f, false, false);
    }
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.4f;
}

/**
 * Offset/Address/Size: 0x5230 | 0x8001E56C | size: 0x90
 */
void cFielder::SetHardStopTurnAnimState()
{
    if (m_pBall != nullptr)
    {
        SetAnimState(0x25, false, 0.03f, false, false);
    }
    else
    {
        SetAnimState(0x13, false, 0.03f, false, false);
    }
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.4f;
}

/**
 * Offset/Address/Size: 0x51CC | 0x8001E508 | size: 0x64
 */
void cFielder::SetRunBackwardsAnimState()
{
    SetAnimState(0x27, true, 0.067f, true, false);

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    InitMovementStrafing(pTweaks->fRunningStrafeDirectionSeekSpeed, pTweaks->fRunningStrafeDirectionSeekFalloff, pTweaks->fRunningStrafeAccel, pTweaks->fRunningStrafeDecel);

    m_aActualMovementDirection = m_aDesiredMovementDirection;
}

/**
 * Offset/Address/Size: 0x515C | 0x8001E498 | size: 0x70
 */
void cFielder::SetRunToBackRunningAnimState()
{
    SetAnimState(0x2d, true, 0.067f, true, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_aActualMovementDirection = m_aDesiredMovementDirection;
    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.25f;
}

/**
 * Offset/Address/Size: 0x50EC | 0x8001E428 | size: 0x70
 */
void cFielder::SetBackRunningToRunAnimState()
{
    SetAnimState(0x2e, true, 0.067f, true, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_aActualMovementDirection = m_aDesiredMovementDirection;
    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.25f;
}

/**
 * Offset/Address/Size: 0x507C | 0x8001E3B8 | size: 0x70
 */
void cFielder::SetBackRunningStopAnimState()
{
    SetAnimState(0x2a, true, 0.067f, true, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_aActualMovementDirection = m_aDesiredMovementDirection;
    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.25f;
}

/**
 * Offset/Address/Size: 0x500C | 0x8001E348 | size: 0x70
 */
void cFielder::SetBackRunningStopStartAnimState()
{
    SetAnimState(0x2b, true, 0.067f, true, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_aActualMovementDirection = m_aDesiredMovementDirection;
    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.25f;
}

/**
 * Offset/Address/Size: 0x4FA8 | 0x8001E2E4 | size: 0x64
 */
void cFielder::SetBackRunningStopRecoverAnimState()
{
    SetAnimState(0x2c, true, 0.067f, true, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_aActualMovementDirection = m_aDesiredMovementDirection;
}

/**
 * Offset/Address/Size: 0x4F44 | 0x8001E280 | size: 0x64
 */
void cFielder::SetStopAnimState()
{
    SetAnimState(0xb, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0x4EE8 | 0x8001E224 | size: 0x5C
 */
void cFielder::SetStopWBAnimState()
{
    SetAnimState(0x23, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
}

/**
 * Offset/Address/Size: 0x4E84 | 0x8001E1C0 | size: 0x64
 */
void cFielder::SetStrafeLeftAnimState()
{
    SetAnimState(0x28, true, 0.067f, true, false);

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    InitMovementStrafing(pTweaks->fRunningStrafeDirectionSeekSpeed, pTweaks->fRunningStrafeDirectionSeekFalloff, pTweaks->fRunningStrafeAccel, pTweaks->fRunningStrafeDecel);

    m_aActualMovementDirection = m_aDesiredMovementDirection;
}

/**
 * Offset/Address/Size: 0x4E20 | 0x8001E15C | size: 0x64
 */
void cFielder::SetStrafeRightAnimState()
{
    SetAnimState(0x29, true, 0.067f, true, false);

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    InitMovementStrafing(pTweaks->fRunningStrafeDirectionSeekSpeed, pTweaks->fRunningStrafeDirectionSeekFalloff, pTweaks->fRunningStrafeAccel, pTweaks->fRunningStrafeDecel);

    m_aActualMovementDirection = m_aDesiredMovementDirection;
}

/**
 * Offset/Address/Size: 0x4DB4 | 0x8001E0F0 | size: 0x6C
 */
void cFielder::SetIdleStrafeAnimState()
{
    SetAnimState(0, true, 0.2f, false, false);

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    InitMovementRunning(pTweaks->fRunningDirectionSeekSpeed, pTweaks->fRunningDirectionSeekFalloff, pTweaks->fRunningAccel, pTweaks->fRunningDecel);

    m_fActualSpeed = 0.0f;
    m_aActualMovementDirection = m_aDesiredMovementDirection;
}

/**
 * Offset/Address/Size: 0x4D58 | 0x8001E094 | size: 0x5C
 */
void cFielder::SetIdleAnimState()
{
    SetAnimState(0, true, 0.2f, false, false);

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    InitMovementRunning(pTweaks->fRunningDirectionSeekSpeed, pTweaks->fRunningDirectionSeekFalloff, pTweaks->fRunningAccel, pTweaks->fRunningDecel);
}

/**
 * Offset/Address/Size: 0x4CFC | 0x8001E038 | size: 0x5C
 */
void cFielder::SetIdleWBAnimState()
{
    SetAnimState(0x15, true, 0.2f, false, false);

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    InitMovementRunning(pTweaks->fRunningWBDirectionSeekSpeed, pTweaks->fRunningWBDirectionSeekFalloff, pTweaks->fRunningWBAccel, pTweaks->fRunningWBDecel);
}

/**
 * Offset/Address/Size: 0x4CA8 | 0x8001DFE4 | size: 0x54
 */
void cFielder::RunningSABcallback(unsigned int nParam1, cPN_SingleAxisBlender* pSAB)
{
    cFielder* pThis = (cFielder*)nParam1;

    if (pThis->m_eAnimID == ACTION_LOOSE_BALL_SHOT || pThis->m_eAnimID == 0x1A || pThis->m_eAnimID == ACTION_RUNNING_WB_TURBO)
    {
        float fWeight = (0.5f * pThis->m_fLeanAmount) + 0.5f;
        if (pThis->m_pCurrentAnimController->m_bMirror != 0)
        {
            fWeight = 1.0f - fWeight;
        }

        pSAB->m_fDesiredWeight = fWeight;
        return;
    }
    pSAB->m_fDesiredWeight = pSAB->m_fSmoothedWeight;
}

inline void cFielder::SetRunLeanSAB(const int* pSABAnims, int nNumSABAnims, int nPrimaryAnim)
{
    cPN_SingleAxisBlender* pSAB;
    cPN_SAnimController* synchingController;
    int i;
    cPN_SAnimController* nextController;

    pSAB = CreateSingleAxisBlender(pSABAnims, nNumSABAnims, nPrimaryAnim, RunningSABcallback, 0.0f, nullptr);

    synchingController = (cPN_SAnimController*)pSAB->GetChild(nPrimaryAnim);
    synchingController->m_fSynchronizedWeight = 0.0f;

    for (i = 0; i < nNumSABAnims; ++i)
    {
        if (i != nPrimaryAnim)
        {
            nextController = (cPN_SAnimController*)pSAB->GetChild(i);
            nextController->m_bIsSynchronized = true;
            synchingController->m_pSynchronizedController = nextController;
            synchingController = nextController;
        }
    }

    *m_pAILayer = new (AllocateBlender()) cPN_Blender(*m_pAILayer, pSAB, 0.0f);
}

/**
 * Offset/Address/Size: 0x4B70 | 0x8001DEAC | size: 0x138
 */
void cFielder::SetRunningAnimState(float)
{
    int RunningAnims[3] = {
        0x0000001B,
        0x0000001A,
        0x0000001C,
    };

    SetRunLeanSAB(RunningAnims, 3, 1);

    FielderTweaks* pTweaks = ((FielderTweaks*)m_pTweaks);
    InitMovementRunning(pTweaks->fRunningDirectionSeekSpeed, pTweaks->fRunningDirectionSeekFalloff, pTweaks->fRunningAccel, pTweaks->fRunningDecel);
}

/**
 * Offset/Address/Size: 0x4A30 | 0x8001DD6C | size: 0x140
 */
void cFielder::SetRunningTurboAnimState()
{
    int RunningTurboAnims[3] = {
        0x0000001B,
        0x0000001A,
        0x0000001C,
    };

    SetRunLeanSAB(RunningTurboAnims, 3, 1);

    FielderTweaks* pTweaks = ((FielderTweaks*)m_pTweaks);
    InitMovementRunning(pTweaks->fRunningTurboDirectionSeekSpeed, pTweaks->fRunningTurboDirectionSeekFalloff, pTweaks->fRunningTurboAccel, pTweaks->fRunningTurboDecel);

    mActionRunningVars.bFirstCycleOfTurbo = true;
}

/**
 * Offset/Address/Size: 0x48F8 | 0x8001DC34 | size: 0x138
 */
void cFielder::SetRunningWBAnimState(float)
{
    int RunningWBAnims[3] = {
        0x0000001B,
        0x0000001A,
        0x0000001C,
    };

    SetRunLeanSAB(RunningWBAnims, 3, 1);

    FielderTweaks* pTweaks = ((FielderTweaks*)m_pTweaks);
    InitMovementRunning(pTweaks->fRunningWBDirectionSeekSpeed, pTweaks->fRunningWBDirectionSeekFalloff, pTweaks->fRunningWBAccel, pTweaks->fRunningWBDecel);
}

/**
 * Offset/Address/Size: 0x48A8 | 0x8001DBE4 | size: 0x50
 */
bool cFielder::ShouldIClearBall()
{
    cNet* pOtherNet = m_pTeam->GetOtherNet();
    float fPositionX = m_v3Position.f.x;
    float fSideSign = pOtherNet->m_sideSign;
    float fResult = fPositionX * fSideSign;

    return fResult <= 0.0f;
}

/**
 * Offset/Address/Size: 0x4894 | 0x8001DBD0 | size: 0x14
 */
bool cFielder::ShouldILeadPass()
{
    return m_eActionState == ACTION_RUNNING;
}

/**
 * Offset/Address/Size: 0x4564 | 0x8001D8A0 | size: 0x330
 * TODO: 99.09% match - MWCC batches struct member loads by ascending offset (x before y),
 * but target loads y first. This causes f29/f30 register swap in second half (~34 diffs).
 */
bool cFielder::CanISlideAttack(const nlVector3& v3Position, const nlVector3& v3Velocity, float* fTime)
{
    float t;
    int nNumSolutions;
    float pSolutions[2];
    float fMaxT;
    nlPolar polarDesiredVelocity;

    if (m_eActionState == ACTION_SLIDE_ATTACK)
    {
        return false;
    }

    if (v3Position.f.z > 1.0f)
    {
        return false;
    }

    float fYDiff = m_v3Position.f.y - v3Position.f.y;
    float fXDiff = m_v3Position.f.x - v3Position.f.x;

    t = 99999.0f;

    GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;

    if (nlSqrt(fXDiff * fXDiff + fYDiff * fYDiff, true) < pGameTweaks->fSlideAttackRadius)
    {
        FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
        const nlVector3& myPos = m_v3Position;
        float fSpeed = 1.0f;
        float fSlideSpeed = pTweaks->fRunningWBTurboSpeedLevel2;

        if (fSlideSpeed >= 0.0f)
        {
            switch (m_ePowerup)
            {
            case POWER_UP_MUSHROOM:
                fSpeed *= g_pGame->m_pGameTweaks->fMushroomSpeed;
                break;
            case POWER_UP_STAR:
                fSpeed *= g_pGame->m_pGameTweaks->fStarSpeed;
                break;
            }
        }

        fSpeed *= fSlideSpeed;

        switch (m_ePowerup)
        {
        case POWER_UP_MUSHROOM:
        case POWER_UP_STAR:
            fSpeed *= 1.4f;
            break;
        }

        CalcInterceptXY(myPos, fSpeed, pTweaks->fPhysCapsuleRadius, v3Position, v3Velocity, nNumSolutions, pSolutions);

        if (nNumSolutions != 0)
        {
            if (nNumSolutions == 2)
            {
                t = pSolutions[0] < pSolutions[1] ? pSolutions[0] : pSolutions[1];
            }
            else
            {
                t = pSolutions[0];
            }
        }
    }

    float fZero = 0.0f;
    fMaxT = g_pGame->m_pGameTweaks->unk2A4 + g_pGame->m_pGameTweaks->unk2A8;

    if (t > fZero && t <= fMaxT)
    {
        if (fTime != nullptr)
        {
            float fInterceptY = v3Velocity.f.y * t + v3Position.f.y;
            float fInterceptX = v3Velocity.f.x * t + v3Position.f.x;
            float fToInterceptY = fInterceptY - m_v3Position.f.y;
            float fToInterceptX = fInterceptX - m_v3Position.f.x;

            float fLenSq = fToInterceptY * fToInterceptY + fToInterceptX * fToInterceptX;
            float fInvLen = nlRecipSqrt(fZero + fLenSq, true);
            float fDesiredVelX = fInvLen * fToInterceptX;
            float fDesiredVelY = fInvLen * fToInterceptY;

            FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
            float fSlideSpeed = pTweaks->fRunningWBTurboSpeedLevel2;

            float fSpeedX = 1.0f;
            if (fSlideSpeed >= 0.0f)
            {
                switch (m_ePowerup)
                {
                case POWER_UP_MUSHROOM:
                    fSpeedX *= g_pGame->m_pGameTweaks->fMushroomSpeed;
                    break;
                case POWER_UP_STAR:
                    fSpeedX *= g_pGame->m_pGameTweaks->fStarSpeed;
                    break;
                }
            }

            fSpeedX *= fSlideSpeed;

            switch (m_ePowerup)
            {
            case POWER_UP_MUSHROOM:
            case POWER_UP_STAR:
                fSpeedX *= 1.4f;
                break;
            }

            fDesiredVelX *= fSpeedX;

            float fSpeedY = 1.0f;
            if (fSlideSpeed >= 0.0f)
            {
                switch (m_ePowerup)
                {
                case POWER_UP_MUSHROOM:
                    fSpeedY *= g_pGame->m_pGameTweaks->fMushroomSpeed;
                    break;
                case POWER_UP_STAR:
                    fSpeedY *= g_pGame->m_pGameTweaks->fStarSpeed;
                    break;
                }
            }

            fSpeedY *= fSlideSpeed;

            switch (m_ePowerup)
            {
            case POWER_UP_MUSHROOM:
            case POWER_UP_STAR:
                fSpeedY *= 1.4f;
                break;
            }

            fDesiredVelY *= fSpeedY;

            nlCartesianToPolar(polarDesiredVelocity, fDesiredVelX, fDesiredVelY);

            *fTime = t;
        }

        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x4518 | 0x8001D854 | size: 0x4C
 */
void cFielder::SetPosition(const nlVector3& v3Position)
{
    m_v3DesiredPosition = v3Position;
    cCharacter::SetPosition(v3Position);
    m_fDistanceToDesiredPosition = -9999.9f;
}

/**
 * Offset/Address/Size: 0x3F70 | 0x8001D2AC | size: 0x5A8
 */
void cFielder::SetDesiredSpeedAndDirectionToPosition(float, const nlVector3&, eTurboRequest, float, float)
{
}

/**
 * Offset/Address/Size: 0x3EE8 | 0x8001D224 | size: 0x88
 */
void cFielder::SetDesiredSpeed(float fMinSpeed, float fMaxSpeed)
{
    if (m_pController != nullptr)
    {
        float fMagnitude = m_pController->GetMovementStickMagnitude();
        if (fMagnitude > 0.0f)
        {
            m_fDesiredSpeed = (fMaxSpeed - fMinSpeed) * m_pController->GetMovementStickMagnitude() + fMinSpeed;
        }
        else
        {
            m_fDesiredSpeed = 0.0f;
        }
    }
}

/**
 * Offset/Address/Size: 0x3E8C | 0x8001D1C8 | size: 0x5C
 */
float cFielder::GetSpeedPowerupAdjusted(float fSpeed)
{
    float fMultiplier = 1.0f;

    if (fSpeed >= 0.0f)
    {
        switch (m_ePowerup)
        {
        case POWER_UP_MUSHROOM:
            fMultiplier = fMultiplier * g_pGame->m_pGameTweaks->fMushroomSpeed;
            break;
        case POWER_UP_STAR:
            fMultiplier = fMultiplier * g_pGame->m_pGameTweaks->fStarSpeed;
            break;
        }
    }

    return fMultiplier * fSpeed;
}

/**
 * Offset/Address/Size: 0x3E0C | 0x8001D148 | size: 0x80
 */
float cFielder::GetSlideAttackSpeed()
{
    float fSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel2;
    float fResult = 1.0f;

    if (fSpeed >= 0.0f)
    {
        switch (m_ePowerup)
        {
        case POWER_UP_MUSHROOM:
            fResult *= g_pGame->m_pGameTweaks->fMushroomSpeed;
            break;
        case POWER_UP_STAR:
            fResult *= g_pGame->m_pGameTweaks->fStarSpeed;
            break;
        }
    }

    fResult *= fSpeed;

    if (m_ePowerup >= 9)
    {
        return fResult;
    }

    if (m_ePowerup < POWER_UP_MUSHROOM)
    {
        return fResult;
    }

    float fMultiplier = 1.4f;
    return fResult * fMultiplier;
}

/**
 * Offset/Address/Size: 0x3D5C | 0x8001D098 | size: 0xB0
 */
bool cFielder::SetDesire(eFielderDesireState eNewDesire, float fConfidence)
{
    GetCommonDesireData(eNewDesire).NormalizeConfidence(fConfidence);
    m_fDesireConfidence = fConfidence;

    eFielderDesireState currentDesire = m_eFielderDesireState;
    if (currentDesire != eNewDesire)
    {
        // Don't save certain desires as previous
        if (currentDesire != 0x17 && currentDesire != 0x0 && currentDesire != 0x15)
        {
            m_ePrevFielderDesireState = currentDesire;
        }

        float fDuration = m_tDesireDuration.GetSeconds();
        CleanUpDesire(eNewDesire);
        m_tDesireDuration.SetSeconds(fDuration);
        m_eFielderDesireState = eNewDesire;

        return true;
    }
    return false;
}

extern "C" eStrafeDirection CalculateStrafeDirection__8cFielderFUsUs(cFielder*, unsigned short, unsigned short);

/**
 * Offset/Address/Size: 0x3B44 | 0x8001CE80 | size: 0x218
 * TODO: 99.74% match - remaining r0/r3 register allocation swap in the
 * isStrafing comparison block (lwz/cmpwi/li/clrlwi sequence at +0x10C).
 */
u8 cFielder::ShouldIStrafe()
{
    u16 aDesiredFacingDir = m_aDesiredMovementDirection;
    bool shouldStrafe;

    if (m_pBall != NULL)
    {
        shouldStrafe = true;
        bool isTurboing = false;
        s32 animID = m_eAnimID;

        if (animID == 0x1D)
            goto setTrue1;
        if (animID == 0x1E)
            goto setTrue1;
        if (animID == 0x1F)
            goto setTrue1;
        if (animID == 0x20)
            goto setTrue1;
        if (animID == 0x21)
            goto setTrue1;
        if (animID == 0x22)
            goto setTrue1;
        goto check1;
    setTrue1:
        isTurboing = true;
    check1:
        if (!isTurboing)
        {
            shouldStrafe = false;
        }
    }
    else
    {
        shouldStrafe = true;
        bool isTurboing = false;
        s32 animID = m_eAnimID;

        if (animID == 0x10)
            goto setTrue2;
        if (animID == 0x0F)
            goto setTrue2;
        if (animID == 0x11)
            goto setTrue2;
        goto check2;
    setTrue2:
        isTurboing = true;
    check2:
        if (!isTurboing)
        {
            shouldStrafe = false;
        }
    }

    if (!shouldStrafe)
    {
        float fStrafeBall = Fuzzy::ShouldIStrafeBall(this).mData.f;
        float temp = Fuzzy::ShouldIStrafeMark(this).mData.f;
        float fStrafeMark = temp;
        float fTotalScore = fStrafeBall + fStrafeMark;

        if (GetGlobalPad() != NULL)
        {
            fStrafeBall = fTotalScore;
            fStrafeMark = 0.0f;
        }

        bool isStrafing = false;
        s32 strafeDir = mActionRunningVars.eLastStrafeDirection;
        if (strafeDir == STRAFE_LEFT)
            goto setStrafeTrue;
        if (strafeDir == STRAFE_RIGHT)
            goto setStrafeTrue;
        if (strafeDir == STRAFE_BACK)
            goto setStrafeTrue;
        goto checkStrafe;
    setStrafeTrue:
        isStrafing = true;
    checkStrafe:

        float threshold;
        if (isStrafing)
        {
            threshold = 0.3f;
        }
        else
        {
            threshold = 0.5f;
        }

        if (fTotalScore > threshold)
        {
            float ballWeight = fStrafeBall / fTotalScore;
            float targetY = ballWeight * g_pBall->m_v3Position.f.y;
            float targetX = ballWeight * g_pBall->m_v3Position.f.x;

            if (m_pMark != NULL)
            {
                float markWeight = fStrafeMark / fTotalScore;
                targetX += markWeight * m_pMark->m_v3Position.f.x;
                targetY += markWeight * m_pMark->m_v3Position.f.y;
            }

            float angle = nlATan2f(targetY - m_v3Position.f.y, targetX - m_v3Position.f.x);
            aDesiredFacingDir = (u16)(s32)(10430.378f * angle);
        }

        eStrafeDirection strafeResult = CalculateStrafeDirection__8cFielderFUsUs(this, aDesiredFacingDir, m_aDesiredMovementDirection);
        if (strafeResult == STRAFE_FORWARD)
        {
            aDesiredFacingDir = m_aDesiredMovementDirection;
        }
    }

    m_aDesiredFacingDirection = aDesiredFacingDir;
    return m_aDesiredFacingDirection != m_aDesiredMovementDirection;
}

/**
 * Offset/Address/Size: 0x39F8 | 0x8001CD34 | size: 0x14C
 */
bool cFielder::ShouldITurboWithoutBall()
{
    if (m_pMark != NULL)
    {
        if (m_pTeam->mpCurrentSituation != SITUATION_OFFENSE)
        {
            if (m_pMark->m_fDesiredSpeed > m_pTweaks->fRunningSpeed)
            {
                return true;
            }

            const nlVector3& offNet = m_pMark->GetAIOffNetLocation(NULL);

            float dx = m_pMark->m_v3Position.f.x - offNet.f.x;
            float dy = m_pMark->m_v3Position.f.y - offNet.f.y;
            float distOff = nlSqrt(dx * dx + dy * dy, true);

            const nlVector3& defNet = GetAIDefNetLocation(NULL);

            float dx2 = m_v3Position.f.x - defNet.f.x;
            float dy2 = m_v3Position.f.y - defNet.f.y;
            float distDef = nlSqrt(dx2 * dx2 + dy2 * dy2, true);

            if (distOff < distDef)
            {
                return true;
            }
        }
    }

    if (-9999.9f == m_fDistanceToDesiredPosition)
    {
        float dx = m_v3Position.f.x - m_v3DesiredPosition.f.x;
        float dy = m_v3Position.f.y - m_v3DesiredPosition.f.y;
        m_fDistanceToDesiredPosition = nlSqrt(dx * dx + dy * dy, true);
    }

    if (m_fDistanceToDesiredPosition > 5.0f)
    {
        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x3890 | 0x8001CBCC | size: 0x168
 */
void cFielder::ShouldIWave()
{
    cFielder* pOwner = g_pBall->GetOwnerFielder();
    if (pOwner == NULL)
    {
        return;
    }

    if (g_pBall->GetOwnerFielder() == this)
    {
        return;
    }

    if (!IsOnSameTeam(g_pBall->GetOwnerFielder()))
    {
        return;
    }

    if (m_nPowerupAnimID < 0 && ((cPoseNode*)m_pPowerupLayer)->GetChild(1) == NULL)
    {
        if (g_pBall->GetOwnerFielder()->DoCalcCanDoPerfectPass(this, m_v3Position))
        {
            SetPowerupAnimState(99);

            static float fTimer;
            static s8 init;

            if (!init)
            {
                fTimer = 0.0f;
                init = true;
            }

            if (FixedUpdateTask::mSimulationTime < fTimer)
            {
                fTimer = 0.0f;
            }

            if (fTimer != 0.0f)
            {
                if (FixedUpdateTask::mSimulationTime - fTimer < 3.0f)
                {
                    return;
                }
            }

            fTimer = FixedUpdateTask::mSimulationTime;
            PlayRandomCharDialogue(CHAR_DIALOGUE_WAVE, VECTORS, 1.0f, 0.5f);
        }
    }
    else
    {
        if (((cPoseNode*)m_pPowerupLayer)->GetChild(1) != NULL)
        {
            if (m_nPowerupAnimID < 0)
            {
                if (!g_pBall->GetOwnerFielder()->DoCalcCanDoPerfectPass(this, m_v3Position))
                {
                    ClearPowerupAnimState(false);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x3668 | 0x8001C9A4 | size: 0x228
 */
void cFielder::TestCollisionForInvicibility(cFielder* pOpponent)
{
    cFielder* pReactee = NULL;
    cFielder* pAttacker = NULL;
    static ePowerUpType currPowerup = POWER_UP_STAR;

    if (m_ePowerup == POWER_UP_STAR)
    {
        currPowerup = m_ePowerup;
    }

    if (IsOnSameTeam(pOpponent))
    {
        return;
    }

    ePowerUpType thisPowerup = m_ePowerup;

    do
    {
        bool bThisInvincible = (thisPowerup == POWER_UP_STAR && m_tPowerupEffectTime.m_uPackedTime != 0) || (mActionShootToScoreVars.isCurrentlyInvincible != 0);
        if (bThisInvincible)
        {
            bool bOtherInvincible = (pOpponent->m_ePowerup == POWER_UP_STAR && pOpponent->m_tPowerupEffectTime.m_uPackedTime != 0) || (pOpponent->mActionShootToScoreVars.isCurrentlyInvincible != 0);
            if (!bOtherInvincible)
            {
                pReactee = pOpponent;
                pAttacker = this;
                break;
            }
        }

        bool bOtherInvincible = (pOpponent->m_ePowerup == POWER_UP_STAR && pOpponent->m_tPowerupEffectTime.m_uPackedTime != 0) || (pOpponent->mActionShootToScoreVars.isCurrentlyInvincible != 0);
        if (bOtherInvincible)
        {
            bool bThisInvincible2 = (thisPowerup == POWER_UP_STAR && m_tPowerupEffectTime.m_uPackedTime != 0) || (mActionShootToScoreVars.isCurrentlyInvincible != 0);
            if (!bThisInvincible2)
            {
                pReactee = this;
                pAttacker = pOpponent;
            }
        }
    } while (false);

    if (pReactee == NULL)
        return;

    if (pReactee->IsFallenDown(0.0f))
        return;

    pReactee->InitActionSlideAttackReact(this, true);

    PowerupBase::PlayPowerupSound(currPowerup, PowerupBase::PWRUP_SOUND_HIT, m_pPhysicsCharacter, 100.0f);

    if (pAttacker->CanPickupBall(g_pBall))
    {
        pAttacker->PickupBall(g_pBall);
    }

    if (g_pGame->IsGameplayOrOvertime())
    {
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_POWERUPS_HIT,
            pReactee->m_pTeam->m_nSide,
            pReactee->m_ID,
            0,
            0,
            0,
            0);
    }
}

/**
 * Offset/Address/Size: 0x355C | 0x8001C898 | size: 0x10C
 */
void cFielder::TestButtonsToQueueActions(float fTime)
{
    if (GetGlobalPad() == nullptr)
    {
        return;
    }
    if (m_pBall == nullptr)
    {
        return;
    }

    if (GetGlobalPad()->JustPressed(PAD_PASS, true))
    {
        m_eLastPadAction = PAD_PASS;
    }
    else if (GetGlobalPad()->JustPressed(PAD_SHOOT, true))
    {
        m_pShotMeter->Reset();
        m_pShotMeter->m_fTime = 0.0f;
        m_eLastPadAction = PAD_SHOOT;
    }
    else if (GetGlobalPad()->JustPressed(PAD_DEKE, true))
    {
        m_eLastPadAction = PAD_DEKE;
    }
    else if (m_pController->GetCStickMovementStickMagnitude() > 0.0f)
    {
        u16 direction = m_pController->GetCStickMovementStickDirection();
        s16 diff = (s16)(direction - m_aActualFacingDirection);
        if (diff < 0)
        {
            m_eLastPadAction = PAD_DEKE_RIGHT;
        }
        else
        {
            m_eLastPadAction = PAD_DEKE_LEFT;
        }
    }
}

/**
 * Offset/Address/Size: 0x346C | 0x8001C7A8 | size: 0xF0
 */
bool cFielder::TestQueuedActions()
{
    bool bResult = false;
    int eAction = m_eLastPadAction;

    if (eAction == PAD_SHOOT)
    {
        InitActionRunningWB(false);
        bResult = true;
    }
    else if (eAction == PAD_PASS)
    {
        bool bAllowLeadPass = false;
        if (GetGlobalPad() != nullptr)
        {
            GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
            if (GetGlobalPad()->GetPressure(PAD_AIM, true) > pTweaks->unk2B0)
            {
                bAllowLeadPass = true;
            }
        }

        InitActionPass(DoFindBestPassTarget(bAllowLeadPass, false), bAllowLeadPass, false);
        bResult = true;
    }
    else if (eAction >= PAD_DEKE && eAction <= PAD_DEKE_RIGHT)
    {
        InitActionDeke(m_eLastPadAction);
        bResult = true;
    }

    return bResult;
}

/**
 * Offset/Address/Size: 0x2BB0 | 0x8001BEEC | size: 0x8BC
 * TODO: 99.48% match - register allocation artifact in inlined SetAttemptOneTouchPass
 * (target reuses r28/this for pPassTarget2; scratch compiler picks r26 instead).
 */
void cFielder::TestButtonsRunning()
{
    if (!m_bCanTestController)
    {
        return;
    }

    if (GetGlobalPad()->JustPressed(PAD_PASS, true))
    {
        cFielder* pPassTarget = (cFielder*)g_pBall->GetPassTargetFielder();
        bool bIsVolleyPassToMe = (pPassTarget != NULL
                                  && pPassTarget != this
                                  && pPassTarget->m_pTeam == m_pTeam
                                  && pPassTarget->GetGlobalPad() == NULL
                                  && (pPassTarget->m_eFielderDesireState == FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE
                                      || pPassTarget->m_eFielderDesireState == FIELDERDESIRE_RECEIVE_PASS_FROM_RUN)
                                  && g_pBall->m_pPrevOwner == this);

        if (bIsVolleyPassToMe)
        {
            ((cFielder*)g_pBall->GetPassTargetFielder())->SetAttemptOneTouchPass();
        }
        else if (CanLooseBallPass())
        {
            GameTweaks* tweaks = g_pGame->m_pGameTweaks;
            bool bAllowLeadPass = GetGlobalPad()->GetPressure(PAD_AIM, true) > tweaks->unk2B0;
            InitActionLooseBallPass(NULL, bAllowLeadPass);
        }
    }
    else if (GetGlobalPad()->JustPressed(PAD_HIT, true))
    {
        InitActionHit(DoFindBestHitTarget());
    }
    else if (GetGlobalPad()->JustPressed(PAD_SLIDE_ATTACK, true) || GetGlobalPad()->JustPressed(PAD_SHOOT, true))
    {
        cFielder* pPassTarget = (cFielder*)g_pBall->GetPassTargetFielder();
        bool bIsVolleyShotToMe = (pPassTarget != NULL
                                  && pPassTarget != this
                                  && pPassTarget->m_pTeam == m_pTeam
                                  && pPassTarget->GetGlobalPad() == NULL
                                  && (pPassTarget->m_eFielderDesireState == FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE
                                      || pPassTarget->m_eFielderDesireState == FIELDERDESIRE_RECEIVE_PASS_FROM_RUN)
                                  && g_pBall->m_pPrevOwner == this);

        if (bIsVolleyShotToMe)
        {
            ((cFielder*)g_pBall->GetPassTargetFielder())->SetAttemptOneTouchShot();
        }
        else if (CanLooseBallShoot())
        {
            GameTweaks* tweaks = g_pGame->m_pGameTweaks;
            bool bChipShot = GetGlobalPad()->GetPressure(PAD_AIM, true) > tweaks->unk2B0;
            InitActionLooseBallShot(bChipShot);
        }
        else if (!IsOnSameTeam(g_pBall->m_pOwner))
        {
            InitActionSlideAttack(NULL, -1.0f);
        }
    }
}

/**
 * Offset/Address/Size: 0x2A6C | 0x8001BDA8 | size: 0x144
 */
void cFielder::TestButtonsRunningWB(float fTime)
{
    bool shouldAttempt = false;

    cGlobalPad* pad = GetGlobalPad();
    if (pad != NULL)
    {
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        float pressure = GetGlobalPad()->GetPressure(PAD_AIM, true);
        if (pressure > tweaks->unk2B0)
        {
            shouldAttempt = true;
        }
    }

    if (GetGlobalPad()->JustPressed(PAD_PASS, true))
    {
        InitActionPass(DoFindBestPassTarget(shouldAttempt, false), shouldAttempt, false);
    }
    else if (GetGlobalPad()->JustPressed(PAD_SHOOT, true))
    {
        m_pShotMeter->Reset();
        m_pShotMeter->m_fTime = 0.0f;
    }
    else if (GetGlobalPad()->JustPressed(PAD_DEKE, true))
    {
        InitActionDeke(PAD_DEKE);
    }
    else if (m_pController->GetCStickMovementStickMagnitude() > 0.0f)
    {
        InitActionDeke(PAD_DEKE);
    }
}

/**
 * Offset/Address/Size: 0x2A34 | 0x8001BD70 | size: 0x38
 */
void cFielder::PreUpdate(float fTime)
{
    cPlayer::PreUpdate(fTime);
    m_bHasBeenUpdated = false;
    mbWasHitByPowerupThisFrame = false;
}

/**
 * Offset/Address/Size: 0x27EC | 0x8001BB28 | size: 0x248
 */
void cFielder::PostPhysicsUpdate()
{
    cPlayer::PostPhysicsUpdate();

    if (m_eActionState == ACTION_ONETIMER || m_eActionState == ACTION_LOOSE_BALL_SHOT)
    {
        if (m_pCurrentAnimController->TestTrigger(mActionOneTimerVars.fOneTimerAnimTime))
        {
            if (g_pBall->m_pOwner == NULL)
            {
                nlVector3 ballPos;
                nlVector3 ballPhysicsPos;
                g_pBall->m_pPhysicsBall->GetPosition(&ballPhysicsPos);
                ballPos = g_pBall->m_v3Position;

                nlVector3* pJointPos;
                int jointIndex;
                jointIndex = m_nBallJointIndex;
                if (TestCollision(0.18f, GetPrevJointPosition(jointIndex), *(pJointPos = &GetJointPosition(jointIndex)), 0.18f, ballPos, ballPhysicsPos))
                {
                    g_pBall->SetPosition(GetJointPosition(m_nBallJointIndex));

                    m_pShotMeter->Reset();
                    m_pShotMeter->m_fTime = 0.0f;

                    u8 wasPerfectPass = g_pBall->mbHyperSTS;
                    m_pShotMeter->CalcOneTimerValue(this, wasPerfectPass);

                    g_pBall->ClearPassTarget();

                    cNet* pOtherNet = m_pTeam->GetOtherNet();
                    float posX = m_v3Position.f.x;
                    float sideSign = pOtherNet->m_sideSign;
                    if (!(posX * sideSign <= 0.0f))
                    {
                        g_pBall->SetOwner(this);
                        g_pBall->ClearOwner();
                        DoRegularShooting();

                        bool bIsChipShot = false;
                        if (mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot)
                        {
                            bIsChipShot = true;
                        }

                        if (bIsChipShot)
                        {
                            EmitBallShot(this, BALL_EFFECT_CHIP_SHOT, NULL, false);
                        }
                        else
                        {
                            bool bIsOneTimerShot = false;
                            if (g_pBall->m_tShotTimer.m_uPackedTime != 0 && g_pBall->m_unk_0xA4 != 0)
                            {
                                bIsOneTimerShot = true;
                            }

                            if (bIsOneTimerShot)
                            {
                                EmitBallShot(this, BALL_EFFECT_PERFECT_SHOT, NULL, false);
                                FireCameraRumbleFilter(0.0f, 0.2f);
                                Play3DSFX(Audio::eCharSFX(0x3D), VECTORS, 100.0f);
                            }
                            else
                            {
                                EmitBallShot(this, BALL_EFFECT_ONETIMER_SHOT, NULL, false);
                            }
                        }
                    }
                    else
                    {
                        DoClearBall();
                    }

                    if (FixedUpdateTask::mTimeScale < 1.0f)
                    {
                        Audio::FadeFilterFromCurrentToZero();
                    }
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x234C | 0x8001B688 | size: 0x4A0
 * TODO: 95.95% match - bool materialization (missing b/li r0,0 false branch)
 *       and switch binary search vs sequential comparison for bIsActive
 */
void cFielder::Update(float fDeltaT)
{
    if (mtKickOffWaitTimer.m_uPackedTime != 0)
    {
        if (mtKickOffWaitTimer.Countdown(fDeltaT, 0.0f))
        {
            InitActionPass(DoFindBestPassTarget(false, false), false, true);
            g_pEventManager->CreateValidEvent(0xB, 0x14);
            mbCanKickoff = false;
        }
    }

    bool bIsGameplay = g_pGame->IsGameplayOrOvertime();

    if (bIsGameplay)
    {
        m_DesireCommonVars.tAge.Countup(fDeltaT, 10.0f);
        m_DesireCommonVars.tMiscTimer.Countdown(fDeltaT, 0.0f);
        m_tDesireDuration.Countdown(fDeltaT, 0.0f);

        if (m_tFrozenTimer.m_uPackedTime != 0)
        {
            if (m_tFrozenTimer.Countdown(fDeltaT, 0.0f))
            {
                EmitUnFreeze(this);
            }
        }

        if (m_tPowerupEffectTime.m_uPackedTime != 0)
        {
            if (m_tPowerupEffectTime.Countdown(fDeltaT, 0.0f))
            {
                switch (m_ePowerup)
                {
                case POWER_UP_STAR:
                    KillStar(this);
                    m_ePowerup = POWER_UP_NONE;
                    mnNumPowerups = 0;
                    m_tPowerupEffectTime.m_uPackedTime = 0;
                    break;
                case POWER_UP_MUSHROOM:
                    KillMushroom(this);
                    m_ePowerup = POWER_UP_NONE;
                    mnNumPowerups = 0;
                    m_tPowerupEffectTime.m_uPackedTime = 0;
                    break;
                default:
                    break;
                }
            }
        }

        if (mtBombImpactTime.m_uPackedTime != 0)
        {
            if (mtBombImpactTime.Countdown(fDeltaT, 0.0f))
            {
                float fBombRadius = mfBombImpactRadius;
                if (m_eActionState != ACTION_POST_WHISTLE)
                {
                    bool bSkip = (m_ePowerup == POWER_UP_STAR && m_tPowerupEffectTime.m_uPackedTime != 0) || mActionShootToScoreVars.isCurrentlyInvincible;
                    if (!bSkip)
                    {
                        AddRandomDirt();
                        if (g_pBall->m_pPassTarget != NULL && g_pBall->m_pPassTarget == this)
                        {
                            g_pBall->ClearPassTarget();
                        }
                        InitActionBombReact(mv3BombImpactLocation, fBombRadius);
                    }
                }
            }
        }
    }

    cPlayer::Update(fDeltaT);

    m_fDistanceToDesiredPosition = -9999.9f;
    m_v3AccumDesiredPos.f.x = 0.0f;
    m_v3AccumDesiredPos.f.y = 0.0f;
    m_v3AccumDesiredPos.f.z = 0.0f;
    m_fAccumDesiredPosWeight = 0.0f;

    if (m_tFrozenTimer.m_uPackedTime == 0)
    {
        eFielderActionState eAction = m_eActionState;
        bool bHoldTime = false;
        ShotMeter* pMeter = m_pShotMeter;

        bool bCheckState = (eAction == ACTION_RUNNING_WB_TURBO && m_pCurrentAnimController->m_fTime > 0.2f && m_pCurrentAnimController->m_fTime < 0.975f);

        if (bCheckState || eAction == ACTION_DEKE || eAction == ACTION_SLIDE_ATTACK)
        {
            int shotState = pMeter->m_eShotMeterState;
            if (shotState == 1 || shotState == 3)
            {
                bHoldTime = true;
            }
        }

        pMeter->Update(fDeltaT, bHoldTime);
    }

    if (m_tFrozenTimer.m_uPackedTime == 0)
    {
        bool bNeedDesire = (m_eFielderDesireState == FIELDERDESIRE_NEED_DESIRE || m_tDesireDuration.m_uPackedTime == 0);
        if (bNeedDesire)
        {
            CalculateNewDesire();
        }

        UpdateDesireState(fDeltaT);
        m_pAvoidance->Update(fDeltaT);
        UpdateActionState(fDeltaT);
        UpdateHeadTracking(fDeltaT);
        cCharacter::Update(fDeltaT);
    }

    {
        eFielderActionState eAction = m_eActionState;
        Audio::cCharacterSFX* pSFX = m_pCharacterSFX;
        bool bIsActive = false;

        switch (eAction)
        {
        case ACTION_RUNNING_WB:
        case ACTION_RUNNING_WB_TURBO:
        case ACTION_RUNNING_WB_TURBO_TURN:
            bIsActive = true;
            break;
        default:
            break;
        }

        if (bIsActive || eAction == ACTION_DEKE || (eAction == ACTION_NEED_ACTION && GetGlobalPad() != NULL))
        {
            if (m_fActualSpeed >= 4.5f && m_fDesiredSpeed > 0.0f && !g_pGame->mbCaptainShotToScoreOn)
            {
                if (!pSFX->IsMovementLoopStarted() || (pSFX->IsMovementLoopStarted() && !pSFX->IsMovementLoopPlaying()))
                {
                    pSFX->StartMovementLoop();
                }
            }
            else
            {
                if (pSFX->IsMovementLoopStarted() && m_fDesiredSpeed < 0.001f && m_fActualSpeed < 0.5f && m_eActionState != ACTION_RUNNING_WB_TURBO_TURN)
                {
                    pSFX->StopMovementLoop();
                }
            }
        }
        else
        {
            pSFX->StopMovementLoop();
        }
    }

    UpdateController(fDeltaT);
    m_bHasBeenUpdated = true;
    RandomChance(0.0f);
}

/**
 * Offset/Address/Size: 0x21D8 | 0x8001B514 | size: 0x174
 * TODO: 94.9% match - progress made, but not complete
 */
void cFielder::ThrowPowerup()
{
    cFielder* pTarget;

    if (!g_pGame->mbCaptainShotToScoreOn)
    {
        pTarget = m_pPowerupTarget;

        switch (m_ePowerup)
        {
        case POWER_UP_NONE:
            return;
        case POWER_UP_STAR:
            m_tPowerupEffectTime.SetSeconds(g_pGame->m_pGameTweaks->fStarEffectTime);
            EmitStar(this);
            break;
        case POWER_UP_MUSHROOM:
        {
            f32 time = g_pGame->m_pGameTweaks->fMushroomEffectTime;
            if (m_eCharacterClass < PEACH && m_eCharacterClass >= LUIGI)
            {
                time *= 0.5f;
            }
            m_tPowerupEffectTime.SetSeconds(time);
            EmitMushroom(this);
            InitBlur(0);
            break;
        }
        case POWER_UP_GREEN_SHELL:
        case POWER_UP_RED_SHELL:
        case POWER_UP_SPINY_SHELL:
        case POWER_UP_FREEZE_SHELL:
        case POWER_UP_BANANA:
        case POWER_UP_BOBOMB:
            m_pPowerupTarget = FindPowerupTarget(this, NULL);
            PowerupCreateAndThrow(this, m_ePowerup, mnNumPowerups, NULL);
            break;
        case POWER_UP_CHAIN_CHOMP:
            BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->Fall(this, pTarget);
            break;
        default:
            return;
        }

        if (g_pGame->IsGameplayOrOvertime())
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_POWERUPS_USED, m_pTeam->m_nSide, m_ID, 0, 0, 0, 0);
        }
        m_pPowerupTarget = NULL;
    }
}

inline void ExecutePowerupEffect(cFielder* pFielder)
{
    if (g_pGame->mbCaptainShotToScoreOn)
        return;

    switch (pFielder->m_ePowerup)
    {
    case POWER_UP_NONE:
        return;
    case POWER_UP_STAR:
        pFielder->m_tPowerupEffectTime.SetSeconds(g_pGame->m_pGameTweaks->fStarEffectTime);
        EmitStar(pFielder);
        break;
    case POWER_UP_MUSHROOM:
    {
        f32 fTime = g_pGame->m_pGameTweaks->fMushroomEffectTime;
        if (pFielder->m_eCharacterClass >= LUIGI && pFielder->m_eCharacterClass < PEACH)
        {
            fTime *= 1.33f;
        }
        pFielder->m_tPowerupEffectTime.SetSeconds(fTime);
        EmitMushroom(pFielder);
        pFielder->InitBlur(0);
        break;
    }
    case POWER_UP_GREEN_SHELL:
    case POWER_UP_RED_SHELL:
    case POWER_UP_SPINY_SHELL:
    case POWER_UP_FREEZE_SHELL:
    case POWER_UP_BANANA:
    case POWER_UP_BOBOMB:
        pFielder->m_pPowerupTarget = FindPowerupTarget(pFielder, NULL);
        PowerupCreateAndThrow(pFielder, pFielder->m_ePowerup, pFielder->mnNumPowerups, NULL);
        break;
    case POWER_UP_CHAIN_CHOMP:
    {
        cFielder* pTgt = pFielder->m_pPowerupTarget;
        BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->Fall(pFielder, pTgt);
        break;
    }
    }
    if (g_pGame->IsGameplayOrOvertime())
    {
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_POWERUPS_USED, pFielder->m_pTeam->m_nSide, pFielder->m_ID, 0, 0, 0, 0);
    }
    pFielder->m_pPowerupTarget = NULL;
}

/**
 * Offset/Address/Size: 0x1D18 | 0x8001B054 | size: 0x4C0
 * TODO: 94.28% match - MWCC CSEs m_ePowerup across dispatch switch and inlined ExecutePowerupEffect,
 * using r3 instead of r0 for dispatch (~20 register diffs). Target validation switch also missing
 * explicit -1 check (~6 diffs) - adding case POWER_UP_NONE changes MWCC binary search pivot order.
 */
void cFielder::SetPowerup(ePowerUpType eNewPowerup, int nnumOfPowerups, cFielder* pTarget)
{
    switch (m_ePowerup)
    {
    case POWER_UP_STAR:
        KillStar(this);
        m_ePowerup = POWER_UP_NONE;
        mnNumPowerups = 0;
        m_tPowerupEffectTime.m_uPackedTime = 0;
        break;
    case POWER_UP_MUSHROOM:
        KillMushroom(this);
        m_ePowerup = POWER_UP_NONE;
        mnNumPowerups = 0;
        m_tPowerupEffectTime.m_uPackedTime = 0;
        break;
    }

    switch (eNewPowerup)
    {
    case POWER_UP_GREEN_SHELL:
    case POWER_UP_RED_SHELL:
    case POWER_UP_SPINY_SHELL:
    case POWER_UP_FREEZE_SHELL:
    case POWER_UP_BANANA:
    case POWER_UP_BOBOMB:
    case POWER_UP_CHAIN_CHOMP:
        if (pTarget == NULL)
        {
            pTarget = FindPowerupTarget(this, NULL);
        }
        break;
    default:
        pTarget = NULL;
        break;
    }

    m_ePowerup = eNewPowerup;
    mnNumPowerups = nnumOfPowerups;
    m_pPowerupTarget = pTarget;

    if (m_ePowerup == POWER_UP_NONE)
        return;

    int dir = 0;
    if (m_pPowerupTarget != NULL)
    {
        dir = (GetFacingDeltaToPosition(m_pPowerupTarget->m_v3Position) >> 14) & 3;
    }

    static int ThrowAnims[] = { 95, 98, 97, 96 };
    SetPowerupAnimState(ThrowAnims[dir]);
    m_nPowerupAnimID = ThrowAnims[dir];

    if (m_eActionState == ACTION_SLIDE_ATTACK)
    {
        mActionSlideAttackVars.bWasStarMushroomUsedDuring = true;
    }

    if (m_tFrozenTimer.m_uPackedTime != 0)
        return;

    switch (m_ePowerup)
    {
    case POWER_UP_CHAIN_CHOMP:
        ExecutePowerupEffect(this);
        if (g_pGame->IsGameplayOrOvertime())
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_POWERUPS_USED, m_pTeam->m_nSide, m_ID, 0, 0, 0, 0);
        }
        m_pPowerupTarget = NULL;
        ClearPowerupAnimState(false);
        m_ePowerup = POWER_UP_NONE;
        return;
    case POWER_UP_MUSHROOM:
    case POWER_UP_STAR:
        ExecutePowerupEffect(this);
        if (g_pGame->IsGameplayOrOvertime())
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_POWERUPS_USED, m_pTeam->m_nSide, m_ID, 0, 0, 0, 0);
        }
        m_pPowerupTarget = NULL;
        ClearPowerupAnimState(false);
        break;
    }
}

/**
 * Offset/Address/Size: 0x1BE4 | 0x8001AF20 | size: 0x134
 */
void cFielder::UseTeamPowerup(cFielder* pTarget)
{
    cTeam* pTeam;

    if ((m_ePowerup == POWER_UP_STAR) || (m_tFrozenTimer.m_uPackedTime != 0))
    {
        return;
    }

    if (IsFallenDown(0.0f))
    {
        if (m_pTeam->IsCurrentStar())
        {
            return;
        }
        if (m_pTeam->IsCurrentMushroom())
        {
            return;
        }
    }

    if (!m_pTeam->IsCurrentNoPowerup())
    {
        pTeam = m_pTeam;
        SetPowerup(pTeam->GetCurrentPowerUp().eType, pTeam->GetCurrentPowerUp().nnumOfPowerups, pTarget);
        m_pTeam->ClearCurrentPowerUp();
    }
    else
    {
        ePowerUpType powerupType = m_pTeam->GetPowerUpByIndex(1).eType;
        if (powerupType == POWER_UP_NONE)
        {
            return;
        }
        m_pTeam->TogglePowerup(true);
        pTeam = m_pTeam;
        SetPowerup(pTeam->GetCurrentPowerUp().eType, pTeam->GetCurrentPowerUp().nnumOfPowerups, pTarget);
        m_pTeam->ClearCurrentPowerUp();
    }
}

/**
 * Offset/Address/Size: 0x1AB4 | 0x8001ADF0 | size: 0x130
 */
void cFielder::UpdateActionState(float dt)
{
    switch (m_eActionState)
    {
    case ACTION_DEKE:
        ActionDeke(dt);
        break;
    case ACTION_ELECTROCUTION:
        ActionElectrocution(dt);
        break;
    case ACTION_HIT:
        ActionHit(dt);
        break;
    case ACTION_HIT_REACT:
        ActionHitReact(dt);
        break;
    case ACTION_LATE_ONETIMER_FROM_VOLLEY:
        ActionLateOneTimerFromVolley(dt);
        break;
    case ACTION_IDLE_TURN:
        ActionIdleTurn(dt);
        break;
    case ACTION_LOOSE_BALL_PASS:
        ActionLooseBallPass(dt);
        break;
    case ACTION_LOOSE_BALL_SHOT:
        ActionLooseBallShot(dt);
        break;
    case ACTION_ONETIMER:
        ActionOneTimer(dt);
        break;
    case ACTION_ONETOUCH_PASS_FROM_VOLLEY:
        ActionOneTouchPassFromVolley(dt);
        break;
    case ACTION_PASS:
        ActionPass(dt);
        break;
    case ACTION_POST_WHISTLE:
        ActionPostWhistle(dt);
        break;
    case ACTION_RECEIVE_PASS:
        ActionReceivePass(dt);
        break;
    case ACTION_RUNNING:
        ActionRunning(dt);
        break;
    case ACTION_RUNNING_WB:
        ActionRunningWB(dt);
        break;
    case ACTION_RUNNING_WB_TURBO:
        ActionRunningWBTurbo(dt);
        break;
    case ACTION_RUNNING_WB_TURBO_TURN:
        ActionRunningWBTurboTurn(dt);
        break;
    case ACTION_SHOT:
        ActionShot(dt);
        break;
    case ACTION_SHOOT_TO_SCORE:
        ActionShootToScore(dt);
        break;
    case ACTION_SLIDE_ATTACK:
        ActionSlideAttack(dt);
        break;
    case ACTION_SLIDE_ATTACK_REACT:
        ActionSlideAttackReact(dt);
        break;
    case ACTION_BOMB_REACT:
        ActionBombReact(dt);
        break;
    case ACTION_BANANA_REACT:
        ActionBananaReact(dt);
        break;
    case ACTION_SHELL_REACT:
        ActionShellReact(dt);
        break;
    case ACTION_STS_HIT_REACT:
        ActionSTSHitReact(dt);
        break;
    case ACTION_SQUISH_REACT:
        ActionSquishReact(dt);
        break;
    case ACTION_SLIDE_FAIL_REACT:
        ActionSlideAttackFailReact(dt);
        break;
    case ACTION_WAIT:
        ActionWait(dt);
        break;
    }
    DoHandleActiveShotMeter();
}

/**
 * Offset/Address/Size: 0x17CC | 0x8001AB08 | size: 0x2E8
 * TODO: 98.39% match - branch shape mismatch in ACTION_ONETIMER anim-range gate
 * around 0x8001AB58..0x8001AB70 (bge/b pattern vs blt path).
 */
void cFielder::UpdateHeadTracking(float)
{
    switch (m_eActionState)
    {
    case ACTION_NEED_ACTION:
    case ACTION_DEKE:
    case ACTION_ELECTROCUTION:
    case ACTION_HIT:
    case ACTION_HIT_REACT:
    case ACTION_IDLE_TURN:
    case ACTION_LATE_ONETIMER_FROM_VOLLEY:
    case ACTION_ONETOUCH_PASS_FROM_VOLLEY:
    case ACTION_RUNNING_WB_TURBO:
    case ACTION_RUNNING_WB_TURBO_TURN:
    case ACTION_SHOT:
    case ACTION_SHOOT_TO_SCORE:
    case ACTION_SLIDE_ATTACK_REACT:
    case ACTION_BOMB_REACT:
    case ACTION_SHELL_REACT:
    case ACTION_BANANA_REACT:
    case ACTION_STS_HIT_REACT:
    case ACTION_SQUISH_REACT:
    case ACTION_SLIDE_FAIL_REACT:
        m_pHeadTrack->m_bTrackOOI = false;
        break;

    case ACTION_ONETIMER:
        if (m_eAnimID < 0x50)
        {
            if (!(m_eAnimID < 0x48))
            {
                m_pHeadTrack->m_bTrackOOI = false;
                break;
            }
        }

        if (m_pCurrentAnimController->m_fTime > mActionOneTimerVars.fOneTimerAnimTime)
        {
            m_pHeadTrack->m_bTrackOOI = false;
        }
        else
        {
            m_pHeadTrack->m_v3OOI = g_pBall->m_v3Position;
            m_pHeadTrack->m_bTrackOOI = true;
        }
        break;

    case ACTION_LOOSE_BALL_PASS:
    case ACTION_LOOSE_BALL_SHOT:
        if (m_pCurrentAnimController->m_fTime > mActionOneTimerVars.fOneTimerAnimTime)
        {
            m_pHeadTrack->m_bTrackOOI = false;
        }
        else
        {
            m_pHeadTrack->m_v3OOI = g_pBall->m_v3Position;
            m_pHeadTrack->m_bTrackOOI = true;
        }
        break;

    case ACTION_PASS:
    case ACTION_RECEIVE_PASS:
    case ACTION_SLIDE_ATTACK:
        if (m_pBall == nullptr)
        {
            if (m_eAnimID != 0x35)
            {
                m_pHeadTrack->m_v3OOI = g_pBall->m_v3Position;
                m_pHeadTrack->m_bTrackOOI = true;
                break;
            }
        }

        m_pHeadTrack->m_bTrackOOI = false;
        break;

    case ACTION_RUNNING_WB:
        if (!BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->IsHidden())
        {
            m_pHeadTrack->m_v3OOI = BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->mv3Position;
            m_pHeadTrack->m_bTrackOOI = true;
        }
        else
        {
            m_pHeadTrack->m_bTrackOOI = false;
        }
        break;

    case ACTION_RUNNING:
        if (!BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->IsHidden())
        {
            m_pHeadTrack->m_v3OOI = BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->mv3Position;
        }
        else
        {
            m_pHeadTrack->m_v3OOI = g_pBall->m_v3Position;
        }
        m_pHeadTrack->m_bTrackOOI = true;
        break;

    case ACTION_POST_WHISTLE:
    {
        cPlayer* pScorer = g_pGame->m_pScorer;
        if (pScorer != nullptr)
        {
            m_pHeadTrack->m_v3OOI = pScorer->m_v3Position;
        }
        else
        {
            m_pHeadTrack->m_v3OOI = g_pBall->m_v3Position;
        }
        m_pHeadTrack->m_bTrackOOI = true;
        break;
    }

    case ACTION_WAIT:
        m_pHeadTrack->m_v3OOI = g_pBall->m_v3Position;
        m_pHeadTrack->m_bTrackOOI = true;
        break;
    }
}

/**
 * Offset/Address/Size: 0x13C4 | 0x8001A700 | size: 0x408
 * TODO: 98.99% match - MWCC generates blt instead of bge+b for SkillLevel >= SUPERSTAR check (compiler code layout quirk)
 */
struct ShootToScoreMeterScratch
{
    u8 pad[0x28];
    u8 m_bMeterVisible;
};

extern "C" ShootToScoreMeterScratch instance__17ShootToScoreMeter;
extern "C" void RumbleMeter__17ShootToScoreMeterFfff(ShootToScoreMeterScratch*, float, float, float);

void cFielder::UpdateController(float)
{
    extern cTeam* g_pCurrentlyUpdatingTeam;

    int nNumUsers;
    int i;
    float yPercentage;
    float xPercentage;

    switch (m_eActionState)
    {
    case ACTION_NEED_ACTION:
    case ACTION_PASS:
    case ACTION_POST_WHISTLE:
        if (m_tFrozenTimer.m_uPackedTime != 0)
        {
            if (GetGlobalPad() != NULL)
            {
                if (GetGlobalPad()->JustPressed(PAD_SWITCH, true))
                {
                    if (m_pBall == NULL)
                    {
                        SwapController();
                    }
                }
            }
        }
        break;

    case ACTION_RUNNING_WB:
    case ACTION_RUNNING_WB_TURBO:
    case ACTION_RUNNING_WB_TURBO_TURN:
    case ACTION_SHOOT_TO_SCORE:
        if (GetGlobalPad() != NULL)
        {
            if (GetGlobalPad()->JustPressed(PAD_SWITCH, true))
            {
                if (m_tFrozenTimer.m_uPackedTime != 0)
                {
                    if (m_pBall == NULL)
                    {
                        SwapController();
                        break;
                    }
                }

                if (m_pBall == NULL)
                {
                    SwapController();
                }
            }
        }
        break;

    case ACTION_DEKE:
    case ACTION_ELECTROCUTION:
    case ACTION_HIT:
    case ACTION_HIT_REACT:
    case ACTION_IDLE_TURN:
    case ACTION_LATE_ONETIMER_FROM_VOLLEY:
    case ACTION_LOOSE_BALL_PASS:
    case ACTION_LOOSE_BALL_SHOT:
    case ACTION_ONETIMER:
    case ACTION_ONETOUCH_PASS_FROM_VOLLEY:
    case ACTION_RECEIVE_PASS:
    case ACTION_RUNNING:
    case ACTION_SHOT:
    case ACTION_SLIDE_ATTACK:
    case ACTION_SLIDE_ATTACK_REACT:
    case ACTION_BOMB_REACT:
    case ACTION_SHELL_REACT:
    case ACTION_BANANA_REACT:
    case ACTION_STS_HIT_REACT:
    case ACTION_SQUISH_REACT:
    case ACTION_SLIDE_FAIL_REACT:
    case ACTION_WAIT:
        if (GetGlobalPad() != NULL)
        {
            if (GetGlobalPad()->JustPressed(PAD_SWITCH, true))
            {
                if (m_pBall == NULL)
                {
                    SwapController();
                }
            }
        }
        break;
    }

    if ((g_pBall->GetOwnerFielder() != NULL)
        && !IsOnSameTeam(g_pBall->GetOwnerFielder())
        && (g_pBall->GetOwnerFielder()->m_eActionState == ACTION_SHOOT_TO_SCORE))
    {
        if (instance__17ShootToScoreMeter.m_bMeterVisible != 0)
        {
            nNumUsers = 0;
            i = 0;
            while (i < 4)
            {
                if (m_pTeam->GetFielder(i)->GetGlobalPad() != NULL)
                {
                    nNumUsers++;
                }
                i++;
            }

            if (GetGlobalPad() == NULL)
            {
                const GameplaySettings& gameplayOptions = nlSingleton<GameInfoManager>::s_pInstance->GetGameplayOptions();
                if (gameplayOptions.SkillLevel >= GameplaySettings::SUPERSTAR)
                {
                    if ((gameplayOptions.SkillLevel < (GameplaySettings::eSkillLevel)5)
                        && (nNumUsers == 0))
                    {
                        SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                        if (nlRandomf(1.0f, &nlDefaultSeed) < pSkillTweaks->Shoot_CaptainS2SSecondButtonChance)
                        {
                            EmitSolidRumble(g_pBall->GetOwnerFielder());

                            pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                            yPercentage = (float)nlRandom(2, &nlDefaultSeed) - 1.0f;
                            RumbleMeter__17ShootToScoreMeterFfff(
                                &instance__17ShootToScoreMeter,
                                pSkillTweaks->Shoot_CaptainS2SSecondButtonChance,
                                (float)nlRandom(2, &nlDefaultSeed) - 1.0f,
                                yPercentage);
                        }
                    }
                }
                else
                {
                    return;
                }
            }
            else if (m_pController->GetCStickMovementStickMagnitude() > 0.0f)
            {
                mActionRumbleVars.fRumbleDirection = (float)(u16)m_pController->GetCStickMovementStickDirection();

                EmitSolidRumble(g_pBall->GetOwnerFielder());
                EmitSolidRumble(this);

                if (mActionRumbleVars.fRumbleDirection < 32768.0f)
                {
                    yPercentage = InterpolateRangeClamped(-1.0f, 1.0f, 32768.0f, 0.0f, mActionRumbleVars.fRumbleDirection);
                }
                else
                {
                    yPercentage = InterpolateRangeClamped(-1.0f, 1.0f, 32768.0f, 65000.0f, mActionRumbleVars.fRumbleDirection);
                }

                if ((mActionRumbleVars.fRumbleDirection > 16384.0f)
                    && (mActionRumbleVars.fRumbleDirection < 49152.0f))
                {
                    xPercentage = InterpolateRangeClamped(-1.0f, 1.0f, 49152.0f, 16384.0f, mActionRumbleVars.fRumbleDirection);
                }
                else if (mActionRumbleVars.fRumbleDirection <= 16384.0f)
                {
                    xPercentage = InterpolateRangeClamped(1.0f, 0.0f, 16384.0f, 0.0f, mActionRumbleVars.fRumbleDirection);
                }
                else
                {
                    xPercentage = InterpolateRangeClamped(-1.0f, 0.0f, 49152.0f, 65000.0f, mActionRumbleVars.fRumbleDirection);
                }

                RumbleMeter__17ShootToScoreMeterFfff(
                    &instance__17ShootToScoreMeter,
                    m_pController->GetCStickMovementStickMagnitude(),
                    yPercentage,
                    xPercentage);
            }
        }
    }
    else
    {
        mActionRumbleVars.fRumbleDirection = 0.0f;
    }
}

/**
 * Offset/Address/Size: 0x13A0 | 0x8001A6DC | size: 0x24
 */
void cFielder::UpdatePlay(float fTime)
{
    m_pCurrentPlay->Update(fTime);
}

/**
 * Offset/Address/Size: 0x1338 | 0x8001A674 | size: 0x68
 */
float cFielder::GetDistanceToDesiredPos()
{
    if (m_fDistanceToDesiredPosition == -9999.9f)
    {
        float dx = m_v3Position.f.x - m_v3DesiredPosition.f.x;
        float dy = m_v3Position.f.y - m_v3DesiredPosition.f.y;
        float distanceSquared = dx * dx + dy * dy;
        m_fDistanceToDesiredPosition = nlSqrt(distanceSquared, true);
    }
    return m_fDistanceToDesiredPosition;
}

/**
 * Offset/Address/Size: 0x1224 | 0x8001A560 | size: 0x114
 */
bool cFielder::S2SShootWasPressed()
{
    bool result;
    bool padResult;

    result = false;

    if (m_eActionState == ACTION_SHOOT_TO_SCORE)
    {

        if (GetGlobalPad() != NULL)
        {
            padResult = false;
            if (GetGlobalPad()->JustPressed(PAD_SHOOT, true) || GetGlobalPad()->JustPressed(PAD_PASS, true))
            {
                padResult = true;
            }
            result = padResult;
        }
        else
        {
            // No pad - use float logic
            if (mActionShootToScoreVars.fFrameButtonDownTime1 < 0.0f)
            {
                result = (mActionShootToScoreVars.fMeterFractionTime >= (m_DesireCommonVars.fMisc - 0.009f));
            }
            else
            {
                result = (mActionShootToScoreVars.fMeterFractionTime <= -(m_DesireCommonVars.fMisc - 0.009f));
            }
            mActionShootToScoreVars.bShootWasPressed = result;
        }
    }

    if (mActionShootToScoreVars.fFrameButtonDownTime2 >= 0.0f)
    {
        mActionShootToScoreVars.bShootWasPressed = false;
        result = false;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x11AC | 0x8001A4E8 | size: 0x78
 * TODO: 100% match, but I believe the goto in the class-inline is probably movable to another class-inline method.
 */
void cFielder::StartRunning()
{
    if (!IsRunningState())
    {
        if (m_pBall != 0)
        {
            InitActionRunningWB(false);
            return;
        }
        InitActionRunning();
    }
}

/**
 * Offset/Address/Size: 0xC14 | 0x80019F50 | size: 0x598
 */
bool cFielder::DoAILooseBallActionSelection()
{
}

/**
 * Offset/Address/Size: 0x6BC | 0x800199F8 | size: 0x558
 */
void cFielder::DoAIReceivePassActionSelection()
{
}

/**
 * Offset/Address/Size: 0x3C | 0x80019378 | size: 0x680
 */
bool cFielder::DoAIWindupActionSelection()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x8001933C | size: 0x3C
 */
void cFielder::DoSpeedBoost()
{
    eShotMeterState meterState = m_pShotMeter->m_eShotMeterState;
    bool bReturn = (meterState == SHOT_METER_ACTIVE || meterState == SHOT_METER_STS_ACTIVE || meterState == SHOT_METER_STS_TRANSISTION);

    if (bReturn)
        return;

    m_fActualSpeed = 0.0f;
}
