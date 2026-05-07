#include "Game/AI/Powerups.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/AiUtil.h"
#include "Game/Render/Bowser.h"
#include "Game/Render/ChainChomp.h"
#include "Game/Render/NPCManager.h"
#include "Game/BasicStadium.h"
#include "Game/CharacterTriggers.h"
#include "Game/GameInfo.h"
#include "Game/Physics/PhysicsSphere.h"
#include "Game/Physics/PhysicsShell.h"
#include "Game/Physics/PhysicsBanana.h"
#include "Game/Physics/PhysicsAIBall.h"
#include "Game/Physics/PhysicsCharacter.h"
#include "Game/Ball.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Effects/EffectsGroup.h"
#include "Game/Game.h"
#include "Game/WorldManager.h"
#include "Game/Sys/audio.h"
#include "Game/Audio/WorldAudio.h"
#include "NL/nlMath.h"
#include "NL/nlFormat.h"
#include "NL/nlSlotPool.h"
#include "NL/nlDebug.h"

int gBobombAnticipationVoiceID = -1;
unsigned long uPowerupTexID[NUM_POWER_UPS] = { 0 };
static PowerupSounds powerupSounds[9] = {
    // POWER_UP_GREEN_SHELL
    { 0x60, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x8A, 0x5F },
    // POWER_UP_RED_SHELL
    { 0x60, 0x65, 0x66, 0x67, 0x68, 0x69, 0x8A, 0x5F },
    // POWER_UP_SPINY_SHELL
    { 0x60, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0xFFFFFFFF, 0x5F },
    // POWER_UP_FREEZE_SHELL
    { 0x60, 0x71, 0x72, 0x73, 0x74, 0x75, 0x8A, 0x5F },
    // POWER_UP_BANANA
    { 0x60, 0x80, 0xFFFFFFFF, 0x82, 0x81, 0x81, 0x8A, 0x82 },
    // POWER_UP_BOBOMB
    { 0x60, 0x89, 0x88, 0x8B, 0xFFFFFFFF, 0xFFFFFFFF, 0x8A, 0x8A },
    // POWER_UP_CHAIN_CHOMP
    { 0x60, 0x8D, 0x8E, 0x8F, 0xFFFFFFFF, 0x90, 0xFFFFFFFF, 0xFFFFFFFF },
    // POWER_UP_MUSHROOM
    { 0x60, 0x7D, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7E },
    // POWER_UP_STAR
    { 0x60, 0x84, 0xFFFFFFFF, 0x85, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x86 },
};
PowerupBase* g_pPowerups[25] = { 0 };

namespace
{
struct Pair
{
    /* 0x0 */ unsigned long hashId;
    /* 0x4 */ const PowerupBase* powerup;
}; // total size: 0x8

struct PowerupRegistry
{
    /* 0x0 */ Pair registry[25];
}; // total size: 0xC8

PowerupModelPool powerupModelPool;
PowerupRegistry powerupRegistry;

unsigned long uBOBOMB_MASTER_OBJECT;
unsigned long uBANANA_MASTER_OBJECT;
unsigned long uRED_SHELL_MASTER_OBJECT;
unsigned long uGREEN_SHELL_MASTER_OBJECT;
unsigned long uSPINY_SHELL_MASTER_OBJECT;
unsigned long uFREEZE_SHELL_MASTER_OBJECT;

const char* uGREEN_SHELL_STREAK_TEXTURE;
const char* uRED_SHELL_STREAK_TEXTURE;
const char* uSPINY_SHELL_STREAK_TEXTURE;
const char* uFREEZE_SHELL_STREAK_TEXTURE;
const char* uBANANA_STREAK_TEXTURE;
const char* uBOBOMB_STREAK_TEXTURE;

} // namespace

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

// extern Audio::cWorldSFX gPowerupSFX;

SlotPool<FreezeShell> FreezeShell::m_FreezeShellSlotPool;
SlotPool<GreenShell> GreenShell::m_GreenShellSlotPool;
SlotPool<SpinyShell> SpinyShell::m_SpinyShellSlotPool;
SlotPool<RedShell> RedShell::m_RedShellSlotPool;
SlotPool<Banana> Banana::m_BananaSlotPool;
SlotPool<Bobomb> Bobomb::m_BobombSlotPool;

void FreezeShell::operator delete(void* ptr)
{
    ((SlotPoolEntry*)ptr)->m_next = m_FreezeShellSlotPool.m_FreeList;
    m_FreezeShellSlotPool.m_FreeList = (SlotPoolEntry*)ptr;
}

void GreenShell::operator delete(void* ptr)
{
    ((SlotPoolEntry*)ptr)->m_next = m_GreenShellSlotPool.m_FreeList;
    m_GreenShellSlotPool.m_FreeList = (SlotPoolEntry*)ptr;
}

void Bobomb::operator delete(void* ptr)
{
    ((SlotPoolEntry*)ptr)->m_next = m_BobombSlotPool.m_FreeList;
    m_BobombSlotPool.m_FreeList = (SlotPoolEntry*)ptr;
}

void SpinyShell::operator delete(void* ptr)
{
    ((SlotPoolEntry*)ptr)->m_next = m_SpinyShellSlotPool.m_FreeList;
    m_SpinyShellSlotPool.m_FreeList = (SlotPoolEntry*)ptr;
}

void Banana::operator delete(void* ptr)
{
    ((SlotPoolEntry*)ptr)->m_next = m_BananaSlotPool.m_FreeList;
    m_BananaSlotPool.m_FreeList = (SlotPoolEntry*)ptr;
}

void RedShell::operator delete(void* ptr)
{
    ((SlotPoolEntry*)ptr)->m_next = m_RedShellSlotPool.m_FreeList;
    m_RedShellSlotPool.m_FreeList = (SlotPoolEntry*)ptr;
}

// /**
//  * Offset/Address/Size: 0x114 | 0x80060A74 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<int>(const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80060960 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, int>(const BasicString<char, Detail::TempStringAllocator>&, const int&)
// {
// }

/**
 * Offset/Address/Size: 0x8 | 0x80060958 | size: 0x8
 */
u32 PowerupHitPlayerEventData::GetID()
{
    return 0x1b9;
}

/**
 * Offset/Address/Size: 0x0 | 0x80060950 | size: 0x8
 */
u32 PowerupUsedEventData::GetID()
{
    return 0x1af;
}

/**
 * Offset/Address/Size: 0x0 | 0x80060948 | size: 0x8
 */
DrawableObject* DrawableObject::Clone() const
{
    return NULL;
}

/**
 * Offset/Address/Size: 0x5EBC | 0x800607A8 | size: 0x1A0
 */
cFielder* FindPowerupTarget(cFielder* pThrower, Bowser* pBowser)
{
    float fBestScore = 99999.9f;
    cFielder* pBestCandidate = NULL;
    cTeam* pTeam;
    unsigned short aDirection;
    int i;
    float fTempScore;
    cFielder* pCandidate;

    if (pThrower != NULL)
    {
        pTeam = pThrower->m_pTeam->GetOtherTeam();
        aDirection = pThrower->m_aActualFacingDirection;
    }
    else
    {
        int team = nlRandom(2, &nlDefaultSeed);
        pTeam = g_pTeams[team];
        aDirection = pBowser->maFacingDirection;
    }

    for (i = 0; i < 4; i++)
    {
        fTempScore = 99999.9f;
        pCandidate = pTeam->GetFielder(i);

        if (!pCandidate->IsFallenDown(0.0f) && !pCandidate->IsFrozen())
        {
            if (pThrower != NULL)
            {
                if (pThrower->m_pController != NULL)
                {
                    float fMag = pThrower->m_pController->GetMovementStickMagnitude();
                    if (fMag)
                    {
                        aDirection = pThrower->m_pController->GetMovementStickDirection();
                    }
                }
                fTempScore = pThrower->DoFlashLight(
                    pCandidate->m_v3Position, aDirection, g_pGame->m_pGameTweaks->fAngleWeighting, 0.0f, 9999.0f);
            }
            else
            {
                fTempScore = cPlayer::DoFlashLight(
                    pBowser->mv3Position, pCandidate->m_v3Position, aDirection, g_pGame->m_pGameTweaks->fAngleWeighting, 0.0f, 9999.0f);
            }
        }

        if (g_pBall->GetOwnerFielder() == pCandidate || g_pBall->GetPassTargetFielder() == pCandidate)
        {
            fTempScore = 0.0f;
        }

        if (fTempScore < fBestScore)
        {
            pBestCandidate = pCandidate;
            fBestScore = fTempScore;
        }
    }

    if (pBestCandidate == NULL)
    {
        pBestCandidate = pTeam->GetStriker();
    }

    return pBestCandidate;
}

/**
 * Offset/Address/Size: 0x5998 | 0x80060284 | size: 0x524
 */
