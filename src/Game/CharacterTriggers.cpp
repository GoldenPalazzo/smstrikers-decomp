#include "Game/CharacterTriggers.h"
#include "Game/AnimInventory.h"
#include "Game/Player.h"
#include "Game/Ball.h"
#include "Game/CharacterTemplate.h"
#include "Game/Game.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/Powerups.h"
#include "Game/ReplayManager.h"
#include "Game/Render/ElectricFence.h"
#include "Game/RumbleActions.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/Sys/debug.h"

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
};

extern cWorldSFX gCrowdSFX;
extern cWorldSFX gStadGenSFX;
} // namespace Audio

extern cTeam* g_pTeams[2] __attribute__((section(".data")));

/**
 * Offset/Address/Size: 0x5190 | 0x801A3F40 | size: 0x94
 */
void UpdateEmitterFromCharacterIdxWithoutAnimController(EmissionController& emitter, int index)
{
    if (ReplayManager::Instance()->mRender != NULL)
    {
        ReplayManager* mgr1 = ReplayManager::Instance();
        DrawableCharacter* pChar = &mgr1->mRender->mCharacters[index];
        emitter.SetPosition(pChar->mPosition);
        ReplayManager* mgr2 = ReplayManager::Instance();
        pChar = &mgr2->mRender->mCharacters[index];
        emitter.SetVelocity(pChar->mVelocity);
        ReplayManager* mgr3 = ReplayManager::Instance();
        pChar = &mgr3->mRender->mCharacters[index];
        emitter.SetPoseAccumulator(*pChar->mPoseAccumulator);
    }
}

/**
 * Offset/Address/Size: 0x50B0 | 0x801A3E60 | size: 0xE0
 */
void UpdateEmitterFromCharacterIdxWithCoordSys(EmissionController& ec, int characterIdx)
{
    if (ReplayManager::Instance()->mRender != NULL)
    {
        DrawableCharacter* pChar = &ReplayManager::Instance()->mRender->mCharacters[characterIdx];
        nlMatrix4 mRot;
        nlMakeRotationMatrixZ(mRot, 0.0000958738f * (f32)pChar->mFacingDirection);
        nlVector3 forward;
        forward.f.x = mRot.m[0][0];
        forward.f.y = mRot.m[0][1];
        forward.f.z = mRot.m[0][2];
        ec.SetDirection(forward);
        ec.SetPosition(pChar->mPosition);
        ec.SetVelocity(pChar->mVelocity);
        ec.SetPoseAccumulator(*pChar->mPoseAccumulator);
        ec.SetAnimController(pChar->GetAnimController());
    }
}

/**
 * Offset/Address/Size: 0x4FB8 | 0x801A3D68 | size: 0xF8
 */
