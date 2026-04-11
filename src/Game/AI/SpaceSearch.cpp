#include "Game/AI/SpaceSearch.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/Fuzzy.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/AI/AiUtil.h"

static nlVector2 g_vRunToNetDistanceConfidence = { 50.0f, 1.5f };      /* const */
static nlVector2 g_vRunToNetFormationDistConfidence = { 10.0f, 1.5f }; /* const */

/**
 * Offset/Address/Size: 0xB0 | 0x800642BC | size: 0xD8
 */
SSearchBestPass::~SSearchBestPass()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x8006420C | size: 0xB0
 */
SSearchRunToNet::~SSearchRunToNet()
{
}

/**
 * Offset/Address/Size: 0x1874 | 0x800641C4 | size: 0x48
 */
SpaceSearch::~SpaceSearch()
{
}

/**
 * Offset/Address/Size: 0x1284 | 0x80063BD4 | size: 0x5F0
 */
void SpaceSearch::FindBestPosition(nlVector3&, const nlVector3&, eFieldDirection, const nlVector3*, float, unsigned short)
{
}

/**
 * Offset/Address/Size: 0x117C | 0x80063ACC | size: 0x108
 */
SSearchOpenLane::SSearchOpenLane(cPlayer* pPlayer1, cPlayer* pPlayer2)
    : SpaceSearch(pPlayer1 != NULL ? pPlayer1->m_pTeam->m_pNet->m_sideSign : pPlayer2->m_pTeam->m_pNet->m_sideSign)
{
    if (pPlayer2 != NULL)
    {
        m_pBallOwner = pPlayer1;
        m_pPassTarget = pPlayer2;
        m_bOtherPosIsTarget = false;

        if (m_pBallOwner != NULL)
        {
            m_v3OtherPos = pPlayer1->m_v3Position;
        }
        else
        {
            m_v3OtherPos = g_pBall->m_v3Position;
        }
    }
    else
    {
        m_pBallOwner = pPlayer1;
        m_pPassTarget = NULL;
        m_v3OtherPos = pPlayer1->GetAIOffNetLocation(NULL);
        m_bOtherPosIsTarget = true;
    }
}

/**
 * Offset/Address/Size: 0x1060 | 0x800639B0 | size: 0x11C
 */
