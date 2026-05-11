#include "Game/Physics/PhysicsShell.h"
#include "Game/AI/Powerups.h"
#include "Game/Game.h"
#include "Game/AI/Fielder.h"
#include "Game/Physics/PhysicsBanana.h"
#include "Game/Physics/PhysicsObject.h"
#include "Game/Sys/eventman.h"

extern CollisionSpace* g_CollisionSpace;
extern PhysicsWorld* g_PhysicsWorld;

/**
 * Offset/Address/Size: 0x0 | 0x8013B968 | size: 0x94
 */
bool PhysicsShell::SetContactInfo(dContact* contact, PhysicsObject* other, bool first)
{
    if (first)
    {
        SetDefaultContactInfo(contact);
    }

    if (other->GetObjectType() == 0x11)
    {
        contact->surface.bounce = g_pGame->m_pGameTweaks->fShellBounceGround;
    }
    else
    {
        contact->surface.bounce = g_pGame->m_pGameTweaks->fShellBounce;
    }

    contact->surface.bounce_vel = 5.0f;
    contact->surface.mu = 0.0f;

    return true;
}

/**
 * Offset/Address/Size: 0x94 | 0x8013B9FC | size: 0x78
 */
void PhysicsShell::PostUpdate()
{
    PhysicsObject::PostUpdate();

    nlVector3 velocity;
    GetLinearVelocity(&velocity);

    nlVector3& pos = GetPosition();
    if (pos.f.z > 20.0f && velocity.f.z > 0.0f)
    {
        velocity.f.z *= 0.9f;
        SetLinearVelocity(velocity);
    }
}

/**
 * Offset/Address/Size: 0x10C | 0x8013BA74 | size: 0x898
 */
ContactType PhysicsShell::Contact(PhysicsObject* obj, dContact* info, int numContacts)
{
    ContactType returnValue = TWO_WAY_CONTACT;
    nlVector3 curPosition;
    int objType;

    bool bVar5, bVar6;
    int iVar8;

    this->GetPosition(&curPosition);

    switch (objType) {
        case 0xd: // 
        case 0xe:
            cFielder *pcVar7 = static_cast<cFielder*>(static_cast<PhysicsCharacter*>(obj->m_parentObject)->m_pAICharacter);
            if (pcVar7->m_eClassType == FIELDER) {
                if (this->m_pPowerupObject->mtNoHitTimer.m_uPackedTime != 0
                        && this->m_pPowerupObject->m_pThrower == pcVar7) {
                    return NO_CONTACT;
                }
                bool isFrozen = pcVar7->IsFrozen();
                if (isFrozen)
                    returnValue = ONE_WAY_CONTACT_THIS;
                else if (this->m_pTriggerCallbackFunc != nullptr) {
                    nlVector3 local = {};
                    this->m_pTriggerCallbackFunc(this, obj, local, this->m_pCallbackParam);
                }
                if (this->m_pPowerupObject->meSize == POWERUPSIZE_LARGE)
                    return NO_CONTACT;
                if (this->m_pPowerupObject->m_eType == POWER_UP_SPINY_SHELL) {
                    if (!isFrozen && pcVar7->IsInvincible())
                        return NO_CONTACT;
                    returnValue = ONE_WAY_CONTACT_THIS;
                }
            }
            else {
                Event *pEVar9 = g_pEventManager->CreateValidEvent(0x23, 0x2c);
                CollisionPowerupGroundData *pEVar13 = static_cast<CollisionPowerupGroundData *>(&pEVar9->m_data);
                this->GetPosition(&pEVar13->position);
                pEVar13->eType = this->m_pPowerupObject->m_eType;
            }
            break;
        case 0xf:
            cBall *fVar8 = static_cast<PhysicsAIBall *>(obj)->m_pAIBall;
            pcVar7 = static_cast<cFielder *>(fVar8->m_pOwner);
            if (pcVar7 == nullptr) {
                bVar5 = false;
                if (fVar8->m_tShotTimer.m_uPackedTime != 0 /*&& fVar8->field_0xa2*/)
                    bVar5 = true;
                if (bVar5) {
                    this->m_pPowerupObject->m_bShouldDestroy = true;
                    return NO_CONTACT;
                }
                if (this->m_pPowerupObject->meSize != POWERUPSIZE_SMALL)
                    iVar8 = 2;
            }
            else {
                if (pcVar7->m_eClassType == FIELDER) {
                    if (this->m_pPowerupObject->mtNoHitTimer.m_uPackedTime != 0
                            && this->m_pPowerupObject->m_pThrower == pcVar7) {
                        return NO_CONTACT;
                    }
                    bool isBallAway = pcVar7->IsBallAwayFromCarrier();
                    if (isBallAway) {
                        if (this->m_pTriggerCallbackFunc != nullptr) {
                            nlVector3 local = {};
                            this->m_pTriggerCallbackFunc(this, obj, local, this->m_pCallbackParam);
                        }
                        return NO_CONTACT;
                    }
                }
                else {
                    bVar6 = true;
                }
                if (this->m_pPowerupObject->m_eType == POWER_UP_SPINY_SHELL)
                    iVar8 = 2;
            }
            if (!bVar6 && this->m_pTriggerCallbackFunc != nullptr) {
                nlVector3 local;
                this->m_pTriggerCallbackFunc(this, obj, local, this->m_pCallbackParam);
            }
            if (iVar8 == 2)
                return ONE_WAY_CONTACT_OTHER;
            break;
        default:
            int temp_iVar8;
            if (objType == 0x11 && numContacts > 0) {
                do {

                } while (iVar8 != 0);
            }
            break;
        case 0x13: // PhysicsShell
            PhysicsShell *pShell = static_cast<PhysicsShell *>(obj);
            if (this->m_pPowerupObject->mtNoHitTimer.m_uPackedTime != 0
                    && pShell->m_pPowerupObject->m_pThrower == this->m_pPowerupObject->m_pThrower)
                return NO_CONTACT;
            if (this->m_pTriggerCallbackFunc != nullptr) {
                nlVector3 local = {};
                this->m_pTriggerCallbackFunc(this, obj, local, this->m_pCallbackParam);
            }
            if (pShell->m_pPowerupObject->meSize < this->m_pPowerupObject->meSize)
                return NO_CONTACT;
            break;
        case 0x14:
            break;
        case 0x1a:
            break;
    }
    if (bVar6) {
        if (objType == 0x19) {

        }
    }

    return returnValue;
}

/**
 * Offset/Address/Size: 0x9A4 | 0x8013C30C | size: 0x7C
 */
PhysicsShell::PhysicsShell(float radius)
    : PhysicsSphere(g_CollisionSpace, g_PhysicsWorld, radius)
{

    m_pTriggerCallbackFunc = 0;
    m_pCallbackParam = 0;
    m_pPowerupObject = 0;
    mbIsInNet = false;
    m_bIsSupportedByGround = false;

    SetCollide(0xef);
    SetCategory(0x20);
    m_gravity = -32.f;
}
