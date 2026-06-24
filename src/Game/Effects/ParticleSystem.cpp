#include "Game/Effects/ParticleSystem.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/GL/GLInventory.h"
#include "Game/GL/gluMeshWriter.h"
#include "Game/Sys/debug.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"
#include "NL/gl/glUserData.h"
#include "NL/nlMemory.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "types.h"

int ParticleSystem::m_nNumViews = 0;
int ParticleSystem::m_NumInstances = 0;
eGLView ParticleSystem::m_eViews[8];

struct FreeParticles : public efList
{
    FreeParticles()
    {
        m_headNode = nullptr;
        m_tailNode = nullptr;
        m_numNodes = 0;
    }
};

FreeParticles freeParticles;

void* textureFrames[36] = { nullptr };
static Particle* particleMemory;
static int MaxNumParticles;

extern float m_fAspect__14ParticleSystem;
extern volatile u8 m_AllowInFront__14ParticleSystem;
extern glModel* (*m_LightingCallback__14ParticleSystem)(glModel*);
extern u8 (*m_Callback__14ParticleSystem)(eGLView, unsigned long, efList&, EffectsTemplate*, nlVector3&, nlVector3&, nlMatrix4*);

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
 * TODO: 97.91% match - register allocation diffs in tilt rotation and
 * hackyFacingAngle rotation blocks; one length-square instruction order diff remains.
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
    if (tilt <= -90.0f)
    {
        tilt = -89.9f;
    }
    else if (tilt >= 90.0f)
    {
        tilt = 89.9f;
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
        float posX = localPos.f.x;

        localDir.f.x = (dirX * cos) + (localDir.f.z * sin);
        localDir.f.z = (-dirX * sin) + (localDir.f.z * cos);

        nlVec3Set(localPos,
            (posX * cos) + (localPos.f.z * sin),
            (localPos.f.y = localPos.f.y),
            (-posX * sin) + (localPos.f.z * cos));
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
 * TODO: 97.61% match - remaining diffs are register-allocation only
 * in persistent emitter/loop state.
 */
#pragma opt_common_subs off
void ParticleSystem::CreateNewParticles(int numParticles)
{
    void (*emit)(nlVector3&, nlVector3&, EffectsTemplate*, EffectsSpec*, const nlMatrix4&);
    float oneOverLife;
    nlVector3 baseDir;
    nlVector3 dir;
    int i;
    nlMatrix4 mCoordSys;

    UpdateCoordSys(mCoordSys);
    EffectsTemplate* pTempl = m_pTemplate;
    if (pTempl->m_bLocalSpace)
    {
        baseDir.f.x = 0.0f;
        baseDir.f.y = 0.0f;
        baseDir.f.z = 1.0f;
    }
    else
    {
        baseDir = m_vForward;
    }
    switch (pTempl->m_eEmitter)
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
        hackyFacingAngle = m_aFacing;
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
    for (i = 0; i < numParticles; i++)
    {
        Particle* pPart;
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
        m_Particles.Insert(pPart);
        dir = baseDir;
        emit(pPart->position, dir, m_pTemplate, m_pSpec, mCoordSys);
        pPart->position.f.x += m_vSourcePosition.f.x;
        pPart->position.f.y += m_vSourcePosition.f.y;
        pPart->position.f.z += m_vSourcePosition.f.z;
        pPart->lifeSpan = RandomizedValue(m_pTemplate->m_rParticleLife.base, m_pTemplate->m_rParticleLife.range);
        oneOverLife = 1.0f / pPart->lifeSpan;
        pPart->dRot = RandomizedValue(m_pTemplate->m_rRotation.base, m_pTemplate->m_rRotation.range);
        {
            f32 rot;
            if (pPart->dRot == 0.0f)
            {
                rot = 0.0f;
            }
            else
            {
                rot = RandomizedValue(0.0f, 360.0f);
            }
            pPart->rot = rot;
        }
        pPart->mass = RandomizedValue(m_pTemplate->m_rMass.base, m_pTemplate->m_rMass.range);
        EffectsTemplate* pTemplate = m_pTemplate;
        float sizeBegin = RandomizedValue(pTemplate->m_rSizeBegin.base, pTemplate->m_rSizeBegin.range);
        float sizeEnd = RandomizedValue(pTemplate->m_rSizeEnd.base, pTemplate->m_rSizeEnd.range);
        pPart->size = sizeBegin;
        pPart->dSize = oneOverLife * (sizeEnd - sizeBegin);
        float inheritVelocity = RandomizedValue(m_pTemplate->m_rInheritVelocity.base, m_pTemplate->m_rInheritVelocity.range);
        float velX = inheritVelocity * m_vVelocity.f.x;
        float velZ = inheritVelocity * m_vVelocity.f.z;
        float velY = inheritVelocity * m_vVelocity.f.y;
        float vel = RandomizedValue(m_pTemplate->m_rVelocity.base, m_pTemplate->m_rVelocity.range);
        float velDirZ = vel * dir.f.z + velZ;
        float velDirY = vel * dir.f.y + velY;
        float velDirX = vel * dir.f.x + velX;
        float speedSquared = (velDirX * velDirX) + (velDirY * velDirY) + (velDirZ * velDirZ);
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
        pPart->acceleration = RandomizedValue(m_pTemplate->m_rAcceleration.base, m_pTemplate->m_rAcceleration.range);
        pPart->frame = 0.0f;
        pPart->FPS = RandomizedValue(m_pTemplate->m_rFPS.base, m_pTemplate->m_rFPS.range);
    }
}
#pragma opt_common_subs reset

/**
 * Offset/Address/Size: 0x15AC | 0x801F6704 | size: 0x354
 */
void ParticleSystem::UpdateParticle(ParticleReturn* pReturn, Particle* pPart, EffectsTemplate* pTemplate, const nlVector3& viewRight, const nlVector3& viewUp, const nlMatrix4* pCoordSys)
{
    int colourIndex = (int)(24.0f * (pPart->timeElapsed / pPart->lifeSpan));
    nlColour* pColours = pTemplate->m_cColour;
    pReturn->c = pColours[colourIndex];

    float size = (pPart->dSize * pPart->timeElapsed) + pPart->size;
    float d = pPart->timeElapsed * (((0.5f * pPart->acceleration) * pPart->timeElapsed) + pPart->velocity);
    float s2 = 0.5f * size;

    float posY;
    float posZ;
    float posX = (d * pPart->velDir.f.x) + pPart->position.f.x;
    posZ = (d * pPart->velDir.f.z) + pPart->position.f.z;
    posY = (d * pPart->velDir.f.y) + pPart->position.f.y;
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
    animFrame %= pTemplate->m_nFrames;

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
    float x0 = (cs * viewRight.f.x) + (sn * viewUp.f.x);
    float y0 = (cs * viewRight.f.y) + (sn * viewUp.f.y);
    float z0 = (cs * viewRight.f.z) + (sn * viewUp.f.z);
    float x1 = ((-sn) * viewRight.f.x) + (cs * viewUp.f.x);
    float y1 = ((-sn) * viewRight.f.y) + (cs * viewUp.f.y);
    float z1 = ((-sn) * viewRight.f.z) + (cs * viewUp.f.z);

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

static inline void RenderLightOnField(const EffectsLight& light)
{
    float heightFrac = 1.0f - (light.m_v3Position.f.z / light.m_fRadius);
    if (!(heightFrac <= 0.0f))
    {
        if (heightFrac > 1.0f)
        {
            heightFrac = 1.0f;
        }

        glSetDefaultState(true);
        glSetCurrentTexture(glGetTexture("global/light_blob"), GLTT_Diffuse);
        glSetRasterState(GLS_AlphaBlend, 3);
        glSetRasterState(GLS_DepthWrite, 0);
        glSetCurrentRasterState(glHandleizeRasterState());

        float dim = 1.4f * ((2.0f * light.m_fRadius) * (heightFrac * heightFrac));
        nlMatrix4 mRot;
        mRot.SetIdentity();
        glQuad3 q;
        q.SetupRotatedRectangle(dim, dim, mRot, false, false);
        q.SetColour(light.m_Colour);
        for (int i = 0; i < 4; i++)
        {
            nlVec3Add(q.m_pos[i], q.m_pos[i], light.m_v3Position);
            q.m_pos[i].f.z = 0.03125f;
            q.m_colour[i].c[3] = (unsigned char)((int)q.m_colour[i].c[3] / 3);
        }

        glModel* pModel = (glModel*)q.GetModel(true);
        void* pUserData = glUserAlloc(GLUD_NoFog, 0, false);
        glUserAttach(pUserData, pModel->packets, false);
        glViewAttachModel((eGLView)7, 2, pModel);
    }
}

/**
 * Offset/Address/Size: 0x970 | 0x801F5AC8 | size: 0xC3C
 */
void ParticleSystem::RenderAllParticles(eGLView view)
{
    static int _tris[6] = { 0, 1, 2, 0, 2, 3 };
    static u32 WhiteTexture;
    static s8 init;

    ParticleReturn ret;
    GLMeshWriter mesh;
    eGLStream stream_decl[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
    nlVector3 viewRight;
    nlVector3 viewUp;
    nlMatrix4 viewMatrix;
    nlMatrix4 mCoordSys;
    bool cullBackFaces;

    UpdateCoordSys(mCoordSys);
    if (m_Particles.m_numNodes == 0)
    {
        return;
    }
    cullBackFaces = true;

    if (m_pTemplate->m_eBillboard == EfBill_Billboard)
    {
        glViewGetViewMatrix(view, viewMatrix);
        nlVec3Set(viewRight, viewMatrix.e[0], viewMatrix.e[4], viewMatrix.e[8]);
        nlVec3Set(viewUp, viewMatrix.e[1], viewMatrix.e[5], viewMatrix.e[9]);
        nlVec3Scale(viewRight, m_fAspect__14ParticleSystem);
    }
    else if (m_pTemplate->m_eBillboard == EfBill_Groundboard)
    {
        viewRight.f.x = 1.0f;
        viewRight.f.y = 0.0f;
        viewRight.f.z = 0.0f;
        viewUp.f.x = 0.0f;
        viewUp.f.y = 1.0f;
        viewUp.f.z = 0.0f;
    }
    else if (m_pTemplate->m_eBillboard == EfBill_SoftwareControlled)
    {
        nlMatrix4 rot;
        viewUp.f.x = 0.0f;
        viewUp.f.y = 0.0f;
        viewUp.f.z = 1.0f;
        viewRight.f.x = 1.0f;
        viewRight.f.y = 0.0f;
        viewRight.f.z = 0.0f;
        cullBackFaces = false;
        nlMakeRotationMatrixZ(rot, 0.0000958738f * (float)(unsigned short)(m_aFacing + 0x4000));
        nlMultDirVectorMatrix(viewRight, viewRight, rot);
    }

    glSetDefaultState(true);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetRasterState(GLS_Culling, cullBackFaces ? 1 : 0);

    if (m_AllowInFront__14ParticleSystem != 0)
    {
        if (m_pTemplate->m_bInFront || ((m_pSpec != nullptr) && m_pSpec->m_bInFront))
        {
            glSetRasterState(GLS_DepthTest, 0);
        }
    }

    switch (m_pTemplate->m_eBlend)
    {
    case EfBlend_Normal:
        glSetRasterState(GLS_AlphaBlend, 1);
        break;
    case EfBlend_Additive:
        glSetRasterState(GLS_AlphaBlend, 3);
        break;
    }

    glSetRasterState(GLS_AlphaTest, 1);
    glSetCurrentRasterState(glHandleizeRasterState());

    if (!init)
    {
        WhiteTexture = glGetTexture("global/white");
        init = 1;
    }

    glSetCurrentTexture(m_pTemplate->m_hTexture, GLTT_Diffuse);
    glSetCurrentProgram(glGetProgram("3d unlit"));

    if ((m_pSpec != nullptr) && m_pSpec->m_bLight)
    {
        Particle* pPart = (Particle*)m_Particles.m_headNode;
        const nlMatrix4* pCoord = m_pTemplate->m_bLocalSpace ? &mCoordSys : nullptr;
        while (pPart != nullptr)
        {
            int colourIndex = (int)(24.5f * (pPart->timeElapsed / pPart->lifeSpan));
            EffectsLight light;
            light.m_Colour = m_pTemplate->m_cColour[colourIndex];
            light.m_fRadius = 0.5f * ((pPart->dSize * pPart->timeElapsed) + pPart->size);

            float d = pPart->timeElapsed * (((0.5f * pPart->acceleration) * pPart->timeElapsed) + pPart->velocity);
            nlVector3 position;
            nlVec3Set(position,
                (d * pPart->velDir.f.x) + pPart->position.f.x,
                (d * pPart->velDir.f.y) + pPart->position.f.y,
                (d * pPart->velDir.f.z) + pPart->position.f.z);
            if (pCoord != nullptr)
            {
                nlMultPosVectorMatrix(position, position, *pCoord);
            }
            position.f.z += pPart->mass * ((-9.81f * pPart->timeElapsed) * pPart->timeElapsed);

            light.m_v3Position = position;
            EmissionManager::AddEffectsLight(light);
            RenderLightOnField(light);
            pPart = (Particle*)pPart->m_nextNode;
        }
    }
    else if (m_pTemplate->m_uModelID != 0xFFFFFFFF)
    {
        glModel* pModel;
        GLVertexAnim* pAnim = glInventory.GetVertexAnim(m_pTemplate->m_uModelID);
        eEffectsBlend blendType;

        nlMatrix4 m;
        nlMatrix4 mScale;
        nlMatrix4 mRot;
        nlMatrix4 mCoord;
        mCoord = mCoordSys;
        mCoord.e[2] = -mCoord.e[2];
        mCoord.e[6] = -mCoord.e[6];
        mCoord.e[10] = -mCoord.e[10];

        switch (m_pTemplate->m_eBlend)
        {
        case EfBlend_Normal:
            blendType = EfBlend_Additive;
            break;
        case EfBlend_Additive:
            blendType = (eEffectsBlend)3;
            break;
        }

        Particle* pPart = (Particle*)m_Particles.m_headNode;
        const nlMatrix4* pCoord = m_pTemplate->m_bLocalSpace ? &mCoordSys : nullptr;

        while (pPart != nullptr)
        {
            if (pAnim == nullptr)
            {
                pModel = glModelDupNoStreams(glInventory.GetModel(m_pTemplate->m_uModelID), true, false);
            }

            UpdateParticle(&ret, pPart, m_pTemplate, viewRight, viewUp, pCoord);

            float rotRad = 3.1415927f * ret.position[1].f.y / 180.0f;
            float size = ret.position[1].f.x;
            if (m_pTemplate->m_eBillboard == EfBill_Billboard)
            {
                float facingRot = 0.0000958738f * (float)(unsigned short)(m_aFacing + 0x8000);
                rotRad += facingRot;
            }

            nlMakeRotationMatrixZ(mRot, rotRad);
            nlMakeScaleMatrix(mScale, size, size, size);
            nlMultMatrices(mScale, mScale, mRot);

            nlMultMatrices(m, mCoord, mScale);
            m.e[12] = ret.position[0].f.x;
            m.e[13] = ret.position[0].f.y;
            m.e[14] = ret.position[0].f.z;

            void* pUserData = glUserAlloc(GLUD_ConstantColour, 4, false);
            if (pUserData != nullptr)
            {
                u32* pDst = (u32*)glUserGetData(pUserData);
                *pDst = *(u32*)&ret.c;
            }

            u32 hMatrix = glAllocMatrix();
            if (hMatrix != 0xFFFFFFFF)
            {
                glSetMatrix(hMatrix, m);
            }

            float meshRateScale = 1.0f;
            if (pAnim != nullptr)
            {
                if (m_pTemplate->m_bMatchLifespan)
                {
                    meshRateScale = (((float)pAnim->m_nNumFrames) / pAnim->m_fFrameRate) / pPart->lifeSpan;
                    float frameFrac = pPart->timeElapsed / pPart->lifeSpan;
                    float frame = frameFrac * (float)(pAnim->m_nNumFrames - 1);
                    pModel = pAnim->GetModel((int)frame);
                }
                else
                {
                    meshRateScale = pPart->FPS / pAnim->m_fFrameRate;
                    float frame = pPart->FPS * pPart->timeElapsed;
                    float bound = (float)pAnim->m_nNumFrames;
                    while (frame >= bound)
                    {
                        frame -= bound;
                    }
                    pModel = pAnim->GetModel((int)frame);
                }
            }

            glModelPacket* pPacket = pModel->packets;
            while (pPacket < &pModel->packets[pModel->numPackets])
            {
                glSetRasterState(pPacket->state.raster, GLS_Culling, 0);
                glSetRasterState(pPacket->state.raster, GLS_AlphaBlend, blendType);
                glSetRasterState(pPacket->state.raster, GLS_AlphaTest, 1);
                glSetRasterState(pPacket->state.raster, GLS_AlphaTestRef, 3);
                pPacket->state.matrix = hMatrix;
                if (pUserData != nullptr)
                {
                    glUserAttach(pUserData, pPacket, false);
                }

                GLTextureAnim* pTex = glInventory.GetTextureAnim(pPacket->state.texture[0]);
                if (pTex != nullptr)
                {
                    pPacket->state.texture[0] = pTex->GetTextureHandle(meshRateScale * pPart->timeElapsed);
                }
                pPacket = (glModelPacket*)((u8*)pPacket + sizeof(glModelPacket));
            }

            if (m_pTemplate->m_bLit)
            {
                pModel = m_LightingCallback__14ParticleSystem(pModel);
            }

            glViewAttachModel(view, m_uLayer + 1, pModel);
            pPart = (Particle*)pPart->m_nextNode;
        }
    }
    else if (m_Callback__14ParticleSystem == nullptr)
    {
        bool bQuads = glHasQuads();
        bool began;
        if (bQuads)
        {
            began = mesh.Begin(m_Particles.m_numNodes * 4, GLP_QuadList, 3, stream_decl, false);
        }
        else
        {
            began = mesh.Begin(m_Particles.m_numNodes * 6, GLP_TriList, 3, stream_decl, false);
        }

        if (began)
        {
            Particle* pPart = (Particle*)m_Particles.m_headNode;
            const nlMatrix4* pCoord = m_pTemplate->m_bLocalSpace ? &mCoordSys : nullptr;
            while (pPart != nullptr)
            {
                UpdateParticle(&ret, pPart, m_pTemplate, viewRight, viewUp, pCoord);
                int i;
                if (bQuads)
                {
                    for (i = 0; i < 4; i++)
                    {
                        mesh.Texcoord(ret.texcoord[i][0], ret.texcoord[i][1]);
                        mesh.Colour(ret.c);
                        mesh.Vertex(ret.position[i]);
                    }
                }
                else
                {
                    for (i = 0; i < 6; i++)
                    {
                        mesh.Texcoord(ret.texcoord[_tris[i]][0], ret.texcoord[_tris[i]][1]);
                        mesh.Colour(ret.c);
                        mesh.Vertex(ret.position[_tris[i]]);
                    }
                }
                pPart = (Particle*)pPart->m_nextNode;
            }

            if (mesh.End())
            {
                glViewAttachModel(view, m_uLayer, mesh.GetModel());
            }
            else
            {
                tDebugPrintManager::Print(DC_RENDER, "couldn't end mesh built by sprites\n");
            }
        }
        else
        {
            tDebugPrintManager::Print(DC_RENDER, "could not begin a mesh for sprites\n");
        }
    }
    else
    {
        nlMatrix4* pCoord = m_pTemplate->m_bLocalSpace ? &mCoordSys : nullptr;
        if (!m_Callback__14ParticleSystem(view, m_uLayer, m_Particles, m_pTemplate, viewRight, viewUp, pCoord))
        {
            tDebugPrintManager::Print(DC_RENDER, "too many particles for the fast-path\n");
        }
    }
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

static void AllocateParticles()
{
    int i;
    int count = MaxNumParticles;

    particleMemory = new (nlMalloc(count * 0x4C + 0x10, 8, false)) Particle[count];

    tDebugPrintManager::Print(DC_RENDER, "%dKB used by Particle pool\n", (unsigned)(MaxNumParticles * 0x4C) >> 10);

    for (i = 0; i < MaxNumParticles; i++)
    {
        freeParticles.Insert(&particleMemory[i]);
    }
}

/**
 * Offset/Address/Size: 0xAC | 0x801F5204 | size: 0xCC
 * TODO: 99.71% match - allocation count uses r29 instead of r31
 */
bool fxParticleStartup(int maxNumParticles)
{
    MaxNumParticles = maxNumParticles;
    BuildFrameTable();
    AllocateParticles();
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
