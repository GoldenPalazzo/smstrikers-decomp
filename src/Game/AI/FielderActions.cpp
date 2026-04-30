#include "Game/AI/FielderActions.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Camera/animcam.h"
#include "Game/Camera/rumblefilter.h"
#include "Game/RumbleActions.h"
#include "Game/Sys/eventman.h"
#include "Game/Sys/audio.h"
#include "Game/ReplayManager.h"
#include "Game/Drawable/DrawableCharacter.h"
#include "Game/FixedUpdateTask.h"
#include "Game/ParticleUpdateTask.h"
#include "Game/SAnim.h"
#include "Game/AnimInventory.h"
#include "Game/Game.h"
#include "Game/Ball.h"
#include "Game/CharacterTriggers.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/AI/ShotMeter.h"
#include "Game/AI/FuzzyVariant.h"
#include "Game/AI/AiUtil.h"
#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Physics/PhysicsColumn.h"
#include "Game/MathHelpers.h"
#include "Game/DB/StatsTracker.h"
#include "Game/GameInfo.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/Field.h"
#include "Game/Render/ShootToScoreMeter.h"
#include "Game/Render/ShootToScoreArrow.h"
#include "Game/Render/Wiper.h"
#include "Game/Effects/EffectsGroup.h"
#include "Game/World.h"
#include "NL/nlBasicString.h"
#include "NL/nlFormat.h"
#include "NL/nlConfig.h"
#include "NL/nlFunction.h"
#include "NL/nlMath.h"
#include "NL/plat/plataudio.h"
#include "types.h"

f32 CANT_COLLIDE = *(f32*)__float_max;

extern unsigned int nlDefaultSeed;
extern FuzzyVariant fvNotSet;
extern cBall* g_pBall;

const LooseBallContactAnimInfo* GetOneTimerLeadGroundContactAnims();

struct HitReactInfo
{
    /* 0x0 */ int nAnimID;
    /* 0x4 */ u16 aFacingDirectionOffset;
}; // total size: 0x8

HitReactInfo g_HitReactInfo[4] = {
    { 0x6F, 0x8000 },
    { 0x72, 0x4000 },
    { 0x71, 0x0000 },
    { 0x70, 0xc000 },
};
// extern HitReactInfo g_HitReactInfo[4];
// static unsigned short g_IdleTurnCompletionDelta = 0xB6; // size: 0x2, address: 0x803976D2

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };
static nlVector3 captainStsTargetPos = { 0.0f, 0.0f, 0.0f };
static bool setCaptainStscaptainStsTargetPos;

static cRumbleFilter rumbleFilter;
static float sfMatrixCamDuration = 4.25f;
static float sfMatrixCamZoomTime = 3.35f;
static float sfMatrixCamInitialDistanceFromTarget = 2.0f;
static float sfMatrixCamFinalDistanceFromTarget = 10.0f;
static float sfMatrixCamFinalHeightAboveTarget = 4.5f;
static float sfMatrixCamInitialAngle = -15.0f;
static float sfMatrixCamNumRevolutions = 0.61f;
static float sfMatrixCamStartFrame = 103.9f;
static float sfFirstKickFrame = 103.9f;
static float sfMatrixCamTimeScale = 0.005f;
static float sfMatrixCamParticleTimeScale = 0.06f;
static bool sbMatrixCamTurnOffModelRendering = true;
static float sfMatrixCamFOV = 50.0f;
static float sfHyperStrikeFadeOutSpeed = 1.0f;
static float sfHyperStrikeFadeInSpeed = 0.01f;
static float sfOtherMatrixCamDuration = 3.0f;
static float sfOtherMatrixCamSpinRate = 172.0f;
static float sfOtherMatrixCamFinalDistanceFromTarget = 7.0f;
static float sfOtherMatrixCamTransitionTime = 0.2f;
static float sfHyperStrikeAnimCamBeginFrame = 63.0f;
static float sfHyperStrikeAnimCamStartTime = 0.4f;
static float sfOtherMatrixCamTimeScale = 0.5f;
static float sfTimeBetweenApplyingRumbleFilters = 0.04f;
static float sfRumblePauseTime = 0.12f;
static float sfHyperStrikeRumbleSpringConstant = 8000.0f;
static float sfHyperStrikeRumbleDampingConstant = 15.0f;
static float sfMatrixCamInitialHeightAboveTarget;
static bool sbMatrixCamUseWorldDarkening;
static float sfMatrixCamPauseAfterSpin;
static float sfOtherMatrixCamZoomTime;
static float sfHyperStrikeMaxRumbleIntensity;
static bool sbDoShatteredGlassTransition;

extern cCharacter* g_pCurrentlyUpdatingCharacter;
extern float g_fFixedUpdateTick;
extern unsigned char sSTSLighting__17DrawableCharacter;
extern unsigned char sbIsHyperShootToScoreRenderingEnabled__5World;
extern unsigned char sbShowPositiveXNetDuringHyperStrike__5World;
extern void Flash__10PhotoFlashFv();

template <typename StringType, typename ValueType>
StringType Format(const StringType&, const ValueType&);

static u16 g_IdleTurnCompletionDelta = 0x00;

int SlideAttackReactAnims[4] = {
    0x66,
    0x69,
    0x68,
    0x67,
};

const static int ShellAttackReactAnims[4] = {
    0x66,
    0x69,
    0x68,
    0x67,
};

int PassingAnims[4] = {
    0x33,
    0x36,
    0x35,
    0x34,
};

// /**
//  * Offset/Address/Size: 0x13C | 0x80030010 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator> >::operator% <const char*>(const char* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x114 | 0x8002FFE8 | size: 0x28
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator> >::operator BasicString<char, Detail::TempStringAllocator>() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8002FED4 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, const char*>(const BasicString<char, Detail::TempStringAllocator>&, const char* const&)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8002FEBC | size: 0x18
 */
template cBaseCamera* nlDLRingGetStart<cBaseCamera>(cBaseCamera*);

// /**
//  * Offset/Address/Size: 0x118 | 0x8002FE7C | size: 0x40
//  */
// void 0x8002FEBC..0x8002FED4 | size : 0x18
// {
// }

// /**
//  * Offset/Address/Size: 0x30 | 0x8002FD94 | size: 0xE8
//  */
// void Detail::LexicalCastImpl<BasicString<char, Detail::TempStringAllocator>, const char*>::Do(const char*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8002FD64 | size: 0x30
//  */
// void LexicalCast<BasicString<char, Detail::TempStringAllocator>, const char*>(const char* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8002FD5C | size: 0x8
//  */
// void cNet::GetPostRadius()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8002FD14 | size: 0x48
//  */
// void Function1<void, EmissionController&>::FunctorBase::~FunctorBase()
// {
// }

/**
 * Offset/Address/Size: 0x8654 | 0x8002F18C | size: 0xB88
 * TODO: 91.5% match - 2 extra extsh after neg in abs computation (MWCC peephole opt)
 */
void cFielder::asmRunning()
{
    s16 nAbsActualToDesiredFacingDirection;
    s16 nAbsActualToDesiredMovementDirection;
    eStrafeDirection strafeDir;
    u8 bFirstTime;
    int nNewAnimState;

    nAbsActualToDesiredFacingDirection = (s16)(u16)abs_s16((s16)(m_aDesiredFacingDirection - m_aActualFacingDirection));
    nAbsActualToDesiredMovementDirection = (s16)(u16)abs_s16((s16)(m_aDesiredMovementDirection - m_aActualMovementDirection));

    mActionRunningVars.eLastStrafeDirection = CalculateStrafeDirection(m_aDesiredFacingDirection, m_aDesiredMovementDirection);

    do
    {
        bFirstTime = false;

        switch (m_eAnimID)
        {
        default:
            SetIdleAnimState();
            bFirstTime = true;
            break;

        case 0x04:
        case 0x05:
        case 0x06:
            if (ShouldStartCrossBlend(0))
            {
                if (m_fDesiredSpeed <= m_pTweaks->fJoggingSpeed)
                {
                    SetIdleAnimState();
                }
                else
                {
                    m_fActualSpeed = m_pTweaks->fRunningSpeed;
                    SetRunningAnimState(0.1f);
                }
            }
            break;

        case 0x27:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_RIGHT:
                SetStrafeRightAnimState();
                break;
            case STRAFE_LEFT:
                SetStrafeLeftAnimState();
                break;
            case STRAFE_IDLE:
            case STRAFE_FORWARD:
                if (m_fActualSpeed > 3.1f)
                {
                    if (nAbsActualToDesiredFacingDirection >= 0x3a98)
                    {
                        SetBackRunningToRunAnimState();
                    }
                    else
                    {
                        SetBackRunningStopAnimState();
                    }
                }
                else
                {
                    SetIdleStrafeAnimState();
                }
                break;
            case STRAFE_BACK:
            {
                FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
                m_fDesiredSpeed = pTweaks->fRunningBackwardsSpeed;
                break;
            }
            }
            break;

        case 0x2E:
            if (ShouldStartCrossBlend(7))
            {
                SetRunningAnimState(0.1f);
            }
            break;

        case 0x2F:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
                if (m_pCurrentAnimController->m_fTime >= 0.5f)
                {
                    SetIdleStrafeAnimState();
                }
                break;
            case STRAFE_RIGHT:
                if (ShouldStartCrossBlend(0))
                {
                    SetIdleStrafeAnimState();
                }
                break;
            case STRAFE_LEFT:
                if (ShouldStartCrossBlend(0x28))
                {
                    SetStrafeLeftAnimState();
                }
                break;
            case STRAFE_FORWARD:
                if (ShouldStartCrossBlend(7))
                {
                    if (m_fActualSpeed < 2.0f)
                    {
                        SetStartAnimState(-1);
                    }
                    else
                    {
                        SetRunningAnimState(0.1f);
                    }
                }
                break;
            case STRAFE_BACK:
                if (ShouldStartCrossBlend(0x27))
                {
                    SetRunBackwardsAnimState();
                }
                break;
            }
            break;

        case 0x30:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
                if (m_pCurrentAnimController->m_fTime >= 0.5f)
                {
                    SetIdleStrafeAnimState();
                }
                break;
            case STRAFE_LEFT:
                if (ShouldStartCrossBlend(0))
                {
                    SetIdleStrafeAnimState();
                }
                break;
            case STRAFE_RIGHT:
                if (ShouldStartCrossBlend(0x29))
                {
                    SetStrafeRightAnimState();
                }
                break;
            case STRAFE_FORWARD:
                if (ShouldStartCrossBlend(7))
                {
                    if (m_fActualSpeed < 2.0f)
                    {
                        SetStartAnimState(-1);
                    }
                    else
                    {
                        SetRunningAnimState(0.1f);
                    }
                }
                break;
            case STRAFE_BACK:
                if (ShouldStartCrossBlend(0x27))
                {
                    SetRunBackwardsAnimState();
                }
                break;
            }
            break;

        case 0x2A:
        {
            bool animFinished = false;
            if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
            {
                animFinished = true;
            }
            if (animFinished)
            {
                switch (mActionRunningVars.eLastStrafeDirection)
                {
                case STRAFE_IDLE:
                case STRAFE_RIGHT:
                case STRAFE_LEFT:
                case STRAFE_BACK:
                    SetBackRunningStopRecoverAnimState();
                    break;
                case STRAFE_FORWARD:
                    nNewAnimState = ((m_aDesiredFacingDirection - m_aActualFacingDirection + 0x2000) >> 14) & 3;
                    if (nNewAnimState != 0)
                    {
                        SetStartAnimState(nNewAnimState);
                    }
                    else
                    {
                        SetBackRunningStopStartAnimState();
                    }
                    break;
                }
            }
            break;
        }

        case 0x2B:
            if (ShouldStartCrossBlend(7))
            {
                switch (mActionRunningVars.eLastStrafeDirection)
                {
                case STRAFE_RIGHT:
                case STRAFE_LEFT:
                case STRAFE_BACK:
                    SetStopAnimState();
                    break;
                case STRAFE_IDLE:
                    SetIdleStrafeAnimState();
                    break;
                case STRAFE_FORWARD:
                    SetRunningAnimState(0.1f);
                    break;
                }
            }
            break;

        case 0x2C:
        {
            bool animFinished = false;
            if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
            {
                animFinished = true;
            }
            if (animFinished)
            {
                switch (mActionRunningVars.eLastStrafeDirection)
                {
                case STRAFE_BACK:
                    SetRunBackwardsAnimState();
                    break;
                case STRAFE_RIGHT:
                    SetStrafeRightAnimState();
                    break;
                case STRAFE_LEFT:
                    SetStrafeLeftAnimState();
                    break;
                case STRAFE_IDLE:
                    SetIdleAnimState();
                    break;
                case STRAFE_FORWARD:
                    SetStartAnimState(-1);
                    break;
                }
            }
            break;
        }

        case 0x28:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
            case STRAFE_RIGHT:
                if (m_fActualSpeed > 3.1f)
                {
                    SetAnimState(0x30, true, 0.2f, false, false);
                    InitMovementFromAnim(0, v3Zero, 1.0f, false);
                    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.5f;
                }
                else
                {
                    SetIdleStrafeAnimState();
                }
                break;
            case STRAFE_LEFT:
            {
                FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
                m_fDesiredSpeed = pTweaks->fRunningStrafeSpeed;
                break;
            }
            case STRAFE_FORWARD:
                SetRunningAnimState(0.1f);
                m_aDesiredFacingDirection = m_aDesiredMovementDirection;
                m_aActualFacingDirection = m_aDesiredFacingDirection;
                break;
            case STRAFE_BACK:
                SetRunBackwardsAnimState();
                break;
            }
            break;

        case 0x29:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
            case STRAFE_LEFT:
                if (m_fActualSpeed > 3.1f)
                {
                    SetAnimState(0x2f, true, 0.2f, false, false);
                    InitMovementFromAnim(0, v3Zero, 1.0f, false);
                    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.5f;
                }
                else
                {
                    SetIdleStrafeAnimState();
                }
                EmitDust(this, "stop");
                break;
            case STRAFE_RIGHT:
            {
                FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
                m_fDesiredSpeed = pTweaks->fRunningStrafeSpeed;
                break;
            }
            case STRAFE_FORWARD:
                SetRunningAnimState(0.1f);
                m_aDesiredFacingDirection = m_aDesiredMovementDirection;
                m_aActualFacingDirection = m_aDesiredFacingDirection;
                break;
            case STRAFE_BACK:
                SetRunBackwardsAnimState();
                break;
            }
            break;

        case 0x0B:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
                if (ShouldStartCrossBlend(0))
                {
                    SetIdleAnimState();
                }
                break;
            case STRAFE_RIGHT:
                if (ShouldStartCrossBlend(0x29))
                {
                    SetStrafeRightAnimState();
                }
                break;
            case STRAFE_LEFT:
                if (ShouldStartCrossBlend(0x28))
                {
                    SetStrafeLeftAnimState();
                }
                break;
            case STRAFE_FORWARD:
                if (nAbsActualToDesiredFacingDirection >= 0x639c)
                {
                    if (m_fActualSpeed > 2.1f)
                    {
                        if (!m_tSwapFacingTimer.GetSeconds())
                        {
                            SetHardStopAnimState();
                        }
                    }
                    else
                    {
                        SetStartAnimState(-1);
                    }
                }
                else
                {
                    if (ShouldStartCrossBlend(0))
                    {
                        SetStartAnimState(-1);
                    }
                }
                break;
            case STRAFE_BACK:
                if (m_pCurrentAnimController->m_fTime > 0.4f)
                {
                    SetRunBackwardsAnimState();
                    m_fActualSpeed = 0.0f;
                }
                break;
            }
            break;

        case 0x31:
        case 0x32:
            if (ShouldStartCrossBlend(0))
            {
                SetIdleAnimState();
            }
            break;

        case 0x00:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
                m_fDesiredSpeed = 0.0f;
                break;
            case STRAFE_RIGHT:
                SetStrafeRightAnimState();
                break;
            case STRAFE_LEFT:
                SetStrafeLeftAnimState();
                break;
            case STRAFE_FORWARD:
                if (m_fActualSpeed < 2.0f)
                {
                    SetStartAnimState(-1);
                }
                else
                {
                    SetRunningAnimState(0.1f);
                }
                break;
            case STRAFE_BACK:
                SetRunBackwardsAnimState();
                break;
            }
            break;

        case 0x07:
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
                if (m_fActualSpeed > 3.1f)
                {
                    SetStopAnimState();
                }
                else
                {
                    SetIdleAnimState();
                }
                break;
            case STRAFE_FORWARD:
                if (nAbsActualToDesiredFacingDirection >= 0x639c)
                {
                    if (m_fActualSpeed > 2.1f)
                    {
                        if (!m_tSwapFacingTimer.GetSeconds())
                        {
                            SetHardStopAnimState();
                        }
                    }
                    else
                    {
                        SetStartAnimState(-1);
                    }
                }
                else
                {
                    if (m_fDesiredSpeed > ((FielderTweaks*)m_pTweaks)->fRunningTurboSpeed - 0.1f)
                    {
                        SetRunningTurboAnimState();
                    }
                }
                break;
            case STRAFE_RIGHT:
                SetStrafeRightAnimState();
                break;
            case STRAFE_LEFT:
                SetStrafeLeftAnimState();
                break;
            case STRAFE_BACK:
                if (m_fDesiredSpeed > m_pTweaks->fJoggingSpeed)
                {
                    if (nAbsActualToDesiredMovementDirection < 0x4000)
                    {
                        SetRunToBackRunningAnimState();
                    }
                    else
                    {
                        SetStopAnimState();
                    }
                }
                else
                {
                    SetStopAnimState();
                }
                break;
            }
            break;

        case 0x2D:
            if (ShouldStartCrossBlend(0x27))
            {
                SetRunBackwardsAnimState();
            }
            break;

        case 0x0F:
        {
            switch (mActionRunningVars.eLastStrafeDirection)
            {
            case STRAFE_IDLE:
            case STRAFE_RIGHT:
            case STRAFE_LEFT:
            case STRAFE_BACK:
                SetStopAnimState();
                break;
            case STRAFE_FORWARD:
                if (nAbsActualToDesiredFacingDirection >= 0x639c)
                {
                    SetStopAnimState();
                }
                else
                {
                    if (m_fDesiredSpeed <= m_pTweaks->fRunningSpeed && !mActionRunningVars.bFirstCycleOfTurbo)
                    {
                        SetRunningAnimState(0.1f);
                    }
                }
                break;
            }
            if (ShouldStartCrossBlend(7))
            {
                if (mActionRunningVars.bFirstCycleOfTurbo)
                {
                    mActionRunningVars.bFirstCycleOfTurbo = false;
                }
            }
            break;
        }

        case 0x12:
        {
            bool animFinished = false;
            if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
            {
                animFinished = true;
            }
            if (animFinished)
            {
                if (nAbsActualToDesiredMovementDirection >= 0x639c && m_fDesiredSpeed > m_pTweaks->fJoggingSpeed)
                {
                    SetHardStopTurnAnimState();
                }
                else
                {
                    SetHardStopRecoverAnimState();
                }
            }
            break;
        }

        case 0x13:
            if (ShouldStartCrossBlend(0))
            {
                if (m_fDesiredSpeed >= m_pTweaks->fJoggingSpeed)
                {
                    m_fActualSpeed = m_pTweaks->fRunningSpeed;
                    SetRunningAnimState(0.1f);
                }
                else
                {
                    SetStopAnimState();
                }
            }
            break;

        case 0x14:
            if (ShouldStartCrossBlend(7))
            {
                SetIdleAnimState();
            }
            break;
        }
    } while (bFirstTime);
}