void PowerupThrowPosition(int nThrowOrder, eThrowStyle eStyle, PowerupBase* pNewPowerup, PowerupBase* pFirstPowerup)
{
    f32 fPowerupOffSet = 2.0f * ((PhysicsSphere*)pFirstPowerup->m_pPhysicsObject)->GetRadius();
    fPowerupOffSet += 0.5f;

    switch (eStyle)
    {
    case THROW_HORIZONTAL_LINE:
    {
        nlVector3 v3StartPosition;
        nlVector3 v3VelocityDirection;
        nlVector3 v3PerpToVelocity;

        pNewPowerup->m_v3Velocity = pFirstPowerup->m_v3Velocity;
        pNewPowerup->m_pPhysicsObject->SetLinearVelocity(pFirstPowerup->m_v3Velocity);

        v3VelocityDirection = pFirstPowerup->m_v3Velocity;
        v3VelocityDirection.f.z = 0.0f;
        f32 invLen = nlRecipSqrt(v3VelocityDirection.f.x * v3VelocityDirection.f.x + v3VelocityDirection.f.y * v3VelocityDirection.f.y + v3VelocityDirection.f.z * v3VelocityDirection.f.z, true);
        nlVec3Set(v3VelocityDirection, invLen * v3VelocityDirection.f.x, invLen * v3VelocityDirection.f.y, invLen * v3VelocityDirection.f.z);

        if (nThrowOrder % 2 == 0)
        {
            RotateVectorZAxis(v3PerpToVelocity, v3VelocityDirection, 0x4000);
        }
        else
        {
            RotateVectorZAxis(v3PerpToVelocity, v3VelocityDirection, 0xC000);
        }

        fPowerupOffSet *= (f32)((nThrowOrder + 1) / 2);
        nlVec3Set(v3PerpToVelocity, fPowerupOffSet * v3PerpToVelocity.f.x, fPowerupOffSet * v3PerpToVelocity.f.y, fPowerupOffSet * v3PerpToVelocity.f.z);
        nlVec3Set(v3StartPosition, pFirstPowerup->m_v3Position.f.x + v3PerpToVelocity.f.x, pFirstPowerup->m_v3Position.f.y + v3PerpToVelocity.f.y, pFirstPowerup->m_v3Position.f.z + v3PerpToVelocity.f.z);

        pNewPowerup->m_v3Position = v3StartPosition;
        pNewPowerup->m_pPhysicsObject->SetPosition(pNewPowerup->m_v3Position, PhysicsObject::WORLD_COORDINATES);
        break;
    }
    case THROW_ARROW:
    {
        nlVector3 v3StartPosition;
        nlVector3 v3VelocityDirection;
        nlVector3 v3PerpToVelocity;

        pNewPowerup->m_v3Velocity = pFirstPowerup->m_v3Velocity;
        pNewPowerup->m_pPhysicsObject->SetLinearVelocity(pFirstPowerup->m_v3Velocity);

        v3VelocityDirection = pFirstPowerup->m_v3Velocity;
        v3VelocityDirection.f.z = 0.0f;
        f32 invLen = nlRecipSqrt(v3VelocityDirection.f.x * v3VelocityDirection.f.x + v3VelocityDirection.f.y * v3VelocityDirection.f.y + v3VelocityDirection.f.z * v3VelocityDirection.f.z, true);
        nlVec3Set(v3VelocityDirection, invLen * v3VelocityDirection.f.x, invLen * v3VelocityDirection.f.y, invLen * v3VelocityDirection.f.z);

        if (nThrowOrder % 2 == 0)
        {
            RotateVectorZAxis(v3PerpToVelocity, v3VelocityDirection, 0x4000);
        }
        else
        {
            RotateVectorZAxis(v3PerpToVelocity, v3VelocityDirection, 0xC000);
        }

        fPowerupOffSet *= (f32)((nThrowOrder + 1) / 2);

        nlVec3Set(v3PerpToVelocity, fPowerupOffSet * v3PerpToVelocity.f.x, fPowerupOffSet * v3PerpToVelocity.f.y, fPowerupOffSet * v3PerpToVelocity.f.z);
        nlVec3Set(v3VelocityDirection, fPowerupOffSet * v3VelocityDirection.f.x, fPowerupOffSet * v3VelocityDirection.f.y, fPowerupOffSet * v3VelocityDirection.f.z);

        RotateVectorZAxis(v3VelocityDirection, v3VelocityDirection, 0x8000);

        nlVec3Set(v3StartPosition,
            pFirstPowerup->m_v3Position.f.x + (v3PerpToVelocity.f.x + v3VelocityDirection.f.x),
            pFirstPowerup->m_v3Position.f.y + (v3PerpToVelocity.f.y + v3VelocityDirection.f.y),
            pFirstPowerup->m_v3Position.f.z + (v3PerpToVelocity.f.z + v3VelocityDirection.f.z));

        pNewPowerup->m_v3Position = v3StartPosition;
        pNewPowerup->m_pPhysicsObject->SetPosition(pNewPowerup->m_v3Position, PhysicsObject::WORLD_COORDINATES);
        break;
    }
    case THROW_SURROUND:
    {
        pNewPowerup->m_v3Position = pFirstPowerup->m_v3Position;
        pNewPowerup->m_pPhysicsObject->SetPosition(pNewPowerup->m_v3Position, PhysicsObject::WORLD_COORDINATES);

        nlVector3 v3CurrentVelocity;
        v3CurrentVelocity = pFirstPowerup->m_v3Velocity;

        s16 nFlipAngle = (s16)(((nThrowOrder + 1) / 2) * 0x3333);
        if (nThrowOrder % 2 != 0)
        {
            nFlipAngle = -nFlipAngle;
        }
        RotateVectorZAxis(v3CurrentVelocity, v3CurrentVelocity, (u16)nFlipAngle);

        pNewPowerup->m_v3Velocity = v3CurrentVelocity;
        pNewPowerup->m_pPhysicsObject->SetLinearVelocity(v3CurrentVelocity);
        break;
    }
    case THROW_SPREAD:
    {
        pNewPowerup->m_v3Position = pFirstPowerup->m_v3Position;
        pNewPowerup->m_pPhysicsObject->SetPosition(pNewPowerup->m_v3Position, PhysicsObject::WORLD_COORDINATES);

        nlVector3 v3CurrentVelocity;
        v3CurrentVelocity = pFirstPowerup->m_v3Velocity;

        s16 nFlipAngle = (s16)(((nThrowOrder + 1) / 2) * 0x1999);
        if (nThrowOrder % 2 != 0)
        {
            nFlipAngle = -nFlipAngle;
        }
        RotateVectorZAxis(v3CurrentVelocity, v3CurrentVelocity, (u16)nFlipAngle);

        pNewPowerup->m_v3Velocity = v3CurrentVelocity;
        pNewPowerup->m_pPhysicsObject->SetLinearVelocity(v3CurrentVelocity);
        break;
    }
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x4F00 | 0x8005F7EC | size: 0xA98
 */
void PowerupCreateAndThrow(cFielder*, ePowerUpType, int, Bowser*)
{
}

/**
 * Offset/Address/Size: 0x4EB4 | 0x8005F7A0 | size: 0x4C
 * TODO: 98.8% match - r4/r5 register swap on strength-reduced loop pointer and index.
 * File uses -inline deferred.
 */
PowerupBase* FindPowerUp(unsigned long hashOfDrawable)
{
    const Pair* entry = powerupRegistry.registry;
    for (int i = 0; i < 25; ++i, ++entry)
    {
        if (hashOfDrawable == entry->hashId)
        {
            return const_cast<PowerupBase*>(powerupRegistry.registry[i].powerup);
        }
    }
    return nullptr;
}

// /**
//  * Offset/Address/Size: 0x4C00 | 0x8005F4EC | size: 0x2B4
//  */
/**
 * Offset/Address/Size: 0x4C00 | 0x8005F4EC | size: 0x2B4
 * TODO: 92.6% match - compiler hoists string literal lis/addi outside loop, using one extra callee-saved register (stmw r24 vs r25)
 */
void PowerupModelPool::Initialize(int type, unsigned long objHashName)
{
    DrawableObject* obj = WorldManager::s_World->FindDrawableObject(objHashName);
    int i;

    obj->m_uObjectFlags &= ~1;
    obj->m_uObjectFlags |= 0x80;

    for (i = 0; i < 25; i++)
    {
        mObjs[type][i] = obj->Clone();

        BasicString<char, Detail::TempStringAllocator> name = Format(BasicString<char, Detail::TempStringAllocator>("powerup_generated_{0}"), mNum);

        mObjs[type][i]->m_uHashID = nlStringLowerHash(name.c_str());
        mObjs[type][i]->m_uObjectFlags &= ~1;

        WorldManager::s_World->AddDrawableObject(mObjs[type][i]->m_uHashID, mObjs[type][i]);

        mFree[type][i] = 1;
        mNum++;
    }
}

/**
 * Offset/Address/Size: 0x4B68 | 0x8005F454 | size: 0x98
 * TODO: 96.8% match - lwz r5 / li r4 instruction scheduling swap on first Initialize call only.
 *       Compiler scheduler quirk with stwu (mNum=0 store) interaction.
 */
void InitializePowerups()
{
    powerupModelPool.mNum = 0;
    powerupModelPool.Initialize(POWER_UP_FREEZE_SHELL, uFREEZE_SHELL_MASTER_OBJECT);
    powerupModelPool.Initialize(POWER_UP_SPINY_SHELL, uSPINY_SHELL_MASTER_OBJECT);
    powerupModelPool.Initialize(POWER_UP_GREEN_SHELL, uGREEN_SHELL_MASTER_OBJECT);
    powerupModelPool.Initialize(POWER_UP_RED_SHELL, uRED_SHELL_MASTER_OBJECT);
    powerupModelPool.Initialize(POWER_UP_BANANA, uBANANA_MASTER_OBJECT);
    powerupModelPool.Initialize(POWER_UP_BOBOMB, uBOBOMB_MASTER_OBJECT);
}

/**
 * Offset/Address/Size: 0x4AEC | 0x8005F3D8 | size: 0x7C
 */
void CompactPowerups()
{
    SlotPoolBase::BaseFreeBlocks(&GreenShell::m_GreenShellSlotPool, sizeof(GreenShell));
    SlotPoolBase::BaseFreeBlocks(&RedShell::m_RedShellSlotPool, sizeof(RedShell));
    SlotPoolBase::BaseFreeBlocks(&SpinyShell::m_SpinyShellSlotPool, sizeof(SpinyShell));
    SlotPoolBase::BaseFreeBlocks(&FreezeShell::m_FreezeShellSlotPool, sizeof(FreezeShell));
    SlotPoolBase::BaseFreeBlocks(&Banana::m_BananaSlotPool, sizeof(Banana));
    SlotPoolBase::BaseFreeBlocks(&Bobomb::m_BobombSlotPool, sizeof(Bobomb));
}

/**
 * Offset/Address/Size: 0x465C | 0x8005EF48 | size: 0x490
 */
PowerupBase::PowerupBase(cFielder* pTarget, ePowerUpType eType, float fRadius, ePowerupSize eSize, bool bExplode, int nIndex)
    : m_bShouldDestroy(false)
    , m_pDrawableObj(NULL)
    , m_pTarget(pTarget)
    , m_eType(eType)
{
    m_aOrientation = 0;
    m_scale = 1.0f;
    m_szStreakTexture = NULL;
    m_fBlurWidth = 0.0f;
    m_fBlurLength = 0.0f;
    m_uVoiceID = 0;
    m_pBlurHandler = NULL;
    m_nIndex = nIndex;
    meSize = eSize;
    mbExploder = bExplode;

    switch (eType)
    {
    case POWER_UP_GREEN_SHELL:
    {
        PhysicsShell* pShell = (PhysicsShell*)nlMalloc(sizeof(PhysicsShell), 8, false);
        pShell = new (pShell) PhysicsShell(fRadius);
        m_pPhysicsObject = pShell;
        PhysicsShell* pObj = (PhysicsShell*)m_pPhysicsObject;
        pObj->m_pPowerupObject = this;
        pObj->m_pTriggerCallbackFunc = (void (*)(PhysicsObject*, PhysicsObject*, nlVector3&, void*))CollisionCallback;
        pObj->m_pCallbackParam = this;
        m_szStreakTexture = uGREEN_SHELL_STREAK_TEXTURE;
        mtActiveTimer.SetSeconds(g_pGame->m_pGameTweaks->fGreenShellActiveTime);
        m_fBlurWidth = 2.0f * (fRadius / 3.0f);
        m_fBlurLength = (f32)(2.0 * fRadius);
        break;
    }
    case POWER_UP_RED_SHELL:
    {
        PhysicsShell* pShell = (PhysicsShell*)nlMalloc(sizeof(PhysicsShell), 8, false);
        pShell = new (pShell) PhysicsShell(fRadius);
        m_pPhysicsObject = pShell;
        PhysicsShell* pObj = (PhysicsShell*)m_pPhysicsObject;
        pObj->m_pPowerupObject = this;
        pObj->m_pTriggerCallbackFunc = (void (*)(PhysicsObject*, PhysicsObject*, nlVector3&, void*))CollisionCallback;
        pObj->m_pCallbackParam = this;
        m_szStreakTexture = uRED_SHELL_STREAK_TEXTURE;
        mtActiveTimer.SetSeconds(g_pGame->m_pGameTweaks->fRedShellActiveTime);
        m_fBlurWidth = 2.0f * (fRadius / 3.0f);
        m_fBlurLength = (f32)(2.0 * fRadius);
        break;
    }
    case POWER_UP_SPINY_SHELL:
    {
        PhysicsShell* pShell = (PhysicsShell*)nlMalloc(sizeof(PhysicsShell), 8, false);
        pShell = new (pShell) PhysicsShell(fRadius);
        m_pPhysicsObject = pShell;
        PhysicsShell* pObj = (PhysicsShell*)m_pPhysicsObject;
        pObj->m_pPowerupObject = this;
        pObj->m_pTriggerCallbackFunc = (void (*)(PhysicsObject*, PhysicsObject*, nlVector3&, void*))CollisionCallback;
        pObj->m_pCallbackParam = this;
        m_szStreakTexture = uSPINY_SHELL_STREAK_TEXTURE;
        mtActiveTimer.SetSeconds(g_pGame->m_pGameTweaks->fSpinyShellActiveTime);
        m_fBlurWidth = 2.0f * (fRadius / 3.0f);
        m_fBlurLength = (f32)(2.0 * fRadius);
        break;
    }
    case POWER_UP_FREEZE_SHELL:
    {
        PhysicsShell* pShell = (PhysicsShell*)nlMalloc(sizeof(PhysicsShell), 8, false);
        pShell = new (pShell) PhysicsShell(fRadius);
        m_pPhysicsObject = pShell;
        PhysicsShell* pObj = (PhysicsShell*)m_pPhysicsObject;
        pObj->m_pPowerupObject = this;
        pObj->m_pTriggerCallbackFunc = (void (*)(PhysicsObject*, PhysicsObject*, nlVector3&, void*))CollisionCallback;
        pObj->m_pCallbackParam = this;
        m_szStreakTexture = uFREEZE_SHELL_STREAK_TEXTURE;
        mtActiveTimer.SetSeconds(g_pGame->m_pGameTweaks->fFreezeShellActiveTime);
        m_fBlurWidth = 2.0f * (fRadius / 3.0f);
        m_fBlurLength = (f32)(2.0 * fRadius);
        break;
    }
    case POWER_UP_BANANA:
    {
        PhysicsBanana* pBanana = (PhysicsBanana*)nlMalloc(sizeof(PhysicsBanana), 8, false);
        pBanana = new (pBanana) PhysicsBanana(fRadius);
        m_pPhysicsObject = pBanana;
        PhysicsBanana* pObj = (PhysicsBanana*)m_pPhysicsObject;
        pObj->m_pPowerupObject = this;
        pObj->m_pTriggerCallbackFunc = (void (*)(PhysicsObject*, PhysicsObject*, nlVector3&, void*))CollisionCallback;
        pObj->m_pCallbackParam = this;
        m_szStreakTexture = uBANANA_STREAK_TEXTURE;
        mtActiveTimer.SetSeconds(g_pGame->m_pGameTweaks->fBananaActiveTime);
        m_fBlurWidth = 0.0f;
        m_fBlurLength = 0.0f;
        break;
    }
    case POWER_UP_BOBOMB:
    {
        PhysicsBanana* pBobomb = (PhysicsBanana*)nlMalloc(sizeof(PhysicsBanana), 8, false);
        pBobomb = new (pBobomb) PhysicsBanana(fRadius);
        m_pPhysicsObject = pBobomb;
        PhysicsBanana* pObj = (PhysicsBanana*)m_pPhysicsObject;
        pObj->m_pPowerupObject = this;
        pObj->m_pTriggerCallbackFunc = (void (*)(PhysicsObject*, PhysicsObject*, nlVector3&, void*))CollisionCallback;
        pObj->m_pCallbackParam = this;
        m_szStreakTexture = uBOBOMB_STREAK_TEXTURE;
        mtActiveTimer.SetSeconds(g_pGame->m_pGameTweaks->fBobombActiveTime);
        m_fBlurWidth = 0.5f * fRadius;
        m_fBlurLength = (f32)(2.0 * fRadius);
        break;
    }
    }

    m_aOrientation = nlRandom(65000, &nlDefaultSeed);

    if (eType == POWER_UP_BANANA || eType == POWER_UP_RED_SHELL)
    {
        mtNoHitTimer.SetSeconds(1.0f);
    }
    else
    {
        mtNoHitTimer.SetSeconds(0.4f);
    }

    m_v3Position.f.x = 0.0f;
    m_v3Position.f.y = 0.0f;
    m_v3Position.f.z = fRadius;
    m_v3PrevPosition = m_v3Position;
    m_v3Velocity = v3Zero;

    m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    m_pPhysicsObject->SetLinearVelocity(m_v3Velocity);
    m_pPhysicsObject->EnableCollisions();
}

/**
 * Offset/Address/Size: 0x4540 | 0x8005EE2C | size: 0x11C
 */
PowerupBase::~PowerupBase()
{
    u32 voiceID = m_uVoiceID;

    if (voiceID != 0)
    {
        if (voiceID != (u32)Audio::GetSndIDError())
        {
            Audio::gPowerupSFX.StopEmitter((SFXEmitter*)m_uVoiceID, 0);
            m_uVoiceID = 0;
        }
    }

    DrawableObject* pDrawable = m_pDrawableObj;
    int type = m_eType;

    for (int i = 0; i < 25; i++)
    {
        if (pDrawable == powerupModelPool.mObjs[type][i])
        {
            pDrawable->m_uObjectFlags &= ~1;
            powerupModelPool.mFree[type][i] = true;
            break;
        }
    }

    delete m_pPhysicsObject;
}

/**
 * Offset/Address/Size: 0x451C | 0x8005EE08 | size: 0x24
 */
float PowerupBase::GetRadius() const
{
    return ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
}

/**
 * Offset/Address/Size: 0x4404 | 0x8005ECF0 | size: 0x118
 */
void PowerupBase::Update(float dt)
{
    nlPolar polar;

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsObject->GetPosition(&m_v3Position);
    m_pPhysicsObject->GetLinearVelocity(&m_v3Velocity);

    if (m_v3Position.f.z < ((PhysicsSphere*)m_pPhysicsObject)->GetRadius())
    {
        m_v3Position.f.z = ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
        m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    }

    if (m_pBlurHandler != nullptr)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }

    mtActiveTimer.Countdown(dt, 0.0f);
    mtNoHitTimer.Countdown(dt, 0.0f);

    UpdateTransform();

    if (m_pBlurHandler != nullptr)
    {
        nlCartesianToPolar(polar, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar.r < 0.5f)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = nullptr;
        }
    }
}

