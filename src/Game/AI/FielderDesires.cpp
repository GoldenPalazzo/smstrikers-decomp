#include "Game/AI/FielderDesires.h"

#include "Game/AI/AiUtil.h"
#include "Game/AI/FilteredRandom.h"
#include "Game/AI/Fuzzy.h"

#include "Game/AI/AvoidController.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/Scripts/CommonScript.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/AI/SpaceSearch.h"
#include "Game/AnimInventory.h"
#include "Game/FormationDefines.h"
#include "Game/GameTweaks.h"

extern FuzzyVariant fvNotSet;
extern cTeam* g_pCurrentlyUpdatingTeam;
static float g_fLooseBallActionRethinkTime;
extern cFielder* g_pScriptCurrentFielder;

enum eShotMeterState
{
    SHOT_METER_INACTIVE = 0,
    SHOT_METER_ACTIVE = 1,
    SHOT_METER_RELEASED = 2,
    SHOT_METER_STS_ACTIVE = 3,
    SHOT_METER_STS_TRANSISTION = 4,
    SHOT_METER_STS_RELEASED = 5,
};

class ShotMeter
{
public:
    eShotMeterState m_eShotMeterState;
    float GetTotalDuration() const;
    void CalcOneTimerValue(cFielder* pFielder, bool bWasPerfectPass);
};

CommonDesireData g_vDesireCommonData[NUM_FIELDERDESIRES];

struct SupportBallAILocation
{
    float x0;
    float y0;
    float x1;
    float y1;
};

static const nlVector3 v3Zero = {
    0.0f,
    0.0f,
    0.0f,
};

static const SupportBallAILocation g_vSupportBallDefensiveAILocations[6] = {
    { 0.0f, 1.0f, 0.6f, 0.45f },
    { 0.0f, -1.0f, 0.6f, -0.45f },
    { 4.0f, 1.0f, 3.4f, 0.45f },
    { 4.0f, -1.0f, 3.4f, -0.45f },
    { 2.0f, 1.0f, 1.4f, 0.4f },
    { 2.0f, -1.0f, 1.4f, -0.4f },
};

static const SupportBallAILocation g_vSupportBallOffensiveAILocations[6] = {
    { 0.0f, 1.0f, 0.6f, 0.45f },
    { 0.0f, -1.0f, 0.6f, -0.45f },
    { 4.0f, 1.0f, 3.4f, 0.45f },
    { 4.0f, -1.0f, 3.4f, -0.45f },
    { 2.0f, 1.0f, 2.6f, 0.4f },
    { 2.0f, -1.0f, 2.6f, -0.4f },
};

static const nlVector2 g_vMarkingNetPassBalance = {
    0.0f,
    0.25f,
};

static const nlVector2 g_vMarkDistance = {
    7.0f,
    4.0f,
};

static const nlVector2 g_vMarkFormationBalance = {
    0.8f,
    1.0f,
};

static const nlVector2 g_vMarkBallOwner = {
    0.0f,
    0.5f,
};

static const nlVector2 g_vMarkImmediateThreatCoeff = {
    1.0f,
    0.5f,
};

static const nlVector2 g_vMarkFollowTimeDelay = {
    0.4f,
    0.1f,
};

static inline void CalcDeltaToTarget(nlVector3& outDelta, const nlVector3& target, const nlVector3& origin)
{
    outDelta.f.x = target.f.x - origin.f.x;
    outDelta.f.y = target.f.y - origin.f.y;
    outDelta.f.z = target.f.z - origin.f.z;
}

/**
 * Offset/Address/Size: 0x668C | 0x80037410 | size: 0x3C
 */
float CommonDesireData::CalcFuzzyChance(float fChance)
{
    return FGREATER(fChance, m_RandomGenerator.genrand());
}

/**
 * Offset/Address/Size: 0x6668 | 0x800373EC | size: 0x24
 */
bool CommonDesireData::CalcBoolChance(float fChance)
{
    return m_RandomChanceGen.genrand(fChance);
}

/**
 * Offset/Address/Size: 0x660C | 0x80037390 | size: 0x5C
 */
float CommonDesireData::NormalizeConfidence(float fConfidence)
{
    if (m_ConfidenceExtrema.f.x > fConfidence)
    {
        m_ConfidenceExtrema.f.x = (0.5f * fConfidence) + (0.5f * m_ConfidenceExtrema.f.x);
    }
    if (m_ConfidenceExtrema.f.y < fConfidence)
    {
        m_ConfidenceExtrema.f.y = (0.5f * fConfidence) + (0.5f * m_ConfidenceExtrema.f.y);
    }
    return NormalizeVal(fConfidence, m_ConfidenceExtrema);
}

/**
 * Offset/Address/Size: 0x65F8 | 0x8003737C | size: 0x14
 */
CommonDesireData& GetCommonDesireData(eFielderDesireState desireType)
{
    return g_vDesireCommonData[desireType];
}

/**
 * Offset/Address/Size: 0xA84 | 0x80037F0C | size: 0xCC
 */
CommonDesireData::CommonDesireData(const CommonDesireData& other)
    : m_DesireType(other.m_DesireType)
    , m_ConfidenceExtrema(other.m_ConfidenceExtrema)
    , m_RandomGenerator(other.m_RandomGenerator)
    , m_RandomChanceGen(other.m_RandomChanceGen)
{
}

/**
 * Offset/Address/Size: 0x6484 | 0x80037208 | size: 0x174
 */
void cFielder::QueueDesire(eFielderDesireState eDesireType, float fDuration, FuzzyVariant opt1, FuzzyVariant opt2)
{
    ClearQueuedDesire();

    m_sQueuedDesireParams.eDesireType = eDesireType;
    m_sQueuedDesireParams.fDuration = fDuration;
    m_sQueuedDesireParams.opt1 = opt1;
    m_sQueuedDesireParams.opt2 = opt2;
}

/**
 * Offset/Address/Size: 0x63C8 | 0x8003714C | size: 0xBC
 */
void cFielder::ClearQueuedDesire()
{
    m_sQueuedDesireParams.fDuration = 0.0f;
    m_sQueuedDesireParams.eDesireType = FIELDERDESIRE_NEED_DESIRE;
    m_sQueuedDesireParams.opt1 = fvNotSet;
    m_sQueuedDesireParams.opt2 = fvNotSet;
}

/**
 * Offset/Address/Size: 0x620C | 0x80036F90 | size: 0x1BC
 */
bool cFielder::InitDesire(const sDesireParams* pParams, float fConfidence)
{
    return InitDesire(pParams->eDesireType, fConfidence, pParams->fDuration, pParams->opt1, pParams->opt2);
}

/**
 * Offset/Address/Size: 0x54DC | 0x80036260 | size: 0xD30
 */
bool cFielder::InitDesire(eFielderDesireState, float, float, FuzzyVariant, FuzzyVariant)
{
    FORCE_DONT_INLINE;
    return false;
}

/**
 * Offset/Address/Size: 0x4700 | 0x80035484 | size: 0xDDC
 */
void cFielder::UpdateDesireState(float)
{
}

/**
 * Offset/Address/Size: 0x469C | 0x80035420 | size: 0x64
 */
void cFielder::EndDesire(bool bCheckTimer)
{
    bool bShouldSetDuration = true;

    if (bCheckTimer)
    {
        bShouldSetDuration = m_DesireCommonVars.tAge.GetSeconds() > 0.5f;
    }

    if (bShouldSetDuration)
    {
        SetDesireDuration(0.0f, true);
    }
}

/**
 * Offset/Address/Size: 0x45C8 | 0x8003534C | size: 0xD4
 */
void cFielder::CleanUpDesire(eFielderDesireState eNewDesireState)
{
    switch (m_eFielderDesireState)
    {
    case FIELDERDESIRE_WINDUP_PASS:
        AbortPendingThoughts();
        break;

    case FIELDERDESIRE_USER_CONTROLLED:
        SetNoPickUpTime(0.0f);
        break;

    case FIELDERDESIRE_ONETIMER:
        SetNoPickUpTime(0.0f);
        SetSpaceSearch(nullptr);
        break;

    case FIELDERDESIRE_POST_WHISTLE:
        SetNoPickUpTime(0.0f);
        SetSpaceSearch(nullptr);
        break;

    case FIELDERDESIRE_CUT_AND_BREAK:
    case FIELDERDESIRE_GET_OPEN:
    case FIELDERDESIRE_RUN_TO_NET:
    case FIELDERDESIRE_PASS:
        if (eNewDesireState != FIELDERDESIRE_RECEIVE_PASS_FROM_RUN && eNewDesireState != FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE)
        {
            SetSpaceSearch(nullptr);
        }
        break;

    case FIELDERDESIRE_SUPPORT_BALL_OFFENSIVE:
    case FIELDERDESIRE_USE_POWERUP:
        SetSpaceSearch(nullptr);
        break;

    case FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE:
    case FIELDERDESIRE_RECEIVE_PASS_FROM_RUN:
        break;

    default:
        break;
    }

    SetDesireDuration(0.0f, true);
    m_eFielderDesireState = FIELDERDESIRE_NEED_DESIRE;
}