/**
 * Offset/Address/Size: 0x7F3C | 0x8002EA74 | size: 0x718
 */
void cFielder::asmRunningWB(float fDeltaT)
{
    float fIdleToRunWBDesiredSpeed = 0.1f + m_pTweaks->fJoggingSpeed;
    s16 nAbsActualToDesiredFacingDirection = (s16)(u16)abs_s16((s16)(m_aDesiredFacingDirection - m_aActualFacingDirection));
    bool bFirstTime;

    do
    {
        bFirstTime = false;

        switch (m_eAnimID)
        {
        default:
        {
            if (mActionRunningWBVars.bWaitForAnimToFinish)
            {
                bool bAnimFinished = false;
                if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
                {
                    bAnimFinished = true;
                }
                if (bAnimFinished || m_fDesiredSpeed >= fIdleToRunWBDesiredSpeed)
                {
                    mActionRunningWBVars.bWaitForAnimToFinish = false;
                }
            }

            if (mActionRunningWBVars.bWaitForAnimToFinish)
            {
                break;
            }

            bool bShotMeterActive = false;
            eShotMeterState state = m_pShotMeter->m_eShotMeterState;
            if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
            {
                bShotMeterActive = true;
            }

            if (bShotMeterActive)
            {
                SetWindupWBAnimState();
            }
            else if (m_eAnimID == 0x54)
            {
                SetRunningWBAnimState(0.1f);
            }
            else
            {
                SetIdleWBAnimState();
            }
            bFirstTime = true;
            break;
        }

        case 0x16:
        case 0x17:
        case 0x18:
        {
            if (ShouldStartCrossBlend(0x23))
            {
                if (m_fDesiredSpeed <= m_pTweaks->fJoggingSpeed)
                {
                    SetIdleWBAnimState();
                }
                else
                {
                    m_fActualSpeed = m_pTweaks->fRunningSpeed;
                    SetRunningWBAnimState(0.1f);
                }
            }
            break;
        }

        case 0x23:
        {
            bool bShotMeterActive = false;
            eShotMeterState state = m_pShotMeter->m_eShotMeterState;
            if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
            {
                bShotMeterActive = true;
            }

            if (bShotMeterActive)
            {
                SetWindupWBAnimState();
                break;
            }

            if (ShouldStartCrossBlend(0x15))
            {
                SetIdleWBAnimState();
                m_fActualSpeed = 0.0f;
                m_aActualMovementDirection = m_aActualFacingDirection;
                break;
            }

            if (nAbsActualToDesiredFacingDirection >= 0x639C)
            {
                if (m_fActualSpeed > 2.1f)
                {
                    SetHardStopAnimState();
                }
                else if (m_fActualSpeed < 2.0f)
                {
                    SetStartWBAnimState();
                }
                break;
            }

            if (m_fDesiredSpeed > m_pTweaks->fJoggingSpeed)
            {
                if (m_fActualSpeed < 2.0f)
                {
                    SetStartWBAnimState();
                }
            }
            break;
        }

        case 0x56:
        case 0x57:
        {
            cNet* pOtherNet = m_pTeam->GetOtherNet();
            float fAngle = nlATan2f(pOtherNet->m_baseLocation.f.y - m_v3Position.f.y, pOtherNet->m_baseLocation.f.x - m_v3Position.f.x);
            m_aDesiredFacingDirection = (u16)(s32)(10430.378f * fAngle);

            u16 aNewFacingDirection = SeekDirection(
                m_aActualFacingDirection,
                m_aDesiredFacingDirection,
                ((FielderTweaks*)m_pTweaks)->fShotWindupDirectionSeekSpeed,
                ((FielderTweaks*)m_pTweaks)->fShotWindupDirectionSeekFalloff,
                fDeltaT);
            SetFacingDirection(aNewFacingDirection);

            m_fDesiredSpeed = 0.0f;

            float fTotalDuration = !IsCaptain() ? g_pGame->m_pGameTweaks->unk2D0 : m_pShotMeter->GetTotalDuration();

            float fTimeLeft = fTotalDuration - m_pShotMeter->m_fTime;
            if (fTimeLeft < 0.01f)
            {
                fTimeLeft = 0.01f;
            }

            m_fDecel = (m_fActualSpeed - m_fDesiredSpeed) / fTimeLeft;
            if (m_fDecel > 50.0f)
            {
                m_fDecel = 50.0f;
                break;
            }

            if (m_fDecel < ((FielderTweaks*)m_pTweaks)->fShotWindupDecel)
            {
                m_fDecel = ((FielderTweaks*)m_pTweaks)->fShotWindupDecel;
                break;
            }

            break;
        }

        case 0x15:
        {
            bool bShotMeterActive = false;
            eShotMeterState state = m_pShotMeter->m_eShotMeterState;
            if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
            {
                bShotMeterActive = true;
            }

            if (bShotMeterActive)
            {
                SetWindupWBAnimState();
                break;
            }

            if (m_fDesiredSpeed > 1.0f)
            {
                if (m_fActualSpeed < 2.0f)
                {
                    SetStartWBAnimState();
                }
                else
                {
                    SetRunningWBAnimState(0.1f);
                }
                break;
            }

            if (m_fActualSpeed > 3.1f)
            {
                SetStopWBAnimState();
                break;
            }

            m_fDesiredSpeed = 0.0f;
            break;
        }

        case 0x1a:
        {
            bool bShotMeterActive = false;
            eShotMeterState state = m_pShotMeter->m_eShotMeterState;
            if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
            {
                bShotMeterActive = true;
            }

            if (bShotMeterActive)
            {
                SetWindupWBAnimState();
                break;
            }

            if (m_fDesiredSpeed > ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed + 0.15f
                && m_tBallPossessionTimer.GetSeconds() > 0.4f)
            {
                if (m_ePowerup == 0x7 || m_ePowerup == 0x8)
                {
                    SetAction(ACTION_RUNNING_WB);
                    mActionRunningWBVars.bWaitForAnimToFinish = false;
                    mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
                    m_aActualMovementDirection = m_aActualFacingDirection;
                }
                else
                {
                    SetAction(ACTION_RUNNING_WB_TURBO);
                    InitMovementRunningNoTurn(((FielderTweaks*)m_pTweaks)->fRunningWBTurboAccel, ((FielderTweaks*)m_pTweaks)->fRunningWBTurboDecel);

                    bool bForceMirrorSwap;
                    cPN_SAnimController* pController = m_pCurrentAnimController;
                    if (!pController->m_bMirror)
                    {
                        bForceMirrorSwap = false;
                        if (m_eAnimID == 0x1A)
                        {
                            if (pController->m_fTime > 0.25f && pController->m_fTime < 0.75)
                            {
                                bForceMirrorSwap = true;
                            }
                        }
                        mActionRunningWBTurboVars.bForcedMirrorSwap = bForceMirrorSwap;
                    }
                    else
                    {
                        bForceMirrorSwap = false;
                        if (m_eAnimID == 0x1A)
                        {
                            if (pController->m_fTime < 0.25f || pController->m_fTime > 0.75)
                            {
                                bForceMirrorSwap = true;
                            }
                        }
                        mActionRunningWBTurboVars.bForcedMirrorSwap = bForceMirrorSwap;
                    }

                    SetRunTurboAnimState(0x1D, mActionRunningWBTurboVars.bForcedMirrorSwap);
                }
                break;
            }

            if (nAbsActualToDesiredFacingDirection >= 0x639C)
            {
                if (m_fActualSpeed > 3.1f)
                {
                    SetHardStopAnimState();
                }
                else
                {
                    SetStartWBAnimState();
                }
                break;
            }

            if (m_fDesiredSpeed < m_pTweaks->fJoggingSpeed - 0.15f)
            {
                if (m_fActualSpeed > 3.1f)
                {
                    SetStopWBAnimState();
                }
                else
                {
                    SetIdleWBAnimState();
                }
                break;
            }

            break;
        }

        case 0x24:
        {
            bool bAnimFinished = false;
            if (m_pCurrentAnimController->m_ePlayMode == PM_HOLD && m_pCurrentAnimController->m_fTime == 1.0f)
            {
                bAnimFinished = true;
            }
            if (bAnimFinished)
            {
                if (m_fDesiredSpeed < m_pTweaks->fJoggingSpeed)
                {
                    SetHardStopRecoverAnimState();
                }
                else
                {
                    SetHardStopTurnAnimState();
                }
            }
            break;
        }

        case 0x25:
            if (ShouldStartCrossBlend(0x15))
            {
                if (m_fDesiredSpeed >= m_pTweaks->fJoggingSpeed)
                {
                    m_fActualSpeed = m_pTweaks->fRunningSpeed;
                    SetRunningWBAnimState(0.1f);
                }
                else
                {
                    SetStopWBAnimState();
                }
            }
            break;

        case 0x26:
            if (ShouldStartCrossBlend(0x1A))
            {
                SetIdleWBAnimState();
                m_fActualSpeed = 0.0f;
            }
            break;
        }
    } while (bFirstTime);
}

/**
 * Offset/Address/Size: 0x7F18 | 0x8002EA50 | size: 0x24
 */
void cFielder::EndAction()
{
    SetAction(ACTION_NEED_ACTION);
}

/**
 * Offset/Address/Size: 0x7D0C | 0x8002E844 | size: 0x20C
 */
