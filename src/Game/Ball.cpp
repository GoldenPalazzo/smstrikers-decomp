#include "Game/WorldManager.h"
#include "NL/platqmath.h"
#pragma pool_data off

#include "Game/Ball.h"
#include "Game/CharacterTriggers.h"
#include "Game/Drawable/DrawableObj.h"
#include "Game/EventDataTypes.h"
#include "Game/ObjectBlur.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/FuzzyVariant.h"

#include "NL/nlDebug.h"
#include "NL/nlMath.h"
#include "NL/nlMemory.h"

#include "Game/Goalie.h"

#include "Game/Physics/PhysicsAIBall.h"
#include "Game/Physics/PhysicsObject.h"
#include "Game/Physics/RayCollider.h"
#include "Game/Physics/PhysicsFakeBall.h"

#include "Game/FixedUpdateTask.h"
#include "Game/ParticleUpdateTask.h"

#include "Game/Game.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/AI/AiUtil.h"
#include "Game/FE/feHelpFuncs.h"
extern float g_BallAirResistance;

static f32 CANT_COLLIDE = *(f32*)__float_max;
cBall* g_pBall = NULL;
static float gfPerfectPassSFXVol;
static bool gbCanFadeOutPerfectPassSFX = true;

nlMatrix3 m3Ident = { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f };
// nlMatrix4 m4Ident = { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };

/**
 * Offset/Address/Size: 0x0 | 0x800099D4 | size: 0x10C
 */
float cBall::PredictLandingSpotAndTime(nlVector3& v3Dest)
{
    float fTime = 0.0f;

    if (m_v3Position.f.z > 1.0f)
    {
        int numSolutions;
        float times[2];

        SolveQuadratic(0.5f * m_pPhysicsBall->m_gravity, m_v3Velocity.f.z, m_v3Position.f.z, numSolutions, times[0], times[1]);

        float t = 100000000.0f;
        float* root = times;
        for (int i = numSolutions; i > 0; i--)
        {
            if (*root >= 0.0f)
            {
                t = (t <= *root) ? t : *root;
            }
            root++;
        }

        fTime = t;
        float x = t * m_v3Velocity.f.x + m_v3Position.f.x;
        float z = t * m_v3Velocity.f.z + m_v3Position.f.z;
        float y = t * m_v3Velocity.f.y + m_v3Position.f.y;
        v3Dest.f.x = x;
        v3Dest.f.y = y;
        v3Dest.f.z = z;
        v3Dest.f.z = 0.0f;
    }
    else
    {
        v3Dest = m_v3Position;
    }

    return fTime;
}

/**
 * Offset/Address/Size: 0x10C | 0x80009AE0 | size: 0x44
 */
void cBall::KillBlurHandler()
{
    if (m_pBlurHandler != NULL)
    {
        m_pBlurHandler->Die(0.f);
        m_pBlurHandler = NULL;
    }
}

/**
 * Offset/Address/Size: 0x150 | 0x80009B24 | size: 0x1EC
 */
void cBall::ClearPassTarget()
{
    if (mbHyperSTS)
    {
        Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
    }
    Audio::FadeFilterFromCurrentToZero();

    FixedUpdateTask::mTimeScale = 1.0f;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (m_pBlurHandler != 0)
    {
        m_pBlurHandler->Die(0.5f);
        m_pBlurHandler = 0;
    }

    KillBallShot("ball_shot_perfect_glow", true);
    KillBallShot("ball_pass_perfect_glow", true);
    KillBallShot("shoot_to_score_shot", false);
    KillBallShot("ball_shot_onetimer", false);

    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

    if (mbHyperSTS)
    {
        void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
        PassBallData* eventdata = new (data) PassBallData();
        eventdata->pPasser = m_pPrevOwner;
        eventdata->pTarget = NULL;

        bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
        eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
    }

    mbHyperSTS = false;
    mbIsPerfectShot = false;

    gbCanFadeOutPerfectPassSFX = true;

    if (AudioLoader::IsInited())
    {
        gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
    }

    if (m_pPassTarget)
    {
        m_pPassTarget = NULL;
    }

    m_v3PassIntercept.f.x = 0.f;
    m_v3PassIntercept.f.y = 0.f;
    m_v3PassIntercept.f.z = 0.f;

    m_tPassTargetTimer.m_uPackedTime = 0;

    if (m_uVoiceID)
    {
        Audio::StopSFX(m_uVoiceID);
        m_uVoiceID = 0;
    }
}

/**
 * Offset/Address/Size: 0x33C | 0x80009D10 | size: 0x40
 */
void cBall::SetPassTargetTimer(float seconds)
{
    m_tPassTargetTimer.SetSeconds(seconds);
    m_fTotalPassTime = seconds;
}

/**
 * Offset/Address/Size: 0x37C | 0x80009D50 | size: 0x20
 */
void cBall::SetPassTarget(cPlayer* passTargetPlayer, const nlVector3& pos, bool)
{
    m_pPassTarget = passTargetPlayer;
    m_v3PassIntercept = pos;
}

/**
 * Offset/Address/Size: 0x39C | 0x80009D70 | size: 0x90
 */
void cBall::WarpTo(const nlVector3& toPos)
{
    m_v3Position = toPos;
    m_pPhysicsBall->SetPosition(toPos, PhysicsObject::WORLD_COORDINATES);
    m_pPhysicsBall->SetRotation(m3Ident);
    FakeBallWorld::InvalidateBallCache();
    m_bBallPathChangeCount = m_bBallPathChangeCount + 1;
    m_v3PrevPosition = toPos;
}

/**
 * Offset/Address/Size: 0x42C | 0x80009E00 | size: 0x30C
 * TODO: 99.23% match - FPR register allocation
 *       (f8/f5/f6 vs f5/f6/f8 for normalized direction + zero constant)
 *       in both velocity cross-product blocks.
 */
