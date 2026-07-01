#include "Game/Render/NetMesh.h"

#include "Game/Camera/CameraMan.h"
#include "dolphin/types.h"
#include "NL/glx/glxTexture.h"
#include "NL/nlMemory.h"

#include "Game/Drawable/DrawableObj.h"

#include "Game/WorldManager.h"
#include "Game/Ball.h"

#include "Game/Physics/NetMeshModelLoader.h"
#include "Game/Physics/PhysicsAIBall.h"

#include "Game/Sys/eventman.h"

bool NetMesh::s_bAnimatedNetMeshEnabled = false;
bool NetMesh::s_bAlwaysActive = false;
float NetMesh::s_fReboundForceCoefficient = 6.0f;
float NetMesh::s_fVelocityDampingCoefficient = 0.7f;
float NetMesh::s_fBallRadiusExaggerationFactor = 2.0f;
float NetMesh::s_fBallRadiusExaggerationFactor2 = 4.0f;
float NetMesh::s_fNetGravityMagnitude = 10.0f;
float NetMesh::s_fLooseness = 0.08f;
int NetMesh::s_NumConstraintIterations = 1;
float NetMesh::s_fInactivityThreshold = 0.01f;
float NetMesh::s_fIsBallMovingThreshold = 0.01f;
float NetMesh::s_fDampening = 1.0f;
float NetMesh::s_fNetStretchLimit = 1.0f;
unsigned long NetMesh::sNetTextureHandle = 0;
bool NetMesh::s_bUseStretchLimit = false;
NetMesh* NetMesh::spPositiveXNetMesh = nullptr;
NetMesh* NetMesh::spNegativeXNetMesh = nullptr;
bool NetMesh::sbDontUseLowestNetTextureLOD = false;

extern f32 g_fFixedUpdateTick;

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };

/**
 * Offset/Address/Size: 0x1178 | 0x8012FF98 | size: 0x84
 */
NetMesh::NetMesh(bool positiveEnd)
{
    mbInitialized = false;
    mbFirstUpdate = true;
    m_v3Position = NULL;
    m_v3PrevPosition = NULL;
    m_v3Accel = NULL;
    m_TriStripIndices = NULL;
    m_v2TextureCoords = NULL;
    m_v3Normal = NULL;
    mNetMeshDrawableObjectID = 0;
    mbPositiveEnd = positiveEnd;
    m_NumParticles = 0;
    m_fBallPenetrationDepth = 0.0f;
    mfMinX = 0.0f;
    mfMaxX = 0.0f;
    mfMinY = 0.0f;
    mfMaxY = 0.0f;
    m_NumPositionConstraints = 0;
    m_NumDistanceConstraints = 0;
    mbIsActive = true;
    mbBallIsInsideNet = false;
    mfMotion = 0.0f;
    m_aDistanceConstraints = NULL;
    m_aPositionConstraints = NULL;

    if (positiveEnd != 0)
    {
        spPositiveXNetMesh = this;
    }
    if (positiveEnd == 0)
    {
        spNegativeXNetMesh = this;
    }
}

/**
 * Offset/Address/Size: 0xFE8 | 0x8012FE08 | size: 0x190
 */
void NetMesh::Allocate(int numParticles, int numDistanceConstraints, int numPositionConstraints)
{
    m_v3Position = (nlVector3*)nlMalloc(numParticles * sizeof(nlVector3), 8, false);
    m_v3PrevPosition = (nlVector3*)nlMalloc(numParticles * sizeof(nlVector3), 8, false);
    m_v3Accel = (nlVector3*)nlMalloc(numParticles * sizeof(nlVector3), 8, false);
    m_v3Normal = (nlVector3*)nlMalloc(numParticles * sizeof(nlVector3), 8, false);
    m_v2TextureCoords = (shortVector2*)nlMalloc(numParticles * sizeof(shortVector2), 8, false);
    m_bIsParticleFixed = (bool*)nlMalloc(numParticles, 8, false);
    m_aDistanceConstraints = (cDistanceConstraint*)nlMalloc(numDistanceConstraints * sizeof(cDistanceConstraint), 8, false);
    m_aPositionConstraints = (cPositionConstraint*)nlMalloc(numPositionConstraints * sizeof(cPositionConstraint), 8, false);

    for (int i = 0; i < numParticles; i++)
    {
        m_bIsParticleFixed[i] = false;
    }
}

/**
 * Offset/Address/Size: 0xF54 | 0x8012FD74 | size: 0x94
 */
