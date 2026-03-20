#include "Game/Physics/PhysicsNPC.h"
#include "Game/Physics/PhysicsCharacter.h"
#include "Game/Physics/PhysicsAIBall.h"
#include "Game/Physics/PhysicsShell.h"
#include "Game/Physics/PhysicsBanana.h"
#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Render/SkinAnimatedMovableNPC.h"
#include "Game/Render/ChainChomp.h"

#include "Game/Player.h"
#include "Game/Ball.h"
#include "Game/Powerups.h"

extern CollisionSpace* g_CollisionSpace;

/**
 * Offset/Address/Size: 0x440 | 0x8013B8F8 | size: 0x68
 */
PhysicsNPC::PhysicsNPC(float radius)
    : PhysicsSphere(g_CollisionSpace, (PhysicsWorld*)0, radius)
{
    mpTriggerCallbackFunc = NULL;
    mpAINPC = NULL;

    SetCollide(0x00);
    SetCategory(0xFF);
}

/**
 * Offset/Address/Size: 0x438 | 0x8013B8F0 | size: 0x8
 */
void PhysicsNPC::SetCallbackFunction(CallbackFn cb)
{
    mpTriggerCallbackFunc = cb;
}

/**
 * Offset/Address/Size: 0xC0 | 0x8013B578 | size: 0x378
 * TODO: 99.05% match - r4/r28 register scheduling for aiChar, f2/f1 float load order in callbacks
 */
