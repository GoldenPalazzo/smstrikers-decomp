#include "Game/Render/Bowser.h"
#include "Game/SAnim/pnFeather.h"
#include "Game/SAnim/pnBlender.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/HeadTrack.h"
#include "Game/AI/Powerups.h"
#include "Game/AI/Fielder.h"
#include "Game/Field.h"
#include "Game/Ball.h"
#include "Game/Physics/PhysicsShell.h"
#include "Game/Physics/PhysicsBanana.h"
#include "Game/Sys/eventman.h"
#include "Game/ReplayManager.h"
#include "NL/nlString.h"
#include "Game/PoseAccumulator.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Render/NetMesh.h"
#include "Game/Game.h"
#include "Game/Audio/AudioLoader.h"
// #include "Game/SoundProps/bowsergensoundproperties.h"
#include "math.h"

SoundPropAccessor* gpBOWSERSoundPropAccessor;

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

float Bowser::mfYAxisTilt = 0.0f;

extern nlVector3 gv3BowserHomePosition;

/**
 * Offset/Address/Size: 0x4BBC | 0x8015D930 | size: 0x2C
 */
void AnimSoundCallback(unsigned int eventID)
{
    g_pEventManager->CreateValidEvent(eventID, 0x14);
}

/**
 * Offset/Address/Size: 0x4638 | 0x8015D3AC | size: 0x584
 */
Bowser::Bowser(cSHierarchy& pHierarchy, int nModelID, PhysicsNPC& mpPhysObj, cInventory<cSAnim>* pInventorySAnim)
    : SkinAnimatedMovableNPC(pHierarchy, nModelID, mpPhysObj)
{
}

/**
 * Offset/Address/Size: 0x44F0 | 0x8015D264 | size: 0x148
 */
Bowser::~Bowser()
{
}

extern "C" cPN_Blender* __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(cPN_Blender*, cPoseNode*, cPoseNode*, float);
extern "C" cPN_SAnimController* __ct__19cPN_SAnimControllerFP6cSAnimPC12AnimRetarget9ePlayModePFUiP19cPN_SAnimController_vUib(cPN_SAnimController*, cSAnim*, const AnimRetarget*, ePlayMode, void (*)(unsigned int, cPN_SAnimController*), unsigned int, bool);

/**
 * Offset/Address/Size: 0x325C | 0x8015BFD0 | size: 0x1294
 */
void Bowser::Update(float)
{
}

/**
 * Offset/Address/Size: 0x3054 | 0x8015BDC8 | size: 0x208
 * TODO: 99.96% match - remaining diff is one branch immediate in the ball
 * damage gate; switch jump table body matches target with case set
 * 0x04/0x0D/0x0E/0x0F/0x13/0x14.
 */
