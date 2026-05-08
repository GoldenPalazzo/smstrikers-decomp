#include "Game/Goalie.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/Fielder.h"
#include "Game/Net.h"
#include "Game/Team.h"
#include "Game/Ball.h"
#include "Game/GameTweaks.h"
#include "Game/SAnim/pnSingleAxisBlender.h"
#include "Game/SAnim/pnBlender.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/CharacterTriggers.h"
#include "Game/AI/FilteredRandom.h"
#include "Game/Field.h"
#include "Game/Physics/PhysicsFakeBall.h"
#include "NL/nlMath.h"
#include "Game/MathHelpers.h"

static f32 CANT_COLLIDE = *(f32*)__float_max;

cTeam* g_pCurrentlyUpdatingTeam;
extern cBall* g_pBall;
extern FakeBallWorld* g_pFakeBallWorld;
f32 Goalie::mfGoalieStepDist;
f32 mfGoalieStrafeDist;
f32 mfGoalieRunDist;
f32 mfGoalieUrgentDist;
f32 gfRepositionThreshold;

static int gOffplayDejected[5] = { 0x90, 0x91, 0x92, 0x93, 0x94 };
static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

inline float CalculateDistanceSquared2D(const nlVector3& pos1, const nlVector3& pos2)
{
    nlVector3 delta;
    nlVec3Sub(delta, pos1, pos2);
    return delta.GetLengthSq2D();
}

/**
 * Offset/Address/Size: 0x4518 | 0x80052A54 | size: 0x1E8
 */
void Goalie::ActionLooseBallCatch(float deltaTime)
{
    mfTargetTime -= deltaTime;

    if (m_eAnimID == GOALIEACTION_STS)
    {
        float targetTime = mfTargetTime;
        if (targetTime <= mBlendInfo.mfMilestoneTime[2] + 0.01f)
        {
            float clampedValue = mBlendInfo.mfMilestoneTime[2] - targetTime;
            clampedValue = nlMaxEquals(clampedValue, mBlendInfo.mfStartTime);
            PlayBlendedAnims(clampedValue, -1);
        }
    }
    else
    {

        if (mpSaveData == NULL || m_pCurrentAnimController->m_fTime > 0.95f)
        {
            if (m_pBall == NULL)
            {
                InitActionMove(false);
                return;
            }
            InitActionMoveWB();
            return;
        }

        if (g_pBall->m_pOwner != NULL)
        {
            return;
        }

        if (!m_pCurrentAnimController->TestTrigger(mpSaveData->mfMilestonePercent[2]))
        {
            return;
        }

        const nlVector3& leftHandPos = GetJointPosition(m_nLeftHandJointIndex);
        const nlVector3& rightHandPos = GetJointPosition(m_nRightHandJointIndex);

        float distSqLeft = CalculateDistanceSquared(g_pBall->m_v3Position, leftHandPos);
        if (distSqLeft < 1.0f || CalculateDistanceSquared(g_pBall->m_v3Position, rightHandPos) < 1.0f)
        {
            Audio::SoundAttributes soundAttrs;
            soundAttrs.Init();
            soundAttrs.SetSoundType(0xC0, true);
            soundAttrs.UseStationaryPosVector(m_v3Position);
            soundAttrs.mf_Volume = 1.0f;
            Audio::gStadGenSFX.Play(soundAttrs);

            PickupBall(g_pBall);
            mbPickedUp = true;
            g_pBall->ClearShotInProgress();
            EmitGoalieCatch(this, "goalie_catch", false);
        }
    }
}

/**
 * Offset/Address/Size: 0x3E6C | 0x800523A8 | size: 0x6AC
 * TODO: 91.1% match - f0/f1 register swap at c8-cc for pickupTime/animTime
 *       load order causes cascading diffs in time computation and clamping
 */
void Goalie::ActionLooseBallDesperate(float fDeltaT)
{
    cBall* pBall = g_pBall;
    int animID = m_eAnimID;
    const LooseBallInfo* pInfo = mpLooseBallInfo;
    if (pInfo->mnAnimID == animID)
    {
        cPN_SAnimController* pAnim = m_pCurrentAnimController;
        bool bAnimDone = false;
        if (pAnim->m_ePlayMode == PM_HOLD && pAnim->m_fTime == 1.0f)
            bAnimDone = true;
        if (bAnimDone)
        {
            if (animID == 0x7C)
            {
                InitActionPursueRecover();
                return;
            }
            if (m_pBall == NULL)
            {
                InitActionMove(false);
                return;
            }
            InitActionMoveWB();
            return;
        }
        if (pBall->m_pOwner != NULL)
            goto handleOwner;
        if (pAnim->m_fTime < pInfo->mfPickupTime)
        {
            float fRatio = pAnim->m_fTime / pInfo->mfPickupTime;
            float fTimeRemaining = pInfo->mfPickupTime * pInfo->mfAnimDuration - pInfo->mfAnimDuration * pAnim->m_fTime;
            float fGoalLineX = cField::GetGoalLineX(1U);
            float fTimeScale = 0.25f * fTimeRemaining;
            float fLimit = fGoalLineX - 0.2f;
            nlVector3 v3GuessBallPos;
            v3GuessBallPos.f.x = fTimeScale * g_pBall->m_v3Velocity.f.x + pBall->m_v3Position.f.x;
            v3GuessBallPos.f.z = fTimeScale * g_pBall->m_v3Velocity.f.z + pBall->m_v3Position.f.z;
            v3GuessBallPos.f.y = fTimeScale * g_pBall->m_v3Velocity.f.y + pBall->m_v3Position.f.y;
            if ((float)fabs(v3GuessBallPos.f.x) > fLimit)
            {
                float fClampedX;
                if (v3GuessBallPos.f.x > 0.0f)
                    fClampedX = fLimit;
                else
                    fClampedX = -fLimit;
                if ((float)fabs(pBall->m_v3Position.f.x) < fLimit)
                {
                    float fDX = pBall->m_v3Position.f.x - fClampedX;
                    float fDY = pBall->m_v3Position.f.y - v3GuessBallPos.f.y;
                    float fDXOrig = pBall->m_v3Position.f.x - v3GuessBallPos.f.x;
                    float fNewY = fDX * fDY;
                    fNewY = fNewY / fDXOrig;
                    fNewY = pBall->m_v3Position.f.y - fNewY;
                    v3GuessBallPos.f.y = fNewY;
                }
                v3GuessBallPos.f.x = fClampedX;
            }
            TrackTarget(v3GuessBallPos, fRatio);
            const nlVector3& v3Head = GetJointPosition(m_nHeadJointIndex);
            nlVector3 v3HeadCopy;
            v3HeadCopy = v3Head;
            float fAbsX = (float)fabs(v3HeadCopy.f.x);
            float fDX2 = 0.0f;
            float fLimitH = cField::GetGoalLineX(1U) - 0.5f;
            float fNetY = 0.5f * cNet::m_fNetWidth;
            if (fAbsX > fLimitH)
            {
                if ((float)fabs(v3HeadCopy.f.y) > fNetY)
                    fDX2 = fAbsX - fLimitH;
            }
            const nlVector3& v3RHand = GetJointPosition(m_nRightHandJointIndex);
            nlVector3 v3RHandCopy;
            v3RHandCopy = v3RHand;
            fAbsX = (float)fabs(v3RHandCopy.f.x);
            float fLimitR = cField::GetGoalLineX(1U) - 0.4f;
            if (fAbsX > fLimitR)
            {
                if ((float)fabs(v3RHandCopy.f.y) > fNetY)
                {
                    float fDiff = fAbsX - fLimitR;
                    if (fDiff > fDX2)
                        fDX2 = fDiff;
                }
            }
            const nlVector3& v3LHand = GetJointPosition(m_nLeftHandJointIndex);
            nlVector3 v3LHandCopy;
            v3LHandCopy = v3LHand;
            float fAbsX3 = (float)fabs(v3LHandCopy.f.x);
            if (fAbsX3 > fLimitR)
            {
                if ((float)fabs(v3LHandCopy.f.y) > fNetY)
                {
                    float fDiff = fAbsX3 - fLimitR;
                    if (fDiff > fDX2)
                        fDX2 = fDiff;
                }
            }
            if (fDX2 > 0.0f)
            {
                nlVector3 v3AdjPos;
                v3AdjPos = m_v3Position;
                if (v3AdjPos.f.x > 0.0f)
                    fDX2 *= -1.0f;
                v3AdjPos.f.x = v3AdjPos.f.x + fDX2;
                SetPosition(v3AdjPos);
            }
            return;
        }
        else
        {
            const nlVector3& v3BallJoint = GetJointPosition(m_nBallJointIndex);
            float dY = pBall->m_v3Position.f.y - v3BallJoint.f.y;
            float dX = pBall->m_v3Position.f.x - v3BallJoint.f.x;
            float dZ = pBall->m_v3Position.f.z - v3BallJoint.f.z;
            float distSq = dY * dY;
            distSq = dX * dX + distSq;
            distSq = dZ * dZ + distSq;
            if (distSq < 0.36f)
            {
                InitiatePanicGrab(NULL);
            }
            return;
        }
    handleOwner:
        if (m_pBall != NULL)
            return;
        SetGoalieAction(GOALIEACTION_PURSUE_BALL_POUNCE, 0);
    }
    else
    {
        if (muBallChangeCount != pBall->m_bBallPathChangeCount || mnOffplayPending != 0 || pBall->m_pOwner != NULL)
        {
            InitActionMove(false);
            return;
        }
        mfTargetTime = mfTargetTime - fDeltaT;
        DoNavigation(fDeltaT, 0.0f, NAVI_FOLLOW_TARGET);
        const nlVector3& v3Head = GetJointPosition(m_nHeadJointIndex);
        nlVector3 v3HeadCopy;
        v3HeadCopy = v3Head;
        float fAbsX = (float)fabs(v3HeadCopy.f.x);
        float fDX = 0.0f;
        float fLimitH = cField::GetGoalLineX(1U) - 0.5f;
        float fNetY = 0.5f * cNet::m_fNetWidth;
        if (fAbsX > fLimitH)
        {
            if ((float)fabs(v3HeadCopy.f.y) > fNetY)
                fDX = fAbsX - fLimitH;
        }
        const nlVector3& v3RHand = GetJointPosition(m_nRightHandJointIndex);
        nlVector3 v3RHandCopy;
        v3RHandCopy = v3RHand;
        fAbsX = (float)fabs(v3RHandCopy.f.x);
        float fLimitR = cField::GetGoalLineX(1U) - 0.4f;
        if (fAbsX > fLimitR)
        {
            if ((float)fabs(v3RHandCopy.f.y) > fNetY)
            {
                float fDiff = fAbsX - fLimitR;
                if (fDiff > fDX)
                    fDX = fDiff;
            }
        }
        const nlVector3& v3LHand = GetJointPosition(m_nLeftHandJointIndex);
        nlVector3 v3LHandCopy;
        v3LHandCopy = v3LHand;
        float fAbsX3 = (float)fabs(v3LHandCopy.f.x);
        if (fAbsX3 > fLimitR)
        {
            if ((float)fabs(v3LHandCopy.f.y) > fNetY)
            {
                float fDiff = fAbsX3 - fLimitR;
                if (fDiff > fDX)
                    fDX = fDiff;
            }
        }
        if (fDX > 0.0f)
        {
            nlVector3 v3AdjPos;
            v3AdjPos = m_v3Position;
            if (v3AdjPos.f.x > 0.0f)
                fDX *= -1.0f;
            v3AdjPos.f.x = v3AdjPos.f.x + fDX;
            SetPosition(v3AdjPos);
        }
        const LooseBallInfo* pInfoE = mpLooseBallInfo;
        cBall* pBallE = g_pBall;
        float fCatchRadius = 0.6f + pInfoE->mfPickupDistance;
        float fPickupTimeE = pInfoE->mfPickupTime;
        float fAnimDurE = pInfoE->mfAnimDuration;
        float fCatchRadSq = fCatchRadius * fCatchRadius;
        float fTimeProduct = fPickupTimeE * fAnimDurE;
        float fGuessY = fTimeProduct * pBallE->m_v3Velocity.f.y + pBall->m_v3Position.f.y;
        float fGuessX = fTimeProduct * pBallE->m_v3Velocity.f.x + pBall->m_v3Position.f.x;
        if (mfTargetTime < 0.02f)
            goto doPlayNewAnim;
        {
            float fAbsBallX = (float)fabs(pBall->m_v3Position.f.x);
            float fGoalLine2 = cField::GetGoalLineX(1U) - 1.0f;
            if (fAbsBallX > fGoalLine2)
                goto doPlayNewAnim;
        }
        {
            float dYc = m_v3Position.f.y - pBall->m_v3Position.f.y;
            float dXc = m_v3Position.f.x - pBall->m_v3Position.f.x;
            float distSqC = dYc * dYc;
            distSqC = dXc * dXc + distSqC;
            if (distSqC < fCatchRadSq)
                goto doPlayNewAnim;
        }
        {
            float dYg = m_v3Position.f.y - fGuessY;
            float dXg = m_v3Position.f.x - fGuessX;
            float distSqG = dYg * dYg;
            distSqG = dXg * dXg + distSqG;
            if (distSqG < fCatchRadSq)
                goto doPlayNewAnim;
        }
        return;
    doPlayNewAnim:
        PlayNewAnim(mpLooseBallInfo->mnAnimID);
        InitMovementFromAnim(0, v3Zero, 1.0f, false);
    }
}