void cBall::UpdateOrientation(float fDeltaT)
{
    nlQuaternion qOrientationDelta;
    nlVector3 v3AngVel;
    float fInvAng;
    nlQuaternion qNewOrientation;

    if (m_pOwner == NULL)
    {
        u8 bUseAngularVel = 0;
        if (m_pPhysicsBall->m_bUseAngularVel != 0 || m_pPhysicsBall->m_fSpinTimer > 0.0f)
        {
            bUseAngularVel = 1;
        }

        if (bUseAngularVel != 0)
        {
            m_pPhysicsBall->GetAngularVelocity(&v3AngVel);

            float fAng = nlSqrt(v3AngVel.f.x * v3AngVel.f.x + v3AngVel.f.y * v3AngVel.f.y + v3AngVel.f.z * v3AngVel.f.z, true);
            if (fAng > 0.01f)
            {
                fInvAng = 1.0f / fAng;
                nlVec3Scale(v3AngVel, fInvAng);
                nlMakeQuat(qOrientationDelta, v3AngVel, fAng * fDeltaT);
            }
            else
            {
                qOrientationDelta.f.z = 0.0f;
                qOrientationDelta.f.y = 0.0f;
                qOrientationDelta.f.x = 0.0f;
                qOrientationDelta.f.w = 1.0f;
            }
        }
        else
        {
            qOrientationDelta.f.z = 0.0f;
            qOrientationDelta.f.y = 0.0f;
            qOrientationDelta.f.x = 0.0f;
            qOrientationDelta.f.w = 1.0f;

            float fVel = nlSqrt(m_v3Velocity.f.x * m_v3Velocity.f.x + m_v3Velocity.f.y * m_v3Velocity.f.y + m_v3Velocity.f.z * m_v3Velocity.f.z, true);
            if (fVel > 0.0001f)
            {
                nlVector3 v3Axis;
                nlVector3 v3Direction = m_v3Velocity;
                float fZero = 0.0f;
                float fOne = 1.0f;

                float fDirX = v3Direction.f.x / fVel;
                float fDirY = v3Direction.f.y / fVel;
                float fDirZ = v3Direction.f.z / fVel;

                v3Direction.f.x = fDirX;
                v3Direction.f.y = fDirY;
                v3Direction.f.z = fDirZ;

                v3Axis.f.x = fZero * v3Direction.f.z - fOne * v3Direction.f.y;
                v3Axis.f.y = -fZero * v3Direction.f.z + fOne * v3Direction.f.x;
                v3Axis.f.z = fZero * v3Direction.f.y - fZero * v3Direction.f.x;

                nlMakeQuat(qOrientationDelta, v3Axis, fDeltaT * (fVel / 0.18f));
            }
        }
    }
    else
    {
        m_pPhysicsBall->SetUseAngularVelocity(false);

        switch (m_pOwner->m_eBallRotationMode)
        {
        case BRM_ANIMATED:
            m_pOwner->GetAnimatedBallOrientation(m_qOrientation);
            return;
        case BRM_MATCH_VELOCITY:
        {
            qOrientationDelta.f.z = 0.0f;
            qOrientationDelta.f.y = 0.0f;
            qOrientationDelta.f.x = 0.0f;
            qOrientationDelta.f.w = 1.0f;

            float fVel = nlSqrt(m_v3Velocity.f.x * m_v3Velocity.f.x + m_v3Velocity.f.y * m_v3Velocity.f.y + m_v3Velocity.f.z * m_v3Velocity.f.z, true);
            if (fVel > 0.0001f)
            {
                nlVector3 v3Axis;
                nlVector3 v3Direction = m_v3Velocity;
                float fZero = 0.0f;
                float fOne = 1.0f;

                float fDirX = v3Direction.f.x / fVel;
                float fDirY = v3Direction.f.y / fVel;
                float fDirZ = v3Direction.f.z / fVel;

                v3Direction.f.x = fDirX;
                v3Direction.f.y = fDirY;
                v3Direction.f.z = fDirZ;

                v3Axis.f.x = fZero * v3Direction.f.z - fOne * v3Direction.f.y;
                v3Axis.f.y = -fZero * v3Direction.f.z + fOne * v3Direction.f.x;
                v3Axis.f.z = fZero * v3Direction.f.y - fZero * v3Direction.f.x;

                nlMakeQuat(qOrientationDelta, v3Axis, fDeltaT * (fVel / 0.18f));
            }
            break;
        }
        }
    }

    nlMultQuat(qNewOrientation, qOrientationDelta, m_qOrientation);
    nlQuatNormalize(m_qOrientation, qNewOrientation);
}

/**
 * Offset/Address/Size: 0x738 | 0x8000A10C | size: 0x308
 */