float SSearchOpenLane::EvaluatePosition(const nlVector3& position, const nlVector3&, eFieldDirection, unsigned short)
{
    float fWeightedSum = 0.0f;
    float fTotalWeight = 0.0f;

    float fOpenToPosition;
    if (m_bOtherPosIsTarget)
    {
        fOpenToPosition = OpenToPosition(position, m_v3OtherPos, NULL, m_pBallOwner, m_pPassTarget, false);
    }
    else
    {
        fOpenToPosition = OpenToPosition(m_v3OtherPos, position, NULL, m_pBallOwner, m_pPassTarget, false);
    }

    fOpenToPosition = 0.0f + NormalizeVal(fOpenToPosition, 0.0f, 0.8f);
    fWeightedSum += fOpenToPosition;
    fTotalWeight += 1.0f;

    if (m_pPassTarget != NULL)
    {
        float fWideOpen = WideOpenPosition(position, m_pPassTarget->m_pTeam->GetOtherTeam(), NULL);
        fWeightedSum += 0.55f * fWideOpen;
        fTotalWeight += 0.55f;
    }

    float fNearToSideline = NearToSideline(position);
    float fAwayFromSideline = 1.0f - fNearToSideline;
    fWeightedSum += 0.2f * fAwayFromSideline;
    fTotalWeight += 0.2f;

    if (fTotalWeight > 0.0f)
    {
        return fWeightedSum / fTotalWeight;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0xF18 | 0x80063868 | size: 0x148
 */
float SSearchIdealShot::EvaluatePosition(const nlVector3& position, const nlVector3&, eFieldDirection, unsigned short)
{
    float fWeightedSum = 0.0f;
    float fTotalWeight = 0.0f;

    float fOpenToPosition = OpenToPosition(position, m_SSearchOpenLane.m_pBallOwner->GetAIOffNetLocation(NULL), NULL, m_SSearchOpenLane.m_pBallOwner, NULL, false);
    fWeightedSum += 0.5f * fOpenToPosition + 0.0f;
    fTotalWeight += 0.5f;

    float fOtherOpenToPosition = OpenPosition(position, m_SSearchOpenLane.m_pBallOwner->m_pTeam->GetOtherTeam(), m_SSearchOpenLane.m_pBallOwner, NULL);
    fWeightedSum += 0.3f * fOtherOpenToPosition;
    fTotalWeight += 0.3f;

    const cNet* pNet = m_pGoalie->m_pTeam->m_pNet;

    float fLikelyToScore = LikelyToScoreFromPosition(position, m_pGoalie->m_v3Position, pNet, false);
    fWeightedSum += 1.5f * fLikelyToScore;
    fTotalWeight += 1.5f;

    float fInFrontOfNet = PositionIsInFrontOfNet(position, pNet);
    fWeightedSum += 0.5f * fInFrontOfNet;
    fTotalWeight += 0.5f;

    float fNearToGoalie = NearToGoaliePosition(position, m_pGoalie->m_v3Position);
    fWeightedSum += 1.0f - fNearToGoalie;
    fTotalWeight += 1.0f;

    if (fTotalWeight > 0.0f)
    {
        return fWeightedSum / fTotalWeight;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0xCEC | 0x8006363C | size: 0x22C
 */
SSearchBestPass::SSearchBestPass(cPlayer* pBallOwner, cPlayer* pPassTarget, bool bAllowLeadPass, bool bIsPerfectPass)
    : SpaceSearch(pBallOwner != NULL ? pBallOwner->m_pTeam->m_pNet->m_sideSign : pPassTarget->m_pTeam->m_pNet->m_sideSign)
    , m_SSearchIdealShot(pPassTarget)
    , m_SSearchOpenLane(pBallOwner, pPassTarget)
{
    m_bAllowLeadPass = bAllowLeadPass;
    m_bIsPerfectPass = bIsPerfectPass;
}

/**
 * Offset/Address/Size: 0xA10 | 0x80063360 | size: 0x2DC
 * TODO: 98.74% match - remaining f29/f30/f31 register allocation mismatch.
 */
float CalcIdealShootingPositionScore(const nlVector3& v3TestPosition, const nlVector3& v3OtherPosition, const nlVector2& v2OffNetPosition, float fMaxDistance)
{
    float fMaxDistanceLocal;
    float fIdealDistance;
    float fScore;
    float fDy;
    float fDx;
    float fCandidateX;
    float fCandidateY;

    fMaxDistanceLocal = fMaxDistance;
    fDy = v2OffNetPosition.f.y - v3TestPosition.f.y;
    fDx = v2OffNetPosition.f.x - v3TestPosition.f.x;
    fScore = 0.0f;

    fIdealDistance = g_pGame->m_pFuzzyTweaks->vIdealDistanceForShooting.f.x;

    {
        float fDistSq = fDy * fDy;
        fDistSq += fDx * fDx;

        if (fDistSq <= fIdealDistance * fIdealDistance)
        {
            float fInvDist = nlRecipSqrt(fDistSq, true);
            float fNegIdealDistance = -fIdealDistance;
            fCandidateX = v2OffNetPosition.f.x + fNegIdealDistance * (fInvDist * fDx);
            fCandidateY = v2OffNetPosition.f.y + fNegIdealDistance * (fInvDist * fDy);
        }
        else
        {
            float fDist = nlSqrt(fDistSq, true);
            float fScale = (fDist - fIdealDistance) / fDist;
            fCandidateX = v3TestPosition.f.x + fScale * fDx;
            fCandidateY = v3TestPosition.f.y + fScale * fDy;
        }
    }

    {
        float fDeltaY = fCandidateY - v3TestPosition.f.y;
        float fDeltaX = fCandidateX - v3TestPosition.f.x;
        float fIdealRange = g_pGame->m_pFuzzyTweaks->vIdealDistanceForShooting.f.y;

        if ((fDeltaX * fDeltaX + fDeltaY * fDeltaY) < (fIdealRange * fIdealRange))
        {
            nlVector3 v3OffNetPos = { 0.0f, 0.0f, 0.0f };
            v3OffNetPos.f.x = v2OffNetPosition.f.x;
            v3OffNetPos.f.y = v2OffNetPosition.f.y;
            fScore = PositionIsAtIdealDistanceForShooting(v3TestPosition, v3OffNetPos);
        }
        else
        {
            float fDx2 = v2OffNetPosition.f.x - v3OtherPosition.f.x;
            float fDy2 = v2OffNetPosition.f.y - v3OtherPosition.f.y;
            float fIdealDistance2 = g_pGame->m_pFuzzyTweaks->vIdealDistanceForShooting.f.x;
            float fCandidateX2;
            float fCandidateY2;
            float fDistSq2 = fDy2 * fDy2;
            fDistSq2 += fDx2 * fDx2;

            if (fDistSq2 <= fIdealDistance2 * fIdealDistance2)
            {
                float fInvDist2 = nlRecipSqrt(fDistSq2, true);
                float fNegIdealDistance2 = -fIdealDistance2;
                fCandidateX2 = v2OffNetPosition.f.x + fNegIdealDistance2 * (fInvDist2 * fDx2);
                fCandidateY2 = v2OffNetPosition.f.y + fNegIdealDistance2 * (fInvDist2 * fDy2);
            }
            else
            {
                float fDist2 = nlSqrt(fDistSq2, true);
                float fScale2 = (fDist2 - fIdealDistance2) / fDist2;
                fCandidateX2 = v3OtherPosition.f.x + fScale2 * fDx2;
                fCandidateY2 = v3OtherPosition.f.y + fScale2 * fDy2;
            }

            {
                float fCandidateLenSq = fCandidateX2 * fCandidateX2 + fCandidateY2 * fCandidateY2;
                if (fCandidateLenSq > 0.0f)
                {
                    float fDy3 = v3TestPosition.f.y - v3OtherPosition.f.y;
                    float fDx3 = v3TestPosition.f.x - v3OtherPosition.f.x;
                    float fDeltaLenSq = fDx3 * fDx3 + fDy3 * fDy3;

                    if (fDeltaLenSq > 0.0f)
                    {
                        float fInvDeltaLen;
                        float fInvCandidateLen;
                        float fDot;

                        fScore = NormalizeVal(nlSqrt(fDeltaLenSq, true), 0.0f, fMaxDistanceLocal);

                        fInvDeltaLen = nlRecipSqrt(fDeltaLenSq, true);
                        fInvCandidateLen = nlRecipSqrt(fCandidateLenSq, true);

                        fDot = (fInvDeltaLen * fDy3) * (fInvCandidateLen * fCandidateY2)
                             + (fInvDeltaLen * fDx3) * (fInvCandidateLen * fCandidateX2);
                        fScore = fScore * NormalizeVal(fDot, 0.0f, 1.0f);
                    }
                }
            }
        }
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0x594 | 0x80062EE4 | size: 0x47C
 * TODO: 97.67% match - f27/f28 callee-saved register cascade from sideline
 * temp allocation, fabsf subtraction optimization, fcmpu operand order
 */
float SSearchBestPass::EvaluatePosition(const nlVector3& position, const nlVector3& v3OtherPosition, eFieldDirection eSearchDir, unsigned short aDirection)
{
    float fWeightedSum = 0.0f;
    float fTotalWeight = 0.0f;

    float fOpenLane = m_SSearchOpenLane.EvaluatePosition(position, v3OtherPosition, eSearchDir, aDirection);
    fWeightedSum += 0.7f * fOpenLane;
    fTotalWeight += 0.7f;

    float fSidelineDist = fabsf(fabsf(position.f.y) - cField::GetSidelineY(1));
    float fSidelineNorm = NormalizeVal(fSidelineDist, 1.0f, 5.0f);
    float fSidelineInv = 1.0f - fSidelineNorm;
    fWeightedSum += 0.2f * (fSidelineNorm * fSidelineInv);
    fTotalWeight += 0.2f * fSidelineInv;

    float fInOffensiveZone = InOffensiveZone(position, m_fNetDirection < 0.0f ? (eTeamSide)0 : (eTeamSide)1);

    float fIdealShot = m_SSearchIdealShot.EvaluatePosition(position, v3OtherPosition, eSearchDir, aDirection);
    fWeightedSum += fInOffensiveZone * (0.15f * fIdealShot);
    fTotalWeight += 0.15f * fInOffensiveZone;

    nlVector2 v2GoalLine = { 0.0f, 0.0f };
    v2GoalLine.f.x = cField::GetGoalLineX(-m_fNetDirection);

    float fIdealPositionScore = CalcIdealShootingPositionScore(position, v3OtherPosition, v2GoalLine, m_unk_0x0C);
    float fIdealPositionWeight = InterpolateRangeClamped(1.0f, 1.5f, 0.0f, 1.0f, 1.0f - fInOffensiveZone);
    fWeightedSum += 0.2f * (fIdealPositionScore * fIdealPositionWeight);
    fTotalWeight += 0.2f * fIdealPositionWeight;

    if (m_SSearchOpenLane.m_pPassTarget != NULL)
    {
        nlVector3 v3PassTargetVelocity = m_SSearchOpenLane.m_pPassTarget->m_v3Velocity;
        float fVelY = v3PassTargetVelocity.f.y;
        float fVelZ = v3PassTargetVelocity.f.z;
        float fVxSq = v3PassTargetVelocity.f.x * v3PassTargetVelocity.f.x;
        float fVySq = fVelY * fVelY;
        float fVzSq = fVelZ * fVelZ;
        float fPassTargetVelocitySq = fVxSq + fVySq + fVzSq;

        if (fPassTargetVelocitySq >= 12.25f)
        {
            float fInvPassTargetVelocity = nlRecipSqrt(fPassTargetVelocitySq, true);

            float fDy = position.f.y - v3OtherPosition.f.y;
            v3PassTargetVelocity.f.y = fInvPassTargetVelocity * fVelY;
            v3PassTargetVelocity.f.z = fInvPassTargetVelocity * fVelZ;
            v3PassTargetVelocity.f.x *= fInvPassTargetVelocity;

            float fDx = position.f.x - v3OtherPosition.f.x;
            float fDz = position.f.z - v3OtherPosition.f.z;
            float fDistSq = fDx * fDx + fDy * fDy + fDz * fDz;

            if (fDistSq > 0.2f)
            {
                float fInvDist = nlRecipSqrt(fDistSq, true);
                float fDot = (fInvDist * fDy) * v3PassTargetVelocity.f.y
                           + (fInvDist * fDx) * v3PassTargetVelocity.f.x
                           + (fInvDist * fDz) * v3PassTargetVelocity.f.z;

                fWeightedSum += 0.15f * (0.0f >= fDot ? 0.0f : fDot);
                fTotalWeight += 0.15f;
            }
        }

        if (m_SSearchOpenLane.m_pBallOwner != NULL)
        {
            float fDy2 = position.f.y - m_SSearchOpenLane.m_pBallOwner->m_v3Position.f.y;
            float fDx2 = position.f.x - m_SSearchOpenLane.m_pBallOwner->m_v3Position.f.x;
            float fDistToOwner = nlSqrt(fDx2 * fDx2 + fDy2 * fDy2, true);

            if (m_bIsPerfectPass)
            {
                float fPerfectPassDist = NormalizeVal(fDistToOwner, g_pGame->m_pFuzzyTweaks->vFarTeammateConfidenceDistance);
                fWeightedSum += 0.5f * fPerfectPassDist;
                fTotalWeight += 0.5f;
            }
            else
            {
                float fNearPassDist = NormalizeVal(fDistToOwner, g_pGame->m_pFuzzyTweaks->vNearTeammateConfidenceDistance);
                fWeightedSum += 0.15f * (1.0f - fNearPassDist);
                fTotalWeight += 0.15f;
            }

            float fPassSpeed = m_bAllowLeadPass ? m_SSearchOpenLane.m_pBallOwner->m_pTweaks->fPassVolleySpeedMax : m_SSearchOpenLane.m_pBallOwner->m_pTweaks->fPassGroundSpeedMax;

            float fDy3 = m_SSearchOpenLane.m_pBallOwner->m_v3Position.f.y - position.f.y;
            float fDx3 = m_SSearchOpenLane.m_pBallOwner->m_v3Position.f.x - position.f.x;
            float fOwnerDistNorm = nlSqrt(fDx3 * fDx3 + fDy3 * fDy3, true) / fPassSpeed;

            float fDy4 = m_SSearchOpenLane.m_pPassTarget->m_v3Position.f.y - position.f.y;
            float fDx4 = m_SSearchOpenLane.m_pPassTarget->m_v3Position.f.x - position.f.x;
            float fDistToTarget = nlSqrt(fDx4 * fDx4 + fDy4 * fDy4, true);

            float fNearZero = (float)(fabsf(fOwnerDistNorm - 0.0f) <= 0.0001f);
            if (fNearZero == 0.0f)
            {
                float fLeadPass = NormalizeVal(fDistToTarget / fOwnerDistNorm, 0.0f, m_SSearchOpenLane.m_pPassTarget->m_pTweaks->fJoggingSpeed);
                fLeadPass *= (float)m_bIsPerfectPass;

                fWeightedSum += fLeadPass != 0.0f ? 0.2f : 0.1f;
                fTotalWeight += m_bIsPerfectPass ? 0.2f : 0.1f;
            }
        }
    }

    if (fTotalWeight > 0.0f)
    {
        return fWeightedSum / fTotalWeight;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x538 | 0x80062E88 | size: 0x5C
 */
SSearchOpenLane::~SSearchOpenLane()
{
}

/**
 * Offset/Address/Size: 0x4B0 | 0x80062E00 | size: 0x88
 */
SSearchIdealShot::~SSearchIdealShot()
{
}

/**
 * Offset/Address/Size: 0x380 | 0x80062CD0 | size: 0x130
 */
SSearchRunToNet::SSearchRunToNet(cPlayer* pPlayer)
    : SpaceSearch(pPlayer->m_pTeam->m_pNet->m_sideSign)
    , m_SSearchIdealShot(pPlayer)
{
}

/**
 * Offset/Address/Size: 0x194 | 0x80062AE4 | size: 0x1EC
 */
float SSearchRunToNet::EvaluatePosition(const nlVector3& v3TestPosition, const nlVector3& v3CenterPos, eFieldDirection eSearchDir, unsigned short aDirection)
{
    float fTotalSum = 0.0f;
    float fTotalWeight = 0.0f;
    cFielder* pBallOwner = (cFielder*)m_SSearchIdealShot.m_SSearchOpenLane.m_pBallOwner;

    nlVector3 v3NetPosition = pBallOwner->GetAIOffNetLocation(NULL);
    float fNetX = v3NetPosition.f.x;
    float fNetY = v3NetPosition.f.y;

    float fDx = pBallOwner->m_v3Position.f.x - fNetX;
    float fDy = pBallOwner->m_v3Position.f.y - fNetY;
    if (fDx * fDx + fDy * fDy < 2500.0f)
    {
        fTotalSum += m_SSearchIdealShot.EvaluatePosition(v3TestPosition, v3CenterPos, eSearchDir, aDirection);
        fTotalWeight += 1.0f;
    }
    else
    {
        float fDxNet = fNetX - v3TestPosition.f.x;
        float fDyNet = fNetY - v3TestPosition.f.y;
        float fDistToNet = nlSqrt(fDxNet * fDxNet + fDyNet * fDyNet, true);
        float fNormDist = NormalizeVal(fDistToNet, g_vRunToNetDistanceConfidence);
        fTotalSum += 0.5f * fNormDist;
        fTotalWeight += 0.5f;

        float fOpenPos = OpenPosition(v3TestPosition, pBallOwner->m_pTeam->GetOtherTeam(), NULL, NULL);
        fTotalSum += 0.3f * fOpenPos;
        fTotalWeight += 0.3f;

        OpenToPosition(v3TestPosition, v3NetPosition, pBallOwner->m_pTeam->GetOtherTeam(), pBallOwner, NULL, true);

        nlVector3 v3FormationPos;
        pBallOwner->GetFormationPosition(v3FormationPos, 0.5f);
        float fFormDx = v3FormationPos.f.x - v3TestPosition.f.x;
        float fFormDy = v3FormationPos.f.y - v3TestPosition.f.y;
        float fFormDist = nlSqrt(fFormDx * fFormDx + fFormDy * fFormDy, true);
        float fNormFormDist = NormalizeVal(fFormDist, g_vRunToNetFormationDistConfidence);
        fTotalSum += 0.4f * fNormFormDist;
        fTotalWeight += 0.4f;

        float fCloseToSideline = CloseToSideline(v3TestPosition, NULL, false);
        fTotalSum += 0.2f * (1.0f - fCloseToSideline);
        fTotalWeight += 0.2f;
    }

    if (fTotalWeight > 0.0f)
    {
        return fTotalSum / fTotalWeight;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x158 | 0x80062AA8 | size: 0x3C
 */
SSearchCutAndBreak::SSearchCutAndBreak(cPlayer* pPlayer)
    : SpaceSearch(pPlayer->m_pTeam->m_pNet->m_sideSign)
{
    m_pPlayer = pPlayer;
}

/**
 * Offset/Address/Size: 0x5C | 0x800629AC | size: 0xFC
 */
float SSearchCutAndBreak::EvaluatePosition(const nlVector3& v3TestPosition, const nlVector3&, eFieldDirection, unsigned short)
{
    float fWeightedSum = 0.0f;
    float fTotalWeight = 0.0f;

    float fInFrontOfNet = PositionIsInFrontOfNet(v3TestPosition, m_pPlayer->m_pTeam->GetOtherTeam()->m_pNet);
    fWeightedSum += 0.5f * FGREATER(fInFrontOfNet, 0.5f) + 0.0f;
    fTotalWeight += 0.5f;

    float fWideOpen = WideOpenPosition(v3TestPosition, m_pPlayer->m_pTeam->GetOtherTeam(), m_pPlayer);
    fWeightedSum += 0.4f * fWideOpen;
    fTotalWeight += 0.4f;

    float fIdealDistance = PositionIsAtIdealDistanceForShooting(v3TestPosition, m_pPlayer->GetAIOffNetLocation(NULL));
    fWeightedSum += 0.3f * fIdealDistance;
    fTotalWeight += 0.3f;

    if (fTotalWeight > 0.0f)
    {
        return fWeightedSum / fTotalWeight;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x0 | 0x80062950 | size: 0x5C
 */
SSearchCutAndBreak::~SSearchCutAndBreak()
{
}