void Bowser::CollisionCallback(PhysicsObject* pObjA, PhysicsObject*, const nlVector3&)
{
    Bowser* pObj = (Bowser*)((PhysicsNPC*)this)->mpAINPC;
    cCharacter* pCharacter = NULL;
    int type = pObjA->GetObjectType();

    switch (type)
    {
    case 4:
    case 0x0D:
    case 0x0E:
        pCharacter = (cCharacter*)((PhysicsCharacter*)pObjA->m_parentObject)->m_pAICharacter;
        break;

    case 0x0F:
    {
        cBall* pBall = ((PhysicsAIBall*)pObjA)->m_pAIBall;
        bool shouldDamage = false;
        u32 timer = pBall->m_tShotTimer.m_uPackedTime;
        if (timer != 0)
        {
            if (pBall->mbCanDamage)
            {
                shouldDamage = true;
            }
        }
        if (!shouldDamage)
        {
            shouldDamage = false;
            if (timer != 0)
            {
                if (pBall->m_unk_0xA4)
                {
                    shouldDamage = true;
                }
            }
        }
        if (shouldDamage)
        {
            g_pEventManager->CreateValidEvent(0x62, 0x14);
        }
        if (pBall->m_pOwner != NULL)
        {
            pCharacter = (cCharacter*)pBall->m_pOwner;
        }
        else
        {
            if (pBall->m_pPassTarget != NULL)
            {
                pBall->ClearPassTarget();
            }
            if (pBall->m_tShotTimer.m_uPackedTime != 0)
            {
                pBall->ClearShotInProgress();
            }
        }
        break;
    }

    case 0x13:
    {
        PowerupBase* pPowerup = ((PhysicsShell*)pObjA)->m_pPowerupObject;
        if (!pPowerup->m_bShouldDestroy)
        {
            if (pPowerup->meSize == POWERUPSIZE_LARGE)
            {
                g_pEventManager->CreateValidEvent(0x62, 0x14);
            }
        }
        ((PhysicsShell*)pObjA)->m_pPowerupObject->m_bShouldDestroy = true;
        break;
    }

    case 0x14:
    {
        PowerupBase* pPowerup = ((PhysicsBanana*)pObjA)->m_pPowerupObject;
        if (!pPowerup->m_bShouldDestroy)
        {
            if (pPowerup->meSize == POWERUPSIZE_LARGE)
            {
                g_pEventManager->CreateValidEvent(0x62, 0x14);
            }
        }
        ((PhysicsBanana*)pObjA)->m_pPowerupObject->m_bShouldDestroy = true;
        break;
    }
    }

    if (pCharacter != NULL)
    {
        if (pCharacter->m_eClassType == FIELDER)
        {
            if (!((cFielder*)pCharacter)->IsFallenDown(0.0f))
            {
                if (!((cFielder*)pCharacter)->IsInvincible())
                {
                    Event* pEvent = g_pEventManager->CreateValidEvent(0x30, 0x20);
                    CollisionBowserPlayerData* pData = new (&pEvent->m_data) CollisionBowserPlayerData();
                    pData->pFielder = (cFielder*)pCharacter;
                    pData->pBowser = pObj;
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x2B64 | 0x8015B8D8 | size: 0x4F0
 * TODO: 94.79% match - beq/b vs bne branch pattern, ResetBowserTimer code placement,
 *       and lfd constant propagation vs lfs reload for fabsf(mfYAxisTilt). All -inline deferred artifacts.
 */
void Bowser::ActionInit()
{
    eBowserAttackType savedAttackType;
    u32 savedVisible;

    if (nlSingleton<GameInfoManager>::s_pInstance->IsTiltingFieldOn())
        return;
    if (!nlSingleton<GameInfoManager>::s_pInstance->IsBowserAttackEnabled())
        return;
    if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode != false)
    {
        return;
    }

    mbIsVisible = false;
    mbFirstTime = true;

    if (mAttackType == BOWSER_ATTACK_STOMP)
    {
        mStompStage++;
        if (fabsf(cCameraManager::m_UpVectorStack[cCameraManager::m_UpVectorStackSize].f.x) < 0.001f
            && fabsf(cCameraManager::m_UpVectorStack[cCameraManager::m_UpVectorStackSize].f.y) < 0.001f
            && mStompStage == 2)
        {
            mAttackType = BOWSER_ATTACK_ROLL;
            EmissionManager::Destroy((unsigned long)this, fxGetGroup("bowser_fire"));
            g_pEventManager->CreateValidEvent(0x65, 0x14);
            savedVisible = mbIsVisible;
            mbIsVisible = false;
            meBowserState = BOWSER_STATE_HIDDEN;
            mfDesiredSpeed = 0.0f;
            if (mpFeatherBlender->GetChild(1) != NULL)
            {
                delete mpFeatherBlender->GetChild(1);
                mpFeatherBlender->SetChild(1, NULL);
            }
            mpFeatherController = NULL;
            SetPosition(gv3BowserHomePosition);
            mv3Velocity = v3Zero;
            maFacingDirection = 0;
            mpPhysObj->DisableCollisions();
            if (mAttackType == BOWSER_ATTACK_STOMP && mStompStage != 2)
            {
                g_pGame->ResetBowserTimer(g_pGame->m_pGameTweaks->unk31C);
                return;
            }
            savedAttackType = mAttackType;
            mfYAxisTilt = 0.0f;
            cCameraManager::SetWorldUpVectorTilt(0.0f, 0.0f);
            if (g_pBall != NULL)
            {
                PhysicsAIBall* pPhys = g_pBall->m_pPhysicsBall;
                if (pPhys != NULL)
                {
                    if (fabsf(mfYAxisTilt) > 0.01f)
                    {
                        nlVector3 tiltForce = { 0.0f, 0.0f, 0.0f };
                        tiltForce.f.x = -mfYAxisTilt * g_pGame->m_pGameTweaks->unk338;
                        pPhys->m_v3TiltForce = tiltForce;
                        g_pBall->m_pPhysicsBall->m_bUseTiltForce = true;
                    }
                    else
                    {
                        pPhys->m_bUseTiltForce = false;
                    }
                }
            }
            mAttackType = BOWSER_ATTACK_ROLL;
            if (g_pGame->m_pGameTweaks->unk310 < 0.0f)
                g_pGame->ResetBowser();
            if (!mbAlive)
                return;
            mbAlive = false;
            if (!nlSingleton<GameInfoManager>::s_pInstance->IsBowserAttackEnabled())
                return;
            if (savedAttackType == BOWSER_ATTACK_STOMP)
                return;
            if (savedVisible == 0)
                return;
            g_pEventManager->CreateValidEvent(0x37, 0x14);
            return;
        }
    }
    else
    {
        float savedDuration = g_pGame->m_fGameDuration;
        float gameTime = g_pGame->GetGameTime();
        if (savedDuration - gameTime < 15.0f)
            return;
        mbAlive = true;
        mAttackType = (eBowserAttackType)nlRandom(NUM_BOWSER_ATTACKS, &nlDefaultSeed);
        mbDoTilt = true;
        switch (mAttackType)
        {
        case BOWSER_ATTACK_ROLL:
            ActionFall();
            mbDoTilt = false;
            break;
        case BOWSER_ATTACK_JUMP:
        case BOWSER_ATTACK_CRAZY:
            ActionFall();
            break;
        case BOWSER_ATTACK_STOMP:
            mStompStage = 0;
            break;
        }
    }

    if (mAttackType != BOWSER_ATTACK_STOMP)
        return;
    if (mStompStage == 0)
    {
        cBaseCamera* cam = nlDLRingGetStart(cCameraManager::m_cameraStack);
        const nlVector3& target = cam->GetTargetPosition();
        mv3TargetPos = target;
    }
    else
    {
        mv3TargetPos = v3Zero;
        if (mfYAxisTilt > 0.0f)
            mv3TargetPos.f.x = 8.0f;
        else
            mv3TargetPos.f.x = -8.0f;
    }
    if (mStompStage != 2)
    {
        float clampX = cField::GetGoalLineX(1u) - 3.0f;
        if (fabsf(mv3TargetPos.f.x) > clampX)
        {
            if (mv3TargetPos.f.x > 0.0f)
                mv3TargetPos.f.x = clampX;
            else
                mv3TargetPos.f.x = -clampX;
        }
        float clampY = cField::GetSidelineY(1u) - 3.0f;
        if (fabsf(mv3TargetPos.f.y) > clampY)
        {
            if (mv3TargetPos.f.y < 0.0f)
                mv3TargetPos.f.y = -clampY;
            else
                mv3TargetPos.f.y = clampY;
        }
        if (fabsf(mv3TargetPos.f.x) < 8.0f)
        {
            if (mv3TargetPos.f.x < 0.0f)
                mv3TargetPos.f.x = -8.0f;
            else
                mv3TargetPos.f.x = 8.0f;
        }
    }
    ActionStomp();
}

/**
 * Offset/Address/Size: 0x28A4 | 0x8015B618 | size: 0x2C0
 */
void Bowser::ActionThrow()
{
    if (CheckForAbort())
    {
        return;
    }

    if (!GameInfoManager::s_pInstance->GetGameplayOptions().PowerUps)
    {
        ActionIdle();
        return;
    }

    if (mAnimID != BOWSER_ANIM_WALK)
    {
        mAnimID = BOWSER_ANIM_WALK;

        cPN_SAnimController* controller = NULL;

        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
        }

        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
        {
            controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
            cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
        }

        controller = new (controller) cPN_SAnimController(
            mpAnim[BOWSER_ANIM_WALK],
            (const AnimRetarget*)0,
            PM_CYCLIC,
            (void (*)(unsigned int, cPN_SAnimController*))0,
            (unsigned int)0,
            (bool)0);

        cPN_Blender* blender;

        if (mpFeatherBlender->GetChild(0) != NULL)
        {
            blender = NULL;

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
            }

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
            {
                blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
                cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
            }

            // fake match - using the constructor directly is currently not matching because of a mr missing..
            if (blender != NULL)
            {
                blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, 0.15f);
            }
            // new (blender) cPN_Blender(*mpFeatherBlender->GetChildPtr(0), controller, 0.15f);
            // if (blender != NULL)
            // {
            //     blender = new cPN_Blender(*mpFeatherBlender->GetChildPtr(0), controller, 0.15f);
            // }
        }
        else
        {
            blender = (cPN_Blender*)controller;
        }

        mpFeatherBlender->SetChild(0, blender);
        mpAnimController = controller;
    }

    meBowserState = BOWSER_STATE_THROW;

    if (mpFeatherBlender->GetChild(1) != NULL)
    {
        delete mpFeatherBlender->GetChild(1);
        mpFeatherBlender->SetChild(1, NULL);
    }

    cPN_SAnimController* controller;
    mpFeatherController = (controller = NULL);

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }

    // fake match - using the constructor directly is currently not matching because of a mr missing..
    if (controller != NULL)
    {
        controller = __ct__19cPN_SAnimControllerFP6cSAnimPC12AnimRetarget9ePlayModePFUiP19cPN_SAnimController_vUib(controller,
            mpAnim[BOWSER_ANIM_THROW],
            nullptr,
            PM_HOLD,
            nullptr,
            0,
            false);
    }
    // new (blender) cPN_Blender(*mpFeatherBlender->GetChildPtr(0), controller, 0.2f);
    // if (controller != NULL)
    // {
    //     controller = new cPN_SAnimController(mpAnim[BOWSER_ANIM_THROW], nullptr, PM_HOLD, nullptr, 0, false);
    // }

    mpFeatherController = controller;

    mpFeatherBlender->ClearNodeWeights();
    mpFeatherBlender->SetNodeWeight(0xE, 1.0f, 0.2f);
    mpFeatherBlender->SetChild(1, mpFeatherController);
    mpFeatherBlender->BeginBlendIn(0.2f);

    if (g_pBall->GetOwnerFielder() != NULL && !g_pBall->GetOwnerFielder()->IsInvincible())
    {
        mpTarget = g_pBall->GetOwnerFielder();
    }
    else
    {
        mpTarget = FindPowerupTarget((cFielder*)NULL, this);
    }
}

