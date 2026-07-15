#include "Game/Physics/PhysicsAIBall.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/Fielder.h"
#include "Game/Ball.h"
#include "Game/CharacterTemplate.h"
#include "Game/Field.h"
#include "Game/FixedUpdateTask.h"
#include "Game/Goalie.h"
#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Physics/PhysicsGoalie.h"
#include "Game/Physics/PhysicsNet.h"

extern CollisionSpace* g_CollisionSpace;
extern PhysicsWorld* g_PhysicsWorld;

/**
 * Offset/Address/Size: 0x0 | 0x80133A34 | size: 0x84
 */
bool PhysicsAIBall::IsBallOutsideNet(const nlVector3& v3Pos)
{
    f32 radius = g_pBall->m_pPhysicsBall->GetRadius();
    f64 absX = (float)fabs(v3Pos.f.x);
    f32 threshold = cField::GetGoalLineX((unsigned int)1);
    f32 sum;
    f32 fAbsX;
    sum = radius + threshold;
    fAbsX = (f32)absX;
    threshold = sum - 0.08f;
    return fAbsX < threshold;
}

inline float nlVec3GetX(const nlVector3& v)
{
    return (float)fabs(v.f.x);
}

inline void InterpolateVector(nlVector3& out, const nlVector3& oldPos, const nlVector3& newPos, float alpha)
{
    float oneMinusAlpha = 1.0f - alpha;
    out.f.x = (oneMinusAlpha * oldPos.f.x) + (alpha * newPos.f.x);
    out.f.y = (oneMinusAlpha * oldPos.f.y) + (alpha * newPos.f.y);
    out.f.z = (oneMinusAlpha * oldPos.f.z) + (alpha * newPos.f.z);
}

/**
 * Offset/Address/Size: 0x84 | 0x80133AB8 | size: 0x1A0
 * TODO: 96.15% match - instruction scheduling diffs at prologue (lfs/mr interleaving).
 * Caused by -inline deferred (scratch) vs -inline auto (build) scheduler difference.
 */
bool PhysicsAIBall::DidBallJustEnterNet(const nlVector3& oldPos, nlVector3 newPos)
{
    f32 absOldX = nlVec3GetX(oldPos);
    f32 absNewX = nlVec3GetX(newPos);
    f32 radius = g_pBall->m_pPhysicsBall->GetRadius();
    f32 goalLineX = cField::GetGoalLineX((unsigned int)1);
    f32 threshold = (goalLineX + radius) - 0.08f;
    nlVector3 impactPos;

    if ((absOldX < threshold) && (absNewX >= threshold))
    {
        f32 xDelta = newPos.f.x - oldPos.f.x;

        if ((float)fabs(xDelta) > 0.0001f)
        {
            f32 planeX;
            if (newPos.f.x > 0.0f)
            {
                planeX = threshold;
            }
            else
            {
                planeX = -threshold;
            }

            f32 t = (planeX - oldPos.f.x) / xDelta;
            InterpolateVector(impactPos, oldPos, newPos, t);
        }
        else
        {
            impactPos = newPos;
        }

        if ((impactPos.f.z > 0.0f) && (impactPos.f.z < cNet::m_fNetHeight))
        {
            f32 netWidth = cNet::m_fNetWidth;
            f32 halfScale = 0.5f;
            if ((impactPos.f.y > (halfScale * -netWidth)) && (impactPos.f.y < (halfScale * netWidth)))
            {
                return true;
            }
        }
    }

    return false;
}