/**
 * Offset/Address/Size: 0x4204 | 0x80034F88 | size: 0x3C4
 * TODO: 99.85% match - CanISlideAttack r4/r5 arg eval order (scratch-only compiler register allocation artifact)
 */
void cFielder::DesireInterceptBall(float fDeltaT)
{
    bool bTrackBall;
    nlVector3 v3DesirePosition;
    nlVector3 v3FutureTargetPosition;
    float fTime;

    switch (m_eDesireSubState)
    {
    case 0:
    {
        bTrackBall = true;

        if (m_DesireCommonVars.tMiscTimer.m_uPackedTime == 0)
        {
            if (DoAILooseBallActionSelection())
            {
                m_DesireCommonVars.tMiscTimer.SetSeconds(0.0f);
                bTrackBall = false;
            }
            else
            {
                m_DesireCommonVars.tMiscTimer.SetSeconds(g_fLooseBallActionRethinkTime);
            }
        }

        if (bTrackBall)
        {
            cPlayer* pPassTarget = g_pBall->m_pPassTarget;
            if (pPassTarget != NULL && pPassTarget->m_eClassType == FIELDER)
            {
                float fVolley = ReceivingVolleyPass(pPassTarget);
                if (fVolley || High(g_pBall) >= 0.5f)
                {
                    ((cFielder*)pPassTarget)->CalcPointOnPerimeter(v3DesirePosition, m_v3Position, 2.0f);
                }
                else
                {
                    float fSeconds = g_pBall->m_tPassTargetTimer.GetSeconds();
                    float fz = pPassTarget->m_v3Position.f.z + fSeconds * pPassTarget->m_v3Velocity.f.z;
                    float fy = pPassTarget->m_v3Position.f.y + fSeconds * pPassTarget->m_v3Velocity.f.y;
                    float fx = pPassTarget->m_v3Position.f.x + fSeconds * pPassTarget->m_v3Velocity.f.x;
                    v3FutureTargetPosition.f.x = fx;
                    v3FutureTargetPosition.f.y = fy;
                    v3FutureTargetPosition.f.z = fz;

                    v3DesirePosition = GetClosestPointOnLineABFromPointC(g_pBall->m_v3Position, v3FutureTargetPosition, m_v3Position);
                }

                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                if (pSkillTweaks->Def_SlideAttackChance > 0.0f)
                {
                    if (CanISlideAttack(g_pBall->m_v3Position, g_pBall->m_v3Velocity, &fTime))
                    {
                        InitActionSlideAttack(NULL, fTime);
                        m_eDesireSubState = 1;
                        bTrackBall = false;
                    }
                }
            }
            else
            {
                float fRawTime = m_pTeam->mfBallInterceptTimes[m_ID];
                float fInterceptTime = (0.5f <= fRawTime) ? 0.5f : fRawTime;

                float fz = g_pBall->m_v3Position.f.z + fInterceptTime * g_pBall->m_v3Velocity.f.z;
                float fy = g_pBall->m_v3Position.f.y + fInterceptTime * g_pBall->m_v3Velocity.f.y;
                float fx = g_pBall->m_v3Position.f.x + fInterceptTime * g_pBall->m_v3Velocity.f.x;
                v3DesirePosition.f.x = fx;
                v3DesirePosition.f.y = fy;
                v3DesirePosition.f.z = fz;
            }
        }

        if (bTrackBall)
        {
            eTurboRequest turboRequest = TR_MOVING_TARGET;
            float dx = v3DesirePosition.f.x - m_v3Position.f.x;
            float dy = v3DesirePosition.f.y - m_v3Position.f.y;
            if (nlSqrt(dx * dx + dy * dy, true) < 1.0f)
            {
                turboRequest = TR_FAR_DISTANCE;
            }

            SetDesiredSpeedAndDirectionToPosition(fDeltaT, v3DesirePosition, turboRequest, 0.3f, 0.6f);
            m_pAvoidance->UseMinimumAvoidance(NULL);
        }

        if (m_pBall == NULL)
        {
            cPlayer* pOwner = g_pBall->m_pOwner;
            if (pOwner == NULL || pOwner->m_eClassType != GOALIE)
            {
                break;
            }
        }
        SetDesireDuration(0.0f, true);
        break;
    }
    case 1:
    {
        SetDesireDuration(999999.9f, true);

        if (m_tSlideAttackTimer.m_uPackedTime != 0)
        {
            if (mActionSlideAttackVars.bAttackSucceeded == 0)
            {
                float fBallSpeed = nlSqrt(
                    g_pBall->m_v3Velocity.f.x * g_pBall->m_v3Velocity.f.x + g_pBall->m_v3Velocity.f.y * g_pBall->m_v3Velocity.f.y + g_pBall->m_v3Velocity.f.z * g_pBall->m_v3Velocity.f.z,
                    true);

                if (fBallSpeed > 0.05f)
                {
                    const nlVector3& v3BallVel = g_pBall->m_v3Velocity;
                    float fBallClosingSpeed = GetClosingSpeed2D(
                        GetJointPosition(m_nLeftFootJointIndex),
                        m_v3Velocity,
                        g_pBall->m_v3Position,
                        v3BallVel);

                    if (fBallClosingSpeed < 0.0f)
                    {
                        if (nlRandomf(1.0f, &nlDefaultSeed) > 0.5f)
                        {
                            m_tSlideAttackTimer.SetSeconds(0.0f);
                            m_eDesireSubState = 2;
                        }
                    }
                }
            }
        }
        else
        {
            m_eDesireSubState = 2;
        }
        break;
    }
    case 2:
    {
        if (m_eActionState == ACTION_NEED_ACTION)
        {
            SetDesireDuration(0.0f, true);
        }
        break;
    }
    }
}

/**
 * Offset/Address/Size: 0x39EC | 0x80034770 | size: 0x818
 */
