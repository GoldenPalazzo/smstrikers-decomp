#pragma pool_data off

#include "Game/Camera/FollowCam.h"
#include "Game/Ball.h"
#include "Game/Team.h"
#include "Game/ReplayManager.h"

#include "NL/gl/glMatrix.h"

cTeam* g_pTeams[2] __attribute__((section(".data"))) = { NULL, NULL };

// extern s32 currentlySelectedTarget$647;
extern u8 g_bProfiling;
extern u8 g_bTweaking;
// extern u8 init$648;
static f32 g_fDistanceSeek = 0.5f;
static f32 g_fMaxDistance = 40.0f;
static f32 g_fMinDistance = 2.0f;
static f32 g_fFollowCamOOISeek = 0.1f;
static f32 g_fFollowCamOOIZSeek = 0.025f;
static f32 g_fFollowCamMaxRotPerFrame = 400.0f;
static u16 g_aFollowCamMaxPitch = 0x3555;
static u16 g_aFollowCamMinPitch = 0x4B9;
static f32 g_fFollowCamMaxZOffset = 3.0f;
static f32 g_fFollowCamMinZOffset = 0.6f;

inline float nlLerp(float a, float b, float t)
{
    return a + t * (b - a);
}

cFollowCamera::cFollowCamera(FollowTarget followTarget)
{
    m_FollowTarget = followTarget;
    m_bOOISet = false;
    m_aFacingDirection = 0;
    m_aPitch = 0x1000;
    m_fOOIDistance = 5.0f;
    m_bPitchLimits = true;
    m_bControlsLocked = false;
    m_matView.SetIdentity();
}

/**
 * Offset/Address/Size: 0x0 | 0x801A8F18 | size: 0x668
 * TODO: 94.32% match - GPR r28-r31 allocation mismatch (context-level, all diffs are register swaps)
 */