/**
 * Offset/Address/Size: 0x3DFC | 0x8005E6E8 | size: 0x608
 * TODO: 95.0% match - register-only diffs (30 r-diffs), code structure 100% correct
 */
int PowerupBase::AwardPowerup(cTeam* pTeam)
{
    if (g_pGame->mIsPure)
    {
        return -1;
    }

    if (!nlSingleton<GameInfoManager>::s_pInstance->GetGameplayOptions().PowerUps)
    {
        return -1;
    }

    unsigned char bEmptySpot = false;
    for (int i = 0; i < 2; i++)
    {
        if (pTeam->GetPowerUpByIndex(i).eType == POWER_UP_NONE)
        {
            bEmptySpot = true;
        }
    }

    if (!bEmptySpot)
    {
        return -1;
    }

    int nDifference = pTeam->m_nScore - pTeam->GetOtherTeam()->m_nScore;

    int absDiff = nDifference < 0 ? -nDifference : nDifference;
    if ((u32)absDiff <= (u32)g_pGame->m_pGameTweaks->nScoreDifferenceMinimum)
    {
        nDifference = 0;
    }
    else
    {
        int nMax = g_pGame->m_pGameTweaks->nScoreDifferenceMaximum;
        if (nDifference < -nMax)
        {
            nDifference = -nMax;
        }
        else if (nDifference > nMax)
        {
            nDifference = nMax;
        }
    }

    if (nDifference < 0)
    {
        nDifference *= nDifference;
        nDifference = -nDifference;
    }
    else
    {
        nDifference *= nDifference;
    }

    int nChanceForChainChomp = g_pGame->m_pGameTweaks->nChanceForChainChomp - nDifference;

    cTeam* pOtherTeam = pTeam->GetOtherTeam();
    for (int i = 0; i < 2; i++)
    {
        if (pOtherTeam->GetPowerUpByIndex(i).eType == POWER_UP_CHAIN_CHOMP)
        {
            nChanceForChainChomp = 0;
        }
        if (pTeam->GetPowerUpByIndex(i).eType == POWER_UP_CHAIN_CHOMP)
        {
            nChanceForChainChomp = 0;
        }
    }

    if (!BasicStadium::GetCurrentStadium()->mpNPCManager->mpChainChomp->IsHidden() || BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->meBowserState != BOWSER_STATE_HIDDEN)
    {
        nChanceForChainChomp = 0;
    }

    cFielder* pCaptain = pTeam->GetCaptain();
    cFielder* pSideKick = pTeam->GetFielder(1);

    int nChanceForStar = (nChanceForChainChomp > 0 ? nChanceForChainChomp : 0) + g_pGame->m_pGameTweaks->nChanceForStar - nDifference;

    if (nDifference >= -1)
    {
        nChanceForChainChomp = 0;
        nChanceForStar = 0;
    }

    FielderTweaks* pCapTweaks = (FielderTweaks*)pCaptain->m_pTweaks;
    FielderTweaks* pSkTweaks = (FielderTweaks*)pSideKick->m_pTweaks;

    int nChanceForSpinyShell = g_pGame->m_pGameTweaks->nChanceForSpinyShell + pCapTweaks->nChanceForSpinyShell + pSkTweaks->nChanceForSpinyShell + (nChanceForStar > 0 ? nChanceForStar : 0) - nDifference;
    int nChanceForRedShell = g_pGame->m_pGameTweaks->nChanceForRedShell + pCapTweaks->nChanceForRedShell + pSkTweaks->nChanceForRedShell + (nChanceForSpinyShell > 0 ? nChanceForSpinyShell : 0) - nDifference;
    int nChanceForBanana = g_pGame->m_pGameTweaks->nChanceForBanana + pCapTweaks->nChanceForBanana + pSkTweaks->nChanceForBanana + (nChanceForRedShell > 0 ? nChanceForRedShell : 0) + nDifference;
    int nChanceForBoBomb = g_pGame->m_pGameTweaks->nChanceForBoBomb + pCapTweaks->nChanceForBoBomb + pSkTweaks->nChanceForBoBomb + (nChanceForBanana > 0 ? nChanceForBanana : 0);
    int nChanceForMushroom = g_pGame->m_pGameTweaks->nChanceForMushroom + pCapTweaks->nChanceForMushroom + pSkTweaks->nChanceForMushroom + (nChanceForBoBomb > 0 ? nChanceForBoBomb : 0) + nDifference;
    int nChanceForGreenShell = g_pGame->m_pGameTweaks->nChanceForGreenShell + pCapTweaks->nChanceForGreenShell + pSkTweaks->nChanceForGreenShell + (nChanceForMushroom > 0 ? nChanceForMushroom : 0) + nDifference;
    int nChanceForFreezeShell = g_pGame->m_pGameTweaks->nChanceForFreezeShell + pCapTweaks->nChanceForFreezeShell + pSkTweaks->nChanceForFreezeShell + (nChanceForGreenShell > 0 ? nChanceForGreenShell : 0) + nDifference;

    int nChance = nlRandom(nChanceForFreezeShell, &nlDefaultSeed);

    ePowerUpType powerUpType;
    switch (nlSingleton<GameInfoManager>::s_pInstance->GetCustomPowerups())
    {
    case CP_FREEZING:
        nChanceForGreenShell = 0;
        nChanceForMushroom = 0;
        nChanceForBoBomb = 0;
        nChanceForBanana = 0;
        nChanceForRedShell = 0;
        nChanceForSpinyShell = 0;
        nChanceForStar = 0;
        nChanceForChainChomp = 0;
        powerUpType = POWER_UP_FREEZE_SHELL;
        break;
    case CP_SHELLS:
        nChanceForMushroom = 0;
        nChanceForBoBomb = 0;
        nChanceForBanana = 0;
        nChanceForStar = 0;
        nChanceForChainChomp = 0;
        powerUpType = POWER_UP_GREEN_SHELL;
        break;
    case CP_GIANT:
        nChanceForBanana = 0;
        nChanceForMushroom = 0;
        nChanceForStar = 0;
        powerUpType = POWER_UP_GREEN_SHELL;
        break;
    case CP_ENCHANCEMENT:
        nChanceForFreezeShell = 0;
        nChanceForGreenShell = 0;
        nChanceForBoBomb = 0;
        nChanceForBanana = 0;
        nChanceForRedShell = 0;
        nChanceForSpinyShell = 0;
        nChanceForChainChomp = 0;
        powerUpType = POWER_UP_MUSHROOM;
        break;
    case CP_EXPLOSIVE:
        nChanceForFreezeShell = 0;
        nChanceForGreenShell = 0;
        nChanceForMushroom = 0;
        nChanceForBanana = 0;
        nChanceForRedShell = 0;
        nChanceForSpinyShell = 0;
        nChanceForStar = 0;
        nChanceForChainChomp = 0;
        powerUpType = POWER_UP_BOBOMB;
        break;
    default:
        powerUpType = POWER_UP_MUSHROOM;
        break;
    }

    if (nChance < nChanceForChainChomp)
    {
        powerUpType = POWER_UP_CHAIN_CHOMP;
    }
    else if (nChance < nChanceForStar)
    {
        powerUpType = POWER_UP_STAR;
    }
    else if (nChance < nChanceForSpinyShell)
    {
        powerUpType = POWER_UP_SPINY_SHELL;
    }
    else if (nChance < nChanceForRedShell)
    {
        powerUpType = POWER_UP_RED_SHELL;
    }
    else if (nChance < nChanceForBanana)
    {
        powerUpType = POWER_UP_BANANA;
    }
    else if (nChance < nChanceForBoBomb)
    {
        powerUpType = POWER_UP_BOBOMB;
    }
    else if (nChance < nChanceForMushroom)
    {
        powerUpType = POWER_UP_MUSHROOM;
    }
    else if (nChance < nChanceForGreenShell)
    {
        powerUpType = POWER_UP_GREEN_SHELL;
    }
    else if (nChance <= nChanceForFreezeShell)
    {
        powerUpType = POWER_UP_FREEZE_SHELL;
    }

    int nNumOfPowerups = 1;
    float fMultiplesBonus = ((FielderTweaks*)pCaptain->m_pTweaks)->fChanceForMultiples * 0.5f;
    float fRandom = nlRandomf(1.0f, &nlDefaultSeed);
    float fFiveChance;
    float fThreeChance;

    switch (powerUpType)
    {
    case POWER_UP_GREEN_SHELL:
    case POWER_UP_FREEZE_SHELL:
        fFiveChance = fMultiplesBonus + g_pGame->m_pGameTweaks->fShellFiveChance;
        fThreeChance = fMultiplesBonus + fFiveChance + g_pGame->m_pGameTweaks->fShellThreeChance;
        if (fThreeChance > fRandom)
        {
            nNumOfPowerups = 3;
        }
        else if (fFiveChance > fRandom)
        {
            nNumOfPowerups = 5;
        }
        break;
    case POWER_UP_RED_SHELL:
    case POWER_UP_SPINY_SHELL:
        fThreeChance = ((FielderTweaks*)pCaptain->m_pTweaks)->fChanceForMultiples + g_pGame->m_pGameTweaks->fShellFiveChance + g_pGame->m_pGameTweaks->fShellThreeChance;
        if (fThreeChance > fRandom)
        {
            nNumOfPowerups = 3;
        }
        break;
    case POWER_UP_BANANA:
        fFiveChance = fMultiplesBonus + g_pGame->m_pGameTweaks->fBananaFiveChance;
        fThreeChance = fMultiplesBonus + fFiveChance + g_pGame->m_pGameTweaks->fBananaThreeChance;
        if (fThreeChance > fRandom)
        {
            nNumOfPowerups = 3;
        }
        else
        {
            nNumOfPowerups = 5;
        }
        break;
    case POWER_UP_BOBOMB:
        fFiveChance = fMultiplesBonus + g_pGame->m_pGameTweaks->fBobombFiveChance;
        fThreeChance = fMultiplesBonus + fFiveChance + g_pGame->m_pGameTweaks->fBobombThreeChance;
        if (fThreeChance > fRandom)
        {
            nNumOfPowerups = 3;
        }
        else if (fFiveChance > fRandom)
        {
            nNumOfPowerups = 5;
        }
        break;
    default:
        break;
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->GetCustomPowerups() == CP_GIANT)
    {
        nNumOfPowerups = 1;
    }

    pTeam->SetCurrentPowerUp(powerUpType, nNumOfPowerups);
    return (int)powerUpType;
}