/**
 * Offset/Address/Size: 0x36CC | 0x80051C08 | size: 0x7A0
 * TODO: 99.90% match - 9 register diffs: f0/f2 swap at mfWaitTime check, f30/f28 for fSmooth, f29/f28 for fAbsX
 */
void Goalie::ActionLooseBallPickup(float fDeltaT)
{
    float fTimeLeft = m_pCurrentAnimController->m_fTime;

    if (fTimeLeft > 0.97f)
    {
        if (g_pBall->m_pOwner != this && mfWaitTime > 0.0f && mpLooseBallInfo->mAnimType != LOOSEBALL_ANIM_KICK)
        {
            m_tNoPickupTimer.SetSeconds(fDeltaT);
        }
        else
        {
            if (m_eAnimID == 0x7C)
            {
                InitActionPursueRecover();
                return;
            }

            if (m_pBall == NULL)
            {
                InitActionMove(false);
                return;
            }

            InitActionMoveWB();
            return;
        }
    }

    if (mpLooseBallInfo->mAnimType == LOOSEBALL_ANIM_KICK)
    {
        if ((m_eAnimID == 5 || m_eAnimID == 4)
            && fTimeLeft > (0.1f + mpLooseBallInfo->mfPickupTime)
            && m_pBall == NULL)
        {
            InitActionMove(false);
            return;
        }

        if (mpPassTarget != NULL)
        {
            float fAngle = nlATan2f(
                mpPassTarget->m_v3Position.f.y - m_v3Position.f.y,
                mpPassTarget->m_v3Position.f.x - m_v3Position.f.x);
            m_aDesiredFacingDirection = (u16)(s32)(10430.378f * fAngle);
        }
        else
        {
            unsigned short dir;
            if (m_v3Position.f.x > 0.0f)
            {
                dir = 0x8000;
            }
            else
            {
                dir = 0;
            }
            m_aDesiredFacingDirection = dir;
        }

        float fSeekSpeed;
        float fSeekFalloff;
        if (mpLooseBallInfo->mnAnimID == 3)
        {
            fSeekSpeed = 150000.0f;
            fSeekFalloff = 2000.0f;
        }
        else
        {
            fSeekSpeed = 30000.0f;
            fSeekFalloff = 3000.0f;
        }

        unsigned short aNewFacingDirection = SeekDirection(
            m_aActualFacingDirection,
            m_aDesiredFacingDirection,
            fSeekSpeed,
            fSeekFalloff,
            fDeltaT);
        SetFacingDirection(aNewFacingDirection);
    }

    if (g_pBall->m_pOwner != this && mfWaitTime > 0.0f)
    {
        TacklePlayer(g_pBall->m_pOwner);

        float fNoPickupTime = m_tNoPickupTimer.GetSeconds();
        if (fNoPickupTime > 0.0f)
        {
            const nlVector3& pickupPos = GetJointPosition(m_nBallJointIndex);
            nlVector3 v3TargetPos = pickupPos;

            float fGoallineX = cField::GetGoalLineX(1U);
            float fDeltaPos = 0.0f;
            if (v3TargetPos.f.x > fGoallineX)
            {
                fDeltaPos = fGoallineX - v3TargetPos.f.x;
            }
            else if (v3TargetPos.f.x < -fGoallineX)
            {
                fDeltaPos = -fGoallineX - v3TargetPos.f.x;
            }

            if (fDeltaPos != 0.0f)
            {
                v3TargetPos.f.x += fDeltaPos;

                nlVector3 v3MyPos = m_v3Position;
                v3MyPos.f.x += fDeltaPos;
                SetPosition(v3MyPos);
            }

            float fBlend;
            float fPercent = fNoPickupTime / mfWaitTime;
            fBlend = 1.0f - fPercent;
            v3TargetPos.f.x = fBlend * v3TargetPos.f.x + fPercent * g_pBall->m_v3Position.f.x;
            v3TargetPos.f.y = fBlend * v3TargetPos.f.y + fPercent * g_pBall->m_v3Position.f.y;
            v3TargetPos.f.z = fBlend * v3TargetPos.f.z + fPercent * g_pBall->m_v3Position.f.z;
            g_pBall->SetPosition(v3TargetPos);

            nlVector3 v3BallVel = g_pBall->m_v3Velocity;
            float fSpeedSq = v3BallVel.f.x * v3BallVel.f.x + v3BallVel.f.y * v3BallVel.f.y + v3BallVel.f.z * v3BallVel.f.z;
            if (fSpeedSq > 64.0f)
            {
                v3BallVel.f.x = 0.3f * v3BallVel.f.x;
                v3BallVel.f.y = 0.3f * v3BallVel.f.y;
                v3BallVel.f.z = 0.3f * v3BallVel.f.z;
                g_pBall->SetVelocity(v3BallVel, SPINTYPE_NONE, NULL);
            }
        }
        else
        {
            Audio::SoundAttributes sndAtr;
            sndAtr.Init();
            sndAtr.SetSoundType(0xB7, true);
            sndAtr.UseStationaryPosVector(m_v3Position);
            sndAtr.mf_Volume = 0.4f;
            Audio::gStadGenSFX.Play(sndAtr);

            PickupBall(g_pBall);
            mbPickedUp = true;
            g_pBall->ClearShotInProgress();
        }
    }

    if (g_pBall->m_pOwner != NULL && g_pBall->m_pOwner != this)
    {
        if (IsOnSameTeam(g_pBall->m_pOwner))
        {
            InitActionMove(false);
            return;
        }

        SetGoalieAction(GOALIEACTION_PURSUE_BALL_POUNCE, 0);
        return;
    }

    if (IsPassThreat())
    {
        InitActionMove(true);
        return;
    }

    if (m_pBall == NULL && mfWaitTime <= 0.0f)
    {
        if (fTimeLeft >= mpLooseBallInfo->mfPickupTime)
        {
            if (mpLooseBallInfo->mAnimType == LOOSEBALL_ANIM_KICK)
            {
                if (!m_pCurrentAnimController->TestTrigger(mpLooseBallInfo->mfPickupTime))
                {
                    return;
                }

                const nlVector3& pickupPos = GetJointPosition(m_nBallJointIndex);
                if (CalculateDistanceSquared(g_pBall->m_v3Position, pickupPos) < 1.0f
                    || CalculateDistanceSquared(g_pBall->m_v3Position, m_v3Position) < 2.25f)
                {
                    InitiatePickup();
                    return;
                }

                InitActionMove(true);
                return;
            }

            const nlVector3& pickupPos = GetJointPosition(m_nBallJointIndex);
            if (CalculateDistanceSquared(g_pBall->m_v3Position, pickupPos) < 1.0f)
            {
                InitiatePickup();
            }
            return;
        }

        float fPercent = (fTimeLeft - mfTargetTime) / (mpLooseBallInfo->mfPickupTime - mfTargetTime);
        float fSmooth = fPercent * (fPercent * ((-2.0f * fPercent) + 3.0f));

        if (!(fSmooth < 0.99f))
        {
            return;
        }

        nlVector3 v3BallVel;
        FakeBallWorld::GetPredictedBallPosition(mpLooseBallInfo->mfAnimDuration * (mpLooseBallInfo->mfPickupTime - fTimeLeft), mv3TargetPosition, v3BallVel);

        TrackTarget(mv3TargetPosition, fSmooth);

        nlVector3 v3MyPos;
        nlVector3 animPos = GetJointPosition(m_nHeadJointIndex);
        float fDX = 0.0f;
        float fAbsX = (float)fabs(animPos.f.x);
        float fLimit = cField::GetGoalLineX(1U) - 0.5f;
        float fNetY = 0.5f * cNet::m_fNetWidth;
        if (fAbsX > fLimit)
        {
            if ((float)fabs(animPos.f.y) > fNetY)
            {
                fDX = fAbsX - fLimit;
            }
        }

        nlVector3 leftHandPos;
        nlVector3 rightHandPos = GetJointPosition(m_nRightHandJointIndex);
        fAbsX = (float)fabs(rightHandPos.f.x);
        fLimit = cField::GetGoalLineX(1U) - 0.4f;
        if (fAbsX > fLimit)
        {
            if ((float)fabs(rightHandPos.f.y) > fNetY)
            {
                float fDiff = fAbsX - fLimit;
                if (fDiff > fDX)
                {
                    fDX = fDiff;
                }
            }
        }

        leftHandPos = GetJointPosition(m_nLeftHandJointIndex);
        fAbsX = (float)fabs(leftHandPos.f.x);
        if (fAbsX > fLimit)
        {
            if ((float)fabs(leftHandPos.f.y) > fNetY)
            {
                float fDiff = fAbsX - fLimit;
                if (fDiff > fDX)
                {
                    fDX = fDiff;
                }
            }
        }

        if (fDX > 0.0f)
        {
            v3MyPos = m_v3Position;
            if (v3MyPos.f.x > 0.0f)
            {
                fDX *= -1.0f;
            }
            v3MyPos.f.x += fDX;
            SetPosition(v3MyPos);
        }
    }
}