void cBall::Update(float fDeltaT)
{
    bool bIsGameplay = false;

    if (g_pGame->m_eGameState == GS_GAMEPLAY || g_pGame->m_eGameState == GS_OVERTIME)
    {
        bIsGameplay = true;
    }

    if (bIsGameplay)
    {
        m_tNoPickupTimer.Countdown(fDeltaT, 0.0f);

        if (m_tShotTimer.m_uPackedTime != 0)
        {
            if (m_tShotTimer.Countdown(fDeltaT, 0.0f))
            {
                float fGameDuration = g_pGame->m_fGameDuration;
                if (g_pGame->GetGameTime() >= fGameDuration
                    && g_pGame->m_eGameState == GS_GAMEPLAY
                    && m_tBuzzerBeaterTimer.m_uPackedTime == 0)
                {
                    m_tBuzzerBeaterTimer.SetSeconds(0.0f);
                }

                m_tShotTimer.m_uPackedTime = 0;
                mbCanDamage = false;
                m_unk_0xA4 = false;

                if (mbHyperSTS)
                {
                    Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
                }

                Audio::FadeFilterFromCurrentToZero();

                FixedUpdateTask::mTimeScale = 1.0f;
                ParticleUpdateTask::SetTimeScale(1.0f);

                if (m_pBlurHandler != 0)
                {
                    m_pBlurHandler->Die(0.0f);
                    m_pBlurHandler = 0;
                }

                KillBallShot("ball_shot_perfect_glow", true);
                KillBallShot("ball_pass_perfect_glow", true);
                KillBallShot("shoot_to_score_shot", false);
                KillBallShot("ball_shot_onetimer", false);

                Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
                Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
                Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

                if (mbHyperSTS)
                {
                    void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
                    PassBallData* eventdata = new (data) PassBallData();
                    eventdata->pPasser = m_pPrevOwner;
                    eventdata->pTarget = NULL;

                    bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
                    eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
                }

                mbHyperSTS = false;
                mbIsPerfectShot = false;

                gbCanFadeOutPerfectPassSFX = true;

                if (AudioLoader::IsInited())
                {
                    gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
                }
            }
        }

        if (m_tPassTargetTimer.m_uPackedTime != 0)
        {
            if (m_tPassTargetTimer.Countdown(fDeltaT, 0.0f))
            {
                m_fTotalPassTime = 0.0f;
            }
        }

        if (m_tBuzzerBeaterTimer.m_uPackedTime != 0)
        {
            m_tBuzzerBeaterTimer.Countdown(fDeltaT, 0.0f);
        }
    }

    if (m_pPassTarget != NULL && mbHyperSTS)
    {
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        if (m_tPassTargetTimer.GetSeconds() < pTweaks->unk23C)
        {
            if (gbCanFadeOutPerfectPassSFX)
            {
                gbCanFadeOutPerfectPassSFX = false;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xA40 | 0x8000A414 | size: 0x100
 */
void cBall::ShootAtFast(nlVector3& v3Vel, const nlVector3& v3Target, float fDesiredTime)
{
    float k = g_BallAirResistance;
    float g = 1.025f * m_pPhysicsBall->m_gravity;
    float eToTheNegativeKT = Exp(-k * fDesiredTime);
    float kSquaredOverOneMinusEToTheNegativeKT = (k * k) / (1.0f - eToTheNegativeKT);
    float oneOverK = 1.0f / k;

    v3Vel.f.x = kSquaredOverOneMinusEToTheNegativeKT * (oneOverK * (v3Target.f.x - m_v3Position.f.x));
    v3Vel.f.y = kSquaredOverOneMinusEToTheNegativeKT * (oneOverK * (v3Target.f.y - m_v3Position.f.y));
    v3Vel.f.z = kSquaredOverOneMinusEToTheNegativeKT * (oneOverK * (v3Target.f.z - m_v3Position.f.z - g * fDesiredTime / k)) + g / k;
}

static inline float CalcSpinRand(eSpinType spin)
{
    float fSpinRand = 0.5f + nlRandomf(2.0f, &nlDefaultSeed);
    if (spin == SPINTYPE_BACK)
    {
        fSpinRand *= -1.0f;
    }
    return fSpinRand;
}

/**
 * Offset/Address/Size: 0xB40 | 0x8000A514 | size: 0x1EC
 */
void cBall::ShootRelease(const nlVector3& v3Velocity, eSpinType SpinType)
{
    nlVector3 v3Up;
    nlVector3 v3AngVel;

    m_v3Velocity = v3Velocity;
    m_pPhysicsBall->SetLinearVelocity(v3Velocity);

    if (SpinType == SPINTYPE_NONE)
    {
        v3AngVel.f.x = 0.0f;
        v3AngVel.f.y = 0.0f;
        v3AngVel.f.z = 0.0f;
    }
    else if ((SpinType == SPINTYPE_FORWARD) || (SpinType == SPINTYPE_BACK))
    {
        float fSpinRand = CalcSpinRand(SpinType);

        static const nlVector3 kZero = { 0.0f, 0.0f, 0.0f };
        v3Up = kZero;
        v3Up.f.z = fSpinRand;

        nlVector3 v3Cross;
        nlVec3CrossProductAlt(v3Cross, v3Up, v3Velocity);
        v3AngVel.f.x = v3Cross.f.z;
        v3AngVel.f.y = v3Cross.f.y;
        v3AngVel.f.z = v3Cross.f.x;
    }
    else if (SpinType == SPINTYPE_ROLLING)
    {
        m_pPhysicsBall->CalcAngularFromLinearVelocity(v3AngVel);
        nlVec3Set(v3AngVel, 0.92f * v3AngVel.f.x, 0.92f * v3AngVel.f.y, 0.92f * v3AngVel.f.z);
    }
    else if (SpinType == SPINTYPE_PARAMETER)
    {
        v3AngVel = *(const nlVector3*)NULL;
    }

    m_pPhysicsBall->SetAngularVelocity(v3AngVel);
    m_pPhysicsBall->SetUseAngularVelocity(true);
    m_pPhysicsBall->SetRotation(m3Ident);
    FakeBallWorld::InvalidateBallCache();
    m_bBallPathChangeCount = m_bBallPathChangeCount + 1;
    m_v3ShotOrigin = m_v3Position;
    m_tNoPickupTimer.SetSeconds(0.1f);
    m_pPhysicsBall->m_bUseMagnusEffect = false;
    m_unk_0xA6 = false;
    mpDamageTarget = NULL;
    m_unk_0xA3 = false;
}

/**
 * Offset/Address/Size: 0xD2C | 0x8000A700 | size: 0x3AC
 */
void cBall::Shoot(const nlVector3& v3Dir, const nlVector3& v3Spin, eSpinType spinType, bool bCanDamage, bool bParam5, bool bParam6)
{
    nlVector3 v3PredPos;
    nlVector3 v3PredVel;
    nlVector3 v3ToDir;
    nlVector3 v3FromDir;
    nlQuaternion qRot;
    nlVector3 v3Up;
    nlVector3 v3AngVel;

    Goalie* pGoalie = m_pPrevOwner->m_pTeam->GetOtherTeam()->GetGoalie();

    m_v3Velocity = v3Dir;
    m_pPhysicsBall->SetLinearVelocity(v3Dir);

    if (spinType == SPINTYPE_NONE)
    {
        v3AngVel.f.x = 0.0f;
        v3AngVel.f.y = 0.0f;
        v3AngVel.f.z = 0.0f;
    }
    else if ((spinType == SPINTYPE_FORWARD) || (spinType == SPINTYPE_BACK))
    {
        float fSpinRand = CalcSpinRand(spinType);

        static const nlVector3 kZero = { 0.0f, 0.0f, 0.0f };
        v3Up = kZero;
        v3Up.f.z = fSpinRand;

        nlVector3 v3Cross;
        nlVec3CrossProductAlt(v3Cross, v3Up, v3Dir);
        v3AngVel.f.x = v3Cross.f.z;
        v3AngVel.f.y = v3Cross.f.y;
        v3AngVel.f.z = v3Cross.f.x;
    }
    else if (spinType == SPINTYPE_ROLLING)
    {
        m_pPhysicsBall->CalcAngularFromLinearVelocity(v3AngVel);
        nlVec3Set(v3AngVel, 0.92f * v3AngVel.f.x, 0.92f * v3AngVel.f.y, 0.92f * v3AngVel.f.z);
    }
    else if (spinType == SPINTYPE_PARAMETER)
    {
        v3AngVel = v3Spin;
    }

    m_pPhysicsBall->SetAngularVelocity(v3AngVel);
    m_pPhysicsBall->SetUseAngularVelocity(true);
    m_pPhysicsBall->SetRotation(m3Ident);
    FakeBallWorld::InvalidateBallCache();

    m_bBallPathChangeCount = m_bBallPathChangeCount + 1;
    m_v3ShotOrigin = m_v3Position;
    m_tNoPickupTimer.SetSeconds(0.1f);
    m_tBuzzerBeaterTimer.SetSeconds(0.0f);
    m_tShotTimer.SetSeconds(1.5f);

    m_unk_0xA3 = bParam6;
    mbCanDamage = bCanDamage;
    if (!m_unk_0xA3)
    {
        m_unk_0xA4 = bParam5;
    }
    else
    {
        m_unk_0xA4 = false;
    }

    m_pShooter = NULL;

    if (m_pPhysicsBall->m_bUseMagnusEffect)
    {
        float fDist = nlSqrt(nlGetLengthSquared3D(m_v3Position.f.x - m_v3ShotTarget.f.x, m_v3Position.f.y - m_v3ShotTarget.f.y, m_v3Position.f.z - m_v3ShotTarget.f.z), true);

        FakeBallWorld::GetPredictedPosAtDistance(fDist, v3PredPos, v3PredVel);

        nlVec3Sub(v3ToDir, m_v3ShotTarget, m_v3Position);
        nlVec3Sub(v3FromDir, v3PredPos, m_v3Position);

        GetRotationBetweenVectors(qRot, v3FromDir, v3ToDir);
        RotateVector(m_v3Velocity, v3Dir, qRot);

        if (m_v3Velocity.f.z < 1.0f && m_v3Position.f.z < 1.0f)
        {
            m_v3Velocity.f.z = 1.0f;
        }

        float fSidelineY = cField::GetSidelineY(1) - 0.5f;
        if (m_v3Position.f.y > fSidelineY && m_v3Velocity.f.y > -0.1f)
        {
            m_v3Velocity.f.y = -0.1f;
        }
        else if (m_v3Position.f.y < 0.5f - cField::GetSidelineY(1) && m_v3Velocity.f.y < 0.1f)
        {
            m_v3Velocity.f.y = 0.1f;
        }

        m_pPhysicsBall->SetLinearVelocity(m_v3Velocity);
        FakeBallWorld::InvalidateBallCache();
    }

    if (bCanDamage)
    {
        pGoalie->InitActionSTSSetup();
    }
    else
    {
        pGoalie->InitActionSaveSetup(true);
    }
}

/**
 * Offset/Address/Size: 0x10D8 | 0x8000AAAC | size: 0x2C
 */
void cBall::SetVisible(bool visible)
{
    DrawableObject* drawable = m_pDrawableBall;
    if (visible != 0)
    {
        drawable->m_uObjectFlags = (drawable->m_uObjectFlags | 1);
        return;
    }
    drawable->m_uObjectFlags = (drawable->m_uObjectFlags & 0xFFFFFFFE);
}

/**
 * Offset/Address/Size: 0x1104 | 0x8000AAD8 | size: 0x1D4
 * TODO: 99.49% match - spin branch still uses different float registers for spin and velocity temporaries.
 */
void cBall::SetVelocity(const nlVector3& velocity, eSpinType spin, const nlVector3* pAngularVelocity)
{
    nlVector3 v3AngVel;

    m_v3Velocity = velocity;
    m_pPhysicsBall->SetLinearVelocity(velocity);

    if (spin == SPINTYPE_NONE)
    {
        v3AngVel.f.x = 0.0f;
        v3AngVel.f.y = 0.0f;
        v3AngVel.f.z = 0.0f;
    }
    else if ((spin == SPINTYPE_FORWARD) || (spin == SPINTYPE_BACK))
    {
        float fSpinRand = CalcSpinRand(spin);

        nlVector3 v3Up = { 0.0f, 0.0f, 0.0f };
        v3Up.f.z = fSpinRand;

        float velY = velocity.f.y;
        float velX = velocity.f.x;

        v3AngVel.f.x = (v3Up.f.y * velocity.f.z) - (v3Up.f.z * velY);
        v3AngVel.f.y = (-v3Up.f.x * velocity.f.z) + (v3Up.f.z * velX);
        v3AngVel.f.z = (velY * v3Up.f.x) - (v3Up.f.y * velX);
    }
    else if (spin == SPINTYPE_ROLLING)
    {
        m_pPhysicsBall->CalcAngularFromLinearVelocity(v3AngVel);
        nlVec3Set(v3AngVel, 0.92f * v3AngVel.f.x, 0.92f * v3AngVel.f.y, 0.92f * v3AngVel.f.z);
    }
    else if (spin == SPINTYPE_PARAMETER)
    {
        v3AngVel = *pAngularVelocity;
    }

    m_pPhysicsBall->SetAngularVelocity(v3AngVel);
    m_pPhysicsBall->SetUseAngularVelocity(true);
    m_pPhysicsBall->SetRotation(m3Ident);
    FakeBallWorld::InvalidateBallCache();
    m_bBallPathChangeCount = m_bBallPathChangeCount + 1;
    m_v3ShotOrigin = m_v3Position;
}

/**
 * Offset/Address/Size: 0x12D8 | 0x8000ACAC | size: 0xF0
 */
void cBall::SetPerfectPass(bool bFlag, bool bNoEvent)
{
    PassBallData* eventdata;

    if ((mbHyperSTS != bFlag) && !bNoEvent)
    {
        EventManager* mgr = g_pEventManager;
        void* data = (u8*)mgr->CreateValidEvent(bFlag ? 0x45 : 0x47, 0x24) + 0x10;
        eventdata = new (data) PassBallData();

        if (bFlag)
        {
            eventdata->pPasser = m_pPrevOwner;
            eventdata->pTarget = m_pPassTarget;
        }
        else
        {
            eventdata->pPasser = m_pPrevOwner;
            eventdata->pTarget = NULL;
        }

        bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
        eventdata->mPasserControllerID = (pad != NULL) ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
    }

    mbHyperSTS = bFlag;
}

/**
 * Offset/Address/Size: 0x13C8 | 0x8000AD9C | size: 0x6C
 */
void cBall::SetPosition(const nlVector3& pos)
{
    m_v3Position = pos;
    m_pPhysicsBall->SetPosition(pos, PhysicsObject::WORLD_COORDINATES);
    m_pPhysicsBall->SetRotation(m3Ident);
    FakeBallWorld::InvalidateBallCache();
    m_bBallPathChangeCount++;
}

/**
 * Offset/Address/Size: 0x1434 | 0x8000AE08 | size: 0x230
 */
void cBall::SetOwner(cPlayer* pOwner)
{
    m_pOwner = pOwner;
    m_pLastTouch = pOwner;

    if (mbHyperSTS)
    {
        Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
    }

    Audio::FadeFilterFromCurrentToZero();

    FixedUpdateTask::mTimeScale = 1.0f;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (m_pBlurHandler != 0)
    {
        m_pBlurHandler->Die(0.5f);
        m_pBlurHandler = 0;
    }

    KillBallShot("ball_shot_perfect_glow", true);
    KillBallShot("ball_pass_perfect_glow", true);
    KillBallShot("shoot_to_score_shot", false);
    KillBallShot("ball_shot_onetimer", false);

    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

    if (mbHyperSTS)
    {
        void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
        PassBallData* eventdata = new (data) PassBallData();
        eventdata->pPasser = m_pPrevOwner;
        eventdata->pTarget = NULL;

        bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
        eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
    }

    mbHyperSTS = false;
    mbIsPerfectShot = false;

    gbCanFadeOutPerfectPassSFX = true;

    if (AudioLoader::IsInited())
    {
        gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
    }

    if (m_pPassTarget)
    {
        m_pPassTarget = NULL;
    }

    m_v3PassIntercept.f.x = 0.f;
    m_v3PassIntercept.f.y = 0.f;
    m_v3PassIntercept.f.z = 0.f;

    m_tPassTargetTimer.m_uPackedTime = 0;

    if (m_uVoiceID)
    {
        Audio::StopSFX(m_uVoiceID);
        m_uVoiceID = 0;
    }

    if (pOwner->m_eClassType != GOALIE)
    {
        g_pGame->SetPotentialScorer(pOwner);
    }

    m_pPhysicsBall->m_bUseMagnusEffect = false;
    m_unk_0xA6 = false;
    mpDamageTarget = NULL;
    m_unk_0xA3 = false;
}

/**
 * Offset/Address/Size: 0x1664 | 0x8000B038 | size: 0x50
 */
bool cBall::IsBuzzerBeaterSet() const
{

    bool res = false;
    if (m_tBuzzerBeaterTimer.m_uPackedTime != 0)
    {
        return true;
    }

    if (g_pBall->m_tShotTimer.m_uPackedTime != 0)
    {
        return true;
    }

    if ((Goalie::mbPosGoalieNetCheck != 0) || (Goalie::mbNegGoalieNetCheck != 0))
    {
        res = true;
    }

    return res;
}

/**
 * Offset/Address/Size: 0x16B4 | 0x8000B088 | size: 0x48
 */
void cBall::HandleBuzzerBeater(float seconds)
{
    if (seconds < 0.0f)
    {
        m_tBuzzerBeaterTimer.m_uPackedTime = 0;
        return;
    }
    if (m_tBuzzerBeaterTimer.m_uPackedTime == 0)
    {
        m_tBuzzerBeaterTimer.SetSeconds(seconds);
    }
}

/**
 * Offset/Address/Size: 0x16FC | 0x8000B0D0 | size: 0x44
 */
void cBall::ClearBallBlur()
{
    if (m_pBlurHandler != NULL)
    {
        m_pBlurHandler->Die(0.5f);
        m_pBlurHandler = NULL;
    }
}

/**
 * Offset/Address/Size: 0x1740 | 0x8000B114 | size: 0x20C
 */
void cBall::ClearShotInProgress()
{
    float fGameDuration = g_pGame->m_fGameDuration;
    if (g_pGame->GetGameTime() >= fGameDuration
        && g_pGame->m_eGameState == GS_GAMEPLAY
        && m_tBuzzerBeaterTimer.m_uPackedTime == 0)
    {
        m_tBuzzerBeaterTimer.SetSeconds(0.0f);
    }

    m_tShotTimer.m_uPackedTime = 0;
    mbCanDamage = false;
    m_unk_0xA4 = false;

    if (mbHyperSTS)
    {
        Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
    }

    Audio::FadeFilterFromCurrentToZero();

    FixedUpdateTask::mTimeScale = 1.0f;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (m_pBlurHandler != 0)
    {
        m_pBlurHandler->Die(0.0f);
        m_pBlurHandler = 0;
    }

    KillBallShot("ball_shot_perfect_glow", true);
    KillBallShot("ball_pass_perfect_glow", true);
    KillBallShot("shoot_to_score_shot", false);
    KillBallShot("ball_shot_onetimer", false);

    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

    if (mbHyperSTS)
    {
        void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
        PassBallData* eventdata = new (data) PassBallData();
        eventdata->pPasser = m_pPrevOwner;
        eventdata->pTarget = NULL;

        bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
        eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
    }

    mbHyperSTS = false;
    mbIsPerfectShot = false;

    gbCanFadeOutPerfectPassSFX = true;

    if (AudioLoader::IsInited())
    {
        gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
    }
}

/**
 * Offset/Address/Size: 0x194C | 0x8000B320 | size: 0x370
 */
void cBall::InitiateBallBlur(eBallShotEffectType effectType, cPlayer* pPlayer)
{
    if (m_pBlurHandler != NULL)
    {
        BlurManager::DestroyHandler(m_pBlurHandler, 0.0f);
        m_pBlurHandler = NULL;
    }

    switch (effectType)
    {
    case BALL_EFFECT_S2S_SUPER_SHOT:
    {
        char textureName[32] = "";
        nlStrNCpy(textureName, "global/shoottoscorestreak", 0x20);
        m_pBlurHandler = BlurManager::GetNewHandler(textureName, g_pGame->m_pGameTweaks->unk288, g_pGame->m_pGameTweaks->unk28C, true);
        break;
    }

    case BALL_EFFECT_S2S_SHOT:
        if (pPlayer != NULL)
        {
            if (pPlayer->IsCaptain() || nlSingleton<GameInfoManager>::s_pInstance->GetTeam((s16)pPlayer->m_pTeam->m_nSide) == TEAM_MYSTERY)
            {
                char textureName[32] = "";

                switch (nlSingleton<GameInfoManager>::s_pInstance->GetTeam((s16)pPlayer->m_pTeam->m_nSide))
                {
                case TEAM_DAISY:
                    nlStrNCpy(textureName, "global/daisyshoottoscorestreak", 0x20);
                    break;
                case TEAM_DONKEYKONG:
                    nlStrNCpy(textureName, "global/dkshoottoscorestreak", 0x20);
                    break;
                case TEAM_LUIGI:
                    nlStrNCpy(textureName, "global/luigishoottoscorestreak", 0x20);
                    break;
                case TEAM_MARIO:
                    nlStrNCpy(textureName, "global/marioshoottoscorestreak", 0x20);
                    break;
                case TEAM_PEACH:
                    nlStrNCpy(textureName, "global/peachshoottoscorestreak", 0x20);
                    break;
                case TEAM_WALUIGI:
                    nlStrNCpy(textureName, "global/washoottoscorestreak", 0x20);
                    break;
                case TEAM_WARIO:
                    nlStrNCpy(textureName, "global/warioshoottoscorestreak", 0x20);
                    break;
                case TEAM_YOSHI:
                    nlStrNCpy(textureName, "global/yoshishoottoscorestreak", 0x20);
                    break;
                case TEAM_MYSTERY:
                    nlStrNCpy(textureName, "global/mysshoottoscorestreak", 0x20);
                    break;
                default:
                    nlStrNCpy(textureName, "global/shoottoscorestreak", 0x20);
                    break;
                }

                m_pBlurHandler = BlurManager::GetNewHandler(textureName, g_pGame->m_pGameTweaks->unk288, g_pGame->m_pGameTweaks->unk28C, true);
                break;
            }
        }

        m_pBlurHandler = BlurManager::GetNewHandler("global/shoottoscorestreak", g_pGame->m_pGameTweaks->unk288, g_pGame->m_pGameTweaks->unk28C, true);
        break;

    case BALL_EFFECT_PERFECT_SHOT:
        m_pBlurHandler = BlurManager::GetNewHandler("global/greenshotstreak", 0.15f, 0x1E, true);
        break;

    case BALL_EFFECT_PERFECT_PASS:
        m_pBlurHandler = BlurManager::GetNewHandler("global/perfectpassstreak", 0.15f, 0x1E, true);
        break;

    case BALL_EFFECT_CHIP_SHOT:
        m_pBlurHandler = BlurManager::GetNewHandler("global/blueshotstreak", 0.15f, 0x1E, true);
        break;

    default:
        m_pBlurHandler = BlurManager::GetNewHandler("global/shotstreak", 0.15f, 0x1E, false);
        break;
    }
}

/**
 * Offset/Address/Size: 0x1CBC | 0x8000B690 | size: 0xF4
 */
bool cBall::GetInNet(int& nSide)
{
    cPlayer* goalie;

    if (m_pOwner != NULL)
    {
        if (m_pOwner->m_eClassType != GOALIE)
        {
            goto false_exit;
        }
        if (m_pOwner != NULL && m_pOwner->m_eClassType == GOALIE)
        {
            goalie = m_pOwner;
        }
        else
        {
            goalie = NULL;
        }
        if (goalie->m_pPhysicsCharacter->m_CanCollidedWithGoalLine)
        {
            goto false_exit;
        }
    }

    if (!m_pPhysicsBall->mbIsInsideNet)
    {
        goto false_exit;
    }

    nSide = -1;
    {
        cTeam** pTeams = g_pTeams;
        int i;
        for (i = 0; i < 2; i++)
        {
            if (m_v3Position.f.x * pTeams[i]->m_pNet->m_sideSign > 0.0f)
            {
                nSide = i;
            }
        }
    }

    if (m_pOwner != NULL && m_uGoalType != 2 && m_uGoalType != 6)
    {
        m_uGoalType = 5;
    }

    m_unk_0xA6 = false;
    mpDamageTarget = NULL;
    return true;

false_exit:
    return false;
}

/**
 * Offset/Address/Size: 0x1DB0 | 0x8000B784 | size: 0x20
 */
cPlayer* cBall::GetPassTargetFielder() const
{
    cPlayer* player = m_pPassTarget;
    if ((player == NULL) || (player->m_eClassType != FIELDER))
    {
        return NULL;
    }
    return player;
}

/**
 * Offset/Address/Size: 0x1DD0 | 0x8000B7A4 | size: 0x20
 */
cPlayer* cBall::GetOwnerGoalie()
{
    cPlayer* player = m_pOwner;
    if ((player == NULL) || (player->m_eClassType != GOALIE))
    {
        return NULL;
    }
    return player;
}

/**
 * Offset/Address/Size: 0x1DF0 | 0x8000B7C4 | size: 0x20
 */
cFielder* cBall::GetOwnerFielder()
{
    cPlayer* player = m_pOwner;
    if ((player == NULL) || (player->m_eClassType != FIELDER))
    {
        return NULL;
    }
    return (cFielder*)player;
}

/**
 * Offset/Address/Size: 0x1E10 | 0x8000B7E4 | size: 0x28
 */
nlVector3* cBall::GetDrawablePosition() const
{
    const nlMatrix4& mtx = m_pDrawableBall->GetWorldMatrix();
    return (nlVector3*)&(mtx.m[3][0]);
}

/**
 * Offset/Address/Size: 0x1E38 | 0x8000B80C | size: 0x1C
 */
nlVector3* cBall::GetAIVelocity() const
{
    cPlayer* temp_r4 = m_pOwner;
    if (temp_r4 != NULL)
    {
        return &(temp_r4->m_v3Velocity);
    }
    return (nlVector3*)&(m_v3Velocity);
}

static inline float clampAbove(float minVal, float x)
{
    if (minVal >= x)
    {
        return minVal;
    }
    return x;
}

/**
 * Offset/Address/Size: 0x1E54 | 0x8000B828 | size: 0x400
 */
void cBall::PostPhysicsUpdate(float fDeltaT)
{
    m_v3PrevPosition = m_v3Position;
    m_pPhysicsBall->GetPosition(&m_v3Position);
    m_pPhysicsBall->GetLinearVelocity(&m_v3Velocity);

    if (m_unk_0xA6 && mpDamageTarget != NULL)
    {
        nlVector3 v3HitSpot;
        nlVector3 v3CurPos;
        nlVector3 targetDelta;
        nlVector3 currentDelta;
        nlVector3 v3PrevPos;
        float fPercent;
        nlVector3 v3BallVel;
        float fPrevZVel;

        v3HitSpot = mpDamageTarget->GetJointPosition(mpDamageTarget->m_pPoseAccumulator->m_BaseSHierarchy->m_nPelvisNodeIndex);
        v3HitSpot.f.z = clampAbove(0.3f, v3HitSpot.f.z + 0.05f);

        v3CurPos = m_v3Position;
        v3PrevPos = m_v3PrevPosition;

        if (v3CurPos.f.z < 0.3f)
        {
            v3CurPos.f.z = 0.3f;
        }

        if (v3PrevPos.f.z < 0.3f)
        {
            v3PrevPos.f.z = 0.3f;
        }

        nlVec3Set(targetDelta,
            v3HitSpot.f.x - v3PrevPos.f.x,
            v3HitSpot.f.y - v3PrevPos.f.y,
            v3HitSpot.f.z - v3PrevPos.f.z);
        nlVec3Set(currentDelta,
            v3CurPos.f.x - v3PrevPos.f.x,
            v3CurPos.f.y - v3PrevPos.f.y,
            v3CurPos.f.z - v3PrevPos.f.z);

        float targetDist = nlSqrt(targetDelta.GetLengthSq3D(), true);
        float currentDist = nlSqrt(currentDelta.GetLengthSq3D(), true);

        fPercent = 0.5f;
        if (targetDist < currentDist)
        {
            nlVec3Scale(currentDelta, targetDist / targetDist);
        }
        else
        {
            nlVec3Scale(targetDelta, currentDist / targetDist);
        }

        if (targetDist < 5.0f)
        {
            fPercent += 0.5f * (1.0f - targetDist / 5.0f);
        }

        nlVecLerp(currentDelta, currentDelta, targetDelta, fPercent);
        nlVec3Add(v3CurPos, v3PrevPos, currentDelta);

        m_v3Position = v3CurPos;
        m_pPhysicsBall->SetPosition(v3CurPos, PhysicsObject::WORLD_COORDINATES);
        m_pPhysicsBall->SetRotation(m3Ident);

        FakeBallWorld::InvalidateBallCache();
        m_bBallPathChangeCount = m_bBallPathChangeCount + 1;

        fPrevZVel = m_v3Velocity.f.z;
        const nlVector3& ballVelocity = m_v3Velocity;
        float distanceSq = currentDelta.GetLengthSq3D();
        float projectedScale = nlVec3DotProduct(ballVelocity, currentDelta) / distanceSq;
        nlVec3Scale(v3BallVel, currentDelta, projectedScale);
        v3BallVel.f.z = fPrevZVel;

        float speedSq = v3BallVel.GetLengthSq3D();
        if (speedSq < 400.0f)
        {
            float speed = nlSqrt(speedSq, true);
            nlVec3Scale(v3BallVel, 20.0f / speed);
        }

        if (v3CurPos.f.z < 0.4f && v3BallVel.f.z < 0.0f)
        {
            v3BallVel.f.z = 0.0f;
        }

        m_v3Velocity = v3BallVel;
        m_pPhysicsBall->SetLinearVelocity(v3BallVel);
    }

    UpdateOrientation(fDeltaT);

    if (m_pBlurHandler != NULL)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }
}

/**
 * Offset/Address/Size: 0x2254 | 0x8000BC28 | size: 0x5BC
 */
void cBall::CollideWithWallCallback()
{
    if (m_tNoPickupTimer.m_uPackedTime != 0)
    {
        return;
    }

    if (m_pPassTarget != NULL)
    {
        if (mbHyperSTS)
        {
            Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
        }
        Audio::FadeFilterFromCurrentToZero();

        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);

        if (m_pBlurHandler != 0)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = 0;
        }

        KillBallShot("ball_shot_perfect_glow", true);
        KillBallShot("ball_pass_perfect_glow", true);
        KillBallShot("shoot_to_score_shot", false);
        KillBallShot("ball_shot_onetimer", false);

        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

        if (mbHyperSTS)
        {
            void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
            PassBallData* eventdata = new (data) PassBallData();
            eventdata->pPasser = m_pPrevOwner;
            eventdata->pTarget = NULL;

            bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
            eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
        }

        mbHyperSTS = false;
        mbIsPerfectShot = false;

        gbCanFadeOutPerfectPassSFX = true;

        if (AudioLoader::IsInited())
        {
            gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
        }

        if (m_pPassTarget)
        {
            m_pPassTarget = NULL;
        }

        m_v3PassIntercept.f.x = 0.0f;
        m_v3PassIntercept.f.y = 0.0f;
        m_v3PassIntercept.f.z = 0.0f;

        m_tPassTargetTimer.m_uPackedTime = 0;

        if (m_uVoiceID)
        {
            Audio::StopSFX(m_uVoiceID);
            m_uVoiceID = 0;
        }
    }

    bool perfectCatch;
    u32 shotTimer = m_tShotTimer.m_uPackedTime;
    if (shotTimer != 0)
    {
        perfectCatch = false;
        if (shotTimer != 0)
        {
            if (m_unk_0xA4)
            {
                perfectCatch = true;
            }
        }

        if (perfectCatch)
        {
            EmitBallWallHit("perfect_shot_catch");
        }
        else
        {
            bool scoredShot = false;
            if (shotTimer != 0)
            {
                if (mbCanDamage)
                {
                    scoredShot = true;
                }
            }

            if (scoredShot)
            {
                if (m_pPrevOwner != NULL && m_pPrevOwner->m_eClassType == FIELDER)
                {
                    BasicString<char, Detail::TempStringAllocator> effectName(
                        GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((s16)m_pPrevOwner->m_pTeam->m_nSide)));
                    effectName.AppendInPlace("_shoot_to_score_catch");
                    cPlayer* prevOwner = m_pPrevOwner;
                    EmissionController* pController = EmitGeneric(prevOwner, effectName.c_str(), NULL);
                    pController->SetPosition(m_v3Position);
                }
            }
        }

        f32 fGameDuration = g_pGame->m_fGameDuration;
        if (g_pGame->GetGameTime() >= fGameDuration && g_pGame->m_eGameState == GS_GAMEPLAY && m_tBuzzerBeaterTimer.m_uPackedTime == 0)
        {
            m_tBuzzerBeaterTimer.SetSeconds(0.5f);
        }

        m_tShotTimer.m_uPackedTime = 0;
        mbCanDamage = false;
        m_unk_0xA4 = false;

        if (mbHyperSTS)
        {
            Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
        }
        Audio::FadeFilterFromCurrentToZero();

        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);

        if (m_pBlurHandler != 0)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = 0;
        }

        KillBallShot("ball_shot_perfect_glow", true);
        KillBallShot("ball_pass_perfect_glow", true);
        KillBallShot("shoot_to_score_shot", false);
        KillBallShot("ball_shot_onetimer", false);

        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

        if (mbHyperSTS)
        {
            void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
            PassBallData* eventdata = new (data) PassBallData();
            eventdata->pPasser = m_pPrevOwner;
            eventdata->pTarget = NULL;

            bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
            eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
        }

        mbHyperSTS = false;
        mbIsPerfectShot = false;

        gbCanFadeOutPerfectPassSFX = true;

        if (AudioLoader::IsInited())
        {
            gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
        }
    }
}

