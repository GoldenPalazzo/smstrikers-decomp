#include "Game/Render/ChainChomp.h"

#include "Game/Game.h"
#include "Game/Player.h"
#include "Game/ReplayManager.h"
#include "Game/Effects/EmissionController.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/AI/Powerups.h"
#include "Game/Physics/PhysicsShell.h"
#include "Game/Physics/PhysicsBanana.h"
#include "Game/Physics/PhysicsCharacter.h"
#include "Game/Physics/PhysicsAIBall.h"
#include "Game/AI/AiUtil.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glState.h"
#include "NL/glx/glxTexture.h"

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
    unsigned long Play(Audio::eWorldSFX, float, float, bool, float);
};

extern cWorldSFX gPowerupSFX;
extern cWorldSFX gStadGenSFX;
} // namespace Audio

void FireCameraRumbleFilter(float, float);

static unsigned char gbChainChompProjectedShadow;

const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };
const nlVector3 gv3HomePosition = { 0.0f, 0.0f, 10.0f };

/**
 * Offset/Address/Size: 0x12BC | 0x8015EFC0 | size: 0x1D8
 */
ChainChomp::ChainChomp(cSHierarchy& pHierarchy, int nModelID, PhysicsNPC& mpPhysObj, cInventory<cSAnim>* pInventorySAnim)
    : SkinAnimatedMovableNPC(pHierarchy, nModelID, mpPhysObj)
{
}

/**
 * Offset/Address/Size: 0x125C | 0x8015EF60 | size: 0x60
 */
ChainChomp::~ChainChomp()
{
}

/**
 * Offset/Address/Size: 0xAA8 | 0x8015E7AC | size: 0x7B4
 */