/**
 * Offset/Address/Size: 0x3630 | 0x80051B6C | size: 0x9C
 */
void Goalie::ActionLooseBallPursueRolling(float deltaTime)
{
    DoNavigation(deltaTime, 0.2f + mfGoalieStepDist, NAVI_FACE_BALL);

    if ((mnOffplayPending)
        || (!IsLooseBallClose(SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fLooseBallChaseDistance))
        || ((g_pBall->m_pOwner != NULL) && (g_pBall->m_pOwner != this)))
    {
        InitActionMove(true);
        return;
    }

    InitActionLooseBallSetup();
}

/**
 * Offset/Address/Size: 0x35A8 | 0x80051AE4 | size: 0x88
 */
void Goalie::ActionLooseBallSetup(float)
{
    if ((mnOffplayPending)
        || (!IsLooseBallClose(SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fLooseBallChaseDistance))
        || ((g_pBall->m_pOwner != NULL) && (g_pBall->m_pOwner != this)))
    {
        InitActionMove(true);
        return;
    }
    InitActionLooseBallSetup();
}

/**
 * Offset/Address/Size: 0x358C | 0x80051AC8 | size: 0x1C
 */
void Goalie::MoveDirectionCB(unsigned int nParam, cPN_SingleAxisBlender* blender)
{
    Goalie* pGoalie = (Goalie*)nParam;
    float result = 0.0f;
    if (pGoalie->mv3LocalNavTarget.f.y < 0.0f)
    {
        result = 1.0f;
    }
    blender->m_fDesiredWeight = result;
}

/**
 * Offset/Address/Size: 0x3544 | 0x80051A80 | size: 0x48
 */
void Goalie::MoveWeightCB(unsigned int nParam, cPN_SingleAxisBlender* blender)
{
    Goalie* pGoalie = (Goalie*)nParam;
    blender->m_fDesiredWeight = (s32)(u16)abs_s16(pGoalie->maLocalAngle) / 32768.0f;
}

/**
 * Offset/Address/Size: 0x3538 | 0x80051A74 | size: 0xC
 */
void Goalie::StrafeSynchronizedSpeedCallback(unsigned int nParam, cPN_SAnimController* controller)
{
    Goalie* pGoalie = (Goalie*)nParam;
    controller->m_fPlaybackSpeedScale = pGoalie->mfSpeedScale;
}

/**
 * Offset/Address/Size: 0x30E0 | 0x8005161C | size: 0x458
 * TODO: 99.86% match - remaining diffs are local static/literal-pool label IDs.
 */
void Goalie::ActionMove(float deltaTime)
{
    float dt = deltaTime;

    if (mnOffplayPending != GOALIE_OFFPLAY_NONE)
    {
        static FilteredRandomRange randgenDejected;
        int animID = -1;

        switch (mnOffplayPending)
        {
        case GOALIE_OFFPLAY_NONE:
        case GOALIE_OFFPLAY_GOAL_FOR:
            break;

        case GOALIE_OFFPLAY_GOAL_AGAINST:
            animID = gOffplayDejected[randgenDejected.genrand(5)];
            break;

        case GOALIE_OFFPLAY_ENDGAME_WIN:
            break;

        case GOALIE_OFFPLAY_ENDGAME_LOSE:
            animID = gOffplayDejected[randgenDejected.genrand(5)];
            break;

        case GOALIE_OFFPLAY_HALFTIME:
        case GOALIE_OFFPLAY_PENALTY:
            break;

        default:
            break;
        }

        if (animID >= 0)
        {
            mbDoHeadTrack = false;
            SetGoalieAction(GOALIEACTION_OFFPLAY, 0);
            SetAnimState(animID, true, 0.2f, false, false);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);
            return;
        }
    }

    if (g_pBall->m_pOwner != this)
    {
        if (GetGlobalPad() != NULL)
        {
            SwapController();
        }
    }

    bool isPassThreat = IsPassThreat();

    nlVector3 desiredDir;
    nlVector3 targetPos;
    nlVector3 desiredOffset;
    u16 desiredFacing;

    if (isPassThreat)
    {
        if (CanInterceptPass())
        {
            InitActionPassIntercept();
            return;
        }

        cFielder* pPassTarget = (cFielder*)g_pBall->GetPassTargetFielder();
        targetPos = g_pBall->m_v3PassIntercept;
        FindDesiredGoaliePosition(mv3TargetPosition, desiredDir, desiredOffset, desiredFacing, &targetPos);

        if (pPassTarget->m_eFielderDesireState == FIELDERDESIRE_ONETIMER)
        {
            float crouchDuration = GoalieSave::mfCrouchDuration;
            if (g_pBall->m_tPassTargetTimer.GetSeconds() < crouchDuration
                && IsCloseToPlane(mv3TargetPosition, m_v3Position, 1.5f))
            {
                InitActionPreCrouch(GOALIECROUCH_PASS);
                return;
            }

            mUrgency = URGENCY_HIGH;
        }

        mbDoIntercept = true;
    }
    else
    {
        mbDoIntercept = false;
        FindDesiredGoaliePosition(mv3TargetPosition, desiredDir, desiredOffset, desiredFacing, NULL);
        targetPos = g_pBall->m_v3Position;
    }

    mv3NavTarget = mv3TargetPosition;
    m_aDesiredFacingDirection = desiredFacing;

    DoNavigation(dt, 0.2f + mfGoalieStepDist, NAVI_FACE_BALL);

    if (CheckForSTSAttack())
    {
        return;
    }

    if (CheckForLooseBallShotInProgress())
    {
        return;
    }

    if (IsInsideGoalieBox(g_pBall->m_v3Position, 1.0f, 1.0f))
    {
        if (mUrgency == URGENCY_LOW)
        {
            mUrgency = URGENCY_MED;
        }
    }
    else if (mUrgency == URGENCY_MED)
    {
        mUrgency = URGENCY_LOW;
    }

    if (IsLooseBallClose(SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fLooseBallChaseDistance))
    {
        if (!isPassThreat)
        {
            InitActionLooseBallSetup();
        }
        return;
    }

    if (IsOpponentBallCarrierInRange())
    {
        if (IsWithinPounceRange())
        {
            InitActionPursueBallCarrier();
            InitActionPursueBallPounce();
            return;
        }

        cFielder* pOwnerFielder = g_pBall->GetOwnerFielder();

        bool shouldPreCrouch = false;
        s32 shotMeterState = *(s32*)pOwnerFielder->m_pShotMeter;
        if (shotMeterState == 1 || shotMeterState == 3 || shotMeterState == 4)
        {
            shouldPreCrouch = true;
        }

        if (shouldPreCrouch)
        {
            if (IsCloseToPlane(mv3TargetPosition, m_v3Position, 1.2f))
            {
                InitActionPreCrouch(GOALIECROUCH_SHOT);
            }
            else
            {
                mUrgency = URGENCY_HIGH;
            }
            return;
        }

        InitActionPursueBallCarrier();
        return;
    }

    if (IsTeammateHoardingBall())
    {
        InitActionGrabBall();
    }
}

inline s16 ClampMin(const s16 diff, const s16 min)
{
    if (diff >= min)
    {
        return diff;
    }
    return min;
}

inline s16 ClampMax(const s16 diff, const s16 max)
{
    if (diff <= max)
    {
        return diff;
    }
    return max;
}

/**
 * Offset/Address/Size: 0x3070 | 0x800515AC | size: 0x70
 */
void Goalie::RunWeightCB(unsigned int nParam, cPN_SingleAxisBlender* blender)
{
    const Goalie* pGoalie = (Goalie*)nParam;

    s16 diff = (s16)(pGoalie->m_aDesiredFacingDirection - pGoalie->m_aActualFacingDirection);

    s16 minClampedDiff;
    if (diff < -0x31C4)
    {
        minClampedDiff = -0x31C4;
    }
    else
    {
        minClampedDiff = diff;
    }

    s16 clampedDiff;
    if (minClampedDiff > 0x31C4)
    {
        clampedDiff = 0x31C4;
    }
    else
    {
        clampedDiff = minClampedDiff;
    }

    blender->m_fDesiredWeight = (float)(clampedDiff + 0x31C4) / 25480.0f;
}

inline void Goalie::StartRunBlend()
{
    int runAnims[] = { 0x21, 0x1F, 0x20 };

    cPN_SingleAxisBlender* pRunSAB = CreateSingleAxisBlender(runAnims, 3, 1, RunWeightCB, 0.15f, NULL);

    cPN_SAnimController* pPrevCtrlr = NULL;
    for (int i = 0; i < 3; i++)
    {
        cPN_SAnimController* pCtrlr = (cPN_SAnimController*)pRunSAB->GetChild(i);
        if (pPrevCtrlr == NULL)
        {
            pCtrlr->m_fSynchronizedWeight = 0.0f;
        }
        else
        {
            pCtrlr->m_bIsSynchronized = true;
            pPrevCtrlr->m_pSynchronizedController = pCtrlr;
        }
        pPrevCtrlr = pCtrlr;
    }

    cPN_Blender* pNewBlender = AllocateBlender();
    if (pNewBlender != NULL)
    {
        pNewBlender = new ((u8*)pNewBlender) cPN_Blender(*m_pAILayer, pRunSAB, 0.1f);
    }
    *m_pAILayer = pNewBlender;
    InitMovementFromAnimSeek(60000.0f, 4000.0f);
}

/**
 * Offset/Address/Size: 0x27FC | 0x80050D38 | size: 0x874
 */
