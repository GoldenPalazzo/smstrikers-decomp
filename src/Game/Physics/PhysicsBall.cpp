#include "Game/Physics/PhysicsBall.h"
#include "Game/Physics/CollisionSpace.h"

#include "NL/nlFont.h"
#include "NL/nlMath.h"
#include "Game/Physics/PhysicsObject.h"
#include "Game/Physics/PhysicsFakeBall.h"

#include "Game/FixedUpdateTask.h"

#include "Game/Ball.h"

float g_BallFriction = 3.f;
float g_BallFrictionWall = 4.f;
float g_BallBounce = 0.25f;
float g_BallBounceGround = 0.375f;
float g_BallBounceWall = 0.45f;
float g_BallRollingResistance = 5.f;
float g_BallAirResistance = 0.1f;

/**
 * Offset/Address/Size: 0x0 | 0x80134D14 | size: 0xD4
 * TODO: 98.4% match - register allocation difference (f3/f9/f5/f6 vs f9/f5/f3/f4)
 */
void PhysicsBall::CalcAngularFromLinearVelocity(nlVector3& v3AngularVel)
{
    nlVector3 v3Velocity;
    GetLinearVelocity(&v3Velocity);

    nlVector3 v3Up = { 0.0f, 1.0f, 0.0f };
    f32 invRadius = 1.0f / GetRadius();
    nlVector3 v3Look = { 0.0f, 0.0f, 1.0f };

    v3Up.f.z = invRadius;
    v3Look.f.x = v3Velocity.f.x;
    v3Look.f.y = v3Velocity.f.y;

    v3AngularVel.f.x = v3Up.f.y * v3Look.f.z - v3Look.f.y * v3Up.f.z;
    v3AngularVel.f.y = -v3Up.f.x * v3Look.f.z + v3Up.f.z * v3Look.f.x;
    v3AngularVel.f.z = v3Look.f.y * v3Up.f.x - v3Up.f.y * v3Look.f.x;
}

/**
 * Offset/Address/Size: 0xD4 | 0x80134DE8 | size: 0x28
 */
void PhysicsBall::SetUseAngularVelocity(bool param_1)
{
    m_bUseAngularVel = 0;
    if (param_1)
    {
        m_fSpinTimer = 0.08f;
        return;
    }
    m_fSpinTimer = 0.f;
}

/**
 * Offset/Address/Size: 0xFC | 0x80134E10 | size: 0x80
 */
void PhysicsBall::ScaleAngularVelocity(float scale)
{
    nlVector3 v;
    if (m_bUseAngularVel != 0)
    {
        GetAngularVelocity(&v);
        _nlVec3Scale(v, scale);
        SetAngularVelocity(v);
    }
}

// const uint vec_zero[3] __attribute__((section(".rodata"))) = {0, 0, 0};
// const nlVector3 vec_zero __attribute__((section(".rodata"))) = nlVector3(0.f, 0.f, 0.f);

/**
 * Offset/Address/Size: 0x17C | 0x80134E90 | size: 0x51C
 */