/**
 * Offset/Address/Size: 0x26DC | 0x8015B450 | size: 0x1C8
 * TODO: remaining diff is in cPN_Blender placement-new lowering
 * (extra null-check/register move vs target direct ctor call shape).
 */
void Bowser::ActionRoll()
{
    EmissionManager::Destroy((unsigned long)this, fxGetGroup("bowser_fire"));
    g_pEventManager->CreateValidEvent(0x65, 0x14);
    g_pEventManager->CreateValidEvent(0x61, 0x14);

    meBowserState = BOWSER_STATE_ROLL;

    cPN_SAnimController* controller = NULL;
    mAnimID = BOWSER_ANIM_ROLL;

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }

    controller = new (controller) cPN_SAnimController(
        mpAnim[BOWSER_ANIM_ROLL],
        (const AnimRetarget*)0,
        PM_HOLD,
        (void (*)(unsigned int, cPN_SAnimController*))0,
        (unsigned int)0,
        (bool)0);

    cPN_Blender* blender;

    if (mpFeatherBlender->GetChild(0) != NULL)
    {
        blender = NULL;

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
        }

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
        {
            blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
            cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
        }

        // fake match - using the constructor directly is currently not matching because of a mr missing..
        if (blender != NULL)
        {
            blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, 0.2f);
        }
        // new (blender) cPN_Blender(*mpFeatherBlender->GetChildPtr(0), controller, 0.2f);
    }
    else
    {
        blender = (cPN_Blender*)controller;
    }

    mpFeatherBlender->SetChild(0, blender);
    mpAnimController = controller;

    if (g_pBall->GetOwnerFielder() != NULL && !g_pBall->GetOwnerFielder()->IsInvincible())
    {
        mpTarget = g_pBall->GetOwnerFielder();
    }
    else
    {
        mpTarget = FindPowerupTarget((cFielder*)NULL, this);
    }
}

/**
 * Offset/Address/Size: 0x24B4 | 0x8015B228 | size: 0x228
 */
void Bowser::ActionStomp()
{
    nlVector3 vel;
    mAnimID = 1;
    cPN_SAnimController* controller = AllocateSAnimController();
    controller = new (controller) cPN_SAnimController(mpAnim[1], (const AnimRetarget*)0, PM_HOLD, (void (*)(unsigned int, cPN_SAnimController*))0, (unsigned int)0, (bool)0);
    if (mpFeatherBlender->GetChild(0) != NULL)
    {
        delete mpFeatherBlender->GetChild(0);
    }
    mpFeatherBlender->SetChild(0, controller);
    mpAnimController = controller;
    meBowserState = BOWSER_STATE_FALL;
    mfDesiredSpeed = 0.0f;
    nlVector3 targetPos = mv3TargetPos;
    float goalLineX = cField::GetGoalLineX(1U);
    float limitX = goalLineX - 5.0f;
    if ((float)fabs(targetPos.f.x) > limitX)
    {
        if (targetPos.f.x > 0.0f)
        {
            targetPos.f.x = limitX;
        }
        else
        {
            targetPos.f.x = -limitX;
        }
    }
    float sidelineY = cField::GetSidelineY(1U);
    float limitY = sidelineY - 5.0f;
    if ((float)fabs(targetPos.f.y) > limitY)
    {
        if (targetPos.f.y < 0.0f)
        {
            targetPos.f.y = -limitY;
        }
        else
        {
            targetPos.f.y = limitY;
        }
    }
    targetPos.f.z = 200.0f;
    targetPos.f.z = 0.0f;
    targetPos.f.y += 5.0f;
    SetPosition(targetPos);
    maFacingDirection = 0xC000;
    maDesiredFacingDirection = 0xC000;
    vel = v3Zero;
    vel.f.y = 50.0f;
    mv3Velocity = vel;
    mtActiveTimer.m_uPackedTime = 0;
}

/**
 * Offset/Address/Size: 0x2314 | 0x8015B088 | size: 0x1A0
 */
void Bowser::ActionDescend(float fBlendTime)
{
    cPN_SAnimController* controller = NULL;

    mAnimID = 1;

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }

    controller = new (controller) cPN_SAnimController(mpAnim[1], (const AnimRetarget*)0, PM_HOLD, (void (*)(unsigned int, cPN_SAnimController*))0, (unsigned int)0, (bool)0);

    cPN_Blender* blender;

    if (mpFeatherBlender->GetChild(0) != NULL)
    {
        if (fBlendTime > 0.0f)
        {
            blender = NULL;

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
            }

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
            {
                blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
                cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
            }

            if (blender != NULL)
            {
                // this is a fake match - using the constructor directly is currently not matching because of a mr missing..
                blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, fBlendTime);
            }
        }
        else
        {
            delete mpFeatherBlender->GetChild(0);
            blender = (cPN_Blender*)controller;
        }
    }
    else
    {
        blender = (cPN_Blender*)controller;
    }

    mpFeatherBlender->SetChild(0, blender);
    mpAnimController = controller;
    meBowserState = BOWSER_STATE_FALL;
}

