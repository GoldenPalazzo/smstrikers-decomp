#include "Game/Effects/ParticleSystem.h"
#include "Game/Sys/debug.h"
#include "NL/nlMemory.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "types.h"

int ParticleSystem::m_nNumViews = 0;
int ParticleSystem::m_NumInstances = 0;
eGLView ParticleSystem::m_eViews[8];

efList freeParticles;

void* textureFrames[36] = { nullptr };
Particle* particleMemory;
int MaxNumParticles;

/**
 * Offset/Address/Size: 0x26F0 | 0x801F7848 | size: 0x90
 */
ParticleSystem::ParticleSystem(EffectsTemplate* pTemplate, EffectsSpec* pSpec)
{
    m_nextNode = nullptr;
    m_prevNode = nullptr;

    m_Particles.m_headNode = nullptr;
    m_Particles.m_tailNode = nullptr;
    m_Particles.m_numNodes = 0;

    m_NumInstances++;

    m_Mirror.f.x = 1.0f;
    m_Mirror.f.y = 1.0f;
    m_Mirror.f.z = 1.0f;

    m_pTemplate = pTemplate;
    m_pSpec = pSpec;

    m_fElapsedTime = 0.0f;
    m_fNumParticlesToCreate = 0.0f;
    m_fDelay = 0.0f;
    m_uLayer = 0;

    m_vVelocity.f.x = 0.0f;
    m_vVelocity.f.y = 0.0f;
    m_vVelocity.f.z = 0.0f;

    m_vPosition.f.x = 0.0f;
    m_vPosition.f.y = 0.0f;
    m_vPosition.f.z = 0.0f;

    m_vForward.f.x = 0.0f;
    m_vForward.f.y = 1.0f;
    m_vForward.f.z = 0.0f;

    m_vSourcePosition.f.x = 0.0f;
    m_vSourcePosition.f.y = 0.0f;
    m_vSourcePosition.f.z = 0.0f;

    m_aFacing = 0;
    m_bAmDying = false;
    m_bVisible = false;
}

/**
 * Offset/Address/Size: 0x2658 | 0x801F77B0 | size: 0x98
 */
ParticleSystem::~ParticleSystem()
{
    m_NumInstances = m_NumInstances - 1;
    if (m_Particles.m_headNode != nullptr)
    {
        while (m_Particles.m_headNode != nullptr)
        {
            freeParticles.Append(m_Particles.Remove());
        }
    }
}

/**
 * Offset/Address/Size: 0x264C | 0x801F77A4 | size: 0xC
 */
void ParticleSystem::ClearViews()
{
    m_nNumViews = 0;
}

/**
 * Offset/Address/Size: 0x262C | 0x801F7784 | size: 0x20
 */
void ParticleSystem::AddView(eGLView view)
{
    int numViews = m_nNumViews;
    m_nNumViews = numViews + 1;
    m_eViews[numViews] = view;
}

/**
 * Offset/Address/Size: 0x2628 | 0x801F7780 | size: 0x4
 */
void ParticleSystem::UpdateCoordSys()
{
}

/**
 * Offset/Address/Size: 0x2404 | 0x801F755C | size: 0x224
 * TODO: 89.9% match - post-nlRecipSqrt forward/mirror load ordering still differs
 * (target loads Y then X; current emits X then Y), cascading into register-allocation
 * differences in cross-product/normalization blocks.
 */