void cFielder::DesireMark(float fDeltaT)
{
    bool bBestBallInterceptor = (m_pTeam->m_pBallInterceptOrderedFielders[0] == this);

    if (m_pMark == NULL || this == g_pBall->m_pOwner || (bBestBallInterceptor && m_pTeam->mpCurrentSituation == SITUATION_LOOSE))
    {
        SetDesireDuration(0.0f, true);
        return;
    }

    if ((IsOnSameTeam(m_pMark) || bBestBallInterceptor) && m_DesireCommonVars.tAge.GetSeconds() > 0.5f)
    {
        SetDesireDuration(0.0f, true);
    }

    if (m_pMark->m_pBall != NULL)
    {
        SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);

        if (pSkillTweaks->Def_SlideAttackChance > 0.0f && m_pMark->m_tBallPossessionTimer.GetSeconds() > 5.0f)
        {
            InitDesire(FIELDERDESIRE_SLIDE_ATTACK, 0.5f, -1.0f, FuzzyVariant((cPlayer*)m_pMark), fvNotSet);
            return;
        }
    }

    if (m_DesireCommonVars.tMiscTimer.m_uPackedTime == 0)
    {
        SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
        float fTimeDelay = Interpolate(g_vMarkFollowTimeDelay.f.x, g_vMarkFollowTimeDelay.f.y, pSkillTweaks->Def_Marking);
        float fTimeDelayRange = fTimeDelay * 0.8f;
        float fRandomTime = nlRandomf(fTimeDelayRange, &nlDefaultSeed) - (0.5f * fTimeDelayRange);
        m_DesireCommonVars.tMiscTimer.SetSeconds(fTimeDelay + fRandomTime);

        cPlayer* pMark = m_pMark;
        cNet* pNet = m_pTeam->m_pNet;

        float fMarkY = pMark->m_v3Position.f.y + (0.1f * pMark->m_v3Velocity.f.y);
        float fMarkX = pMark->m_v3Position.f.x + (0.1f * pMark->m_v3Velocity.f.x);

        float dirY = pNet->m_baseLocation.f.y - fMarkY;
        float dirX = pNet->m_baseLocation.f.x - fMarkX;
        float dirZ = pNet->m_baseLocation.f.z - 0.0f;

        float fInvLength = nlRecipSqrt((dirY * dirY) + (dirX * dirX) + (dirZ * dirZ), true);
        dirY = fInvLength * dirY;
        dirX = fInvLength * dirX;
        dirZ = fInvLength * dirZ;

        nlVector3 vAccumulated_v3 = v3Zero;
        float fTotalWeight_v3 = 0.0f;

        float fMarkingNetPassBalance = Interpolate(g_vMarkingNetPassBalance.f.x, g_vMarkingNetPassBalance.f.y, pSkillTweaks->Def_Marking);
        float fMarkingDistance = Interpolate(g_vMarkDistance.f.x, g_vMarkDistance.f.y, pSkillTweaks->Def_Marking);
        float fMarkFormationBalance = Interpolate(g_vMarkFormationBalance.f.x, g_vMarkFormationBalance.f.y, pSkillTweaks->Def_Marking);
        float fMarkBallOwnerBalance = Interpolate(g_vMarkBallOwner.f.x, g_vMarkBallOwner.f.y, pSkillTweaks->Def_Marking);
        float fMarkThreatCoeff = Interpolate(g_vMarkImmediateThreatCoeff.f.x, g_vMarkImmediateThreatCoeff.f.y, pSkillTweaks->Def_Marking);

        float fDistanceMultiplier = Interpolate(0.5f, 1.0f, FarToTheirNet(m_pMark));
        fMarkingDistance = fMarkingDistance * fDistanceMultiplier;

        if (UserControlledT(m_pTeam) != 0.0f && (ReceivingPass(m_pMark) != 0.0f || WindingUpForShot(m_pMark) != 0.0f))
        {
            fMarkingDistance = fMarkingDistance * fMarkThreatCoeff;
        }

        if (m_pMark->m_pBall == NULL)
        {
            cPlayer* pSBC = Fuzzy::GetStrategicBallCarrier(m_pTeam->GetOtherTeam()).mData.pPlayer;

            if (pSBC != NULL && pSBC != m_pMark)
            {
                float fSBCY = pSBC->m_v3Position.f.y + (0.1f * pSBC->m_v3Velocity.f.y);
                float fSBCX = pSBC->m_v3Position.f.x + (0.1f * pSBC->m_v3Velocity.f.x);
                float fSBCZ = pSBC->m_v3Position.f.z + (0.1f * pSBC->m_v3Velocity.f.z);

                float sbcDirY = fSBCY - fMarkY;
                float sbcDirX = fSBCX - fMarkX;
                float sbcDirZ = fSBCZ - 0.0f;

                float fSBCInvLength = nlRecipSqrt((sbcDirY * sbcDirY) + (sbcDirX * sbcDirX) + (sbcDirZ * sbcDirZ), true);

                sbcDirY = fSBCInvLength * sbcDirY;
                sbcDirX = fSBCInvLength * sbcDirX;
                sbcDirZ = fSBCInvLength * sbcDirZ;

                float fDotToNet = (sbcDirY * dirY) + (sbcDirX * dirX) + (sbcDirZ * dirZ);
                if (fDotToNet >= 0.0f)
                {
                    float fToMarkNetPassBalance = 1.0f - fMarkingNetPassBalance;
                    dirY = (fToMarkNetPassBalance * dirY) + (fMarkingNetPassBalance * sbcDirY);
                    dirX = (fToMarkNetPassBalance * dirX) + (fMarkingNetPassBalance * sbcDirX);
                    dirZ = (fToMarkNetPassBalance * dirZ) + (fMarkingNetPassBalance * sbcDirZ);
                }

                float fToNetY = pNet->m_baseLocation.f.y - fSBCY;
                float fToNetX = pNet->m_baseLocation.f.x - fSBCX;
                float fToNetZ = pNet->m_baseLocation.f.z - fSBCZ;

                float fToNetInvLength = nlRecipSqrt((fToNetY * fToNetY) + (fToNetX * fToNetX) + (fToNetZ * fToNetZ), true);

                float fThreatTargetZ = (fMarkingDistance * (fToNetInvLength * fToNetZ)) + fSBCZ;
                float fThreatTargetY = (fMarkingDistance * (fToNetInvLength * fToNetY)) + fSBCY;
                float fThreatTargetX = (fMarkingDistance * (fToNetInvLength * fToNetX)) + fSBCX;

                FuzzyVariant markBallOwner = Fuzzy::ShouldIMarkBallOwner(this);
                if (markBallOwner.mData.f > 0.0f)
                {
                    float fWeight = markBallOwner.mData.f * fMarkBallOwnerBalance;

                    vAccumulated_v3.f.z = (fWeight * fThreatTargetZ) + vAccumulated_v3.f.z;
                    vAccumulated_v3.f.y = (fWeight * fThreatTargetY) + vAccumulated_v3.f.y;
                    vAccumulated_v3.f.x = (fWeight * fThreatTargetX) + vAccumulated_v3.f.x;
                    fTotalWeight_v3 = fTotalWeight_v3 + fWeight;
                }
            }
        }

        float fMarkTargetY = (fMarkingDistance * dirY) + fMarkY;
        float fMarkTargetX = (fMarkingDistance * dirX) + fMarkX;
        float fMarkTargetZ = (fMarkingDistance * dirZ) + 0.0f;

        vAccumulated_v3.f.y = (fMarkFormationBalance * fMarkTargetY) + vAccumulated_v3.f.y;
        vAccumulated_v3.f.z = (fMarkFormationBalance * fMarkTargetZ) + vAccumulated_v3.f.z;
        vAccumulated_v3.f.x = (fMarkFormationBalance * fMarkTargetX) + vAccumulated_v3.f.x;
        fTotalWeight_v3 = fTotalWeight_v3 + fMarkFormationBalance;

        nlVector3 v3FormationPosition;
        m_DesireCommonVars.bInPosition = GetFormationPosition(v3FormationPosition, 0.0f);
        if (m_DesireCommonVars.bInPosition)
        {
            v3FormationPosition = m_v3Position;
        }

        float fFormationWeight = 1.0f - fMarkFormationBalance;
        fTotalWeight_v3 = fTotalWeight_v3 + fFormationWeight;
        vAccumulated_v3.f.z = (fFormationWeight * v3FormationPosition.f.z) + vAccumulated_v3.f.z;
        vAccumulated_v3.f.y = (fFormationWeight * v3FormationPosition.f.y) + vAccumulated_v3.f.y;
        vAccumulated_v3.f.x = (fFormationWeight * v3FormationPosition.f.x) + vAccumulated_v3.f.x;

        if (fTotalWeight_v3 > 0.0f)
        {
            m_DesireCommonVars.v3DesiredPosition.f.x = (1.0f / fTotalWeight_v3) * vAccumulated_v3.f.x;
            m_DesireCommonVars.v3DesiredPosition.f.y = (1.0f / fTotalWeight_v3) * vAccumulated_v3.f.y;
            m_DesireCommonVars.v3DesiredPosition.f.z = (1.0f / fTotalWeight_v3) * vAccumulated_v3.f.z;
        }
        else
        {
            m_DesireCommonVars.v3DesiredPosition = v3Zero;
        }
    }

    SetDesiredSpeedAndDirectionToPosition(fDeltaT, m_DesireCommonVars.v3DesiredPosition, TR_FAR_DISTANCE, 0.75f, 0.75f);
    ShouldIStrafe();
}

/**
 * Offset/Address/Size: 0x35E4 | 0x80034368 | size: 0x408
 * TODO: 84.11% match - second loop uses do-while countdown to prevent unrolling; remaining gap is mtctr/bdnz vs cmpwi/blt due to -inline auto vs -inline deferred flags
 */