/**
 * Offset/Address/Size: 0x2080 | 0x8015ADF4 | size: 0x294
 */
void Bowser::ActionFall()
{
    float timerSeconds = g_pGame->m_pGameTweaks->unk314;
    nlVector3 vel;

    if (timerSeconds < g_pGame->m_pGameTweaks->unk318)
    {
        timerSeconds += nlRandomf(g_pGame->m_pGameTweaks->unk318 - timerSeconds, &nlDefaultSeed);
    }

    mtActiveTimer.SetSeconds(timerSeconds);
    mAnimID = BOWSER_ANIM_LAND;

    cPN_SAnimController* controller = NULL;

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }

    if (controller != NULL)
    {
        controller = __ct__19cPN_SAnimControllerFP6cSAnimPC12AnimRetarget9ePlayModePFUiP19cPN_SAnimController_vUib(
            controller,
            mpAnim[BOWSER_ANIM_LAND],
            (const AnimRetarget*)0,
            PM_HOLD,
            (void (*)(unsigned int, cPN_SAnimController*))0,
            (unsigned int)0,
            (bool)0);
    }

    if (mpFeatherBlender->GetChild(0) != NULL)
    {
        delete mpFeatherBlender->GetChild(0);
    }

    mpFeatherBlender->SetChild(0, controller);
    mpAnimController = controller;

    meBowserState = BOWSER_STATE_FALL;
    mfDesiredSpeed = 0.0f;

    cBaseCamera* camera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
    mv3TargetPos = camera->GetTargetPosition();

    nlVector3 targetPos = mv3TargetPos;

    float goalLineX = cField::GetGoalLineX(1U);
    float limitX = goalLineX - 5.0f;
    if ((float)fabs(targetPos.f.x) > limitX)
    {
        if (targetPos.f.x > 0.0f)
        {
            targetPos.f.x = limitX;
        }
        else
        {
            targetPos.f.x = -limitX;
        }
    }

    float sidelineY = cField::GetSidelineY(1U);
    float limitY = sidelineY - 5.0f;
    if ((float)fabs(targetPos.f.y) > limitY)
    {
        if (targetPos.f.y < 0.0f)
        {
            targetPos.f.y = -limitY;
        }
        else
        {
            targetPos.f.y = limitY;
        }
    }

    targetPos.f.z = 200.0f;
    targetPos.f.z = 0.0f;
    targetPos.f.y += 5.0f;
    SetPosition(targetPos);

    maFacingDirection = 0xC000;
    maDesiredFacingDirection = 0xC000;

    vel = v3Zero;
    vel.f.y = 50.0f;
    mv3Velocity = vel;
}

/**
 * Offset/Address/Size: 0x1CE4 | 0x8015AA58 | size: 0x39C
 * TODO: 99.24% match - f31=vx vs target f31=0.0f register allocation diff,
 * likely -inline deferred vs -inline auto on decomp.me.
 */
void Bowser::ActionJump()
{
    meBowserState = BOWSER_STATE_JUMP;
    mAnimID = BOWSER_ANIM_JUMP;

    cPN_SAnimController* controller = NULL;

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }

    if (controller != NULL)
    {
        controller = __ct__19cPN_SAnimControllerFP6cSAnimPC12AnimRetarget9ePlayModePFUiP19cPN_SAnimController_vUib(
            controller,
            mpAnim[BOWSER_ANIM_JUMP],
            (const AnimRetarget*)0,
            PM_HOLD,
            (void (*)(unsigned int, cPN_SAnimController*))0,
            (unsigned int)0,
            (bool)0);
    }

    cPN_Blender* blender;

    if (mpFeatherBlender->GetChild(0) != NULL)
    {
        blender = NULL;

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
        }

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
        {
            blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
            cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
        }

        if (blender != NULL)
        {
            blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, 0.2f);
        }
    }
    else
    {
        blender = (cPN_Blender*)controller;
    }

    mpFeatherBlender->SetChild(0, blender);
    mpAnimController = controller;

    nlVector3 v3JumpPos = g_pBall->m_v3Position;
    v3JumpPos.f.x += nlRandomf(5.0f, &nlDefaultSeed) - 1.5f;

    float fMaxTilt = g_pGame->m_pGameTweaks->unk32C;
    if (v3JumpPos.f.x > 0.0f && mfYAxisTilt <= -fMaxTilt)
    {
        v3JumpPos.f.x *= -1.0f;
    }
    else if (v3JumpPos.f.x < 0.0f && mfYAxisTilt >= fMaxTilt)
    {
        v3JumpPos.f.x *= -1.0f;
    }

    if (v3JumpPos.f.y < 0.0f)
    {
        v3JumpPos.f.y += 1.0f + nlRandomf(5.0f, &nlDefaultSeed);
    }
    else
    {
        v3JumpPos.f.y -= 1.0f + nlRandomf(5.0f, &nlDefaultSeed);
    }

    float goalLineX = cField::GetGoalLineX(1U);
    float limitX = goalLineX - 5.0f;
    if ((float)fabs(v3JumpPos.f.x) > limitX)
    {
        if (v3JumpPos.f.x > 0.0f)
        {
            v3JumpPos.f.x = limitX;
        }
        else
        {
            v3JumpPos.f.x = -limitX;
        }
    }

    float sidelineY = cField::GetSidelineY(1U);
    float limitY = sidelineY - 5.0f;
    if ((float)fabs(v3JumpPos.f.y) > limitY)
    {
        if (v3JumpPos.f.y < 0.0f)
        {
            v3JumpPos.f.y = -limitY;
        }
        else
        {
            v3JumpPos.f.y = limitY;
        }
    }

    float vx = v3JumpPos.f.x - mv3Position.f.x;
    float vy = v3JumpPos.f.y - mv3Position.f.y;

    nlVector3 v3JumpVel;
    v3JumpVel.f.x = vx;
    v3JumpVel.f.y = vy;
    v3JumpVel.f.z = v3JumpPos.f.z - mv3Position.f.z;
    v3JumpVel.f.z = 0.0f;

    float speed = nlGetLength3D(vx, vy, v3JumpVel.f.z);

    v3JumpVel.f.y = vy * 0.5f;
    v3JumpVel.f.x = vx * 0.5f;
    v3JumpVel.f.z *= 0.5f;

    mfDesiredSpeed = 2.2f * speed;
    maDesiredFacingDirection = (u16)(s32)(10430.378f * nlATan2f(v3JumpVel.f.y, v3JumpVel.f.x));
    mv3Velocity = v3JumpVel;
}