void ParticleSystem::UpdateCoordSys(nlMatrix4& mCoordSys)
{
    float rightX, rightY, rightZ;
    float gravY, gravX, gravZ;
    float negGravX;
    float upX;

    float rsqrt = nlRecipSqrt(
        m_vForward.f.x * m_vForward.f.x + m_vForward.f.y * m_vForward.f.y + m_vForward.f.z * m_vForward.f.z, true);

    gravY = rsqrt * m_vForward.f.y;
    gravX = rsqrt * m_vForward.f.x;
    gravZ = rsqrt * m_vForward.f.z;
    gravY *= m_Mirror.f.y;
    gravX *= m_Mirror.f.x;
    gravZ *= m_Mirror.f.z;

    float refX = 0.0f;
    float refZ = 1.0f;
    float refY = refX;

    float dot = refX * gravY + refX * gravX + refZ * gravZ;
    if ((float)__fabs(dot) > 0.999f)
    {
        refY = refZ;
        refZ = refX;
    }

    negGravX = -gravX;

    rightX = gravY * refZ - gravZ * refY;
    rightY = negGravX * refZ + gravZ * refX;
    rightZ = gravX * refY - gravY * refX;

    rsqrt = nlRecipSqrt(
        rightX * rightX + rightY * rightY + rightZ * rightZ, true);
    rightZ *= rsqrt;
    rightX = rsqrt * rightX;
    rightY *= rsqrt;

    float upY = rightZ * gravX - rightX * gravZ;
    upX = rightY * gravZ - rightZ * gravY;
    float upZ = rightX * gravY - rightY * gravX;

    rsqrt = nlRecipSqrt(
        upY * upY + upX * upX + upZ * upZ, true);

    mCoordSys.e[0] = rightX;
    mCoordSys.e[1] = rightY;
    mCoordSys.e[2] = rightZ;
    mCoordSys.e[4] = rsqrt * upX;
    mCoordSys.e[5] = rsqrt * upY;
    mCoordSys.e[6] = rsqrt * upZ;
    mCoordSys.e[8] = negGravX;
    mCoordSys.e[9] = -gravY;
    mCoordSys.e[10] = -gravZ;
    mCoordSys.e[12] = m_vPosition.f.x;
    mCoordSys.e[13] = m_vPosition.f.y;
    mCoordSys.e[14] = m_vPosition.f.z;
    mCoordSys.e[15] = 1.0f;
    mCoordSys.e[11] = 0.0f;
    mCoordSys.e[7] = 0.0f;
    mCoordSys.e[3] = 0.0f;
}

/**
 * Offset/Address/Size: 0x22F4 | 0x801F744C | size: 0x110
 */
void EmitCircularPosition(nlVector3& vPosition, nlVector3& vDirection, EffectsTemplate* pTemplate, EffectsSpec* pSpec, const nlMatrix4& mLocalToWorld)
{
    float randomAngle = RandomizedValue(0.0f, 6.2831855f);

    float sinVal;
    float cosVal;
    nlSinCos(&sinVal, &cosVal, (unsigned short)(int)(10430.378f * randomAngle));

    float radius = RandomizedValue(pTemplate->m_rRadius.base, pTemplate->m_rRadius.range);

    nlVector3 localPos;
    localPos.f.x = cosVal * radius;
    localPos.f.y = -sinVal * radius;
    localPos.f.z = 0.0f;

    if (pSpec != nullptr)
    {
        nlVec3Set(localPos,
            localPos.f.x + pSpec->m_vLocalOffset.f.x,
            localPos.f.y + pSpec->m_vLocalOffset.f.y,
            localPos.f.z + pSpec->m_vLocalOffset.f.z);
    }

    if (pTemplate->m_bLocalSpace)
    {
        vPosition = localPos;
    }
    else
    {
        nlMultPosVectorMatrix(vPosition, localPos, mLocalToWorld);
    }
}

/**
 * Offset/Address/Size: 0x2168 | 0x801F72C0 | size: 0x18C
 */
