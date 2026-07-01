#include "Game/AI/ShotMeter.h"
#include "Game/AI/AIPad.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/Ball.h"
#include "Game/CharacterTweaks.h"
#include "Game/Game.h"
#include "Game/GameTweaks.h"
#include "Game/CharacterTriggers.h"

extern cTeam* g_pCurrentlyUpdatingTeam;
static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

static inline void CalcShotAim(cFielder* pFielder, ShotMeter* pMeter);
static inline void CalcScoreValue(cFielder* pFielder, ShotMeter* pMeter);

/**
 * Offset/Address/Size: 0x6C4 | 0x800627E4 | size: 0x16C
 */
void ShotMeter::Update(float fDeltaT, bool bHoldTime)
{
    if (!bHoldTime)
    {
        m_fTime += fDeltaT;
    }

    switch (m_eShotMeterState)
    {
    case SHOT_METER_ACTIVE:
    {
        if (m_fTime >= g_pGame->m_pGameTweaks->unk2D0)
        {
            float fNetDirection = -1.0f;
            cPlayer* pPrevOwner = g_pBall->m_pOwner;
            if (pPrevOwner != NULL)
            {
                cNet* pOtherNet = pPrevOwner->m_pTeam->GetOtherNet();
                fNetDirection = g_pBall->m_pOwner->m_v3Position.f.x * pOtherNet->m_baseLocation.f.x;
            }
            if (fNetDirection > 0.0f)
            {
                if (g_pBall->GetOwnerFielder() != NULL)
                {
                    if (g_pBall->GetOwnerFielder()->CanDoCaptainShootToScore())
                    {
                        m_eShotMeterState = SHOT_METER_STS_TRANSISTION;
                        break;
                    }
                }
            }
            if (!bHoldTime)
            {
                m_eShotMeterState = SHOT_METER_RELEASED;
            }
        }
        break;
    }
    case SHOT_METER_STS_TRANSISTION:
        m_eShotMeterState = SHOT_METER_STS_ACTIVE;
        break;
    case SHOT_METER_STS_ACTIVE:
    {
        SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
        float fSTSWindupTime = pSkillTweaks->fSTSWindupTime;
        float fShotWindupTime = g_pGame->m_pGameTweaks->unk2D0;
        if (m_fTime >= fShotWindupTime + fSTSWindupTime)
        {
            if (!bHoldTime)
            {
                m_eShotMeterState = SHOT_METER_STS_RELEASED;
            }
        }
        break;
    }
    case SHOT_METER_RELEASED:
    case SHOT_METER_STS_RELEASED:
        m_eShotMeterState = SHOT_METER_INACTIVE;
        break;
    case SHOT_METER_INACTIVE:
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x684 | 0x800627A4 | size: 0x40
 */
void ShotMeter::Abort(cFielder* pFielder)
{
    KillWindups(pFielder);
    m_eShotMeterState = SHOT_METER_INACTIVE;
    m_fTime = 0.0f;
}

/**
 * Offset/Address/Size: 0x258 | 0x80062378 | size: 0x42C
 * TODO: 99.36% match - remaining diffs are in the two normalization blocks
 * (f30/f28), the distance-length temp (f1/f5), and score-weight accumulation.
 */
void ShotMeter::CalcOneTimerValue(cFielder* pFielder, bool bWasPerfectPass)
{
    m_eShotMeterState = SHOT_METER_INACTIVE;

    if (!bWasPerfectPass)
    {
        nlVector3 v3BallDirection;
        float fBallDirectionLengthSq;
        float a, b, c;

        float fBallDirectionX = g_pBall->m_v3Position.f.x - g_pBall->m_v3PrevPosition.f.x;
        float fBallDirectionY = g_pBall->m_v3Position.f.y - g_pBall->m_v3PrevPosition.f.y;
        float fBallDirectionZ = g_pBall->m_v3Position.f.z - g_pBall->m_v3PrevPosition.f.z;

        v3BallDirection.f.x = fBallDirectionX;
        v3BallDirection.f.y = fBallDirectionY;
        v3BallDirection.f.z = fBallDirectionZ;

        a = fBallDirectionX * fBallDirectionX;
        b = fBallDirectionY * fBallDirectionY;
        c = fBallDirectionZ * fBallDirectionZ;
        fBallDirectionLengthSq = a + b + c;

        if (nlSqrt(fBallDirectionLengthSq, true) > 0.0001f)
        {
            float fBallDirectionInvLength = nlRecipSqrt(fBallDirectionLengthSq, true);
            v3BallDirection.f.x = fBallDirectionInvLength * v3BallDirection.f.x;
            v3BallDirection.f.y = fBallDirectionInvLength * fBallDirectionY;
            v3BallDirection.f.z = fBallDirectionInvLength * fBallDirectionZ;
        }
        else
        {
            v3BallDirection = v3Zero;
        }

        nlVector3 v3FielderToNet;
        float fFielderToNetLengthSq;
        const nlVector3& v3OffNetLocation = pFielder->GetAIOffNetLocation(NULL);

        float fFielderToNetX = v3OffNetLocation.f.x - pFielder->m_v3Position.f.x;
        float fFielderToNetY = v3OffNetLocation.f.y - pFielder->m_v3Position.f.y;
        float fFielderToNetZ = v3OffNetLocation.f.z - pFielder->m_v3Position.f.z;

        v3FielderToNet.f.x = fFielderToNetX;
        v3FielderToNet.f.y = fFielderToNetY;
        v3FielderToNet.f.z = fFielderToNetZ;

        a = fFielderToNetX * fFielderToNetX;
        b = fFielderToNetY * fFielderToNetY;
        c = fFielderToNetZ * fFielderToNetZ;
        fFielderToNetLengthSq = a + b + c;

        if (nlSqrt(fFielderToNetLengthSq, true) > 0.0001f)
        {
            float fFielderToNetInvLength = nlRecipSqrt(fFielderToNetLengthSq, true);
            v3FielderToNet.f.x = fFielderToNetInvLength * v3FielderToNet.f.x;
            v3FielderToNet.f.y = fFielderToNetInvLength * fFielderToNetY;
            v3FielderToNet.f.z = fFielderToNetInvLength * fFielderToNetZ;
        }
        else
        {
            v3FielderToNet = v3Zero;
        }

        const nlVector3& v3OffNetLocation2 = pFielder->GetAIOffNetLocation(NULL);
        float fDistY = g_pBall->m_v3Position.f.y - v3OffNetLocation2.f.y;
        float fDistZ = g_pBall->m_v3Position.f.z - v3OffNetLocation2.f.z;
        float fDistX = g_pBall->m_v3Position.f.x - v3OffNetLocation2.f.x;
        float fDistanceValue = InterpolateRangeClamped(0.0f, 1.0f, 20.0f, 7.5f, nlSqrt((fDistX * fDistX) + (fDistY * fDistY) + (fDistZ * fDistZ), true));
        float fDot = (v3FielderToNet.f.x * v3BallDirection.f.x) + (v3FielderToNet.f.y * v3BallDirection.f.y) + (v3FielderToNet.f.z * v3BallDirection.f.z);
        float fDirectionValue = InterpolateRangeClamped(0.0f, 1.0f, 1.0f, 0.0f, fDot);

        m_fSpeedValue = nlRandomf(0.2f * g_pGame->m_pGameTweaks->unk2EC, &nlDefaultSeed);

        float fCombinedValue = (fDirectionValue + fDistanceValue) * 0.5f;
        m_fSpeedValue += InterpolateRangeClamped(0.25f, 0.8f * g_pGame->m_pGameTweaks->unk2EC, 0.0f, 1.0f, fCombinedValue);
    }
    else
    {
        m_fSpeedValue = 1.0f;
    }

    CalcScoreValue(pFielder, this);
    CalcShotAim(pFielder, this);
}

/**
 * Offset/Address/Size: 0x21C | 0x8006233C | size: 0x3C
 */
float ShotMeter::GetTotalDuration() const
{
    float fShotWindupTime;
    float fSTSWindupTime;
    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
    fSTSWindupTime = pSkillTweaks->fSTSWindupTime;
    fShotWindupTime = g_pGame->m_pGameTweaks->unk2D0;
    return fShotWindupTime + fSTSWindupTime;
}

/**
 * Offset/Address/Size: 0x1FC | 0x8006231C | size: 0x20
 */
void ShotMeter::Reset()
{
    m_eShotMeterState = SHOT_METER_ACTIVE;
    m_fTime = 0.0f;
    m_fScoreValue = 0.0f;
    m_fSpeedValue = 0.0f;
    m_fSTSValue = 0.0f;
}

static inline void CalcShotAim(cFielder* pFielder, ShotMeter* pMeter)
{
    cAIPad* pPad = pFielder->m_pController;
    float fAimValue = 0.0f;
    if (pPad != NULL)
    {
        if (pPad->GetMovementStickMagnitude() > 0.0001f)
        {
            s16 dir = pPad->GetMovementStickDirection();
            if ((s16)(dir + 0x8000) >= 0)
            {
                fAimValue = -1.0f;
            }
            else
            {
                fAimValue = 1.0f;
            }
        }
    }
    pMeter->mfSShotAimValue = fAimValue;
}

static inline void CalcScoreValue(cFielder* pFielder, ShotMeter* pMeter)
{
    float fPlayerDistance;
    float fNetOpeness;
    float fChargedValue;
    float fRatingsValue;

    fRatingsValue = LikelyToScore(pFielder);
    fPlayerDistance = PlayerShotDistance(pFielder);
    fChargedValue = pMeter->m_fSpeedValue;

    float fShooting = ((FielderTweaks*)pFielder->m_pTweaks)->fShooting;
    GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;
    float fPositionWeighting = pGameTweaks->unk2D8;
    bool bIsChipShot = pFielder->mActionShotVars.bIsChipShot || pFielder->mActionLooseBallShotVars.bIsChipShot;

    if (!bIsChipShot)
    {
        float fPlayerWeighting = pGameTweaks->unk2E0;
        fShooting *= fPositionWeighting;
        float fNetWeighting = pGameTweaks->unk2DC;
        fNetOpeness = fRatingsValue * fNetWeighting;
        fPlayerDistance *= fPlayerWeighting;
        float fSumWeighting = fPlayerWeighting + fNetWeighting;
        float fTotalWeighting = fPositionWeighting + fSumWeighting;
        float fScore = fNetOpeness + fPlayerDistance;
        float fRemainder = 1.0f - fTotalWeighting;
        fChargedValue *= fRemainder;
        fScore = fChargedValue + fScore;
        pMeter->m_fScoreValue = fShooting + fScore;
    }
    else
    {
        float fChipWeight = pGameTweaks->unk2E4;
        float fGoalieOut = GoalieOutOfPosition(pFielder);
        pGameTweaks = g_pGame->m_pGameTweaks;
        float fGoalieVal = fGoalieOut;
        fGoalieVal *= fChipWeight;
        fShooting *= fPositionWeighting;
        float fChipOpenWeight = pGameTweaks->unk2E8;
        float fOpenVal = fRatingsValue * fChipOpenWeight;
        float fSum = fChipWeight + fChipOpenWeight;
        float fSumWeights = fPositionWeighting + fSum;
        float fResult = fGoalieVal + fOpenVal;
        float fRemainder = 1.0f - fSumWeights;
        fChargedValue *= fRemainder;
        fResult = fChargedValue + fResult;
        pMeter->m_fScoreValue = fShooting + fResult;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80062120 | size: 0x1FC
 * TODO: 99.37% match - the net-openness term (fRatingsValue * net weight) lands
 * in f1 where the target uses f5; this cascades through the score accumulation
 * (f0/f1 instead of f1/f0) in both the chip and non-chip shot branches.
 */
void ShotMeter::ShotReleased(cFielder* pFielder)
{
    KillWindups(pFielder);
    m_eShotMeterState = SHOT_METER_RELEASED;

    GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;
    if (m_fTime > pGameTweaks->unk2D0)
    {
        m_fSpeedValue = 1.0f;
    }
    else
    {
        m_fSpeedValue = Interpolate(0.25f, 1.0f, m_fTime / pGameTweaks->unk2D0);
    }

    CalcScoreValue(pFielder, this);
    CalcShotAim(pFielder, this);
    m_fTime = 0.0f;
}