void cFielder::DesireSupportBall(float fDeltaT, bool bDefensive)
{
    if (g_pBall->m_pOwner == this)
    {
        SetDesireDuration(0.0f, true);
        return;
    }

    const SupportBallAILocation* pAILocations = bDefensive ? g_vSupportBallDefensiveAILocations : g_vSupportBallOffensiveAILocations;
    int iNumSupportLocs = bDefensive ? 6 : 6;

    nlVector3 v3FutureBallFieldLocation;
    nlVector3 v3FutureBallAILocation;

    cBall* pBall = g_pBall;
    v3FutureBallFieldLocation.f.x = pBall->m_v3Position.f.x + (0.2f * pBall->GetAIVelocity()->f.x);

    pBall = g_pBall;
    v3FutureBallFieldLocation.f.y = pBall->m_v3Position.f.y + (0.2f * pBall->GetAIVelocity()->f.y);
    v3FutureBallFieldLocation.f.z = 0.0f;

    FieldLocToAILoc(v3FutureBallAILocation, v3FutureBallFieldLocation, (eTeamSide)m_pTeam->m_nSide);

    int nearestIndices[2] = { -1, -1 };
    float nearestDists[2] = { 1000000000.0f, 1000000000.0f };

    for (int i = 0; i < iNumSupportLocs; i++)
    {
        float dy = v3FutureBallAILocation.f.y - pAILocations[i].y0;
        float dx = v3FutureBallAILocation.f.x - pAILocations[i].x0;
        float dist = nlSqrt(dx * dx + dy * dy, true);

        if (dist < nearestDists[0])
        {
            nearestDists[1] = nearestDists[0];
            nearestIndices[1] = nearestIndices[0];
            nearestIndices[0] = i;
            nearestDists[0] = dist;
        }
        else if (dist < nearestDists[1])
        {
            nearestIndices[1] = i;
            nearestDists[1] = dist;
        }
    }

    nlVector2 v2LocationOffsets[2];
    nlVector2 v2SupportBallAILocs[2];

    int n = 2;
    int i = 0;
    do
    {
        const SupportBallAILocation* pLocation = &pAILocations[nearestIndices[i]];

        v2LocationOffsets[i].f.x = pLocation->x1 - pLocation->x0;
        v2LocationOffsets[i].f.y = pLocation->y1 - pLocation->y0;

        float x = v3FutureBallAILocation.f.x + v2LocationOffsets[i].f.x;
        if (x < 0.0f)
        {
            x = 0.0f;
        }
        if (x > 4.0f)
        {
            x = 4.0f;
        }
        v2SupportBallAILocs[i].f.x = x;

        float y = v3FutureBallAILocation.f.y + v2LocationOffsets[i].f.y;
        if (y < -1.0f)
        {
            y = -1.0f;
        }
        if (y > 1.0f)
        {
            y = 1.0f;
        }
        v2SupportBallAILocs[i].f.y = y;

        i++;
    } while (--n > 0);

    nlVector3 v3TargetAILoc = {
        0.0f,
        0.0f,
        0.0f,
    };

    float fWeight0 = nearestDists[1] / (nearestDists[0] + nearestDists[1]);
    float fWeight1 = 1.0f - fWeight0;

    v3TargetAILoc.f.x = (fWeight1 * v2SupportBallAILocs[1].f.x) + (fWeight0 * v2SupportBallAILocs[0].f.x);
    v3TargetAILoc.f.y = (fWeight1 * v2SupportBallAILocs[1].f.y) + (fWeight0 * v2SupportBallAILocs[0].f.y);

    AILocToFieldLoc(v3TargetAILoc, v3TargetAILoc, (eTeamSide)m_pTeam->m_nSide);

    float fTotalWeight = 0.0f;
    float fAIBallLocationWeight = 0.7f;

    nlVector3 v3DesiredPosition = v3Zero;
    v3DesiredPosition.f.x = (fAIBallLocationWeight * v3TargetAILoc.f.x) + v3DesiredPosition.f.x;
    v3DesiredPosition.f.y = (fAIBallLocationWeight * v3TargetAILoc.f.y) + v3DesiredPosition.f.y;
    v3DesiredPosition.f.z = (fAIBallLocationWeight * v3TargetAILoc.f.z) + v3DesiredPosition.f.z;

    fTotalWeight = fTotalWeight + fAIBallLocationWeight;

    nlVector3 v3FormationPosition;
    m_DesireCommonVars.bInPosition = GetFormationPosition(v3FormationPosition, 0.0f);
    if (m_DesireCommonVars.bInPosition)
    {
        v3FormationPosition = m_v3Position;
    }

    float fFormationWeight = 0.3f;
    fTotalWeight = fTotalWeight + fFormationWeight;

    v3DesiredPosition.f.x = (fFormationWeight * v3FormationPosition.f.x) + v3DesiredPosition.f.x;
    v3DesiredPosition.f.y = (fFormationWeight * v3FormationPosition.f.y) + v3DesiredPosition.f.y;
    v3DesiredPosition.f.z = (fFormationWeight * v3FormationPosition.f.z) + v3DesiredPosition.f.z;

    nlVector3 v3FinalDesiredPosition;
    if (fTotalWeight > 0.0f)
    {
        float fInvWeight = 1.0f / fTotalWeight;
        v3FinalDesiredPosition.f.x = fInvWeight * v3DesiredPosition.f.x;
        v3FinalDesiredPosition.f.y = fInvWeight * v3DesiredPosition.f.y;
        v3FinalDesiredPosition.f.z = fInvWeight * v3DesiredPosition.f.z;
    }
    else
    {
        v3FinalDesiredPosition = v3Zero;
    }

    SetDesiredSpeedAndDirectionToPosition(fDeltaT, v3FinalDesiredPosition, TR_FAR_DISTANCE, 1.0f, 1.0f);
    ShouldIStrafe();
}

/**
 * Offset/Address/Size: 0x33A0 | 0x80034124 | size: 0x244
 */
bool cFielder::InitDesireGetOpen()
{
    if (m_DesireCommonVars.pSBC == this)
    {
        if (m_sQueuedDesireParams.eDesireType == FIELDERDESIRE_GET_OPEN)
        {
            m_sQueuedDesireParams.fDuration = 0.0f;
            m_sQueuedDesireParams.eDesireType = FIELDERDESIRE_NEED_DESIRE;
            m_sQueuedDesireParams.opt1 = fvNotSet;
            m_sQueuedDesireParams.opt2 = fvNotSet;
        }
        return false;
    }

    nlVector3 v3FormationPosition;
    m_DesireCommonVars.bInPosition = GetFormationPosition(v3FormationPosition, -1.0f);
    if (m_DesireCommonVars.bInPosition)
    {
        v3FormationPosition = m_v3Position;
    }

    nlVector3 v3BestPosition = v3FormationPosition;
    const nlVector3* pTargetPosition;
    if (m_DesireCommonVars.pSBC != NULL)
    {
        pTargetPosition = &m_DesireCommonVars.pSBC->m_v3Position;
    }
    else
    {
        pTargetPosition = &g_pBall->m_v3Position;
    }

    nlVector3 v3TargetPosition = *pTargetPosition;
    v3TargetPosition.f.z = 0.0f;

    SetSpaceSearch(new (nlMalloc(0x78, 8, false)) SSearchBestPass(m_DesireCommonVars.pSBC, this, false, false));
    m_pSpaceSearch->m_bDebugOn = false;
    m_pSpaceSearch->FindBestPosition(v3BestPosition, v3FormationPosition, DIR_TOWARD_TARGET, &v3TargetPosition, 4.0f, 0x8000);

    m_DesireCommonVars.v3DesiredPosition.f.x = 0.95f * v3FormationPosition.f.x + 0.05f * v3BestPosition.f.x;
    m_DesireCommonVars.v3DesiredPosition.f.y = 0.95f * v3FormationPosition.f.y + 0.05f * v3BestPosition.f.y;
    m_DesireCommonVars.v3DesiredPosition.f.z = 0.95f * v3FormationPosition.f.z + 0.05f * v3BestPosition.f.z;

    m_pAvoidance->SetThingsToAvoid(0x1F);

    return true;
}

/**
 * Offset/Address/Size: 0x30B4 | 0x80033E38 | size: 0x2EC
 */