void EmitSphericalPosition(nlVector3& vPosition, nlVector3& vDirection, EffectsTemplate* pTemplate, EffectsSpec* pSpec, const nlMatrix4& mLocalToWorld)
{
    float randomZ = RandomizedValue(0.0f, 2.0f);
    float randomAngleValue = RandomizedValue(65536.0f);

    float xyRadius = nlSqrt(1.0f - (randomZ * randomZ), true);

    float sinVal;
    float cosVal;
    nlSinCos(&sinVal, &cosVal, (unsigned short)(int)(10430.378f * randomAngleValue));

    nlVector3 localPos;
    nlVector3 localDir;

    float x = xyRadius * cosVal;
    float y = xyRadius * sinVal;
    float z = randomZ;

    float radius = RandomizedValue(pTemplate->m_rRadius.base, pTemplate->m_rRadius.range);

    nlVec3Set(localDir, x, y, z);
    nlVec3Set(localPos, radius * localDir.f.x, radius * localDir.f.y, radius * localDir.f.z);

    if (pSpec != nullptr)
    {
        nlVec3Set(localPos,
            localPos.f.x + pSpec->m_vLocalOffset.f.x,
            localPos.f.y + pSpec->m_vLocalOffset.f.y,
            localPos.f.z + pSpec->m_vLocalOffset.f.z);
    }

    if (pTemplate->m_bLocalSpace)
    {
        vPosition = localPos;
        vDirection = localDir;
    }
    else
    {
        nlMultPosVectorMatrix(vPosition, localPos, mLocalToWorld);
        nlMultDirVectorMatrix(vDirection, localDir, mLocalToWorld);
    }
}

/**
 * Offset/Address/Size: 0x1FDC | 0x801F7134 | size: 0x18C
 */
void EmitHemisphericalPosition(nlVector3& vPosition, nlVector3& vDirection, EffectsTemplate* pTemplate, EffectsSpec* pSpec, const nlMatrix4& mLocalToWorld)
{
    float randomZ = RandomizedValue(-0.5f, 1.0f);
    float randomAngleValue = RandomizedValue(6.2831855f);

    float xyRadius = nlSqrt(1.0f - (randomZ * randomZ), true);

    float sinVal;
    float cosVal;
    nlSinCos(&sinVal, &cosVal, (unsigned short)(int)(10430.378f * randomAngleValue));

    nlVector3 localPos;
    nlVector3 localDir;

    float x = xyRadius * cosVal;
    float y = xyRadius * sinVal;
    float z = randomZ;

    float radius = RandomizedValue(pTemplate->m_rRadius.base, pTemplate->m_rRadius.range);

    nlVec3Set(localDir, x, y, z);
    nlVec3Set(localPos, radius * localDir.f.x, radius * localDir.f.y, radius * localDir.f.z);

    if (pSpec != nullptr)
    {
        nlVec3Set(localPos,
            localPos.f.x + pSpec->m_vLocalOffset.f.x,
            localPos.f.y + pSpec->m_vLocalOffset.f.y,
            localPos.f.z + pSpec->m_vLocalOffset.f.z);
    }

    if (pTemplate->m_bLocalSpace)
    {
        vPosition = localPos;
        vDirection = localDir;
    }
    else
    {
        nlMultPosVectorMatrix(vPosition, localPos, mLocalToWorld);
        nlMultDirVectorMatrix(vDirection, localDir, mLocalToWorld);
    }
}

/**
 * Offset/Address/Size: 0x1C90 | 0x801F6DE8 | size: 0x34C
 * TODO: 94.83% match - register allocation diffs in tilt rotation and
 * hackyFacingAngle blocks due to -inline deferred flag mismatch on decomp.me
 */
