#include "Game/AI/AvoidController.h"

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };
static const nlVector2 v2Zero = { 0.0f, 0.0f };

/**
 * Offset/Address/Size: 0x22E8 | 0x8000993C | size: 0x98
 */
AvoidController::AvoidController(cFielder* fielder)
{
    m_pFielder = fielder;
    m_pFTweaks = (const FielderTweaks*)fielder->m_pTweaks;
    m_ThingsToAvoid = 0;
    m_CurrentlyAvoiding = 0;
    m_VeryCloseToSideline = false;
    m_UseMinimumAvoidance = false;
    m_pIgnoreThisPlayer = 0;

    m_SidelineNormal.f.x = 0.0f;
    m_SidelineNormal.f.y = 0.0f;
    m_SidelineDirection.f.x = 0.0f;
    m_SidelineDirection.f.y = 0.0f;

    m_LastRepulsionVector[0] = v3Zero;
    m_LastRepulsionVector[1] = v3Zero;
    m_LastRepulsionVector[2] = v3Zero;
    m_LastRepulsionVector[3] = v3Zero;
    m_LastRepulsionVector[4] = v3Zero;
    m_LastRepulsionVector[5] = v3Zero;
}

/**
 * Offset/Address/Size: 0x2250 | 0x800098A4 | size: 0x98
 */
void AvoidController::SetThingsToAvoid(int thingsToAvoid)
{
    m_ThingsToAvoid = thingsToAvoid;
    if (m_ThingsToAvoid != 0)
    {
        return;
    }

    m_CurrentlyAvoiding = 0;
    m_VeryCloseToSideline = false;
    m_SidelineUnavoidable = false;

    m_SidelineNormal = v2Zero;
    m_SidelineDirection = v2Zero;

    m_LastRepulsionVector[0] = v3Zero;
    m_LastRepulsionVector[1] = v3Zero;
    m_LastRepulsionVector[2] = v3Zero;
    m_LastRepulsionVector[3] = v3Zero;
    m_LastRepulsionVector[4] = v3Zero;
    m_LastRepulsionVector[5] = v3Zero;
}

/**
 * Offset/Address/Size: 0x2240 | 0x80009894 | size: 0x10
 */
void AvoidController::UseMinimumAvoidance(cPlayer* player)
{
    m_UseMinimumAvoidance = true;
    m_pIgnoreThisPlayer = player;
}