void ChainChomp::Update(float fDeltaT)
{
    switch (meChainChompState)
    {
    case CHAIN_STATE_FALL:
        if (mtStateTimer.m_uPackedTime == 0)
        {
            break;
        }

        if (mpTarget != NULL)
        {
            maDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(mpTarget->m_v3Position.f.y - mv3Position.f.y, mpTarget->m_v3Position.f.x - mv3Position.f.x));
        }

        Move(fDeltaT);

        if (mtStateTimer.Countdown(fDeltaT, 0.0f))
        {
            meChainChompState = CHAIN_STATE_RECOVER;
            SetAnimState(*mpRecoverAnim, 0.0f, PM_HOLD);

            nlVector3 v3NewPosition = mv3Position;
            v3NewPosition.f.z = 0.0f;
            SetPosition(v3NewPosition);
            mv3Velocity = v3Zero;

            Event* pEvent = g_pEventManager->CreateValidEvent(0x33, 0x1C);
            ShotAtGoalData* pData = new ((u8*)pEvent + 0x10) ShotAtGoalData();
            pData->pShooter = mpThrower;
        }
        break;

    case CHAIN_STATE_RECOVER:
        if (mpAnimController->m_fTime < 0.625)
        {
            nlVector3 v3NewPosition = mpTarget->m_v3Position;
            v3NewPosition.f.z = 0.0f;
            SetPosition(v3NewPosition);
        }

        if (mpAnimController->TestTrigger(0.75f))
        {
            Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x8D, cGameSFX::SFX_STOP_FIRST);
            mpPhysObj->EnableCollisions();

            EmissionController* pControl = EmissionManager::Create(fxGetGroup("chainchomp_land"), 0);
            pControl->SetPosition(mv3Position);

            PowerupBase::PlayPowerupSound(POWER_UP_CHAIN_CHOMP, PowerupBase::PWRUP_SOUND_BOUNCE_GROUND, mpPhysObj, 100.0f);
            Audio::gStadGenSFX.Play((Audio::eWorldSFX)0xCE, 100.0f, -1.0f, true, 100.0f);

            EffectsGroup* pGroup = fxGetGroup("chainchomp_trail");
            if (pGroup->m_specs != NULL && pGroup->m_numSpecs > 0)
            {
                EffectsSpec* pSpec = pGroup->m_specs;
                int i;
                for (i = pGroup->m_numSpecs; i > 0; i--)
                {
                    if (pSpec->m_pTemplate != NULL)
                    {
                        pSpec->m_pTemplate->m_fFountainLife = 12.0f;
                    }
                    pSpec++;
                }
            }

            pControl = EmissionManager::Create(pGroup, 0);
            pControl->m_uUserData = (u32)this;

            {
                Function<EmissionController&> update;
                update.mTag = FREE_FUNCTION;
                update.mFreeFunction = UpdateChainEmitter;
                pControl->SetUpdateCallback(update);
            }

            FireCameraRumbleFilter(0.0f, 0.2f);
        }
        else
        {
            bool bRecoverDone = false;
            if (mpAnimController->m_ePlayMode == PM_HOLD && mpAnimController->m_fTime == 1.0f)
            {
                bRecoverDone = true;
            }

            if (bRecoverDone)
            {
                PowerupBase::PlayPowerupSound(POWER_UP_CHAIN_CHOMP, PowerupBase::PWRUP_SOUND_END, mpPhysObj, 100.0f);
                meChainChompState = CHAIN_STATE_CHASE;
                SetAnimState(*mpIdleAnim, 0.0f, PM_CYCLIC);
                mtStateTimer.SetSeconds(g_pGame->m_pGameTweaks->fChainChompActiveTime);
            }
        }
        break;

    case CHAIN_STATE_CHASE:
        if (mtStateTimer.m_uPackedTime == 0)
        {
            break;
        }

        if (mpTarget != NULL)
        {
            maDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(mpTarget->m_v3Position.f.y - mv3Position.f.y, mpTarget->m_v3Position.f.x - mv3Position.f.x));
        }

        Move(fDeltaT);

        if (mpInEffectSFX == NULL && !g_pGame->mbCaptainShotToScoreOn)
        {
            Audio::SoundAttributes sndAtr;
            sndAtr.Init();
            sndAtr.SetSoundType(0x8E, true);
            sndAtr.UseVectors(mv3Position, mv3Velocity);
            sndAtr.mf_ReturnEmitterOnPlay = true;
            mpInEffectSFX = (SFXEmitter*)Audio::gPowerupSFX.Play(sndAtr);
        }

        {
            static float fBarkT;
            static signed char init;
            if (!init)
            {
                init = 1;
                fBarkT = 0.0f;
            }

            fBarkT += fDeltaT;
            if (fBarkT > 0.7f)
            {
                Audio::SoundAttributes sndAtr;
                sndAtr.Init();
                sndAtr.SetSoundType(0x91, true);
                sndAtr.UseStationaryPosVector(mv3Position);
                Audio::gPowerupSFX.Play(sndAtr);
                fBarkT = 0.0f;
            }
        }

        if (mtStateTimer.Countdown(fDeltaT, 0.0f))
        {
            float fTargetX;
            float fTargetY;

            meChainChompState = CHAIN_STATE_LEAVE;

            if ((0.5f * g_pBall->m_v3Velocity.f.x + g_pBall->m_v3Position.f.x) < 0.0f)
            {
                fTargetX = 40.0f;
            }
            else
            {
                fTargetX = -40.0f;
            }

            if (mv3Position.f.y < 0.0f)
            {
                fTargetY = -8.0f;
            }
            else
            {
                fTargetY = 8.0f;
            }

            maDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(fTargetY - mv3Position.f.y, fTargetX - mv3Position.f.x));
            mfDesiredSpeed = g_pGame->m_pGameTweaks->fChainChompSpeed;
        }
        break;

    case CHAIN_STATE_LEAVE:
        Move(fDeltaT);

        {
            static float fBarkT;
            static signed char init;
            if (!init)
            {
                init = 1;
                fBarkT = 0.0f;
            }

            fBarkT += fDeltaT;
            if (fBarkT > 0.7f)
            {
                Audio::SoundAttributes sndAtr;
                sndAtr.Init();
                sndAtr.SetSoundType(0x91, true);
                sndAtr.UseStationaryPosVector(mv3Position);
                Audio::gPowerupSFX.Play(sndAtr);
                fBarkT = 0.0f;
            }
        }

        if (fabsf(mv3Position.f.x) > 22.5f)
        {
            Event* pEvent = g_pEventManager->CreateValidEvent(0x34, 0x1C);
            ShotAtGoalData* pData = new ((u8*)pEvent + 0x10) ShotAtGoalData();
            pData->pShooter = mpThrower;

            if (mpInEffectSFX != NULL)
            {
                PowerupBase::StopPowerupInEffectSound(mpInEffectSFX);
                mpInEffectSFX = NULL;
            }

            EmissionManager::Kill((unsigned long)this, fxGetGroup("chainchomp_trail"));

            meChainChompState = CHAIN_STATE_HIDDEN;
            mfDesiredSpeed = 0.0f;
            SetAnimState(*mpIdleAnim, 0.0f, PM_CYCLIC);
            SetPosition(gv3HomePosition);
            mv3Velocity = v3Zero;
            maFacingDirection = 0;
            mpPhysObj->DisableCollisions();
            mpThrower = NULL;
            mnThrowerPadID = -1;
            mbIsVisible = false;
        }
        break;

    case CHAIN_STATE_HIDDEN:
    default:
        break;
    }

    {
        nlVector3 v3NewPosition;
        v3NewPosition.f.x = mv3Position.f.x + fDeltaT * mv3Velocity.f.x;
        v3NewPosition.f.y = mv3Position.f.y + fDeltaT * mv3Velocity.f.y;
        v3NewPosition.f.z = mv3Position.f.z + fDeltaT * mv3Velocity.f.z;
        SetPosition(v3NewPosition);
    }

    if (mpInEffectSFX != NULL)
    {
        static nlVector3 vPos;
        static nlVector3 vDir;

        vPos = mv3Position;
        vDir = mv3Velocity;

        mpInEffectSFX->pos.vPos = vPos;
        mpInEffectSFX->dir.vDir = vDir;
    }

    SkinAnimatedNPC::Update(fDeltaT);
}