void EmitSpindularPosition(nlVector3& vPosition, nlVector3& vDirection, EffectsTemplate* pTemplate, EffectsSpec* pSpec, const nlMatrix4& mLocalToWorld)
{
    nlVector3 localPos;
    nlVector3 localDir;
    float sin;
    float cos;
    float randomAngle = RandomizedValue(0.0f, 6.2831855f);

    nlSinCos(&sin, &cos, (unsigned short)(int)(10430.378f * randomAngle));

    float radius = RandomizedValue(pTemplate->m_rRadius.base, pTemplate->m_rRadius.range);

    localPos.f.x = cos * radius;
    localPos.f.y = -sin * radius;
    localPos.f.z = 0.0f;

    float tilt = RandomizedValue(pTemplate->m_rAngle.base, pTemplate->m_rAngle.range);
    if (tilt <= -1.0f)
    {
        tilt = -1.0f;
    }
    else if (tilt >= 1.0f)
    {
        tilt = 1.0f;
    }

    localDir.f.z = nlTan((unsigned short)(((int)(-tilt * 65536.0f)) / 360));
    localDir.f.x = cos;
    localDir.f.y = -sin;

    float length = nlRecipSqrt((localDir.f.x * localDir.f.x) + (localDir.f.y * localDir.f.y) + (localDir.f.z * localDir.f.z), false);

    nlVec3Set(localDir,
        length * localDir.f.x,
        length * localDir.f.y,
        length * localDir.f.z);

    float tiltRotation = RandomizedValue(pTemplate->m_rTilt.base, pTemplate->m_rTilt.range);
    tiltRotation = -tiltRotation * 3.14159265f / 180.0f;

    if (tiltRotation != 0.0f)
    {
        nlSinCos(&sin, &cos, (unsigned short)(int)(10430.378f * tiltRotation));

        float dirX = localDir.f.x;
        float dirZ = localDir.f.z;
        float posX = localPos.f.x;
        float posY = localPos.f.y;
        float posZ = localPos.f.z;

        localDir.f.z = (-dirX * sin) + (dirZ * cos);
        localDir.f.x = (dirX * cos) + (dirZ * sin);

        localPos.f.x = (posX * cos) + (posZ * sin);
        localPos.f.y = posY;
        localPos.f.z = (-posX * sin) + (posZ * cos);
    }

    if (pSpec != nullptr)
    {
        nlVec3Set(localPos,
            localPos.f.x + pSpec->m_vLocalOffset.f.x,
            localPos.f.y + pSpec->m_vLocalOffset.f.y,
            localPos.f.z + pSpec->m_vLocalOffset.f.z);
    }

    if (pTemplate->m_bLocalSpace)
    {
        vPosition = localPos;
        vDirection = localDir;
    }
    else
    {
        nlMultDirVectorMatrix(vPosition, localPos, mLocalToWorld);
        nlMultDirVectorMatrix(vDirection, localDir, mLocalToWorld);

        extern unsigned short hackyFacingAngle;
        if (hackyFacingAngle != 0)
        {
            nlSinCos(&sin, &cos, hackyFacingAngle);

            float dirY = vDirection.f.y;
            float dirX = vDirection.f.x;
            float newDirY = (dirX * sin) + (dirY * cos);
            float newDirX = (dirX * cos) + (-dirY * sin);
            vDirection.f.x = newDirX;
            vDirection.f.y = newDirY;

            float posY = vPosition.f.y;
            float posX = vPosition.f.x;
            float newPosY = (posX * sin) + (posY * cos);
            float newPosX = (posX * cos) + (-posY * sin);
            vPosition.f.x = newPosX;
            vPosition.f.y = newPosY;
        }

        nlVec3Set(vPosition,
            vPosition.f.x + mLocalToWorld.m[3][0],
            vPosition.f.y + mLocalToWorld.m[3][1],
            vPosition.f.z + mLocalToWorld.m[3][2]);
    }
}

/**
 * Offset/Address/Size: 0x1900 | 0x801F6A58 | size: 0x390
 * TODO: 96.50% match - remaining diffs are register-allocation only
 * (persistent this/numParticles/emitter regs and float temp register mapping).
 */
