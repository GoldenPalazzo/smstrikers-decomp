#include "Game/Camera/GoalCam.h"

#include "NL/gl/glMatrix.h"
#include "NL/globalpad.h"

#include "Game/Ball.h"
#include "Game/Field.h"
#include "Game/MathHelpers.h"

// static bool init$541;
// static f32 fDir;
static bool gbCamFreeze;
static s32 gnCamType;

f32 gfDistance = 10.0f;
f32 gfHeight = 3.0f;
f32 gfSideMult = 2.0f;
f32 gfSideBias = 0.4f;
f32 gfCamDir = 3.1415927f;
f32 gfCamTilt = 0.1f;
f32 gfCamDist = 14.0f;

/**
 * Offset/Address/Size: 0x6CC | 0x801AAC68 | size: 0x7C
 */
GoalCamera::GoalCamera()
{
    m_vecTarget.f.x = 0.0f;
    m_vecTarget.f.y = 0.0f;
    m_vecTarget.f.z = 0.0f;
    Update(0.0f);
}

/**
 * Offset/Address/Size: 0x670 | 0x801AAC0C | size: 0x5C
 */
GoalCamera::~GoalCamera()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x801AA59C | size: 0x670
 * TODO: 81.8% match - camType==0 WeightedSum: 0.9f*lastpos.[xyz] fmul is hoisted
 * ahead of the per-component fmadd loop instead of staying interleaved with cam reload/store.
 */