void cFollowCamera::Update(float dt)
{
    int i;                  // r31
    cGlobalPad* pad;        // r29
    cCharacter* pCharacter; // r0
    float rx, ry;
    nlMatrix4 m4Orient; // r1+0x14

    pad = cPadManager::GetPad(0);
    if (!pad->IsConnected())
    {
        m_matView.SetIdentity();
        return;
    }

    if (m_FollowTarget == FOLLOW_CHARACTER)
    {
        pCharacter = NULL;
        for (i = 0; i < 2; i++)
        {
            pCharacter = g_pTeams[i]->GetControlledPlayer(pad);
            if (pCharacter != NULL)
            {
                break;
            }
        }
        if (!pCharacter)
        {
            m_matView.SetIdentity();
            return;
        }

        m_v3OOI = pCharacter->m_v3Position;
    }
    else if (m_FollowTarget == FOLLOW_BALL)
    {
        m_v3OOI = *g_pBall->GetDrawablePosition();
    }
    else if (m_FollowTarget == FOLLOW_ANIM_VIEWER_CHARACTER)
    {
        // EMPTY
    }
    else if (m_FollowTarget == FOLLOW_SELECTABLE)
    {
        RenderSnapshot* snap = ReplayManager::Instance()->mRender;
        static s32 currentlySelectedTarget = 0;

        const int count = snap->NumDrawableObjects();

        if (!g_bTweaking && dt > 0.0f)
        {
            if (cPadManager::GetPad(0)->JustPressed(1, false) && !g_bTweaking)
            {
                int v = (currentlySelectedTarget + count - 1);
                currentlySelectedTarget = v - (v / count) * count; // mod
            }
            if (cPadManager::GetPad(0)->JustPressed(2, false) && !g_bTweaking)
            {
                int v = currentlySelectedTarget + 1;
                currentlySelectedTarget = v - (v / count) * count; // mod
            }
        }

        m_v3OOI = snap->GetPositionForDrawableObject(currentlySelectedTarget);
    }
    else
    {
        m_matView.SetIdentity();
        return;
    }

    const float frac = (m_fOOIDistance - g_fMinDistance) / (g_fMaxDistance - g_fMinDistance);
    const float zOff = nlLerp(g_fFollowCamMinZOffset, g_fFollowCamMaxZOffset, frac);
    m_v3OOI.f.z += zOff;

    m_v3OOIDampenedPrev = m_v3OOIDampened;

    m_v3OOIDampened.f.x = g_fFollowCamOOISeek * m_v3OOI.f.x + (1.0f - g_fFollowCamOOISeek) * m_v3OOIDampened.f.x;
    m_v3OOIDampened.f.y = g_fFollowCamOOISeek * m_v3OOI.f.y + (1.0f - g_fFollowCamOOISeek) * m_v3OOIDampened.f.y;
    m_v3OOIDampened.f.z = g_fFollowCamOOIZSeek * m_v3OOI.f.z + (1.0f - g_fFollowCamOOIZSeek) * m_v3OOIDampened.f.z;

    const float aL = cPadManager::GetPad(0)->GetPressure(0x400, false);
    const float aR = cPadManager::GetPad(0)->GetPressure(0x800, false);
    if ((aL > 0.0f && cPadManager::GetPad(0)->JustPressed(0x800, false))
        || (aR > 0.0f && cPadManager::GetPad(0)->JustPressed(0x400, false)))
    {
        m_bControlsLocked = !m_bControlsLocked;
    }

    if (!g_bTweaking && !g_bProfiling && !m_bControlsLocked)
    {
        const float scroll = pad->AnalogLeftY();
        m_fOOIDistance -= g_fDistanceSeek * scroll;
    }

    if (m_fOOIDistance > g_fMaxDistance)
        m_fOOIDistance = g_fMaxDistance;
    else if (m_fOOIDistance < g_fMinDistance)
        m_fOOIDistance = g_fMinDistance;

    m_aFacingDirection = m_aFacingDirection + (int)(g_fFollowCamMaxRotPerFrame * pad->AnalogRightX());
    m_aPitch = m_aPitch + (int)(g_fFollowCamMaxRotPerFrame * pad->AnalogRightY());

    if (m_bPitchLimits)
    {
        if (m_aPitch > g_aFollowCamMaxPitch)
            m_aPitch = g_aFollowCamMaxPitch;
        if (m_aPitch < g_aFollowCamMinPitch)
            m_aPitch = g_aFollowCamMinPitch;
    }

    const float vx = -m_matView.m[1][2];
    const float vy = -m_matView.m[0][2];

    const float dx = (m_v3OOIDampened.f.x - m_v3OOIDampenedPrev.f.x);
    const float dy = (m_v3OOIDampened.f.y - m_v3OOIDampenedPrev.f.y);

    const float denom = nlSqrt(vx * vx + vy * vy, true);
    const float t = (vx * dy + vy * dx) / (denom * denom);

    rx = dy - t * vx;
    ry = dx - t * vy;
    const float len = nlSqrt(rx * rx + ry * ry, true);

    const float invDist = len / m_fOOIDistance;
    const float angleShortF = 10430.378f * invDist;
    const u16 angleShort = (u16)(int)angleShortF;

    const float signCheck = (ry * vx - rx * vy);
    if (signCheck >= 0.0f)
        m_aFacingDirection = m_aFacingDirection - angleShort;
    else
        m_aFacingDirection = m_aFacingDirection + angleShort;

    nlVec3Set(m_v3CameraPosition, m_fOOIDistance, 0.0f, 0.0f);

    nlMakeRotationMatrixY(m4Orient, -m_aPitch * 0.0000958738f);
    nlMultPosVectorMatrix(m_v3CameraPosition, m_v3CameraPosition, m4Orient);

    nlMakeRotationMatrixZ(m4Orient, m_aFacingDirection * 0.0000958738f);
    nlMultPosVectorMatrix(m_v3CameraPosition, m_v3CameraPosition, m4Orient);

    m_v3CameraPosition.f.x += m_v3OOIDampened.f.x;
    m_v3CameraPosition.f.y += m_v3OOIDampened.f.y;
    m_v3CameraPosition.f.z += m_v3OOIDampened.f.z;

    glMatrixLookAt(m_matView, m_v3CameraPosition, m_v3OOIDampened, mUpVector);
}