bool cFielder::InitDesireOneTimerFromRun(unsigned short aFutureFacingDirection, const nlVector3& v3FuturePosition, const nlVector3& v3PassIntercept, bool bVolleyPassReceive, bool bIsChipShot)
{
    extern float g_fSimulationTick;

    float fBallContactTime;

    const LooseBallContactAnimInfo* pBestBallContactAnimInfo = GetOneTimerBallContactAnimInfo(
        aFutureFacingDirection, v3FuturePosition, m_pTeam->GetOtherNet()->m_baseLocation, true, bVolleyPassReceive);

    m_DesireOneTimerVars.nOneTimerAnim = pBestBallContactAnimInfo->nAnimID;

    const cSAnim* contactAnim = m_pAnimInventory->GetAnim(pBestBallContactAnimInfo->nAnimID);
    m_DesireOneTimerVars.fOneTimerAnimTime = pBestBallContactAnimInfo->fAnimContactFrame / (float)contactAnim->m_nNumKeys;

    bool bFoundContact;
    if (bVolleyPassReceive)
    {
        bFoundContact = DoLooseBallContactFromRunVolley(
            m_DesireOneTimerVars.v3DesiredPosition,
            m_DesireOneTimerVars.fDesiredTime,
            m_DesireOneTimerVars.v3BallPosition,
            fBallContactTime,
            pBestBallContactAnimInfo,
            v3PassIntercept);
    }
    else
    {
        bFoundContact = DoLooseBallContactFromRun(
            m_DesireOneTimerVars.v3DesiredPosition,
            m_DesireOneTimerVars.fDesiredTime,
            m_DesireOneTimerVars.v3BallPosition,
            fBallContactTime,
            pBestBallContactAnimInfo,
            v3PassIntercept);
    }

    if (!bFoundContact)
    {
        return false;
    }

    m_DesireOneTimerVars.aDesiredFacingDirection = m_aActualFacingDirection;
    m_DesireOneTimerVars.bIsChipShot = bIsChipShot;
    m_DesireOneTimerVars.bVolleyPassReceive = bVolleyPassReceive;

    if (m_DesireOneTimerVars.fDesiredTime > (2.0f * g_fSimulationTick))
    {
        m_DesireOneTimerVars.fDesiredTime -= g_fSimulationTick;

        SetDesire(FIELDERDESIRE_ONETIMER, 0.5f);

        m_eDesireSubState = 0;
        InitActionRunning();

        nlVector3 v3Me2DesiredPosition;
        nlVec3Set(*(nlVector3*)&v3Me2DesiredPosition,
            m_DesireOneTimerVars.v3DesiredPosition.f.x - m_v3Position.f.x,
            m_DesireOneTimerVars.v3DesiredPosition.f.y - m_v3Position.f.y,
            m_DesireOneTimerVars.v3DesiredPosition.f.z - m_v3Position.f.z);

        unsigned short aDesiredAngle = (unsigned short)(int)(10430.378f * nlATan2f(v3Me2DesiredPosition.f.y, v3Me2DesiredPosition.f.x));

        s16 angleDiff = aDesiredAngle - m_aActualFacingDirection;
        int absDiff = angleDiff;
        if (angleDiff < 0)
            absDiff = -angleDiff;

        if ((u16)absDiff < 0x4000)
        {
            float fSpeed = nlSqrt(v3Me2DesiredPosition.f.x * v3Me2DesiredPosition.f.x + v3Me2DesiredPosition.f.y * v3Me2DesiredPosition.f.y, true) / m_DesireOneTimerVars.fDesiredTime;
            m_fDesiredSpeed = fSpeed;
            m_fActualSpeed = fSpeed;
            m_aDesiredFacingDirection = aDesiredAngle;
            m_aActualFacingDirection = aDesiredAngle;
            m_aDesiredMovementDirection = m_aDesiredFacingDirection;
        }
        else
        {
            m_fActualSpeed = 0.0f;
        }
    }
    else
    {
        const cSAnim* pOneTimerAnim = m_pAnimInventory->GetAnim(m_DesireOneTimerVars.nOneTimerAnim);
        float fAnimTimeInSecs = m_DesireOneTimerVars.fOneTimerAnimTime * ((float)pOneTimerAnim->m_nNumKeys / 30.0f);
        float fPlaybackScale = fAnimTimeInSecs / (fAnimTimeInSecs + m_DesireOneTimerVars.fDesiredTime);

        if (fPlaybackScale > 0.85f)
        {
            return false;
        }

        SetDesire(FIELDERDESIRE_ONETIMER, 0.5f);
        m_eDesireSubState = 1;

        SetFacingDirection(m_DesireOneTimerVars.aDesiredFacingDirection);

        InitActionOneTimer(
            m_DesireOneTimerVars.nOneTimerAnim,
            m_DesireOneTimerVars.v3DesiredPosition,
            m_DesireOneTimerVars.fOneTimerAnimTime,
            m_DesireOneTimerVars.bIsChipShot);

        m_pCurrentAnimController->m_fPlaybackSpeedScale = fPlaybackScale;
    }

    SetDesireDuration(3.0f, false);
    SetNoPickUpTime(3.0f);
    g_pBall->SetPassTargetTimer(fBallContactTime);
    m_pAvoidance->SetThingsToAvoid(0);

    return true;
}

/**
 * Offset/Address/Size: 0x2E60 | 0x80033BE4 | size: 0x254
 * TODO: 99.6% match - initial ball position/velocity load order and f28/f29/f30 usage differ in the dot-product precheck path.
 */