/**
 * Offset/Address/Size: 0x1A90 | 0x8015A804 | size: 0x254
 */
void Bowser::ActionHide()
{
    EmissionManager::Destroy((unsigned long)this, fxGetGroup("bowser_fire"));
    g_pEventManager->CreateValidEvent(0x65, 0x14);

    bool wasVisible = mbIsVisible;
    mbIsVisible = false;
    meBowserState = BOWSER_STATE_HIDDEN;
    mfDesiredSpeed = 0.0f;

    if (mpFeatherBlender->GetChild(1) != NULL)
    {
        cPoseNode* pChild = mpFeatherBlender->GetChild(1);
        delete pChild;
        mpFeatherBlender->SetChild(1, NULL);
    }

    mpFeatherController = NULL;
    SetPosition(gv3BowserHomePosition);

    mv3Velocity = v3Zero;
    maFacingDirection = 0;

    mpPhysObj->DisableCollisions();

    if (!(mAttackType == BOWSER_ATTACK_STOMP && mStompStage != 2))
    {
        eBowserAttackType oldAttackType = mAttackType;

        SetTiltParameters(0.0f);
        mAttackType = BOWSER_ATTACK_ROLL;

        if (g_pGame->m_pGameTweaks->unk310 < 0.0f)
        {
            g_pGame->ResetBowser();
        }

        if (mbAlive)
        {
            mbAlive = false;

            if (GameInfoManager::s_pInstance->IsBowserAttackEnabled() && oldAttackType != BOWSER_ATTACK_STOMP && wasVisible)
            {
                g_pEventManager->CreateValidEvent(0x37, 0x14);
            }
        }
    }
    else
    {
        g_pGame->ResetBowserTimer(g_pGame->m_pGameTweaks->unk31C);
    }
}

/**
 * Offset/Address/Size: 0x177C | 0x8015A4F0 | size: 0x314
 * TODO: 99.39% match - inlined SetTiltParameters(0.0f) temp stack slots are
 * swapped between the first and second call sites.
 */
void Bowser::ActionReset()
{
    mAttackType = BOWSER_ATTACK_ROLL;

    if (mbAlive)
    {
        eBowserAttackType oldAttackType;
        bool wasVisible;

        EmissionManager::Destroy((unsigned long)this, fxGetGroup("bowser_fire"));
        g_pEventManager->CreateValidEvent(0x65, 0x14);

        wasVisible = mbIsVisible;
        mbIsVisible = false;
        meBowserState = BOWSER_STATE_HIDDEN;
        mfDesiredSpeed = 0.0f;

        if (mpFeatherBlender->GetChild(1) != NULL)
        {
            cPoseNode* pChild = mpFeatherBlender->GetChild(1);
            delete pChild;
            mpFeatherBlender->SetChild(1, NULL);
        }

        mpFeatherController = NULL;
        SetPosition(gv3BowserHomePosition);

        mv3Velocity = v3Zero;
        maFacingDirection = 0;

        mpPhysObj->DisableCollisions();

        if (!(mAttackType == BOWSER_ATTACK_STOMP && mStompStage != 2))
        {
            oldAttackType = mAttackType;

            SetTiltParameters(0.0f);
            mAttackType = BOWSER_ATTACK_ROLL;

            if (g_pGame->m_pGameTweaks->unk310 < 0.0f)
            {
                g_pGame->ResetBowser();
            }

            if (mbAlive)
            {
                mbAlive = false;

                if (GameInfoManager::s_pInstance->IsBowserAttackEnabled() && oldAttackType != BOWSER_ATTACK_STOMP && wasVisible)
                {
                    g_pEventManager->CreateValidEvent(0x37, 0x14);
                }
            }
        }
        else
        {
            g_pGame->ResetBowserTimer(g_pGame->m_pGameTweaks->unk31C);
        }
    }

    mbFirstTime = true;
    SetTiltParameters(0.0f);
}

/**
 * Offset/Address/Size: 0x1534 | 0x8015A2A8 | size: 0x248
 */
void Bowser::ActionLeave()
{
    if (meBowserState == BOWSER_STATE_LEAVE)
        return;

    if (GameInfoManager::s_pInstance->IsBowserAttackEnabled())
    {
        g_pEventManager->CreateValidEvent(0x3b, 0x14);
    }

    meBowserState = BOWSER_STATE_LEAVE;
    mAnimID = BOWSER_ANIM_JUMP;

    cPN_SAnimController* controller = NULL;

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }

    controller = new (controller) cPN_SAnimController(mpAnim[BOWSER_ANIM_JUMP], (const AnimRetarget*)0, PM_HOLD, (void (*)(unsigned int, cPN_SAnimController*))0, (unsigned int)0, (bool)0);

    cPN_Blender* blender;

    if (mpFeatherBlender->GetChild(0) != NULL)
    {
        blender = NULL;

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
        }

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
        {
            blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
            cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
        }

        if (blender != NULL)
        {
            blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, 1.0f);
        }
    }
    else
    {
        blender = (cPN_Blender*)controller;
    }

    mpFeatherBlender->SetChild(0, blender);
    mpAnimController = controller;

    cBaseCamera* camera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
    nlVector3 v3CameraTarget = camera->GetTargetPosition();

    nlVector3 v3Velocity = { 200.0f, 0.0f, 0.0f };

    unsigned short aDesired = 0;

    if (v3CameraTarget.f.x > mv3Position.f.x)
    {
        v3Velocity.f.x *= -1.0f;
        aDesired = 0x8000;
    }

    maDesiredFacingDirection = aDesired;
    mv3Velocity = v3Velocity;

    mtStateTimer.SetSeconds(3.0f);

    if (mpFeatherBlender->GetChild(1) != NULL)
    {
        mpFeatherBlender->BeginBlendOut(0.25f);
    }

    mpFeatherController = NULL;
}

/**
 * Offset/Address/Size: 0x1384 | 0x8015A0F8 | size: 0x1B0
 */