void PhysicsBall::AddResistanceForces()
{
    nlVector3 v3Vel;
    nlVector3 v3Force;
    nlVector3 v3AngVel;
    nlVector3 v3CrossForce;
    nlVector3 v3LinVel2;
    nlVector3 v3LinVelM;
    nlVector3 v3MagnusForce;
    nlVector3 v3AngVelM;
    nlVector3 v3LinVel1;
    nlVector3 v3AngVel2;
    u8 bApply;

    v3Vel = GetLinearVelocity();
    if (m_parentObject == NULL)
    {
        if (m_bIsSupportedByGround != 0 && m_bUseAngularVel == 0)
        {
            f32 speed = nlSqrt(v3Vel.f.z * v3Vel.f.z + (v3Vel.f.x * v3Vel.f.x + v3Vel.f.y * v3Vel.f.y), true);
            if (speed > 0.01f)
            {
                f32 factor = -g_BallRollingResistance / speed;
                v3Force.f.x = factor * v3Vel.f.x;
                v3Force.f.y = factor * v3Vel.f.y;
                v3Force.f.z = factor * v3Vel.f.z;
                AddForceAtCentreOfMass(v3Force);
            }
        }
        f32 drag = -g_BallAirResistance;
        v3Force.f.x = drag * v3Vel.f.x;
        v3Force.f.y = drag * v3Vel.f.y;
        v3Force.f.z = drag * v3Vel.f.z;
        AddForceAtCentreOfMass(v3Force);
    }
    if (m_bUseTiltForce != 0 && g_pBall->m_pPassTarget == NULL)
    {
        bApply = 0;
        if (g_pBall->m_tShotTimer.m_uPackedTime != 0 && g_pBall->mbCanDamage != 0)
            bApply = 1;
        if (bApply == 0)
            AddForceAtCentreOfMass(m_v3TiltForce);
    }
    if (m_fSpinTimer > 0.f)
    {
        m_fSpinTimer = m_fSpinTimer - FixedUpdateTask::GetPhysicsUpdateTick();
        if (m_fSpinTimer <= 0.f)
            m_bUseAngularVel = 1;
    }
    if (m_parentObject == NULL && m_bUseAngularVel != 0)
    {
        f32 threshold = 0.02f + GetRadius();
        if (GetPosition().f.z < threshold)
        {
            GetLinearVelocity(&v3LinVel1);
            nlVector3 localNorm = { 0.f, 0.f, 0.f };
            f32 invR = 1.0f / GetRadius();
            nlVector3 localGndVel = { 0.f, 0.f, 0.f };
            localNorm.f.z = invR;
            localGndVel.f.x = v3LinVel1.f.x;
            localGndVel.f.y = v3LinVel1.f.y;
            f32 crossX = localNorm.f.y * localGndVel.f.z - localNorm.f.z * localGndVel.f.y;
            f32 crossY = localNorm.f.z * localGndVel.f.x - localNorm.f.x * localGndVel.f.z;
            f32 crossZ = localNorm.f.x * localGndVel.f.y - localNorm.f.y * localGndVel.f.x;
            GetAngularVelocity(&v3AngVel);
            f32 torqueX = 0.25f * (crossX - v3AngVel.f.x);
            f32 torqueY = 0.25f * (crossY - v3AngVel.f.y);
            f32 torqueZ = 0.25f * (crossZ - v3AngVel.f.z);
            dBodyAddTorque(m_bodyID, torqueX, torqueY, torqueZ);
            GetAngularVelocity(&v3AngVel2);
            v3AngVel2.f.z = 0.f;
            nlVector3 localRad = { 0.f, 0.f, 0.f };
            localRad.f.z = GetRadius();
            v3CrossForce.f.x = v3AngVel2.f.y * localRad.f.z - v3AngVel2.f.z * localRad.f.y;
            v3CrossForce.f.y = v3AngVel2.f.z * localRad.f.x - v3AngVel2.f.x * localRad.f.z;
            v3CrossForce.f.z = v3AngVel2.f.x * localRad.f.y - v3AngVel2.f.y * localRad.f.x;
            GetLinearVelocity(&v3LinVel2);
            v3CrossForce.f.x = 5.f * (v3CrossForce.f.x - v3LinVel2.f.x);
            v3CrossForce.f.y = 5.f * (v3CrossForce.f.y - v3LinVel2.f.y);
            v3CrossForce.f.z = 5.f * (v3CrossForce.f.z - v3LinVel2.f.z);
            AddForceAtCentreOfMass(v3CrossForce);
            v3CrossForce.f.z = 0.f;
            if (torqueX * torqueX + torqueZ * torqueZ + torqueY * torqueY < 0.0001f
                && v3CrossForce.f.x * v3CrossForce.f.x + v3CrossForce.f.y * v3CrossForce.f.y + v3CrossForce.f.z * v3CrossForce.f.z < 0.00003f)
                m_bUseAngularVel = 0;
        }
    }
    if (m_parentObject == NULL && m_bUseMagnusEffect != 0)
    {
        f32 threshold = 0.02f + GetRadius();
        nlVector3& pos = GetPosition();
        if (threshold < pos.f.z)
        {
            GetLinearVelocity(&v3LinVelM);
            if (v3LinVelM.f.x * v3LinVelM.f.x + v3LinVelM.f.z * v3LinVelM.f.z + v3LinVelM.f.y * v3LinVelM.f.y > 1.f)
            {
                GetAngularVelocity(&v3AngVelM);
                if (v3AngVelM.f.x * v3AngVelM.f.x + v3AngVelM.f.z * v3AngVelM.f.z + v3AngVelM.f.y * v3AngVelM.f.y > 1.f)
                {
                    v3MagnusForce.f.x = (v3AngVelM.f.z * v3LinVelM.f.y - v3AngVelM.f.y * v3LinVelM.f.z) * 0.04f;
                    v3MagnusForce.f.z = (v3AngVelM.f.y * v3LinVelM.f.x - v3AngVelM.f.x * v3LinVelM.f.y) * 0.075f;
                    v3MagnusForce.f.y = (-v3AngVelM.f.z * v3LinVelM.f.x + v3AngVelM.f.x * v3LinVelM.f.z) * 0.075f;
                    AddForceAtCentreOfMass(v3MagnusForce);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x698 | 0x801353AC | size: 0x250
 */
ContactType PhysicsBall::Contact(PhysicsObject* other, dContact* contact, int param)
{
    nlVector3 pos;
    nlVector3 _pos;

    f32 temp_f0;
    f32 temp_f1;
    f32 temp_f1_2;
    f32 temp_f2;
    f32 temp_f29;
    f32 temp_f30;
    f32 temp_f31;
    s32 objType;
    s32 i;
    dContact* c;

    objType = other->GetObjectType();
    GetPosition(&pos);

    if (objType == 0x11)
    {
        c = contact;
        for (i = 0; i < param; i++)
        {
            if ((c->geom.pos[2] <= pos.f.z) && (c->geom.normal[2] > 0.9f))
            {
                m_bIsSupportedByGround = 1;
                break;
            }
            c++;
        }
    }

    if (m_parentObject != NULL)
    {
        if (objType == 0x11)
        {
            GetPosition(&pos);
            if ((contact->geom.normal[2] > 0.f) && ((contact->geom.pos[2] + GetRadius()) < pos.f.z))
            {
                _pos = GetPosition();

                temp_f2 = contact->geom.normal[2];
                temp_f1 = contact->geom.depth;
                temp_f31 = temp_f2 * temp_f1;
                _pos.f.z += temp_f31;
                SetPosition(_pos, WORLD_COORDINATES);

                if (contact->geom.normal[2] > 0.95f)
                {
                    return NO_CONTACT;
                }

                temp_f30 = contact->geom.normal[1];
                temp_f29 = contact->geom.normal[0];
                temp_f1 = 0.f;
                temp_f0 = temp_f30 * temp_f30;
                temp_f0 = (temp_f29 * temp_f29) + temp_f0;
                temp_f1 = temp_f1 + temp_f0;
                temp_f1_2 = nlRecipSqrt(temp_f1, true);
                contact->geom.normal[0] = (f32)(temp_f1_2 * temp_f29);
                contact->geom.normal[1] = (f32)(temp_f1_2 * temp_f30);
                temp_f0 = 0.f;
                contact->geom.normal[2] = (f32)(temp_f1_2 * temp_f0);
                contact->geom.depth = (f32)(contact->geom.depth - temp_f31);
            }
        }
        return m_parentObject->Contact(other, contact, param);
    }

    if ((objType != 0x11) && (objType != 0xD) && (objType != 0xE) && (objType != 8))
    {
        cBall* ball;

        m_bUseMagnusEffect = 0;
        FakeBallWorld::InvalidateBallCache();
        g_pBall->m_bBallPathChangeCount = g_pBall->m_bBallPathChangeCount + 1;
        g_pBall->m_bBallDeflectCount = g_pBall->m_bBallDeflectCount + 1;

        ball = g_pBall;
        ball->m_unk_0xA6 = 0;
        ball->mpDamageTarget = NULL;
    }

    return TWO_WAY_CONTACT;
}

/**
 * Offset/Address/Size: 0x8E8 | 0x801355FC | size: 0x78
 */
void PhysicsBall::CloneBall(const PhysicsBall& other)
{
    CloneObject(other);

    u32* src = (u32*)&other.m_v3TiltForce;
    u32* dst = (u32*)&m_v3TiltForce;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];

    m_bUseTiltForce = other.m_bUseTiltForce;
    m_bIsSupportedByGround = other.m_bIsSupportedByGround;
    m_bUseAngularVel = other.m_bUseAngularVel;
    m_bUseMagnusEffect = other.m_bUseMagnusEffect;
    m_fSpinTimer = other.m_fSpinTimer;
}

/**
 * Offset/Address/Size: 0x960 | 0x80135674 | size: 0x164
 */
void PhysicsBall::PostUpdate()
{
    nlVector3 linVel;
    nlVector3 pos;

    PhysicsObject::PostUpdate();
    GetLinearVelocity(&linVel);

    float l = (linVel.f.x * linVel.f.x) + (linVel.f.y * linVel.f.y) + (linVel.f.z * linVel.f.z);
    if (l > 2500.f)
    {
        const f32 f = 50.f / nlSqrt(l, true);
        nlVec3Set(linVel, f * linVel.f.x, f * linVel.f.y, f * linVel.f.z);
        SetLinearVelocity(linVel);
    }

    if ((GetPosition().f.z > 20.f) && (linVel.f.z > 0.f))
    {
        linVel.f.z *= 0.9f;
        SetLinearVelocity(linVel);
    }

    if (GetPosition().f.z < GetRadius())
    {
        m_bIsSupportedByGround = 1;
        GetPosition(&pos);
        pos.f.z = GetRadius();
        SetPosition(pos, WORLD_COORDINATES);

        linVel.f.z = linVel.f.z * -g_BallBounceGround;
        SetLinearVelocity(linVel);
    }
}

/**
 * Offset/Address/Size: 0xAC4 | 0x801357D8 | size: 0xAC
 */
void PhysicsBall::PreUpdate()
{
    nlVector3 vec;
    GetLinearVelocity(&vec);

    float l = (vec.f.x * vec.f.x) + (vec.f.y * vec.f.y) + (vec.f.z * vec.f.z);
    if (l > 2500.f)
    {
        float n = 50.f / nlSqrt(l, true);
        nlVec3Set(vec, n * vec.f.x, n * vec.f.y, n * vec.f.z);
        SetLinearVelocity(vec);
    }
    PhysicsObject::PreUpdate();
    AddResistanceForces();
}

/**
 * Offset/Address/Size: 0xB70 | 0x80135884 | size: 0xC
 */
void PhysicsBall::PreCollide()
{
    m_bIsSupportedByGround = false;
    // return (int)m_bIsSupportedByGround;
}

/**
 * Offset/Address/Size: 0xB7C | 0x80135890 | size: 0x114
 */
bool PhysicsBall::SetContactInfo(dContact* contact, PhysicsObject* other, bool param)
{
    if (m_parentObject != NULL)
    {
        return m_parentObject->SetContactInfo(contact, other, param);
    }

    if (param != 0)
    {
        SetDefaultContactInfo(contact);
    }

    if (other->GetObjectType() != 8)
    {
        if (other->GetObjectType() == 0x11)
        {
            contact->surface.bounce = (f32)g_BallBounceGround;
        }
        else if (other->GetObjectType() == 0x19)
        {
            contact->surface.bounce = (f32)g_BallBounceWall;
        }
        else
        {
            contact->surface.bounce = (f32)g_BallBounce;
        }

        contact->surface.bounce_vel = 0.f;
        if (other->GetObjectType() == 0x19)
        {
            contact->surface.mu = (f32)g_BallFrictionWall;
        }
        else
        {
            contact->surface.mu = (f32)g_BallFriction;
        }
    }

    return true;
}

/**
 * Offset/Address/Size: 0xC90 | 0x801359A4 | size: 0x8
 */
float PhysicsBall::GetBallMaxVelocity()
{
    return 50.0;
}

/**
 * Offset/Address/Size: 0xC98 | 0x801359AC | size: 0x88
 */
PhysicsBall::PhysicsBall(CollisionSpace* space, PhysicsWorld* world, float radius)
    : PhysicsSphere(space, world, radius)
{
    m_bUseTiltForce = 0;
    m_bIsSupportedByGround = 0;
    m_bUseAngularVel = 0;
    m_bUseMagnusEffect = 0;
    m_fSpinTimer = 0.f;

    SetCategory(0x20);
    SetCollide(0xaf);

    m_gravity = -14.f;

    float temp = 0.f;
    m_v3TiltForce.f.x = temp;
    m_v3TiltForce.f.y = temp;
    m_v3TiltForce.f.z = temp;
}