void Goalie::ActionMoveWB(float fDeltaT)
{
    if (mnSubstate == 6)
    {
        bool isAnimDone = false;
        cPN_SAnimController* pCtrl = m_pCurrentAnimController;
        if (pCtrl->m_ePlayMode == PM_HOLD)
        {
            if (1.0f == pCtrl->m_fTime)
            {
                isAnimDone = true;
            }
        }
        if (isAnimDone)
        {
            mnSubstate = 0;
        }
        else
        {
            return;
        }
    }

    if (m_pController == NULL)
    {
        goto no_pad;
    }

    if (mfWaitTime > 0.0f)
    {
    }
    else
    {
        goto no_pad;
    }

    if (m_pBall == NULL)
    {
        InitActionMove(false);
        return;
    }

    {
        mfWaitTime -= fDeltaT;

        float stickMag = m_pController->GetMovementStickMagnitude();

        if (stickMag > 0.0f)
        {
            mfTargetTime = 0.0f;

            f32 penaltyBoxX, penaltyBoxY;
            penaltyBoxY = cField::GetPenaltyBoxY() - 0.5f;
            penaltyBoxX = 0.5f + cField::GetPenaltyBoxX(1U);

            u16 direction = m_pController->GetMovementStickDirection();
            m_aDesiredFacingDirection = direction;

            float jogging = m_pTweaks->fJoggingSpeed;
            float running = m_pTweaks->fRunningSpeed;
            m_fDesiredSpeed = jogging + stickMag * (running - jogging);

            float posX = m_v3Position.f.x;
            float posY = m_v3Position.f.y;
            u16 dir = m_aDesiredFacingDirection;

            if ((float)fabs(posX) < penaltyBoxX)
            {
                if (posX > 0.0f)
                {
                    dir = (u16)(dir + 0x8000);
                }

                u16 d = dir;
                if (d < 0x1C18 || d > 0xE3E7)
                {
                    m_fDesiredSpeed = 0.0f;
                }
                else if (d < 0x43E8)
                {
                    dir = 0x43E8;
                }
                else if (d > 0xBC17)
                {
                    dir = 0xBC17;
                }

                if (posX > 0.0f)
                {
                    dir += 0x8000;
                }
            }

            if ((float)fabs(posY) > penaltyBoxY)
            {
                if (posY < 0.0f)
                {
                    dir = (u16)(dir + 0x8000);
                }

                u16 d = dir;
                if (d < 0x23E8 || d > 0xFC17)
                {
                    dir = 0xFC17;
                }
                else if (d < 0x5C18)
                {
                    m_fDesiredSpeed = 0.0f;
                }
                else if (d < 0x83E8)
                {
                    dir = 0x83E8;
                }

                if (posY < 0.0f)
                {
                    dir += 0x8000;
                }
            }

            if (m_fDesiredSpeed > 0.0f)
            {
                m_aDesiredFacingDirection = dir;
            }
            else
            {
                m_aDesiredFacingDirection = m_aActualFacingDirection;
            }
        }
        else
        {
            mfTargetTime += fDeltaT;
            m_fDesiredSpeed = 0.0f;
            m_aDesiredFacingDirection = m_aActualFacingDirection;
        }

        {
            bool bClamped = false;

            if ((float)fabs(m_v3Position.f.x) < cField::GetPenaltyBoxX(1U))
            {
                if (m_v3Position.f.x > 0.0f)
                {
                    m_aDesiredFacingDirection = 0;
                }
                else
                {
                    m_aDesiredFacingDirection = 0x8000;
                }
                bClamped = true;
            }

            if ((float)fabs(m_v3Position.f.y) > cField::GetPenaltyBoxY())
            {
                u16 yDir;
                if (m_v3Position.f.y > 0.0f)
                {
                    yDir = 0xC000;
                }
                else
                {
                    yDir = 0x4000;
                }

                if (bClamped)
                {
                    s16 diff = (s16)(yDir - m_aDesiredFacingDirection);
                    m_aDesiredFacingDirection = (u16)(m_aDesiredFacingDirection + (s16)(diff * 0.5f));
                }
                else
                {
                    m_aDesiredFacingDirection = yDir;
                }
                bClamped = true;
            }

            if (bClamped)
            {
                if (m_fDesiredSpeed < 0.001f)
                {
                    m_fDesiredSpeed = m_pTweaks->fJoggingSpeed;
                }
            }
        }

        if (GetGlobalPad()->JustPressed(0x17, true))
        {
            m_pTeam->TogglePowerup(false);
        }

        if (GetGlobalPad()->JustPressed(0x1c, true))
        {
            m_eLastPadAction = (ePadActions)0x25;
            InitActionPass(false);
            return;
        }

        if (GetGlobalPad()->JustPressed(0x1b, true))
        {
            m_eLastPadAction = (ePadActions)0x25;
            InitActionPass(true);
            return;
        }

        if (mfTargetTime > 1.0f)
        {
            nlVector3 v3Center = m_v3Position;
            float dist = nlGetLength3D(m_v3Position.f.x, m_v3Position.f.y, m_v3Position.f.z);
            float invDist = -1.0f / dist;
            v3Center.f.x = invDist * m_v3Position.f.x;
            v3Center.f.y = invDist * m_v3Position.f.y;
            v3Center.f.z = invDist * m_v3Position.f.z;

            float dot = m_m4WorldMatrix.m[0][1] * v3Center.f.y + m_m4WorldMatrix.m[0][0] * v3Center.f.x + m_m4WorldMatrix.m[0][2] * v3Center.f.z;

            if (dot > 0.5)
            {
                mfTargetTime = 0.0f;
                PlayNewAnim(14);
                InitMovementFromAnim(0, v3Zero, 1.0f, false);
            }
        }

        if (m_fDesiredSpeed > 0.01f)
        {
            if (m_eAnimID == 0x1F)
            {
                return;
            }

            StartRunBlend();
        }
        else
        {
            if (m_eAnimID == 0x0E)
            {
                return;
            }
            PlayNewAnim(9);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);
        }
        return;
    }

no_pad:
    mfWaitTime = 0.0f;

    switch (mnSubstate)
    {
    case 0:
        mnSubstate = 4;
        return;

    case 5:
    {
        bool isAnimDone = false;
        cPN_SAnimController* pCtrl = m_pCurrentAnimController;
        if (pCtrl->m_ePlayMode == PM_HOLD)
        {
            if (1.0f == pCtrl->m_fTime)
            {
                isAnimDone = true;
            }
        }
        if (!isAnimDone)
        {
            return;
        }
        mnSubstate = 4;
        return;
    }

    case 4:
    {
        float posY = m_v3Position.f.y;
        float posX = m_v3Position.f.x;
        float angle = nlATan2f(-posY, -posX);
        m_aDesiredFacingDirection = (u16)(s32)(10430.378f * angle);

        float absX = (float)fabs(m_v3Position.f.x);
        float goalLineX = cField::GetGoalLineX(1U);
        if (goalLineX - 3.0f >= absX)
        {
            s16 diff = (s16)(m_aDesiredFacingDirection - m_aActualFacingDirection);
            if (diff < 0)
            {
                diff = -diff;
            }
            if ((u16)diff <= 0xDAC)
            {
                mnSubstate = 7;
                return;
            }
        }

        m_fDesiredSpeed = m_pTweaks->fRunningSpeed;

        if (m_eAnimID == 0x1F)
        {
            return;
        }

        StartRunBlend();
        return;
    }

    case 7:
        InitActionPass(true);
        return;

    default:
        return;
    }
}

/**
 * Offset/Address/Size: 0x2758 | 0x80050C94 | size: 0xA4
 */
void Goalie::ActionSaveSetup(float deltaTime)
{
    float deflectResult = CheckForDelflectAwayFromNet();

    if (deflectResult < 0.0f)
    {
        return;
    }

    if (deflectResult > 0.0f)
    {
        InitActionSaveSetup(false);
        return;
    }

    if (mnOffplayPending != 0)
    {
        InitActionMove(true);
        return;
    }

    mfWaitTime -= deltaTime;
    if (mfWaitTime <= 0.01f)
    {
        InitActionSave();
    }
}

/**
 * Offset/Address/Size: 0x25D8 | 0x80050B14 | size: 0x180
 */
void Goalie::ActionSaveReposition(float deltaTime)
{
    if (mnOffplayPending != 0)
    {
        InitActionMove(true);
        return;
    }

    mfWaitTime -= deltaTime;

    float distSq = nlGetLengthSquared2D(m_v3Position.f.x - mv3NavTarget.f.x, m_v3Position.f.y - mv3NavTarget.f.y);

    bool shouldReposition = false;
    if ((distSq < gfRepositionThreshold * gfRepositionThreshold) || (distSq > mfTargetDist && distSq < 1.6899998f))
    {
        shouldReposition = true;
    }

    mfTargetDist = distSq;

    float deflectResult = CheckForDelflectAwayFromNet();
    if (deflectResult < 0.0f)
    {
        return;
    }

    if (mfWaitTime <= 0.02f || deflectResult > 0.0f || shouldReposition)
    {
        InitActionSaveSetup(false);
        return;
    }

    if (mfWaitTime < 0.05f)
    {
        PlayNewAnim(10);
        InitMovementFromAnimSeek(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff);
        return;
    }

    float ballDx = g_pBall->m_v3Position.f.x - m_v3Position.f.x;
    float ballDy = g_pBall->m_v3Position.f.y - m_v3Position.f.y;
    float angle = nlATan2f(ballDy, ballDx);
    m_aDesiredFacingDirection = (u16)(s32)(10430.378f * angle); // @1734 constant

    DoNavigation(deltaTime, gfRepositionThreshold, NAVI_FACE_DESIRED);
}

/**
 * Offset/Address/Size: 0x20BC | 0x800505F8 | size: 0x51C
 * TODO: 97.32% match - fAnimTime in f29 instead of target f31, cascading register diffs in smoothstep/ball-catch sections
 */