/**
 * Offset/Address/Size: 0x38B4 | 0x8005E1A0 | size: 0x548
 */
void PowerupBase::CollisionCallback(PhysicsObject* pObjA, PhysicsObject* pObjB, const nlVector3& v3Pos, void* pParam)
{
    PowerupBase* pObj = (PowerupBase*)pParam;
    cCharacter* pCharacter = NULL;
    cPlayer* pPlayerTarget = NULL;

    int type = pObjB->GetObjectType();
    switch (type)
    {
    case 0x0D:
    case 0x0E:
        pCharacter = ((PhysicsCharacter*)pObjB->m_parentObject)->m_pAICharacter;
        break;
    case 0x0F:
    {
        cBall* pBall = ((PhysicsAIBall*)pObjB)->m_pAIBall;
        cFielder* pFielder = (cFielder*)pBall->m_pOwner;
        if (pFielder != NULL)
        {
            pCharacter = (cCharacter*)pFielder;
            if (((cCharacter*)pFielder)->m_eClassType != FIELDER)
                break;
            if (pBall->GetOwnerFielder()->IsBallAwayFromCarrier())
            {
                Event* pEvent = g_pEventManager->CreateValidEvent(0x2D, 0x24);
                CollisionBallShellData* pData = new ((u8*)pEvent + 0x10) CollisionBallShellData();
                pData->v3CollisionVelocity = pObj->m_v3Velocity;
            }
        }
        else
        {
            bool bPerfectShot = (pBall->m_tShotTimer.m_uPackedTime != 0) && pBall->m_unk_0xA4;
            if (bPerfectShot)
                EmitBallWallHit("perfect_shot_catch");
            else if (pObj->m_pTarget != NULL)
                EmitBallWallHit("goalie_catch");
            pObj->m_pTarget = NULL;
            pBall->ClearShotInProgress();
            pBall->ClearPassTarget();
            pBall->m_unk_0xA6 = false;
            pBall->mpDamageTarget = NULL;
        }
        break;
    }
    case 0x13:
        if (pObj->meSize > ((PhysicsShell*)pObjB)->m_pPowerupObject->meSize)
        {
            ((PhysicsShell*)pObjB)->m_pPowerupObject->m_bShouldDestroy = true;
        }
        else if (pObj->meSize == ((PhysicsShell*)pObjB)->m_pPowerupObject->meSize)
        {
            if (pObj->m_eType == POWER_UP_BANANA)
            {
                pObj->m_bShouldDestroy = true;
            }
            else if (pObj->m_eType == POWER_UP_SPINY_SHELL && ((PhysicsShell*)pObjB)->m_pPowerupObject->m_eType == POWER_UP_SPINY_SHELL)
            {
                ((PhysicsShell*)pObjB)->m_pPowerupObject->m_bShouldDestroy = true;
                pObj->m_bShouldDestroy = true;
            }
            else if (pObj->m_eType == POWER_UP_SPINY_SHELL)
            {
                ((PhysicsShell*)pObjB)->m_pPowerupObject->m_bShouldDestroy = true;
            }
            else if (((PhysicsShell*)pObjB)->m_pPowerupObject->m_eType == POWER_UP_SPINY_SHELL)
            {
                pObj->m_bShouldDestroy = true;
            }
            else
            {
                ((PhysicsShell*)pObjB)->m_pPowerupObject->m_bShouldDestroy = true;
                pObj->m_bShouldDestroy = true;
            }
        }
        else
        {
            pObj->m_bShouldDestroy = true;
        }
        break;
    case 0x14:
    {
        PowerupBase* pOther = ((PhysicsBanana*)pObjB)->m_pPowerupObject;
        if (pObj->meSize > pOther->meSize)
        {
            pOther->m_bShouldDestroy = true;
        }
        else if (pObj->meSize == pOther->meSize)
        {
            pOther->m_bShouldDestroy = true;
            if (pObj->m_eType != POWER_UP_SPINY_SHELL)
                pObj->m_bShouldDestroy = true;
        }
        else
        {
            pObj->m_bShouldDestroy = true;
        }
        break;
    }
    default:
        if (pObj->m_eType == POWER_UP_BOBOMB && !((Bobomb*)pObj)->mbIsMine && ((PhysicsShell*)pObjA)->mbIsInNet)
            pObj->m_bShouldDestroy = true;
        break;
    }

    if (pCharacter == NULL)
        goto check_event;
    if (pCharacter->m_eClassType == GOALIE)
    {
        pObj->m_bShouldDestroy = true;
        goto check_event;
    }
    pPlayerTarget = (cPlayer*)pCharacter;
    if (((cFielder*)pCharacter)->mbWasHitByPowerupThisFrame)
        return;
    ((cFielder*)pCharacter)->mbWasHitByPowerupThisFrame = true;

    {
        ePowerUpType eType = pObj->m_eType;
        if (eType == POWER_UP_BANANA)
        {
            Event* pEvent = g_pEventManager->CreateValidEvent(0x2B, 0x30);
            CollisionPlayerBananaData* pData = new ((u8*)pEvent + 0x10) CollisionPlayerBananaData();
            pData->pPlayer = (cFielder*)pCharacter;
            pData->pThrower = pObj->m_pThrower;
            pData->nThrowerPadID = pObj->m_nThrowerPadID;
            pData->v3CollisionLocation = v3Pos;
            pObj->m_bShouldDestroy = true;
        }
        else if (eType != POWER_UP_BOBOMB)
        {
            if (eType != POWER_UP_FREEZE_SHELL)
            {
                Event* pEvent = g_pEventManager->CreateValidEvent(0x29, 0x40);
                CollisionPlayerShellData* pData = new ((u8*)pEvent + 0x10) CollisionPlayerShellData();
                pData->pPlayer = (cFielder*)pCharacter;
                pData->eSize = (int)pObj->meSize;
                pData->pThrower = pObj->m_pThrower;
                pData->nThrowerPadID = (u8)pObj->m_nThrowerPadID;
                if (pObj->mbExploder)
                {
                    pData->bIsExploder = true;
                }
                else
                {
                    pData->bIsExploder = false;
                }
                pData->v3CollisionLocation = v3Pos;
                pData->v3CollisionVelocity = pObj->m_v3Velocity;
            }
            else
            {
                Event* pEvent = g_pEventManager->CreateValidEvent(0x2A, 0x28);
                CollisionPlayerFreezeData* pData = new ((u8*)pEvent + 0x10) CollisionPlayerFreezeData();
                pData->pPlayer = (cFielder*)pCharacter;
                pData->eSize = (int)pObj->meSize;
                pData->pThrower = pObj->m_pThrower;
                pData->nThrowerPadID = pObj->m_nThrowerPadID;
            }
            if (pObj->meSize != POWERUPSIZE_LARGE)
            {
                pObj->m_bShouldDestroy = true;
            }
            else
            {
                nlVector3 v3NewVelocity = pObj->m_v3Velocity;
                nlPolar aSpeedOut;
                nlCartesianToPolar(aSpeedOut, v3NewVelocity.f.x, v3NewVelocity.f.y);
                v3NewVelocity.f.z = 0.5f * aSpeedOut.r;
                pObj->m_v3Velocity = v3NewVelocity;
                pObj->m_pPhysicsObject->SetLinearVelocity(v3NewVelocity);
                pObj->m_pTarget = NULL;
            }
        }
        else
        {
            pObj->m_bShouldDestroy = true;
        }
    }

    if (pObj->m_eType == POWER_UP_SPINY_SHELL)
        pObj->m_bShouldDestroy = false;
    if (((cFielder*)pCharacter)->IsInvincible())
        pObj->m_bShouldDestroy = true;

check_event:
    if (pPlayerTarget != NULL)
    {
        Event* pEvent = g_pEventManager->CreateValidEvent(0x1E, 0x24);
        PowerupHitPlayerEventData* pData = new ((u8*)pEvent + 0x10) PowerupHitPlayerEventData();
        pData->Type = pObj->m_eType;
        pData->Thrower = (cPlayer*)pObj->m_pThrower;
        pData->Target = pPlayerTarget;
    }
}