/**
 * Offset/Address/Size: 0x2810 | 0x8000C1E4 | size: 0x3C4
 */
void cBall::CollideWithGroundCallback()
{
    if (mbHyperSTS)
    {
        Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
    }
    Audio::FadeFilterFromCurrentToZero();

    FixedUpdateTask::mTimeScale = 1.0f;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (m_pBlurHandler != 0)
    {
        m_pBlurHandler->Die(0.5f);
        m_pBlurHandler = 0;
    }

    KillBallShot("ball_shot_perfect_glow", true);
    KillBallShot("ball_pass_perfect_glow", true);
    KillBallShot("shoot_to_score_shot", false);
    KillBallShot("ball_shot_onetimer", false);

    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

    if (mbHyperSTS)
    {
        void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
        PassBallData* eventdata = new (data) PassBallData();
        eventdata->pPasser = m_pPrevOwner;
        eventdata->pTarget = NULL;

        bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
        eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
    }

    mbHyperSTS = false;
    mbIsPerfectShot = false;

    gbCanFadeOutPerfectPassSFX = true;

    if (AudioLoader::IsInited())
    {
        gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
    }

    if (m_pPassTarget)
    {
        m_pPassTarget = NULL;
    }

    m_v3PassIntercept.f.x = 0.f;
    m_v3PassIntercept.f.y = 0.f;
    m_v3PassIntercept.f.z = 0.f;

    m_tPassTargetTimer.m_uPackedTime = 0;

    if (m_uVoiceID)
    {
        Audio::StopSFX(m_uVoiceID);
        m_uVoiceID = 0;
    }

    f32 fGameDuration = g_pGame->m_fGameDuration;
    if (g_pGame->GetGameTime() >= fGameDuration && g_pGame->m_eGameState == GS_GAMEPLAY && m_tBuzzerBeaterTimer.m_uPackedTime == 0)
    {
        m_tBuzzerBeaterTimer.SetSeconds(0.5f);
    }

    m_tShotTimer.m_uPackedTime = 0;
    mbCanDamage = false;
    m_unk_0xA4 = false;

    if (mbHyperSTS)
    {
        Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
    }
    Audio::FadeFilterFromCurrentToZero();

    FixedUpdateTask::mTimeScale = 1.0f;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (m_pBlurHandler != 0)
    {
        m_pBlurHandler->Die(0.5f);
        m_pBlurHandler = 0;
    }

    KillBallShot("ball_shot_perfect_glow", true);
    KillBallShot("ball_pass_perfect_glow", true);
    KillBallShot("shoot_to_score_shot", false);
    KillBallShot("ball_shot_onetimer", false);

    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

    if (mbHyperSTS)
    {
        void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
        PassBallData* eventdata = new (data) PassBallData();
        eventdata->pPasser = m_pPrevOwner;
        eventdata->pTarget = NULL;

        bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
        eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
    }

    mbHyperSTS = false;
    mbIsPerfectShot = false;

    gbCanFadeOutPerfectPassSFX = true;

    if (AudioLoader::IsInited())
    {
        gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
    }
}