nlVector3 GetClosestWallPoint(const nlVector3& pos)
{
    nlVector3 topSideline = pos;
    topSideline.f.y = cField::GetSidelineY(1U);

    nlVector3 rightGoalLine = pos;
    rightGoalLine.f.x = cField::GetGoalLineX(1.0f);

    nlVector3 leftGoalLine = pos;
    leftGoalLine.f.x = cField::GetGoalLineX(-1.0f);

    nlVector3 bottomSideline = pos;
    bottomSideline.f.y = -topSideline.f.y;

    f32 distTop = fabsf(pos.f.y - topSideline.f.y);
    f32 distBottom = fabsf(pos.f.y - bottomSideline.f.y);
    f32 distRight = fabsf(pos.f.x - rightGoalLine.f.x);
    f32 distLeft = fabsf(pos.f.x - leftGoalLine.f.x);

    if (distTop < distBottom)
    {
        if (distRight < distLeft)
        {
            if (distTop < distRight)
            {
                return topSideline;
            }
            else
            {
                return rightGoalLine;
            }
        }
        else
        {
            if (distTop < distLeft)
            {
                return topSideline;
            }
            else
            {
                return leftGoalLine;
            }
        }
    }
    else
    {
        if (distRight < distLeft)
        {
            if (distBottom < distRight)
            {
                return bottomSideline;
            }
            else
            {
                return rightGoalLine;
            }
        }
        else
        {
            if (distBottom < distLeft)
            {
                return bottomSideline;
            }
            else
            {
                return leftGoalLine;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x77D8 | 0x8002E310 | size: 0x534
 * TODO: wrong register for pClosestOpponent -> r31 instead of r27
 */
void cFielder::InitActionDeke(ePadActions padAction)
{
    m_pShotMeter->Abort(this);

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_DEKE);

    mActionDekeVars.bStickWasReset = false;
    mActionDekeVars.bPossibleSuccessfulDeke = false;
    mActionDekeVars.bPossibleTurboMove = false;

    if (padAction == PAD_DEKE)
    {
        if ((m_pController != nullptr) && (m_pController->GetCStickMovementStickMagnitude() > 0.0f))
        {
            u16 cStickDirection = m_pController->GetCStickMovementStickDirection();
            s16 facingDelta = (s16)(cStickDirection - m_aActualFacingDirection);
            if (facingDelta < 0)
                SetAnimState(0x55, true, 0.2f, false, false);
            else
                SetAnimState(0x54, true, 0.2f, false, false);
        }
        else
        {
            nlVector3 wallPoint = GetClosestWallPoint(m_v3Position);
            float dist = nlSqrt(CalculateDistanceSquared(wallPoint, m_v3Position), true);

            if (dist < 2.25f)
            {
                if (GetFacingDeltaToPosition(wallPoint) < 0)
                    SetAnimState(0x54, true, 0.2f, false, false);
                else
                    SetAnimState(0x55, true, 0.2f, false, false);
            }
            else
            {
                int i;
                float fClosestDist;
                cPlayer* pOpponent;
                cPlayer* pClosestOpponent;

                pClosestOpponent = nullptr;
                fClosestDist = 99999.9f;
                i = 0;

                do
                {
                    pOpponent = m_pTeam->GetOtherTeam()->GetPlayer(i);
                    float flashLightResult = DoFlashLight(
                        pOpponent->m_v3Position,
                        m_aActualFacingDirection,
                        g_pGame->m_pGameTweaks->fDekeAngleWeighting,
                        0.0f,
                        9999.0f);

                    if (flashLightResult < fClosestDist)
                    {
                        pClosestOpponent = pOpponent;
                        fClosestDist = flashLightResult;
                    }
                    i++;
                } while (i < 5);

                if (GetFacingDeltaToPosition(pClosestOpponent->m_v3Position) < 0)
                {
                    SetAnimState(0x54, true, 0.2f, false, false);
                }
                else
                {
                    SetAnimState(0x55, true, 0.2f, false, false);
                }
            }
        }
    }
    else
    {
        switch (padAction)
        {
        case PAD_DEKE_RIGHT:
            SetAnimState(0x55, true, 0.2f, false, false);
            break;
        case PAD_DEKE_LEFT:
            SetAnimState(0x54, true, 0.2f, false, false);
            break;
        }
    }

    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    int nSFX = 0;
    switch (m_eAnimID)
    {
    case 0x54:
        nSFX = 0xF;
        break;
    case 0x55:
        nSFX = 0x10;
        break;
    }

    Play3DSFX(Audio::eCharSFX(nSFX), VECTORS, 100.0f);

    cFielder* pOpponent;
    int j;
    cTeam* pOtherTeam;
    bool bHasNearbyThreat;

    bHasNearbyThreat = false;
    pOtherTeam = m_pTeam->GetOtherTeam();
    j = 0;

    do
    {
        if (!bHasNearbyThreat)
        {
            pOpponent = pOtherTeam->GetFielder(j);
            if (pOpponent->IsSlideTackling() || pOpponent->IsHitting())
            {
                if (CalculateDistanceSquared(pOpponent->m_v3Position, m_v3Position) < 6.25f)
                {
                    u16 angleDiff = (u16)abs_s16(pOpponent->m_aActualFacingDirection - m_aActualFacingDirection);
                    if (angleDiff > 0x2000)
                        bHasNearbyThreat = true;
                }
            }
        }
        j++;
    } while (j < 4);

    mActionDekeVars.bPossibleSuccessfulDeke = bHasNearbyThreat;
}
/**
 * Offset/Address/Size: 0x74A8 | 0x8002DFE0 | size: 0x330
 */
/**
 * Offset/Address/Size: 0x74A8 | 0x8002DFE0 | size: 0x330
 */
void cFielder::ActionDeke(float dt)
{
    cGlobalPad* pGlobalPad = GetGlobalPad();
    if (pGlobalPad != nullptr)
    {
        if (!mActionDekeVars.bStickWasReset)
        {
            float cStickMagnitude = m_pController->GetCStickMovementStickMagnitude();
            if (cStickMagnitude < 0.0001f)
            {
                if (!GetGlobalPad()->IsPressed(0x1D, 0x1))
                {
                    mActionDekeVars.bStickWasReset = true;
                }
            }
        }

        if (mActionDekeVars.bStickWasReset)
        {
            TestButtonsToQueueActions(dt);
        }

        bool bTurboPressed = m_pController->IsTurboPressed();
        if (bTurboPressed)
        {
            if (!mActionDekeVars.bTurboButtonDownLastUpdate)
            {
                mActionDekeVars.bPossibleTurboMove = true;
            }
            mActionDekeVars.bTurboButtonDownLastUpdate = true;
        }
        else
        {
            mActionDekeVars.bTurboButtonDownLastUpdate = false;
        }
    }

    if (!mActionDekeVars.bPossibleSuccessfulDeke)
    {
        cFielder* pOpponent;
        int i;
        cTeam* pOtherTeam;
        bool bHasThreat;

        bHasThreat = false;
        pOtherTeam = m_pTeam->GetOtherTeam();

        for (i = 0; i < 4; i++)
        {
            if (!bHasThreat)
            {
                pOpponent = pOtherTeam->GetFielder(i);
                if (pOpponent->IsSlideTackling() || pOpponent->IsHitting())
                {
                    float distSq = CalculateDistanceSquared(pOpponent->m_v3Position, m_v3Position);
                    if (distSq < 6.25f)
                    {
                        u16 angleDiff = (u16)abs_s16(pOpponent->m_aActualFacingDirection - m_aActualFacingDirection);
                        if (angleDiff > 0x2000)
                        {
                            bHasThreat = true;
                        }
                    }
                }
            }
        }
        mActionDekeVars.bPossibleSuccessfulDeke = bHasThreat;
    }

    if (ShouldStartCrossBlend(0x1A))
    {
        if (mActionDekeVars.bPossibleSuccessfulDeke)
        {
            const nlVector3& offNetLocation = GetAIOffNetLocation(nullptr);
            nlPolar polar;
            nlVector3 delta;
            nlVec3Sub(delta, offNetLocation, m_v3Position);
            nlCartesianToPolar(polar, delta.f.x, delta.f.y);

            u16 absAngleDiff = (u16)abs_s16(m_aActualFacingDirection - polar.a);
            if (absAngleDiff < 0x4000)
            {
                DoAwardPowerupStuff(AWARD_POWERUP_CONTEXT_DEKE, 0.0f);
            }
        }

        if (m_pBall == nullptr)
        {
            SetAction(ACTION_NEED_ACTION);
            m_eLastPadAction = (ePadActions)0x25;
            return;
        }

        if (TestQueuedActions())
        {
            m_eLastPadAction = (ePadActions)0x25;
            return;
        }

        if (mActionDekeVars.bPossibleSuccessfulDeke && GetGlobalPad() == nullptr)
        {
            float fRandomRange = 0.2f * ((FielderTweaks*)m_pTweaks)->fDekeing;
            float factor = 0.5f;

            bool turboTurn;
            if ((nlRandomf(fRandomRange, &nlDefaultSeed) - (fRandomRange * factor)) > 0.0f)
            {
                turboTurn = true;
            }
            else
            {
                turboTurn = false;
            }
            mActionDekeVars.bPossibleTurboMove = turboTurn;
        }

        if (mActionDekeVars.bPossibleTurboMove && mActionDekeVars.bPossibleSuccessfulDeke && !IsInvincible())
        {
            m_ePowerup = (ePowerUpType)0x7;
            mnNumPowerups = 1;
            m_pPowerupTarget = nullptr;
            ThrowPowerup();
            m_tPowerupEffectTime.SetSeconds(0.5f);
        }

        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x71BC | 0x8002DCF4 | size: 0x2EC
 */
void cFielder::InitActionElectrocution(const nlVector3& wallPosition, const nlVector3& wallNormal)
{
    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_ELECTROCUTION);

    SetFacingDirection(nlVector3ToAngle(wallNormal, 0x8000));

    SetAnimState(0x74, true, 0.2f, false, false);
    InitMovementNone(0.0f, 0.0f);

    nlVector3 jointPos = GetJointPosition(m_nBip01JointIndex_0xA4);

    nlVector3 futureJointPos;
    GetJointPositionFuture(&futureJointPos, 0, m_nBip01JointIndex_0xA4, 0.0f, false, false, false);

    futureJointPos.f.z += 0.1f;
    jointPos.f.z = (jointPos.f.z >= futureJointPos.f.z) ? jointPos.f.z : futureJointPos.f.z;

    SetPosition(jointPos);

    mActionElectrocutionVars.electrocutionTime = 1.0f;

    nlVector3 effectPos;
    effectPos.f.x = wallPosition.f.x;
    effectPos.f.y = wallPosition.f.y;
    effectPos.f.z = jointPos.f.z;
    CharacterElectrocutionEffect(this, effectPos, wallNormal);

    BeginRumbleAction(RUMBLE_SOLID_CONTACT, GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x6F6C | 0x8002DAA4 | size: 0x250
 * TODO: 99.80% match - f3/f5 register swap for launchVelocity x/y components.
 */
void cFielder::ActionElectrocution(float dt)
{
    switch (m_eAnimID)
    {
    case 0x74:
    {
        mActionElectrocutionVars.electrocutionTime -= dt;
        if (mActionElectrocutionVars.electrocutionTime < 0.0f)
        {
            SetAnimState(0x75, true, 0.2f, false, false);
            InitMovementCoast();

            nlVector3 launchVelocity = { 1.0f, 2.0f, 8.0f };
            float cosAngle;
            float sinAngle;
            nlSinCos(&sinAngle, &cosAngle, m_aActualFacingDirection);

            nlVector3 velocity;
            nlVec3SetRotatedXY(velocity, launchVelocity, cosAngle, sinAngle);
            SetVelocity(velocity);

            EmitElectrocutionExplosion(this);
            BeginRumbleAction((eRumbleActionPreset)1, GetGlobalPad());
        }
        break;
    }
    case 0x75:
    {
        nlVector3 velocity = m_v3Velocity;
        velocity.f.x *= 0.99f;
        velocity.f.y *= 0.99f;
        velocity.f.z += -30.0f * dt;
        SetVelocity(velocity);

        m_v3Position.f.z += dt * m_v3Velocity.f.z;
        if (m_v3Position.f.z < 0.0f)
        {
            m_v3Position.f.z = 0.0f;
            m_v3Velocity.f.z = 0.0f;

            SetAnimState(0x76, true, 0.2f, false, false);
            InitMovementFromAnim(0, v3Zero, 0.0f, false);
            BeginRumbleAction((eRumbleActionPreset)1, GetGlobalPad());

            Audio::SoundAttributes attrs;
            attrs.Init();
            attrs.me_ClassType = 1;
            attrs.SetSoundType(0xB, true);
            attrs.UseStationaryPosVector(m_v3Position);
            PlaySFX(attrs);
        }
        break;
    }
    case 0x76:
        if (ShouldStartCrossBlend(7))
        {
            SetAction(ACTION_NEED_ACTION);
            m_pCharacterSFX->StopPlayingRandomCharDialogue(CHAR_DIALOGUE_ELECTROCUTE);
        }
        break;
    }
}

/**
 * Offset/Address/Size: 0x6AF4 | 0x8002D62C | size: 0x478
 */
void cFielder::InitActionHit(cFielder* pTarget)
{
    if (IsFrozen())
    {
        return;
    }

    if (pTarget != nullptr)
    {
        cSAnim* pAnim = m_pAnimInventory->GetAnim(0x6E);
        nlVector3 rootTransStart;
        nlVector3 rootTransEnd;

        pAnim->GetRootTrans(4.0f / (float)pAnim->m_nNumKeys, &rootTransStart);
        pAnim->GetRootTrans(14.0f / (float)pAnim->m_nNumKeys, &rootTransEnd);

        float distance = nlSqrt(CalculateDistanceSquared(rootTransEnd, rootTransStart), true) / 0.3333333f;

        int nInterceptResult = 0;

        nlVector3 targetVelocity = pTarget->m_v3Velocity;
        if (pTarget->m_eActionState == 0)
        {
            targetVelocity = v3Zero;
        }

        // Calculate intercept
        float fInterceptTimes[2];
        float combinedRadius = 0.5f * (m_pTweaks->fPhysCapsuleRadius + pTarget->m_pTweaks->fPhysCapsuleRadius);
        CalcInterceptXY(m_v3Position, distance, combinedRadius, pTarget->m_v3Position, targetVelocity, nInterceptResult, fInterceptTimes);

        float fTime;

        if (nInterceptResult != 0)
        {
            if (nInterceptResult == 2)
            {
                fTime = nlMinEquals(fInterceptTimes[0], fInterceptTimes[1]);
            }
            else
            {
                fTime = fInterceptTimes[0];
            }
        }
        else
        {
            fTime = 0.3f;
        }

        nlVector3 interceptPos;
        interceptPos.f.x = (fTime * pTarget->m_v3Velocity.f.x) + pTarget->m_v3Position.f.x;
        interceptPos.f.y = (fTime * pTarget->m_v3Velocity.f.y) + pTarget->m_v3Position.f.y;

        float angleRad = nlATan2f(interceptPos.f.y - m_v3Position.f.y, interceptPos.f.x - m_v3Position.f.x);
        u16 targetAngle = (u16)(s32)(10430.378f * angleRad); // @5580 = 10430.378f

        SetFacingDirection(targetAngle);

        // Set facing direction fields
        m_aDesiredFacingDirection = m_aActualFacingDirection;
        m_aDesiredMovementDirection = m_aDesiredFacingDirection;
        m_aActualMovementDirection = m_aDesiredMovementDirection;
    }

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_HIT);
    SetAnimState(0x6E, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    PlayRandomCharDialogue(0, VECTORS, 100.0f, -1.0f);

    // Create event
    const Event* pEvent = g_pEventManager->CreateValidEvent(0x16, 0x28);
    PlayerAttackData* pData = new ((u8*)pEvent + 0x10) PlayerAttackData();
    pData->pAttacker = this;

    bool bHasGlobalPad = GetGlobalPad() != nullptr;
    pData->nAttackerPadID = bHasGlobalPad ? GetGlobalPad()->m_padIndex : -1;

    pData->pTarget = pTarget;

    m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.5f;
}

/**
 * Offset/Address/Size: 0x6A8C | 0x8002D5C4 | size: 0x68
 */
void cFielder::ActionHit(float)
{
    if (ShouldStartCrossBlend(7))
    {
        m_fActualSpeed = 0.0f;
        m_fDesiredSpeed = 0.0f;
        SetVelocity(v3Zero);
        InitMovementCoast();
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x66F4 | 0x8002D22C | size: 0x398
 */
bool cFielder::InitActionHitReact(cPlayer* pAttacker, unsigned short desiredFacingDirection, bool bDoFrameLock)
{
    CleanUpPowerupEffect();

    if (IsFallenDown(0.0f))
    {
        return false;
    }

    if (IsFrozen())
    {
        m_tFrozenTimer.SetSeconds(0.0f);
        EmitUnFreeze(this);
    }

    mActionHitReactActionVars.bDoFrameLock = bDoFrameLock;

    if (m_pBall != nullptr)
    {
        ReleaseBall();
        ShootBallDueToContact(pAttacker->m_v3Velocity);

        if (!mActionHitReactActionVars.bDoFrameLock)
        {
            FireCameraRumbleFilter(0.0f, 0.2f);
        }

        if (pAttacker->m_eClassType == FIELDER && ((cFielder*)pAttacker)->IsHitting())
        {
            ((cFielder*)pAttacker)->DoPenaltyCardBooking(this, PEN_TYPE_HIT_WITH_BALL);
        }
    }
    else if (pAttacker->m_eClassType == FIELDER && ((cFielder*)pAttacker)->IsHitting())
    {
        ((cFielder*)pAttacker)->DoPenaltyCardBooking(this, PEN_TYPE_HIT_NO_BALL);
    }

    if (!mActionHitReactActionVars.bDoFrameLock)
    {
        BeginRumbleAction(RUMBLE_SOLID_CONTACT, GetGlobalPad());
    }

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_HIT_REACT);

    s16 angleDiff = (s16)((u16)(desiredFacingDirection + 0x8000) - m_aActualFacingDirection) + 0x2000;
    u32 index = ((angleDiff >> 14) & 3);

    SetAnimState(g_HitReactInfo[index].nAnimID, true, 0.2f, false, false);
    SetFacingDirection(desiredFacingDirection + g_HitReactInfo[index].aFacingDirectionOffset);

    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    if (g_pGame->IsGameplayOrOvertime())
    {
        StatsTracker::Instance()->TrackStat(STATS_HITS_MADE, pAttacker->m_pTeam->m_nSide, pAttacker->m_ID, 0, 0, 0, 0);
    }

    m_fDesiredSpeed = 0.0f;
    return true;
}

/**
 * Offset/Address/Size: 0x6658 | 0x8002D190 | size: 0x9C
 */
void cFielder::ActionHitReact(float)
{
    m_pCurrentAnimController->TestFrameTrigger(2.0f);

    if ((m_pCurrentAnimController->TestFrameTrigger(3.0f)) && (mActionHitReactActionVars.bDoFrameLock))
    {
        BeginRumbleAction(RUMBLE_SOLID_CONTACT, GetGlobalPad());
        FireCameraRumbleFilter(0.0f, 0.2f);
        mActionHitReactActionVars.bDoFrameLock = false;
    }

    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x653C | 0x8002D074 | size: 0x11C
 */
void cFielder::InitActionIdleTurn(unsigned short desiredFacingDirection)
{
    m_aDesiredFacingDirection = desiredFacingDirection;
    u16 angleDiff = (desiredFacingDirection - m_aActualFacingDirection) + 0x2000;

    if (angleDiff < 0x4000)
    {
        SetAnimState(0, true, 0.2f, false, false);
        InitMovementNone(65000.0f, 0.0f);
        m_aDesiredFacingDirection = desiredFacingDirection;
    }
    else
    {
        if (angleDiff < 0x8000)
        {
            SetAnimState(3, true, 0.2f, false, false);
        }
        else if (angleDiff < 0xC000)
        {
            SetAnimState(2, true, 0.2f, false, false);
        }
        else
        {
            SetAnimState(1, true, 0.2f, false, false);
        }
        InitMovementFromAnim(CalcAnimTurnAdjust(m_aActualFacingDirection, m_aDesiredFacingDirection, m_eAnimID), v3Zero, 1.0f, false);
    }

    SetAction(ACTION_IDLE_TURN);
}

/**
 * Offset/Address/Size: 0x64A4 | 0x8002CFDC | size: 0x98
 */
void cFielder::ActionIdleTurn(float)
{
    if (m_eAnimID == 0)
    {
        s16 angleDiff = (u16)abs_s16(m_aDesiredFacingDirection - m_aActualFacingDirection);

        if (angleDiff < g_IdleTurnCompletionDelta)
        {
            SetAction(ACTION_NEED_ACTION);
        }
    }
    else
    {
        bool shouldSetAction = false;
        cPN_SAnimController* ctrl = m_pCurrentAnimController;

        if (ctrl->m_ePlayMode == PM_HOLD)
        {
            if (ctrl->m_fTime == 0.0f)
            {
                shouldSetAction = true;
            }
        }

        if (shouldSetAction)
        {
            SetAction(ACTION_NEED_ACTION);
        }
    }
}

static inline void SetFacingAnim(cFielder* pFielder, const u16& facingDelta, const int* animIDs)
{
    // facingDelta += 0x2000;
    int animID = animIDs[facingDelta >> 14];
    pFielder->SetAnimState(animID, true, 0.2f, false, false);
}

/**
 * Offset/Address/Size: 0x61A0 | 0x8002CCD8 | size: 0x304
 */
void cFielder::InitActionLateOneTimerFromVolley()
{
    DoResetShotMeter(0.0f);

    ShotMeter* pShotMeter = m_pShotMeter;
    pShotMeter->CalcOneTimerValue(this, UsePerfectPass());

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_LATE_ONETIMER_FROM_VOLLEY);

    int LateOneTimerFromVolleyAnims[4] = {
        0x50,
        0x53,
        0x52,
        0x51,
    };

    s16 facingDelta = GetFacingDeltaToPosition(m_pTeam->GetOtherNet()->m_baseLocation);
    SetAnimState(LateOneTimerFromVolleyAnims[(u16)(facingDelta + 0x2000) >> 14], true, 0.2f, false, false);

    InitMovementFromAnim(0, v3Zero, 0.0f, false);

    if (ShouldIClearBall())
    {
        DoClearBall();
        return;
    }

    DoRegularShooting();

    bool bIsOneTimerShot = false;
    if (g_pBall->m_tShotTimer.m_uPackedTime != 0 && g_pBall->m_unk_0xA4 != 0)
    {
        bIsOneTimerShot = true;
    }

    if (bIsOneTimerShot)
    {
        EmitBallShot(this, BALL_EFFECT_PERFECT_SHOT, nullptr, false);
        FireCameraRumbleFilter(0.0f, 0.2f);
        Play3DSFX(Audio::eCharSFX(0x3D), VECTORS, 100.0f);
        return;
    }

    EmitBallShot(this, BALL_EFFECT_ONETIMER_SHOT, nullptr, false);
}

/**
 * Offset/Address/Size: 0x615C | 0x8002CC94 | size: 0x44
 */
void cFielder::ActionLateOneTimerFromVolley(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x5DF0 | 0x8002C928 | size: 0x36C
 */
/**
 * TODO: 95.00% match (scratch hvdpW) - 10 register diffs in fMaxSimulatedTime
 * int-to-float conversion (offsets 60-80): f0/f1/f2 cyclically rotated by 1.
 * Deep MWCC register allocator quirk, not fixable via C-level changes.
 */
void cFielder::DoCommonInitActionLooseBall(const nlVector3& rv3OneTimerTarget)
{
    nlVector3 v3SimulatedBallPos;
    nlVector3 v3ContactOffsetLocal;
    nlVector3 v3ContactOffsetWorld;
    nlVector3 v3MoveAdjustment;
    float fPhysicsRadius;
    float fCos;
    float fSin;

    cSAnim* pLeadGroundContactAnim = m_pAnimInventory->GetAnim(GetOneTimerLeadGroundContactAnims()->nAnimID);
    float fMaxSimulatedTime = 0.6f * (GetOneTimerLeadGroundContactAnims()->fAnimContactFrame / (float)pLeadGroundContactAnim->m_nNumKeys);

    float fSimulatedTime = 0.0f;

    FakeBallWorld::ResetBallIterator();

    while (fSimulatedTime < fMaxSimulatedTime)
    {
        FakeBallWorld::GetNextBallPosition(v3SimulatedBallPos);
        fSimulatedTime += FixedUpdateTask::GetPhysicsUpdateTick();

        if (ClipPositionToSidelines(v3SimulatedBallPos, 0.18f))
        {
            fSimulatedTime = fMaxSimulatedTime;
            break;
        }
    }

    LooseBallContactAnimInfo* pBestBallContactAnimInfo = GetOneTimerBallContactAnimInfo(m_aActualFacingDirection, m_v3Position, rv3OneTimerTarget, true, false);

    u16 aDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(rv3OneTimerTarget.f.y - m_v3Position.f.y, rv3OneTimerTarget.f.x - m_v3Position.f.x));

    switch (pBestBallContactAnimInfo->nAnimID)
    {
    case 0x45:
        aDesiredFacingDirection += 0x4000;
        break;
    case 0x46:
        aDesiredFacingDirection -= 0x4000;
        break;
    case 0x47:
        aDesiredFacingDirection += 0x8000;
        break;
    }

    SetFacingDirection(aDesiredFacingDirection);

    cSAnim* pBestContactAnim = m_pAnimInventory->GetAnim(pBestBallContactAnimInfo->nAnimID);

    GetJointPositionFuture(
        &v3ContactOffsetLocal,
        pBestBallContactAnimInfo->nAnimID,
        m_nBallJointIndex,
        pBestBallContactAnimInfo->fAnimContactFrame / (float)pBestContactAnim->m_nNumKeys,
        true,
        true,
        false);

    nlSinCos(&fSin, &fCos, m_aActualFacingDirection);

    nlVector3* pContactOffsetWorld = &v3ContactOffsetWorld;
    float xSin = v3ContactOffsetLocal.f.x * fSin;
    float ySin = v3ContactOffsetLocal.f.y * fSin;
    pContactOffsetWorld->f.z = v3ContactOffsetLocal.f.z;
    pContactOffsetWorld->f.x = (v3ContactOffsetLocal.f.x * fCos) - ySin;
    pContactOffsetWorld->f.y = (v3ContactOffsetLocal.f.y * fCos) + xSin;

    mActionOneTimerVars.fOneTimerAnimTime = pBestBallContactAnimInfo->fAnimContactFrame / (float)pBestContactAnim->m_nNumKeys;

    SetAnimState(pBestBallContactAnimInfo->nAnimID, true, 0.2f, false, false);

    m_pCurrentAnimController->m_fPlaybackSpeedScale = (pBestBallContactAnimInfo->fAnimContactFrame / 30.0f)
                                                    / (fSimulatedTime + FixedUpdateTask::GetPhysicsUpdateTick());

    v3MoveAdjustment.f.y = (v3SimulatedBallPos.f.y - pContactOffsetWorld->f.y) - m_v3Position.f.y;
    v3MoveAdjustment.f.z = (v3SimulatedBallPos.f.z - pContactOffsetWorld->f.z) - m_v3Position.f.z;
    v3MoveAdjustment.f.x = (v3SimulatedBallPos.f.x - pContactOffsetWorld->f.x) - m_v3Position.f.x;

    InitMovementFromAnim(0, v3MoveAdjustment, mActionOneTimerVars.fOneTimerAnimTime, false);

    m_pPhysicsCharacter->m_pPlayerPlayerColumn->GetRadius(&fPhysicsRadius);

    float fMaxGoalX = cField::GetGoalLineX(1U) - 0.5f;
    float fNetWidth = cNet::m_fNetWidth;
    float fMaxGoalY = (0.5f * fNetWidth) + 1.5f;
    float fMinGoalY = ((0.5f * fNetWidth) - cNet::GetPostRadius()) - fPhysicsRadius;

    float fAbsX = (float)fabs(v3SimulatedBallPos.f.x);
    if ((fAbsX < fMaxGoalX)
        || ((float)fabs(v3SimulatedBallPos.f.y) > fMaxGoalY)
        || ((float)fabs(v3SimulatedBallPos.f.y) < fMinGoalY))
    {
        m_pPhysicsCharacter->m_CanCollideWithWall = false;
    }
}

/**
 * Offset/Address/Size: 0x5BEC | 0x8002C724 | size: 0x204
 */
void cFielder::InitActionLooseBallPass(cFielder* pPassTarget, bool bVolleyPass)
{
    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_LOOSE_BALL_PASS);

    cPlayer* finalPassTarget;
    if (pPassTarget != NULL)
    {
        finalPassTarget = pPassTarget;
    }
    else
    {
        finalPassTarget = DoFindBestPassTarget(false, false);
    }

    mActionLooseBallPassVars.passTarget = finalPassTarget;
    mActionLooseBallPassVars.bVolleyPass = bVolleyPass;

    DoCommonInitActionLooseBall(mActionLooseBallPassVars.passTarget->m_v3Position);

    m_bCanTestController = false;
    SetNoPickUpTime(3.0f);
}

/**
 * Offset/Address/Size: 0x5B40 | 0x8002C678 | size: 0xAC
 */
void cFielder::ActionLooseBallPass(float)
{
    if (m_pCurrentAnimController->TestTrigger(mActionOneTimerVars.fOneTimerAnimTime))
    {
        if (CanPickupBallFromPass(g_pBall))
        {
            if (!IsOnSameTeam(g_pBall->m_pPassTarget))
            {
                g_pBall->ClearPassTarget();
            }

            DoRegularPassing(mActionLooseBallPassVars.passTarget, mActionLooseBallPassVars.bVolleyPass, true, false, false);
        }
    }

    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x5924 | 0x8002C45C | size: 0x21C
 */
void cFielder::InitActionLooseBallShot(bool bIsChipShot)
{
    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_LOOSE_BALL_SHOT);

    mActionLooseBallShotVars.bIsChipShot = bIsChipShot;

    DoCommonInitActionLooseBall(m_pTeam->GetOtherNet()->m_baseLocation);

    SetNoPickUpTime(3.0f);
    DoResetShotMeter(0.0f);

    ShotMeter* pShotMeter = m_pShotMeter;
    pShotMeter->CalcOneTimerValue(this, UsePerfectPass());
}

/**
 * Offset/Address/Size: 0x58E0 | 0x8002C418 | size: 0x44
 */
void cFielder::ActionLooseBallShot(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x57BC | 0x8002C2F4 | size: 0x124
 */
void cFielder::InitActionOneTimer(int animID, nlVector3& targetPos, float fAdjustEndTime, bool bIsChipShot)
{
    g_pGame->DoPerfectPassSlowDown();

    if (((u32)animID - 0x48) > 7)
    {
        mActionShotVars.bIsChipShot = bIsChipShot;
    }
    else
    {
        mActionShotVars.bIsChipShot = false;
    }

    SetAction(ACTION_ONETIMER);
    mActionOneTimerVars.fOneTimerAnimTime = fAdjustEndTime;
    SetAnimState(animID, true, 0.2f, false, false);

    nlVector3 v3Direction;
    nlVec3Set(v3Direction, targetPos.f.x - m_v3Position.f.x, targetPos.f.y - m_v3Position.f.y, targetPos.f.z - m_v3Position.f.z);

    InitMovementFromAnim(0, v3Direction, fAdjustEndTime, false);
    m_aDesiredFacingDirection = m_aActualFacingDirection;

    if (m_eCharacterClass == DONKEYKONG && m_ePowerup == POWER_UP_NONE)
    {
        ClearPowerupAnimState(false);
    }
}

/**
 * Offset/Address/Size: 0x5778 | 0x8002C2B0 | size: 0x44
 */
void cFielder::ActionOneTimer(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x5518 | 0x8002C050 | size: 0x260
 */
void cFielder::InitActionOneTouchPassFromVolley(cPlayer* pPlayer)
{
    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_ONETOUCH_PASS_FROM_VOLLEY);

    int LateOneTimerFromVolleyAnims[4] = {
        0x50,
        0x53,
        0x52,
        0x51,
    };

    s16 facingDelta = GetFacingDeltaToPosition(pPlayer->m_v3Position);
    SetAnimState(LateOneTimerFromVolleyAnims[(u16)(facingDelta + 0x2000) >> 14], true, 0.2f, false, false);

    InitMovementFromAnim(0, v3Zero, 0.0f, false);

    DoRegularPassing(pPlayer, false, true, true, true);
}

/**
 * Offset/Address/Size: 0x54D4 | 0x8002C00C | size: 0x44
 */
void cFielder::ActionOneTouchPassFromVolley(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x530C | 0x8002BE44 | size: 0x1C8
 */
bool cFielder::DoCalcCanDoPerfectPass(cFielder* pPassTarget, const nlVector3& v3PassPosition)
{
    const nlVector3& v3OffNetLocPos = pPassTarget->GetAIOffNetLocation(NULL);
    float fDistSqPosToOffNet = CalculateDistanceSquared(v3PassPosition, v3OffNetLocPos);
    const nlVector3& v3OffNetLocThis = GetAIOffNetLocation(NULL);

    float fMaxDistToNet = 14.0f;
    float fMinDistToPassTarget = 4.0f;
    float fDistSqThisToOffNet = CalculateDistanceSquared(m_v3Position, v3OffNetLocThis);
    float fDistSqBallToPos = CalculateDistanceSquared(v3PassPosition, g_pBall->m_v3Position);
    float fPasserMaxDistToNet = fMaxDistToNet;

    fMaxDistToNet *= fMaxDistToNet;
    float fPasserMaxDistToNetSq = fPasserMaxDistToNet;
    fPasserMaxDistToNetSq *= fPasserMaxDistToNetSq;
    fMinDistToPassTarget *= fMinDistToPassTarget;

    float fPasseeScore = 0.66999996f * OpenToTheirNet(pPassTarget);
    float fPasserScore = 0.33f * Open(pPassTarget);

    bool result = false;
    if (((fPasseeScore + fPasserScore) > 0.72f)
        && (fDistSqBallToPos > fMinDistToPassTarget)
        && (fDistSqPosToOffNet < fMaxDistToNet)
        && (fDistSqThisToOffNet < fPasserMaxDistToNetSq))
    {
        result = true;
    }
    return result;
}

/**
 * Offset/Address/Size: 0x50A4 | 0x8002BBDC | size: 0x268
 * TODO: 99.87% match - `PassingAnims` address load still uses `r4` base instead of `r3`
 */
void cFielder::InitActionPass(cPlayer* pPassTarget, bool bVolleyPass, bool bAllowLeadPass)
{
    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_PASS);

    s16 facingDelta = GetFacingDeltaToPosition(pPassTarget->m_v3Position);
    SetAnimState(PassingAnims[(u16)(facingDelta + 0x2000) >> 14], true, 0.2f, false, false);

    InitMovementCoast();

    if (bVolleyPass)
    {
        nlVector3 delta;
        nlVec3Sub(delta, m_v3Position, pPassTarget->m_v3Position);
        float minDistSq = g_pGame->m_pGameTweaks->fVolleyPassMinDistance;
        minDistSq *= minDistSq;
        float distSq = delta.GetLengthSq3D();

        if (distSq < minDistSq)
        {
            bVolleyPass = false;
        }
    }

    mActionPassingVars.bVolleyPass = bVolleyPass;
    mActionPassingVars.pPassTarget = pPassTarget;
    mActionPassingVars.bAllowLeadPass = !bAllowLeadPass;
}

/**
 * Offset/Address/Size: 0x5020 | 0x8002BB58 | size: 0x84
 */
void cFielder::ActionPass(float)
{
    if (m_pBall != nullptr)
    {
        if (m_pCurrentAnimController->TestTrigger(0.12f))
        {
            DoRegularPassing(mActionPassingVars.pPassTarget, mActionPassingVars.bVolleyPass, mActionPassingVars.bAllowLeadPass, false, false);
        }
    }

    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x4F98 | 0x8002BAD0 | size: 0x88
 */
void cFielder::InitActionPostWhistle()
{
    SetAction(ACTION_POST_WHISTLE);
    SetAnimState(0, false, 0.0f, false, false);
    InitMovementNone(0.0f, 0.0f);
    m_aDesiredFacingDirection = m_aActualFacingDirection;
    m_fActualSpeed = 0.0f;
    SetVelocity(v3Zero);
    m_pShotMeter->Abort(this);
}

/**
 * Offset/Address/Size: 0x4F94 | 0x8002BACC | size: 0x4
 */
void cFielder::ActionPostWhistle(float)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x4C38 | 0x8002B770 | size: 0x35C
 */
void cFielder::InitActionBombReact(const nlVector3& v3BombPosition, float fRadius)
{
    CleanUpPowerupEffect();

    if (g_pBall->m_pOwner == this)
    {
        ReleaseBall();
        ShootBallDueToContact(m_aActualFacingDirection);
    }

    BeginRumbleAction(RUMBLE_MEDIUM_CONTACT, GetGlobalPad());

    nlVector3 delta;
    nlVec3Sub(delta, m_v3Position, v3BombPosition);
    float fDistance = nlSqrt(delta.GetLengthSq3D(), true) - fRadius;

    if (fDistance > 1.0f && !IsFallenDown(0.0f))
    {
        InitActionBombHitReact(v3BombPosition);
        return;
    }

    if (!IsFallenDown(0.0f))
    {
        PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk24C);
    }

    if (IsFrozen())
    {
        m_tFrozenTimer.SetSeconds(0.0f);
        EmitUnFreeze(this);
    }

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_BOMB_REACT);

    s16 facingDelta = GetFacingDeltaToPosition(v3BombPosition);
    u16 absFacingDelta = (u16)abs_s16(facingDelta);

    if (absFacingDelta < 0x4000)
    {
        SetAnimState(0x6C, true, 0.2f, false, false);
    }
    else
    {
        SetAnimState(0x6D, true, 0.2f, false, false);
    }

    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0x4968 | 0x8002B4A0 | size: 0x2D0
 * TODO: Just one reg mismatch on SetFacingAnim
 */
void cFielder::InitActionBombHitReact(const nlVector3& v3BombPosition)
{
    FORCE_DONT_INLINE;
    CleanUpPowerupEffect();

    if (IsFrozen())
    {
        m_tFrozenTimer.SetSeconds(0.0f);
        EmitUnFreeze(this);
    }

    mActionHitReactActionVars.bDoFrameLock = false;

    if (!IsFallenDown(0.0f))
    {
        PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk250);
    }

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_HIT_REACT);

    u32 index = (((u16)((u16)GetFacingDeltaToPosition(v3BombPosition)) >> 14) & 3);
    SetAnimState(g_HitReactInfo[index].nAnimID, true, 0.2f, false, false);

    float angleRad = nlATan2f(m_v3Position.f.y - v3BombPosition.f.y, m_v3Position.f.x - v3BombPosition.f.x);
    u16 targetAngle = (u16)(s32)(10430.378f * angleRad);
    SetFacingDirection(targetAngle + g_HitReactInfo[index].aFacingDirectionOffset);

    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0x46D0 | 0x8002B208 | size: 0x298
 */
void cFielder::InitActionBananaReact(const nlVector3&)
{
    CleanUpPowerupEffect();

    u16 angleDiff = (u16)abs_s16(m_aActualFacingDirection - m_aActualMovementDirection);
    if (angleDiff < 0x4000)
    {
        SetAnimState(0x6A, true, 0.2f, false, false);
    }
    else
    {
        SetAnimState(0x6B, true, 0.2f, false, false);
    }

    if (g_pBall->m_pOwner == this)
    {
        ReleaseBall();
        ShootBallDueToContact(m_aActualFacingDirection);
    }

    BeginRumbleAction(RUMBLE_MEDIUM_CONTACT, GetGlobalPad());

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_BANANA_REACT);

    InitMovementFromAnim(0, v3Zero, 1.0f, false);
    PlayRandomCharDialogue(1, VECTORS, 100.0f, -1.0f);

    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0x4468 | 0x8002AFA0 | size: 0x268
 * TODO: Just one reg mismatch on SetFacingAnim
 */
void cFielder::InitActionShellReact(const nlVector3& v3CollisionLocation, const nlVector3& v3CollisionVelocity)
{
    CleanUpPowerupEffect();

    if (g_pBall->m_pOwner == this)
    {
        ReleaseBall();
        ShootBallDueToContact(v3CollisionVelocity);
    }

    BeginRumbleAction(RUMBLE_MEDIUM_CONTACT, GetGlobalPad());

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_SHELL_REACT);

    u16 facingDelta = (u16)(GetFacingDeltaToPosition(v3CollisionLocation) + 0x2000);
    SetFacingAnim(this, facingDelta, ShellAttackReactAnims);

    // s16 facingDelta = GetFacingDeltaToPosition(v3CollisionLocation);
    // SetAnimState(ShellAttackReactAnims[(u16)(facingDelta + 0x2000) >> 14], true, 0.2f, false, false);

    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0x4400 | 0x8002AF38 | size: 0x68
 */
void cFielder::InitActionRunning()
{
    m_pHeadTrack->m_bTrackOOI = true;

    if (m_eActionState != ACTION_RUNNING)
    {
        mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
        m_aActualMovementDirection = m_aActualFacingDirection;
        m_aDesiredFacingDirection = m_aActualFacingDirection;
        m_aDesiredMovementDirection = m_aActualMovementDirection;
        m_fDesiredSpeed = m_fActualSpeed;
        mActionRunningVars.bFirstCycleOfTurbo = false;
    }

    SetAction(ACTION_RUNNING);
}

/**
 * Offset/Address/Size: 0x435C | 0x8002AE94 | size: 0xA4
 */
void cFielder::ActionRunning(float dt)
{
    if (m_pBall != nullptr)
    {
        SetAction(ACTION_RUNNING_WB);
        mActionRunningWBVars.bWaitForAnimToFinish = false;
        mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
        m_aActualMovementDirection = m_aActualFacingDirection;
    }
    else
    {
        asmRunning();

        if (CanPickupBall(g_pBall))
        {
            PickupBall(g_pBall);
            SetAction(ACTION_RUNNING_WB);
            mActionRunningWBVars.bWaitForAnimToFinish = false;
            mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
            m_aActualMovementDirection = m_aActualFacingDirection;
        }
    }
}

/**
 * Offset/Address/Size: 0x430C | 0x8002AE44 | size: 0x50
 */
void cFielder::InitActionRunningWB(bool bWaitForAnimToFinish)
{
    SetAction(ACTION_RUNNING_WB);
    mActionRunningWBVars.bWaitForAnimToFinish = bWaitForAnimToFinish;
    mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
    m_aActualMovementDirection = m_aActualFacingDirection; // m_aDesiredMovementDirection
}

/**
 * Offset/Address/Size: 0x42D4 | 0x8002AE0C | size: 0x38
 */
void cFielder::ActionRunningWB(float dt)
{
    if (m_pBall == nullptr)
    {
        SetAction(ACTION_NEED_ACTION);
    }
    else
    {
        asmRunningWB(dt);
    }
}

/**
 * Offset/Address/Size: 0x3F28 | 0x8002AA60 | size: 0x3AC
 */
void cFielder::ActionRunningWBTurbo(float fDeltaT)
{
    cGlobalPad* pPad = GetGlobalPad();
    if (pPad != NULL && m_pBall != NULL)
    {
        if (m_pController->IsTurboPressed())
        {
            if (m_pController->GetMovementStickMagnitude() > 0.001f)
            {
                bool bShotMeterActive = false;
                eShotMeterState state = m_pShotMeter->m_eShotMeterState;
                if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
                {
                    bShotMeterActive = true;
                }

                if (!bShotMeterActive)
                    goto setTurboSpeed;
            }
        }

        m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed;
        goto afterSpeed;

    setTurboSpeed:
        m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel1;
    }

afterSpeed:
    if (m_fDesiredSpeed > ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed + 0.1f)
    {
        switch (m_eAnimID)
        {
        case 0x1D:
        default:
            m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel1;
            break;
        case 0x1E:
            m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel2;
            break;
        case 0x1F:
            m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel3;
            break;
        }
    }

    if (m_pCurrentAnimController->m_fTime > 0.5f || m_pCurrentAnimController->m_fTime < -0.5f)
    {
        if (m_pBall != NULL)
        {
            u16 aNewFacingDirection = SeekDirection(
                m_aActualFacingDirection,
                m_aDesiredFacingDirection,
                ((FielderTweaks*)m_pTweaks)->fRunningWBTurboDirectionSeekSpeed,
                ((FielderTweaks*)m_pTweaks)->fRunningWBTurboDirectionSeekFalloff,
                fDeltaT);

            SetFacingDirection(aNewFacingDirection);
            m_aActualMovementDirection = aNewFacingDirection;

            s16 nFacingDelta = (s16)(m_aDesiredFacingDirection - m_aActualFacingDirection);

            if (TestQueuedActions())
            {
                m_eLastPadAction = PAD_NONE;
                goto done;
            }

            if (m_fDesiredSpeed < ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel1 - 0.1f
                || m_ePowerup == POWER_UP_MUSHROOM || m_ePowerup == POWER_UP_STAR)
            {
                SetAction(ACTION_RUNNING_WB);
                mActionRunningWBVars.bWaitForAnimToFinish = false;
                mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
                m_aActualMovementDirection = m_aActualFacingDirection;
                goto done;
            }

            if (m_pCurrentAnimController->m_fTime > 0.5f)
            {
                int absDelta = nFacingDelta;
                if (nFacingDelta < 0)
                    absDelta = -nFacingDelta;

                if ((u16)absDelta > 0x2000)
                {
                    SetAction(ACTION_RUNNING_WB_TURBO_TURN);

                    static int RunningWBTurboTurnAnim[4] = { 0x22, 0x21, 0x22, 0x20 };

                    m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboTurnSpeed;
                    int facingDiff = m_aDesiredFacingDirection - m_aActualFacingDirection;
                    SetAnimState(RunningWBTurboTurnAnim[((facingDiff + 0x2000) >> 14) & 3], true, 0.2f, false, false);

                    InitMovementFromAnim(
                        CalcAnimTurnAdjust(m_aActualFacingDirection, m_aDesiredFacingDirection, m_eAnimID),
                        v3Zero,
                        1.0f,
                        false);
                }
                else
                {
                    switch (m_eAnimID)
                    {
                    case 0x1D:
                        SetRunTurboAnimState(0x1E, mActionRunningWBTurboVars.bForcedMirrorSwap);
                        break;
                    case 0x1E:
                        SetRunTurboAnimState(0x1F, mActionRunningWBTurboVars.bForcedMirrorSwap);
                        break;
                    case 0x1F:
                        SetRunTurboAnimState(0x1F, mActionRunningWBTurboVars.bForcedMirrorSwap);
                        break;
                    default:
                        SetRunTurboAnimState(0x1D, false);
                        break;
                    }
                }
            }

            goto done;
        }
    }

    if (m_pBall == NULL)
    {
        if (ShouldStartCrossBlend(7))
        {
            SetAction(ACTION_NEED_ACTION);
        }
    }

done:
    switch (m_eAnimID)
    {
    case 0x1D:
    default:
        m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel1;
        break;
    case 0x1E:
        m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel2;
        break;
    case 0x1F:
        m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel3;
        break;
    }
}

/**
 * Offset/Address/Size: 0x3CA8 | 0x8002A7E0 | size: 0x280
 * TODO: 95.41% match - remaining diffs in turbo-speed branch inversion and
 * r3/r4 register allocation in mirror-swap logic before SetRunTurboAnimState.
 */
void cFielder::ActionRunningWBTurboTurn(float fDeltaT)
{
    if (m_pBall == nullptr)
    {
        if (ShouldStartCrossBlend(7))
        {
            SetAction(ACTION_NEED_ACTION);
        }
    }
    else
    {
        if (GetGlobalPad() != nullptr)
        {
            if (m_pController->IsTurboPressed())
            {
                if (m_pController->GetMovementStickMagnitude() > 0.001f)
                {
                    bool bShotMeterActive = false;
                    eShotMeterState state = m_pShotMeter->m_eShotMeterState;
                    if (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION)
                    {
                        bShotMeterActive = true;
                    }

                    if (!bShotMeterActive)
                        goto setTurboSpeed;
                }
            }

            m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBSpeed;
            goto testButtons;

        setTurboSpeed:
            m_fDesiredSpeed = ((FielderTweaks*)m_pTweaks)->fRunningWBTurboSpeedLevel1;

        testButtons:
            TestButtonsToQueueActions(fDeltaT);
        }

        if (ShouldStartCrossBlend(7))
        {
            if (TestQueuedActions())
            {
                m_eLastPadAction = PAD_NONE;
            }
            else if (m_fDesiredSpeed > (((FielderTweaks*)m_pTweaks)->fRunningWBSpeed + 0.1f))
            {
                m_aDesiredMovementDirection = m_aActualFacingDirection;
                m_aActualMovementDirection = m_aDesiredMovementDirection;

                if (m_ePowerup == POWER_UP_MUSHROOM || m_ePowerup == POWER_UP_STAR)
                {
                    SetAction(ACTION_RUNNING_WB);
                    mActionRunningWBVars.bWaitForAnimToFinish = false;
                    mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
                    m_aActualMovementDirection = m_aActualFacingDirection;
                }
                else
                {
                    SetAction(ACTION_RUNNING_WB_TURBO);
                    InitMovementRunningNoTurn(((FielderTweaks*)m_pTweaks)->fRunningWBTurboAccel, ((FielderTweaks*)m_pTweaks)->fRunningWBTurboDecel);

                    bool bForceMirrorSwap;
                    cPN_SAnimController* pController = m_pCurrentAnimController;
                    if (!pController->m_bMirror)
                    {
                        bForceMirrorSwap = false;
                        if (m_eAnimID == 0x1A)
                        {
                            if (pController->m_fTime > 0.25f && pController->m_fTime < 0.75)
                            {
                                bForceMirrorSwap = true;
                            }
                        }
                        mActionRunningWBTurboVars.bForcedMirrorSwap = bForceMirrorSwap;
                    }
                    else
                    {
                        bForceMirrorSwap = false;
                        if (m_eAnimID == 0x1A)
                        {
                            if (pController->m_fTime < 0.25f || pController->m_fTime > 0.75)
                            {
                                bForceMirrorSwap = true;
                            }
                        }
                        mActionRunningWBTurboVars.bForcedMirrorSwap = bForceMirrorSwap;
                    }

                    SetRunTurboAnimState(0x1D, mActionRunningWBTurboVars.bForcedMirrorSwap);
                }
            }
            else
            {
                m_aDesiredMovementDirection = m_aActualFacingDirection;
                m_aActualMovementDirection = m_aDesiredMovementDirection;
                SetAction(ACTION_RUNNING_WB);
                mActionRunningWBVars.bWaitForAnimToFinish = false;
                mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
                m_aActualMovementDirection = m_aActualFacingDirection;
                InitMovementRunning(m_pTweaks->fRunningDirectionSeekSpeed, m_pTweaks->fRunningDirectionSeekFalloff, ((FielderTweaks*)m_pTweaks)->fRunningAccel, ((FielderTweaks*)m_pTweaks)->fRunningDecel);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x383C | 0x8002A374 | size: 0x46C
 */
void cFielder::InitActionShot(bool bIsChipShot)
{
    if (m_pBall == nullptr)
    {
        m_pHeadTrack->m_bTrackOOI = true;

        if (m_eActionState != ACTION_RUNNING)
        {
            mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
            m_aActualMovementDirection = m_aActualFacingDirection;
            m_aDesiredFacingDirection = m_aActualFacingDirection;
            m_aDesiredMovementDirection = m_aActualMovementDirection;
            m_fDesiredSpeed = m_fActualSpeed;
            mActionRunningVars.bFirstCycleOfTurbo = false;
        }

        SetAction(ACTION_RUNNING);
        return;
    }

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_SHOT);

    mActionShotVars.bIsChipShot = bIsChipShot;

    switch (m_eAnimID)
    {
    case 0x56:
    {
        if (!(m_pShotMeter->m_fSpeedValue >= 0.99f))
        {
            SetAnimState(0x58, true, 0.2f, false, false);
        }
        else
        {
            SetAnimState(0x5A, true, 0.2f, false, false);
        }
        break;
    }
    case 0x57:
    {
        if (!(m_pShotMeter->m_fSpeedValue >= 0.99f))
        {
            SetAnimState(0x59, true, 0.2f, false, false);
        }
        else
        {
            SetAnimState(0x5B, true, 0.2f, false, false);
        }
        break;
    }
    default:
    {
        if (GetFacingDeltaToPosition(m_pTeam->GetOtherNet()->m_baseLocation) < 0)
        {
            if (!(m_pShotMeter->m_fSpeedValue >= 0.99f))
            {
                SetAnimState(0x59, true, 0.2f, false, false);
            }
            else
            {
                SetAnimState(0x5B, true, 0.2f, false, false);
            }
        }
        else
        {
            if (!(m_pShotMeter->m_fSpeedValue >= 0.99f))
            {
                SetAnimState(0x58, true, 0.2f, false, false);
            }
            else
            {
                SetAnimState(0x5A, true, 0.2f, false, false);
            }
        }
        break;
    }
    }
    m_fDesiredSpeed = 0.0f;

    static float shootingSeekSpeed = 300000.0f;
    static float shootingSeekFalloff = 4000.0f;

    InitMovementNone(shootingSeekSpeed, shootingSeekFalloff);

    nlVector3 pos = m_pTeam->GetOtherNet()->m_baseLocation;
    m_aDesiredFacingDirection = 10430.378f * nlATan2f(pos.f.y - m_v3Position.f.y, pos.f.x - m_v3Position.f.x);
}

/**
 * Offset/Address/Size: 0x36B8 | 0x8002A1F0 | size: 0x184
 */
void cFielder::ActionShot(float)
{
    if (m_pBall != nullptr && m_pCurrentAnimController->TestFrameTrigger(1.825f))
    {
        if (!ShouldIClearBall())
        {
            bool bIsChipShot = false;
            if (mActionShotVars.bIsChipShot || mActionLooseBallShotVars.bIsChipShot)
            {
                bIsChipShot = true;
            }

            if (bIsChipShot)
            {
                EmitBallShot(this, BALL_EFFECT_CHIP_SHOT, nullptr, false);
            }
            else
            {
                if (m_pShotMeter->m_fSpeedValue >= 0.99f)
                {
                    EmitBallShot(this, BALL_EFFECT_PERFECT_SHOT, nullptr, false);
                    FireCameraRumbleFilter(0.0f, 0.2f);
                    Play3DSFX(Audio::eCharSFX(0x38), VECTORS, 100.0f);
                }
                else if (m_tBallPossessionTimer.GetSeconds() < 0.1f)
                {
                    EmitBallShot(this, BALL_EFFECT_ONETIMER_SHOT, nullptr, false);
                }
                else
                {
                    EmitBallShot(this, BALL_EFFECT_REGULAR_SHOT, nullptr, false);
                }
            }
            DoRegularShooting();
        }
        else
        {
            DoClearBall();
        }
        InitMovementFromAnim(0, v3Zero, 1.0f, false);
    }

    if (ShouldStartCrossBlend(7))
    {
        m_pHeadTrack->m_bTrackOOI = false;
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x3180 | 0x80029CB8 | size: 0x538
 * TODO: 99.76% match - f30/f31 callee-saved FP register allocation swap for fAbsPosX
 */
void cFielder::InitActionShootToScore()
{
    ActionShootToScoreVars stsVars;
    memset(&stsVars, 0, sizeof(stsVars));
    mActionShootToScoreVars = stsVars;

    m_pCharacterSFX->StopMovementLoop();

    if (m_pBall == NULL)
    {
        m_pShotMeter->Abort(this);
        m_pHeadTrack->m_bTrackOOI = true;

        if (m_eActionState != ACTION_RUNNING)
        {
            mActionRunningVars.eLastStrafeDirection = STRAFE_IDLE;
            m_aActualMovementDirection = m_aActualFacingDirection;
            m_aDesiredFacingDirection = m_aActualFacingDirection;
            m_aDesiredMovementDirection = m_aActualMovementDirection;
            m_fDesiredSpeed = m_fActualSpeed;
            mActionRunningVars.bFirstCycleOfTurbo = false;
        }

        SetAction(ACTION_RUNNING);
        return;
    }

    bool bCloseToGoalLine = false;
    f32 fAbsPosX = fabs(m_v3Position.f.x);
    f32 fGoalLineX = cField::GetGoalLineX(1U);
    if (fAbsPosX > fGoalLineX - 2.0f)
    {
        f32 fAbsPosY = fabs(m_v3Position.f.y);
        if (fAbsPosY < 0.5f * cNet::m_fNetWidth + 1.0f)
        {
            bCloseToGoalLine = true;
        }
    }

    if (g_pGame->m_eGameState == GS_END_GAME || bCloseToGoalLine)
    {
        KillWindup(this, "ball_sts_windup", true);
        InitActionShot(false);
        return;
    }

    KillWindup(this, "ball_sts_windup", true);

    if (IsCaptain() || nlSingleton<GameInfoManager>::s_pInstance->GetTeam((s16)m_pTeam->m_nSide) == 8)
    {
        mActionShootToScoreVars.isCaptainSts = true;
    }
    else
    {
        mActionShootToScoreVars.isCaptainSts = false;
    }

    mActionShootToScoreVars.isCurrentlyInvincible = false;
    mActionShootToScoreVars.isInUnbreakablePart = false;
    mActionShootToScoreVars.captainStsCamera = NULL;
    SetAction(ACTION_SHOOT_TO_SCORE);

    if (m_eAnimID == 0x56)
    {
        SetAnimState(0x5D, true, 0.2f, false, false);
    }
    else
    {
        SetAnimState(0x5E, true, 0.2f, false, false);
    }

    nlVector3 v3NetLocation = m_pTeam->GetOtherNet()->m_baseLocation;
    f32 fAngleRad = nlATan2f(v3NetLocation.f.y - m_v3Position.f.y, v3NetLocation.f.x - m_v3Position.f.x);
    u16 nAngleUnits = (u16)(s32)(10430.378f * fAngleRad);
    s16 nTurnAdjust = CalcAnimTurnAdjust(m_aActualFacingDirection, nAngleUnits, m_eAnimID);

    f32 fSpeed = 12.0f / (f32)m_pCurrentAnimController->m_pSAnim->m_nNumKeys;
    InitMovementFromAnim(nTurnAdjust, v3Zero, fSpeed, false);

    Play3DSFX(Audio::eCharSFX(0x1C), PHYSOBJ, 100.0f);

    mActionShootToScoreVars.fShootToScoreActiveTime = 0.0f;
    mActionShootToScoreVars.fFrameButtonDownTime1 = -1.0f;
    mActionShootToScoreVars.fFrameButtonDownTime2 = -1.0f;
    mActionShootToScoreVars.fGreenRegionWidth = g_pGame->m_pGameTweaks->unk29C;
    mActionShootToScoreVars.fMeterFractionTime = 0.0f;
    mActionShootToScoreVars.v3MeterPosition.f.x = 0.0f;
    mActionShootToScoreVars.v3MeterPosition.f.y = 0.0f;
    mActionShootToScoreVars.v3MeterPosition.f.z = 0.0f;
    mActionShootToScoreVars.bShootWasPressed = false;

    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    f32 fFraction = pTweaks->fS2S1stJumpFrame / (f32)m_pCurrentAnimController->m_pSAnim->m_nNumKeys;
    nlVector3 v3Dummy;
    u16 aDummy;
    GetCurrentAnimFuture(-1, fFraction, v3Dummy, mActionShootToScoreVars.v3MeterPosition, aDummy);

    mActionShootToScoreVars.v3MeterPosition.f.z += 0.01f;
    ShootToScoreMeter::instance.m_v3OriginalMeterPosition = mActionShootToScoreVars.v3MeterPosition;
    ShootToScoreMeter::instance.m_v3MeterPosition = mActionShootToScoreVars.v3MeterPosition;

    mActionShootToScoreVars.fCaptainYellowWidth = g_pGame->m_pGameTweaks->unk290;

    switch (m_eCharacterClass)
    {
    case DAISY:
    case PEACH:
        mActionShootToScoreVars.fCaptainYellowWidth *= 1.5f;
        break;
    case DONKEYKONG:
    case WARIO:
    case MYSTERY:
        mActionShootToScoreVars.fCaptainYellowWidth *= 0.75f;
        break;
    case WALUIGI:
    case YOSHI:
        mActionShootToScoreVars.fCaptainYellowWidth *= 1.25f;
        break;
    default:
        break;
    }

    ShootToScoreMeter::instance.TurnOnMeter(ShootToScoreMeter::REGULAR_SHOOT_TO_SCORE_PHASE1, mActionShootToScoreVars.fCaptainYellowWidth);

    f32 fMinAmount = g_pGame->m_pGameTweaks->unk1E8;
    f32 fMaxAmount = g_pGame->m_pGameTweaks->unk1EC;
    const nlVector3& v3OffNet = GetAIOffNetLocation(NULL);
    f32 dx = m_v3Position.f.x - v3OffNet.f.x;
    f32 dy = m_v3Position.f.y - v3OffNet.f.y;
    f32 dz = m_v3Position.f.z - v3OffNet.f.z;
    f32 fTimeScale = InterpolateRangeClamped(fMinAmount, fMaxAmount, 9.0f, 18.0f, nlSqrt(dx * dx + dy * dy + dz * dz, true));
    FixedUpdateTask::mTimeScale = fTimeScale;
    ParticleUpdateTask::SetTimeScale(fTimeScale);

    const Event* pEvent = g_pEventManager->CreateValidEvent(0x3F, 0x1C);
    ShotAtGoalData* pData = new ((u8*)pEvent + 0x10) ShotAtGoalData();
    pData->pShooter = this;
}

/**
 * Offset/Address/Size: 0x3158 | 0x80029C90 | size: 0x28
 */
void MatrixCamFinishedCallback(MatrixEffectCam*)
{
    const float timeScale = 1.0f;
    FixedUpdateTask::mTimeScale = timeScale;
    ParticleUpdateTask::SetTimeScale(timeScale);
}

/**
 * Offset/Address/Size: 0x2D08 | 0x80029840 | size: 0x450
 * TODO: 95.26% match - persistent r28/r29/r30 register-allocation drift around
 *       BasicString construction and a remaining Flash symbol relocation mismatch.
 */
void cFielder::SetupCaptainSTSAnimCam(bool arg1)
{
    extern void Flash__10PhotoFlashFv();

    mActionShootToScoreVars.captainStsCamera = new ((cAnimCamera*)nlMalloc(sizeof(cAnimCamera), 8, false)) cAnimCamera();

    BasicString<char, Detail::TempStringAllocator> cameraName = Format(BasicString<char, Detail::TempStringAllocator>("{0}_ShootToScoreCamera"),
        GetTeamName(nlSingleton<GameInfoManager>::s_pInstance
                ->mGameInfo[nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode]
                ->mTeamIndex[(s16)m_pTeam->m_nSide]));

    if (mActionShootToScoreVars.captainStsCamera->CameraAnimationExists(cameraName.c_str()))
    {
        mActionShootToScoreVars.captainStsCamera->SelectCameraAnimation(cameraName.c_str());
    }
    else
    {
        mActionShootToScoreVars.captainStsCamera->SelectCameraAnimation("ShootToScoreCamera");
    }

    mActionShootToScoreVars.captainStsCamera->m_bCyclic = false;

    captainStsTargetPos = m_v3Position;
    captainStsTargetPos.f.z = 0.0f;

    if (captainStsTargetPos.f.x < cField::GetGoalLineX(0U) + 6.0f)
    {
        captainStsTargetPos.f.x = cField::GetGoalLineX(0U) + 6.0f;
    }

    if (captainStsTargetPos.f.x > cField::GetGoalLineX(1U) - 6.0f)
    {
        captainStsTargetPos.f.x = cField::GetGoalLineX(1U) - 6.0f;
    }

    setCaptainStscaptainStsTargetPos = true;
    mActionShootToScoreVars.captainStsCamera->m_OffsetPos = captainStsTargetPos;

    if (captainStsTargetPos.f.x >= 0.0f)
    {
        mActionShootToScoreVars.captainStsCamera->mFacingAngle = m_aActualFacingDirection;
    }
    else
    {
        nlVector3 mirror = { -1.0f, 1.0f, 1.0f };
        mActionShootToScoreVars.captainStsCamera->m_Mirror = mirror;
        mActionShootToScoreVars.captainStsCamera->mFacingAngle = m_aActualFacingDirection + 0x8000;
    }

    mActionShootToScoreVars.captainStsCamera->m_bUseSimulationTime = true;

    if (arg1)
    {
        mActionShootToScoreVars.captainStsCamera->m_fAnimationTime = sfHyperStrikeAnimCamStartTime;
        mActionShootToScoreVars.captainStsCamera->BuildAnimViewMatrix(mActionShootToScoreVars.captainStsCamera->m_matView);
        Flash__10PhotoFlashFv();
        cCameraManager::PushCamera(mActionShootToScoreVars.captainStsCamera);
    }
    else
    {
        cCameraManager::PushCamera(mActionShootToScoreVars.captainStsCamera);
    }
}

/**
 * Offset/Address/Size: 0x2D04 | 0x8002983C | size: 0x4
 */
void OtherMatrixCamFinishedCallback(MatrixEffectCam*)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x2C4C | 0x80029784 | size: 0xB8
 */
void HyperStrikeEffectUpdate(EmissionController& controller)
{
    if (ReplayManager::Instance()->mRender != nullptr)
    {
        const u32 characterIndex = controller.m_uUserData;
        {
            DrawableCharacter& character = ReplayManager::Instance()->mRender->mCharacters[characterIndex];
            controller.SetPoseAccumulator(*character.mPoseAccumulator);
        }
        {
            DrawableCharacter& character = ReplayManager::Instance()->mRender->mCharacters[characterIndex];
            controller.SetAnimController(character.GetAnimController());
        }
    }

    nlVector3 viewVector;
    viewVector.f.x = cCameraManager::m_matView.f.m31;
    viewVector.f.y = cCameraManager::m_matView.f.m32;
    viewVector.f.z = cCameraManager::m_matView.f.m33;
    cCameraManager::GetViewVector(viewVector);
    controller.SetDirection(viewVector);
}

/**
 * Offset/Address/Size: 0x17D0 | 0x80028308 | size: 0x147C
 */
/**
 * Offset/Address/Size: 0x16D0 | 0x80028308 | size: 0x1480
 * TODO: 90.12% match - f30/f31 register swap (fHalfAnimationTime/fGreenRegionMaxWidth),
 * missing f25 callee-saved for fMinOverMax, missing r26 for g_pTeams iteration,
 * r4/r5 swap for pTweaks/isCaptainSts
 */
void cFielder::ActionShootToScore(float)
{
    if (setCaptainStscaptainStsTargetPos)
    {
        setCaptainStscaptainStsTargetPos = false;
        SetPosition(captainStsTargetPos);
    }

    unsigned int nTotalFrames = m_pCurrentAnimController->m_pSAnim->m_nNumKeys;
    FielderTweaks* pTweaks = (FielderTweaks*)m_pTweaks;
    bool isCaptainSts = mActionShootToScoreVars.isCaptainSts;
    float fTotalTime = (float)nTotalFrames;

    float fHalfAnimationTime;
    float fGreenRegionMaxWidth;
    float fShootToScore1stJumpTime = pTweaks->fS2S1stJumpFrame / fTotalTime;

    if (isCaptainSts)
    {
        fHalfAnimationTime = (1.0f + pTweaks->fCaptainS2SNisBeginFrame) / fTotalTime;
    }
    else
    {
        fHalfAnimationTime = (1.0f + pTweaks->fS2SKickFrame) / fTotalTime;
    }

    float fCaptainPercentage = pTweaks->fCaptainS2SNisBeginFrame / fTotalTime;

    if (isCaptainSts)
    {
        fGreenRegionMaxWidth = pTweaks->fCaptainS2SNisEndFrame / fTotalTime;
    }
    else
    {
        fGreenRegionMaxWidth = pTweaks->fS2SKickFrame / fTotalTime;
    }

    float fSweetSpotDiff = fShootToScore1stJumpTime - fCaptainPercentage;

    static bool bShotNISCaptainS2S;
    static signed char init;
    fSweetSpotDiff = (float)fabs(fSweetSpotDiff);
    float fSweetSpotHalfWidth = fSweetSpotDiff * 0.5f;
    float fSweetSpotCenter = fShootToScore1stJumpTime + fSweetSpotHalfWidth;
    if (!init)
    {
        bShotNISCaptainS2S = false;
        init = 1;
    }

    if (m_pCurrentAnimController->TestTrigger(fShootToScore1stJumpTime))
    {
        if (m_ePowerup == 8)
        {
            CleanUpPowerupEffect();
        }
    }

    float fAnimTime = m_pCurrentAnimController->m_fTime;
    if (fAnimTime <= fHalfAnimationTime)
    {
        if (fAnimTime <= fShootToScore1stJumpTime)
        {
            mActionShootToScoreVars.fMeterFractionTime = 0.0f;
        }
        else if (fAnimTime <= fSweetSpotCenter)
        {
            mActionShootToScoreVars.fMeterFractionTime = (fAnimTime - fShootToScore1stJumpTime) / (fSweetSpotCenter - fShootToScore1stJumpTime);
        }
        else
        {
            float fResult = 1.0f - (fAnimTime - fSweetSpotCenter) / (fCaptainPercentage - fSweetSpotCenter);
            mActionShootToScoreVars.fMeterFractionTime = fResult;

            if (mActionShootToScoreVars.fFrameButtonDownTime1 < 0.0f)
            {
                mActionShootToScoreVars.fFrameButtonDownTime1 = 1.0f;
            }

            if (m_pCurrentAnimController->m_fTime > fCaptainPercentage)
            {
                FixedUpdateTask::mTimeScale = 0.1f;
                ParticleUpdateTask::SetTimeScale(0.1f);
                mActionShootToScoreVars.fMeterFractionTime = 0.0f;

                if (mActionShootToScoreVars.fFrameButtonDownTime2 < 0.0f)
                {
                    mActionShootToScoreVars.fFrameButtonDownTime2 = 0.0f;
                }
            }
            else
            {
                float fShootToScoreSlowMoMax = g_pGame->m_pGameTweaks->unk1EC;
                float fShootToScoreSlowMoMin = g_pGame->m_pGameTweaks->unk1E8;
                nlVector3 v3OffNet = GetAIOffNetLocation(NULL);
                float dy = m_v3Position.f.y - v3OffNet.f.y;
                float dx = m_v3Position.f.x - v3OffNet.f.x;
                float dz = m_v3Position.f.z - v3OffNet.f.z;
                float fDist = nlSqrt(dy * dy + dx * dx + dz * dz, true);
                float fTimeScale = InterpolateRangeClamped(fShootToScoreSlowMoMin, fShootToScoreSlowMoMax, 9.0f, 18.0f, fDist);
                FixedUpdateTask::mTimeScale = fTimeScale;
                ParticleUpdateTask::SetTimeScale(fTimeScale);
            }
        }
    }

    fAnimTime = m_pCurrentAnimController->m_fTime;
    if (fAnimTime <= fSweetSpotCenter)
    {
        ShootToScoreMeter::instance.SetWhiteBarPosition(mActionShootToScoreVars.fMeterFractionTime);
    }
    else
    {
        if (mActionShootToScoreVars.fShootToScoreActiveTime > 0.0f)
        {
            ShootToScoreMeter::instance.SetWhiteBarPosition(mActionShootToScoreVars.fShootToScoreActiveTime);
        }
        else
        {
            ShootToScoreMeter::instance.SetWhiteBarPosition(mActionShootToScoreVars.fMeterFractionTime);
        }
    }

    if (S2SShootWasPressed())
    {
        if (mActionShootToScoreVars.fFrameButtonDownTime1 < 0.0f)
        {
            float fMeterPos = mActionShootToScoreVars.fMeterFractionTime;
            if (fMeterPos > g_pGame->m_pGameTweaks->unk298)
            {
                float fSweetSpotOffset = g_pGame->m_pGameTweaks->unk294;
                float fGreenWidth = g_pGame->m_pGameTweaks->unk29C;
                float fDiff = fSweetSpotOffset - fMeterPos;
                fDiff = fabs(fDiff);
                fDiff = (float)fDiff;

                if (fDiff < fGreenWidth)
                {
                    mActionShootToScoreVars.fFrameButtonDownTime1 = fSweetSpotOffset;
                    BeginRumbleAction((eRumbleActionPreset)3, GetGlobalPad());
                    ShootToScoreMeter::instance.meHyper = STS_POSSIBLE_HYPER;
                }
                else
                {
                    mActionShootToScoreVars.fFrameButtonDownTime1 = fMeterPos;
                    BeginRumbleAction((eRumbleActionPreset)0, GetGlobalPad());
                }

                Play3DSFX(Audio::eCharSFX(0x15), PHYSOBJ, 100.0f);
            }
        }
        else if (mActionShootToScoreVars.fFrameButtonDownTime2 < 0.0f)
        {
            float fMeterPos2 = mActionShootToScoreVars.fMeterFractionTime;
            if (fMeterPos2 < 0.6f)
            {
                if (m_pCurrentAnimController->m_fTime >= fSweetSpotCenter && m_pCurrentAnimController->m_fTime < fCaptainPercentage)
                {
                    float fYellowWidth = mActionShootToScoreVars.fGreenRegionWidth;
                    float fCenter = g_pGame->m_pGameTweaks->unk298;
                    float fDiffFromCenter = fCenter - fMeterPos2;
                    fDiffFromCenter = fabs(fDiffFromCenter);
                    fDiffFromCenter = (float)fDiffFromCenter;

                    if (fDiffFromCenter < fYellowWidth)
                    {
                        mActionShootToScoreVars.fFrameButtonDownTime2 = fMeterPos2;
                        if (mActionShootToScoreVars.fFrameButtonDownTime1 == g_pGame->m_pGameTweaks->unk294)
                        {
                            BeginRumbleAction((eRumbleActionPreset)5, GetGlobalPad());
                            ShootToScoreMeter::instance.meHyper = STS_GOT_HYPER;
                            ShootToScoreMeter::instance.m_MeterType = ShootToScoreMeter::REGULAR_SHOOT_TO_SCORE_PHASE2;
                            Play3DSFX(Audio::eCharSFX(0x17), PHYSOBJ, 100.0f);
                        }
                        else
                        {
                            BeginRumbleAction((eRumbleActionPreset)3, GetGlobalPad());
                        }
                    }
                    else
                    {
                        mActionShootToScoreVars.fFrameButtonDownTime2 = fMeterPos2;
                        BeginRumbleAction((eRumbleActionPreset)0, GetGlobalPad());
                    }

                    Play3DSFX(Audio::eCharSFX(0x15), PHYSOBJ, 100.0f);
                }
            }
        }
    }

    if (mActionShootToScoreVars.fFrameButtonDownTime1 > 0.0f)
    {
        float fSweetSpotOffset = g_pGame->m_pGameTweaks->unk294;
        float fSweetSpotPercent = fSweetSpotOffset - mActionShootToScoreVars.fFrameButtonDownTime1;
        fSweetSpotPercent = fabs(fSweetSpotPercent);
        float fAbsSweetSpotPercent = (float)fSweetSpotPercent;

        float fMultiplier;
        if (m_eCharacterClass > 12)
        {
            fMultiplier = 1.0f;
        }
        else
        {
            switch (m_eCharacterClass)
            {
            case DONKEYKONG:
            case WARIO:
            case MYSTERY:
                fMultiplier = 1.1f;
                break;
            case WALUIGI:
            case YOSHI:
                fMultiplier = 0.92f;
                break;
            case DAISY:
            case PEACH:
                fMultiplier = 0.84f;
                break;
            default:
                fMultiplier = 1.0f;
                break;
            }
        }

        GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;
        fMultiplier = fMultiplier * pGameTweaks->unk2A0;

        if (fAbsSweetSpotPercent >= pGameTweaks->unk29C)
        {
            mActionShootToScoreVars.fGreenRegionWidth = pGameTweaks->unk29C;
        }
        else
        {
            float fNewWidth = InterpolateRangeClamped(pGameTweaks->unk29C, fMultiplier, mActionShootToScoreVars.fCaptainYellowWidth, pGameTweaks->unk29C, fAbsSweetSpotPercent);
            float fMinOverMax = g_pGame->m_pGameTweaks->unk29C / g_pGame->m_pGameTweaks->unk2A0;
            nlVector3 v3OffNet2 = GetAIOffNetLocation(NULL);
            float dy2 = m_v3Position.f.y - v3OffNet2.f.y;
            float dx2 = m_v3Position.f.x - v3OffNet2.f.x;
            float dz2 = m_v3Position.f.z - v3OffNet2.f.z;
            float fDist2 = nlSqrt(dy2 * dy2 + dx2 * dx2 + dz2 * dz2, true);
            InterpolateRangeClamped(1.0f, fMinOverMax, 9.0f, 18.0f, fDist2);

            mActionShootToScoreVars.fGreenRegionWidth = fNewWidth;

            if (mActionShootToScoreVars.fGreenRegionWidth < g_pGame->m_pGameTweaks->unk29C)
            {
                mActionShootToScoreVars.fGreenRegionWidth = g_pGame->m_pGameTweaks->unk29C;
            }
        }

        ShootToScoreMeter::instance.SetSavedWhiteBarPosition(mActionShootToScoreVars.fFrameButtonDownTime1);
        ShootToScoreMeter::instance.mbShowSavedWhiteBar = true;
    }

    if (m_pCurrentAnimController->TestTrigger(fSweetSpotCenter) || mActionShootToScoreVars.fFrameButtonDownTime1 > 0.0f)
    {
        if (ShootToScoreMeter::instance.m_MeterType != ShootToScoreMeter::REGULAR_SHOOT_TO_SCORE_PHASE2)
        {
            ShootToScoreMeter::instance.SetGreenBarPosition(g_pGame->m_pGameTweaks->unk298);
            ShootToScoreMeter::instance.SetGreenRegionWidth(2.0f * mActionShootToScoreVars.fGreenRegionWidth);
        }
    }

    if (m_pCurrentAnimController->TestTrigger(fHalfAnimationTime))
    {
        DoCalcShootToScoreResult(
            g_pGame->m_pGameTweaks->unk294,
            g_pGame->m_pGameTweaks->unk298,
            mActionShootToScoreVars.fFrameButtonDownTime1,
            mActionShootToScoreVars.fFrameButtonDownTime2,
            mActionShootToScoreVars.fGreenRegionWidth);
    }

    unsigned int nTotalFrames2 = m_pCurrentAnimController->m_pSAnim->m_nNumKeys;
    static signed char init2;
    static float sfTimeSinceLastRumbleFilter;
    float fTotalTime2 = (float)nTotalFrames2;
    float lightOffTime = 94.0f / fTotalTime2;
    float firstKickTime = sfFirstKickFrame / (float)nTotalFrames2;
    float shaolinTime = 67.0f / (float)nTotalFrames2;
    float hyperStrikeAnimCamBeginTime = sfHyperStrikeAnimCamBeginFrame / (float)nTotalFrames2;

    if (!init2)
    {
        sfTimeSinceLastRumbleFilter = 0.0f;
        init2 = 1;
    }

    if (mActionShootToScoreVars.isCaptainSts)
    {
        if (m_pCurrentAnimController->TestTrigger(0.08f))
        {
            if (m_v3Position.f.x < 0.0f)
            {
                SetAnimState(0x5E, false, 0.0f, false, false);
            }
            else
            {
                SetAnimState(0x5D, false, 0.0f, false, false);
            }

            m_pCurrentAnimController->m_fPrevTime = m_pCurrentAnimController->m_fTime;
            m_pCurrentAnimController->m_fTime = 0.08f;
        }

        if (mActionShootToScoreVars.captainStsCamera == NULL)
        {
            if (m_pCurrentAnimController->TestTrigger(fHalfAnimationTime))
            {
                EmissionManager::DestroyAll(true);

                if (meS2SResult == S2S_SUPER_SHOT)
                {
                    EmitShootToScoreHyperStrike(this);

                    for (int t = 0; t < 2; t++)
                    {
                        cTeam* pTeam = g_pTeams[t];
                        for (int f = 0; f < 4; f++)
                        {
                            cFielder* pFielder = pTeam->GetFielder(f);
                            if (pFielder != this)
                            {
                                if (pFielder->IsFrozen())
                                {
                                    EmitUnFreeze(pFielder);
                                }
                                if (pFielder->IsInvincible())
                                {
                                    pFielder->CleanUpPowerupEffect();
                                }
                                pFielder->ClearPowerupAnimState(false);
                                pFielder->SetFrozen(10000000.0f);
                            }
                        }
                    }

                    g_pGame->ResetPowerups(false);
                }

                float fFadeToDarkAmount = GetConfigFloat(Config::Global(), "captain_sts/fade_to_dark_amount", 1.0f);
                WorldDarkening::Instance().Fade(sfHyperStrikeFadeOutSpeed, fFadeToDarkAmount);
                ClearPowerupAnimState(false);
                mActionShootToScoreVars.isCurrentlyInvincible = true;
                ShootToScoreMeter::instance.m_bMeterVisible = false;
                g_pGame->mbCaptainShotToScoreOn = true;
                g_pBall->m_pDrawableBall->m_uObjectFlags |= 0x40;

                if (FixedUpdateTask::mTimeScale < 1.0f)
                {
                    Audio::FadeFilterFromCurrentToZero();
                }

                if (meS2SResult != S2S_SUPER_SHOT || sbDoShatteredGlassTransition)
                {
                    Audio::FadeFilterFromCurrentToZero();
                    FixedUpdateTask::mTimeScale = 1.0f;
                    ParticleUpdateTask::SetTimeScale(1.0f);

                    if (meS2SResult == S2S_SUPER_SHOT && sbDoShatteredGlassTransition)
                    {
                        Wiper::Instance().DoWipe("break_glass");
                        m_pCurrentAnimController->m_fPrevTime = m_pCurrentAnimController->m_fTime;
                        m_pCurrentAnimController->m_fTime = hyperStrikeAnimCamBeginTime;
                    }

                    SetupCaptainSTSAnimCam(false);
                }
                else
                {
                    MatrixEffectCam* pMatrixCam = new (nlMalloc(0x140, 8, false)) MatrixEffectCam();
                    pMatrixCam->mbUseGameplayTransparencyFlags = true;
                    pMatrixCam->m_pFilter = &rumbleFilter;
                    pMatrixCam->mfSpinDuration = sfOtherMatrixCamDuration;
                    pMatrixCam->mfZoomTime = sfOtherMatrixCamZoomTime;
                    FixedUpdateTask::mTimeScale = sfOtherMatrixCamTimeScale;
                    ParticleUpdateTask::SetTimeScale(sfOtherMatrixCamTimeScale);

                    bool bIsSpinMirrored = m_v3Position.f.x < 0.0f;
                    pMatrixCam->mfFOV = sfMatrixCamFOV;
                    float fSpinRate = sfOtherMatrixCamSpinRate * (bIsSpinMirrored ? -1.0f : 1.0f);
                    pMatrixCam->mfSpinRate = fSpinRate;

                    cBaseCamera* pCurrentCam = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
                    const nlVector3& cameraPos = pCurrentCam->GetCameraPosition();

                    m_pPoseAccumulator->GetNodeMatrix(g_pCurrentlyUpdatingCharacter->m_nHeadJointIndex);
                    nlVector3* pBallPos = &g_pBall->m_v3Position;

                    pMatrixCam->Reset(cameraPos, *pBallPos, *pBallPos);
                    pMatrixCam->mbFollowBall = true;
                    pMatrixCam->mfDesiredDistanceFromTarget = sfOtherMatrixCamFinalDistanceFromTarget;
                    pMatrixCam->mfDesiredHeightAboveTarget = 0.0f;

                    cCameraManager::PushCameraWithTransition(pMatrixCam, sfOtherMatrixCamTransitionTime, (eCameraTransition)1, NULL);
                    pMatrixCam->mFinishedCallback = (void (*)(MatrixEffectCam*))OtherMatrixCamFinishedCallback;
                    sfTimeSinceLastRumbleFilter = 2.0f * sfTimeBetweenApplyingRumbleFilters;
                }

                DrawableCharacter::RenderOnlyOneCharacter(*this, false);
                mActionShootToScoreVars.preCaptainStsPlaybackSpeed = m_pCurrentAnimController->m_fPlaybackSpeedScale;
                m_pCurrentAnimController->m_fPlaybackSpeedScale = 1.0f;
            }
        }

        if (meS2SResult == S2S_SUPER_SHOT)
        {
            float fAnimTime2 = m_pCurrentAnimController->m_fTime;
            if (fAnimTime2 > fHalfAnimationTime)
            {
                float fRumbleEnd = hyperStrikeAnimCamBeginTime - sfRumblePauseTime;
                if (fAnimTime2 < fRumbleEnd)
                {
                    if (sfTimeSinceLastRumbleFilter > sfTimeBetweenApplyingRumbleFilters)
                    {
                        float fProgress = (fAnimTime2 - fHalfAnimationTime) / (fRumbleEnd - fHalfAnimationTime);
                        float fIntensity = fProgress * sfHyperStrikeMaxRumbleIntensity;
                        FireCameraRumbleFilter(0.0f, fIntensity);
                        sfTimeSinceLastRumbleFilter = 0.0f;

                        cBaseCamera* pTopCam = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
                        if (pTopCam->m_pFilter != NULL)
                        {
                            nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter->Ks = sfHyperStrikeRumbleSpringConstant;
                            nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack)->m_pFilter->Kd = sfHyperStrikeRumbleDampingConstant;
                        }
                    }

                    sfTimeSinceLastRumbleFilter += g_fFixedUpdateTick * FixedUpdateTask::mTimeScale;
                }
            }
        }

        if (mActionShootToScoreVars.captainStsCamera == NULL && meS2SResult == S2S_SUPER_SHOT)
        {
            if (m_pCurrentAnimController->TestTrigger(hyperStrikeAnimCamBeginTime))
            {
                Audio::FadeFilterFromCurrentToZero();
                FixedUpdateTask::mTimeScale = 1.0f;
                ParticleUpdateTask::SetTimeScale(1.0f);
                SetupCaptainSTSAnimCam(true);
            }
        }

        GetConfigFloat(Config::Global(), "captain_sts/fade_to_dark_frame_begin", 33.0f);

        if (mActionShootToScoreVars.captainStsCamera != NULL || meS2SResult == S2S_SUPER_SHOT)
        {
            if (m_pCurrentAnimController->TestTrigger(shaolinTime))
            {
                const char* teamName = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((s16)m_pTeam->m_nSide));
                BasicString<char, Detail::TempStringAllocator> effectName(teamName);
                effectName.AppendInPlace("_captain_sts_effect");

                EffectsGroup* pGroup = fxGetGroup(effectName.c_str());
                if (pGroup != NULL)
                {
                    EmissionController* pEmitCtrl = EmitGeneric(this, effectName.c_str(), NULL);
                    if (pEmitCtrl != NULL)
                    {
                        Function1<void, EmissionController&> callback;
                        callback.mTag = FREE_FUNCTION;
                        callback.mFreeFunction = HyperStrikeEffectUpdate;
                        pEmitCtrl->SetUpdateCallback(callback);
                        if (callback.mTag == FUNCTOR && callback.mFunctor != NULL)
                        {
                            callback.mFunctor->~FunctorBase();
                        }
                    }
                }

                sSTSLighting__17DrawableCharacter = 1;
            }
        }

        {
            unsigned int nTotalFrames3 = m_pCurrentAnimController->m_pSAnim->m_nNumKeys;
            float fTotalTime3 = (float)nTotalFrames3;
            float matrixCamStartTime = sfMatrixCamStartFrame / fTotalTime3;

            if (meS2SResult == S2S_SUPER_SHOT)
            {
                if (m_pCurrentAnimController->TestTrigger(matrixCamStartTime))
                {
                    cBaseCamera* pCurrentCam = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
                    const nlVector3& cameraPos = pCurrentCam->GetCameraPosition();
                    nlVector3* pBallPos = &g_pBall->m_v3Position;

                    Flash__10PhotoFlashFv();

                    if (sbMatrixCamTurnOffModelRendering)
                    {
                        bool bShowPositiveXNet = pBallPos->f.x > 0.0f;
                        sbIsHyperShootToScoreRenderingEnabled__5World = 1;
                        sbShowPositiveXNetDuringHyperStrike__5World = bShowPositiveXNet;
                    }

                    FixedUpdateTask::mTimeScale = sfMatrixCamTimeScale;
                    ParticleUpdateTask::SetTimeScale(sfMatrixCamParticleTimeScale);

                    MatrixEffectCam* pMatrixCam2 = new (nlMalloc(0x140, 8, false)) MatrixEffectCam();
                    cCameraManager::PushCamera(pMatrixCam2);
                    pMatrixCam2->m_pFilter = &rumbleFilter;

                    pMatrixCam2->mfSpinDuration = sfMatrixCamDuration;
                    pMatrixCam2->mfPauseAfterSpin = sfMatrixCamPauseAfterSpin;
                    pMatrixCam2->mfZoomTime = sfMatrixCamZoomTime;

                    float fSpinAngle = 360.0f * sfMatrixCamNumRevolutions;
                    cAnimCamera* pStsCamera = mActionShootToScoreVars.captainStsCamera;
                    float fSpinRate = fSpinAngle / sfMatrixCamDuration;

                    bool bIsMirrored = false;
                    if (pStsCamera->m_Mirror.f.x < 0.0f || pStsCamera->m_Mirror.f.y < 0.0f || pStsCamera->m_Mirror.f.z < 0.0f)
                    {
                        bIsMirrored = true;
                    }
                    pMatrixCam2->mfSpinRate = fSpinRate * (bIsMirrored ? -1.0f : 1.0f);

                    pMatrixCam2->mfDesiredDistanceFromTarget = sfMatrixCamFinalDistanceFromTarget;
                    pMatrixCam2->mfDesiredHeightAboveTarget = sfMatrixCamFinalHeightAboveTarget;
                    pMatrixCam2->mfFOV = sfMatrixCamFOV;
                    pMatrixCam2->mbFollowBall = true;

                    cTeam* pOtherTeam = m_pTeam->GetOtherTeam();
                    nlVector3* pGoalPos = &pOtherTeam->m_pNet->m_baseLocation;
                    pMatrixCam2->Reset(cameraPos, *pBallPos, *pGoalPos);

                    pMatrixCam2->SetInitialDistance(sfMatrixCamInitialDistanceFromTarget);

                    float fAngle = 3.1415927f * sfMatrixCamInitialAngle / 180.0f;
                    float fFacingAngle = 0.0000958738f * (float)m_aActualFacingDirection;
                    pMatrixCam2->SetInitialAngle(fAngle + fFacingAngle);

                    pMatrixCam2->SetInitialHeightAboveTarget(sfMatrixCamInitialHeightAboveTarget);

                    pMatrixCam2->mFinishedCallback = (void (*)(MatrixEffectCam*))MatrixCamFinishedCallback;
                    DrawableCharacter::RenderOnlyOneCharacter(*this, true);
                }
            }
        }

        if (mActionShootToScoreVars.captainStsCamera != NULL || meS2SResult == S2S_SUPER_SHOT)
        {
            if (m_pCurrentAnimController->TestTrigger(lightOffTime))
            {
                sSTSLighting__17DrawableCharacter = 0;
            }
        }

        if (mActionShootToScoreVars.captainStsCamera != NULL || meS2SResult == S2S_SUPER_SHOT)
        {
            if (m_pCurrentAnimController->TestTrigger(firstKickTime))
            {
                if (sbMatrixCamUseWorldDarkening && meS2SResult == S2S_SUPER_SHOT)
                {
                }
                else
                {
                    float fFadeFromDarkSpeed = GetConfigFloat(Config::Global(), "captain_sts/fade_from_dark_speed", 100.0f);
                    WorldDarkening::Instance().Fade(fFadeFromDarkSpeed, 0.0f);
                }

                mActionShootToScoreVars.isInUnbreakablePart = true;

                bool bIsSuperShot = (meS2SResult == S2S_SUPER_SHOT);
                g_pBall->m_unk_0xA5 = bIsSuperShot;
                EmitBallShot(this, (eBallShotEffectType)1, NULL, false);

                bShotNISCaptainS2S = true;
                sSTSLighting__17DrawableCharacter = 0;
                g_pBall->m_pDrawableBall->m_uObjectFlags &= ~0x40;
            }
        }

        if (mActionShootToScoreVars.captainStsCamera != NULL)
        {
            if (m_pCurrentAnimController->TestTrigger(fGreenRegionMaxWidth))
            {
                cCharacter::m_ModelType = CharModel_Rigid;
                cCameraManager::Remove(*mActionShootToScoreVars.captainStsCamera);
                if (mActionShootToScoreVars.captainStsCamera != NULL)
                {
                    mActionShootToScoreVars.captainStsCamera->~cAnimCamera();
                }
                mActionShootToScoreVars.captainStsCamera = NULL;
                m_pCurrentAnimController->m_fPlaybackSpeedScale = mActionShootToScoreVars.preCaptainStsPlaybackSpeed;
            }
        }
    }
    else
    {
        if (m_pCurrentAnimController->TestTrigger(fGreenRegionMaxWidth))
        {
            EmitBallShot(this, (eBallShotEffectType)1, NULL, false);
        }
    }

    if (m_pCurrentAnimController->TestTrigger(fGreenRegionMaxWidth))
    {
        DoRegularShooting();
        Audio::FadeFilterFromCurrentToZero();
        FixedUpdateTask::mTimeScale = 1.0f;
        ParticleUpdateTask::SetTimeScale(1.0f);

        if (meS2SResult != S2S_SUPER_SHOT)
        {
            g_pBall->InitiateBallBlur((eBallShotEffectType)1, this);
            DrawableCharacter::RenderAllCharacters();
            sSTSLighting__17DrawableCharacter = 0;
        }
        else
        {
            WorldDarkening::Instance().Fade(sfHyperStrikeFadeInSpeed, 0.0f);

            for (int t = 0; t < 2; t++)
            {
                cTeam* pTeam = g_pTeams[t];
                for (int f = 0; f < 4; f++)
                {
                    cFielder* pFielder = pTeam->GetFielder(f);
                    if (pFielder != this)
                    {
                        pFielder->SetFrozen(0.0f);
                    }
                }
            }
        }

        if (meS2SResult != S2S_SUPER_SHOT && meS2SResult != S2S_SCORE)
        {
            g_pGame->mbCaptainShotToScoreOn = false;
        }

        if (FixedUpdateTask::mTimeScale < 1.0f)
        {
            Audio::FadeFilterFromCurrentToZero();
        }

        bShotNISCaptainS2S = false;
        FireCameraRumbleFilter(0.0f, 0.2f);
        mActionShootToScoreVars.isCurrentlyInvincible = false;
        mActionShootToScoreVars.isInUnbreakablePart = false;
        ShootToScoreMeter::instance.m_bMeterVisible = false;

        const Event* pEvent = g_pEventManager->CreateValidEvent(0x40, 0x1C);
        ShotAtGoalData* pData = new ((u8*)pEvent + 0x10) ShotAtGoalData();
        pData->pShooter = this;

        g_pEventManager->CreateValidEvent(0x42, 0x14);
    }

    if (ShouldStartCrossBlend(7))
    {
        SetAction((eFielderActionState)-1);
    }
}

/**
 * Offset/Address/Size: 0x13C8 | 0x80027F00 | size: 0x408
 */
void cFielder::InitActionSlideAttack(cFielder* pTarget, float fTime)
{
    nlPolar polarFacing;
    nlPolar polarSpeed;
    nlVector3 v3SlideAttackVelocity;
    nlVector3 v3TargetPosition;
    nlVector3 v3TargetVelocity;
    float fTargetX;
    float fTargetY;
    float fMaxTime;
    float fInterceptTime;
    nlVector3 v3Delta;

    if (IsFrozen())
    {
        return;
    }

    v3SlideAttackVelocity = m_v3Velocity;

    SetAction(ACTION_SLIDE_ATTACK);
    SetAnimState(0x64, true, 0.2f, false, false);
    InitMovementCoast();

    m_tSlideAttackTimer.SetSeconds(g_pGame->m_pGameTweaks->unk2A4);

    mActionSlideAttackVars.eSlideAttackState = SLIDE_ATTACK_DOWN;
    mActionSlideAttackVars.bAttackSucceeded = false;
    mActionSlideAttackVars.bIsBButtonReset = true;
    mActionSlideAttackVars.bWasStarMushroomUsedDuring = false;

    if (GetGlobalPad() != NULL)
    {
        if (GetGlobalPad()->IsPressed(0x1C, true))
        {
            mActionSlideAttackVars.bIsBButtonReset = false;
        }
    }

    if (fTime < 0.0f)
    {
        fInterceptTime = DoFindBestSlideAttackTarget(v3TargetPosition, v3TargetVelocity);
    }
    else
    {
        if (pTarget != NULL)
        {
            v3TargetPosition = pTarget->m_v3Position;
            v3TargetVelocity = pTarget->m_v3Velocity;
        }
        else
        {
            v3TargetPosition = g_pBall->m_v3Position;
            v3TargetVelocity = g_pBall->m_v3Velocity;
        }

        fInterceptTime = fTime;
    }

    fMaxTime = g_pGame->m_pGameTweaks->unk2A4 + g_pGame->m_pGameTweaks->unk2A8;

    if (fInterceptTime >= 0.0f && fInterceptTime <= fMaxTime)
    {
        fTargetX = v3TargetPosition.f.x + v3TargetVelocity.f.x * fInterceptTime;
        fTargetY = v3TargetPosition.f.y + v3TargetVelocity.f.y * fInterceptTime;
    }
    else
    {
        fTargetX = v3TargetPosition.f.x + v3TargetVelocity.f.x * fMaxTime;
        fTargetY = v3TargetPosition.f.y + v3TargetVelocity.f.y * fMaxTime;
    }

    nlVec3Sub(v3Delta, m_v3Position, g_pBall->m_v3Position);
    if (nlSqrt(v3Delta.GetLengthSq3D(), true) < (0.1f + m_pTweaks->fPhysCapsuleRadius))
    {
        float fLenSq = v3SlideAttackVelocity.f.x * v3SlideAttackVelocity.f.x
                     + v3SlideAttackVelocity.f.y * v3SlideAttackVelocity.f.y
                     + v3SlideAttackVelocity.f.z * v3SlideAttackVelocity.f.z;

        if (fLenSq > 0.0001f)
        {
            float fInvLen = nlRecipSqrt(fLenSq, true);
            _nlVec3Scale(v3SlideAttackVelocity, fInvLen);
        }

        v3SlideAttackVelocity.f.x *= GetSlideAttackSpeed();
        v3SlideAttackVelocity.f.y *= GetSlideAttackSpeed();
        v3SlideAttackVelocity.f.z = 0.0f;
    }
    else
    {
        v3SlideAttackVelocity.f.x = fTargetX - m_v3Position.f.x;
        v3SlideAttackVelocity.f.y = fTargetY - m_v3Position.f.y;
        v3SlideAttackVelocity.f.z = 0.0f;

        {
            float fLenSq = v3SlideAttackVelocity.f.x * v3SlideAttackVelocity.f.x
                         + v3SlideAttackVelocity.f.y * v3SlideAttackVelocity.f.y
                         + v3SlideAttackVelocity.f.z * v3SlideAttackVelocity.f.z;

            if (fLenSq > 0.0001f)
            {
                float fInvLen = nlRecipSqrt(fLenSq, true);
                _nlVec3Scale(v3SlideAttackVelocity, fInvLen);
            }
        }

        v3SlideAttackVelocity.f.x *= GetSlideAttackSpeed();
        v3SlideAttackVelocity.f.y *= GetSlideAttackSpeed();

        nlCartesianToPolar(polarFacing, v3SlideAttackVelocity.f.x, v3SlideAttackVelocity.f.y);

        m_aDesiredFacingDirection = polarFacing.a;
        SetFacingDirection(m_aDesiredFacingDirection);
        m_aActualMovementDirection = m_aDesiredFacingDirection;
    }

    v3SlideAttackVelocity.f.z = 0.0f;

    SetVelocity(v3SlideAttackVelocity);

    nlCartesianToPolar(polarSpeed, v3SlideAttackVelocity.f.x, v3SlideAttackVelocity.f.y);

    {
        float fSpeed = polarSpeed.r;
        m_fDesiredSpeed = fSpeed;
        m_fActualSpeed = fSpeed;
    }

    Play3DSFX(Audio::eCharSFX(0xC), PHYSOBJ, 100.0f);
    PlayRandomCharDialogue(0, VECTORS, 100.0f, -1.0f);
}

/**
 * Offset/Address/Size: 0xE2C | 0x80027964 | size: 0x59C
 */
void cFielder::ActionSlideAttack(float fDeltaTime)
{
    if (!g_pGame->IsGameplayOrOvertime())
    {
        SetAction(ACTION_NEED_ACTION);
    }

    if (!mActionSlideAttackVars.bAttackSucceeded && mActionSlideAttackVars.eSlideAttackState == SLIDE_ATTACK_DOWN && m_ePowerup == POWER_UP_STAR)
    {
        float dx = g_pBall->m_v3Position.f.x - m_v3Position.f.x;
        float dy = g_pBall->m_v3Position.f.y - m_v3Position.f.y;
        float dist = dx * dx + dy * dy;
        nlSqrt(dist, true);
        dist = nlSqrt(dist, true);
        float invDist = 1.0f / dist;

        dy = invDist * dy;
        dx = invDist * dx;
        float speed = nlGetLength2D(m_v3Velocity.f.x, m_v3Velocity.f.y);

        float scaledDy = 8.5f * dy;
        float scaledDx = 8.5f * dx;
        float slideVelY = scaledDy + m_v3Velocity.f.y;
        float slideVelX = scaledDx + m_v3Velocity.f.x;

        float slideMag = nlSqrt(slideVelY * slideVelY + slideVelX * slideVelX, true);
        float invSlideMag = 1.0f / slideMag;
        float normSlideY = invSlideMag * slideVelY;
        float normSlideX = invSlideMag * slideVelX;

        float dot = normSlideY * dy + normSlideX * dx;

        if (dot >= 0.0f)
        {
            float vx = speed * normSlideX;
            float vy = speed * normSlideY;
            nlVector3 vel;
            vel.f.z = 0.0f;
            vel.f.x = vx;
            vel.f.y = vy;

            nlPolar polar;
            nlCartesianToPolar(polar, vx, vy);
            m_aDesiredFacingDirection = polar.a;
            m_aActualFacingDirection = m_aDesiredFacingDirection;
            m_aActualMovementDirection = m_aActualFacingDirection;
            SetVelocity(vel);
        }
    }

    cBall* pBall = g_pBall;
    if (pBall->m_pOwner == NULL && !pBall->m_unk_0xA6 && pBall->m_tShotTimer.m_uPackedTime == 0)
    {
        if (pBall->m_tNoPickupTimer.m_uPackedTime == 0 || pBall->m_pPrevOwner == NULL || pBall->m_pPrevOwner->m_eClassType != GOALIE)
        {
            nlVector3& jointPos = GetJointPosition(m_nBallJointIndex);
            nlVector3& prevJointPos = GetPrevJointPosition(m_nBallJointIndex);

            mActionSlideAttackVars.bAttackSucceeded = TestCollision(0.05f, prevJointPos, jointPos, 0.18f, pBall->m_v3PrevPosition, pBall->m_v3Position);
            if (mActionSlideAttackVars.bAttackSucceeded)
            {
                cPlayer* pPrevOwner = g_pBall->m_pPrevOwner;
                if (pPrevOwner != NULL && pPrevOwner->m_eClassType == FIELDER)
                {
                    if (!IsOnSameTeam(pPrevOwner))
                    {
                        if (((cFielder*)g_pBall->m_pPrevOwner)->m_eActionState == ACTION_SLIDE_ATTACK_REACT)
                        {
                            PlayerAttackData* pData = new (&g_pEventManager->CreateValidEvent(0x19, 0x28)->m_data) PlayerAttackData();
                            pData->pAttacker = this;
                            pData->nAttackerPadID = GetGlobalPad() ? GetGlobalPad()->m_padIndex : -1;
                            pData->pTarget = NULL;
                        }
                        else if (g_pBall->m_pPassTarget != NULL)
                        {
                            PlayerAttackData* pData = new (&g_pEventManager->CreateValidEvent(0x19, 0x28)->m_data) PlayerAttackData();
                            pData->pAttacker = this;
                            pData->nAttackerPadID = GetGlobalPad() ? GetGlobalPad()->m_padIndex : -1;
                            pData->pTarget = NULL;
                        }
                    }
                }

                PickupBall(g_pBall);
            }
        }
    }

    float fSpeed = m_fActualSpeed;
    if (mActionSlideAttackVars.bAttackSucceeded)
    {
        fSpeed = GetSlideAttackSpeed();

        if (GetGlobalPad() != NULL && mActionSlideAttackVars.bIsBButtonReset)
        {
            TestButtonsToQueueActions(fDeltaTime);
        }
        else
        {
            if (GetGlobalPad() != NULL)
            {
                if (GetGlobalPad()->JustReleased(PAD_SHOOT, true))
                {
                    if (!mActionSlideAttackVars.bIsBButtonReset)
                    {
                        mActionSlideAttackVars.bIsBButtonReset = true;
                    }
                }
            }
        }
    }

    {
        nlVector3 vel;
        nlPolarToCartesian(vel.f.x, vel.f.y, m_aActualFacingDirection, fSpeed);
        vel.f.z = 0.0f;
        SetVelocity(vel);
    }

    switch (mActionSlideAttackVars.eSlideAttackState)
    {
    case SLIDE_ATTACK_DOWN:
    {
        if (m_pCurrentAnimController->TestFrameTrigger(2.0f))
        {
            EmitSlideTackleTrail(this);
        }

        bool bShouldDecelerate = false;
        if (GetGlobalPad() != NULL)
        {
            if (!GetGlobalPad()->IsPressed(PAD_SLIDE_ATTACK, true))
            {
                bShouldDecelerate = true;
            }
        }

        if (m_tSlideAttackTimer.m_uPackedTime == 0 || bShouldDecelerate)
        {
            mActionSlideAttackVars.eSlideAttackState = SLIDE_ATTACK_DECELERATE;
            InitMovementDecelerateExponential(g_pGame->m_pGameTweaks->unk2AC);
            m_tSlideAttackTimer.SetSeconds(g_pGame->m_pGameTweaks->unk2A8);
            m_fDesiredSpeed = 0.0f;
        }
        break;
    }

    case SLIDE_ATTACK_DECELERATE:
    {
        if (m_tSlideAttackTimer.m_uPackedTime != 0)
        {
            break;
        }

        if (!mActionSlideAttackVars.bWasStarMushroomUsedDuring && m_ePowerup != POWER_UP_STAR)
        {
            CleanUpPowerupEffect();
        }

        if (m_pBall != NULL)
        {
            if (!TestQueuedActions())
            {
                SetAction(ACTION_NEED_ACTION);
            }
        }
        else
        {
            SetAction(ACTION_NEED_ACTION);
        }

        m_eLastPadAction = PAD_NONE;
        break;
    }
    }
}

/**
 * Offset/Address/Size: 0xC2C | 0x80027764 | size: 0x200
 */
void cFielder::InitActionSlideAttackFailReact()
{
    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_SLIDE_FAIL_REACT);

    SetAnimState(0x65, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0xBE8 | 0x80027720 | size: 0x44
 */
void cFielder::ActionSlideAttackFailReact(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x93C | 0x80027474 | size: 0x2AC
 */
void cFielder::InitActionSquishReact(const nlVector3& dir)
{
    CleanUpPowerupEffect();

    if (IsFallenDown(0.0f))
    {
        return;
    }

    if (IsFrozen())
    {
        m_tFrozenTimer.SetSeconds(0.0f);
        EmitUnFreeze(this);
    }

    if (g_pBall->m_pOwner == this)
    {
        ReleaseBall();
        ShootBallDueToContact(dir);
    }

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_SQUISH_REACT);

    SetAnimState(0x5C, true, 0.2f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    nlPolar polar;
    nlCartesianToPolar(polar, dir.f.x, dir.f.y);
    SetFacingDirection(polar.a);
    m_aActualMovementDirection = polar.a;

    EmitTackleImpact(this);
    PlayAttackReactionSounds(g_pGame->m_pGameTweaks->unk25C);

    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0x8A0 | 0x800273D8 | size: 0x9C
 */
void cFielder::DoSlideAttackStats()
{
    const Event* pEvent = g_pEventManager->CreateValidEvent(0x19, 0x28);
    PlayerAttackData* pData = new ((u8*)pEvent + 0x10) PlayerAttackData();

    pData->pAttacker = this;

    bool bHasGlobalPad = GetGlobalPad() != nullptr;
    pData->nAttackerPadID = bHasGlobalPad ? GetGlobalPad()->m_padIndex : -1;

    pData->pTarget = nullptr;
}

/**
 * Offset/Address/Size: 0x5C0 | 0x800270F8 | size: 0x2E0
 * TODO: Just one reg mismatch on SetFacingAnim
 */
void cFielder::InitActionSlideAttackReact(cPlayer* pAttacker, bool bSkipEvent)
{
    CleanUpPowerupEffect();

    if (!IsFallenDown(0.0f))
    {
        if (IsFrozen())
        {
            m_tFrozenTimer.SetSeconds(0.0f);
            EmitUnFreeze(this);
        }

        if (m_pBall != nullptr)
        {
            ReleaseBall();
            ShootBallDueToContact(pAttacker->m_v3Velocity);
        }

        InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
        SetAction(ACTION_SLIDE_ATTACK_REACT);

        u16 facingDelta = (u16)(GetFacingDeltaToPosition(pAttacker->m_v3Position) + 0x2000);
        SetAnimState(SlideAttackReactAnims[facingDelta >> 14], true, 0.2f, false, false);

        InitMovementFromAnim(0, v3Zero, 1.0f, false);

        if (!bSkipEvent)
        {
            const Event* pEvent = g_pEventManager->CreateValidEvent(0x1A, 0x28);
            PlayerAttackData* pData = new ((u8*)pEvent + 0x10) PlayerAttackData();

            pData->pAttacker = static_cast<const cFielder*>(pAttacker);

            bool bHasGlobalPad = pAttacker->GetGlobalPad() != nullptr;
            pData->nAttackerPadID = bHasGlobalPad ? pAttacker->GetGlobalPad()->m_padIndex : -1;

            pData->pTarget = this;
        }

        m_fDesiredSpeed = 0.0f;
    }
}

/**
 * Offset/Address/Size: 0x3BC | 0x80026EF4 | size: 0x204
 */
void cFielder::InitActionSTSHitReact(cPlayer*)
{
    CleanUpPowerupEffect();

    InitDesire(FIELDERDESIRE_FINISH_ACTION, 0.5f, -1.0f, fvNotSet, fvNotSet);
    SetAction(ACTION_STS_HIT_REACT);

    SetAnimState(0x73, false, 0.0333f, false, false);
    InitMovementFromAnim(0, v3Zero, 1.0f, false);

    m_fDesiredSpeed = 0.0f;
}

/**
 * Offset/Address/Size: 0x378 | 0x80026EB0 | size: 0x44
 */
void cFielder::ActionSlideAttackReact(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x334 | 0x80026E6C | size: 0x44
 */
void cFielder::ActionBombReact(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x2AC | 0x80026DE4 | size: 0x88
 */
void cFielder::ActionSTSHitReact(float)
{
    m_pCurrentAnimController->TestFrameTrigger(3.0f);

    if (m_pCurrentAnimController->TestFrameTrigger(4.0f))
    {
        FireCameraRumbleFilter(0.0f, 0.2f);
        BeginRumbleAction(RUMBLE_SOLID_CONTACT, GetGlobalPad());
    }

    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x268 | 0x80026DA0 | size: 0x44
 */
void cFielder::ActionShellReact(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x224 | 0x80026D5C | size: 0x44
 */
void cFielder::ActionBananaReact(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x1E0 | 0x80026D18 | size: 0x44
 */
void cFielder::ActionSquishReact(float)
{
    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x118 | 0x80026C50 | size: 0xC8
 */
void cFielder::InitActionReceivePass(int animID, nlVector3& v3TargetPos, float fAdjustEndTime)
{
    SetAction(ACTION_RECEIVE_PASS);
    SetAnimState(animID, true, 0.2f, false, false);

    nlVector3 v3Direction;
    nlVec3Set(v3Direction,
        v3TargetPos.f.x - m_v3Position.f.x,
        v3TargetPos.f.y - m_v3Position.f.y,
        v3TargetPos.f.z - m_v3Position.f.z);

    InitMovementFromAnim(0, v3Direction, fAdjustEndTime, false);
    m_aDesiredFacingDirection = m_aActualFacingDirection;
}

/**
 * Offset/Address/Size: 0x68 | 0x80026BA0 | size: 0xB0
 */
void cFielder::ActionReceivePass(float)
{
    if (CanPickupBallFromPass(g_pBall))
    {
        bool bSetPerfectPass = false;
        if (g_pBall->mbHyperSTS)
        {
            if (m_DesireReceivePassSharedVars.iAttemptOneTouchShot != 0)
            {
                if (m_eAnimID != 60)
                {
                    bSetPerfectPass = true;
                }
            }
        }

        PickupBall(g_pBall);

        if (bSetPerfectPass)
        {
            g_pBall->SetPerfectPass(true, true);
        }
    }

    if (ShouldStartCrossBlend(7))
    {
        SetAction(ACTION_NEED_ACTION);
    }
}

/**
 * Offset/Address/Size: 0x4 | 0x80026B3C | size: 0x64
 */
void cFielder::InitActionWait()
{
    SetAction(ACTION_WAIT);
    SetAnimState(0, true, 0.0f, false, false);
    InitMovementNone(0.0f, 0.0f);
    m_aDesiredFacingDirection = m_aActualFacingDirection;
}

/**
 * Offset/Address/Size: 0x0 | 0x80026B38 | size: 0x4
 */
void cFielder::ActionWait(float)
{
    // EMPTY
}

// At the bottom of FielderActions.cpp -- REMOVE once real callers exist.
void FielderActions_stub()
{
    FormatImpl<BasicString<char, Detail::TempStringAllocator> > impl;
    BasicString<char, Detail::TempStringAllocator> result = (BasicString<char, Detail::TempStringAllocator>)impl;

    const char* s = "";
    LexicalCast<BasicString<char, Detail::TempStringAllocator>, const char*>(s);
}