/**
 * Offset/Address/Size: 0x97C | 0x8015E680 | size: 0x12C
 */
void ChainChomp::CollisionCallback(PhysicsObject* pObjA, PhysicsObject* pObjB, const nlVector3& v3Normal)
{
    ChainChomp* pChainChomp = (ChainChomp*)((PhysicsNPC*)this)->mpAINPC;
    cFielder* pFielder = NULL;

    switch (pObjA->GetObjectType())
    {
    case 0x04:
    case 0x0D:
    case 0x0E:
        pFielder = (cFielder*)((PhysicsCharacter*)pObjA->m_parentObject)->m_pAICharacter;
        break;
    case 0x0F:
    {
        cBall* pBall = ((PhysicsAIBall*)pObjA)->m_pAIBall;
        if (pBall->m_pOwner != NULL)
        {
            pFielder = (cFielder*)pBall->m_pOwner;
        }
        else
        {
            pBall->ClearPassTarget();
            pBall->ClearShotInProgress();
        }
        break;
    }
    case 0x13:
        ((PhysicsShell*)pObjA)->m_pPowerupObject->m_bShouldDestroy = true;
        break;
    case 0x14:
        ((PhysicsBanana*)pObjA)->m_pPowerupObject->m_bShouldDestroy = true;
        break;
    }

    if (pFielder != NULL && pFielder->m_eClassType == FIELDER && !pFielder->IsFallenDown(0.0f))
    {
        Event* pEvent = g_pEventManager->CreateValidEvent(0x2F, 0x20);
        CollisionChainPlayerData* pData = new ((u8*)pEvent + 0x10) CollisionChainPlayerData();
        pData->pFielder = pFielder;
        pData->pChain = pChainChomp;
    }
}

/**
 * Offset/Address/Size: 0x8E8 | 0x8015E5EC | size: 0x94
 */