void UpdateEmitterFromCharacter(EmissionController& ec)
{
    if (ReplayManager::Instance()->mRender != NULL)
    {
        int characterIdx = ec.m_uUserData;
        if (ReplayManager::Instance()->mRender != NULL)
        {
            ReplayManager* mgr1 = ReplayManager::Instance();
            DrawableCharacter* pChar = &mgr1->mRender->mCharacters[characterIdx];
            ec.SetPosition(pChar->mPosition);
            RenderSnapshot* render = ReplayManager::Instance()->mRender;
            ec.SetVelocity(render->mCharacters[characterIdx].mVelocity);
            ReplayManager* mgr3 = ReplayManager::Instance();
            pChar = &mgr3->mRender->mCharacters[characterIdx];
            ec.SetPoseAccumulator(*pChar->mPoseAccumulator);
            ReplayManager* mgr4 = ReplayManager::Instance();
            pChar = &mgr4->mRender->mCharacters[characterIdx];
            ec.SetAnimController(pChar->GetAnimController());
        }

        const cCharacter* pCharacter = DrawableCharacter::OnlyRenderingOneCharacter();
        if (pCharacter != NULL)
        {
            if (g_pCharacters[characterIdx] != pCharacter)
            {
                ec.Die();
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x4F34 | 0x801A3CE4 | size: 0x84
 */
void UpdateEmitterPoseFromCharacter(EmissionController& emitter)
{
    if (ReplayManager::Instance()->mRender != NULL)
    {
        int index = emitter.m_uUserData;
        ReplayManager* mgr1 = ReplayManager::Instance();
        DrawableCharacter* pChar = &mgr1->mRender->mCharacters[index];
        emitter.SetPoseAccumulator(*pChar->mPoseAccumulator);
        ReplayManager* mgr2 = ReplayManager::Instance();
        pChar = &mgr2->mRender->mCharacters[index];
        emitter.SetAnimController(pChar->GetAnimController());
    }
}

/**
 * Offset/Address/Size: 0x4ECC | 0x801A3C7C | size: 0x68
 */
void UpdateEmitterFromBall(EmissionController& emitter)
{
    if (ReplayManager::Instance()->mRender != NULL)
    {
        ReplayManager* mgr1 = ReplayManager::Instance();
        emitter.SetPosition(mgr1->mRender->mBall.mPosition);
        ReplayManager* mgr2 = ReplayManager::Instance();
        emitter.SetVelocity(mgr2->mRender->mBall.mVelocity);
    }
}

/**
 * Offset/Address/Size: 0x4D18 | 0x801A3AC8 | size: 0x1B4
 */
EmissionController* EmitGeneric(cCharacter* pCharacter, const char* baseName, const char* characterName)
{
    EffectsGroup* pGroup;

    if (characterName != NULL)
    {
        char effectName[0x100];
        nlStrNCat<char>(effectName, characterName, "_", 0x100);
        nlStrNCat<char>(effectName, effectName, baseName, 0x100);
        nlStrNCat<char>(effectName, effectName, "_", 0x100);
        nlStrNCat<char>(effectName, effectName, "grass", 0x100);

        pGroup = fxGetGroup(effectName);
        if (pGroup == NULL && characterName[0] != '\0')
        {
            char fallbackName[0x100];
            nlStrNCat<char>(fallbackName, "", "_", 0x100);
            nlStrNCat<char>(fallbackName, fallbackName, baseName, 0x100);
            nlStrNCat<char>(fallbackName, fallbackName, "_", 0x100);
            nlStrNCat<char>(fallbackName, fallbackName, "grass", 0x100);
            pGroup = fxGetGroup(fallbackName);
        }
    }
    else
    {
        pGroup = fxGetGroup(baseName);
    }

    EmissionController* pControl = EmissionManager::Create(pGroup, 0);
    const nlVector3 v3Direction = { 0.0f, 0.0f, 1.0f };
    pControl->SetVelocity(v3Direction);
    pControl->m_fGround = 0.02f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pControl->SetUpdateCallback(update);
    }
    pCharacter->AttachEffect(pControl);
    return pControl;
}

/**
 * Offset/Address/Size: 0x2B1C | 0x801A18CC | size: 0x21FC
 */
void CharacterTriggerHandler(unsigned int uParam)
{
    class AnimTriggerCallbackInfo
    {
    public:
        /* 0x0 */ unsigned long m_uEventID;
        /* 0x4 */ float m_fIntensity;
    }; // total size: 0x8

    extern cCharacter* g_pCurrentlyUpdatingCharacter;

    // TODO: 38.15% match - dispatch tree and per-trigger action blocks still need
    // exact case-level behavior, call ordering, and register allocation.
    AnimTriggerCallbackInfo* pInfo = (AnimTriggerCallbackInfo*)uParam;
    cCharacter* pCharacter = g_pCurrentlyUpdatingCharacter;
    switch (pInfo->m_uEventID)
    {
    case 0x002EF345:
    case 0x1333263B:
        break;

    case 0xC9F4F4B0:
    case 0x50DF765D:
    case 0x3741435B:
    case 0xE36392C8:
    case 0x25642360:
    case 0x35B0F74E:
        if (pCharacter->m_pCharacterSFX != NULL)
        {
            pCharacter->m_pCharacterSFX->PlayRandomWalkFootstep(pInfo->m_fIntensity, false);
        }
        break;

    case 0x0F3E9247:
    case 0xC5408AC8:
        ((cPlayer*)pCharacter)->ClearPowerupAnimState(false);
        break;

    case 0x9DE91576:
        ((cFielder*)pCharacter)->DoSpeedBoost();
        break;

    case 0x4DDC1C64:
        ((cFielder*)pCharacter)->ThrowPowerup();
        pCharacter->Play3DSFX((Audio::eCharSFX)0x2A, (PosUpdateMethod)2, 1.0f);
        break;

    case 0xB26140F5:
        g_pEventManager->CreateValidEvent(0x12, 0x38);
        break;

    case 0x0618ECF3:
    case 0x09656D24:
    case 0x8758B65A:
    case 0x8F5ED456:
    case 0x95014E78:
    case 0x9913FAA3:
    case 0x9F338B11:
    case 0xD4DEDCAF:
    {
        Audio::SoundAttributes attrs;
        attrs.Init();
        attrs.SetSoundType(0x36, true);
        attrs.UseStationaryPosVector(pCharacter->m_v3Position);
        Audio::gStadGenSFX.Play(attrs);
        break;
    }

    case 0xC28E3737:
        Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xB9, cGameSFX::SFX_STOP_FIRST);
        break;

    case 0x6D249451:
        pCharacter->EndEffect(fxGetGroup("freeze"));
        break;

    case 0xFD96314E:
        pCharacter->StopSFX((Audio::eCharSFX)0x16);
        pCharacter->StopSFX((Audio::eCharSFX)0x1D);
        break;

    case 0x1DB5C7FF:
        pCharacter->StopSFX((Audio::eCharSFX)0x16);
        break;

    case 0x0A9AD93F:
    {
        const nlVector3 normal = { 0.0f, 0.0f, 1.0f };
        CharacterElectrocutionEffect(pCharacter, pCharacter->m_v3Position, normal);
        break;
    }

    case 0xEE2E062C:
        KillWindups(pCharacter);
        break;

    case 0x00266A23:
    case 0x73819990:
        EmitStar((cFielder*)pCharacter);
        break;

    case 0x77935C1C:
    case 0x97831FA1:
        EmitMushroom((cFielder*)pCharacter);
        break;

    case 0xD4C678A2:
    case 0xA1638ABB:
        EmitFreeze((cPlayer*)pCharacter);
        break;

    case 0x0E4E0F3F:
    case 0x64D870A7:
    case 0xAC6452C8:
        EmitDaze((cPlayer*)pCharacter);
        break;

    case 0x1DF278FA:
    case 0x5251A784:
        EmitUnFreeze((cPlayer*)pCharacter);
        break;

    case 0xB631C31A:
        EmitBallPass((cPlayer*)pCharacter);
        break;

    case 0x6580E428:
    case 0x7D452499:
    case 0x8F6B5826:
    case 0x18F99186:
    case 0x71FED95E:
        BeginRumbleAction(RUMBLE_SMALL_CONTACT, ((cPlayer*)pCharacter)->GetGlobalPad());
        break;

    case 0x0C3A8B39:
        BeginRumbleAction(RUMBLE_SOLID_CONTACT, ((cPlayer*)pCharacter)->GetGlobalPad());
        pCharacter->Play3DSFX((Audio::eCharSFX)0x36, (PosUpdateMethod)2, 1.0f);
        break;

    case 0x19076C94:
    case 0xA89AC233:
    case 0x884CBC6E:
        pCharacter->Play3DSFX((Audio::eCharSFX)0x36, (PosUpdateMethod)2, 1.0f);
        pCharacter->PlayRandomCharDialogue(4, (PosUpdateMethod)2, 100.0f, -1.0f);
        break;

    case 0x7BEE7EA1:
    case 0x8758B729:
    case 0xC7114630:
    case 0x05120C87:
    case 0x12D0B9BF:
    case 0x31DDC064:
    case 0x42E9F9CF:
    case 0x479F48A7:
    case 0xA9BF9E5A:
    case 0xF2DA216B:
    case 0xFCE1230C:
        pCharacter->PlayRandomCharDialogue(6, (PosUpdateMethod)2, 100.0f, -1.0f);
        break;

    case 0x21001B24:
    case 0x2EF4FA11:
    case 0x4C59A919:
    case 0xACDB2215:
    case 0xB8684601:
    case 0xC19CB638:
    case 0xD900F524:
    case 0x93E76D8E:
    case 0xEF7B7383:
        pCharacter->Play3DSFX((Audio::eCharSFX)0x36, (PosUpdateMethod)2, 1.0f);
        break;

    case 0x5F5115CE:
        EmitSolidRumble((cPlayer*)pCharacter);
        break;

    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x2A64 | 0x801A1814 | size: 0xB8
 */
void GetAnimTriggerInfo(cCharacter* pCharacter, int animIndex, bool (*callback)(float, float, unsigned long, float, void*), void* pData)
{
    cSAnim* pAnim = pCharacter->m_pAnimInventory->GetAnim(animIndex);
    cSAnimCallback* cb = pAnim->m_pCallbackList;

    while (cb != NULL)
    {
        cSAnim* pTriggerAnim = (cSAnim*)cb->m_nParam1;
        float numKeys = (float)pAnim->m_nNumKeys;
        if (!callback(cb->m_fTime, numKeys / 60.0f, pTriggerAnim->m_uHashID, 30.0f, pData))
        {
            break;
        }
        cb = cb->next;
    }
}

/**
 * Offset/Address/Size: 0x2A18 | 0x801A17C8 | size: 0x4C
 */
float GetCurrentAnimTriggerTime(cCharacter* pCharacter, unsigned long uTriggerID, unsigned int uInstanceNumber)
{
    cSAnimCallback* cb = pCharacter->m_pCurrentAnimController->m_pSAnim->m_pCallbackList;
    unsigned int count = 0;

    while (cb != NULL)
    {
        cSAnim* pCallbackAnim = (cSAnim*)cb->m_nParam1;
        if (uTriggerID == pCallbackAnim->m_uHashID)
        {
            count++;
            if (count - 1 == uInstanceNumber)
            {
                return cb->m_fTime;
            }
        }
        cb = cb->next;
    }
    return -1.0f;
}

/**
 * Offset/Address/Size: 0x281C | 0x801A15CC | size: 0x1FC
 */
void EmitBallImpact(cPlayer* pPlayer, bool bSilent)
{
    if (g_pBall->mbIsPerfectShot)
    {
        Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xB9, cGameSFX::SFX_STOP_FIRST);
    }
    Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xBA, cGameSFX::SFX_STOP_FIRST);

    EmissionController* pController = EmissionManager::Create(fxGetGroup("ball_hit_impact"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pPlayer->AttachEffect(pController);
    pController->SetPosition(g_pBall->m_v3Position);
    pController->SetVelocity(g_pBall->m_v3Velocity);

    eClassTypes classType = pPlayer->m_eClassType;
    s32 sfxId = 0xB7;
    if (classType == FIELDER)
    {
        switch (pPlayer->m_eAnimID)
        {
        case 0x38:
        case 0x3D:
        case 0x3E:
        case 0x3F:
            sfxId = 0xB2;
            break;
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x21:
        case 0x22:
            sfxId = 0xC3;
            break;
        case 0x1A:
        case 0x1B:
        case 0x1C:
            sfxId = 0xC3;
            break;
        }
    }

    if (classType == FIELDER && !bSilent)
    {
        Audio::SoundAttributes attrs;
        attrs.Init();
        attrs.SetSoundType(sfxId, true);
        attrs.UseStationaryPosVector(pPlayer->m_v3Position);
        Audio::gStadGenSFX.Play(attrs);
    }

    BeginRumbleAction(RUMBLE_SMALL_CONTACT, pPlayer->GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x2658 | 0x801A1408 | size: 0x1C4
 */
void EmitBallPass(cPlayer* pPlayer)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup("ball_pass_glow"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pPlayer->AttachEffect(pController);
    pController->SetPosition(g_pBall->m_v3Position);

    Audio::SoundAttributes attrsTrue;
    if (g_pBall->mbIsPerfectShot)
    {
        attrsTrue.Init();
        attrsTrue.SetSoundType(0xB8, true);
        attrsTrue.UseStationaryPosVector(pPlayer->m_v3Position);
        Audio::gStadGenSFX.Play(attrsTrue);

        attrsTrue.Init();
        attrsTrue.SetSoundType(0xB9, true);
        attrsTrue.UsePhysObj(g_pBall->m_pPhysicsBall);
        Audio::gStadGenSFX.Play(attrsTrue);
    }
    else
    {
        Audio::SoundAttributes attrs;
        attrs.Init();
        attrs.SetSoundType(0xB6, true);
        attrs.UseStationaryPosVector(pPlayer->m_v3Position);
        Audio::gStadGenSFX.Play(attrs);
    }

    if (!g_pBall->mbHyperSTS)
    {
        pPlayer->PlayRandomCharDialogue(6, (PosUpdateMethod)2, 100.0f, -1.0f);
    }
}

/**
 * Offset/Address/Size: 0x1B10 | 0x801A08C0 | size: 0xB48
 */
void EmitBallShot(cPlayer*, eBallShotEffectType, cPlayer*, bool)
{
}

/**
 * Offset/Address/Size: 0x1A44 | 0x801A07F4 | size: 0xCC
 */
void KillBallShot(const char* name, bool kill)
{
    Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xB9, cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xBA, cGameSFX::SFX_STOP_FIRST);

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            cFielder* pFielder = g_pTeams[i]->GetFielder(j);
            if (pFielder)
            {
                if (kill)
                {
                    pFielder->KillEffect(fxGetGroup(name));
                }
                else
                {
                    pFielder->EndEffect(fxGetGroup(name));
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1A18 | 0x801A07C8 | size: 0x2C
 */
void EmitSolidRumble(cPlayer* player)
{
    BeginRumbleAction(RUMBLE_SHOT_CONTACT, player->GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x18D0 | 0x801A0680 | size: 0x148
 */
void EmitElectrocutionExplosion(cCharacter* pCharacter)
{
    if (g_pGame->mbCaptainShotToScoreOn)
    {
        return;
    }

    Function<EmissionController&> update2;
    EmissionController* pController = EmissionManager::Create(fxGetGroup("electrocution_explosion"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.92f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pCharacter->AttachEffect(pController);
    pController->m_uUserData = GetCharacterIndex(pCharacter);
    update2.mTag = FREE_FUNCTION;
    update2.mFreeFunction = UpdateEmitterFromCharacter;
    pController->SetUpdateCallback(update2);
}

/**
 * Offset/Address/Size: 0x1738 | 0x801A04E8 | size: 0x198
 */
void EmitDaze(cPlayer* pCharacter)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup("dazed"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;

    {
        Function<EmissionController&> update2;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }
        pCharacter->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(pCharacter);
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = UpdateEmitterFromCharacter;
        pController->SetUpdateCallback(update2);
    }

    pCharacter->m_pCharacterSFX->StopPlayingAllRandomCharDialogue();

    if (pCharacter->m_eClassType == GOALIE)
    {
        Audio::SoundAttributes attrs;
        attrs.Init();
        attrs.SetSoundType(0x36, true);
        attrs.UseStationaryPosVector(pCharacter->m_v3Position);
        attrs.m_unk_0x7B = true;
        pCharacter->PlaySFX(attrs);
    }
    else
    {
        pCharacter->Play3DSFX((Audio::eCharSFX)0x36, (PosUpdateMethod)2, 1.0f);
    }
}

/**
 * Offset/Address/Size: 0x16FC | 0x801A04AC | size: 0x3C
 */
void KillDaze(cPlayer* player)
{
    player->KillEffect(fxGetGroup("dazed"));
}

/**
 * Offset/Address/Size: 0x1598 | 0x801A0348 | size: 0x164
 */
void EmitFreeze(cPlayer* pCharacter)
{
    pCharacter->Play3DSFX((Audio::eCharSFX)0x1f, (PosUpdateMethod)2, 1.0f);
    EmissionController* pController = EmissionManager::Create(fxGetGroup("freeze"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update2;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }
        pCharacter->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(pCharacter);
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = UpdateEmitterFromCharacter;
        pController->SetUpdateCallback(update2);
    }
    BeginRumbleAction(RUMBLE_SHOT_CONTACT, pCharacter->GetGlobalPad());
    pCharacter->m_pEffectsTexturing = fxGetTexturing(eFXTex_Freeze);
}

/**
 * Offset/Address/Size: 0x1420 | 0x801A01D0 | size: 0x178
 */
void EmitUnFreeze(cPlayer* pCharacter)
{
    pCharacter->Play3DSFX((Audio::eCharSFX)0x20, (PosUpdateMethod)2, 1.0f);
    pCharacter->KillEffect(fxGetGroup("freeze"));
    pCharacter->m_pEffectsTexturing = NULL;
    EmissionController* pController = EmissionManager::Create(fxGetGroup("unfreeze_explosion"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update2;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }
        pCharacter->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(pCharacter);
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = UpdateEmitterFromCharacter;
        pController->SetUpdateCallback(update2);
    }
    BeginRumbleAction((eRumbleActionPreset)1, pCharacter->GetGlobalPad());
}

static s32 sNumUpdatesBetweenElectrocutionToggles;

/**
 * Offset/Address/Size: 0x1400 | 0x801A01B0 | size: 0x20
 */
void ElectrocutionFinishedCallback(EmissionController& ec)
{
    g_pCharacters[ec.m_uUserData]->m_pEffectsTexturing = NULL;
}

/**
 * Offset/Address/Size: 0x1290 | 0x801A0040 | size: 0x170
 */
void ElectrocutionUpdateCallback(EmissionController& ec)
{
    cCharacter* pCharacter = g_pCharacters[ec.m_uUserData];
    s32 charIndex = GetCharacterIndex(pCharacter);

    if (ReplayManager::Instance()->mRender != NULL)
    {
        ReplayManager* mgr1 = ReplayManager::Instance();
        DrawableCharacter* pChar = &mgr1->mRender->mCharacters[charIndex];
        ec.SetPosition(pChar->mPosition);
        ReplayManager* mgr2 = ReplayManager::Instance();
        ec.SetVelocity(mgr2->mRender->mCharacters[charIndex].mVelocity);
        ReplayManager* mgr3 = ReplayManager::Instance();
        pChar = &mgr3->mRender->mCharacters[charIndex];
        ec.SetPoseAccumulator(*pChar->mPoseAccumulator);
        ReplayManager* mgr4 = ReplayManager::Instance();
        pChar = &mgr4->mRender->mCharacters[charIndex];
        ec.SetAnimController(pChar->GetAnimController());
    }

    static s32 numUpdatesUntilNextToggle = 0;
    static bool isEffectOn = false;

    if (numUpdatesUntilNextToggle == 0)
    {
        if (isEffectOn)
        {
            pCharacter->SetElectrocutionTextureEnabled(false);
        }
        else
        {
            pCharacter->SetElectrocutionTextureEnabled(true);
        }
        isEffectOn = !isEffectOn;
        numUpdatesUntilNextToggle = nlRandom(sNumUpdatesBetweenElectrocutionToggles, &nlDefaultSeed);
    }
    else
    {
        numUpdatesUntilNextToggle--;
    }
}

/**
 * Offset/Address/Size: 0x1060 | 0x8019FE10 | size: 0x230
 * TODO: 99.36% match - stack slot assignment differs for update callback temporaries:
 * first SetUpdateCallback uses sp+0x10 instead of sp+0x08; later update2 uses sp+0x08
 * instead of sp+0x10.
 */
void CharacterElectrocutionEffect(cCharacter* pCharacter, const nlVector3& v3Position, const nlVector3& v3Normal)
{
    if (g_pGame->mbCaptainShotToScoreOn)
    {
        return;
    }

    if (!EmissionManager::IsPlaying(GetCharacterIndex(pCharacter), fxGetGroup("electrocution")))
    {
        pCharacter->m_pCharacterSFX->Stop((Audio::eCharSFX)0x46, cGameSFX::SFX_STOP_FIRST);
        pCharacter->Play3DSFX((Audio::eCharSFX)0x46, (PosUpdateMethod)2, 1.0f);
        pCharacter->PlayRandomCharDialogue(4, (PosUpdateMethod)2, 1.0f, -1.0f);
    }

    EmissionController* pController = EmissionManager::Create(fxGetGroup("electrocution_aura"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> finished;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }

        pCharacter->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(pCharacter);
        finished.mTag = FREE_FUNCTION;
        finished.mFreeFunction = ElectrocutionFinishedCallback;
        pController->SetFinishedCallback(finished);
    }
    {
        Function<EmissionController&> update2;
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = ElectrocutionUpdateCallback;
        pController->SetUpdateCallback(update2);
    }
    EmitElectricFenceCharacterEffect(v3Position, v3Normal, GetCharacterIndex(pCharacter));
}

/**
 * Offset/Address/Size: 0xFF4 | 0x8019FDA4 | size: 0x6C
 */
void EmitBallWallHit(const char* name)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup(name), 0);
    static const nlVector3 velocity = { 0.0f, 0.0f, 0.0f };
    nlVector3 vel = velocity;
    pController->SetVelocity(vel);
    pController->SetPosition(g_pBall->m_v3Position);
}

/**
 * Offset/Address/Size: 0xE7C | 0x8019FC2C | size: 0x178
 */
void EmitGoalieCatch(cPlayer* pPlayer, const char* name, bool bRumble)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup(name), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pPlayer->AttachEffect(pController);
    pController->SetPosition(g_pBall->m_v3Position);

    if (bRumble)
    {
        for (int i = 0; i < 2; i++)
        {
            cTeam* pTeam = g_pTeams[i];
            if (pTeam != NULL)
            {
                for (int j = 0; j < 4; j++)
                {
                    cFielder* pFielder = pTeam->GetFielder(j);
                    if (pFielder != NULL)
                    {
                        BeginRumbleAction(RUMBLE_SHOOT_TO_SCORE, pFielder->GetGlobalPad());
                    }
                }
            }
        }
    }

    Event* pEvent = g_pEventManager->CreateValidEvent(0x12, 0x38);
    GoalieSaveData* pSaveData = new ((u8*)pEvent + 0x10) GoalieSaveData();
    pSaveData->pGoalie = pPlayer;
}

/**
 * Offset/Address/Size: 0xD2C | 0x8019FADC | size: 0x150
 */
void EmitShootToScoreHyperStrike(cFielder* pFielder)
{
    Function1<void, EmissionController&> update2;
    EmissionController* pController = EmissionManager::Create(fxGetGroup("shoot_to_score_hyper"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.92f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pFielder->AttachEffect(pController);
    pController->SetPosition(g_pBall->m_v3Position);
    update2.mTag = FREE_FUNCTION;
    update2.mFreeFunction = UpdateEmitterFromBall;
    pController->SetUpdateCallback(update2);
    if (update2.mTag == FUNCTOR)
    {
        delete update2.mFunctor;
    }
    update2.mTag = EMPTY;
    BeginRumbleAction(RUMBLE_SHOOT_TO_SCORE_HYPER, pFielder->GetGlobalPad());
}

/**
 * Offset/Address/Size: 0xC04 | 0x8019F9B4 | size: 0x128
 */
void EmitWindupAtBall(cCharacter* pCharacter, const char* name)
{
    Function<EmissionController&> update2;
    EmissionController* pController = EmissionManager::Create(fxGetGroup(name), 0);
    static const nlVector3 velocity = { 0.0f, 0.0f, 0.0f };
    nlVector3 vel = velocity;
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pCharacter->AttachEffect(pController);
    update2.mTag = FREE_FUNCTION;
    update2.mFreeFunction = UpdateEmitterFromBall;
    pController->SetUpdateCallback(update2);
}

/**
 * Offset/Address/Size: 0xB00 | 0x8019F8B0 | size: 0x104
 */
void EmitWindupAtCharacter(cCharacter* pCharacter, const char* name)
{
    Audio::gCrowdSFX.Stop((Audio::eWorldSFX)0x9F, cGameSFX::SFX_STOP_FIRST);
    pCharacter->Play3DSFX((Audio::eCharSFX)0x16, (PosUpdateMethod)1, 1.0f);
    EmissionController* pController = EmissionManager::Create(fxGetGroup(name), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 0.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pCharacter->AttachEffect(pController);
}

/**
 * Offset/Address/Size: 0xA8C | 0x8019F83C | size: 0x74
 */
void KillWindups(cCharacter* pCharacter)
{
    KillWindup(pCharacter, "ball_shot_windup", true);
    KillWindup(pCharacter, "ball_pass_windup", true);
    KillWindup(pCharacter, "ball_sts_windup", true);
    KillWindup(pCharacter, "shoot_to_score_windup", true);
}

/**
 * Offset/Address/Size: 0x87C | 0x8019F62C | size: 0x210
 */
void KillWindup(cCharacter* pCharacter, const char* name, bool bKill)
{
    if (nlStrCmp(name, "shoot_to_score_windup") == 0 && g_pBall->mbHyperSTS && pCharacter->IsPlayingEffect(fxGetGroup(name)))
    {
        pCharacter->StopSFX(Audio::CHARSFX_SHOT_WINDUP);
        pCharacter->StopSFX(Audio::CHARSFX_EFFORTS_HEAD_SHAKE);
    }
    else if (nlStrCmp(name, "ball_pass_windup") == 0 && pCharacter->IsPlayingEffect(fxGetGroup(name)) && bKill)
    {
        pCharacter->StopSFX(Audio::CHARSFX_SHOT_WINDUP);
        pCharacter->StopSFX(Audio::CHARSFX_EFFORTS_HEAD_SHAKE);
    }
    else if (nlStrCmp(name, "ball_sts_windup") == 0 && pCharacter->IsPlayingEffect(fxGetGroup(name)))
    {
        Audio::gCrowdSFX.Stop((Audio::eWorldSFX)0x9e, cGameSFX::SFX_STOP_FIRST);
        pCharacter->StopSFX(Audio::CHARSFX_SUPER_KICK_WINDUP);
        pCharacter->StopSFX(Audio::CHARSFX_SHOT_WINDUP);
        pCharacter->StopSFX(Audio::CHARSFX_EFFORTS_HEAD_SHAKE);
        pCharacter->StopSFX(Audio::CHARSFX_SHOT_RELEASE);
        pCharacter->StopSFX(Audio::CHARSFX_SWISH_DOWN_02);
        pCharacter->StopSFX(Audio::CHARSFX_SWISH_DOWN_03);

        if (((cFielder*)pCharacter)->meS2SResult == S2S_SUPER_SHOT)
        {
            pCharacter->StopSFX(Audio::CHARSFX_GEN_STOS_JUMP);
        }

        if (IsCaptain(pCharacter->m_eCharacterClass))
        {
            pCharacter->StopSFX(Audio::CHARSFX_EFFORTS_KICK_SUPER);
            pCharacter->StopSFX(Audio::CHARSFX_EFFORTS_SHOT_WINDUP);
        }
    }
    else if (nlStrCmp(name, "ball_shot_windup") == 0)
    {
        pCharacter->StopSFX(Audio::CHARSFX_SHOT_WINDUP);
        pCharacter->StopSFX(Audio::CHARSFX_EFFORTS_HEAD_SHAKE);
    }

    pCharacter->KillEffect(fxGetGroup(name));
}

/**
 * Offset/Address/Size: 0x838 | 0x8019F5E8 | size: 0x44
 */
void EmitTurbo(cPlayer* player, const char* unused)
{
    player->InitBlur(12);
    BeginRumbleAction((eRumbleActionPreset)0, player->GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x6F0 | 0x8019F4A0 | size: 0x148
 */
void EmitDust(cPlayer* player, const char* name)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup(name), 0);
    const nlVector3 velocity = { 0.0f, 0.0f, 0.0f };
    pController->SetVelocity(velocity);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update2;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }
        player->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(player);
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = UpdateEmitterFromCharacter;
        pController->SetUpdateCallback(update2);
    }
    BeginRumbleAction((eRumbleActionPreset)0, player->GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x570 | 0x8019F320 | size: 0x180
 */
void EmitMushroom(cFielder* pFielder)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup("mushroom"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 0.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update2;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }
        pFielder->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(pFielder);
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = UpdateEmitterFromCharacter;
        pController->SetUpdateCallback(update2);
    }
    PowerupBase::PlayPowerupSound(POWER_UP_MUSHROOM, PowerupBase::PWRUP_SOUND_ACTIVATE, pFielder->m_pPhysicsCharacter, 1.0f);
    pFielder->StopSFX(Audio::CHARSFX_PWRUP_MUSH_IN_EFFECT);
    pFielder->Play3DSFX(Audio::CHARSFX_PWRUP_MUSH_IN_EFFECT, (PosUpdateMethod)1, 1.0f);
    tDebugPrintManager::Print(DC_SOUND, "***EmitMushroom()***\n");
}

/**
 * Offset/Address/Size: 0x510 | 0x8019F2C0 | size: 0x60
 */
void KillMushroom(cFielder* pFielder)
{
    pFielder->StopSFX(Audio::CHARSFX_PWRUP_MUSH_IN_EFFECT);
    PowerupBase::PlayPowerupSound(POWER_UP_MUSHROOM, PowerupBase::PWRUP_SOUND_END, pFielder->m_pPhysicsCharacter, 1.0f);
    pFielder->EndBlur();
    tDebugPrintManager::Print(DC_SOUND, "***KillMushroom()***\n");
}

/**
 * Offset/Address/Size: 0x388 | 0x8019F138 | size: 0x188
 */
void EmitStar(cFielder* pFielder)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup("star"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 0.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update2;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }
        pFielder->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(pFielder);
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = UpdateEmitterFromCharacter;
        pController->SetUpdateCallback(update2);
    }
    PowerupBase::PlayPowerupSound(POWER_UP_STAR, PowerupBase::PWRUP_SOUND_ACTIVATE, pFielder->m_pPhysicsCharacter, 1.0f);
    pFielder->StopSFX(Audio::CHARSFX_PWRUP_STAR_IN_EFFECT);
    pFielder->Play3DSFX(Audio::CHARSFX_PWRUP_STAR_IN_EFFECT, (PosUpdateMethod)1, 1.0f);
    pFielder->m_pEffectsTexturing = fxGetTexturing(eFXTex_Star);
    tDebugPrintManager::Print(DC_SOUND, "***EmitStar()***\n");
}

/**
 * Offset/Address/Size: 0x320 | 0x8019F0D0 | size: 0x68
 */
void KillStar(cFielder* pFielder)
{
    pFielder->StopSFX(Audio::CHARSFX_PWRUP_STAR_IN_EFFECT);
    pFielder->KillEffect(fxGetGroup("star"));
    pFielder->EndBlur();
    pFielder->m_pEffectsTexturing = NULL;
    tDebugPrintManager::Print(DC_SOUND, "***KillStar()***\n");
}

/**
 * Offset/Address/Size: 0x2C8 | 0x8019F078 | size: 0x58
 */
void EmitChainBite(cFielder* pFielder)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup("chain_bite"), 0);
    pController->SetPosition(pFielder->m_v3Position);
    BeginRumbleAction(RUMBLE_SHOT_CONTACT, pFielder->GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x17C | 0x8019EF2C | size: 0x14C
 */
void EmitTackleImpact(cPlayer* pCharacter)
{
    EmissionController* pController = EmissionManager::Create(fxGetGroup("tackle_impact"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 1.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.02f;
    {
        Function<EmissionController&> update2;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterPoseFromCharacter;
            pController->SetUpdateCallback(update);
        }
        pCharacter->AttachEffect(pController);
        pController->m_uUserData = GetCharacterIndex(pCharacter);
        update2.mTag = FREE_FUNCTION;
        update2.mFreeFunction = UpdateEmitterFromCharacter;
        pController->SetUpdateCallback(update2);
    }
    BeginRumbleAction(RUMBLE_SOLID_CONTACT, pCharacter->GetGlobalPad());
}

/**
 * Offset/Address/Size: 0x44 | 0x8019EDF4 | size: 0x138
 */
void EmitSlideTackleTrail(cCharacter* pCharacter)
{
    Function<EmissionController&> update2;
    EmissionController* pController = EmissionManager::Create(fxGetGroup("SLIDE_TRAIL"), 0);
    const nlVector3 vel = { 0.0f, 0.0f, 0.0f };
    pController->SetVelocity(vel);
    pController->m_fGround = 0.0f;
    {
        Function<EmissionController&> update;
        update.mTag = FREE_FUNCTION;
        update.mFreeFunction = UpdateEmitterPoseFromCharacter;
        pController->SetUpdateCallback(update);
    }
    pCharacter->AttachEffect(pController);
    pController->m_uUserData = GetCharacterIndex(pCharacter);
    update2.mTag = FREE_FUNCTION;
    update2.mFreeFunction = UpdateEmitterFromCharacter;
    pController->SetUpdateCallback(update2);
}

/**
 * Offset/Address/Size: 0x0 | 0x8019EDB0 | size: 0x44
 */
void KillSlideTackleTrail(cCharacter* pCharacter)
{
    const EffectsGroup* pGroup = fxGetGroup("SLIDE_TRAIL");
    pCharacter->EndEffect(pGroup);
}