NetMesh::~NetMesh()
{
    delete[] m_v3Position;
    delete[] m_v3PrevPosition;
    delete[] m_v3Accel;
    delete[] m_v3Normal;
    delete[] m_v2TextureCoords;
    delete[] m_TriStripIndices;
    delete[] m_aDistanceConstraints;
    delete[] m_aPositionConstraints;
    delete[] m_bIsParticleFixed;
}

/**
 * Offset/Address/Size: 0xEF4 | 0x8012FD14 | size: 0x60
 */
int NetMesh::SetPositionConstraint(int particleIndex, const nlVector3& v3Position)
{
    m_aPositionConstraints[m_NumPositionConstraints].nParticle = particleIndex;
    m_aPositionConstraints[m_NumPositionConstraints].v3Position = v3Position;
    m_NumPositionConstraints += 1;
    m_bIsParticleFixed[particleIndex] = true;
    return m_NumPositionConstraints - 1;
}

/**
 * Offset/Address/Size: 0xEAC | 0x8012FCCC | size: 0x48
 */
void NetMesh::SetDistanceConstraint(int nParticleA, int nParticleB, float fDistance)
{
    m_aDistanceConstraints[m_NumDistanceConstraints].nParticleA = nParticleA;
    m_aDistanceConstraints[m_NumDistanceConstraints].nParticleB = nParticleB;
    m_aDistanceConstraints[m_NumDistanceConstraints].fDistance = fDistance;
    m_NumDistanceConstraints += 1;
}

/**
 * Offset/Address/Size: 0xE40 | 0x8012FC60 | size: 0x6C
 */
void NetMesh::UpdateUntilRelaxed()
{
    mbIsActive = true;
    while (mbIsActive)
    {
        Update(g_fFixedUpdateTick, v3Zero, v3Zero, false, nullptr);
    }
}

inline static void AccumForces(NetMesh* self, nlVector3& newPos)
{
    nlVector3* upVector = &cCameraManager::m_UpVectorStack[cCameraManager::m_UpVectorStackSize];
    float gravityMagnitude = -NetMesh::s_fNetGravityMagnitude;

    newPos.f.x = gravityMagnitude * upVector->f.x;
    newPos.f.y = gravityMagnitude * upVector->f.y;
    newPos.f.z = gravityMagnitude * upVector->f.z;

    for (int i = 0; i < self->m_NumParticles; i++)
    {
        self->m_v3Accel[i] = newPos;
    }
}

inline static void Integrate(NetMesh* self, float t, nlVector3& v3Temp)
{
    for (int i = 0; i < self->m_NumParticles; i++)
    {
        nlVector3& v3Pos = self->m_v3Position[i];
        nlVector3& v3PrevPos = self->m_v3PrevPosition[i];
        nlVector3& v3Accel = self->m_v3Accel[i];

        v3Temp = v3Pos;

        v3Pos.f.x = v3Pos.f.x + ((NetMesh::s_fDampening * (v3Pos.f.x - v3PrevPos.f.x)) + (t * (v3Accel.f.x * t)));
        v3Pos.f.y = v3Pos.f.y + ((NetMesh::s_fDampening * (v3Pos.f.y - v3PrevPos.f.y)) + (t * (v3Accel.f.y * t)));
        v3Pos.f.z = v3Pos.f.z + ((NetMesh::s_fDampening * (v3Pos.f.z - v3PrevPos.f.z)) + (t * (v3Accel.f.z * t)));

        v3PrevPos = v3Temp;
    }
}

inline static void ComputeMotion(NetMesh* self)
{
    for (int i = 0; i < self->m_NumParticles; i++)
    {
        nlVector3& v3Pos = self->m_v3Position[i];
        nlVector3& v3PrevPos = self->m_v3PrevPosition[i];
        float motion = ((float)fabs(v3PrevPos.f.x - v3Pos.f.x) + (float)fabs(v3PrevPos.f.y - v3Pos.f.y)) + (float)fabs(v3PrevPos.f.z - v3Pos.f.z);

        if (motion > self->mfMotion)
        {
            self->mfMotion = motion;
        }
    }
}

inline static bool IsBallMoving(PhysicsSphere* sphere)
{
    nlVector3 vel;
    sphere->GetLinearVelocity(&vel);
    return ((vel.f.x * vel.f.x) + (vel.f.y * vel.f.y) + (vel.f.z * vel.f.z)) > NetMesh::s_fIsBallMovingThreshold;
}

/**
 * Offset/Address/Size: 0xAA8 | 0x8012F8C8 | size: 0x398
 */