void UpdateChainEmitter(EmissionController& controller)
{
    if (ReplayManager::Instance()->mRender != nullptr)
    {
        {
            ReplayManager* replayManager = ReplayManager::Instance();
            controller.SetPosition(replayManager->mRender->mChainChomp.mPosition);
        }
        {
            ReplayManager* replayManager = ReplayManager::Instance();
            controller.SetVelocity(replayManager->mRender->mChainChomp.mVelocity);
        }
        nlVector3 direction = { 0.0f, 0.0f, 0.0f };
        controller.SetDirection(direction);
    }
}

/**
 * Offset/Address/Size: 0x608 | 0x8015E30C | size: 0x2E0
 * TODO: 99.24% match - remaining FPR allocation/scheduling mismatch in score weighting.
 */
void ChainChomp::FindTarget(cTeam* pTeam)
{
    float fBestScore = 99999.9f;
    cFielder* pBestCandidate = NULL;
    cFielder* pFielder;
    cFielder* pFielder2;
    int i;
    float fTempScore;
    cFielder* pCandidate;

    if (g_pBall->GetOwnerFielder() != NULL)
    {
        pFielder = g_pBall->GetOwnerFielder();

        if (pFielder->IsOnSameTeam(pTeam->GetStriker()) && !pFielder->IsFallenDown(0.0f) && pFielder != mpTarget)
        {
            pBestCandidate = g_pBall->GetOwnerFielder();
        }
    }
    else if (g_pBall->GetPassTargetFielder() != NULL)
    {
        pFielder2 = (cFielder*)g_pBall->GetPassTargetFielder();

        if (pFielder2->IsOnSameTeam(pTeam->GetStriker()) && !pFielder2->IsFallenDown(0.0f) && pFielder2 != mpTarget)
        {
            pBestCandidate = pFielder2;
        }
    }

    if (pBestCandidate == NULL)
    {
        for (i = 0; i < 4; i++)
        {
            fTempScore = 999999.9f;
            pCandidate = pTeam->GetFielder(i);

            if (!pCandidate->IsFallenDown(0.0f) && pCandidate != mpTarget)
            {
                float dx;
                float dy;

                dy = pCandidate->m_v3Position.f.y - mv3Position.f.y;
                dx = pCandidate->m_v3Position.f.x - mv3Position.f.x;
                float fDist = nlSqrt(dx * dx + dy * dy, true);
                float fConverted = 10430.378f * nlATan2f(dy, dx);
                s16 angleDiff = (s16)(maFacingDirection - (u16)(s32)fConverted);
                u16 absDelta = (u16)((angleDiff < 0) ? -angleDiff : angleDiff);
                float fAngleWeighting = g_pGame->m_pGameTweaks->fAngleWeighting;

                fTempScore = fDist * (1.0f - fAngleWeighting) + (float)absDelta * fAngleWeighting;
            }

            if (fTempScore < fBestScore)
            {
                pBestCandidate = pCandidate;
                fBestScore = fTempScore;
            }
        }
    }

    if (pBestCandidate == NULL)
    {
        float fTargetX;
        float fTargetY;
        float fY;

        mtStateTimer.SetSeconds(0.0f);
        meChainChompState = CHAIN_STATE_LEAVE;

        if ((0.5f * g_pBall->m_v3Velocity.f.x + g_pBall->m_v3Position.f.x) < 0.0f)
        {
            fTargetX = 40.0f;
        }
        else
        {
            fTargetX = -40.0f;
        }

        fY = mv3Position.f.y;

        if (fY < 0.0f)
        {
            fTargetY = -8.0f;
        }
        else
        {
            fTargetY = 8.0f;
        }

        maDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(fTargetY - fY, fTargetX - mv3Position.f.x));
        mfDesiredSpeed = g_pGame->m_pGameTweaks->fChainChompSpeed;
        pBestCandidate = pTeam->GetStriker();
    }

    mpTarget = pBestCandidate;
}

/**
 * Offset/Address/Size: 0x4B4 | 0x8015E1B8 | size: 0x154
 */