ContactType PhysicsNPC::Contact(PhysicsObject* other, dContact* contact, int what)
{
    nlVector3 position;
    GetPosition(&position);
    cCharacter* aiChar;
    cBall* ball;
    int objectType = other->GetObjectType();
    switch (objectType)
    {
    case 0x04:
    case 0x0D:
    case 0x0E:
    {
        PhysicsCharacter* physChar = (PhysicsCharacter*)other->m_parentObject;
        if (physChar->m_pAICharacter->m_eClassType == FIELDER)
        {
            aiChar = physChar->m_pAICharacter;
            bool shouldTrigger = true;
            if (mpAINPC->GetSkinAnimatedNPC_Type() == SkinAnimatedNPC_CHAIN_CHOMP)
            {
                if (!((cPlayer*)aiChar)->IsOnSameTeam((cPlayer*)((ChainChomp*)mpAINPC)->mpTarget))
                {
                    shouldTrigger = false;
                }
            }
            if (shouldTrigger)
            {
                if (mpTriggerCallbackFunc != NULL)
                {
                    f32 px = contact->geom.pos[0];
                    f32 py = contact->geom.pos[1];
                    f32 pz = contact->geom.pos[2];
                    nlVector3 contactPos;
                    contactPos.f.x = px;
                    contactPos.f.y = py;
                    contactPos.f.z = pz;
                    mpTriggerCallbackFunc(this, other, contactPos);
                    break;
                }
            }
            return NO_CONTACT;
        }
        else if (physChar->m_pAICharacter->m_eClassType == GOALIE)
        {
            if (mpAINPC->GetSkinAnimatedNPC_Type() == SkinAnimatedNPC_BOWSER)
            {
                return ONE_WAY_CONTACT_OTHER;
            }
        }
        break;
    }
    case 0x0F:
    {
        ball = ((PhysicsAIBall*)other)->m_pAIBall;
        ball->m_unk_0xA6 = false;
        ball->mpDamageTarget = NULL;
        if (ball->m_pOwner != NULL)
        {
            if (ball->m_pOwner->m_eClassType != FIELDER)
                break;
            if (mpAINPC->GetSkinAnimatedNPC_Type() != SkinAnimatedNPC_CHAIN_CHOMP)
                break;
            if (ball->m_pOwner->IsOnSameTeam((cPlayer*)((ChainChomp*)mpAINPC)->mpTarget))
            {
                if (mpTriggerCallbackFunc != NULL)
                {
                    f32 px = contact->geom.pos[0];
                    f32 py = contact->geom.pos[1];
                    f32 pz = contact->geom.pos[2];
                    nlVector3 contactPos;
                    contactPos.f.x = px;
                    contactPos.f.y = py;
                    contactPos.f.z = pz;
                    mpTriggerCallbackFunc(this, other, contactPos);
                }
                break;
            }
            else
            {
                return NO_CONTACT;
            }
        }
        else
        {
            bool cannotCollide = false;
            if (((PhysicsAIBall*)other)->m_pAIBall->m_tShotTimer.m_uPackedTime != 0 && ((PhysicsAIBall*)other)->m_pAIBall->mbCanDamage)
            {
                cannotCollide = true;
            }
            if (cannotCollide)
            {
                return NO_CONTACT;
            }
            if (mpTriggerCallbackFunc != NULL)
            {
                f32 px = contact->geom.pos[0];
                f32 py = contact->geom.pos[1];
                f32 pz = contact->geom.pos[2];
                nlVector3 contactPos;
                contactPos.f.x = px;
                contactPos.f.y = py;
                contactPos.f.z = pz;
                mpTriggerCallbackFunc(this, other, contactPos);
            }
            ((PhysicsBall*)other)->m_bUseMagnusEffect = false;
            FakeBallWorld::InvalidateBallCache();
            return ONE_WAY_CONTACT_OTHER;
        }
    }
    case 0x13:
    {
        PhysicsShell* shell = (PhysicsShell*)other;
        if (shell->m_pPowerupObject->mtNoHitTimer.m_uPackedTime != 0)
        {
            if (mpAINPC->GetSkinAnimatedNPC_Type() == SkinAnimatedNPC_BOWSER)
            {
                if (shell->m_pPowerupObject->m_pThrower == NULL)
                {
                    return NO_CONTACT;
                }
            }
        }
        if (mpTriggerCallbackFunc != NULL)
        {
            f32 px = contact->geom.pos[0];
            f32 py = contact->geom.pos[1];
            f32 pz = contact->geom.pos[2];
            nlVector3 contactPos;
            contactPos.f.x = px;
            contactPos.f.y = py;
            contactPos.f.z = pz;
            mpTriggerCallbackFunc(this, other, contactPos);
        }
        break;
    }
    case 0x14:
    {
        PhysicsBanana* banana = (PhysicsBanana*)other;
        if (banana->m_pPowerupObject->mtNoHitTimer.m_uPackedTime != 0)
        {
            if (mpAINPC->GetSkinAnimatedNPC_Type() == SkinAnimatedNPC_BOWSER)
            {
                if (banana->m_pPowerupObject->m_pThrower == NULL)
                {
                    return NO_CONTACT;
                }
            }
        }
        if (mpTriggerCallbackFunc != NULL)
        {
            f32 px = contact->geom.pos[0];
            f32 py = contact->geom.pos[1];
            f32 pz = contact->geom.pos[2];
            nlVector3 contactPos;
            contactPos.f.x = px;
            contactPos.f.y = py;
            contactPos.f.z = pz;
            mpTriggerCallbackFunc(this, other, contactPos);
        }
        break;
    }
    default:
        break;
    }
    if (mpAINPC->GetSkinAnimatedNPC_Type() == SkinAnimatedNPC_BOWSER)
    {
        return ONE_WAY_CONTACT_OTHER;
    }
    return NO_CONTACT;
}

/**
 * Offset/Address/Size: 0x70 | 0x8013B528 | size: 0x50
 */
bool PhysicsNPC::SetContactInfo(dContact* contact, PhysicsObject* /*other*/, bool first)
{
    if (first)
    {
        SetDefaultContactInfo(contact);
    }

    contact->surface.bounce = 0.01f;
    contact->surface.bounce_vel = 0.0f;
    contact->surface.mu = 5.0f;

    return true;
}