/**
 * Offset/Address/Size: 0x2BD4 | 0x8000C5A8 | size: 0xA18
 */
void cBall::CollideWithCharacterCallback(cPlayer* pCharacter, const nlVector3& v3PreBallVelocity)
{
    if (m_pOwner == NULL)
    {
        float currentVel = nlSqrt(m_v3Velocity.f.x * m_v3Velocity.f.x + m_v3Velocity.f.y * m_v3Velocity.f.y, true);
        if (currentVel > 0.1f)
        {
            float previousVel = nlSqrt(v3PreBallVelocity.f.x * v3PreBallVelocity.f.x + v3PreBallVelocity.f.y * v3PreBallVelocity.f.y, true);
            float speedDifference = (previousVel - currentVel) / currentVel;
            speedDifference = fabsf(speedDifference);

            if (speedDifference < 0.2f)
            {
                float dot = m_v3Velocity.f.x * v3PreBallVelocity.f.x + m_v3Velocity.f.y * v3PreBallVelocity.f.y;
                if (dot > previousVel * (0.99f * currentVel))
                {
                    return;
                }
            }
        }
    }

    if (m_tShotTimer.m_uPackedTime != 0)
    {
        float fGameDuration = g_pGame->m_fGameDuration;
        if (g_pGame->GetGameTime() >= fGameDuration && g_pGame->m_eGameState == GS_GAMEPLAY && m_tBuzzerBeaterTimer.m_uPackedTime == 0)
        {
            m_tBuzzerBeaterTimer.SetSeconds(0.5f);
        }

        m_tShotTimer.m_uPackedTime = 0;
        mbCanDamage = false;
        m_unk_0xA4 = false;

        if (mbHyperSTS)
        {
            Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
        }
        Audio::FadeFilterFromCurrentToZero();

        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);

        if (m_pBlurHandler != 0)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = 0;
        }

        KillBallShot("ball_shot_perfect_glow", true);
        KillBallShot("ball_pass_perfect_glow", true);
        KillBallShot("shoot_to_score_shot", false);
        KillBallShot("ball_shot_onetimer", false);

        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

        if (mbHyperSTS)
        {
            void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
            PassBallData* eventdata = new (data) PassBallData();
            eventdata->pPasser = m_pPrevOwner;
            eventdata->pTarget = NULL;

            bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
            eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
        }

        mbHyperSTS = false;
        mbIsPerfectShot = false;

        gbCanFadeOutPerfectPassSFX = true;

        if (AudioLoader::IsInited())
        {
            gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
        }

        if (pCharacter->m_eClassType != GOALIE)
        {
            Audio::SoundAttributes sndAtr;
            sndAtr.Init();
            sndAtr.SetSoundType(0xB7, true);
            sndAtr.UseStationaryPosVector(pCharacter->m_v3Position);
            Audio::gStadGenSFX.Play(sndAtr);
        }
    }

    cFielder* pOwnerFielder = (cFielder*)m_pOwner;
    if (pOwnerFielder != NULL && pOwnerFielder->m_eClassType == FIELDER)
    {
        pOwnerFielder = (cFielder*)m_pOwner;
    }
    else
    {
        pOwnerFielder = NULL;
    }

    if (m_pPassTarget != NULL)
    {
        if (m_pPassTarget->m_pTeam != pCharacter->m_pTeam)
        {
            Audio::SoundAttributes sndAtr;
            sndAtr.Init();
            sndAtr.SetSoundType(0xB7, true);
            sndAtr.UseStationaryPosVector(pCharacter->m_v3Position);
            Audio::gStadGenSFX.Play(sndAtr);
        }

        if (mbHyperSTS)
        {
            Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
        }
        Audio::FadeFilterFromCurrentToZero();

        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);

        if (m_pBlurHandler != 0)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = 0;
        }

        KillBallShot("ball_shot_perfect_glow", true);
        KillBallShot("ball_pass_perfect_glow", true);
        KillBallShot("shoot_to_score_shot", false);
        KillBallShot("ball_shot_onetimer", false);

        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
        Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

        if (mbHyperSTS)
        {
            void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
            PassBallData* eventdata = new (data) PassBallData();
            eventdata->pPasser = m_pPrevOwner;
            eventdata->pTarget = NULL;

            bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
            eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
        }

        mbHyperSTS = false;
        mbIsPerfectShot = false;

        gbCanFadeOutPerfectPassSFX = true;

        if (AudioLoader::IsInited())
        {
            gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
        }

        if (m_pPassTarget)
        {
            m_pPassTarget = NULL;
        }

        m_v3PassIntercept.f.x = 0.f;
        m_v3PassIntercept.f.y = 0.f;
        m_v3PassIntercept.f.z = 0.f;

        m_tPassTargetTimer.m_uPackedTime = 0;

        if (m_uVoiceID)
        {
            Audio::StopSFX(m_uVoiceID);
            m_uVoiceID = 0;
        }

        m_uGoalType = 4;

        if (pCharacter->m_eClassType == FIELDER && ((cFielder*)pCharacter)->IsSlideTackling())
        {
            ((cFielder*)pCharacter)->DoAwardPowerupStuff(AWARD_POWERUP_INTERCEPT_PASS, 0.0f);
        }
    }

    if (pOwnerFielder != NULL && pCharacter->m_eClassType == FIELDER && pCharacter->m_pTeam != pOwnerFielder->m_pTeam)
    {
        cFielder* pCharacterFielder = (cFielder*)pCharacter;

        pOwnerFielder->TestCollisionForInvicibility((cFielder*)pCharacter);

        if (((cFielder*)pCharacter)->IsSlideTackling())
        {
            nlVector3 v3ContactLocation = pCharacter->m_v3Position;
            nlVector3 v3PhysicsRadialSpot;
            nlPolarToCartesian(v3PhysicsRadialSpot.f.x, v3PhysicsRadialSpot.f.y, pCharacter->m_aActualFacingDirection, pCharacter->m_pTweaks->fPhysCapsuleRadius);

            v3PhysicsRadialSpot.f.z = 0.0f;
            nlVec3Add(v3ContactLocation, v3ContactLocation, v3PhysicsRadialSpot);

            s16 nHitterContactLocationFacingDelta = pCharacter->GetFacingDeltaToPosition(v3ContactLocation);
            u16 absFacingDelta = nHitterContactLocationFacingDelta < 0 ? -nHitterContactLocationFacingDelta : nHitterContactLocationFacingDelta;

            if (absFacingDelta < 0x2000)
            {
                if (pOwnerFielder->IsSlideTackling())
                {
                    s16 nHitteeContactLocationFacingDelta = pOwnerFielder->GetFacingDeltaToPosition(v3ContactLocation);
                    u16 absOwnerFacingDelta = nHitteeContactLocationFacingDelta < 0 ? -nHitteeContactLocationFacingDelta : nHitteeContactLocationFacingDelta;

                    if (absOwnerFacingDelta < 0x2000)
                    {
                        if (pOwnerFielder->m_fActualSpeed < pCharacterFielder->m_fActualSpeed)
                        {
                            pOwnerFielder->InitActionSlideAttackReact(pCharacterFielder, false);
                            pCharacterFielder->SetSlideAttackSuccessFlag();
                            pCharacterFielder->PickupBall(g_pBall);
                            pCharacterFielder->DoSlideAttackStats();
                        }
                        else
                        {
                            pCharacterFielder->InitActionSlideAttackReact(pOwnerFielder, false);
                            pOwnerFielder->SetSlideAttackSuccessFlag();
                        }
                    }
                    else
                    {
                        pOwnerFielder->InitActionSlideAttackReact(pCharacterFielder, false);
                        pCharacterFielder->DoPenaltyCardBooking(pOwnerFielder, PEN_TYPE_SLIDE_WITH_BALL);
                        pCharacterFielder->SetSlideAttackSuccessFlag();

                        if (pCharacterFielder->CanPickupBall(g_pBall))
                        {
                            pCharacterFielder->PickupBall(g_pBall);
                            pCharacterFielder->DoSlideAttackStats();
                        }
                    }
                }
                else
                {
                    pOwnerFielder->InitActionSlideAttackReact(pCharacterFielder, false);
                    pCharacterFielder->DoPenaltyCardBooking(pOwnerFielder, PEN_TYPE_SLIDE_WITH_BALL);
                    pCharacterFielder->SetSlideAttackSuccessFlag();
                    pCharacterFielder->PickupBall(g_pBall);
                    pCharacterFielder->DoSlideAttackStats();
                }
            }
        }
        else if (pOwnerFielder->IsSlideTackling())
        {
            if (!((cFielder*)pCharacter)->IsHitting())
            {
                ((cFielder*)pCharacter)->InitActionSlideAttackReact(pOwnerFielder, false);
                pOwnerFielder->SetSlideAttackSuccessFlag();
            }
        }
        else if (pOwnerFielder->IsBallAwayFromCarrier() && !((cFielder*)pCharacter)->IsFallenDown(0.0f))
        {
            pOwnerFielder->ReleaseBall();

            do
            {
                if (!((cFielder*)pCharacter)->IsFrozen() && (((cFielder*)pCharacter)->m_eActionState == ACTION_RUNNING || ((cFielder*)pCharacter)->m_eActionState == ACTION_SLIDE_ATTACK))
                {
                    s16 delta = pCharacterFielder->GetFacingDeltaToPosition(g_pBall->m_v3Position);
                    u16 absDelta = delta < 0 ? -delta : delta;

                    if (absDelta < 0x4000)
                    {
                        pCharacterFielder->PickupBall(g_pBall);
                        pCharacterFielder->InitActionRunningWB(false);
                        break;
                    }
                }

                pOwnerFielder->ShootBallDueToContact(pCharacterFielder->m_v3Velocity);
                pOwnerFielder->SetNoPickUpTime(0.33f);
                pCharacterFielder->SetNoPickUpTime(0.33f);
            } while (0);

            pOwnerFielder->InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
        }
    }

    if (m_pOwner == NULL)
    {
        m_pLastTouch = pCharacter;
        FakeBallWorld::InvalidateBallCache();
        m_bBallDeflectCount++;
    }

    m_bBallPathChangeCount++;

    if (pCharacter->m_eClassType == FIELDER)
    {
        m_v3ShotOrigin = m_v3Position;
    }
}