void Bowser::Move(float fDeltaT)
{
    float speed = nlSqrt(mv3Velocity.f.x * mv3Velocity.f.x + mv3Velocity.f.y * mv3Velocity.f.y + mv3Velocity.f.z * mv3Velocity.f.z, true);

    float newSpeed = SeekSpeed(speed, mfDesiredSpeed, 100.0f, 200.0f, fDeltaT);

    maFacingDirection = SeekDirection(maFacingDirection, maDesiredFacingDirection, 0.05f, 0.5f, fDeltaT);

    nlVector3 vel;
    nlVector3 newPos;
    nlPolarToCartesian(vel.f.x, vel.f.y, maDesiredFacingDirection, newSpeed);
    vel.f.z = mv3Velocity.f.z;
    mv3Velocity = vel;

    newPos = mv3Position;
    newPos.f.x += vel.f.x * fDeltaT;
    newPos.f.y += vel.f.y * fDeltaT;

    float goalLineX = cField::GetGoalLineX(1U);
    float limitX = goalLineX - 5.0f;
    if ((float)fabs(newPos.f.x) > limitX)
    {
        if (newPos.f.x > 0.0f)
        {
            newPos.f.x = limitX;
        }
        else
        {
            newPos.f.x = -limitX;
        }
    }

    float sidelineY = cField::GetSidelineY(1U);
    float limitY = sidelineY - 5.0f;
    if ((float)fabs(newPos.f.y) > limitY)
    {
        if (newPos.f.y < 0.0f)
        {
            newPos.f.y = -limitY;
        }
        else
        {
            newPos.f.y = limitY;
        }
    }

    SetPosition(newPos);
}

/**
 * Offset/Address/Size: 0x1114 | 0x80159E88 | size: 0x270
 */
void Bowser::ActionIdle()
{
    if (CheckForAbort())
    {
        return;
    }

    meBowserState = BOWSER_STATE_IDLE;

    if (mAnimID != BOWSER_ANIM_WALK)
    {
        mAnimID = BOWSER_ANIM_WALK;

        cPN_SAnimController* controller = NULL;

        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
        }

        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
        {
            controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
            cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
        }

        controller = new (controller) cPN_SAnimController(
            mpAnim[BOWSER_ANIM_WALK],
            (const AnimRetarget*)0,
            PM_CYCLIC,
            (void (*)(unsigned int, cPN_SAnimController*))0,
            (unsigned int)0,
            (bool)0);

        cPN_Blender* blender;

        if (mpFeatherBlender->GetChild(0) != NULL)
        {
            blender = NULL;

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
            }

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
            {
                blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
                cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
            }

            if (blender != NULL)
            {
                blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, 0.5f);
            }
        }
        else
        {
            blender = (cPN_Blender*)controller;
        }

        mpFeatherBlender->SetChild(0, blender);
        mpAnimController = controller;
    }

    nlVector3 pos = mv3Position;
    pos.f.z = 0.0f;
    SetPosition(pos);

    mv3Velocity = v3Zero;

    mtStateTimer.SetSeconds(g_pGame->m_pGameTweaks->unk320);

    EmissionManager::Destroy((unsigned long)this, fxGetGroup("bowser_fire"));
    g_pEventManager->CreateValidEvent(0x65, 0x14);

    EmissionController* controller = EmissionManager::Create(fxGetGroup("bowser_fire"), 0);
    controller->m_uUserData = (unsigned long)this;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateFireEmitter;
        controller->SetUpdateCallback(update);
    }

    g_pEventManager->CreateValidEvent(0x64, 0x14);
}

/**
 * Offset/Address/Size: 0x1044 | 0x80159DB8 | size: 0xD0
 */
void Bowser::SetTiltParameters(float fYAxisTilt)
{
    mfYAxisTilt = fYAxisTilt;
    cCameraManager::SetWorldUpVectorTilt(0.0f, fYAxisTilt);

    if ((g_pBall != NULL) && (g_pBall->m_pPhysicsBall != NULL))
    {
        if ((float)fabs(fYAxisTilt) > 0.01f)
        {
            nlVector3 tiltForce = { 0.0f, 0.0f, 0.0f };
            tiltForce.f.x = -fYAxisTilt * g_pGame->m_pGameTweaks->unk338;

            g_pBall->m_pPhysicsBall->m_v3TiltForce = tiltForce;
            g_pBall->m_pPhysicsBall->m_bUseTiltForce = true;
            return;
        }
        g_pBall->m_pPhysicsBall->m_bUseTiltForce = false;
    }
}

/**
 * Offset/Address/Size: 0x954 | 0x801596C8 | size: 0x6F0
 * TODO: 93.03% match - r29/r31 register swap for 'this' pointer due to dead code
 * GS_END_GAME block's savedVisible/savedAttackType variables changing MWCC graph coloring
 */