static inline bool DidBallJustEnterNetWithPhysicsBall(PhysicsAIBall* pPhysicsBall, const nlVector3& oldPos, nlVector3 newPos)
{
    f32 absOldX = nlVec3GetX(oldPos);
    f32 absNewX = nlVec3GetX(newPos);
    f32 radius = pPhysicsBall->GetRadius();
    f32 goalLineX = cField::GetGoalLineX((unsigned int)1);
    f32 threshold = (goalLineX + radius) - 0.08f;
    nlVector3 impactPos;

    if ((absOldX < threshold) && (absNewX >= threshold))
    {
        f32 xDelta = newPos.f.x - oldPos.f.x;

        if ((float)fabs(xDelta) > 0.0001f)
        {
            f32 planeX;
            if (newPos.f.x > 0.0f)
            {
                planeX = threshold;
            }
            else
            {
                planeX = -threshold;
            }

            f32 t = (planeX - oldPos.f.x) / xDelta;
            InterpolateVector(impactPos, oldPos, newPos, t);
        }
        else
        {
            impactPos = newPos;
        }

        if ((impactPos.f.z > 0.0f) && (impactPos.f.z < cNet::m_fNetHeight))
        {
            f32 netWidth = cNet::m_fNetWidth;
            f32 halfScale = 0.5f;
            if ((impactPos.f.y > (halfScale * -netWidth)) && (impactPos.f.y < (halfScale * netWidth)))
            {
                return true;
            }
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x0 | 0x80133C58 | size: 0x310
 * TODO: 99.69% match - velY/normalY register swap in reflection math and velocitySq compare register differ
 */
void PhysicsAIBall::CheckIfBallWentThroughGoalPost()
{
    if (m_parentObject == NULL)
    {
        nlVector3 oldPosition;
        nlVector3 newPosition;

        GetPosition(&newPosition);
        oldPosition = m_unk_0x44;

        nlVector3 ballPosition = { 0.0f, 0.0f, 0.0f };
        nlVector3 contactNormal = { 0.0f, 0.0f, 0.0f };
        bool contact;
        PhysicsObject* physicsObject = NULL;
        nlVector3 v3ExitVel;
        nlVector3 v3AngVel;

        if (oldPosition.f.x > 0.0f)
        {
            PhysicsNet* pNet = PhysicsNet::spPhysNetPositiveX;
            float radius = GetRadius();
            contact = pNet->SweepTestForBallContact(oldPosition, newPosition, GetLinearVelocity(), radius, ballPosition, contactNormal, &physicsObject);
        }
        else
        {
            PhysicsNet* pNet = PhysicsNet::spPhysNetNegativeX;
            float radius = GetRadius();
            contact = pNet->SweepTestForBallContact(oldPosition, newPosition, GetLinearVelocity(), radius, ballPosition, contactNormal, &physicsObject);
        }

        if ((contact != 0) && (m_unk_0x59 == 0))
        {
            float contactZ = (0.005f * contactNormal.f.z) + ballPosition.f.z;
            float contactY = (0.005f * contactNormal.f.y) + ballPosition.f.y;
            float contactX = (0.005f * contactNormal.f.x) + ballPosition.f.x;
            ballPosition.f.x = contactX;
            ballPosition.f.y = contactY;
            ballPosition.f.z = contactZ;

            const nlVector3& v3BallVel = GetLinearVelocity();
            float velY = v3BallVel.f.y;
            float normalZ = contactNormal.f.z;
            float velZ = v3BallVel.f.z;
            float normalY = contactNormal.f.y;
            float velX = v3BallVel.f.x;
            float velYSq = velY * velY;
            float velYNormalY = normalY * velY;
            float normalX = contactNormal.f.x;
            float normalYSq = normalY * normalY;
            float velXSq = velX * velX;
            float dotXY = (velX * normalX) + velYNormalY;
            float normalLenXY = (normalX * normalX) + normalYSq;
            float velZSq = velZ * velZ;
            float velDotNormal = (velZ * normalZ) + dotXY;
            float normalLengthSq = (normalZ * normalZ) + normalLenXY;
            float reflectScale = velDotNormal / normalLengthSq;

            nlVec3Set(v3ExitVel,
                (-2.0f * (reflectScale * normalX)) + velX,
                (-2.0f * (reflectScale * normalY)) + velY,
                (-2.0f * (reflectScale * normalZ)) + velZ);

            float velocitySq = velZSq + (velXSq + velYSq);

            nlVec3Scale(v3ExitVel, 0.35f);

            if (velocitySq < 1.0f)
            {
                if (ballPosition.f.x > 0.0f)
                {
                    v3ExitVel.f.x = v3ExitVel.f.x - 0.3f;
                }
                else
                {
                    v3ExitVel.f.x += 0.3f;
                }

                float physicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
                float dt = 0.3f * physicsTick;

                ballPosition.f.z = (dt * v3ExitVel.f.z) + ballPosition.f.z;
                ballPosition.f.y = (dt * v3ExitVel.f.y) + ballPosition.f.y;
                ballPosition.f.x = (dt * v3ExitVel.f.x) + ballPosition.f.x;
            }

            GetAngularVelocity(&v3AngVel);

            v3AngVel.f.z = 0.8f * v3AngVel.f.z;
            float scaledAngY = 0.8f * v3AngVel.f.y;
            float scaledAngX = 0.8f * v3AngVel.f.x;
            v3AngVel.f.x = scaledAngX;
            v3AngVel.f.y = scaledAngY;

            SetPosition(ballPosition, PhysicsObject::WORLD_COORDINATES);
            SetLinearVelocity(v3ExitVel);
            SetAngularVelocity(v3AngVel);

            m_bUseMagnusEffect = false;
            FakeBallWorld::InvalidateBallCache();

            m_pAIBall->m_bBallPathChangeCount += 1;
            cBall* pAIBall = m_pAIBall;
            pAIBall->m_unk_0xA6 = false;
            pAIBall->mpDamageTarget = NULL;
        }
    }
}

/**
 * Offset/Address/Size: 0x534 | 0x80133F68 | size: 0x460
 */
void PhysicsAIBall::CheckIfBallWentThroughGoalie()
{
    if (m_unk_0x5A != 0)
    {
        return;
    }

    nlVector3 oldPosition;
    nlVector3 newPosition;

    GetPosition(&newPosition);
    oldPosition = m_unk_0x44;

    cPlayer* pGoaliePlayer = (cPlayer*)g_pCharacters[8];
    Goalie* pGoalie = (Goalie*)pGoaliePlayer;

    if ((newPosition.f.x * pGoaliePlayer->m_v3Position.f.x) < 0.0f)
    {
        pGoalie = (Goalie*)g_pCharacters[9];
    }

    if (pGoalie->m_pBall != NULL)
    {
        return;
    }

    if (pGoalie->m_tNoPickupTimer.m_uPackedTime != 0)
    {
        return;
    }

    nlVector3 ballPosition = { 0.0f, 0.0f, 0.0f };
    nlVector3 contactNormal = { 0.0f, 0.0f, 0.0f };
    bool contact = false;

    if ((s32)m_unk_0x50 > 3)
    {
        float radius = GetRadius();
        contact = pGoalie->GetPhysicsGoalie()->SweepTestForBallContact(oldPosition, newPosition, GetLinearVelocity(), radius, ballPosition, contactNormal);
    }

    if (contact)
    {
        SetPosition(ballPosition, PhysicsObject::WORLD_COORDINATES);

        dContact contactInfo;
        SetContactInfo(&contactInfo, pGoalie->m_pPhysicsCharacter, true);

        contactInfo.geom.normal[0] = contactNormal.f.x;
        contactInfo.geom.normal[1] = contactNormal.f.y;
        contactInfo.geom.normal[2] = contactNormal.f.z;
        contactInfo.geom.g1 = m_geomID;
        contactInfo.geom.g2 = NULL;

        nlVector3 contactPos;
        nlVec3ScaleAdd(contactPos, -GetRadius(), contactNormal, ballPosition);

        contactInfo.geom.pos[1] = contactPos.f.y;
        contactInfo.geom.pos[2] = contactPos.f.z;
        contactInfo.geom.pos[0] = contactPos.f.x;
        contactInfo.geom.depth = 0.0f;

        if (!pGoalie->PreCollideWithBallCallback(contactInfo))
        {
            return;
        }

        FakeBallWorld::InvalidateBallCache();
        m_bUseMagnusEffect = false;

        nlVector3 v3Vel;
        GetLinearVelocity(&v3Vel);

        Event* event = g_pEventManager->CreateValidEvent(0x27, 0x30);
        CollisionPlayerBallData* pEventData = new (&event->m_data) CollisionPlayerBallData();

        m_pAIBall->ClearBallBlur();

        CollisionPlayerBallData* pCollisionData = (CollisionPlayerBallData*)pEventData;
        pCollisionData->pPlayer = pGoalie;
        pCollisionData->pBall = m_pAIBall;
        pCollisionData->velocity = v3Vel;
        pCollisionData->boneID = 0;
        m_unk_0x50 = 0;

        const float normalY = contactNormal.f.y;
        const float normalX = contactNormal.f.x;
        float normalZ;
        const float velX = v3Vel.f.x;
        const float velY = v3Vel.f.y;
        const float velZ = v3Vel.f.z;
        const float normalYSq = normalY * normalY;
        const float velYNormalY = velY * normalY;
        const float normalLenXY = (normalX * normalX) + normalYSq;
        normalZ = contactNormal.f.z;
        const float dotXY = (velX * normalX) + velYNormalY;
        const float normalLengthSq = (normalZ * normalZ) + normalLenXY;
        const float velDotNormal = (velZ * normalZ) + dotXY;
        const float reflectScale = velDotNormal / normalLengthSq;

        nlVector3 v3ExitVel;
        v3ExitVel.f.x = (-2.0f * (reflectScale * normalX)) + velX;
        v3ExitVel.f.y = (-2.0f * (reflectScale * normalY)) + velY;
        v3ExitVel.f.z = (-2.0f * (reflectScale * normalZ)) + velZ;

        SaveData* pSaveData = pGoalie->mpSaveData;
        if (pSaveData != NULL && (pSaveData->muSaveType & 0x38) != 0)
        {
            nlVector3 v3DeflectFudge;
            RotateVectorZAxis(v3DeflectFudge, v3ExitVel, (u16)-pGoalie->m_aActualFacingDirection);

            float exitSpeed = nlSqrt(
                (v3ExitVel.f.x * v3ExitVel.f.x) + (v3ExitVel.f.y * v3ExitVel.f.y) + (v3ExitVel.f.z * v3ExitVel.f.z),
                true);

            v3DeflectFudge.f.x += 0.5f;

            float saveY = pSaveData->mv3SavePos.f.y;
            v3DeflectFudge.f.y = saveY;

            if (saveY > 0.0f)
            {
                v3DeflectFudge.f.y += 2.0f;
            }
            else
            {
                v3DeflectFudge.f.y -= 2.0f;
            }

            v3DeflectFudge.f.z = 0.5f + pSaveData->mv3SavePos.f.z;
            v3DeflectFudge.f.y = v3DeflectFudge.f.y * (exitSpeed * (0.2f + nlRandomf(0.1f, &nlDefaultSeed)));
            v3DeflectFudge.f.z = v3DeflectFudge.f.z * (exitSpeed * (0.15f + nlRandomf(0.05f, &nlDefaultSeed)));

            RotateVectorZAxis(v3DeflectFudge, v3DeflectFudge, pGoalie->m_aActualFacingDirection);

            float zero = 0.0f;
            float one = 1.0f;
            v3ExitVel.f.x = (zero * v3ExitVel.f.x) + (one * v3DeflectFudge.f.x);
            v3ExitVel.f.y = (zero * v3ExitVel.f.y) + (one * v3DeflectFudge.f.y);
            v3ExitVel.f.z = (zero * v3ExitVel.f.z) + (one * v3DeflectFudge.f.z);
        }

        float scale = 0.175f;
        v3ExitVel.f.z = scale * v3ExitVel.f.z;
        float scaledY = scale * v3ExitVel.f.y;
        float scaledX = scale * v3ExitVel.f.x;
        v3ExitVel.f.x = scaledX;
        v3ExitVel.f.y = scaledY;

        SetLinearVelocity(v3ExitVel);
    }

    m_unk_0x50 += 1;
}

/**
 * Offset/Address/Size: 0x994 | 0x801343C8 | size: 0x3A4
 * TODO: 99.08% match - remaining diffs are in net-entry helper register allocation.
 */
void PhysicsAIBall::PostUpdate()
{
    extern bool gbEnableBallGoalieSweepTest;
    extern float sfBallGoalieSweepTestVelocityThreshold;
    extern void* __vt__9EventData[];
    extern void* __vt__23CollisionBallGroundData[];

    nlVector3 v3IncidentVel;
    CollisionBallGroundData* pEventData;
    nlVector3 ballPosition;
    nlVector3 oldPosition;
    nlVector3 newPosition;

    GetLinearVelocity(&v3IncidentVel);
    PhysicsBall::PostUpdate();

    if (m_bIsSupportedByGround)
    {
        cBall* pBallFields = m_pAIBall;
        if (pBallFields->m_tNoPickupTimer.m_uPackedTime == 0)
        {
            pBallFields->m_unk_0xA6 = false;
            pBallFields->mpDamageTarget = NULL;

            if (v3IncidentVel.f.z < -1.0f)
            {
                pEventData = (CollisionBallGroundData*)((u8*)g_pEventManager->CreateValidEvent(0x24, 0x3C) + 0x10);
                if (pEventData != NULL)
                {
                    *(void**)pEventData = __vt__9EventData;
                    *(void**)pEventData = __vt__23CollisionBallGroundData;
                }

                CollisionBallGroundData* pGroundData = (CollisionBallGroundData*)pEventData;
                s32 bIsShot = 0;

                pGroundData->pBall = m_pAIBall;

                cBall* pEventBallFields = pGroundData->pBall;
                if (pEventBallFields->m_tShotTimer.m_uPackedTime != 0)
                {
                    if (pEventBallFields->m_unk_0xA4 != 0)
                    {
                        bIsShot = 1;
                    }
                }

                if ((u8)bIsShot)
                {
                    pGroundData->bIsShot = 1;
                    m_pAIBall->ClearBallBlur();
                }
                else
                {
                    pGroundData->bIsShot = 0;
                }

                GetPosition(&pGroundData->position);
                pGroundData->normal.f.x = 0.0f;
                pGroundData->normal.f.y = 0.0f;
                pGroundData->normal.f.z = 1.0f;
                pGroundData->fVecZComponent = v3IncidentVel.f.z;
            }
        }
    }

    if (gbEnableBallGoalieSweepTest)
    {
        CheckIfBallWentThroughGoalie();
    }

    if (PhysicsNet::sbSweepTestEnabled)
    {
        CheckIfBallWentThroughGoalPost();
    }

    m_unk_0x59 = false;

    GetRadius();
    GetPosition(&ballPosition);

    {
        f32 radius = g_pBall->m_pPhysicsBall->GetRadius();
        f64 absX = (float)fabs(ballPosition.f.x);
        f32 threshold = cField::GetGoalLineX((unsigned int)1);
        f32 sum;
        f32 fAbsX;
        sum = radius + threshold;
        fAbsX = (f32)absX;
        threshold = sum - 0.08f;

        if (fAbsX < threshold)
        {
            m_unk_0x58 = false;
        }
        else
        {
            GetPosition(&newPosition);
            oldPosition = m_pAIBall->m_v3PrevPosition;

            if (DidBallJustEnterNetWithPhysicsBall(g_pBall->m_pPhysicsBall, oldPosition, newPosition))
            {
                m_unk_0x58 = true;
            }
        }
    }

    const nlVector3& v3Vel = GetLinearVelocity();
    const float velocitySq = (v3Vel.f.x * v3Vel.f.x) + (v3Vel.f.y * v3Vel.f.y) + (v3Vel.f.z * v3Vel.f.z);
    bool& bSpeedBelowThreshold = m_unk_0x5A;
    bSpeedBelowThreshold = velocitySq < (sfBallGoalieSweepTestVelocityThreshold * sfBallGoalieSweepTestVelocityThreshold);
}

/**
 * Offset/Address/Size: 0xD38 | 0x8013476C | size: 0x4C
 */
void PhysicsAIBall::PreUpdate()
{
    PhysicsBall::PreUpdate();
    m_unk_0x44 = GetPosition();
}

/**
 * Offset/Address/Size: 0xD84 | 0x801347B8 | size: 0x47C
 * TODO: 98.52% match - remaining diffs are mostly register assignment shifts
 *       across this/obj/info/numContacts/pFielder.
 */
ContactType PhysicsAIBall::Contact(PhysicsObject* obj, dContact* info, int numContacts)
{
    extern bool gbEnableBallGoalieSweepTest;
    extern float sfMaxBallBounceSpeed;

    int objID;
    cFielder* pFielder;
    nlVector3 ballPosition;
    float radius;
    unsigned char hitWall;
    int i;
    nlVector3 ballVelocity;
    nlPolar aBallSpeed;
    CollisionBallWallData* pEventData;
    float scale;
    nlVector3 vel;

    objID = obj->GetObjectType();

    if (objID == 0xD || objID == 0xE)
    {
        if (gbEnableBallGoalieSweepTest)
        {
            PhysicsCharacter* physicsCharacter = (PhysicsCharacter*)obj->m_parentObject;
            pFielder = (cFielder*)physicsCharacter->m_pAICharacter;

            if (pFielder->m_eClassType == 3)
            {
                if (m_unk_0x5A == 0)
                {
                    return NO_CONTACT;
                }
            }
            else if (pFielder->m_eClassType == 2)
            {
                bool isShootToScore = m_pAIBall->m_tShotTimer.m_uPackedTime != 0 && m_pAIBall->mbCanDamage;

                if (isShootToScore)
                {
                    if (((cPlayer*)pFielder)->m_tNoPickupTimer.m_uPackedTime == 0)
                    {
                        Event* event = g_pEventManager->CreateValidEvent(0x28, 0x20);
                        CollisionPlayerShootToScoreBallData* shootData = new (&event->m_data) CollisionPlayerShootToScoreBallData();
                        shootData->pFielder = pFielder;
                        shootData->pBall = m_pAIBall;
                    }
                    return NO_CONTACT;
                }
            }
        }
    }

    if (objID == 0x1A)
    {
        bool canDamage = false;
        cBall* pBall = m_pAIBall;
        if (pBall->m_tShotTimer.m_uPackedTime != 0)
        {
            if (pBall->mbCanDamage)
            {
                canDamage = true;
            }
        }

        if (canDamage)
        {
            return NO_CONTACT;
        }
    }

    if (PhysicsNet::sbSweepTestEnabled)
    {
        if (PhysicsNet::IsAGoalPost(obj))
        {
            return NO_CONTACT;
        }
    }

    if (m_parentObject == NULL)
    {
        if (objID == 0x19 || objID == 0x5)
        {
            if (m_unk_0x58)
            {
                return NO_CONTACT;
            }

            GetPosition(&ballPosition);
            radius = GetRadius();

            if ((float)fabs(ballPosition.f.x) > cField::GetGoalLineX(1u) - 2.0f * radius)
            {
                if ((float)fabs(ballPosition.f.y) < cNet::m_fNetWidth / 2.0f - radius)
                {
                    float netHeight = cNet::m_fNetHeight;
                    if ((float)fabs(ballPosition.f.z) < netHeight - radius)
                    {
                        return NO_CONTACT;
                    }
                }
                m_unk_0x59 = true;
            }

            cBall* pBall = m_pAIBall;
            if (pBall->m_tNoPickupTimer.m_uPackedTime != 0)
            {
                if (pBall->m_pPassTarget != NULL || pBall->m_tShotTimer.m_uPackedTime != 0)
                {
                    return NO_CONTACT;
                }
            }

            dContact* pContact = info;
            hitWall = 0;
            for (i = 0; i < numContacts; i++)
            {
                if (pContact->geom.normal[2] < 0.08f)
                {
                    hitWall = 1;
                    break;
                }
                pContact++;
            }

            if (hitWall || objID == 0x5)
            {
                pBall->m_unk_0xA6 = false;
                pBall->mpDamageTarget = NULL;

                ballVelocity = GetLinearVelocity();
                float velY = ballVelocity.f.y;

                nlCartesianToPolar(aBallSpeed, ballVelocity.f.x, velY);

                if (aBallSpeed.r > 1.0f)
                {
                    Event* event = g_pEventManager->CreateValidEvent(0x20, 0x3C);
                    pEventData = new (&event->m_data) CollisionBallWallData();
                    pEventData->pBall = m_pAIBall;

                    bool bIsShot = m_pAIBall->m_tShotTimer.m_uPackedTime != 0 && m_pAIBall->m_unk_0xA4;

                    pEventData->bIsPerfect = bIsShot;

                    float speedSq = (ballVelocity.f.z * ballVelocity.f.z) + ((ballVelocity.f.x * ballVelocity.f.x) + (velY * velY));

                    u32 shotTimer = m_pAIBall->m_tShotTimer.m_uPackedTime;
                    pEventData->bIsShot = (shotTimer != 0);

                    float posY;
                    float posZ = info->geom.pos[2];
                    posY = info->geom.pos[1];
                    float posX = info->geom.pos[0];
                    pEventData->position.f.x = posX;
                    pEventData->position.f.y = posY;
                    pEventData->position.f.z = posZ;

                    float normalY;
                    float normalZ = info->geom.normal[2];
                    normalY = info->geom.normal[1];
                    float normalX = info->geom.normal[0];
                    pEventData->normal.f.x = normalX;
                    pEventData->normal.f.y = normalY;
                    pEventData->normal.f.z = normalZ;

                    pEventData->fCollisionVecLen = nlSqrt(speedSq, true);

                    ScaleAngularVelocity(0.9f);
                    m_pAIBall->ClearBallBlur();
                }

                if (aBallSpeed.r > sfMaxBallBounceSpeed)
                {
                    scale = sfMaxBallBounceSpeed / aBallSpeed.r;
                    vel.f.x = scale * ballVelocity.f.x;
                    vel.f.y = scale * velY;
                    vel.f.z = scale * ballVelocity.f.z;
                    SetLinearVelocity(vel);
                }
            }
        }
        else if (objID == 0x7)
        {
            if (!m_unk_0x58)
            {
                return NO_CONTACT;
            }

            if (PhysicsNet::IsAGoalWall(obj))
            {
                info->surface.soft_cfm = PhysicsNet::sfWallSoftness;
            }
        }
    }

    return PhysicsBall::Contact(obj, info, numContacts);
}

/**
 * Offset/Address/Size: 0x1200 | 0x80134C34 | size: 0x68
 */
PhysicsAIBall::PhysicsAIBall(float radius)
    : PhysicsBall(g_CollisionSpace, g_PhysicsWorld, radius)
{
    m_pAIBall = NULL;
    m_unk_0x50 = 9999;
    m_unk_0x58 = false;
    m_unk_0x44.f.x = 0.f;
    m_unk_0x44.f.y = 0.f;
    m_unk_0x44.f.z = 0.f;
}