void NetMesh::Update(float dt, const nlVector3& ballPosition, const nlVector3& ballPrevPosition, bool bExaggerateBallSize, PhysicsSphere* sphere)
{
    nlVector3 newPos;
    nlVector3 oldPos;

    if (mbIsActive || s_bAlwaysActive)
    {
        AddForcesToBall(ballPosition, sphere);

        AccumForces(this, newPos);

        mfMotion = 0.0f;

        Integrate(this, dt, oldPos);

        SatisfyConstraints(ballPosition, bExaggerateBallSize);

        ComputeMotion(this);
    }
    else
    {
        m_numAffectedParticles = 0;
    }

    if (!mbFirstUpdate)
    {
        if (PhysicsAIBall::IsBallOutsideNet(ballPosition))
        {
            mbBallIsInsideNet = false;
        }
        else
        {
            if (PhysicsAIBall::DidBallJustEnterNet(ballPrevPosition, ballPosition))
            {
                float x = ballPosition.f.x;
                if (((x > 0.0f) && mbPositiveEnd) || ((x < 0.0f) && !mbPositiveEnd))
                {
                    mbBallIsInsideNet = true;
                }
            }
        }

        if ((mfMotion > s_fInactivityThreshold)
            || (mbBallIsInsideNet
                && ((sphere == NULL)
                    || IsBallMoving(sphere)
                    || ((m_numAffectedParticles == 0) && mbIsActive))))
        {
            mbIsActive = true;
        }
        else
        {
            mbIsActive = false;
            m_fBallPenetrationDepth = 0.0f;
        }
    }

    mbFirstUpdate = false;
}

/**
 * Offset/Address/Size: 0xA60 | 0x8012F880 | size: 0x48
 */
void NetMesh::JoltNet(float zDisplacement)
{
    mJolt = zDisplacement;
    for (s32 i = 0; i < m_NumParticles; i++)
    {
        m_v3Position[i].f.z += zDisplacement;
    }

    mbIsActive = true;
    mbFirstUpdate = true;
}

/**
 * TODO: 99.82% match - remaining point stack-store and center-distance register swaps.
 */