bool Bowser::CheckForAbort()
{
    bool isGameplayOrOvertime = false;
    eGameState gameState = g_pGame->m_eGameState;
    if (gameState == GS_GAMEPLAY || gameState == GS_OVERTIME)
        isGameplayOrOvertime = true;

    if (!isGameplayOrOvertime)
    {
        if (meBowserState == BOWSER_STATE_LEAVE)
            return true;

        if (nlSingleton<GameInfoManager>::s_pInstance->IsBowserAttackEnabled())
        {
            g_pEventManager->CreateValidEvent(0x3b, 0x14);
        }

        meBowserState = BOWSER_STATE_LEAVE;
        mAnimID = BOWSER_ANIM_JUMP;

        cPN_SAnimController* controller = NULL;

        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
        }

        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
        {
            controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
            cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
        }

        controller = new (controller) cPN_SAnimController(
            mpAnim[BOWSER_ANIM_JUMP],
            (const AnimRetarget*)0,
            PM_HOLD,
            (void (*)(unsigned int, cPN_SAnimController*))0,
            (unsigned int)0,
            (bool)0);

        cPN_Blender* blender;

        if (mpFeatherBlender->GetChild(0) != NULL)
        {
            blender = NULL;

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
            }

            if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
            {
                blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
                cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
            }

            if (blender != NULL)
            {
                blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, 0.2f);
            }
        }
        else
        {
            blender = (cPN_Blender*)controller;
        }

        mpFeatherBlender->SetChild(0, blender);
        mpAnimController = controller;

        cBaseCamera* camera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
        nlVector3 v3CameraTarget = camera->GetTargetPosition();

        nlVector3 v3Velocity = { 25.0f, 0.0f, 25.0f };
        unsigned short aDesired = 0;

        if (v3CameraTarget.f.x > mv3Position.f.x)
        {
            v3Velocity.f.x *= -1.0f;
            aDesired = 0x8000;
        }

        maDesiredFacingDirection = aDesired;
        mv3Velocity = v3Velocity;

        mtStateTimer.SetSeconds(2.0f);

        if (mpFeatherBlender->GetChild(1) != NULL)
        {
            mpFeatherBlender->BeginBlendOut(0.1f);
        }

        mpFeatherController = NULL;
        return true;
    }

    if (gameState == GS_END_GAME)
    {
        EmissionManager::Destroy((unsigned long)this, fxGetGroup("bowser_fire"));
        g_pEventManager->CreateValidEvent(0x65, 0x14);

        u8 savedVisible = mbIsVisible;
        mbIsVisible = false;
        meBowserState = BOWSER_STATE_HIDDEN;
        mfDesiredSpeed = 0.0f;

        if (mpFeatherBlender->GetChild(1) != NULL)
        {
            delete mpFeatherBlender->GetChild(1);
            mpFeatherBlender->SetChild(1, NULL);
        }

        mpFeatherController = NULL;
        SetPosition(gv3BowserHomePosition);

        mv3Velocity = v3Zero;
        maFacingDirection = 0;

        mpPhysObj->DisableCollisions();

        if (!(mAttackType == BOWSER_ATTACK_STOMP && mStompStage != 2))
        {
            eBowserAttackType savedAttackType = mAttackType;

            SetTiltParameters(0.0f);
            mAttackType = BOWSER_ATTACK_ROLL;

            if (g_pGame->m_pGameTweaks->unk310 < 0.0f)
            {
                g_pGame->ResetBowser();
            }

            if (mbAlive)
            {
                mbAlive = false;

                if (nlSingleton<GameInfoManager>::s_pInstance->IsBowserAttackEnabled()
                    && savedAttackType != BOWSER_ATTACK_STOMP
                    && savedVisible)
                {
                    g_pEventManager->CreateValidEvent(0x37, 0x14);
                }
            }
        }
        else
        {
            g_pGame->ResetBowserTimer(g_pGame->m_pGameTweaks->unk31C);
        }
        return true;
    }

    float fDuration = g_pGame->m_fGameDuration;
    float fRemainingTime = fDuration - g_pGame->GetGameTime();
    if (fRemainingTime >= 15.0f)
        return false;

    if (mAttackType == BOWSER_ATTACK_STOMP)
        return false;

    if (meBowserState == BOWSER_STATE_LEAVE)
        return true;

    if (nlSingleton<GameInfoManager>::s_pInstance->IsBowserAttackEnabled())
    {
        g_pEventManager->CreateValidEvent(0x3b, 0x14);
    }

    meBowserState = BOWSER_STATE_LEAVE;
    mAnimID = BOWSER_ANIM_JUMP;

    cPN_SAnimController* controller = NULL;

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, sizeof(cPN_SAnimController));
    }

    if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
    {
        controller = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
        cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
    }

    controller = new (controller) cPN_SAnimController(
        mpAnim[BOWSER_ANIM_JUMP],
        (const AnimRetarget*)0,
        PM_HOLD,
        (void (*)(unsigned int, cPN_SAnimController*))0,
        (unsigned int)0,
        (bool)0);

    cPN_Blender* blender;

    if (mpFeatherBlender->GetChild(0) != NULL)
    {
        blender = NULL;

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_Blender::m_BlenderSlotPool, sizeof(cPN_Blender));
        }

        if (cPN_Blender::m_BlenderSlotPool.m_FreeList != NULL)
        {
            blender = (cPN_Blender*)cPN_Blender::m_BlenderSlotPool.m_FreeList;
            cPN_Blender::m_BlenderSlotPool.m_FreeList = cPN_Blender::m_BlenderSlotPool.m_FreeList->m_next;
        }

        if (blender != NULL)
        {
            blender = __ct__11cPN_BlenderFP9cPoseNodeP9cPoseNodef(blender, *mpFeatherBlender->GetChildPtr(0), controller, 0.2f);
        }
    }
    else
    {
        blender = (cPN_Blender*)controller;
    }

    mpFeatherBlender->SetChild(0, blender);
    mpAnimController = controller;

    cBaseCamera* camera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
    nlVector3 v3CameraTarget = camera->GetTargetPosition();

    nlVector3 v3Velocity = { 25.0f, 0.0f, 25.0f };
    unsigned short aDesired = 0;

    if (v3CameraTarget.f.x > mv3Position.f.x)
    {
        v3Velocity.f.x *= -1.0f;
        aDesired = 0x8000;
    }

    maDesiredFacingDirection = aDesired;
    mv3Velocity = v3Velocity;

    mtStateTimer.SetSeconds(2.0f);

    if (mpFeatherBlender->GetChild(1) != NULL)
    {
        mpFeatherBlender->BeginBlendOut(0.1f);
    }

    mpFeatherController = NULL;
    return true;
}

/**
 * Offset/Address/Size: 0x888 | 0x801595FC | size: 0xCC
 */
void Bowser::UpdateFireEmitter(EmissionController& controller)
{
    if (ReplayManager::Instance()->mRender != nullptr)
    {
        RenderSnapshot* pRenderSnapshot = ReplayManager::Instance()->mRender;
        cPoseAccumulator* pPoseAccumulator = pRenderSnapshot->mBowser.mPoseAccumulator;

        controller.SetPosition(pRenderSnapshot->mBowser.mPosition);
        controller.SetVelocity(pRenderSnapshot->mBowser.mVelocity);
        controller.SetPoseAccumulator(*pPoseAccumulator);

        static unsigned int HeadJointID = nlStringHash("bip01 head");

        nlMatrix4& headMatrix = pPoseAccumulator->GetNodeMatrixByHashID(HeadJointID);
        nlVector3 direction;
        // = headMatrix.GetTranslation();
        nlVec3Set(direction, headMatrix.f.m21, headMatrix.f.m22, headMatrix.f.m23);
        // direction.f.x = headMatrix.f.m21; // row 1, column 0
        // direction.f.y = headMatrix.f.m22; // row 1, column 1
        // direction.f.z = headMatrix.f.m23; // row 1, column 2
        controller.SetDirection(direction);
    }
}

/**
 * Offset/Address/Size: 0x810 | 0x80159584 | size: 0x78
 */