void ParticleSystem::CreateNewParticles(int numParticles)
{
    ParticleSystem* pSys = this;
    int count = numParticles;

    void (*emit)(nlVector3&, nlVector3&, EffectsTemplate*, EffectsSpec*, const nlMatrix4&);
    float oneOverLife;
    nlVector3 baseDir;
    nlVector3 dir;
    int i;
    Particle* pPart;
    nlMatrix4 mCoordSys;

    pSys->UpdateCoordSys(mCoordSys);

    if (pSys->m_pTemplate->m_bLocalSpace)
    {
        baseDir.f.x = 0.0f;
        baseDir.f.y = 0.0f;
        baseDir.f.z = 1.0f;
    }
    else
    {
        baseDir = pSys->m_vForward;
    }

    switch (pSys->m_pTemplate->m_eEmitter)
    {
    case Emitter_Circle:
        emit = EmitCircularPosition;
        break;
    case Emitter_Sphere:
        emit = EmitSphericalPosition;
        break;
    case Emitter_Spindle:
    {
        extern unsigned short hackyFacingAngle;
        hackyFacingAngle = pSys->m_aFacing;
        emit = EmitSpindularPosition;
        break;
    }
    case Emitter_Hemisphere:
        emit = EmitHemisphericalPosition;
        break;
    default:
        emit = 0;
        break;
    }

    for (i = 0; i < count; i++)
    {
        if (freeParticles.m_headNode == 0)
        {
            pPart = 0;
        }
        else
        {
            pPart = (Particle*)freeParticles.Remove();
        }

        if (pPart == 0)
        {
            break;
        }

        memset(pPart, 0, sizeof(Particle));
        pSys->m_Particles.Insert(pPart);

        dir = baseDir;
        emit(pPart->position, dir, pSys->m_pTemplate, pSys->m_pSpec, mCoordSys);

        pPart->position.f.x += pSys->m_vSourcePosition.f.x;
        pPart->position.f.y += pSys->m_vSourcePosition.f.y;
        pPart->position.f.z += pSys->m_vSourcePosition.f.z;

        pPart->lifeSpan = RandomizedValue(pSys->m_pTemplate->m_rParticleLife.base, pSys->m_pTemplate->m_rParticleLife.range);
        oneOverLife = 1.0f / pPart->lifeSpan;

        pPart->dRot = RandomizedValue(pSys->m_pTemplate->m_rRotation.base, pSys->m_pTemplate->m_rRotation.range);
        if (pPart->dRot == 0.0f)
        {
            pPart->rot = 0.0f;
        }
        else
        {
            pPart->rot = RandomizedValue(0.0f, 360.0f);
        }

        pPart->mass = RandomizedValue(pSys->m_pTemplate->m_rMass.base, pSys->m_pTemplate->m_rMass.range);

        EffectsTemplate* pTemplate = pSys->m_pTemplate;
        float sizeBegin = RandomizedValue(pTemplate->m_rSizeBegin.base, pTemplate->m_rSizeBegin.range);
        float sizeEnd = RandomizedValue(pTemplate->m_rSizeEnd.base, pTemplate->m_rSizeEnd.range);

        pPart->size = sizeBegin;
        pPart->dSize = oneOverLife * (sizeEnd - sizeBegin);

        float inheritVelocity = RandomizedValue(pSys->m_pTemplate->m_rInheritVelocity.base, pSys->m_pTemplate->m_rInheritVelocity.range);
        float velZ = inheritVelocity * pSys->m_vVelocity.f.z;
        float velY = inheritVelocity * pSys->m_vVelocity.f.y;
        float velX = inheritVelocity * pSys->m_vVelocity.f.x;

        float vel = RandomizedValue(pSys->m_pTemplate->m_rVelocity.base, pSys->m_pTemplate->m_rVelocity.range);

        float velDirY = vel * dir.f.y + velY;
        float velDirX = vel * dir.f.x + velX;
        float velDirZ = vel * dir.f.z + velZ;
        float speedSquared = (velDirY * velDirY) + (velDirX * velDirX) + (velDirZ * velDirZ);

        pPart->velocity = nlSqrt(speedSquared, true);

        if (pPart->velocity == 0.0f)
        {
            pPart->velDir.f.x = 0.0f;
            pPart->velDir.f.y = 0.0f;
            pPart->velDir.f.z = 0.0f;
        }
        else
        {
            float invSpeed = nlRecipSqrt(speedSquared, true);
            pPart->velDir.f.x = invSpeed * velDirX;
            pPart->velDir.f.y = invSpeed * velDirY;
            pPart->velDir.f.z = invSpeed * velDirZ;
        }

        pPart->acceleration = RandomizedValue(pSys->m_pTemplate->m_rAcceleration.base, pSys->m_pTemplate->m_rAcceleration.range);
        pPart->frame = 0.0f;
        pPart->FPS = RandomizedValue(pSys->m_pTemplate->m_rFPS.base, pSys->m_pTemplate->m_rFPS.range);
    }
}