/**
 * Offset/Address/Size: 0x3374 | 0x8005DC60 | size: 0x540
 * TODO: 94.25% match - GPR r27/r28/r29 and FPR f27 register allocation
 * differ due to -inline deferred vs -inline auto compiler flag difference
 */
void PowerupBase::ThrowAt(cFielder* pThrower, Bowser*)
{
    unsigned long voiceID = m_uVoiceID;
    if (voiceID != 0)
    {
        if (voiceID != Audio::GetSndIDError())
        {
            Audio::gPowerupSFX.StopEmitter((SFXEmitter*)m_uVoiceID, 0);
            m_uVoiceID = 0;
        }
    }

    ePowerUpType type = m_eType;
    PhysicsObject* pPhysObj = m_pPhysicsObject;

    if (type >= NUM_POWER_UPS)
    {
        Audio::GetSndIDError();
    }
    else if (!Audio::IsInited())
    {
        Audio::GetSndIDError();
    }
    else
    {
        Audio::SoundAttributes attributes;
        attributes.Init();
        unsigned long soundID = powerupSounds[type].sndActivate;
        if (soundID != 0xFFFFFFFF)
        {
            attributes.SetSoundType(soundID, true);
            if (type == POWER_UP_BOBOMB)
            {
                attributes.UsePhysObj(pPhysObj);
                attributes.mf_ReturnEmitterOnPlay = true;
            }
            else
            {
                attributes.UseStationaryPosVector(pPhysObj->GetPosition());
            }
            Audio::gPowerupSFX.GetSFXVol(soundID);
            Audio::gPowerupSFX.Play(attributes);
        }
    }

    if (m_pTarget->m_pTeam == g_pTeams[1])
    {
        Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_YEAH_SMALL, 100.0f, -1.0f, 0, 0.0f);
    }
    else
    {
        Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_OH_SMALL, 100.0f, -1.0f, 0, 0.0f);
    }

    ePowerUpType type2 = m_eType;
    if (type2 == POWER_UP_CHAIN_CHOMP && !g_pGame->mbCaptainShotToScoreOn)
    {
        PhysicsObject* pPhysObj2 = m_pPhysicsObject;
        unsigned long errorCode;
        if (type2 >= NUM_POWER_UPS)
        {
            errorCode = Audio::GetSndIDError();
        }
        else if (!Audio::IsInited())
        {
            errorCode = Audio::GetSndIDError();
        }
        else
        {
            Audio::SoundAttributes attributes2;
            attributes2.Init();
            unsigned long soundID2 = powerupSounds[type2].sndInEffect;
            if (soundID2 == 0xFFFFFFFF)
            {
                errorCode = (unsigned long)-1;
            }
            else
            {
                attributes2.SetSoundType(soundID2, true);
                if (type2 == POWER_UP_BOBOMB)
                {
                    attributes2.UsePhysObj(pPhysObj2);
                    attributes2.mf_ReturnEmitterOnPlay = true;
                }
                else
                {
                    attributes2.UsePhysObj(pPhysObj2);
                    attributes2.mf_ReturnEmitterOnPlay = true;
                }
                Audio::gPowerupSFX.GetSFXVol(soundID2);
                errorCode = Audio::gPowerupSFX.Play(attributes2);
            }
        }
        m_uVoiceID = errorCode;
    }

    nlVector3 v3TargetPos = m_pTarget->m_v3Position;
    nlVector3 v3TargetVel = m_pTarget->m_v3Velocity;

    float fSpeed = 0.0f;
    switch (m_eType)
    {
    case POWER_UP_GREEN_SHELL:
        fSpeed += g_pGame->m_pGameTweaks->fGreenShellSpeed;
        break;
    case POWER_UP_RED_SHELL:
        fSpeed += g_pGame->m_pGameTweaks->fRedShellSpeed;
        break;
    case POWER_UP_SPINY_SHELL:
        fSpeed += g_pGame->m_pGameTweaks->fSpinyShellSpeed;
        break;
    case POWER_UP_FREEZE_SHELL:
        fSpeed += g_pGame->m_pGameTweaks->fFreezeShellSpeed;
        break;
    case POWER_UP_BANANA:
        if (pThrower != NULL)
        {
            s16 fFacingDelta = pThrower->GetFacingDeltaToPosition(m_pTarget->m_v3Position);
            float fMinSpeed = g_pGame->m_pGameTweaks->fBananaSpeed;
            float fMaxSpeed = fMinSpeed + pThrower->m_fActualSpeed;
            s16 absDelta = fFacingDelta;
            if (fFacingDelta < 0)
                absDelta = -fFacingDelta;
            fSpeed = InterpolateRangeClamped(fMinSpeed, fMaxSpeed, 32000.0f, 12500.0f, (float)(u16)absDelta);
        }
        else
        {
            fSpeed += g_pGame->m_pGameTweaks->fBananaSpeed;
        }
        break;
    default:
        break;
    }

    if (m_eType != POWER_UP_BANANA)
    {
        if (meSize == POWERUPSIZE_LARGE)
        {
            fSpeed -= 1.0f;
        }
    }

    float dy = v3TargetPos.f.y - m_v3Position.f.y;
    float dx = v3TargetPos.f.x - m_v3Position.f.x;
    float dz = v3TargetPos.f.z - m_v3Position.f.z;
    float invDist = nlRecipSqrt(dx * dx + dy * dy + dz * dz, true);
    float normDz = invDist * dz;
    float normDy = invDist * dy;
    float normDx = invDist * dx;

    int nNumSolutions;
    float pSolutions[2];
    CalcInterceptXY(m_v3Position, fSpeed, 0.0f, v3TargetPos, v3TargetVel, nNumSolutions, pSolutions);

    if (nNumSolutions != 0)
    {
        float t;
        if (nNumSolutions == 2)
        {
            t = (pSolutions[0] < pSolutions[1]) ? pSolutions[0] : pSolutions[1];
        }
        else
        {
            t = pSolutions[0];
        }

        nlVector3 v3ShellVelocity;
        float predictedX = v3TargetVel.f.x * t + v3TargetPos.f.x;
        float predictedY = v3TargetVel.f.y * t + v3TargetPos.f.y;
        v3ShellVelocity.f.x = (predictedX - m_v3Position.f.x) / t;
        v3ShellVelocity.f.y = (predictedY - m_v3Position.f.y) / t;
        v3ShellVelocity.f.z = 0.0f;
        m_v3Velocity = v3ShellVelocity;
        m_pPhysicsObject->SetLinearVelocity(v3ShellVelocity);
    }
    else
    {
        nlVector3 v3Velocity;
        v3Velocity.f.x = fSpeed * normDx;
        v3Velocity.f.y = fSpeed * normDy;
        v3Velocity.f.z = fSpeed * normDz;
        v3Velocity.f.z = 0.0f;
        m_v3Velocity = v3Velocity;
        m_pPhysicsObject->SetLinearVelocity(v3Velocity);
    }
}

/**
 * Offset/Address/Size: 0x28DC | 0x8005D1C8 | size: 0xA98
 */
void PowerupBase::Destroy(bool)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x27C0 | 0x8005D0AC | size: 0x11C
 */
void PowerupBase::PreThrow(cFielder* pFielder, Bowser* pBowser)
{
    nlVector3 pos;

    if (pFielder != NULL)
    {
        if (pFielder->m_nPowerupAnimID == 0x5F || pFielder->m_nPowerupAnimID == 0x61)
        {
            pos = pFielder->GetJointPosition(pFielder->m_nLeftHandJointIndex);
        }
        else
        {
            pos = pFielder->GetJointPosition(pFielder->m_nRightHandJointIndex);
        }
    }
    else
    {
        nlVector3 localPt = { 0.0f, 0.0f, 0.0f };
        GetWorldPoint(pos, localPt, pBowser->mv3Position, pBowser->maFacingDirection);
    }

    m_v3Position = pos;
    m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    m_v3Velocity = v3Zero;
    m_pPhysicsObject->SetLinearVelocity(v3Zero);
}

/**
 * Offset/Address/Size: 0x24CC | 0x8005CDB8 | size: 0x2F4
 */
void PowerupBase::UpdateTransform()
{
    nlPolar pDirectionalSpeed;
    float fSpeedNormalized;
    float fActualRadius;
    float fNormalRadius;

    nlCartesianToPolar(pDirectionalSpeed, m_v3Velocity.f.x, m_v3Velocity.f.y);
    fSpeedNormalized = NormalizeVal(pDirectionalSpeed.r, 0.0f, g_pGame->m_pGameTweaks->fGreenShellSpeed);

    {
        float z = 0.0f;
        m_aOrientation += (int)(1875.0f * fSpeedNormalized + z);
    }

    switch (m_eType)
    {
    case POWER_UP_BANANA:
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            fActualRadius = g_pGame->m_pGameTweaks->fBananaBigRadius;
            break;
        case POWERUPSIZE_MEDIUM:
            fActualRadius = g_pGame->m_pGameTweaks->fBananaMediumRadius;
            break;
        case POWERUPSIZE_SMALL:
            fActualRadius = g_pGame->m_pGameTweaks->fBananaSmallRadius;
            break;
        }
        fNormalRadius = g_pGame->m_pGameTweaks->fBananaSmallRadius;
        break;

    case POWER_UP_BOBOMB:
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            fActualRadius = g_pGame->m_pGameTweaks->fBobombBigRadius;
            break;
        case POWERUPSIZE_MEDIUM:
            fActualRadius = g_pGame->m_pGameTweaks->fBobombMediumRadius;
            break;
        case POWERUPSIZE_SMALL:
            fActualRadius = g_pGame->m_pGameTweaks->fBobombSmallRadius;
            break;
        }
        fNormalRadius = g_pGame->m_pGameTweaks->fBobombSmallRadius;
        break;

    default:
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            fActualRadius = g_pGame->m_pGameTweaks->fShellBigRadius;
            break;
        case POWERUPSIZE_MEDIUM:
            fActualRadius = g_pGame->m_pGameTweaks->fShellMediumRadius;
            break;
        case POWERUPSIZE_SMALL:
            fActualRadius = g_pGame->m_pGameTweaks->fShellSmallRadius;
            break;
        }
        fNormalRadius = g_pGame->m_pGameTweaks->fShellSmallRadius;
        break;
    }

    m_scale = fActualRadius / fNormalRadius;
    ((PhysicsSphere*)m_pPhysicsObject)->SetRadius(fActualRadius);

    if (m_eType != POWER_UP_BANANA && m_eType != POWER_UP_RED_SHELL)
    {
        if (mtNoHitTimer.m_uPackedTime != 0)
        {
            m_scale = InterpolateRangeClamped(0.33f, m_scale, 0.4f, 0.0f, mtNoHitTimer.GetSeconds());
            ((PhysicsSphere*)m_pPhysicsObject)->SetRadius(m_scale * fNormalRadius);

            if (m_pBlurHandler != NULL && m_scale <= 1.0f)
            {
                m_pBlurHandler->m_fLineWidth = m_scale * m_fBlurWidth;
            }
        }
    }
    else
    {
        float fBananaTimer = mtNoHitTimer.GetSeconds() - 0.6f;
        if (fBananaTimer > 0.0f)
        {
            m_scale = InterpolateRangeClamped(0.33f, m_scale, 0.4f, 0.0f, fBananaTimer);
            if (m_pBlurHandler != NULL && m_scale <= 1.0f)
            {
                m_pBlurHandler->m_fLineWidth = m_scale * m_fBlurWidth;
            }
        }
    }

    if (m_szStreakTexture != NULL && m_pBlurHandler == NULL)
    {
        m_pBlurHandler = BlurManager::GetNewHandler(m_szStreakTexture, m_scale * m_fBlurWidth, 15, true);
    }
}