void Bowser::UpdateBowserLandEmitter(EmissionController& controller)
{
    if (ReplayManager::Instance()->mRender != nullptr)
    {
        RenderSnapshot* pRenderSnapshot = ReplayManager::Instance()->mRender;
        cPoseAccumulator* pPoseAccumulator = pRenderSnapshot->mBowser.mPoseAccumulator;

        controller.SetPosition(pRenderSnapshot->mBowser.mPosition);
        controller.SetVelocity(pRenderSnapshot->mBowser.mVelocity);
        controller.SetPoseAccumulator(*pPoseAccumulator);
    }
}

/**
 * Offset/Address/Size: 0x7A4 | 0x80159518 | size: 0x6C
 */
void Bowser::FindTarget()
{
    if (g_pBall->GetOwnerFielder() != NULL && !g_pBall->GetOwnerFielder()->IsInvincible())
    {
        mpTarget = g_pBall->GetOwnerFielder(); // at 0x0EC
    }
    else
    {
        mpTarget = FindPowerupTarget((cFielder*)NULL, this);
    }
}

/**
 * Offset/Address/Size: 0x334 | 0x801590A8 | size: 0x470
 */
void Bowser::SetupBaseSFX()
{
    if (!AudioLoader::IsInited())
    {
        return;
    }

    m_pCharacterSFX->Init();
    m_pCharacterSFX->mGroup = 8;
    m_pCharacterSFX->mpPhysObj = (PhysicsObject*)mpPhysObj;
    m_pCharacterSFX->SetSFX(gpBOWSERSoundPropAccessor);
    AudioLoader::SetupBowserStadiumSoundTable(this);

    mpAnim[1]->CreateCallback(8.2f / mpAnim[1]->m_nNumKeys, 0x5A, AnimSoundCallback);
    mpAnim[1]->CreateCallback(18.3f / mpAnim[1]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[1]->CreateCallback(18.3f / mpAnim[1]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[1]->CreateCallback(25.0f / mpAnim[1]->m_nNumKeys, 0x60, AnimSoundCallback);
    mpAnim[3]->CreateCallback(3.0f / mpAnim[3]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[3]->CreateCallback(25.5f / mpAnim[3]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[2]->CreateCallback(8.0f / mpAnim[2]->m_nNumKeys, 0x5B, AnimSoundCallback);
    mpAnim[2]->CreateCallback(16.0f / mpAnim[2]->m_nNumKeys, 0x5C, AnimSoundCallback);
    mpAnim[5]->CreateCallback(13.0f / mpAnim[5]->m_nNumKeys, 0x60, AnimSoundCallback);
    mpAnim[6]->CreateCallback(3.0f / mpAnim[6]->m_nNumKeys, 0x5B, AnimSoundCallback);
    mpAnim[6]->CreateCallback(13.0f / mpAnim[6]->m_nNumKeys, 0x5D, AnimSoundCallback);
    mpAnim[6]->CreateCallback(22.7f / mpAnim[6]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[6]->CreateCallback(22.7f / mpAnim[6]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[6]->CreateCallback(32.0f / mpAnim[6]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[6]->CreateCallback(32.8f / mpAnim[6]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[4]->CreateCallback(15.7f / mpAnim[4]->m_nNumKeys, 0x5F, AnimSoundCallback);
    mpAnim[4]->CreateCallback(20.0f / mpAnim[4]->m_nNumKeys, 0x63, AnimSoundCallback);
    mpAnim[4]->CreateCallback(39.0f / mpAnim[4]->m_nNumKeys, 0x5F, AnimSoundCallback);
}

/**
 * Offset/Address/Size: 0x290 | 0x80159004 | size: 0xA4
 */
void Bowser::PlaySFX(Audio::eCharSFX type, PosUpdateMethod posUpdateMethod, float fDelay, bool bIs3D)
{
    Audio::SoundAttributes attrs;
    attrs.Init();
    attrs.SetSoundType(type, bIs3D);
    attrs.mf_DelayTime = fDelay;
    attrs.posUpdateMethod = posUpdateMethod;

    if (posUpdateMethod == VECTORS)
    {
        attrs.UseStationaryPosVector(mv3Position);
    }

    m_pCharacterSFX->Play(attrs);
}

/**
 * Offset/Address/Size: 0x9C | 0x80158E10 | size: 0x1F4
 */
void Bowser::CheckFootSteps()
{
    if (mAnimID != 3)
        return;

    if (!mpAnimController->TestTrigger(0.3f) && !mpAnimController->TestTrigger(0.7f))
        return;

    if (mbDoTilt)
    {
        f32 fNewTilt = mfYAxisTilt;
        GameTweaks* pTweaks = g_pGame->m_pGameTweaks;
        f32 fTiltForce = pTweaks->unk324 + nlRandomf(pTweaks->unk328 - pTweaks->unk324, &nlDefaultSeed);

        if (mAttackType != BOWSER_ATTACK_STOMP)
        {
            fNewTilt -= 0.5f * (fTiltForce * mv3Position.f.x);
        }
        else
        {
            fNewTilt -= fTiltForce * mv3Position.f.x;
        }

        f32 fMaxTilt = g_pGame->m_pGameTweaks->unk32C;
        if (fNewTilt > fMaxTilt)
        {
            fNewTilt = fMaxTilt;
        }
        else if (fNewTilt < -fMaxTilt)
        {
            fNewTilt = -fMaxTilt;
        }

        if (mAttackType == BOWSER_ATTACK_STOMP && mStompStage == 2)
        {
            if (mfYAxisTilt * fNewTilt < 0.0f)
            {
                fNewTilt = 0.0f;
                mbDoTilt = false;
            }
        }

        SetTiltParameters(fNewTilt);
    }

    NetMesh::spPositiveXNetMesh->JoltNet(0.1f);
    NetMesh::spNegativeXNetMesh->JoltNet(0.1f);
    FireCameraRumbleFilter(0.0f, 0.5f);
}

/**
 * Offset/Address/Size: 0x60 | 0x80158DD4 | size: 0x3C
 */
float Bowser::GetHeadSpin() const
{
    return (float)(unsigned short)(int)this->mpHeadTrack->m_fHeadSpin;
}

/**
 * Offset/Address/Size: 0x24 | 0x80158D98 | size: 0x3C
 */
float Bowser::GetHeadTilt() const
{
    return (float)(unsigned short)(int)this->mpHeadTrack->m_fHeadTilt;
}

/**
 * Offset/Address/Size: 0x0 | 0x80158D74 | size: 0x24
 */
void Bowser::DrawShadow(const cPoseAccumulator& poseAccumulator, const nlMatrix4& matrix)
{
    SkinAnimatedNPC::DrawShadow(mpLastModel, matrix);
}