/**
 * Offset/Address/Size: 0x35EC | 0x8000CFC0 | size: 0x1A8
 */
void cBall::ClearBallEffects()
{
    if (mbHyperSTS)
    {
        Audio::gWorldSFX.Stop(Audio::eWorldSFX(0x57), cGameSFX::SFX_STOP_FIRST);
    }
    Audio::FadeFilterFromCurrentToZero();

    FixedUpdateTask::mTimeScale = 1.0f;
    ParticleUpdateTask::SetTimeScale(1.0f);

    if (m_pBlurHandler != 0)
    {
        m_pBlurHandler->Die(0.5f);
        m_pBlurHandler = 0;
    }

    KillBallShot("ball_shot_perfect_glow", true);
    KillBallShot("ball_pass_perfect_glow", true);
    KillBallShot("shoot_to_score_shot", false);
    KillBallShot("ball_shot_onetimer", false);

    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xB9), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBA), cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop(Audio::eWorldSFX(0xBD), cGameSFX::SFX_STOP_FIRST);

    if (mbHyperSTS)
    {
        void* data = (u8*)g_pEventManager->CreateValidEvent(0x47, 0x24) + 0x10;
        PassBallData* eventdata = new (data) PassBallData();
        eventdata->pPasser = m_pPrevOwner;
        eventdata->pTarget = NULL;

        bool pad = eventdata->pPasser->GetGlobalPad() != NULL;
        eventdata->mPasserControllerID = pad ? eventdata->pPasser->GetGlobalPad()->m_padIndex : -1;
    }

    mbHyperSTS = false;
    mbIsPerfectShot = false;

    gbCanFadeOutPerfectPassSFX = true;

    if (AudioLoader::IsInited())
    {
        gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
    }
}