static int AvoidableEnumToIndex(eAvoidableThings avoidable)
{
    if (avoidable == AVOID_EVERYTHING)
    {
        return 5;
    }
    for (int i = 0; i < NUM_AVOIDABLES; i++)
    {
        if ((1 << i) & (int)avoidable)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Offset/Address/Size: 0x2188 | 0x800097DC | size: 0xB8
 */
nlVector3& AvoidController::GetLastRepulsionVector(eAvoidableThings things)
{
    return m_LastRepulsionVector[AvoidableEnumToIndex(things)];
}
/**
 * Offset/Address/Size: 0x12BC | 0x80008910 | size: 0xECC
 */
void AvoidController::Update(float)
{
}

/**
 * Offset/Address/Size: 0x1060 | 0x800086B4 | size: 0x25C
 * TODO: 96.62% match - remaining diffs are x/z delta load order,
 * GetClosingSpeed2D setup register allocation, and fContribution/output FPR allocation.
 */
bool AvoidController::CalcFielderRepulsionVector(nlVector3& v3OutRepulsion)
{
    extern cTeam* g_pTeams[];
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern float GetClosingSpeed2D(const nlVector3&, const nlVector3&, const nlVector3&, const nlVector3&);
    extern float NormalizeVal(float, float, float);

    bool bAvoidedSomething = false;
    float fDeltaX, fDeltaY, fDeltaZ;
    float fDistance, fMagnitude, fClosingSpeed;

    v3OutRepulsion.f.x = 0.0f;
    v3OutRepulsion.f.y = 0.0f;
    v3OutRepulsion.f.z = 0.0f;

    for (int i_team = 0; i_team < 2; i_team++)
    {
        for (int i_fielder = 0; i_fielder < 4; i_fielder++)
        {
            cFielder* pFielder = g_pTeams[i_team]->GetFielder(i_fielder);
            if (pFielder == m_pFielder)
            {
                continue;
            }

            fDeltaZ = m_pFielder->m_v3Position.f.y - pFielder->m_v3Position.f.y;
            fDeltaY = m_pFielder->m_v3Position.f.x - pFielder->m_v3Position.f.x;
            fDeltaX = m_pFielder->m_v3Position.f.z - pFielder->m_v3Position.f.z;

            float fDistanceSq = fDeltaZ * fDeltaZ;
            fDistanceSq += fDeltaX * fDeltaX;
            fDistanceSq += fDeltaY * fDeltaY;
            if (fDistanceSq > 25.0f)
            {
                continue;
            }

            fDistance = nlSqrt(fDistanceSq, true);

            float fInvDistance = 1.0f / fDistance;
            fDeltaZ = fInvDistance * fDeltaZ;
            fDeltaY = fInvDistance * fDeltaY;
            fDeltaX = fInvDistance * fDeltaX;

            fDistance -= pFielder->m_pTweaks->fPhysCapsuleRadius + m_pFTweaks->fPhysCapsuleRadius;
            fClosingSpeed = GetClosingSpeed2D(m_pFielder->m_v3Position, m_pFielder->m_v3Velocity, pFielder->m_v3Position, pFielder->m_v3Velocity);

            fMagnitude = 10.0f * NormalizeVal(fDistance, 3.0f, 1.0f);
            fMagnitude += 3.0f * NormalizeVal(fClosingSpeed, 0.0f, 3.0f);

            if (m_UseMinimumAvoidance)
            {
                fMagnitude *= 2.0f;
            }

            m_pFielder->IsOnSameTeam(pFielder);
            if (m_pFielder->m_pMark == pFielder)
            {
                fMagnitude *= 0.5f;
            }

            if (pFielder == m_pIgnoreThisPlayer)
            {
                fMagnitude *= 2.0f;
            }

            if (m_pFielder->m_pBall != NULL)
            {
                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                fMagnitude *= 2.0f * pSkillTweaks->Off_Avoidance;
            }

            if (fMagnitude <= 0.0f)
            {
                continue;
            }

            float fContribution = 10.0f;
            if (fMagnitude <= fContribution)
            {
                fContribution = fMagnitude;
            }

            float fOutY = v3OutRepulsion.f.y;
            float fOutX = v3OutRepulsion.f.x;
            fOutY = fContribution * fDeltaZ + fOutY;
            float fOutZ = v3OutRepulsion.f.z;
            fOutX = fContribution * fDeltaY + fOutX;
            fOutZ = fContribution * fDeltaX + fOutZ;
            v3OutRepulsion.f.x = fOutX;
            v3OutRepulsion.f.y = fOutY;
            v3OutRepulsion.f.z = fOutZ;
            bAvoidedSomething = true;
        }
    }

    return bAvoidedSomething;
}

/**
 * Offset/Address/Size: 0xE08 | 0x8000845C | size: 0x258
 * TODO: 98.97% match - remaining register-only diffs are in delta load FPR assignment,
 * r7 vs r4 setup for GetClosingSpeed2D(m_pFielder->...), and fContribution/output FPR allocation.
 */
bool AvoidController::CalcPowerupRepulsionVector(nlVector3& v3OutRepulsion)
{
    extern PowerupBase* g_pPowerups[];
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern float GetClosingSpeed2D(const nlVector3&, const nlVector3&, const nlVector3&, const nlVector3&);
    extern float NormalizeVal(float, float, float);

    bool bAvoidedSomething = false;
    float fDeltaX, fDeltaY, fDeltaZ;
    float fDistance;
    float fMagnitude;
    float fClosingSpeed;

    v3OutRepulsion.f.x = 0.0f;
    v3OutRepulsion.f.y = 0.0f;
    v3OutRepulsion.f.z = 0.0f;

    if (m_pFielder->IsInvincible())
    {
        return false;
    }

    for (int i = 0; i < 25; i++)
    {
        PowerupBase* pPowerup = g_pPowerups[i];

        if (pPowerup != NULL)
        {
            fDeltaX = m_pFielder->m_v3Position.f.x - pPowerup->m_v3Position.f.x;
            fDeltaY = m_pFielder->m_v3Position.f.y - pPowerup->m_v3Position.f.y;
            fDeltaZ = m_pFielder->m_v3Position.f.z - pPowerup->m_v3Position.f.z;
            float fDistanceSq = fDeltaY * fDeltaY;
            fDistanceSq += fDeltaX * fDeltaX;
            fDistanceSq += fDeltaZ * fDeltaZ;

            if (fDistanceSq > 25.0f)
            {
                continue;
            }

            fDistance = nlSqrt(fDistanceSq, true);

            float fInvDistance = 1.0f / fDistance;
            fDeltaZ = fInvDistance * fDeltaZ;
            fDeltaY = fInvDistance * fDeltaY;
            fDeltaX = fInvDistance * fDeltaX;

            fDistance -= m_pFTweaks->fPhysCapsuleRadius + pPowerup->GetRadius();

            fClosingSpeed = GetClosingSpeed2D(m_pFielder->m_v3Position, m_pFielder->m_v3Velocity, pPowerup->m_v3Position, pPowerup->m_v3Velocity);

            float fPowerupSpeedSq = pPowerup->m_v3Velocity.f.x * pPowerup->m_v3Velocity.f.x
                                  + pPowerup->m_v3Velocity.f.y * pPowerup->m_v3Velocity.f.y
                                  + pPowerup->m_v3Velocity.f.z * pPowerup->m_v3Velocity.f.z;

            if (fPowerupSpeedSq <= 2.0f)
            {
                fMagnitude = 10.0f * NormalizeVal(fDistance, 3.0f, 1.0f);
            }
            else
            {
                fMagnitude = 10.0f * NormalizeVal(fDistance, 5.0f, 2.0f);
                fMagnitude += 15.0f * NormalizeVal(fClosingSpeed, 0.0f, 4.0f);

                if (m_pFielder->m_pBall != NULL)
                {
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    fMagnitude *= 2.0f * pSkillTweaks->Off_Avoidance;
                }
            }

            if (fMagnitude <= 0.0f)
            {
                continue;
            }

            float fContribution = 30.0f;
            if (fMagnitude <= fContribution)
            {
                fContribution = fMagnitude;
            }

            float fOutY = v3OutRepulsion.f.y;
            float fOutX = v3OutRepulsion.f.x;
            fOutY = fContribution * fDeltaY + fOutY;
            float fOutZ = v3OutRepulsion.f.z;
            fOutX = fContribution * fDeltaX + fOutX;
            fOutZ = fContribution * fDeltaZ + fOutZ;
            v3OutRepulsion.f.x = fOutX;
            v3OutRepulsion.f.y = fOutY;
            v3OutRepulsion.f.z = fOutZ;
            bAvoidedSomething = true;
        }
    }

    return bAvoidedSomething;
}

/**
 * Offset/Address/Size: 0xC30 | 0x80008284 | size: 0x1D8
 */
bool AvoidController::CalcDesiredVelocityToAvoidSideline(
    nlVector2& vNewDesiredVelDir,
    const nlVector2& vCurrentDesiredVelDir,
    const nlVector2& vCurrentVelDir,
    const nlVector2& vSidelinePos,
    const nlVector2& vSidelineNormal)
{
    bool bHitSideline = false;

    float fDotNormalVel = vSidelineNormal.f.x * vCurrentDesiredVelDir.f.x + vSidelineNormal.f.y * vCurrentDesiredVelDir.f.y;

    nlVector2* pPos;
    if (m_pFielder->m_pBall != NULL)
    {
        pPos = (nlVector2*)&m_pFielder->m_pBall->m_v3Position;
    }
    else
    {
        pPos = (nlVector2*)&m_pFielder->m_v3Position;
    }

    float dy = pPos->f.y - vSidelinePos.f.y;
    float dx = pPos->f.x - vSidelinePos.f.x;
    float fDistance = nlSqrt(dx * dx + dy * dy, true);

    float fMaxDistance = 5.0f;
    fDistance -= m_pFTweaks->fPhysCapsuleRadius;
    if (m_CurrentlyAvoiding & AVOID_SIDELINES)
    {
        fMaxDistance = 8.0f;
    }

    float fMinDistance = 0.5f;
    m_SidelineUnavoidable = false;
    m_VeryCloseToSideline = false;

    if (fDistance <= fMinDistance)
    {
        m_VeryCloseToSideline = true;
        m_SidelineNormal = vSidelineNormal;
        m_SidelineDirection = vNewDesiredVelDir;
    }

    if (fDistance <= fMaxDistance)
    {
        if (fDotNormalVel <= 0.0f)
        {
            if (fDotNormalVel > -0.9f)
            {
                float fCos, fSin;
                nlSinCos(&fSin, &fCos, 0x4000);

                nlVector2 vParallelVelDir;
                vParallelVelDir.f.x = vSidelineNormal.f.x * fCos - vSidelineNormal.f.y * fSin;
                vParallelVelDir.f.y = vSidelineNormal.f.y * fCos + vSidelineNormal.f.x * fSin;

                if (vParallelVelDir.f.x * vCurrentDesiredVelDir.f.x + vParallelVelDir.f.y * vCurrentDesiredVelDir.f.y < 0.0f)
                {
                    vParallelVelDir.f.y = v3Zero.f.y - vParallelVelDir.f.y;
                    vParallelVelDir.f.x = v3Zero.f.x - vParallelVelDir.f.x;
                }

                vNewDesiredVelDir = vParallelVelDir;
            }
            else
            {
                vNewDesiredVelDir = v2Zero;
                m_SidelineUnavoidable = true;
            }
            bHitSideline = true;
        }
    }

    return bHitSideline;
}

/**
 * Offset/Address/Size: 0x8C8 | 0x80007F1C | size: 0x368
 */
/**
 * TODO: 98.30% match - extsh. r4,r0 vs r0,r0 register allocation in first abs block
 */
bool AvoidController::CalcDesiredVelocityToAvoidCorner(
    nlVector2& vNewDesiredVelDir,
    const sCornerSegment& corner,
    const nlVector2& vCurrentDesiredVelDir,
    const nlVector2& vCurrentVelDir)
{
    bool bHitSideline = false;
    nlVector2 vSidelinePos;
    nlVector2 vSidelineNormal;
    s16 startDiff;
    int absStartI;

    nlVector2 vPosition = *(nlVector2*)&m_pFielder->m_v3Position;
    nlVector2 vBallPosition;

    if (m_pFielder->m_pBall != NULL)
    {
        vBallPosition = *(nlVector2*)&m_pFielder->m_pBall->m_v3Position;
    }
    else
    {
        vBallPosition = vPosition;
    }

    float fDistX;
    float fDistY;
    fDistY = corner.vCenter.f.y - vBallPosition.f.y;
    fDistX = corner.vCenter.f.x - vBallPosition.f.x;
    if (nlSqrt(fDistX * fDistX + fDistY * fDistY, true) <= corner.fRadius)
    {
        float fDeltaX;
        float fDeltaY;
        fDeltaY = vBallPosition.f.y - corner.vCenter.f.y;
        fDeltaX = vBallPosition.f.x - corner.vCenter.f.x;

        f32 fAngle = 10430.378f * nlATan2f(fDeltaY, fDeltaX);
        u16 aCornerToPos = (u16)(s32)fAngle;

        startDiff = (s16)(aCornerToPos - corner.thetaStart);
        absStartI = startDiff;
        if (startDiff < 0)
            absStartI = -startDiff;
        u16 absStart = (u16)absStartI;

        s16 endDiff = (s16)(aCornerToPos - corner.thetaEnd);
        int absEndI = endDiff;
        if (endDiff < 0)
            absEndI = -endDiff;
        u16 absEnd = (u16)absEndI;

        if (absStart >= absEnd)
            absEnd = absStart;
        if ((s16)absEnd <= 0x4000)
        {
            float fFielderX;
            float fFielderY;
            fFielderY = vPosition.f.y - corner.vCenter.f.y;
            fFielderX = vPosition.f.x - corner.vCenter.f.x;

            f32 fAngle2 = 10430.378f * nlATan2f(fFielderY, fFielderX);
            u16 aCornerToFielder = (u16)(s32)fAngle2;

            startDiff = (s16)(aCornerToFielder - corner.thetaStart);
            absStartI = startDiff;
            if (startDiff < 0)
                absStartI = -startDiff;
            u16 absStart2 = (u16)absStartI;

            s16 endDiff2 = (s16)(aCornerToFielder - corner.thetaEnd);
            int absEnd2I = endDiff2;
            if (endDiff2 < 0)
                absEnd2I = -endDiff2;
            u16 absEnd2 = (u16)absEnd2I;

            if (absStart2 >= absEnd2)
                absEnd2 = absStart2;
            if ((s16)absEnd2 <= 0x4000)
            {
                float fInvDistance = nlRecipSqrt(fFielderX * fFielderX + fFielderY * fFielderY, true);
                float nx;
                float ny;
                ny = fInvDistance * fFielderY;
                nx = fInvDistance * fFielderX;
                vSidelineNormal.f.y = v2Zero.f.y - ny;
                vSidelineNormal.f.x = v2Zero.f.x - nx;
                vSidelinePos.f.y = corner.fRadius * ny + corner.vCenter.f.y;
                vSidelinePos.f.x = corner.fRadius * nx + corner.vCenter.f.x;
                bHitSideline = CalcDesiredVelocityToAvoidSideline(
                    vNewDesiredVelDir, vCurrentDesiredVelDir, vCurrentVelDir, vSidelinePos, vSidelineNormal);
            }
            else if (m_pFielder->IsTurboing())
            {
                float fInvDistance = nlRecipSqrt(fDeltaX * fDeltaX + fDeltaY * fDeltaY, true);
                float nx;
                float ny;
                ny = fInvDistance * fDeltaY;
                nx = fInvDistance * fDeltaX;
                vSidelinePos.f.y = corner.fRadius * ny + corner.vCenter.f.y;
                vSidelineNormal.f.y = v2Zero.f.y - ny;
                vSidelinePos.f.x = corner.fRadius * nx + corner.vCenter.f.x;
                float fCornerDistY = vBallPosition.f.y - vSidelinePos.f.y;
                vSidelineNormal.f.x = v2Zero.f.x - nx;
                float fDotNormalVel = vSidelineNormal.f.y * vCurrentDesiredVelDir.f.y;
                float fCornerDistX = vBallPosition.f.x - vSidelinePos.f.x;
                fDotNormalVel = vSidelineNormal.f.x * vCurrentDesiredVelDir.f.x + fDotNormalVel;
                float fDistance = nlSqrt(fCornerDistX * fCornerDistX + fCornerDistY * fCornerDistY, true);
                float fMaxDistance = 5.0f;
                fDistance -= m_pFTweaks->fPhysCapsuleRadius;
                if (m_CurrentlyAvoiding & AVOID_SIDELINES)
                    fMaxDistance = 8.0f;
                if (fDistance <= fMaxDistance)
                {
                    if (fDotNormalVel <= 0.0f)
                    {
                        vNewDesiredVelDir = v2Zero;
                        bHitSideline = true;
                    }
                }
            }
        }
    }
    return bHitSideline;
}

/**
 * Offset/Address/Size: 0x41C | 0x80007A70 | size: 0x4AC
 */
void AvoidController::AvoidSidelines()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x80007654 | size: 0x41C
 * TODO: 91.40% match - FPR f7 vs f5 for repulsionY cascading through function,
 * addic. r0 vs r4 CSE in v3Zero section, rotation fmadds/fmsubs ordering,
 * speed section register allocation.
 */
void AvoidController::ApplyRepulsionVector(nlVector3 v3Repulsion)
{
    struct LocalVectors
    {
        nlVector3 repulsionDir;
        nlVector3 desiredVelDir;
    } v;

    f32 fCos;
    f32 fSin;
    f32 fDesiredSpeed;

    if (m_pFielder->GetDistanceToDesiredPos() <= 0.5f)
    {
        return;
    }

    f32 fRepulsionMag = nlSqrt(
        v3Repulsion.f.x * v3Repulsion.f.x + v3Repulsion.f.y * v3Repulsion.f.y + v3Repulsion.f.z * v3Repulsion.f.z,
        true);
    if (fRepulsionMag <= 0.0f)
    {
        return;
    }

    v.desiredVelDir = v3Zero;
    if (&v.desiredVelDir != NULL)
    {
        nlSinCos(&v.desiredVelDir.f.y, &v.desiredVelDir.f.x, m_pFielder->m_aDesiredMovementDirection);
    }

    v.desiredVelDir.f.z = 0.0f;

    f32 fInvRepulsionMag = nlRecipSqrt(
        v3Repulsion.f.x * v3Repulsion.f.x + v3Repulsion.f.y * v3Repulsion.f.y + v3Repulsion.f.z * v3Repulsion.f.z,
        true);

    f32 fRepulsionY = fInvRepulsionMag * v3Repulsion.f.y;
    f32 fRepulsionX = fInvRepulsionMag * v3Repulsion.f.x;
    f32 fRepulsionZ = fInvRepulsionMag * v3Repulsion.f.z;

    v.repulsionDir.f.x = fRepulsionX;
    v.repulsionDir.f.y = fRepulsionY;
    v.repulsionDir.f.z = fRepulsionZ;

    f32 fDot = v.desiredVelDir.f.y * fRepulsionY;
    fDot = v.desiredVelDir.f.x * fRepulsionX + fDot;
    fDot = v.desiredVelDir.f.z * fRepulsionZ + fDot;

    if (fDot <= -0.75f || fDot >= 0.7f)
    {
        f32 fRepulsionAngle = nlATan2f(fRepulsionY, fRepulsionX);
        f32 fDesiredAngle = nlATan2f(v.desiredVelDir.f.y, v.desiredVelDir.f.x);

        u16 aDesired = (u16)(s32)(10430.378f * fDesiredAngle);
        u16 aRepulsion = (u16)(s32)(10430.378f * fRepulsionAngle);
        s16 aDelta = (s16)(aRepulsion - aDesired);

        s16 aAdjust;
        if ((f32)aDelta > 0.0f)
        {
            aAdjust = 0x4000;
        }
        else
        {
            aAdjust = -0x4000;
        }

        nlSinCos(&fSin, &fCos, (u16)aAdjust);

        f32 fDesX = v.desiredVelDir.f.x;
        f32 fDesY = v.desiredVelDir.f.y;
        f32 fParallelY = fDesX * fSin + fDesY * fCos;
        f32 fParallelX = fDesX * fCos - fDesY * fSin;

        v.repulsionDir.f.y = fParallelY;
        v.repulsionDir.f.x = fParallelX;

        v3Repulsion.f.x = fRepulsionMag * fParallelX;
        v3Repulsion.f.y = fRepulsionMag * fParallelY;
        v3Repulsion.f.z = fRepulsionMag * v.repulsionDir.f.z;
    }

    if (m_VeryCloseToSideline)
    {
        f32 fNormalY = m_SidelineNormal.f.y;
        f32 fNormalX = m_SidelineNormal.f.x;
        f32 fDotNormalVel = v.repulsionDir.f.y * fNormalY;
        fDotNormalVel = v.repulsionDir.f.x * fNormalX + fDotNormalVel;

        if (fDotNormalVel < -0.1f)
        {
            f32 fSidelineDirX = m_SidelineDirection.f.x;
            f32 fSidelineDirY = m_SidelineDirection.f.y;
            f32 fSidelineLenSq = fSidelineDirX * fSidelineDirX;
            fSidelineLenSq = fSidelineLenSq + fSidelineDirY * fSidelineDirY;

            if (fSidelineLenSq > 0.0f)
            {
                f32 fDotDirVel = v.repulsionDir.f.y * fSidelineDirY;
                f32 fRepulsionDirZ = v.repulsionDir.f.z;
                f32 fSidelineDirZ = 0.0f;
                f32 fMinusOne = -1.0f;

                fDotDirVel = v.repulsionDir.f.x * fSidelineDirX + fDotDirVel;
                v3Repulsion.f.z = fRepulsionMag * fSidelineDirZ;

                fSidelineLenSq = fSidelineDirX * fSidelineDirX + fSidelineDirY * fSidelineDirY + fSidelineDirZ * fSidelineDirZ;
                fDotDirVel = fRepulsionDirZ * fSidelineDirZ + fDotDirVel;

                f32 fScale = fDotDirVel / fSidelineLenSq;
                f32 fParallelY = fScale * fSidelineDirY;
                f32 fParallelX = fScale * fSidelineDirX;
                f32 fParallelZ = fScale * fSidelineDirZ;

                f32 fPerpY = v.repulsionDir.f.y - fParallelY;
                f32 fPerpX = v.repulsionDir.f.x - fParallelX;
                f32 fPerpZ = fRepulsionDirZ - fParallelZ;

                fParallelY = fMinusOne * fPerpY + fParallelY;
                fParallelX = fMinusOne * fPerpX + fParallelX;
                fParallelZ = fMinusOne * fPerpZ + fParallelZ;

                v.repulsionDir.f.y = fParallelY;
                v.repulsionDir.f.z = fParallelZ;
                v.repulsionDir.f.x = fParallelX;
                v.repulsionDir.f.z = fSidelineDirZ;

                v3Repulsion.f.x = fRepulsionMag * fParallelX;
                v3Repulsion.f.y = fRepulsionMag * fParallelY;
            }
            else
            {
                v3Repulsion.f.x = fRepulsionMag * fNormalX;
                v3Repulsion.f.y = fRepulsionMag * fNormalY;
            }
        }
    }

    f32 fCurrentDesiredSpeed = m_pFielder->m_fDesiredSpeed;
    f32 fResultantX = fCurrentDesiredSpeed * v.desiredVelDir.f.x + v3Repulsion.f.x;
    f32 fResultantY = fCurrentDesiredSpeed * v.desiredVelDir.f.y + v3Repulsion.f.y;
    f32 fResultantZ = fCurrentDesiredSpeed * v.desiredVelDir.f.z + v3Repulsion.f.z;

    f32 fResultantMag = nlSqrt(fResultantX * fResultantX + fResultantY * fResultantY + fResultantZ * fResultantZ, true);

    m_pFielder->m_aDesiredMovementDirection = (u16)(s32)(10430.378f * nlATan2f(fResultantY, fResultantX));
    if (m_pFielder->m_eMovementState != MOVEMENT_STRAFING)
    {
        m_pFielder->m_aDesiredFacingDirection = m_pFielder->m_aDesiredMovementDirection;
    }

    if (m_pFielder->m_pBall != NULL)
    {
        fDesiredSpeed = m_pFTweaks->fRunningWBSpeed;
    }
    else
    {
        f32 fRunningDiff = (float)fabs(fResultantMag - m_pFTweaks->fRunningSpeed);
        f32 fJoggingDiff = (float)fabs(fResultantMag - m_pFTweaks->fJoggingSpeed);
        if (fJoggingDiff < fRunningDiff)
        {
            fDesiredSpeed = m_pFTweaks->fJoggingSpeed;
        }
        else
        {
            fDesiredSpeed = m_pFTweaks->fRunningSpeed;
        }
    }

    f32 speed = m_pFielder->m_fDesiredSpeed;
    if (speed >= fDesiredSpeed)
    {
    }
    else
    {
        speed = fDesiredSpeed;
    }
    m_pFielder->m_fDesiredSpeed = speed;
}