void Goalie::ActionSave(float)
{
    Audio::SoundAttributes sndAtr;
    nlVector3 v3AdjPos;
    nlVector3 v3HeadCopy;
    nlVector3 v3LHandCopy;
    nlVector3 v3RHandCopy;

    const nlVector3& v3LHandPos = GetJointPosition(m_nHeadJointIndex);
    v3HeadCopy = v3LHandPos;
    float fAbsX = (float)fabs(v3HeadCopy.f.x);
    float fDX = 0.0f;
    float fLimit = cField::GetGoalLineX(1U) - 0.5f;
    float fNetY = 0.5f * cNet::m_fNetWidth;

    if (fAbsX > fLimit)
    {
        if ((float)fabs(v3HeadCopy.f.y) > fNetY)
        {
            fDX = fAbsX - fLimit;
        }
    }

    const nlVector3& v3RHandPos = GetJointPosition(m_nRightHandJointIndex);
    v3RHandCopy = v3RHandPos;
    fAbsX = (float)fabs(v3RHandCopy.f.x);
    float fLimit2 = cField::GetGoalLineX(1U) - 0.4f;

    if (fAbsX > fLimit2)
    {
        if ((float)fabs(v3RHandCopy.f.y) > fNetY)
        {
            float fDiff = fAbsX - fLimit2;
            if (fDiff > fDX)
            {
                fDX = fDiff;
            }
        }
    }

    const nlVector3& v3LHandPos2 = GetJointPosition(m_nLeftHandJointIndex);
    v3LHandCopy = v3LHandPos2;
    float fAbsX3 = (float)fabs(v3LHandCopy.f.x);

    if (fAbsX3 > fLimit2)
    {
        if ((float)fabs(v3LHandCopy.f.y) > fNetY)
        {
            float fDiff = fAbsX3 - fLimit2;
            if (fDiff > fDX)
            {
                fDX = fDiff;
            }
        }
    }

    if (fDX > 0.0f)
    {
        v3AdjPos = m_v3Position;
        if (v3AdjPos.f.x > 0.0f)
        {
            fDX *= -1.0f;
        }
        v3AdjPos.f.x = v3AdjPos.f.x + fDX;
        SetPosition(v3AdjPos);
    }

    float fCrouchTime;
    SaveData* pSaveData = mpSaveData;
    float fTakeoffTime = pSaveData->mfMilestonePercent[1];
    float fAnimTime = m_pCurrentAnimController->m_fTime;
    fCrouchTime = pSaveData->mfMilestonePercent[0];

    if (fTakeoffTime <= 0.0f)
    {
        float fGoalTime = pSaveData->mfMilestonePercent[2];
        fTakeoffTime = 0.7f * fGoalTime;
        fCrouchTime = 0.4f * fGoalTime;
    }

    if (fAnimTime <= fTakeoffTime)
    {
        if (m_pBall == NULL)
        {
            float deflectResult = CheckForDelflectAwayFromNet();
            if (deflectResult < 0.0f)
            {
                return;
            }
            if (deflectResult > 0.0f)
            {
                if (fAnimTime < fCrouchTime)
                {
                    mGoalieActionState = GOALIEACTION_SAVE_REPOSITION;
                }
                else
                {
                    mGoalieActionState = GOALIEACTION_PRE_CROUCH;
                }
                InitActionSaveSetup(false);
                return;
            }
        }
    }

    if (mbDoHeadTrack)
    {
        float dZ = g_pBall->m_v3Position.f.y - m_v3Position.f.y;
        float dX = g_pBall->m_v3Position.f.x - m_v3Position.f.x;
        float distSq = dX * dX + dZ * dZ;
        float m00 = m_m4WorldMatrix.m[0][0];
        float m01 = m_m4WorldMatrix.m[0][1];
        float m02 = m_m4WorldMatrix.m[0][2];
        if (distSq < 9.0f || dX * m00 + dZ * m01 + 0.0f * m02 < 0.0f)
        {
            mbDoHeadTrack = false;
        }
    }

    if (fAnimTime < mpSaveData->mfMilestonePercent[2])
    {
        float t = fAnimTime / mpSaveData->mfMilestonePercent[2];
        float smoothstep = -2.0f * t + 3.0f;
        smoothstep = t * smoothstep;
        smoothstep = t * smoothstep;
        smoothstep = 1024.0f * smoothstep;
        s16 delta = (s16)(m_aDesiredFacingDirection - m_aActualFacingDirection);
        s32 iSmooth = (s32)smoothstep;
        u16 newFacing = (u16)(m_aActualFacingDirection + (iSmooth * delta) / 1024);
        SetFacingDirection(newFacing);
    }

    if (g_pBall->m_tShotTimer.m_uPackedTime != 0 || g_pBall->m_pPassTarget != NULL)
    {
        if (g_pBall->m_pOwner != this)
        {
            if ((mpSaveData->muSaveType & 3) != 0)
            {
                GoalieTweaks* pTweaks = (GoalieTweaks*)m_pTweaks;
                float fReachSq = pTweaks->fSaveCatchTolerance * pTweaks->fSaveCatchTolerance;
                const nlVector3& v3LHand = GetJointPosition(m_nLeftHandJointIndex);
                const nlVector3& v3RHand = GetJointPosition(m_nRightHandJointIndex);

                float dX = g_pBall->m_v3Position.f.x - v3LHand.f.x;
                float dY = g_pBall->m_v3Position.f.y - v3LHand.f.y;
                float dZ = g_pBall->m_v3Position.f.z - v3LHand.f.z;
                float distSqL = dX * dX;
                distSqL = dY * dY + distSqL;
                distSqL = dZ * dZ + distSqL;

                if (distSqL < fReachSq)
                {
                    goto do_catch;
                }

                dY = g_pBall->m_v3Position.f.y - v3RHand.f.y;
                dX = g_pBall->m_v3Position.f.x - v3RHand.f.x;
                dZ = g_pBall->m_v3Position.f.z - v3RHand.f.z;
                float distSqR = dY * dY;
                distSqR = dX * dX + distSqR;
                distSqR = dZ * dZ + distSqR;

                if (distSqR < fReachSq)
                {
                do_catch:
                    TacklePlayer(g_pBall->m_pOwner);
                    MakeSaveEvent(false);

                    sndAtr.Init();
                    sndAtr.SetSoundType(0xC0, true);
                    sndAtr.UseStationaryPosVector(m_v3Position);
                    Audio::gStadGenSFX.Play(sndAtr);

                    bool bIsPerfect = false;
                    if (g_pBall->m_tShotTimer.m_uPackedTime != 0 && g_pBall->m_unk_0xA4)
                    {
                        bIsPerfect = true;
                    }

                    if (bIsPerfect)
                    {
                        EmitGoalieCatch(this, "perfect_shot_catch", false);
                    }
                    else
                    {
                        EmitGoalieCatch(this, "goalie_catch", false);
                    }

                    PickupBall(g_pBall);
                    g_pBall->ClearShotInProgress();
                    if (g_pBall->m_pPassTarget != NULL)
                    {
                        g_pBall->ClearPassTarget();
                    }
                    mbBallImpacted = true;
                }
            }
        }
    }

    if (m_pCurrentAnimController->m_fTime > 0.95f)
    {
        InitActionDiveRecover();
    }
}

template <typename T>
class nlSingleton
{
public:
    static T* s_pInstance;
};

class GameInfoManager : public nlSingleton<GameInfoManager>
{
public:
    int GetStadium() const;
};

/**
 * Offset/Address/Size: 0x1C30 | 0x8005016C | size: 0x48C
 * TODO: 98.0% match — register allocation in the SetPosition projection block
 *       (mv3NavTarget delta + smoothstep) loads m_v3Position members in a
 *       different order than target; one branch destination is 4 bytes off.
 */