void cFielder::DesireOneTimer(float fDeltaT)
{
    cFielder* fp = this;

    float yDiff = fp->m_DesireOneTimerVars.v3BallPosition.f.y - g_pBall->m_v3Position.f.y;
    float xDiff = fp->m_DesireOneTimerVars.v3BallPosition.f.x - g_pBall->m_v3Position.f.x;
    float invLen = nlRecipSqrt(yDiff * yDiff + xDiff * xDiff, true);
    float targetDirY = invLen * yDiff;
    yDiff = invLen * xDiff;

    cBall* pBall = g_pBall;
    invLen = nlRecipSqrt(pBall->m_v3Velocity.f.x * pBall->m_v3Velocity.f.x + pBall->m_v3Velocity.f.y * pBall->m_v3Velocity.f.y, true);

    float ballDirY = invLen * pBall->m_v3Velocity.f.y;
    float ballDirX = invLen * pBall->m_v3Velocity.f.x;

    if (fp->m_pBall == NULL && fp->m_eDesireSubState != 1)
    {
        invLen = targetDirY * ballDirY + yDiff * ballDirX;
        if (invLen < 0.98f)
        {
            fp->ClearPassTargetIfAmThePassTarget();
            fp->SetDesireDuration(0.0f, true);
            return;
        }
    }

    fp->m_DesireOneTimerVars.fDesiredTime -= fDeltaT;

    switch (fp->m_eDesireSubState)
    {
    case 0:
    {
        if (fp->m_DesireOneTimerVars.fDesiredTime <= 0.0f)
        {
            float yToTarget = fp->m_v3Position.f.y - fp->m_DesireOneTimerVars.v3DesiredPosition.f.y;
            float xToTarget = fp->m_v3Position.f.x - fp->m_DesireOneTimerVars.v3DesiredPosition.f.x;

            if (xToTarget * xToTarget + yToTarget * yToTarget > 4.0f)
            {
                fp->ClearPassTargetIfAmThePassTarget();
                fp->SetDesireDuration(0.0f, true);
                return;
            }

            fp->SetFacingDirection(fp->m_DesireOneTimerVars.aDesiredFacingDirection);
            fp->InitActionOneTimer(
                fp->m_DesireOneTimerVars.nOneTimerAnim,
                fp->m_DesireOneTimerVars.v3DesiredPosition,
                fp->m_DesireOneTimerVars.fOneTimerAnimTime,
                fp->m_DesireOneTimerVars.bIsChipShot);
            fp->m_eDesireSubState = 1;

            cSAnim* pAnim = fp->m_pAnimInventory->GetAnim(fp->m_DesireOneTimerVars.nOneTimerAnim);
            float oneTimerTime = fp->m_DesireOneTimerVars.fOneTimerAnimTime * ((float)pAnim->m_nNumKeys / 30.0f);
            float totalTime = oneTimerTime + fp->m_DesireOneTimerVars.fDesiredTime;

            if (oneTimerTime > 0.0f && totalTime > 0.0f)
            {
                fp->m_pCurrentAnimController->m_fPlaybackSpeedScale = oneTimerTime / totalTime;
            }
        }
        break;
    }

    case 1:
    {
        if (fp->IsActionDone())
        {
            fp->SetDesireDuration(0.0f, true);
        }
        break;
    }

    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x2D08 | 0x80033A8C | size: 0x158
 */
void cFielder::InitDesireReceivePassFromIdle(const LooseBallContactAnimInfo* pAnimInfo, unsigned short aAngle, bool bVolley)
{
    m_DesireReceivePassSharedVars.aDesiredFacingDirection = aAngle;
    m_DesireReceivePassSharedVars.nReceivePassAnim = pAnimInfo->nAnimID;

    cSAnim* pAnim = m_pAnimInventory->GetAnim(pAnimInfo->nAnimID);
    unsigned int nNumKeys = pAnim->m_nNumKeys;

    float fDesiredTime;

    m_DesireReceivePassSharedVars.fReceivePassAnimTime = pAnimInfo->fAnimContactFrame / (float)nNumKeys;
    m_DesireReceivePassSharedVars.iAttemptOneTouchShot = 0;
    m_DesireReceivePassSharedVars.bFailedToInitOneTouchShot = false;
    m_DesireReceivePassSharedVars.iAttemptOneTouchPass = 0;
    m_DesireReceivePassSharedVars.bVolleyPassReceive = bVolley;
    m_DesireReceivePassSharedVars.pOneTouchPassTarget = NULL;

    bool savedTiltForce = g_pBall->m_pPhysicsBall->m_bUseTiltForce;
    g_pBall->m_pPhysicsBall->m_bUseTiltForce = false;

    bool result = DoLooseBallContactFromIdle(
        m_DesireReceivePassSharedVars.v3DesiredPosition,
        m_DesireReceivePassSharedVars.fDesiredTime,
        m_DesireReceivePassSharedVars.v3BallPosition,
        fDesiredTime,
        aAngle,
        pAnimInfo);

    g_pBall->m_pPhysicsBall->m_bUseTiltForce = savedTiltForce;

    if (result)
    {
        SetDesire(FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE, 1.0f);
        SetDesireDuration(0.0f, false);
        InitActionIdleTurn(aAngle);
        m_eDesireSubState = 0;
        SetNoPickUpTime(0.0f);
        g_pBall->SetPassTargetTimer(fDesiredTime);
        g_pBall->SetPassTarget(this, m_DesireReceivePassSharedVars.v3BallPosition, bVolley);
        m_DesireCommonVars.fMisc = fDesiredTime;
        m_pAvoidance->SetThingsToAvoid(0);
    }
}

/**
 * Offset/Address/Size: 0x2080 | 0x80032E04 | size: 0xC88
 */
void cFielder::DesireReceivePassFromIdle(float)
{
}

/**
 * Offset/Address/Size: 0x1DE8 | 0x80032B6C | size: 0x298
 */
void cFielder::InitDesireReceivePassFromRun(const LooseBallContactAnimInfo* pAnimInfo, const nlVector3& rv3Velocity, bool bVolley, const nlVector3& v3PassIntercept)
{
    extern float g_fSimulationTick;

    float fDesiredTime;

    SetVelocity(rv3Velocity);
    SetFacingDirection((unsigned short)(int)(10430.378f * nlATan2f(rv3Velocity.f.y, rv3Velocity.f.x)));

    m_DesireReceivePassSharedVars.aDesiredFacingDirection = m_aActualFacingDirection;
    m_DesireReceivePassSharedVars.nReceivePassAnim = pAnimInfo->nAnimID;

    cSAnim* pAnim = m_pAnimInventory->GetAnim(pAnimInfo->nAnimID);
    unsigned int nNumKeys = pAnim->m_nNumKeys;

    m_DesireReceivePassSharedVars.fReceivePassAnimTime = pAnimInfo->fAnimContactFrame / (float)nNumKeys;
    m_DesireReceivePassSharedVars.iAttemptOneTouchShot = 0;
    m_DesireReceivePassSharedVars.bFailedToInitOneTouchShot = false;
    m_DesireReceivePassSharedVars.iAttemptOneTouchPass = 0;
    m_DesireReceivePassSharedVars.bVolleyPassReceive = bVolley;
    m_DesireReceivePassSharedVars.pOneTouchPassTarget = NULL;

    bool savedTiltForce = g_pBall->m_pPhysicsBall->m_bUseTiltForce;
    g_pBall->m_pPhysicsBall->m_bUseTiltForce = false;

    bool result = DoLooseBallContactFromRun(
        m_DesireReceivePassSharedVars.v3DesiredPosition,
        m_DesireReceivePassSharedVars.fDesiredTime,
        m_DesireReceivePassSharedVars.v3BallPosition,
        fDesiredTime,
        pAnimInfo,
        v3PassIntercept);

    g_pBall->m_pPhysicsBall->m_bUseTiltForce = savedTiltForce;

    if (result)
    {
        SetDesire(FIELDERDESIRE_RECEIVE_PASS_FROM_RUN, 0.5f);
        SetDesireDuration(3.0f, false);

        if (m_DesireReceivePassSharedVars.fDesiredTime > (2.0f * g_fSimulationTick))
        {
            nlVector3 v3DesiredDelta;

            m_DesireReceivePassSharedVars.fDesiredTime -= g_fSimulationTick;
            m_eDesireSubState = 0;

            InitActionRunning();
            SetRunningAnimState(0.1f);

            nlVec3Set(*(nlVector3*)&v3DesiredDelta,
                m_DesireReceivePassSharedVars.v3DesiredPosition.f.x - m_v3Position.f.x,
                m_DesireReceivePassSharedVars.v3DesiredPosition.f.y - m_v3Position.f.y,
                m_DesireReceivePassSharedVars.v3DesiredPosition.f.z - m_v3Position.f.z);
            float fSpeed = nlGetLength2D(v3DesiredDelta.f.x, v3DesiredDelta.f.y) / m_DesireReceivePassSharedVars.fDesiredTime;

            m_fDesiredSpeed = fSpeed;
            m_fActualSpeed = fSpeed;

            unsigned short aDesiredAngle = (unsigned short)(int)(10430.378f * nlATan2f(v3DesiredDelta.f.y, v3DesiredDelta.f.x));
            m_aDesiredFacingDirection = aDesiredAngle;
            m_aActualFacingDirection = aDesiredAngle;
            m_aDesiredMovementDirection = m_aDesiredFacingDirection;
        }
        else
        {
            InitActionReceivePass(
                m_DesireReceivePassSharedVars.nReceivePassAnim,
                m_DesireReceivePassSharedVars.v3DesiredPosition,
                m_DesireReceivePassSharedVars.fReceivePassAnimTime);

            m_eDesireSubState = 1;

            cSAnim* pReceivePassAnim = m_pAnimInventory->GetAnim(m_DesireReceivePassSharedVars.nReceivePassAnim);
            m_pCurrentAnimController->m_fPlaybackSpeedScale = (m_DesireReceivePassSharedVars.fReceivePassAnimTime * ((float)pReceivePassAnim->m_nNumKeys / 30.0f)) / fDesiredTime;
        }

        SetNoPickUpTime(3.0f);
        g_pBall->SetPassTargetTimer(fDesiredTime);
        g_pBall->SetPassTarget(this, m_DesireReceivePassSharedVars.v3BallPosition, bVolley);

        m_DesireCommonVars.tMiscTimer.m_uPackedTime = 0;
        m_DesireCommonVars.fMisc = fDesiredTime;

        m_pAvoidance->SetThingsToAvoid(0);
    }
}

/**
 * Offset/Address/Size: 0x130C | 0x80032090 | size: 0xADC
 */
void cFielder::DesireReceivePassFromRun(float)
{
}

/**
 * Offset/Address/Size: 0xEE4 | 0x80031C68 | size: 0x428
 * TODO: 91.25% match - normalization instruction scheduling (fmuls/fadds swap) cascades to callee-saved FPR allocation diffs in turbo/reaction sections
 */
u8 cFielder::InitDesireRunToNet()
{
    if (m_pBall == NULL)
    {
        if (m_sQueuedDesireParams.eDesireType == FIELDERDESIRE_RUN_TO_NET)
        {
            m_sQueuedDesireParams.fDuration = 0.0f;
            m_sQueuedDesireParams.eDesireType = FIELDERDESIRE_NEED_DESIRE;
            m_sQueuedDesireParams.opt1 = fvNotSet;
            m_sQueuedDesireParams.opt2 = fvNotSet;
        }
        return 0;
    }

    SpaceSearch* pSpaceSearch = new (nlMalloc(0x4C, 8, false)) SSearchRunToNet(this);
    SetSpaceSearch(pSpaceSearch);

    m_pSpaceSearch->m_bDebugOn = false;

    nlVector3 v3BestPosition;
    m_pSpaceSearch->FindBestPosition(v3BestPosition, m_v3Position, DIR_NONE, NULL, 4.0f, 0x8000);

    nlVector3 v3DesiredVelDirection;
    nlVec3Sub(v3DesiredVelDirection, v3BestPosition, m_v3Position);

    float fInvDistance = nlRecipSqrt(
        (v3DesiredVelDirection.f.x * v3DesiredVelDirection.f.x) + (v3DesiredVelDirection.f.y * v3DesiredVelDirection.f.y) + (v3DesiredVelDirection.f.z * v3DesiredVelDirection.f.z), true);

    _nlVec3Scale(v3DesiredVelDirection, fInvDistance);

    float fInvVelocity = nlRecipSqrt(
        (m_v3Velocity.f.x * m_v3Velocity.f.x) + (m_v3Velocity.f.y * m_v3Velocity.f.y) + (m_v3Velocity.f.z * m_v3Velocity.f.z), true);

    float fNormVelY = fInvVelocity * m_v3Velocity.f.y;
    float fNormVelX = fInvVelocity * m_v3Velocity.f.x;
    float fNormVelZ = fInvVelocity * m_v3Velocity.f.z;

    float fDot = (fNormVelX * v3DesiredVelDirection.f.x) + (fNormVelY * v3DesiredVelDirection.f.y) + (fNormVelZ * v3DesiredVelDirection.f.z);

    m_DesireCommonVars.v3DesiredPosition = v3DesiredVelDirection;
    m_DesireCommonVars.turboRequest = TR_FAR_DISTANCE;

    if (fDot >= 0.8f)
    {
        nlVector3 v3ToPosition;
        v3ToPosition.f.z = m_v3Position.f.z + (8.0f * v3DesiredVelDirection.f.z);
        v3ToPosition.f.x = m_v3Position.f.x + (8.0f * v3DesiredVelDirection.f.x);
        v3ToPosition.f.y = m_v3Position.f.y + (8.0f * v3DesiredVelDirection.f.y);

        float bTurboChance = (float)g_vDesireCommonData[m_eFielderDesireState].m_RandomChanceGen.genrand(
            SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->Off_TurboChance);

        float fOpenToPosition = OpenToPosition(m_v3Position, v3ToPosition, m_pTeam->GetOtherTeam(), this, NULL, false);
        float fOpen = Open(g_pScriptCurrentFielder);
        float fBreakaway = OnBreakaway(g_pScriptCurrentFielder);
        float fInvincible = Invincible(g_pScriptCurrentFielder);

        fOpen = (fOpen < fOpenToPosition) ? fOpenToPosition : fOpen;
        fBreakaway = (fBreakaway < fOpen) ? fOpen : fBreakaway;

        if (fInvincible >= fBreakaway)
        {
            fBreakaway = fInvincible;
        }

        float fFarToGoalie = FarToTheirGoalie(g_pScriptCurrentFielder);

        u8 bForceTurbo = 0;
        if (bTurboChance != 0.0f)
        {
            fFarToGoalie = (fFarToGoalie > fBreakaway) ? fBreakaway : fFarToGoalie;

            if (fFarToGoalie >= 0.7f)
            {
                bForceTurbo = 1;
            }
        }

        m_DesireCommonVars.turboRequest = (bForceTurbo != 0) ? TR_FORCED_ON : TR_FAR_DISTANCE;
    }

    float fReaction = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->Off_Reaction;
    float fReactionRandom = 0.7f * (0.3f * (1.0f - fReaction));
    float fReactionOffset = nlRandomf(fReactionRandom, &nlDefaultSeed) - (0.5f * fReactionRandom);
    m_DesireCommonVars.fMisc = 0.7f + fReactionOffset;

    m_pAvoidance->SetThingsToAvoid(0x1F);
    return 1;
}

/**
 * Offset/Address/Size: 0xCA8 | 0x80031A2C | size: 0x23C
 * TODO: 99.8% match - r29/r30 swapped for g_pBall and ballVelocity ref around GetClosingSpeed2D call
 */
void cFielder::DesireSlideAttack(float fDeltaT)
{
    float fTime;
    nlVector3 v3VictimPosition;
    float fBallClosingSpeed;

    switch (m_eDesireSubState)
    {
    case 0:
    {
        if (m_pMark == NULL || m_DesireSlideAttackVars.m_pSlideAttackTarget == NULL || m_DesireSlideAttackVars.m_pSlideAttackTarget != g_pBall->m_pOwner)
        {
            SetDesireDuration(0.0f, true);
            break;
        }

        if (CanISlideAttack(m_DesireSlideAttackVars.m_pSlideAttackTarget->m_v3Position,
                m_DesireSlideAttackVars.m_pSlideAttackTarget->m_v3Velocity,
                &fTime))
        {
            InitActionSlideAttack(m_DesireSlideAttackVars.m_pSlideAttackTarget, fTime);
            m_eDesireSubState = 1;
            break;
        }

        cFielder* pTarget = m_DesireSlideAttackVars.m_pSlideAttackTarget;
        v3VictimPosition.f.x = pTarget->m_v3Position.f.x + 0.25f * pTarget->m_v3Velocity.f.x;
        v3VictimPosition.f.y = pTarget->m_v3Position.f.y + 0.25f * pTarget->m_v3Velocity.f.y;
        v3VictimPosition.f.z = 0.0f;

        u8 turbo = ShouldITurboWithoutBall();
        SetDesiredSpeedAndDirectionToPosition(fDeltaT, v3VictimPosition, (eTurboRequest)(turbo != 0), 1.0f, 1.0f);

        m_pAvoidance->UseMinimumAvoidance(m_DesireSlideAttackVars.m_pSlideAttackTarget);
        ShouldIStrafe();
        break;
    }
    case 1:
    {
        SetDesireDuration(999999.9f, true);

        if (m_tSlideAttackTimer.m_uPackedTime != 0)
        {
            if (mActionSlideAttackVars.bAttackSucceeded == 0)
            {
                float fBallSpeed = nlSqrt(
                    g_pBall->m_v3Velocity.f.x * g_pBall->m_v3Velocity.f.x + g_pBall->m_v3Velocity.f.y * g_pBall->m_v3Velocity.f.y + g_pBall->m_v3Velocity.f.z * g_pBall->m_v3Velocity.f.z,
                    true);

                if (fBallSpeed > 0.05f)
                {
                    const nlVector3& ballVelocity = g_pBall->m_v3Velocity;
                    fBallClosingSpeed = GetClosingSpeed2D(GetJointPosition(m_nLeftFootJointIndex), m_v3Velocity, g_pBall->m_v3Position, ballVelocity);
                    if (fBallClosingSpeed < 0.0f)
                    {
                        if (nlRandomf(1.0f, &nlDefaultSeed) > 0.5f)
                        {
                            m_tSlideAttackTimer.SetSeconds(0.0f);
                            m_eDesireSubState = 2;
                        }
                    }
                }
            }
        }
        else
        {
            m_eDesireSubState = 2;
        }
        break;
    }
    case 2:
    {
        if (m_eActionState == ACTION_NEED_ACTION)
        {
            SetDesireDuration(0.0f, true);
        }
        break;
    }
    }
}

/**
 * Offset/Address/Size: 0x794 | 0x80031518 | size: 0x514
 * TODO: 96.57% match - mr r0,r3 intermediate at a0 instead of direct mr r30,r3
 */
void cFielder::DesireUserControlled(float fDeltaT)
{
    bool bWasActionTaken;
    nlPolar p;
    nlVector3 v3Velocity;

    if (GetGlobalPad() == NULL)
    {
        SetDesireDuration(0.0f, true);
        return;
    }

    if (g_pGame->m_eGameState == GS_KICKOFF)
    {
        if (mbCanKickoff && m_pBall != NULL)
        {
            bWasActionTaken = false;

            if (GetGlobalPad()->JustPressed(PAD_PASS, true))
            {
                bWasActionTaken = GetGlobalPad()->JustPressed(PAD_AIM, true);
                InitActionPass(DoFindBestPassTarget(GetGlobalPad()->JustPressed(PAD_AIM, true), false), bWasActionTaken, true);
                bWasActionTaken = true;
            }
            else if (GetGlobalPad()->JustPressed(PAD_DEKE, true) || m_pController->GetCStickMovementStickMagnitude() > 0.0f)
            {
                InitActionDeke(PAD_DEKE);
                bWasActionTaken = true;
            }
            else if (GetGlobalPad()->JustPressed(PAD_SHOOT, true))
            {
                DoResetShotMeter(0.0f);
                ShotMeter* pShotMeter = m_pShotMeter;
                pShotMeter->CalcOneTimerValue(this, UsePerfectPass());
                InitActionShot(GetGlobalPad()->JustPressed(PAD_AIM, true));
                bWasActionTaken = true;
            }
            else if (GetGlobalPad()->JustPressed(PAD_USE, true))
            {
                if (!IsPlayingPowerupAnim())
                {
                    UseTeamPowerup(NULL);
                }
                StartRunning();
                bWasActionTaken = true;
            }
            else if (m_pController->GetMovementStickMagnitude() > 0.001f)
            {
                if (mtKickOffWaitTimer.GetSeconds() > 0.15f)
                {
                    mtKickOffWaitTimer.SetSeconds(0.15f);
                }
                else if (mtKickOffWaitTimer.GetSeconds() < 0.05f)
                {
                    StartRunning();
                    bWasActionTaken = true;
                }
            }

            if (bWasActionTaken)
            {
                g_pEventManager->CreateValidEvent(0xb, 0x14);
                mtKickOffWaitTimer.SetSeconds(0.0f);
                mbCanKickoff = false;
            }
        }

        m_fDesiredSpeed = 0.0f;
        m_aDesiredFacingDirection = m_aActualFacingDirection;
        m_aDesiredMovementDirection = m_aActualFacingDirection;

        if (GetGlobalPad()->JustPressed(PAD_TOGGLE_POWERUP, true))
        {
            m_pTeam->TogglePowerup(false);
        }
    }
    else
    {
        if (!g_pGame->mbCaptainShotToScoreOn && GetGlobalPad()->JustPressed(PAD_USE, true) && !IsPlayingPowerupAnim())
        {
            UseTeamPowerup(NULL);
        }

        if (GetGlobalPad()->JustPressed(PAD_TOGGLE_POWERUP, true))
        {
            m_pTeam->TogglePowerup(false);
        }

        if (m_eActionState == ACTION_SHOOT_TO_SCORE)
        {
            return;
        }

        if (m_eActionState == ACTION_NEED_ACTION)
        {
            StartRunning();
        }

        SetDesiredFacingDirection();

        if (m_eActionState == ACTION_RUNNING)
        {
            TestButtonsRunning();
            if (m_pController->IsTurboPressed())
                SetDesiredSpeed(m_pTweaks->fRunningSpeed, ((FielderTweaks*)m_pTweaks)->fRunningTurboSpeed);
            else
                SetDesiredSpeed(m_pTweaks->fJoggingSpeed, m_pTweaks->fRunningSpeed);
            if (g_pBall->m_pOwner == NULL)
                DoPositioningInterceptBall();
        }
        else if (m_eActionState == ACTION_RUNNING_WB)
        {
            TestButtonsRunningWB(fDeltaT);
            if (m_pController->IsTurboPressed())
                SetDesiredSpeed(((FielderTweaks*)m_pTweaks)->fRunningWBSpeed, ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel1);
            else
                SetDesiredSpeed(m_pTweaks->fJoggingSpeed, ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed);
        }
        else if (m_eActionState == ACTION_RUNNING_WB_TURBO)
        {
            if (IsBallAwayFromCarrier())
            {
                TestButtonsToQueueActions(fDeltaT);
            }
            else if (!TestQueuedActions())
            {
                TestButtonsRunningWB(fDeltaT);
                u8 bIsShotActive = false;
                eShotMeterState state = m_pShotMeter->m_eShotMeterState;
                if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
                    bIsShotActive = true;
                if (bIsShotActive)
                {
                    m_fActualSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed;
                    InitActionRunningWB(false);
                }
            }
        }

        p.a = m_aDesiredMovementDirection;
        p.r = m_fDesiredSpeed;
        nlPolarToCartesian(v3Velocity, p);
        v3Velocity.f.z = 0.0f;
        m_v3DesiredPosition.f.x = 0.25f * v3Velocity.f.x + m_v3Position.f.x;
        m_v3DesiredPosition.f.y = 0.25f * v3Velocity.f.y + m_v3Position.f.y;
        m_v3DesiredPosition.f.z = 0.25f * v3Velocity.f.z + m_v3Position.f.z;

        if (m_pTeam->mpCurrentSituation != SITUATION_LOOSE)
            ShouldIStrafe();
        ShouldIWave();
    }
}

/**
 * Offset/Address/Size: 0x41C | 0x800311A0 | size: 0x378
 */
void cFielder::DesireUsePowerup(float)
{
    extern float Offensive(cTeam*);

    if (Offensive(this != NULL ? m_pTeam : NULL))
    {
        SetDesireDuration(0.0f, true);
        return;
    }

    switch (m_ePrevFielderDesireState)
    {
    case FIELDERDESIRE_INTERCEPT_BALL:
        InitDesire(FIELDERDESIRE_INTERCEPT_BALL, 1.0f, 0.5f, fvNotSet, fvNotSet);
        break;

    default:
        InitDesire(FIELDERDESIRE_MARK, 1.0f, 0.5f, fvNotSet, fvNotSet);
        break;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80030D84 | size: 0x41C
 * TODO: 99.26% match - r3/r4 register swap for m_pShotMeter ptr vs shotMeterState, instruction scheduling in FuzzyVariant construction
 */
void cFielder::DesireWindupShot(float)
{
    if (m_pBall == NULL)
    {
        SetDesireDuration(0.0f, true);
        return;
    }

    float fDesiredSpeed = m_fActualSpeed;
    float fMaxSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed;
    fDesiredSpeed = (fDesiredSpeed <= fMaxSpeed) ? fDesiredSpeed : fMaxSpeed;
    m_fDesiredSpeed = fDesiredSpeed;

    if (m_DesireWindupForShotVars.bIsBallAwayFromCarrier)
    {
        if (!IsBallAwayFromCarrier())
        {
            DoResetShotMeter(0.0f);
            SetDesireDuration(m_pShotMeter->GetTotalDuration(), false);

            m_DesireWindupForShotVars.bIsBallAwayFromCarrier = false;

            m_DesireCommonVars.tMiscTimer.SetSeconds(g_pGame->m_pGameTweaks->unk2D0 / 3.0f);
        }
        else
        {
            return;
        }
    }

    bool bShootToScore = false;
    unsigned char bSwitchToShootDesire = 0;

    if (m_DesireCommonVars.tMiscTimer.m_uPackedTime == 0)
    {
        if (DoAIWindupActionSelection())
        {
            m_DesireCommonVars.tMiscTimer.SetSeconds(10000000.0f);
        }
        else
        {
            float fTimer = (g_pGame->m_pGameTweaks->unk2D0 / 3.0f) - 0.05f;
            float fMinTimer = 0.1f;
            fMinTimer = (fMinTimer >= fTimer) ? fMinTimer : fTimer;
            m_DesireCommonVars.tMiscTimer.SetSeconds(fMinTimer);
        }
    }

    bool bMeterWindupState = false;
    eShotMeterState shotMeterState = m_pShotMeter->m_eShotMeterState;
    if (shotMeterState == SHOT_METER_ACTIVE || shotMeterState == SHOT_METER_STS_ACTIVE || shotMeterState == SHOT_METER_STS_TRANSISTION)
    {
        bMeterWindupState = true;
    }

    if (bMeterWindupState)
    {
        if (m_pShotMeter->m_eShotMeterState == SHOT_METER_STS_TRANSISTION)
        {
            static FilteredRandomChance s2sChanceGen;
            bool bS2SChance = s2sChanceGen.genrand(SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->Off_CaptainS2SChance);

            float fInvincible = Invincible(g_pScriptCurrentFielder);
            float fOpen = Open(g_pScriptCurrentFielder);
            fOpen = (fOpen >= fInvincible) ? fOpen : fInvincible;

            if (fOpen >= 0.8f && NearToTheirGoalie(g_pScriptCurrentFielder) <= 0.65f && bS2SChance)
            {
                KillWindup(this, "ball_shot_windup", true);
                EmitWindupAtCharacter(this, "ball_sts_windup");
            }
            else
            {
                bSwitchToShootDesire = 1;
            }
        }
    }
    else
    {
        if (m_pShotMeter->m_eShotMeterState == SHOT_METER_RELEASED)
        {
            bSwitchToShootDesire = 1;
        }
        else if (m_pShotMeter->m_eShotMeterState == SHOT_METER_STS_RELEASED)
        {
            bShootToScore = true;
            bSwitchToShootDesire = 1;
        }
    }

    if (m_tDesireDuration.m_uPackedTime == 0)
    {
        bSwitchToShootDesire = 1;
    }

    if (bSwitchToShootDesire)
    {
        SetDesireDuration(0.0f, true);
        InitDesire(FIELDERDESIRE_SHOOT, m_fDesireConfidence, -1.0f, FuzzyVariant(bShootToScore), fvNotSet);
    }
}