void NetMesh::SatisfyConstraints(const nlVector3& ballPosition, bool bExaggerateBallSize)
{
    static float fDeltaZero = 0.001f;

    for (int j = 0; j < s_NumConstraintIterations; j++)
    {
        int i;
        for (i = 0; i < m_NumDistanceConstraints; i++)
        {
            cDistanceConstraint& c = m_aDistanceConstraints[i];
            nlVector3& x1 = m_v3Position[c.nParticleA];
            nlVector3& x2 = m_v3Position[c.nParticleB];

            nlVector3 d;
            nlVec3Sub(d, x1, x2);
            float dy = d.f.y;
            float dyy = dy * dy;
            float dx = d.f.x;
            float dz = d.f.z;

            float length = nlSqrt(dyy + (dx * dx) + (dz * dz), true);

            if ((float)fabs(length) > fDeltaZero)
            {
                float restLength = c.fDistance * (1.0f + s_fLooseness);
                float diff = (length - restLength) / length;
                float halfDiff = 0.5f * diff;

                x1.f.x -= dx * halfDiff;
                x1.f.y -= dy * halfDiff;
                x1.f.z -= dz * halfDiff;

                x2.f.x += dx * halfDiff;
                x2.f.y += dy * halfDiff;
                x2.f.z += dz * halfDiff;
            }
        }

        m_fBallPenetrationDepth = 0.0f;
        m_bPenetratingFixedParticle = false;

        if (mbBallIsInsideNet)
        {
            int numParticlesAffected = 0;
            int iClosestParticle;
            float closestParticleDistSq;

            for (i = 0; i < m_NumParticles; i++)
            {
                if (!m_bPenetratingFixedParticle || m_bIsParticleFixed[i])
                {
                    nlVector3& particlePosition = m_v3Position[i];
                    const nlVector3& particleNormal = m_v3Normal[i];

                    nlVector3 disp;
                    float radius = s_fBallRadiusExaggerationFactor * g_pBall->m_pPhysicsBall->GetRadius();

                    if (bExaggerateBallSize)
                    {
                        radius = s_fBallRadiusExaggerationFactor2 * g_pBall->m_pPhysicsBall->GetRadius();
                    }

                    nlVector3 d;
                    nlVec3Sub(d, ballPosition, particlePosition);
                    float dot = nlVec3DotProduct(d, particleNormal);
                    nlVector3 perp;
                    nlVec3ScaleAdd(perp, -dot, particleNormal, d);
                    float perpDistSq = nlVec3LengthSquared(perp);

                    if ((perpDistSq < closestParticleDistSq) || (i == 0))
                    {
                        closestParticleDistSq = perpDistSq;
                        iClosestParticle = i;
                    }

                    float radiusSq = radius * radius;
                    if (perpDistSq < (4.0f * radiusSq))
                    {
                        nlVector3 pointOnOutsideOfBall = ballPosition;
                        nlVec3ScaleAdd(pointOnOutsideOfBall, radius, particleNormal, pointOnOutsideOfBall);

                        nlVec3Sub(disp, pointOnOutsideOfBall, particlePosition);

                        numParticlesAffected++;

                        float falloffFactor = 1.0f;
                        if (perpDistSq > radiusSq)
                        {
                            float dist = nlSqrt(perpDistSq, false);
                            falloffFactor = 1.0f - ((dist - radius) / radius);
                        }

                        float penetration = nlVec3DotProduct(disp, particleNormal);

                        if (penetration > 0.0f)
                        {
                            if (penetration > m_fBallPenetrationDepth)
                            {
                                m_fBallPenetrationDepth = penetration;
                                m_v3BallPenetrationNormal = particleNormal;
                            }

                            float displacementMag = penetration * falloffFactor;
                            nlVec3ScaleAdd(particlePosition, displacementMag, particleNormal, particlePosition);
                        }
                    }
                }
            }

            m_iClosestParticle = iClosestParticle;
            m_numAffectedParticles = numParticlesAffected;

            if (mbBallIsInsideNet && m_numAffectedParticles == 0)
            {
                float centerY = 0.5f * (mfMinY + mfMaxY);
                float centerX = 0.5f * (mfMinX + mfMaxX);
                float dy = ballPosition.f.y - centerY;
                float dyy = dy * dy;
                float dx = ballPosition.f.x - centerX;
                float zero = 0.0f;
                float dz = ballPosition.f.z;
                dz -= zero;
                m_fBallPenetrationDepth = nlSqrt(dyy + (dx * dx) + (dz * dz), true);
            }
        }

        for (i = 0; i < m_NumPositionConstraints; i++)
        {
            cPositionConstraint& c = m_aPositionConstraints[i];
            nlVector3& x = m_v3Position[c.nParticle];
            float ty;
            float tz;
            float tx;

            tz = c.v3Position.f.z;
            ty = c.v3Position.f.y;
            tx = c.v3Position.f.x;

            x.f.x = tx;
            x.f.y = ty;
            x.f.z = tz;
        }

        {
            static float fGroundHeight = 0.01f;
            for (i = 0; i < m_NumParticles; i++)
            {
                if (m_v3Position[i].f.z < fGroundHeight)
                {
                    m_v3Position[i].f.z = fGroundHeight;
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x220 | 0x8012F040 | size: 0x330
 * TODO: 99.61% match - 1 SDA label difference (@885 vs @467 for 0.0f constant, linker artifact)
 */
void NetMesh::AddForcesToBall(const nlVector3& position, PhysicsSphere* sphere)
{
    class nlVector3 vel;                    // r1+0x2C
    float forceMagnitude;                   // f1
    class nlVector3 force;                  // r1+0x20
    class nlVector3 velocity;               // r1+0x14
    class nlVector3 currentAngularVelocity; // r1+0x8

    if (m_fBallPenetrationDepth > 0.0f)
    {
        if (s_bUseStretchLimit && sphere && (m_fBallPenetrationDepth > s_fNetStretchLimit))
        {
            nlVector3& temp_r3 = sphere->GetLinearVelocity();

            forceMagnitude = (m_v3BallPenetrationNormal.f.x * temp_r3.f.x)
                           + (m_v3BallPenetrationNormal.f.y * temp_r3.f.y)
                           + (m_v3BallPenetrationNormal.f.z * temp_r3.f.z);

            nlVec3Set(vel,
                forceMagnitude * m_v3BallPenetrationNormal.f.x,
                forceMagnitude * m_v3BallPenetrationNormal.f.y,
                forceMagnitude * m_v3BallPenetrationNormal.f.z);

            nlVector3& temp_r3_2 = sphere->GetLinearVelocity();
            nlVec3Set(vel,
                temp_r3_2.f.x - vel.f.x,
                temp_r3_2.f.y - vel.f.y,
                temp_r3_2.f.z - vel.f.z);

            sphere->SetLinearVelocity(vel);
        }

        float forceMagnitude = -(m_fBallPenetrationDepth * s_fReboundForceCoefficient);
        force = m_v3BallPenetrationNormal;
        nlVec3Scale(force, force, forceMagnitude);

        if (sphere)
        {
            if (m_bPenetratingFixedParticle)
            {
                sphere->SetLinearVelocity(v3Zero);
                sphere->SetAngularVelocity(v3Zero);
            }
            sphere->AddForceAtCentreOfMass(force);

            velocity = sphere->GetLinearVelocity();

            if (((velocity.f.x * m_v3BallPenetrationNormal.f.x)
                    + (velocity.f.y * m_v3BallPenetrationNormal.f.y)
                    + (velocity.f.z * m_v3BallPenetrationNormal.f.z))
                > 0.0f)
            {
                nlVec3Set(velocity,
                    s_fVelocityDampingCoefficient * velocity.f.x,
                    s_fVelocityDampingCoefficient * velocity.f.y,
                    s_fVelocityDampingCoefficient * velocity.f.z);
                sphere->SetLinearVelocity(velocity);
            }

            sphere->GetAngularVelocity(&currentAngularVelocity);

            nlVector3 crossProduct;
            nlVec3CrossProductAlt(crossProduct, velocity, m_v3BallPenetrationNormal);

            nlVector3 crossProductScaled;
            float invR = 1.0f / g_pBall->m_pPhysicsBall->GetRadius();
            nlVec3Scale(crossProductScaled, crossProduct, invR);

            // wtf? I mathematically dont understand why the order is reversed here.
            dBodyAddTorque(sphere->m_bodyID,
                0.1f * (crossProductScaled.f.z - currentAngularVelocity.f.x),
                0.1f * (crossProductScaled.f.y - currentAngularVelocity.f.y),
                0.1f * (crossProductScaled.f.x - currentAngularVelocity.f.z));
        }

        Event* pEvent = g_pEventManager->CreateValidEvent(0x32, 0x28);
        BallNetmeshEventData* eventData = new ((u8*)pEvent + 0x10) BallNetmeshEventData();

        eventData->netMesh = this;
        eventData->v3CollisionVelocity = g_pBall->m_v3Velocity;
    }
}

/**
 * Offset/Address/Size: 0xBC | 0x8012EEDC | size: 0x164
 */
void NetMesh::Initialize(unsigned long netMeshDrawableObjectID)
{
    mJolt = 0.0f;
    mNetMeshDrawableObjectID = netMeshDrawableObjectID;

    DrawableObject* dobj = WorldManager::s_World->FindDrawableObject(netMeshDrawableObjectID);
    dobj->m_uObjectFlags &= ~1UL;

    for (int i = 0; i < m_NumParticles; ++i)
    {
        m_bIsParticleFixed[i] = false;
    }

    {
        NetMeshModelLoader loader(*this, netMeshDrawableObjectID);
    }

    const float BIG_POS = 10000.0f;
    const float BIG_NEG = -10000.0f;
    mfMinX = BIG_POS;
    mfMinY = BIG_POS;
    mfMaxX = BIG_NEG;
    mfMaxY = BIG_NEG;

    for (int i = 0; i < m_NumParticles; i++)
    {
        nlVector3& p = m_v3Position[i];
        m_v3PrevPosition[i] = p;

        if (p.f.x > mfMaxX)
            mfMaxX = p.f.x;
        if (p.f.y > mfMaxY)
            mfMaxY = p.f.y;
        if (p.f.x < mfMinX)
            mfMinX = p.f.x;
        if (p.f.y < mfMinY)
            mfMinY = p.f.y;

        nlVec3Set(m_v3Accel[i], 0.0f, 0.0f, 0.0f);
    }

    mbInitialized = true;
}

/**
 * Offset/Address/Size: 0x54 | 0x8012EE74 | size: 0x68
 */
void NetMesh::SetTriStripIndices(int numIndices, const unsigned short* indices)
{
    m_NumTriStripIndices = numIndices;
    m_TriStripIndices = (u16*)nlMalloc(numIndices * 2, 8, false);
    memcpy(m_TriStripIndices, indices, numIndices * 2);
}

/**
 * Offset/Address/Size: 0x4C | 0x8012EE6C | size: 0x8
 */
void NetMesh::SetDontUseLowestNetTextureLOD(bool value)
{
    sbDontUseLowestNetTextureLOD = value;
}

/**
 * Offset/Address/Size: 0x0 | 0x8012EE20 | size: 0x4C
 */
void NetMesh::SetTexture(unsigned long texture)
{
    sNetTextureHandle = texture;
    if (sbDontUseLowestNetTextureLOD != 0)
    {
        PlatTexture* tex = glx_GetTex(texture, true, true);
        tex->m_MaxLevel = tex->m_Levels - 1;
        tex->Prepare();
    }
}