/**
 * Offset/Address/Size: 0x3794 | 0x8000D168 | size: 0x7C
 */
void cBall::ClearOwner()
{
    m_pPrevOwner = m_pOwner;
    m_pOwner = NULL;
    m_pPhysicsBall->EnableCollisions();

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsBall->GetPosition(&m_v3Position);

    m_pPhysicsBall->GetLinearVelocity(&m_v3Velocity);

    m_bBallPathChangeCount++;
}

/**
 * Offset/Address/Size: 0x3810 | 0x8000D1E4 | size: 0x78
 */
cBall::~cBall()
{
    delete m_pPhysicsBall;
    delete m_pBallPosCollider;
}

static nlVector3 v3Zero = { 0.f, 0.f, 0.f };
static nlQuaternion qIdentity = { 0.f, 0.f, 0.f, 1.f };

/**
 * Offset/Address/Size: 0x3908 | 0x8000D2DC | size: 0x260
 */
cBall::cBall()
    : m_bBallPathChangeCount(0)
    , m_bBallDeflectCount(0)
    , m_tShotTimer(0.f)
    , m_tNoPickupTimer(0.f)
    , m_tPassTargetTimer(0.f)
    , m_tBuzzerBeaterTimer(0.f)
{
    u32 t0, t2;
    register u32 t1;
    nlVector3* pz;

    m_pBlurHandler = NULL;
    m_pOwner = NULL;
    m_pPrevOwner = NULL;
    m_pLastTouch = NULL;
    m_pPassTarget = NULL;
    m_pShooter = NULL;

    m_uGoalType = 4;
    m_uVoiceID = 0;

    mbIsPerfectShot = false;
    mbHyperSTS = false;
    mbCanDamage = false;
    m_unk_0xA3 = false;
    m_unk_0xA4 = false;
    m_unk_0xA6 = false;
    mpDamageTarget = NULL;

    m_pDrawableBall = WorldManager::s_World->FindDrawableObject(nlStringHash("gameplay/ball"));

    m_pDrawableBall->m_uObjectFlags |= 0x4;
    m_pDrawableBall->m_uObjectFlags |= 0x10;
    m_pDrawableBall->m_uObjectFlags |= 0x100;

    m_pPhysicsBall = new (nlMalloc(0x5c, 8, FALSE)) PhysicsAIBall(0.18f);
    m_pPhysicsBall->m_pAIBall = this;

    m_v3Position.f.x = 0.f;
    m_v3Position.f.y = 2.f;
    m_v3Position.f.z = 0.18f;

    m_v3PrevPosition = m_v3Position;

    m_v3PassIntercept.f.x = 0.f;
    m_v3PassIntercept.f.y = 0.f;
    m_v3PassIntercept.f.z = 0.f;

    m_pPhysicsBall->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);

    pz = &v3Zero;

    m_qOrientation.f.z = 0.f;
    t1 = pz->as_u32[1];
    m_qOrientation.f.y = 0.f;
    t0 = pz->as_u32[0];
    m_qOrientation.f.x = 0.f;
    t2 = pz->as_u32[2];
    m_qOrientation.f.w = 1.f;

    m_v3ShotOrigin = m_v3Position;

    m_v3Velocity.as_u32[0] = t0;
    m_v3Velocity.as_u32[1] = t1;
    m_v3Velocity.as_u32[2] = t2;

    m_pPhysicsBall->SetLinearVelocity(m_v3Velocity);
    m_pPhysicsBall->SetAngularVelocity(*pz);

    m_fTotalPassTime = 0.f;
    m_tBuzzerBeaterTimer.SetSeconds(0.f);

    nlVector3 rayDir = { 0.f, 0.f, -1.f };

    m_pBallPosCollider = new (nlMalloc(0x2C, 8, FALSE)) RayCollider(1.f, m_v3Position, rayDir);

    if (AudioLoader::IsInited())
    {
        gfPerfectPassSFXVol = Audio::gStadGenSFX.GetSFXVol(0xBA);
    }
}