void GoalCamera::Update(float /*dt*/)
{
    nlVector3 vecUp;
    nlVector3 ballpos;
    nlVector3 lastpos;
    nlVector3 lasttarg;
    nlVector3 dirvec;
    float fDirSin;
    float fDirCos;
    float fTiltSin;
    float fTiltCos;
    nlVector3 midvec;

    vecUp.f.x = 0.0f;
    vecUp.f.y = 0.0f;
    vecUp.f.z = 1.0f;

    if (gnCamType == 0)
    {
        if (g_pBall->m_tShotTimer.m_uPackedTime != 0 || g_pBall->GetOwnerGoalie() != NULL)
        {
            return;
        }
    }

    if (gbCamFreeze != 0)
    {
        return;
    }

    static f32 fDir = 1.0f;

    if (g_pBall->m_pOwner == NULL || g_pBall->m_pOwner->m_eClassType == GOALIE)
    {
        ballpos = g_pBall->m_v3Position;
    }
    else
    {
        ballpos = g_pBall->m_pOwner->m_v3Position;
    }

    lastpos = m_vecCamera;
    lasttarg = m_vecTarget;

    if (gnCamType == 0)
    {
        m_vecTarget.f.x = fDir * cField::GetGoalLineX((unsigned int)1);
        m_vecTarget.f.y = 0.0f;
        m_vecTarget.f.z = 0.0f;

        nlVec3Sub(m_vecCamera, ballpos, m_vecTarget);

        f32 invLen = nlRecipSqrt((m_vecCamera.f.x * m_vecCamera.f.x) + (m_vecCamera.f.y * m_vecCamera.f.y) + (m_vecCamera.f.z * m_vecCamera.f.z), 1);
        nlVec3Scale(m_vecCamera, invLen);

        nlVec3ScaleAdd(m_vecCamera, gfDistance, m_vecCamera, ballpos);
        m_vecCamera.f.z = gfHeight;

        nlVec3WeightedSum(m_vecCamera, 0.1f, m_vecCamera, 0.9f, lastpos);
    }
    else if (gnCamType == 1)
    {
        m_vecTarget = ballpos;

        g_pBall->m_pPhysicsBall->GetLinearVelocity(&dirvec);

        dirvec.f.z = 0.0f;
        dirvec.f.x = 1.0f;
        dirvec.f.y = 0.0f;
        dirvec.f.z = 0.3f;

        {
            f32 invLen = nlRecipSqrt((dirvec.f.x * dirvec.f.x) + (dirvec.f.y * dirvec.f.y) + (dirvec.f.z * dirvec.f.z), 1);
            nlVec3Scale(dirvec, invLen);
            dirvec.f.x = -1.0f * dirvec.f.x;
            dirvec.f.y = -1.0f * dirvec.f.y;
        }

        f32 lx = 0.8f * lastpos.f.x;
        f32 ly = 0.8f * lastpos.f.y;
        f32 lz = 0.8f * lastpos.f.z;
        f32 tx = 0.8f * lasttarg.f.x;
        f32 ty = 0.8f * lasttarg.f.y;
        f32 tz = 0.8f * lasttarg.f.z;

        m_vecCamera.f.x = m_vecTarget.f.x + gfDistance * dirvec.f.x;
        m_vecCamera.f.y = m_vecTarget.f.y + gfDistance * dirvec.f.y;
        m_vecCamera.f.z = m_vecTarget.f.z + gfDistance * dirvec.f.z;
        m_vecCamera.f.z = m_vecTarget.f.z + gfHeight;

        m_vecCamera.f.x = 0.2f * m_vecCamera.f.x + lx;
        m_vecCamera.f.y = 0.2f * m_vecCamera.f.y + ly;
        m_vecCamera.f.z = 0.2f * m_vecCamera.f.z + lz;

        m_vecTarget.f.x = 0.2f * m_vecTarget.f.x + tx;
        m_vecTarget.f.y = 0.2f * m_vecTarget.f.y + ty;
        m_vecTarget.f.z = 0.2f * m_vecTarget.f.z + tz;
    }
    else
    {
        m_vecTarget.f.x = fDir * cField::GetGoalLineX((unsigned int)1);
        m_vecTarget.f.y = 0.0f;
        m_vecTarget.f.z = gfHeight;

        nlSinCos(&fDirSin, &fDirCos, RadToAng16(gfCamDir));
        nlSinCos(&fTiltSin, &fTiltCos, RadToAng16(gfCamTilt));

        f32 lx = 0.9f * lastpos.f.x;
        f32 ly = 0.9f * lastpos.f.y;
        f32 lz = 0.9f * lastpos.f.z;

        m_vecCamera.f.x = fTiltCos * (gfCamDist * fDirCos);
        m_vecCamera.f.y = fTiltCos * (gfCamDist * fDirSin);
        m_vecCamera.f.z = gfCamDist * fTiltSin;

        m_vecCamera.f.x += m_vecTarget.f.x;
        m_vecCamera.f.y += m_vecTarget.f.y;
        m_vecCamera.f.z += m_vecTarget.f.z;

        m_vecCamera.f.x = 0.1f * m_vecCamera.f.x + lx;
        m_vecCamera.f.y = 0.1f * m_vecCamera.f.y + ly;
        m_vecCamera.f.z = 0.1f * m_vecCamera.f.z + lz;

        ballpos = m_vecCamera;
        ballpos.f.z = gfHeight;
    }

    if (cPadManager::GetPad(0)->IsPressed(0x800, 0))
    {
        if (gnCamType != 1)
        {
            float dx = m_vecTarget.f.x - ballpos.f.x;
            float dz = m_vecTarget.f.z - ballpos.f.z;
            float k = gfSideBias;
            float kz = k * ballpos.f.z;
            float k1 = 1.0f - k;
            float tx = m_vecTarget.f.y;
            float by = ballpos.f.y;
            float dy = tx - by;
            float mx = k1 * m_vecTarget.f.x + k * ballpos.f.x;
            float my = k1 * tx + k * by;
            float mz = k1 * m_vecTarget.f.z + kz;

            dirvec.f.x = dx;
            dirvec.f.z = dz;
            dirvec.f.y = dy;
            dirvec.f.z = dx;
            dirvec.f.x = dy;
            dirvec.f.y = -dx;

            midvec.f.x = mx;
            midvec.f.y = my;
            midvec.f.z = mz;

            dirvec.f.x = gfSideMult * dirvec.f.x + mx;
            dirvec.f.y = gfSideMult * dirvec.f.y + my;
            dirvec.f.z = gfHeight;
        }
        else if (gnCamType == 1)
        {
            float y = dirvec.f.y;
            float x = dirvec.f.x;
            float nx = -y;

            dirvec.f.z = x;
            dirvec.f.x = nx;
            dirvec.f.y = x;

            dirvec.f.z = m_vecTarget.f.z + gfDistance * dirvec.f.z;
            dirvec.f.x = m_vecTarget.f.x + gfDistance * dirvec.f.x;
            dirvec.f.y = m_vecTarget.f.y + gfDistance * dirvec.f.y;
            dirvec.f.z = m_vecTarget.f.z + gfHeight;

            midvec = m_vecTarget;
        }
        glMatrixLookAt(m_matView, dirvec, midvec, vecUp);
    }
    else
    {
        glMatrixLookAt(m_matView, m_vecCamera, m_vecTarget, vecUp);
    }
}