/**
 * Offset/Address/Size: 0x2360 | 0x8005CC4C | size: 0x16C
 * TODO: 99.12% match - r5/r6 register swap in second loop (registry ptr vs j counter)
 */
void PowerupBase::Init(cFielder* pFielder, Bowser* pBowser)
{
    int i;
    DrawableObject* pObj;
    int type = m_eType;

    for (i = 0; i < 25; i++)
    {
        if (powerupModelPool.mFree[type][i])
        {
            powerupModelPool.mFree[type][i] = false;
            pObj = powerupModelPool.mObjs[type][i];
            goto found1;
        }
    }
    pObj = NULL;

found1:
    m_pDrawableObj = pObj;

    {
        DrawableObject* pD = m_pDrawableObj;
        pD->m_uObjectFlags |= 0x100;
        unsigned long hashID = m_pDrawableObj->m_uHashID;
        int j = 0;

        for (; j < 25; j++)
        {
            if (powerupRegistry.registry[j].hashId == 0)
            {
                powerupRegistry.registry[j].hashId = hashID;
                powerupRegistry.registry[j].powerup = this;
                goto found2;
            }
        }
        nlBreak();
    }

found2:
    PreThrow(pFielder, pBowser);

    m_pThrower = pFielder;

    if (pFielder != NULL)
    {
        s32 padID;
        bool bHasPad = pFielder->GetGlobalPad() != NULL;
        padID = bHasPad ? pFielder->GetGlobalPad()->m_padIndex : -1;
        m_nThrowerPadID = padID;
    }
}

// /**
//  * Offset/Address/Size: 0x22F0 | 0x8005CBDC | size: 0x70
//  */
// PhysicsShell::~PhysicsShell()
// {
// }

/**
 * Offset/Address/Size: 0x1E00 | 0x8005C6EC | size: 0x250
 * TODO: 99.84% match - jump table relocation symbol mismatch in switch dispatch (@3011 vs generated local label).
 */
unsigned long PowerupBase::PlayPowerupSound(ePowerUpType type, PowerupBase::PowerupSound powerupSnd, PhysicsObject* pPhysObj, float fVol)
{
    Audio::SoundAttributes sndAtr;
    unsigned long sndType;
    float fDefaultVol;

    if (type >= NUM_POWER_UPS)
    {
        return Audio::GetSndIDError();
    }

    if (!Audio::IsInited())
    {
        return Audio::GetSndIDError();
    }

    sndAtr.Init();

    switch (powerupSnd)
    {
    case PWRUP_SOUND_ACQUIRE:
        sndType = powerupSounds[type].sndAcquire;
        break;
    case PWRUP_SOUND_ACTIVATE:
        sndType = powerupSounds[type].sndActivate;
        break;
    case PWRUP_SOUND_IN_EFFECT:
        sndType = powerupSounds[type].sndInEffect;
        break;
    case PWRUP_SOUND_HIT:
        sndType = powerupSounds[type].sndHit;
        break;
    case PWRUP_SOUND_BOUNCE_WALL:
        sndType = powerupSounds[type].sndBounceWall;
        break;
    case PWRUP_SOUND_BOUNCE_GROUND:
        sndType = powerupSounds[type].sndBounceGround;
        break;
    case PWRUP_SOUND_EXPLODE:
        sndType = powerupSounds[type].sndExplode;
        break;
    case PWRUP_SOUND_END:
        sndType = powerupSounds[type].sndEnd;
        break;
    }

    if (sndType == 0xFFFFFFFF)
    {
        return -1;
    }

    if (powerupSnd == PWRUP_SOUND_ACQUIRE)
    {
        sndAtr.SetSoundType(sndType, false);
    }
    else
    {
        sndAtr.SetSoundType(sndType, true);

        if (type == POWER_UP_BOBOMB)
        {
            if ((powerupSnd == PWRUP_SOUND_IN_EFFECT) || (powerupSnd == PWRUP_SOUND_ACTIVATE))
            {
                sndAtr.UsePhysObj(pPhysObj);
                *(u8*)&sndAtr.mp_OwnerSFX = 1;
            }
            else
            {
                sndAtr.UseStationaryPosVector(pPhysObj->GetPosition());
            }
        }
        else
        {
            if (powerupSnd == PWRUP_SOUND_IN_EFFECT)
            {
                sndAtr.UsePhysObj(pPhysObj);
                *(u8*)&sndAtr.mp_OwnerSFX = 1;
            }
            else
            {
                sndAtr.UseStationaryPosVector(pPhysObj->GetPosition());
            }
        }
    }

    fDefaultVol = Audio::gPowerupSFX.GetSFXVol(sndType);

    if (fVol != 1.0f)
    {
        sndAtr.mf_Attenuate = fVol * fDefaultVol;
    }

    return Audio::gPowerupSFX.Play(sndAtr);
}

/**
 * Offset/Address/Size: 0x1DD0 | 0x8005C6BC | size: 0x30
 */
void PowerupBase::StopPowerupInEffectSound(SFXEmitter* pSFXEmitter)
{
    Audio::gPowerupSFX.StopEmitter(pSFXEmitter, 0);
}

/**
 * Offset/Address/Size: 0x2050 | 0x8005C93C | size: 0x2A0
 */
unsigned long PowerupBase::PlayPowerupSound(ePowerUpType type, PowerupBase::PowerupSound powerupSnd, const nlVector3& v3Pos, float fVol)
{
    PhysicsShell dummyShell(1.0f);
    dummyShell.SetPosition(v3Pos, PhysicsObject::WORLD_COORDINATES);

    if (type >= NUM_POWER_UPS)
    {
        type = (ePowerUpType)Audio::GetSndIDError();
    }
    else if (!Audio::IsInited())
    {
        type = (ePowerUpType)Audio::GetSndIDError();
    }
    else
    {
        Audio::SoundAttributes sndAtr;
        sndAtr.Init();

        unsigned long sndType;
        switch (powerupSnd)
        {
        case PWRUP_SOUND_ACQUIRE:
            sndType = powerupSounds[type].sndAcquire;
            break;
        case PWRUP_SOUND_ACTIVATE:
            sndType = powerupSounds[type].sndActivate;
            break;
        case PWRUP_SOUND_IN_EFFECT:
            sndType = powerupSounds[type].sndInEffect;
            break;
        case PWRUP_SOUND_HIT:
            sndType = powerupSounds[type].sndHit;
            break;
        case PWRUP_SOUND_BOUNCE_WALL:
            sndType = powerupSounds[type].sndBounceWall;
            break;
        case PWRUP_SOUND_BOUNCE_GROUND:
            sndType = powerupSounds[type].sndBounceGround;
            break;
        case PWRUP_SOUND_EXPLODE:
            sndType = powerupSounds[type].sndExplode;
            break;
        case PWRUP_SOUND_END:
            sndType = powerupSounds[type].sndEnd;
            break;
        }

        if (sndType == 0xFFFFFFFF)
        {
            type = (ePowerUpType)-1;
        }
        else
        {
            if (powerupSnd == PWRUP_SOUND_ACQUIRE)
            {
                sndAtr.SetSoundType(sndType, false);
            }
            else
            {
                sndAtr.SetSoundType(sndType, true);

                if (type == POWER_UP_BOBOMB)
                {
                    if ((powerupSnd == PWRUP_SOUND_IN_EFFECT) || (powerupSnd == PWRUP_SOUND_ACTIVATE))
                    {
                        sndAtr.UsePhysObj(&dummyShell);
                        sndAtr.mf_ReturnEmitterOnPlay = true;
                    }
                    else
                    {
                        sndAtr.UseStationaryPosVector(dummyShell.GetPosition());
                    }
                }
                else
                {
                    if (powerupSnd == PWRUP_SOUND_IN_EFFECT)
                    {
                        sndAtr.UsePhysObj(&dummyShell);
                        sndAtr.mf_ReturnEmitterOnPlay = true;
                    }
                    else
                    {
                        sndAtr.UseStationaryPosVector(dummyShell.GetPosition());
                    }
                }
            }

            float fDefaultVol = Audio::gPowerupSFX.GetSFXVol(sndType);

            if (100.0f != fVol)
            {
                sndAtr.mf_Volume = fVol * fDefaultVol;
            }

            type = (ePowerUpType)Audio::gPowerupSFX.Play(sndAtr);
        }
    }

    return (unsigned long)type;
}

/**
 * Offset/Address/Size: 0x1C98 | 0x8005C584 | size: 0x138
 */
GreenShell::~GreenShell()
{
}

/**
 * Offset/Address/Size: 0x1A80 | 0x8005C36C | size: 0x218
 */
void GreenShell::Update(float dt)
{
    nlPolar polar;
    nlVector2 vel;
    nlPolar polar2;
    nlVector3 cappedVel;

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsObject->GetPosition(&m_v3Position);
    m_pPhysicsObject->GetLinearVelocity(&m_v3Velocity);

    if (m_v3Position.f.z < ((PhysicsSphere*)m_pPhysicsObject)->GetRadius())
    {
        m_v3Position.f.z = ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
        m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    }

    if (m_pBlurHandler != NULL)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }

    mtActiveTimer.Countdown(dt, 0.0f);
    mtNoHitTimer.Countdown(dt, 0.0f);

    UpdateTransform();

    if (m_pBlurHandler != NULL)
    {
        nlCartesianToPolar(polar, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar.r < 0.5f)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = NULL;
        }
    }

    if (mtNoHitTimer.m_uPackedTime == 0)
    {
        nlCartesianToPolar(polar2, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar2.r < 3.0f)
        {
            m_bShouldDestroy = true;
        }
        else if (polar2.r > 20.0f)
        {
            vel.as_u32[0] = m_v3Velocity.as_u32[0];
            vel.as_u32[1] = m_v3Velocity.as_u32[1];
            f32 velX = vel.f.x;
            f32 velY = vel.f.y;
            f32 sqX = velX * velX;
            f32 sqY = velY * velY;
            f32 recipLen = nlRecipSqrt(sqX + sqY, true);
            vel.f.x = recipLen * velX;
            vel.f.y = recipLen * velY;
            f32 scaledY = 19.0f * vel.f.y;
            f32 scaledX = 19.0f * vel.f.x;
            vel.f.x = scaledX;
            vel.f.y = scaledY;
            cappedVel.f.y = vel.f.y;
            cappedVel.f.x = vel.f.x;
            cappedVel.f.z = m_v3Velocity.f.z;
            m_v3Velocity = cappedVel;
            m_pPhysicsObject->SetLinearVelocity(cappedVel);
        }
    }

    if (m_bShouldDestroy)
    {
        m_pDrawableObj->m_uObjectFlags &= ~1;
        Destroy(false);
    }
}

/**
 * Offset/Address/Size: 0x19AC | 0x8005C298 | size: 0xD4
 */