void ChainChomp::Fall(cFielder* pThrower, cFielder* pTarget)
{
    class nlVector3 v3StartPosition;
    class nlVector3 v3FallVelocity;

    mbIsVisible = true;
    SetAnimState(*mpDropAnim, 0.2f, PM_CYCLIC);
    mpTarget = pTarget;
    mfDesiredSpeed = g_pGame->m_pGameTweaks->fChainChompSpeed;
    PowerupBase::PlayPowerupSound(POWER_UP_CHAIN_CHOMP, PowerupBase::PWRUP_SOUND_ACTIVATE, mpPhysObj, 100.0f);
    meChainChompState = CHAIN_STATE_FALL;

    v3StartPosition = mpTarget->m_v3Position;
    v3StartPosition.f.z = 100.0f;
    SetPosition(v3StartPosition);

    nlVec3Set(v3FallVelocity, 0.0f, 0.0f, -(75.0f / g_pGame->m_pGameTweaks->fChainChompFallTime));
    mv3Velocity = v3FallVelocity;

    mtStateTimer.SetSeconds(g_pGame->m_pGameTweaks->fChainChompFallTime);
    mpThrower = pThrower;

    bool bHasGlobalPad = pThrower->GetGlobalPad() != nullptr;
    mnThrowerPadID = bHasGlobalPad ? pThrower->GetGlobalPad()->m_padIndex : -1;

    mpPhysObj->DisableCollisions();
}

/**
 * Offset/Address/Size: 0x3BC | 0x8015E0C0 | size: 0xF8
 */
void ChainChomp::Hide(bool bIsEndGame)
{
    if (mpInEffectSFX != nullptr)
    {
        PowerupBase::StopPowerupInEffectSound(mpInEffectSFX);
        mpInEffectSFX = nullptr;
    }

    EffectsGroup* pEffectsGroup = fxGetGroup("chainchomp_trail"); // Chain chomp effects group

    if (!bIsEndGame)
    {
        EmissionManager::Kill((unsigned long)this, pEffectsGroup);
    }
    else
    {
        EmissionManager::Destroy((unsigned long)this, pEffectsGroup);
    }

    meChainChompState = CHAIN_STATE_HIDDEN;
    mfDesiredSpeed = 0.0f;

    SetAnimState(*mpIdleAnim, 0.0f, PM_CYCLIC);
    SetPosition(gv3HomePosition);

    mv3Velocity = v3Zero;
    maFacingDirection = 0;

    mpPhysObj->DisableCollisions();

    mpThrower = nullptr;
    mnThrowerPadID = -1;
    mbIsVisible = false;
}

/**
 * Offset/Address/Size: 0x3AC | 0x8015E0B0 | size: 0x10
 */
bool ChainChomp::IsHidden() const
{
    return meChainChompState == CHAIN_STATE_HIDDEN;
}

static bool AvoidSidelines(ChainChomp* pChomp)
{
    nlVector3 v3WallPosition = GetClosestPointOnSidelines(pChomp->mv3Position);
    float dy = v3WallPosition.f.y - pChomp->mv3Position.f.y;
    float dx = v3WallPosition.f.x - pChomp->mv3Position.f.x;
    float fDist = nlSqrt(dx * dx + dy * dy, true);
    if (fDist < 3.0f)
    {
        return true;
    }
    return false;
}

/**
 * Offset/Address/Size: 0x204 | 0x8015DF08 | size: 0x1A8
 */