/**
 * Offset/Address/Size: 0x15AC | 0x801F6704 | size: 0x354
 * TODO: 98.59% match - register name diffs only: f4/f5 swap for posZ/posY at b4/b8/cc/d4,
 * r9/r6 swap in modulo at 19c/1a0, snS2 in f3 vs f5 cascading through billboard section.
 * All instructions and offsets correct, same code size.
 */
void ParticleSystem::UpdateParticle(ParticleReturn* pReturn, Particle* pPart, EffectsTemplate* pTemplate, const nlVector3& viewRight, const nlVector3& viewUp, const nlMatrix4* pCoordSys)
{
    int colourIndex = (int)(24.0f * (pPart->timeElapsed / pPart->lifeSpan));
    nlColour* pColours = pTemplate->m_cColour;
    pReturn->c = pColours[colourIndex];

    float size = (pPart->dSize * pPart->timeElapsed) + pPart->size;
    float d = pPart->timeElapsed * (((0.5f * pPart->acceleration) * pPart->timeElapsed) + pPart->velocity);
    float s2 = 0.5f * size;

    float posZ = (d * pPart->velDir.f.z) + pPart->position.f.z;
    float posY = (d * pPart->velDir.f.y) + pPart->position.f.y;
    float posX = (d * pPart->velDir.f.x) + pPart->position.f.x;
    float rot = (pPart->dRot * pPart->timeElapsed) + pPart->rot;

    nlVector3 position;
    nlVec3Set(position, posX, posY, posZ);

    if (pCoordSys != nullptr)
    {
        nlMultPosVectorMatrix(position, position, *pCoordSys);
    }

    position.f.z = (pPart->mass * ((0.5f * pPart->timeElapsed) * pPart->timeElapsed)) + position.f.z;

    if (pTemplate->m_uModelID != 0xFFFFFFFF)
    {
        pReturn->position[0] = position;
        pReturn->position[1].f.x = size;
        pReturn->position[1].f.y = rot;
        return;
    }

    int animFrame = (int)(pPart->FPS * pPart->timeElapsed + pPart->frame);
    animFrame = animFrame % pTemplate->m_nFrames;

    s16* pFrame = (s16*)((u8*)textureFrames[pTemplate->m_nFrames - 1] + (animFrame << 3));

    pReturn->texcoord[0][0] = pFrame[1];
    pReturn->texcoord[0][1] = pFrame[2];
    pReturn->texcoord[1][0] = pFrame[0];
    pReturn->texcoord[1][1] = pFrame[2];
    pReturn->texcoord[2][0] = pFrame[0];
    pReturn->texcoord[2][1] = pFrame[3];
    pReturn->texcoord[3][0] = pFrame[1];
    pReturn->texcoord[3][1] = pFrame[3];

    float sn;
    float cs;
    nlSinCos(&sn, &cs, (unsigned short)(((int)(65536.0f * rot)) / 360));

    sn = sn * s2;
    cs = cs * s2;
    float nsn = -sn;

    float x0 = (cs * viewRight.f.x) + (sn * viewUp.f.x);
    float x1 = (nsn * viewRight.f.x) + (cs * viewUp.f.x);
    float y0 = (cs * viewRight.f.y) + (sn * viewUp.f.y);
    float y1 = (nsn * viewRight.f.y) + (cs * viewUp.f.y);
    float z0 = (cs * viewRight.f.z) + (sn * viewUp.f.z);
    float z1 = (nsn * viewRight.f.z) + (cs * viewUp.f.z);

    pReturn->position[0].f.x = (position.f.x + x0) + x1;
    pReturn->position[0].f.y = (position.f.y + y0) + y1;
    pReturn->position[0].f.z = (position.f.z + z0) + z1;

    pReturn->position[1].f.x = (position.f.x - x0) + x1;
    pReturn->position[1].f.y = (position.f.y - y0) + y1;
    pReturn->position[1].f.z = (position.f.z - z0) + z1;

    pReturn->position[2].f.x = (position.f.x - x0) - x1;
    pReturn->position[2].f.y = (position.f.y - y0) - y1;
    pReturn->position[2].f.z = (position.f.z - z0) - z1;

    pReturn->position[3].f.x = (position.f.x + x0) - x1;
    pReturn->position[3].f.y = (position.f.y + y0) - y1;
    pReturn->position[3].f.z = (position.f.z + z0) - z1;
}