void GreenShell::Destroy(bool bSilent)
{
    if (!bSilent && !g_pGame->mbCaptainShotToScoreOn)
    {
        EffectsGroup* pEffectsGroup;
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            pEffectsGroup = fxGetGroup("greenshell_explode_big");
            break;
        case POWERUPSIZE_MEDIUM:
            pEffectsGroup = fxGetGroup("greenshell_explode_med");
            break;
        case POWERUPSIZE_SMALL:
            pEffectsGroup = fxGetGroup("greenshell_explode");
            break;
        }

        EmissionController* pController = EmissionManager::Create(pEffectsGroup, 0);
        pController->SetPosition(m_pPhysicsObject->GetPosition());
    }

    PowerupBase::Destroy(bSilent);
}

/**
 * Offset/Address/Size: 0x1874 | 0x8005C160 | size: 0x138
 * TODO: 99.74% match - remaining instruction-only diffs are powerupModelPool unnamed-namespace symbol aliases
 */
RedShell::~RedShell()
{
}

/**
 * Offset/Address/Size: 0x1628 | 0x8005BF14 | size: 0x24C
 */
extern "C" void SeekTarget__8RedShellFv(RedShell*);

void RedShell::Update(float dt)
{
    nlPolar polar;
    nlVector2 vel;
    nlPolar polar2;
    nlVector3 cappedVel;

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsObject->GetPosition(&m_v3Position);
    m_pPhysicsObject->GetLinearVelocity(&m_v3Velocity);

    if (m_v3Position.f.z < ((PhysicsSphere*)m_pPhysicsObject)->GetRadius())
    {
        m_v3Position.f.z = ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
        m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    }

    if (m_pBlurHandler != NULL)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }

    mtActiveTimer.Countdown(dt, 0.0f);
    mtNoHitTimer.Countdown(dt, 0.0f);

    UpdateTransform();

    if (m_pBlurHandler != NULL)
    {
        nlCartesianToPolar(polar, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar.r < 0.5f)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = NULL;
        }
    }

    if (mtNoHitTimer.m_uPackedTime == 0)
    {
        nlCartesianToPolar(polar2, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar2.r < 3.0f)
        {
            m_bShouldDestroy = true;
        }
        else if (polar2.r > 20.0f)
        {
            vel.as_u32[0] = m_v3Velocity.as_u32[0];
            vel.as_u32[1] = m_v3Velocity.as_u32[1];
            f32 velX = vel.f.x;
            f32 velY = vel.f.y;
            f32 sqX = velX * velX;
            f32 sqY = velY * velY;
            f32 recipLen = nlRecipSqrt(sqX + sqY, true);
            vel.f.x = recipLen * velX;
            vel.f.y = recipLen * velY;
            f32 scaledY = 19.0f * vel.f.y;
            f32 scaledX = 19.0f * vel.f.x;
            vel.f.x = scaledX;
            vel.f.y = scaledY;
            cappedVel.f.y = vel.f.y;
            cappedVel.f.x = vel.f.x;
            cappedVel.f.z = m_v3Velocity.f.z;
            m_v3Velocity = cappedVel;
            m_pPhysicsObject->SetLinearVelocity(cappedVel);
        }
    }

    if (mtActiveTimer.m_uPackedTime == 0)
    {
        m_pTarget = NULL;
    }
    else if (mtNoHitTimer.GetSeconds() < 0.8f)
    {
        SeekTarget__8RedShellFv(this);
    }

    if (m_bShouldDestroy)
    {
        m_pDrawableObj->m_uObjectFlags &= ~1u;
        Destroy(false);
    }
}

/**
 * Offset/Address/Size: 0x1554 | 0x8005BE40 | size: 0xD4
 */
void RedShell::Destroy(bool bSilent)
{
    if (!bSilent && !g_pGame->mbCaptainShotToScoreOn)
    {
        EffectsGroup* pEffectsGroup;
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            pEffectsGroup = fxGetGroup("redshell_explode_big");
            break;
        case POWERUPSIZE_MEDIUM:
            pEffectsGroup = fxGetGroup("redshell_explode_med");
            break;
        case POWERUPSIZE_SMALL:
            pEffectsGroup = fxGetGroup("redshell_explode");
            break;
        }

        EmissionController* pController = EmissionManager::Create(pEffectsGroup, 0);
        pController->SetPosition(m_pPhysicsObject->GetPosition());
    }

    PowerupBase::Destroy(bSilent);
}

/**
 * Offset/Address/Size: 0x13C8 | 0x8005BCB4 | size: 0x18C
 */
void RedShell::SeekTarget()
{
    float fCurrSpeed;
    float dx, dy;
    float newVelY, newVelX;
    nlVector3 v3NewVelocity;

    cFielder* target = m_pTarget;
    if (target == NULL)
    {
        return;
    }

    const nlVector3& targetPos = ((cCharacter*)target)->m_v3Position;
    dy = targetPos.f.y - m_v3Position.f.y;
    dx = targetPos.f.x - m_v3Position.f.x;

    float distSq = dy * dy + dx * dx;

    // Dead call - result discarded but compiler doesn't optimize it away
    nlSqrt(distSq, true);

    float invDist = 1.0f / nlSqrt(distSq, true);
    dx = invDist * dx;
    dy = invDist * dy;

    float velX = m_v3Velocity.f.x;
    float velY = m_v3Velocity.f.y;
    fCurrSpeed = nlGetLength2D(velY, velX);

    float turnRate = 5.0f;
    float steerY = turnRate * dy;
    float steerX = turnRate * dx;
    newVelY = steerY + m_v3Velocity.f.y;
    newVelX = steerX + m_v3Velocity.f.x;

    float newSpeed = nlSqrt(newVelY * newVelY + newVelX * newVelX, true);
    float invNewSpeed = 1.0f / newSpeed;
    float normNewVelY = invNewSpeed * newVelY;
    float normNewVelX = invNewSpeed * newVelX;

    float dot = normNewVelY * dy + normNewVelX * dx;
    if (dot < 0.0f)
    {
        m_pTarget = NULL;
    }
    else
    {
        nlVec3Set(v3NewVelocity, fCurrSpeed * normNewVelY, fCurrSpeed * normNewVelX, 0.0f);
        m_v3Velocity = v3NewVelocity;
        m_pPhysicsObject->SetLinearVelocity(v3NewVelocity);
        m_pPhysicsObject->SetLinearVelocity(m_v3Velocity);
    }
}

/**
 * Offset/Address/Size: 0x1290 | 0x8005BB7C | size: 0x138
 */
Banana::~Banana()
{
}

/**
 * Offset/Address/Size: 0x1130 | 0x8005BA1C | size: 0x160
 */
void Banana::Update(float dt)
{
    nlPolar polar;

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsObject->GetPosition(&m_v3Position);
    m_pPhysicsObject->GetLinearVelocity(&m_v3Velocity);

    if (m_v3Position.f.z < ((PhysicsSphere*)m_pPhysicsObject)->GetRadius())
    {
        m_v3Position.f.z = ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
        m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    }

    if (m_pBlurHandler != nullptr)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }

    mtActiveTimer.Countdown(dt, 0.0f);
    mtNoHitTimer.Countdown(dt, 0.0f);

    UpdateTransform();

    if (m_pBlurHandler != nullptr)
    {
        nlCartesianToPolar(polar, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar.r < 0.5f)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = nullptr;
        }
    }

    if (mtActiveTimer.m_uPackedTime == 0)
    {
        m_bShouldDestroy = true;
    }

    if (m_bShouldDestroy)
    {
        m_pDrawableObj->m_uObjectFlags &= ~1;
        Destroy(false);
    }
}

/**
 * Offset/Address/Size: 0x105C | 0x8005B948 | size: 0xD4
 */
void Banana::Destroy(bool bSilent)
{
    if (!bSilent && !g_pGame->mbCaptainShotToScoreOn)
    {
        EffectsGroup* pEffectsGroup;
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            pEffectsGroup = fxGetGroup("banana_explode_big");
            break;
        case POWERUPSIZE_MEDIUM:
            pEffectsGroup = fxGetGroup("banana_explode_med");
            break;
        case POWERUPSIZE_SMALL:
            pEffectsGroup = fxGetGroup("banana_explode");
            break;
        }

        EmissionController* pController = EmissionManager::Create(pEffectsGroup, 0);
        pController->SetPosition(m_pPhysicsObject->GetPosition());
    }

    PowerupBase::Destroy(bSilent);
}

/**
 * Offset/Address/Size: 0xF24 | 0x8005B810 | size: 0x138
 */
SpinyShell::~SpinyShell()
{
}

/**
 * Offset/Address/Size: 0xD0C | 0x8005B5F8 | size: 0x218
 * TODO: 99.78% match - f4/f1 register allocation diff in 5.0f velocity cap
 *       multiplication; likely MWCC register allocator version quirk.
 */
void SpinyShell::Update(float dt)
{
    nlPolar polar;
    nlVector2 vel;
    nlPolar polar2;

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsObject->GetPosition(&m_v3Position);
    m_pPhysicsObject->GetLinearVelocity(&m_v3Velocity);

    if (m_v3Position.f.z < ((PhysicsSphere*)m_pPhysicsObject)->GetRadius())
    {
        m_v3Position.f.z = ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
        m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    }

    if (m_pBlurHandler != NULL)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }

    mtActiveTimer.Countdown(dt, 0.0f);
    mtNoHitTimer.Countdown(dt, 0.0f);

    UpdateTransform();

    if (m_pBlurHandler != NULL)
    {
        nlCartesianToPolar(polar, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar.r < 0.5f)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = NULL;
        }
    }

    if (mtNoHitTimer.m_uPackedTime == 0)
    {
        nlCartesianToPolar(polar2, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar2.r < 0.5f)
        {
            m_bShouldDestroy = true;
        }
        else if (polar2.r > 5.0f)
        {
            vel.as_u32[0] = m_v3Velocity.as_u32[0];
            vel.as_u32[1] = m_v3Velocity.as_u32[1];
            f32 velX = vel.f.x;
            f32 velY = vel.f.y;
            f32 sqX = velX * velX;
            f32 sqY = velY * velY;
            f32 recipLen = nlRecipSqrt(sqX + sqY, true);
            nlVec2Set(vel, recipLen * velX, recipLen * velY);
            nlVec2Set(vel, 5.0f * vel.f.x, 5.0f * vel.f.y);
            nlVector3 cappedVel;
            cappedVel.f.y = vel.f.y;
            cappedVel.f.x = vel.f.x;
            cappedVel.f.z = m_v3Velocity.f.z;
            m_v3Velocity = cappedVel;
            m_pPhysicsObject->SetLinearVelocity(cappedVel);
        }
    }

    if (m_bShouldDestroy)
    {
        m_pDrawableObj->m_uObjectFlags &= ~1u;
        Destroy(false);
    }
}

/**
 * Offset/Address/Size: 0xC38 | 0x8005B524 | size: 0xD4
 */
void SpinyShell::Destroy(bool bSilent)
{
    if (!bSilent && !g_pGame->mbCaptainShotToScoreOn)
    {
        EffectsGroup* pEffectsGroup;
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            pEffectsGroup = fxGetGroup("spiny_shell_explode_big");
            break;
        case POWERUPSIZE_MEDIUM:
            pEffectsGroup = fxGetGroup("spiny_shell_explode_med");
            break;
        case POWERUPSIZE_SMALL:
            pEffectsGroup = fxGetGroup("spiny_shell_explode");
            break;
        }

        EmissionController* pController = EmissionManager::Create(pEffectsGroup, 0);
        pController->SetPosition(m_pPhysicsObject->GetPosition());
    }

    PowerupBase::Destroy(bSilent);
}

/**
 * Offset/Address/Size: 0xB00 | 0x8005B3EC | size: 0x138
 */
FreezeShell::~FreezeShell()
{
}

/**
 * Offset/Address/Size: 0x8E8 | 0x8005B1D4 | size: 0x218
 */