void Goalie::ActionSTS(float fDeltaT)
{
    nlVector3 v3BallDir;
    nlVector4 plane;
    nlVector4 plane2;
    nlVector3 v3Root;
    nlVector3 v3Projected;
    unsigned short aRoot;
    nlVector3 v3GoaliePos;

    f32 fAnimTime = m_pCurrentAnimController->m_fTime;

    if (m_pBall == NULL)
    {
        cBall* pBall = g_pBall;
        float fSavePercent;

        if (mpShooter != NULL)
        {
            nlVector3* pGoaliePos = &m_v3Position;
            fSavePercent = mpLooseBallInfo->mfPickupTime;
            nlVec3Sub(v3BallDir, *pGoaliePos, mpShooter->m_v3Position);
            f32 fDot = v3BallDir.f.x * pBall->m_v3Velocity.f.x
                     + v3BallDir.f.y * pBall->m_v3Velocity.f.y
                     + v3BallDir.f.z * pBall->m_v3Velocity.f.z;
            if (fDot > 0.0f)
            {
                MakePerpendicularPlane(*pGoaliePos, v3BallDir, plane, 0.7f);
                f32 fDist = pBall->m_v3Position.f.x * plane.f.x
                          + pBall->m_v3Position.f.y * plane.f.y
                          + pBall->m_v3Position.f.z * plane.f.z
                          - plane.f.w;
                if (fDist > 0.0f)
                {
                    fSavePercent = m_pCurrentAnimController->m_fTime - 0.001f;
                }
            }
        }
        else
        {
            f32 fMilestone = mpSaveData->mfMilestonePercent[2];
            fSavePercent = fMilestone;
            if (fAnimTime < fMilestone)
            {
                unsigned int uSaveType = mpSaveData->muSaveType;
                if ((uSaveType & 3) != 0 || (uSaveType == 0x20000 && m_eAnimID != 0x6d))
                {
                    f32 fBallFutureX;
                    f32 fBallFutureY;
                    f32 fBallFutureZ;
                    fBallFutureZ = fDeltaT * pBall->m_v3Velocity.f.z + pBall->m_v3Position.f.z;
                    fBallFutureY = fDeltaT * pBall->m_v3Velocity.f.y + pBall->m_v3Position.f.y;
                    fBallFutureX = fDeltaT * pBall->m_v3Velocity.f.x + pBall->m_v3Position.f.x;
                    MakePerpendicularPlane(m_v3Position, m_aActualFacingDirection, plane2, 0.5f);
                    f32 fPlaneDist = fBallFutureX * plane2.f.x
                                   + fBallFutureY * plane2.f.y
                                   + fBallFutureZ * plane2.f.z
                                   - plane2.f.w;
                    if (fPlaneDist < 0.0f)
                    {
                        fSavePercent = m_pCurrentAnimController->m_fTime - 0.001f;
                    }
                }
            }
        }
        if (m_pCurrentAnimController->TestTrigger(fSavePercent))
        {
            HandleSTSContact(g_pBall);
        }
    }
    else
    {
        if (mpSaveData != NULL && mpSaveData->muSaveType == 0x20000 && m_eAnimID != 0x6d)
        {
            if (mfTargetTime < fAnimTime && fAnimTime < 1.0f)
            {
                GetCurrentAnimFuture(-1, 1.0f, v3Root, v3Projected, aRoot);
                f32 dz = mv3NavTarget.f.z - v3Projected.f.z;
                f32 dy = mv3NavTarget.f.y - v3Projected.f.y;
                f32 dx = mv3NavTarget.f.x - v3Projected.f.x;
                v3Root.f.z = dz;
                v3Root.f.y = dy;
                v3Root.f.x = dx;
                f32 posZ = m_v3Position.f.z;
                f32 posY = m_v3Position.f.y;
                f32 posX = m_v3Position.f.x;
                f32 fT = (fAnimTime - mfTargetTime) / (1.0f - mfTargetTime);
                f32 fSmoothT = fT * fT * (-2.0f * fT + 3.0f);
                v3Root.f.z = posZ + fSmoothT * dz;
                v3Root.f.y = posY + fSmoothT * dy;
                v3Root.f.x = posX + fSmoothT * dx;
                SetPosition(v3Root);
            }
            f32 fGoalieNetYLimit = 0.5f * cNet::m_fNetWidth - 0.7f;
            f32 fStadiumVal = 1.0f;
            switch (nlSingleton<GameInfoManager>::s_pInstance->GetStadium())
            {
            case 1:
                fStadiumVal = 1.6f;
                break;
            case 3:
                fStadiumVal = 1.8f;
                break;
            case 4:
                fStadiumVal = 2.0f;
                break;
            }
            f32 fGoalLineLimit = cField::GetGoalLineX(1U);
            fGoalLineLimit += fStadiumVal;
            if (fAnimTime > 0.12f
                && (float)fabs(m_v3Position.f.y) > fGoalieNetYLimit
                && (float)fabs(m_v3Position.f.x) < fGoalLineLimit)
            {
                v3GoaliePos = m_v3Position;
                if (m_v3Position.f.y > 0.0f)
                    v3GoaliePos.f.y = fGoalieNetYLimit;
                else
                    v3GoaliePos.f.y = -fGoalieNetYLimit;
                SetPosition(v3GoaliePos);
            }
        }
    }
    if (fAnimTime > 0.95f)
    {
        SaveData* pSaveData = mpSaveData;
        if (pSaveData == NULL || pSaveData->muSaveType != 0x20000)
        {
            if (mnOffplayPending == GOALIE_OFFPLAY_NONE)
            {
                InitActionSTSRecover();
            }
            else if (pSaveData != NULL)
            {
                unsigned int uSaveType = pSaveData->muSaveType;
                if (uSaveType == 8)
                {
                    InitActionDiveRecover();
                }
                else if (uSaveType == 0x40000 || pSaveData->mnAnimID == 0x6d)
                {
                    InitActionSTSRecover();
                }
                else
                {
                    InitActionMove(true);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1BD8 | 0x80050114 | size: 0x58
 */
void Goalie::ActionSTSSetup(float deltaTime)
{
    if (mnOffplayPending != 0)
    {
        InitActionMove(true);
        return;
    }

    mfWaitTime -= deltaTime;
    if (mfWaitTime <= 0.01f)
    {
        InitActionSTS();
    }
}

/**
 * Offset/Address/Size: 0x1B18 | 0x80050054 | size: 0xC0
 */
void Goalie::ActionSTSRecover(float deltaTime)
{
    if (CheckForSTSAttack())
    {
        return;
    }

    mfWaitTime -= deltaTime;
    if (mfWaitTime <= 0.0f)
    {
        if (m_eAnimID != 0x70)
        {
            PlayNewAnim(0x70);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);
        }
        else
        {
            if (m_pCurrentAnimController->m_fTime > 0.95f)
            {
                InitActionMove(true);
            }
        }
    }
}

static inline float SubtractPosX(const nlVector3& pos, float x)
{
    return pos.f.x - x;
}

static inline float SubtractPosY(const nlVector3& pos, const nlVector3& nav)
{
    return pos.f.y - nav.f.y;
}

/**
 * Offset/Address/Size: 0x19A0 | 0x8004FEDC | size: 0x178
 */
void Goalie::ActionChipShotStumble(float deltaTime)
{
    bool bShouldRecover = false;
    if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
    {
        bShouldRecover = true;
    }

    if (bShouldRecover)
    {
        if (m_eAnimID == 0x70)
        {
            InitActionMove(true);
            return;
        }
        InitActionDiveRecover();
        return;
    }

    if (m_pCurrentAnimController->m_fTime < mpSaveData->mfMilestonePercent[2])
    {
        float deflectResult = CheckForDelflectAwayFromNet();
        if (deflectResult < 0.0f)
        {
            return;
        }

        if (deflectResult > 0.0f)
        {
            m_pPhysicsCharacter->m_CanCollidedWithGoalLine = true;
            InitActionSaveSetup(false);
            return;
        }
    }

    float x = mv3NavTarget.f.x;

    if ((float)fabs(x) > (0.5f + (float)fabs(m_v3Position.f.x)) && m_pCurrentAnimController->m_fTime < 0.5f)
    {
        m_aDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(SubtractPosY(m_v3Position, mv3NavTarget), SubtractPosX(m_v3Position, x)));

        GoalieTweaks* pTweaks = static_cast<GoalieTweaks*>(m_pTweaks);
        u16 newFacing = SeekDirection(
            m_aActualFacingDirection,
            m_aDesiredFacingDirection,
            pTweaks->fThrowingDirectionSeekSpeed,
            pTweaks->fThrowingDirectionSeekFalloff,
            deltaTime);
        SetFacingDirection(newFacing);
    }
}

/**
 * Offset/Address/Size: 0x1904 | 0x8004FE40 | size: 0x9C
 */
void Goalie::ActionDiveRecover(float)
{
    if (m_pBall == nullptr)
    {
        GoalieTweaks* pTweaks = static_cast<GoalieTweaks*>(m_pTweaks);
        if (mFatigue.mfEnergyLevel < pTweaks->fGetupEnergyHigh)
        {
            float result = InterpolateRangeClamped(
                pTweaks->fGetupSpeedLow,
                0.0f,
                pTweaks->fGetupEnergyLow,
                pTweaks->fGetupEnergyHigh,
                mFatigue.mfEnergyLevel);
            m_pCurrentAnimController->m_fPlaybackSpeedScale = result;
        }

        if (ShouldStartCrossBlend(8))
        {
            InitActionMove(false);
        }
    }
    else
    {
        if (ShouldStartCrossBlend(9))
        {
            InitActionMoveWB();
        }
    }
}

/**
 * Offset/Address/Size: 0x17FC | 0x8004FD38 | size: 0x108
 */
void Goalie::ActionPass(float deltaTime)
{
    if (m_pBall != nullptr)
    {
        if (mpPassTarget != nullptr)
        {
            float dy = mpPassTarget->m_v3Position.f.y - m_v3Position.f.y;
            float dx = mpPassTarget->m_v3Position.f.x - m_v3Position.f.x;
            float angleRad = nlATan2f(dy, dx);

            m_aDesiredFacingDirection = (unsigned short)(s32)(10430.378f * angleRad);
        }
        else
        {
            if (m_pTeam->m_pNet->m_baseLocation.f.x > 0.0f)
            {
                m_aDesiredFacingDirection = 0x8000;
            }
            else
            {
                m_aDesiredFacingDirection = 0;
            }
        }

        GoalieTweaks* pTweaks = static_cast<GoalieTweaks*>(m_pTweaks);
        unsigned short newFacing = SeekDirection(
            m_aActualFacingDirection,
            m_aDesiredFacingDirection,
            pTweaks->fThrowingDirectionSeekSpeed,
            pTweaks->fThrowingDirectionSeekFalloff,
            deltaTime);
        SetFacingDirection(newFacing);
    }
    if (ShouldStartCrossBlend(8))
    {
        InitActionMove(false);
    }
}

/**
 * Offset/Address/Size: 0x1554 | 0x8004FA90 | size: 0x2A8
 */
void Goalie::ActionPassIntercept(float deltaTime)
{
    if (muBallDeflectCount != g_pBall->m_bBallDeflectCount)
    {
        InitActionMove(true);
        return;
    }

    mfWaitTime -= deltaTime;

    switch (mnSubstate)
    {
    case 1:
    {
        if (mfWaitTime <= 0.02f)
        {
            InitActionPassInterceptSave();
        }
        return;
    }
    case 4:
    {
        // Calculate angle to target position
        float dy = mv3TargetPosition.f.y - m_v3Position.f.y;
        float dx = mv3TargetPosition.f.x - m_v3Position.f.x;
        float angleToTarget = nlATan2f(dy, dx);
        u16 targetAngle = (u16)(s32)(10430.378f * angleToTarget);

        // Calculate angle to ball position
        dy = g_pBall->m_v3Position.f.y - m_v3Position.f.y;
        dx = g_pBall->m_v3Position.f.x - m_v3Position.f.x;
        float angleToBall = nlATan2f(dy, dx);
        u16 ballAngle = (u16)(s32)(10430.378f * angleToBall);

        // Choose run animation based on angle difference
        s16 angleDiff = (s16)(targetAngle - m_aActualFacingDirection);
        int animID = ChooseRunAnim(angleDiff, mv3TargetPosition, 1.0f);

        // Check if we need to adjust the target angle
        s16 ballAngleDiff = (s16)(ballAngle - targetAngle);
        ballAngleDiff = ballAngleDiff < 0 ? -ballAngleDiff : ballAngleDiff;

        u16 absBallAngleDiff = (u16)ballAngleDiff;

        if ((absBallAngleDiff > 0x4000) && (animID != 8) && ((m_eAnimID == 8) || (m_eAnimID == 0x27)))
        {
            targetAngle += 0x8000;
        }

        if ((mfWaitTime > 0.25f) && (animID != 8))
        {
            PlayNewAnim(animID);
            InitMovementFromAnim(0, v3Zero, 1.0f, false);

            GoalieTweaks* pTweaks = static_cast<GoalieTweaks*>(m_pTweaks);
            u16 newFacing = SeekDirection(
                m_aActualFacingDirection,
                targetAngle,
                pTweaks->fRunningDirectionSeekSpeed,
                pTweaks->fRunningDirectionSeekFalloff,
                deltaTime);
            SetFacingDirection(newFacing);
            return;
        }

        if (CanInterceptPass())
        {
            if (mfWaitTime <= 0.02f)
            {
                InitActionPassInterceptSave();
                return;
            }

            mnSubstate = 1;
            PlayNewAnim(8);

            GoalieTweaks* pTweaks = static_cast<GoalieTweaks*>(m_pTweaks);
            InitMovementFromAnimSeek(pTweaks->fRunningDirectionSeekSpeed, pTweaks->fRunningDirectionSeekFalloff);
            return;
        }

        float tmp = GoalieSave::mfCrouchDuration;
        if ((g_pBall->m_tPassTargetTimer.GetSeconds() < tmp) && (IsCloseToPlane(mv3TargetPosition, m_v3Position, 1.2f)))
        {
            InitActionPreCrouch(GOALIECROUCH_PASS);
            return;
        }

        mUrgency = URGENCY_HIGH;
        InitActionMove(true);
    }
    }
}

/**
 * Offset/Address/Size: 0x135C | 0x8004F898 | size: 0x1F8
 */
void Goalie::ActionPreCrouch(float deltaTime)
{
    nlVector3 targetPos = g_pBall->m_v3Position;

    if (!CheckForSTSAttack())
    {
        if (g_pBall->GetOwnerFielder() != NULL)
        {
            cFielder* pOwnerFielder = g_pBall->GetOwnerFielder();
            if (IsOnSameTeam((cPlayer*)pOwnerFielder))
            {
                InitActionMove(false);
            }
            else if (IsWithinPounceRange())
            {
                InitActionPursueBallCarrier();
                InitActionPursueBallPounce();
            }
            else
            {
                if (pOwnerFielder->m_eActionState != ACTION_SHOT && pOwnerFielder->m_eActionState != ACTION_SHOOT_TO_SCORE)
                {
                    InitActionMove(true);
                }
                else if (mCrouchType != GOALIECROUCH_SHOT)
                {
                    InitActionMove(true);
                }
            }
        }
        else if (g_pBall->m_pPassTarget == NULL)
        {
            if (mpShooter == NULL || (mpShooter->m_eActionState != ACTION_LOOSE_BALL_SHOT) || (mCrouchType != GOALIECROUCH_LOOSEBALL))
            {
                InitActionMove(true);
            }
        }
        else
        {
            if (mCrouchType != GOALIECROUCH_PASS)
            {
                InitActionMove(true);
            }

            targetPos = g_pBall->m_pPassTarget->m_v3Position;
        }

        if (mGoalieActionState == GOALIEACTION_PRE_CROUCH)
        {
            float dy = targetPos.f.y - m_v3Position.f.y;
            float dx = targetPos.f.x - m_v3Position.f.x;
            float angle = nlATan2f(dy, dx);

            m_aDesiredFacingDirection = (u16)(s32)(10430.378f * angle);

            GoalieTweaks* pTweaks = static_cast<GoalieTweaks*>(m_pTweaks);
            u16 newFacing = SeekDirection(
                m_aActualFacingDirection,
                m_aDesiredFacingDirection,
                75000.0f,
                4000.0f,
                deltaTime);
            SetFacingDirection(newFacing);
        }
    }
}

/**
 * Offset/Address/Size: 0xF9C | 0x8004F4D8 | size: 0x3C0
 * TODO: 99.85% match - f10/f2 register swap for dx4 subtraction in dist4Sq CalculateDistanceSquared2D
 */
void Goalie::ActionPursueBallCarrier(float)
{
    if (!CheckForSTSAttack())
    {

        cFielder* pOwnerFielder = g_pBall->GetOwnerFielder();

        if (mnOffplayPending != 0 || pOwnerFielder == NULL || IsOnSameTeam((cPlayer*)pOwnerFielder) || !IsOpponentBallCarrierInRange())
        {
            InitActionMove(true);
            return;
        }

        nlVector3& ballPos = g_pBall->m_v3Position;
        GetLocalPoint(mv3LocalContactPosition, ballPos, m_v3Position, m_aActualFacingDirection);

        nlVector3 ballDelta;
        nlVec3Set(ballDelta,
            ballPos.f.x - m_v3Position.f.x,
            ballPos.f.y - m_v3Position.f.y,
            ballPos.f.z - m_v3Position.f.z);

        nlVector3 desiredPos;
        nlVector3 desiredDir;
        nlVector3 desiredOffset;
        unsigned short desiredAngle;
        FindDesiredGoaliePosition(desiredPos, desiredDir, desiredOffset, desiredAngle, NULL);

        float pickupDistance = mpLooseBallInfo->mfPickupDistance;
        float pounceRange = 0.5f * pickupDistance;
        float ballDistSq = ballDelta.GetLengthSq3D();
        float pickupDistSq = pickupDistance * pickupDistance;
        float pounceRangeSq = pounceRange * pounceRange;
        float thresholdDist;
        if (ballDistSq > pickupDistSq)
        {
            thresholdDist = pickupDistance;
        }
        else if (ballDistSq < pounceRangeSq)
        {
            thresholdDist = pounceRange;
        }
        else
        {
            thresholdDist = nlSqrt(ballDistSq, true);
        }

        float scale = -thresholdDist / nlSqrt(desiredDir.GetLengthSq3D(), true);

        nlVec3Set(desiredPos,
            (scale * desiredDir.f.x) + desiredOffset.f.x,
            (scale * desiredDir.f.y) + desiredOffset.f.y,
            (scale * desiredDir.f.z) + desiredOffset.f.z);

        nlVector3 moveDir;
        nlVec3Sub(moveDir, desiredPos, m_v3Position);

        float dotProduct = (moveDir.f.x * ballDelta.f.x) + (moveDir.f.y * ballDelta.f.y) + (moveDir.f.z * ballDelta.f.z);

        if (dotProduct > 0.0f)
        {
            ballDelta = moveDir;
        }

        float angle = nlATan2f(ballDelta.f.y, ballDelta.f.x);
        m_aDesiredFacingDirection = (u16)(s32)(10430.378f * angle);

        float pickupDistanceSq = mpLooseBallInfo->mfPickupDistance * mpLooseBallInfo->mfPickupDistance;

        nlVector3 opponentLocalPos;
        GetLocalPoint(opponentLocalPos, pOwnerFielder->m_v3Position, m_v3Position, m_aActualFacingDirection);

        float dist3x = mv3LocalContactPosition.f.x * mv3LocalContactPosition.f.x;
        float dist1Sq = CalculateDistanceSquared2D(mv3LocalContactPosition, mpLooseBallInfo->mv3PickupPos);
        float dist3y = mv3LocalContactPosition.f.y * mv3LocalContactPosition.f.y;
        float dist2Sq = CalculateDistanceSquared2D(opponentLocalPos, mpLooseBallInfo->mv3PickupPos);
        float dist3Sq = dist3x + dist3y;
        float dist4Sq = CalculateDistanceSquared2D(pOwnerFielder->m_v3Position, m_v3Position);

        if ((mv3LocalContactPosition.f.x < -0.35f) || (dist1Sq > 0.36f && dist2Sq > 0.36f && dist3Sq > pickupDistanceSq && dist4Sq > pickupDistanceSq))
        {
            s16 angleDiff = (s16)(m_aDesiredFacingDirection - m_aActualFacingDirection);
            int animID = ChooseRunAnim(angleDiff, ballPos, 1.0f);
            PlayNewAnim(animID);

            int opponentActionState = pOwnerFielder->m_eActionState;
            float speedScale = 1.5f;

            if (mbPlayMiss && (opponentActionState != 0xF) && (opponentActionState != 0x10))
            {
                speedScale = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide)->fGoalieDekeSpeed;
            }

            if (speedScale != m_pCurrentAnimController->m_fPlaybackSpeedScale)
            {
                m_pCurrentAnimController->m_fPlaybackSpeedScale = speedScale;
            }

            InitMovementFromAnimSeek(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff);
            return;
        }
        InitActionPursueBallPounce();
    }
}

static inline cPlayer* GetBallOwner(cBall* pBall, cBall** ppBall)
{
    *ppBall = pBall;
    return pBall->m_pOwner;
}

/**
 * Offset/Address/Size: 0xCCC | 0x8004F208 | size: 0x2D0
 */
void Goalie::ActionPursueBallPounce(float)
{
    float animTime = m_pCurrentAnimController->m_fTime;

    if (m_pBall == NULL)
    {
        if (animTime < 0.2f && CheckForSTSAttack())
        {
            PlayNewAnim(8);
            return;
        }

        cBall* pBall;
        cPlayer* pOwner = GetBallOwner(g_pBall, &pBall); // wtf..

        if (pOwner == NULL)
        {
            SetGoalieAction(GOALIEACTION_LOOSEBALL_PICKUP, 0);
            mfTargetTime = 0.0f;
            mfWaitTime = -1.0f;
            return;
        }

        if (IsOnSameTeam(pOwner) || !IsOpponentBallCarrierInRange())
        {
            InitActionMove(true);
            return;
        }

        if (CalculateDistanceSquared(GetJointPosition(m_nBallJointIndex), pBall->m_v3Position) < 0.16000001f
            || CalculateDistanceSquared(GetJointPosition(m_nLeftHandJointIndex), pBall->m_v3Position) < 0.16000001f
            || CalculateDistanceSquared(GetJointPosition(m_nRightHandJointIndex), pBall->m_v3Position) < 0.16000001f)
        {
            ExecutePounce(pOwner, true);
            return;
        }

        float pickupTime = mpLooseBallInfo->mfPickupTime;
        if ((animTime < pickupTime) && g_pBall->m_tShotTimer.m_uPackedTime == 0)
        {
            float ratio = animTime / pickupTime;
            float interpValue = ratio * (ratio * ((-2.0f * ratio) + 3.0f));

            if (interpValue < 0.99f && !mbPlayMiss)
            {
                TrackTarget(g_pBall->m_v3Position, interpValue);
            }
        }
    }

    if (animTime > 0.95f)
    {
        if (m_eAnimID != 0x7C)
        {
            if (m_pBall == NULL)
            {
                const GoalieTweaks* pTweaks = static_cast<const GoalieTweaks*>(m_pTweaks);
                if (mFatigue.mfEnergyLevel < pTweaks->fGetupEnergyHigh)
                {
                    float speed = InterpolateRangeClamped(
                        pTweaks->fGetupSpeedLow,
                        1.0f,
                        pTweaks->fGetupEnergyLow,
                        pTweaks->fGetupEnergyHigh,
                        mFatigue.mfEnergyLevel);
                    m_pCurrentAnimController->m_fPlaybackSpeedScale = speed;
                }

                if (ShouldStartCrossBlend(8))
                {
                    InitActionMove(false);
                    return;
                }
            }
            else
            {
                if (ShouldStartCrossBlend(9))
                {
                    InitActionMoveWB();
                    return;
                }
            }
        }
        else
        {
            InitActionPursueRecover();
        }
    }
}

/**
 * Offset/Address/Size: 0xA7C | 0x8004EFB8 | size: 0x250
 */
void Goalie::ActionOffplay(float)
{
    if (ShouldStartCrossBlend(0x90))
    {
        int animID;
        int currentAnimID = m_eAnimID;
        if (currentAnimID == 0x89 || currentAnimID == 0x8B || currentAnimID == 0x8D)
        {
            animID = 0x8D;
        }
        else if (currentAnimID == 0x88 || currentAnimID == 0x8A || currentAnimID == 0x8C)
        {
            animID = 0x8C;
        }
        else
        {
            static FilteredRandomRange randgenDejected;
            int index = randgenDejected.genrand(5);
            animID = gOffplayDejected[index];
        }

        SetAnimState(animID, true, 0.2f, false, false);
        InitMovementFromAnim(0, v3Zero, 1.0f, false);
    }

    nlVector3 pos = m_v3Position;
    float absX = (float)fabs(pos.f.x);
    float absY = (float)fabs(pos.f.y);

    float goalLineX = cField::GetGoalLineX(1U);
    float adjustedGoalLineX = goalLineX - 0.8f;

    if (absX > adjustedGoalLineX)
    {
        float halfNetWidth = 0.5f * cNet::m_fNetWidth;
        float netWidthAdjusted = halfNetWidth - 0.8f;

        if (absY < halfNetWidth)
        {
            if (absY > netWidthAdjusted)
            {
                if (absX < goalLineX)
                {
                    float newY;
                    float adjustedX = netWidthAdjusted + goalLineX;
                    float deltaX = adjustedX - absX;
                    if (pos.f.y > 0.0f)
                    {
                        newY = deltaX;
                    }
                    else
                    {
                        newY = -deltaX;
                    }
                    pos.f.y = newY;
                }
                else
                {
                    float newY;
                    if (pos.f.y > 0.0f)
                    {
                        newY = netWidthAdjusted;
                    }
                    else
                    {
                        newY = -netWidthAdjusted;
                    }
                    pos.f.y = newY;
                }

                SetPosition(pos);
                SetVelocity(v3Zero);
            }
        }
        else
        {
            float newX;
            if (pos.f.x > 0.0f)
            {
                newX = adjustedGoalLineX;
            }
            else
            {
                newX = -adjustedGoalLineX;
            }
            pos.f.x = newX;
            SetPosition(pos);
            SetVelocity(v3Zero);
        }
    }
}

/**
 * Offset/Address/Size: 0x860 | 0x8004ED9C | size: 0x21C
 * TODO: there are still some register swaps caused by CalculateDistanceSquared2D
 */
void Goalie::ActionLooseBallPursueBouncing(float deltaTime)
{
    if (IsPassThreat() || mnOffplayPending || !IsLooseBallClose(0.0f) || g_pBall->m_pOwner != NULL)
    {
        InitActionMove(true);
        return;
    }

    if (muBallChangeCount != g_pBall->m_bBallPathChangeCount)
    {
        InitActionLooseBallSetup();
        return;
    }

    mfTargetTime -= deltaTime;
    if ((mfTargetTime < 0.1f) || (g_pBall->m_v3Position.f.z < 1.0f && g_pBall->m_v3Velocity.f.z < 3.0f))
    {
        InitActionLooseBallSetup();
        return;
    }

    nlVector3 predictedPos;
    nlVector3 predictedVel;
    FakeBallWorld::GetPredictedBallPosition(mfTargetTime, predictedPos, predictedVel);

    if (CalculateDistanceSquared2D(m_v3Position, predictedPos) < mfTargetDist)
    {
        PlayNewAnim(8);
        InitMovementFromAnim(0, v3Zero, 1.0f, false);

        GetLocalPoint(mv3LocalContactPosition, predictedPos, m_v3Position, m_aActualFacingDirection);
        GetLocalPoint(mv3LocalContactVelocity, predictedVel, m_v3Position, m_aActualFacingDirection);

        InitActionLooseBallCatch();
        return;
    }

    // Calculate angle to predicted position and set facing direction
    float angle = nlATan2f(predictedPos.f.y - m_v3Position.f.y, predictedPos.f.x - m_v3Position.f.x);
    m_aDesiredFacingDirection = (u16)(s32)(10430.378f * angle);

    if (CalculateDistanceSquared(predictedPos, mv3TargetPosition) > mfTargetDist)
    {
        InitActionLooseBallSetup();
        return;
    }

    // Play animation if not already playing
    if (m_eAnimID != 0x26)
    {
        PlayNewAnim(0x26);
        InitMovementFromAnimSeek(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff);
    }
}

/**
 * Offset/Address/Size: 0x6CC | 0x8004EC08 | size: 0x194
 */
void Goalie::ActionSTSAttackSetup(float deltaTime)
{
    if (!IsOpponentInSTS())
    {
        InitActionMove(true);
        return;
    }

    mfWaitTime -= deltaTime;
    if (mfWaitTime <= 0.0)
    {
        InitActionSTSAttack();
        return;
    }

    cFielder* pOwnerFielder = g_pBall->GetOwnerFielder();

    float dx = m_v3Position.f.x - pOwnerFielder->m_v3Position.f.x;
    float dy = m_v3Position.f.y - pOwnerFielder->m_v3Position.f.y;
    float distSq = dx * dx + dy * dy;
    float pickupDistSq = mpLooseBallInfo->mfPickupDistance * mpLooseBallInfo->mfPickupDistance;

    int animID = 8;
    if (distSq > pickupDistSq)
    {
        animID = 0x1A;
    }

    PlayNewAnim(animID);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    GetLocalPoint(mv3LocalContactPosition, pOwnerFielder->m_v3Position, m_v3Position, m_aActualFacingDirection);

    float angle = nlATan2f(mv3LocalContactPosition.f.y, mv3LocalContactPosition.f.x);
    float progressRatio = (mfTargetTime - mfWaitTime) / mfTargetTime;

    s16 angleDeltaInt = (s16)(u16)(s32)(10430.378f * angle);
    s32 multiplierInt = (s32)(1024.0f * (progressRatio * (progressRatio * ((-2.0f * progressRatio) + 3.0f))));
    s32 adjustedDelta = (angleDeltaInt * multiplierInt) / 1024;

    u16 newFacing = adjustedDelta + m_aActualFacingDirection;

    SetFacingDirection(newFacing);
    m_aDesiredFacingDirection = newFacing;
}

/**
 * Offset/Address/Size: 0x2B8 | 0x8004E7F4 | size: 0x414
 * TODO: 99.80% match - remaining diff: 0.306f compare/subtract f0/f2 register swap
 *   at offset 0x10c (context-dependent MWCC register allocation quirk, not fixable at source level)
 */
void Goalie::ActionSTSAttack(float deltaTime)
{
    float animTime = m_pCurrentAnimController->m_fTime;

    if (IsOpponentInSTS())
    {
        if (animTime < mpLooseBallInfo->mfPickupTime)
        {
            u16 actualFacing = m_aActualFacingDirection;
            GetLocalPoint(mv3LocalContactPosition, mpShooter->m_v3Position, m_v3Position, actualFacing);

            if (animTime > 0.1f)
            {
                float t = (animTime - 0.1f) / (mpLooseBallInfo->mfPickupTime - 0.1f);
                float angle = nlATan2f(mv3LocalContactPosition.f.y, mv3LocalContactPosition.f.x);
                u16 aNewAng = (u16)(actualFacing + ((s32)(1024.0f * (t * (t * ((-2.0f * t) + 3.0f)))) * (s16)(u16)(s32)(10430.378f * angle)) / 1024);

                SetFacingDirection(aNewAng);
                m_aDesiredFacingDirection = aNewAng;
            }

            if (animTime > 0.306f)
            {
                float stepScale = deltaTime / (mpLooseBallInfo->mfAnimDuration * (mpLooseBallInfo->mfPickupTime - 0.306f));
                nlVector3 movement = { 0.0f, 0.0f, 0.0f };

                movement.f.x = mfTargetDist;
                RotateVectorZAxis(movement, movement, actualFacing);

                float newX = (stepScale * movement.f.x) + m_v3Position.f.x;
                float newZ = (stepScale * movement.f.z) + m_v3Position.f.z;
                float newY = (stepScale * movement.f.y) + m_v3Position.f.y;
                movement.f.x = newX;
                movement.f.y = newY;
                movement.f.z = newZ;
                SetPosition(movement);
            }
        }

        if (animTime > 0.306f)
        {
            nlVector3 rightFootPos = GetJointPosition(m_nRightFootJointIndex);
            float dx = rightFootPos.f.x - mpShooter->m_v3Position.f.x;
            float dy = rightFootPos.f.y - mpShooter->m_v3Position.f.y;

            if (((dx * dx) + (dy * dy)) < 1.0f)
            {
                WhackSTSPlayer(mpShooter);
            }
        }
    }
    else
    {
        if (animTime <= 0.1f)
        {
            InitActionMove(true);
            return;
        }
    }

    if (animTime > 0.95f)
    {
        InitActionMove(true);
        return;
    }

    if ((mpShooter->m_eAnimID == 0x73) && (animTime < 0.45f))
    {
        cFielder* pShooter = mpShooter;
        nlVector3 rightFootPos = GetJointPosition(m_nRightFootJointIndex);
        float dx = rightFootPos.f.x - pShooter->m_v3Position.f.x;
        float dy = rightFootPos.f.y - pShooter->m_v3Position.f.y;
        float pushDist = 1.0f - nlSqrt((dx * dx) + (dy * dy), true);

        if (pushDist > 0.0f)
        {
            nlVector3 pushVec = { 0.0f, 0.0f, 0.0f };
            pushVec.f.x = pushDist;

            RotateVectorZAxis(pushVec, pushVec, m_aActualFacingDirection);

            float radius;
            mpShooter->m_pPhysicsCharacter->GetRadius(&radius);
            radius += 0.2f;

            if ((float)fabs(pShooter->m_v3Position.f.x) > (cField::GetGoalLineX(1U) - radius))
            {
                float posX = m_v3Position.f.x - pushVec.f.x;
                float posZ = m_v3Position.f.z - pushVec.f.z;
                float posY = m_v3Position.f.y - pushVec.f.y;
                pushVec.f.x = posX;
                pushVec.f.y = posY;
                pushVec.f.z = posZ;
                SetPosition(pushVec);
            }
            else
            {
                float posX = pushVec.f.x + pShooter->m_v3Position.f.x;
                float posZ = pushVec.f.z + pShooter->m_v3Position.f.z;
                float posY = pushVec.f.y + pShooter->m_v3Position.f.y;
                pushVec.f.x = posX;
                pushVec.f.y = posY;
                pushVec.f.z = posZ;
                mpShooter->SetPosition(pushVec);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x168 | 0x8004E6A4 | size: 0x150
 */
void Goalie::ActionSnapBall(float)
{
    unsigned short aRootRot;
    float fTimeLeft;
    nlVector3 v3TargetPos;
    nlVector3 v3RootPos;

    if (g_pBall->m_pOwner != this)
    {
        TacklePlayer(g_pBall->m_pOwner);

        fTimeLeft = m_tNoPickupTimer.GetSeconds();

        if (fTimeLeft > 0.0f)
        {
            GetCurrentAnimFuture(m_nBallJointIndex, m_pCurrentAnimController->m_fTime, v3TargetPos, v3RootPos, aRootRot);

            float invInterpFactor;
            float interpFactor;

            interpFactor = (1.0f / mfWaitTime) * (mfWaitTime - fTimeLeft);
            invInterpFactor = 1.0f - interpFactor;

            v3TargetPos.f.x = (invInterpFactor * g_pBall->m_v3Position.f.x) + (interpFactor * v3TargetPos.f.x);
            v3TargetPos.f.y = (invInterpFactor * g_pBall->m_v3Position.f.y) + (interpFactor * v3TargetPos.f.y);
            v3TargetPos.f.z = (invInterpFactor * g_pBall->m_v3Position.f.z) + (interpFactor * v3TargetPos.f.z);

            g_pBall->SetPosition(v3TargetPos);
            return;
        }
        PickupBall(g_pBall);
        mbPickedUp = true;
        return;
    }

    if (m_pBall == NULL)
    {
        InitActionMove(true);
        return;
    }

    bool shouldMoveWB = false;
    if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
    {
        shouldMoveWB = true;
    }

    if (shouldMoveWB)
    {
        InitActionMoveWB();
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8004E53C | size: 0x168
 */
void Goalie::ActionGrabBall(float)
{
    bool bShouldInitMove = false;
    if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 0.0f)
    {
        bShouldInitMove = true;
    }

    if (bShouldInitMove)
    {
        if (m_pBall == NULL)
        {
            InitActionMove(true);
            return;
        }
        InitActionMoveWB();
        return;
    }

    if (g_pBall->m_pOwner != this)
    {
        if (g_pBall->GetOwnerFielder() == NULL)
        {
            InitActionMove(false);
            return;
        }

        float fTimeThreshold = 0.1f + mpLooseBallInfo->mfPickupTime;
        if (m_pCurrentAnimController->m_fTime < fTimeThreshold)
        {
            float fInterpFactor = m_pCurrentAnimController->m_fTime / fTimeThreshold;
            TrackTarget(g_pBall->m_v3Position, fInterpFactor);

            const nlVector3& jointPos = GetJointPosition(m_nBallJointIndex);

            nlVector3 delta;
            nlVec3Set(delta, g_pBall->m_v3Position.f.x - jointPos.f.x, g_pBall->m_v3Position.f.y - jointPos.f.y, g_pBall->m_v3Position.f.z - jointPos.f.z);

            if (nlGetLengthSquared3D(delta.f.x, delta.f.y, delta.f.z) < 0.25f)
            {
                StealBall(g_pBall->m_pOwner);
                PickupBall(g_pBall);
                mbPickedUp = true;
                g_pBall->ClearShotInProgress();
                EmitGoalieCatch(this, "goalie_catch", false);
            }
        }
    }
}

void GoalieActions_stub(float& a, float& b, float& c, float& d)
{
    a = 1.0f;  // @1577
    b = 0.1f;  // @1578
    c = 0.25f; // @1579
    d = 0.0f;  // @1616
}