/**
 * Offset/Address/Size: 0x970 | 0x801F5AC8 | size: 0xC3C
 */
void ParticleSystem::RenderAllParticles(eGLView)
{
}

/**
 * Offset/Address/Size: 0x954 | 0x801F5AAC | size: 0x1C
 */
void ParticleSystem::Die()
{
    m_fDelay = 0.0f;
    m_fElapsedTime = 100000000000000000000.0f;
    m_bAmDying = true;
}

/**
 * Offset/Address/Size: 0x6F4 | 0x801F584C | size: 0x260
 */
bool ParticleSystem::Update(float dt)
{
    if (m_fDelay > 0.0f)
    {
        m_fDelay -= dt;
        if (m_fDelay < 0.0f)
        {
            m_fDelay = 0.0f;
        }
        return true;
    }

    m_fElapsedTime += dt;

    if (m_pSpec != nullptr)
    {
        float lingerEnd = m_pSpec->m_fLingerEnd;
        if ((lingerEnd >= 0.0f) && !m_bAmDying && (m_fElapsedTime > lingerEnd))
        {
            m_fElapsedTime = m_pSpec->m_fLingerStart;
            if (m_pTemplate->m_fFountainLife <= 0.0f)
            {
                dt = 0.0f;
            }
        }
    }

    float fountainLife = m_pTemplate->m_fFountainLife;
    if (fountainLife <= 0.0f)
    {
        if (!m_bAmDying && (m_Particles.m_headNode == nullptr))
        {
            if ((m_pSpec == nullptr) || (m_pSpec->m_fLingerStart < 0.0f))
            {
                m_bAmDying = true;
            }
            m_fNumParticlesToCreate += RandomizedValue(m_pTemplate->m_rNumber.base, m_pTemplate->m_rNumber.range);
        }
    }
    else if (m_fElapsedTime >= fountainLife)
    {
        m_fElapsedTime = fountainLife;
        m_bAmDying = true;
    }
    else
    {
        m_fNumParticlesToCreate += dt * RandomizedValue(m_pTemplate->m_rNumber.base, m_pTemplate->m_rNumber.range);
    }

    int numParticles = (int)m_fNumParticlesToCreate;
    m_fNumParticlesToCreate -= (float)numParticles;
    if (m_fNumParticlesToCreate < 0.0f)
    {
        m_fNumParticlesToCreate = 0.0f;
    }

    if (numParticles > 0)
    {
        CreateNewParticles(numParticles);
    }

    Particle* p = (Particle*)m_Particles.m_headNode;
    while (p != nullptr)
    {
        Particle* next = (Particle*)p->m_nextNode;
        p->timeElapsed += dt;
        if (p->timeElapsed >= p->lifeSpan)
        {
            m_Particles.Remove(p);
            freeParticles.Append(p);
        }
        p = next;
    }

    if (m_bAmDying && (m_Particles.m_headNode == nullptr))
    {
        return false;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x6E0 | 0x801F5838 | size: 0x14
 */
float ParticleSystem::GetRemainingTime() const
{
    return m_pTemplate->m_fFountainLife - m_fElapsedTime;
}

struct TextureFrame
{
    s16 su;
    s16 suinc;
    s16 sv;
    s16 svinc;
};

static inline TextureFrame* BuildFrameLookup(int numFrames, float inc)
{
    TextureFrame* p = (TextureFrame*)nlMalloc(numFrames * sizeof(TextureFrame), 8, false);
    float u = 0.0f;
    float v = 0.0f;
    TextureFrame* q = p;
    int i = 0;
    while (i < numFrames)
    {
        float fSU = 1024.0f * u;
        float fSV = 1024.0f * v;
        float fSUinc = 1024.0f * (u + inc);
        float fSVinc = 1024.0f * (v + inc);
        q->su = (s16)fSU;
        q->sv = (s16)fSV;
        q->suinc = (s16)fSUinc;
        q->svinc = (s16)fSVinc;
        u += inc;
        if (u >= 0.999f)
        {
            u = 0.0f;
            v += inc;
        }
        i++;
        q++;
    }
    return p;
}

/**
 * Offset/Address/Size: 0x188 | 0x801F52E0 | size: 0x558
 */
void BuildFrameTable()
{
    textureFrames[0] = nlMalloc(sizeof(TextureFrame), 8, false);
    ((TextureFrame*)textureFrames[0])->su = 0;
    ((TextureFrame*)textureFrames[0])->sv = 0;
    ((TextureFrame*)textureFrames[0])->suinc = 1024;
    ((TextureFrame*)textureFrames[0])->svinc = 1024;

    textureFrames[3] = BuildFrameLookup(4, 0.5f);
    textureFrames[8] = BuildFrameLookup(9, 1.0f / 3.0f);
    textureFrames[15] = BuildFrameLookup(16, 0.25f);
    textureFrames[24] = BuildFrameLookup(25, 0.2f);
    textureFrames[35] = BuildFrameLookup(36, 1.0f / 6.0f);
}

// /**
//  * Offset/Address/Size: 0x178 | 0x801F52D0 | size: 0x10
//  */
// Particle::Particle()
// {
// }

static void ParticleConstructor(void* ptr, int)
{
    new (ptr) Particle();
}

/**
 * Offset/Address/Size: 0xAC | 0x801F5204 | size: 0xCC
 * TODO: 94.9% match - instruction scheduling diff at loop init: li r30/mr r29 ordering
 */
bool fxParticleStartup(int maxNumParticles)
{
    MaxNumParticles = maxNumParticles;
    BuildFrameTable();

    particleMemory = new (nlMalloc(MaxNumParticles * 0x4C + 0x10, 8, false)) Particle[MaxNumParticles];

    tDebugPrintManager::Print(DC_RENDER, "%dKB used by Particle pool\n", (unsigned)(MaxNumParticles * 0x4C) >> 10);

    int i = 0;
    int offset = i;
    for (; i < MaxNumParticles; offset += sizeof(Particle), i++)
    {
        freeParticles.Insert((efBaseNode*)((u8*)particleMemory + offset));
    }

    return true;
}

/**
 * Offset/Address/Size: 0x0 | 0x801F5158 | size: 0xAC
 */
bool fxParticleShutdown()
{
    for (int i = 0; i < 36; i++)
    {
        if (textureFrames[i] != nullptr)
        {
            delete[] (u8*)textureFrames[i];
            textureFrames[i] = nullptr;
        }
    }

    while (freeParticles.m_headNode != nullptr)
    {
        freeParticles.Remove();
    }

    if (particleMemory != nullptr)
    {
        if (particleMemory != nullptr)
        {
            delete[] ((u8*)particleMemory - 0x10);
        }
        particleMemory = nullptr;
    }
    return true;
}