void ChainChomp::Move(float fDeltaT)
{
    nlPolar aChainSpeed;
    nlVector3 v3NewVelocity;
    float fSeekSpeed;

    fSeekSpeed = 60000.0f;

    if (meChainChompState == CHAIN_STATE_FALL)
    {
        fSeekSpeed = 180000.0f;
        mfDesiredSpeed = g_pGame->m_pGameTweaks->fChainChompSpeed;
        goto after_speed;
    }
    else if (meChainChompState == CHAIN_STATE_RECOVER)
    {
        fSeekSpeed = 300000.0f;
        goto after_speed;
    }
    else if (meChainChompState != CHAIN_STATE_LEAVE)
    {
        if (AvoidSidelines(this))
        {
            float fHalf = 0.5f;
            fSeekSpeed = 180000.0f;
            mfDesiredSpeed = g_pGame->m_pGameTweaks->fChainChompSpeed * fHalf;
            goto after_speed;
        }
    }

    mfDesiredSpeed = g_pGame->m_pGameTweaks->fChainChompSpeed;
after_speed:

    nlCartesianToPolar(aChainSpeed, mv3Velocity);

    if (aChainSpeed.r < 0.05f)
    {
        aChainSpeed.a = maFacingDirection;
    }

    float fNewSpeed = SeekSpeed(aChainSpeed.r, mfDesiredSpeed, 25.0f, 45.0f, fDeltaT);

    maFacingDirection = SeekDirection(aChainSpeed.a, maDesiredFacingDirection, fSeekSpeed, 3000.0f, fDeltaT);

    nlPolarToCartesian(v3NewVelocity.f.x, v3NewVelocity.f.y, maFacingDirection, fNewSpeed);
    v3NewVelocity.f.z = mv3Velocity.f.z;
    mv3Velocity = v3NewVelocity;
}

/**
 * Offset/Address/Size: 0x0 | 0x8015DD04 | size: 0x204
 */
void ChainChomp::DrawShadow(const cPoseAccumulator& pa, const nlMatrix4& worldMatrix)
{
    if (gbChainChompProjectedShadow)
    {
        SkinAnimatedNPC::DrawShadow(mpLastModel, mWorldMatrix);
        return;
    }

    nlMatrix4& nodeMatrix = pa.GetNodeMatrix(3);

    nlVector3 v3ModelPosition = nodeMatrix.GetTranslation();
    float y = v3ModelPosition.f.y;

    float frac = (mv3Position.f.z - 55.0f) / 200.0f;
    if (frac < 0.0f)
        frac = 0.0f;
    if (frac > 1.0f)
        frac = 1.0f;

    float fAlpha = 200.0f * (1.0f - frac) + 80.0f * frac;
    float radius = g_pGame->m_pGameTweaks->fChainChompRadius;
    float half_dim = 5.0f * frac + (1.0f - frac) * (1.5f * radius);

    int alpha = (int)fAlpha;
    if (alpha < 0)
        alpha = 0;
    if (alpha > 255)
        alpha = 255;

    nlColour c;
    c.c[0] = 255;
    c.c[1] = 255;
    c.c[2] = 255;
    c.c[3] = (unsigned char)alpha;

    glQuad3 quad;
    quad.m_pos[0].f.x = v3ModelPosition.f.x - half_dim;
    quad.m_pos[0].f.y = y - half_dim;
    quad.m_pos[0].f.z = 0.02f;
    quad.m_pos[1].f.x = v3ModelPosition.f.x - half_dim;
    quad.m_pos[1].f.y = y + half_dim;
    quad.m_pos[1].f.z = 0.02f;
    quad.m_pos[2].f.x = v3ModelPosition.f.x + half_dim;
    quad.m_pos[2].f.y = y + half_dim;
    quad.m_pos[2].f.z = 0.02f;
    quad.m_pos[3].f.x = v3ModelPosition.f.x + half_dim;
    quad.m_pos[3].f.y = y - half_dim;
    quad.m_pos[3].f.z = 0.02f;

    quad.m_uv[0].f.x = 1.0f;
    quad.m_uv[0].f.y = 1.0f;
    quad.m_uv[1].f.x = 0.0f;
    quad.m_uv[1].f.y = 1.0f;
    quad.m_uv[2].f.x = 0.0f;
    quad.m_uv[2].f.y = 0.0f;
    quad.m_uv[3].f.x = 1.0f;
    quad.m_uv[3].f.y = 0.0f;

    quad.m_colour[3] = c;
    quad.m_colour[2] = c;
    quad.m_colour[1] = c;
    quad.m_colour[0] = c;

    glSetDefaultState(true);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTexture(glGetTexture("shadows/blob"), GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 3);
    glSetCurrentTextureState(glHandleizeTextureState());

    quad.Attach(GLV_Unshadowed, 0, true);
}