void FreezeShell::Update(float fDeltaT)
{
    nlPolar polar;
    nlVector2 vel;
    nlPolar polar2;

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsObject->GetPosition(&m_v3Position);
    m_pPhysicsObject->GetLinearVelocity(&m_v3Velocity);

    if (m_v3Position.f.z < ((PhysicsSphere*)m_pPhysicsObject)->GetRadius())
    {
        m_v3Position.f.z = ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
        m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    }

    if (m_pBlurHandler != NULL)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }

    mtActiveTimer.Countdown(fDeltaT, 0.0f);
    mtNoHitTimer.Countdown(fDeltaT, 0.0f);

    UpdateTransform();

    if (m_pBlurHandler != NULL)
    {
        nlCartesianToPolar(polar, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar.r < 0.5f)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = NULL;
        }
    }

    if (mtNoHitTimer.m_uPackedTime == 0)
    {
        nlCartesianToPolar(polar2, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar2.r < 3.0f)
        {
            m_bShouldDestroy = true;
        }
        else if (polar2.r > 20.0f)
        {
            vel.as_u32[0] = m_v3Velocity.as_u32[0];
            vel.as_u32[1] = m_v3Velocity.as_u32[1];
            f32 velX = vel.f.x;
            f32 velY = vel.f.y;
            f32 sqX = velX * velX;
            f32 sqY = velY * velY;
            f32 recipLen = nlRecipSqrt(sqX + sqY, true);
            vel.f.x = recipLen * velX;
            vel.f.y = recipLen * velY;
            nlVec2Set(vel, 19.0f * vel.f.x, 19.0f * vel.f.y);
            nlVector3 cappedVel;
            cappedVel.f.y = vel.f.y;
            cappedVel.f.x = vel.f.x;
            cappedVel.f.z = m_v3Velocity.f.z;
            m_v3Velocity = cappedVel;
            m_pPhysicsObject->SetLinearVelocity(cappedVel);
        }
    }

    if (m_bShouldDestroy)
    {
        m_pDrawableObj->m_uObjectFlags &= ~1u;
        Destroy(false);
    }
}

/**
 * Offset/Address/Size: 0x814 | 0x8005B100 | size: 0xD4
 */
void FreezeShell::Destroy(bool bSilent)
{
    if (!bSilent && !g_pGame->mbCaptainShotToScoreOn)
    {
        EffectsGroup* pEffectsGroup;
        switch (meSize)
        {
        case POWERUPSIZE_LARGE:
            pEffectsGroup = fxGetGroup("freeze_shell_explode_big");
            break;
        case POWERUPSIZE_MEDIUM:
            pEffectsGroup = fxGetGroup("freeze_shell_explode_med");
            break;
        case POWERUPSIZE_SMALL:
            pEffectsGroup = fxGetGroup("freeze_shell_explode");
            break;
        }

        EmissionController* pController = EmissionManager::Create(pEffectsGroup, 0);
        pController->SetPosition(m_pPhysicsObject->GetPosition());
    }

    PowerupBase::Destroy(bSilent);
}

/**
 * Offset/Address/Size: 0x6DC | 0x8005AFC8 | size: 0x138
 */
Bobomb::~Bobomb()
{
}

/**
 * Offset/Address/Size: 0x428 | 0x8005AD14 | size: 0x2B4
 */
void Bobomb::Update(float dt)
{
    ePowerUpType type;
    PhysicsObject* pPhysObj;
    u32 soundID;
    nlPolar polar;
    nlVector3 pos;
    EmissionController* pController;
    Audio::SoundAttributes attributes;

    m_v3PrevPosition = m_v3Position;
    m_pPhysicsObject->GetPosition(&m_v3Position);
    m_pPhysicsObject->GetLinearVelocity(&m_v3Velocity);

    if (m_v3Position.f.z < ((PhysicsSphere*)m_pPhysicsObject)->GetRadius())
    {
        m_v3Position.f.z = ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
        m_pPhysicsObject->SetPosition(m_v3Position, PhysicsObject::WORLD_COORDINATES);
    }

    if (m_pBlurHandler != nullptr)
    {
        m_pBlurHandler->AddViewOrientedPoint(m_v3Position, m_v3Velocity);
    }

    mtActiveTimer.Countdown(dt, 0.0f);
    mtNoHitTimer.Countdown(dt, 0.0f);

    UpdateTransform();

    if (m_pBlurHandler != nullptr)
    {
        nlCartesianToPolar(polar, m_v3Velocity.f.x, m_v3Velocity.f.y);
        if (polar.r < 0.5f)
        {
            m_pBlurHandler->Die(0.5f);
            m_pBlurHandler = nullptr;
        }
    }

    if ((m_uVoiceID == 0) && (!g_pGame->mbCaptainShotToScoreOn))
    {
        u32 errorCode;

        type = m_eType;
        pPhysObj = m_pPhysicsObject;

        if (type >= NUM_POWER_UPS)
        {
            errorCode = Audio::GetSndIDError();
        }
        else if (!Audio::IsInited())
        {
            errorCode = Audio::GetSndIDError();
        }
        else
        {
            attributes.Init();
            soundID = powerupSounds[type].sndInEffect;

            if (soundID == 0xFFFFFFFF)
            {
                errorCode = -1;
            }
            else
            {
                attributes.SetSoundType(soundID, true);
                if (type == POWER_UP_BOBOMB)
                {
                    attributes.UsePhysObj(pPhysObj);
                    attributes.mf_ReturnEmitterOnPlay = true;
                }
                else
                {
                    attributes.UsePhysObj(pPhysObj);
                    attributes.mf_ReturnEmitterOnPlay = true;
                }

                Audio::gPowerupSFX.GetSFXVol(soundID);
                errorCode = Audio::gPowerupSFX.Play(attributes);
            }
        }

        m_uVoiceID = errorCode;
    }

    pController = EmissionManager::Create(fxGetGroup("bobomb_tick"), 0);
    pos = m_pPhysicsObject->GetPosition();
    pos.f.z += ((PhysicsSphere*)m_pPhysicsObject)->GetRadius();
    pController->SetPosition(pos);

    if (mtActiveTimer.m_uPackedTime == 0)
    {
        m_bShouldDestroy = true;
    }

    if (m_bShouldDestroy)
    {
        m_pDrawableObj->m_uObjectFlags &= ~1;
        Destroy(false);
    }
}

/**
 * Offset/Address/Size: 0x80 | 0x8005A96C | size: 0x3A8
 * TODO: 99.89% match - 4 register diffs at 0x29c-0x2b0: frsp dest register f0 vs f1,
 *       cascading to maxZSpeed load/compare/store registers. MWCC ternary compiler state artifact.
 */
void Bobomb::ThrowAt(cFielder*, Bowser*)
{
    if (gBobombAnticipationVoiceID == -1)
    {
        if (Audio::IsSFXPlaying(gBobombAnticipationVoiceID))
        {
            goto skip_anticipation;
        }
    }

    gBobombAnticipationVoiceID = Audio::gCrowdSFX.Play((Audio::eWorldSFX)0x9E, 1.0f, 0.5f, true, 1.0f);

skip_anticipation:
    if (m_pTarget->m_pTeam == g_pTeams[1])
    {
        Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_YEAH_SMALL, 1.0f, 0.5f, 0, 0.0f);
    }
    else
    {
        Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_OH_SMALL, 1.0f, 0.5f, 0, 0.0f);
    }

    int nNumSolutions;
    float pSolutions[2];
    nlVector3 v3TargetPos;
    nlVector3 v3TargetVel;

    v3TargetPos = m_pTarget->m_v3Position;
    v3TargetVel = m_pTarget->m_v3Velocity;

    CalcInterceptXY(m_v3Position, g_pGame->m_pGameTweaks->fBobombSpeed, 0.0f, v3TargetPos, v3TargetVel, nNumSolutions, pSolutions);

    float dx, dy, dz;
    float t;

    if (nNumSolutions != 0)
    {
        if (nNumSolutions == 2)
        {
            t = (pSolutions[0] < pSolutions[1]) ? pSolutions[0] : pSolutions[1];
        }
        else
        {
            t = pSolutions[0];
        }

        if (t > g_pGame->m_pGameTweaks->unk228)
        {
            ePowerUpType type = m_eType;
            PhysicsObject* pPhysObj = m_pPhysicsObject;
            unsigned long errorCode;

            if (type >= NUM_POWER_UPS)
            {
                errorCode = Audio::GetSndIDError();
            }
            else if (!Audio::IsInited())
            {
                errorCode = Audio::GetSndIDError();
            }
            else
            {
                Audio::SoundAttributes attributes;
                unsigned long soundID;

                attributes.Init();
                soundID = powerupSounds[type].sndActivate;

                if (soundID == 0xFFFFFFFF)
                {
                    errorCode = -1;
                }
                else
                {
                    attributes.SetSoundType(soundID, true);

                    if (type == POWER_UP_BOBOMB)
                    {
                        attributes.UsePhysObj(pPhysObj);
                        attributes.mf_ReturnEmitterOnPlay = true;
                    }
                    else
                    {
                        attributes.UseStationaryPosVector(pPhysObj->GetPosition());
                    }

                    Audio::gPowerupSFX.GetSFXVol(soundID);
                    errorCode = Audio::gPowerupSFX.Play(attributes);
                }
            }

            pMovementEmitter = (SFXEmitter*)errorCode;
        }

        nlVector3 v3BobombVelocity;
        float targetX = v3TargetVel.f.x * t + v3TargetPos.f.x;
        float targetY = v3TargetVel.f.y * t + v3TargetPos.f.y;

        v3BobombVelocity.f.x = (targetX - m_v3Position.f.x) / t;
        v3BobombVelocity.f.y = (targetY - m_v3Position.f.y) / t;
        v3BobombVelocity.f.z = -(t * (0.5f * m_pPhysicsObject->m_gravity));

        if (v3BobombVelocity.f.z > g_pGame->m_pGameTweaks->fBobombMaxZSpeed)
        {
            v3BobombVelocity.f.z = g_pGame->m_pGameTweaks->fBobombMaxZSpeed;
        }

        m_v3Velocity = v3BobombVelocity;
        m_pPhysicsObject->SetLinearVelocity(v3BobombVelocity);
    }
    else
    {
        dy = v3TargetPos.f.y - m_v3Position.f.y;
        dx = v3TargetPos.f.x - m_v3Position.f.x;
        dz = v3TargetPos.f.z - m_v3Position.f.z;
        float invDist = nlRecipSqrt((dx * dx) + (dy * dy) + (dz * dz), true);
        float speed = g_pGame->m_pGameTweaks->fBobombSpeed;
        nlVector3 v3BobombVelocity;

        v3BobombVelocity.f.x = speed * (invDist * dx);
        v3BobombVelocity.f.y = speed * (invDist * dy);
        v3BobombVelocity.f.z = speed * (invDist * dz);
        v3BobombVelocity.f.z = g_pGame->m_pGameTweaks->fBobombMaxZSpeed;

        m_v3Velocity = v3BobombVelocity;
        m_pPhysicsObject->SetLinearVelocity(v3BobombVelocity);
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8005A8EC | size: 0x80
 */
void Bobomb::Destroy(bool bSilent)
{
    if (gBobombAnticipationVoiceID != -1)
    {
        Audio::StopSFX(gBobombAnticipationVoiceID);
        gBobombAnticipationVoiceID = -1;
    }

    if (pMovementEmitter != nullptr)
    {
        Audio::gPowerupSFX.StopEmitter(pMovementEmitter, 0);
        pMovementEmitter = nullptr;
    }

    PowerupBase::Destroy(bSilent);
}
