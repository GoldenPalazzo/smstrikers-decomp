#include "Game/Camera/GameplayCam.h"

#include "Game/Ball.h"
#include "Game/GameInfo.h"
#include "Game/ReplayManager.h"
#include "Game/Team.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/AiUtil.h"
#include "NL/nlTask.h"
#include "NL/gl/glMatrix.h"
#include "math.h"

f32 CANT_COLLIDE = *(f32*)__float_max;

static float nearZoomPositiveTargetKnotsY[3] = { -5.2f, 0.0f, 4.0f };
static float nearZoomPositiveFieldKnotsY[3] = { -5.6f, 0.0f, 11.65f };
static float farZoomPositiveTargetKnotsY[3] = { -2.6f, -2.6f, 2.0f };
static float farZoomPositiveFieldKnotsY[3] = { 0.0f, 0.0f, 11.65f };

static float nearZoomPositiveTargetKnotsX[2] = { -16.8f, 16.8f };
static float nearZoomPositiveFieldKnotsX[2] = { -17.0f, 14.5f };
static float nearZoomNegativeFieldKnotsX[2] = { -14.5f, 17.0f };
static float nearZoomNeutralFieldKnotsX[2] = { -15.75f, 15.75f };
static float farZoomPositiveTargetKnotsX[2] = { -13.3f, 13.3f };
static float farZoomPositiveFieldKnotsX[2] = { -13.0f, 8.0f };
static float farZoomNegativeFieldKnotsX[2] = { -8.0f, 13.0f };
static float farZoomNeutralFieldKnotsX[2] = { -10.5f, 10.5f };
static float nearZoomWidePositiveTargetKnotsX[2] = { -15.5f, 15.5f };
static float farZoomWidePositiveTargetKnotsX[2] = { -10.1f, 10.1f };

static CameraData gCameraData[4] = {
    {
        2,
        3,
        { nearZoomPositiveTargetKnotsX, nearZoomPositiveTargetKnotsX, nearZoomPositiveTargetKnotsX },
        { nearZoomNeutralFieldKnotsX, nearZoomPositiveFieldKnotsX, nearZoomNegativeFieldKnotsX },
        { nearZoomPositiveTargetKnotsY, nearZoomPositiveTargetKnotsY, nearZoomPositiveTargetKnotsY },
        { nearZoomPositiveFieldKnotsY, nearZoomPositiveFieldKnotsY, nearZoomPositiveFieldKnotsY },
        33.4f,
        20.0f,
        25.0f,
        270.0f,
    },
    {
        2,
        3,
        { farZoomPositiveTargetKnotsX, farZoomPositiveTargetKnotsX, farZoomPositiveTargetKnotsX },
        { farZoomNeutralFieldKnotsX, farZoomPositiveFieldKnotsX, farZoomNegativeFieldKnotsX },
        { farZoomPositiveTargetKnotsY, farZoomPositiveTargetKnotsY, farZoomPositiveTargetKnotsY },
        { farZoomPositiveFieldKnotsY, farZoomPositiveFieldKnotsY, farZoomPositiveFieldKnotsY },
        33.4f,
        35.0f,
        38.0f,
        270.0f,
    },
    {
        2,
        3,
        { nearZoomWidePositiveTargetKnotsX, nearZoomWidePositiveTargetKnotsX, nearZoomWidePositiveTargetKnotsX },
        { nearZoomNeutralFieldKnotsX, nearZoomPositiveFieldKnotsX, nearZoomNegativeFieldKnotsX },
        { nearZoomPositiveTargetKnotsY, nearZoomPositiveTargetKnotsY, nearZoomPositiveTargetKnotsY },
        { nearZoomPositiveFieldKnotsY, nearZoomPositiveFieldKnotsY, nearZoomPositiveFieldKnotsY },
        44.3f,
        20.0f,
        25.0f,
        270.0f,
    },
    {
        2,
        3,
        { farZoomWidePositiveTargetKnotsX, farZoomWidePositiveTargetKnotsX, farZoomWidePositiveTargetKnotsX },
        { farZoomNeutralFieldKnotsX, farZoomPositiveFieldKnotsX, farZoomNegativeFieldKnotsX },
        { farZoomPositiveTargetKnotsY, farZoomPositiveTargetKnotsY, farZoomPositiveTargetKnotsY },
        { farZoomPositiveFieldKnotsY, farZoomPositiveFieldKnotsY, farZoomPositiveFieldKnotsY },
        44.3f,
        35.0f,
        38.0f,
        270.0f,
    },
};

bool gGameplayCameraInReplay = false;

GameplayCameraZoomLevel::GameplayCameraZoomLevel()
    : m_CameraData(NULL)
    , m_fDesiredTargetX(0.0f)
    , m_fDesiredTargetY(0.0f)
    , m_fDampenedTargetX(0.0f)
    , m_fDampenedTargetY(0.0f)
    , m_fTargetSeekSpeedX(0.0f)
    , m_fTargetSeekSpeedY(0.0f)
    , m_fTargetSeekTime(0.15f)
{
}

/**
 * Offset/Address/Size: 0xCC8 | 0x801AA308 | size: 0x110
 */
GameplayCamera::GameplayCamera()
{
    m_bDynamicZoom = true;
    m_fZoom = 0.0f;
    m_fDesiredZoom = 0.0f;
    m_fZoomSeekSpeed = 0.0f;
    m_ForceNeutralAndNearZoom = false;
    m_matView.SetIdentity();
}

/**
 * Offset/Address/Size: 0xB24 | 0x801AA164 | size: 0x1A4
 */
void GameplayCamera::CalcDynamicZoom()
{
    m_fDesiredZoom = 0.0f;

    for (int i = 0; i < 2; i++)
    {
        int numFieldersInInnerZone = 0;
        int numFieldersInMidZone = 0;

        for (int j = 0; j < 4; j++)
        {
            cFielder* fielder = g_pTeams[i]->GetFielder(j);
            nlVector3 screenPosition = fielder->m_v3ScreenPosition;

            screenPosition.f.x = fabsf(screenPosition.f.x);
            screenPosition.f.y = fabsf(screenPosition.f.y);

            if (screenPosition.f.x <= 0.75f && screenPosition.f.y <= 0.75f)
            {
                numFieldersInInnerZone++;
            }

            if (screenPosition.f.x <= 1.0f && screenPosition.f.y <= 1.0f)
            {
                numFieldersInMidZone++;
            }
        }

        int numAssignedControllers = g_pTeams[i]->GetNumAssignedControllers();
        int teamDesiredNumFieldersOnScreen = 0;
        if (numAssignedControllers != 0)
        {
            teamDesiredNumFieldersOnScreen = numAssignedControllers >= 1 ? numAssignedControllers : 1;
        }

        Goalie* goalie = g_pTeams[i]->GetGoalie();
        if (goalie->m_pBall != NULL)
        {
            teamDesiredNumFieldersOnScreen = teamDesiredNumFieldersOnScreen >= 4 ? teamDesiredNumFieldersOnScreen : 4;
        }

        float currentZoom;
        float teamDesiredZoom = 0.0f;
        if (teamDesiredNumFieldersOnScreen > 0)
        {
            if (numFieldersInInnerZone >= teamDesiredNumFieldersOnScreen)
            {
                teamDesiredZoom = 0.0f;
            }
            else if (numFieldersInMidZone < teamDesiredNumFieldersOnScreen)
            {
                teamDesiredZoom = 1.0f;
            }
            else
            {
                teamDesiredZoom = m_fZoom;
            }
        }

        currentZoom = m_fDesiredZoom;
        currentZoom = (currentZoom >= teamDesiredZoom) ? currentZoom : teamDesiredZoom;
        m_fDesiredZoom = currentZoom;
    }
}

/**
 * Offset/Address/Size: 0x8CC | 0x801A9F0C | size: 0x258
 * TODO: 98.57% match - float register allocation differs (omega=f6 vs target f8,
 * change=f8 vs target f6) due to MWCC graph coloring. All instructions correct.
 * File uses -inline deferred which may contribute to the register allocation difference.
 */
void GameplayCamera::Update(float deltaTime)
{
    m_bDynamicZoom = nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mVisualOptions.mIsAutoZoomCamera;
    m_fDesiredZoom = 1.0f - nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mVisualOptions.mCameraZoomLevel;

    if (nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mVisualOptions.mIsWidescreen)
    {
        m_nearZoom.m_CameraData = gCameraData + 2;
        m_farZoom.m_CameraData = gCameraData + 3;
    }
    else
    {
        m_nearZoom.m_CameraData = gCameraData;
        m_farZoom.m_CameraData = gCameraData + 1;
    }

    m_nearZoom.Update(deltaTime, m_ForceNeutralAndNearZoom);
    m_farZoom.Update(deltaTime, m_ForceNeutralAndNearZoom);

    if (m_ForceNeutralAndNearZoom)
    {
        m_fZoom = 0.0f;
    }
    else
    {
        bool gamePaused = (nlTaskManager::m_pInstance->m_CurrState == 1);

        if (m_bDynamicZoom && !gamePaused && !gGameplayCameraInReplay)
        {
            CalcDynamicZoom();
        }

        float clampedDesiredZoom = Interpolate(0.2f, 0.7f, m_fDesiredZoom);
        float smoothTime;

        if (gamePaused)
        {
            smoothTime = 0.1f;
        }
        else
        {
            smoothTime = 0.75f;
        }

        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1.0f / ((x * (0.235f * x * x)) + ((0.48f * x * x) + (1.0f + x)));
        float change = m_fZoom - clampedDesiredZoom;
        float currentVelocity = m_fZoomSeekSpeed;
        float temp = deltaTime * ((change * omega) + currentVelocity);

        m_fZoomSeekSpeed = exp * (currentVelocity - (omega * temp));
        m_fZoom = (exp * (change + temp)) + clampedDesiredZoom;
    }

    float zoom = m_fZoom;
    float inverseZoom = 1.0f - zoom;

    m_v3Target.f.x = (inverseZoom * m_nearZoom.m_v3Target.f.x) + (zoom * m_farZoom.m_v3Target.f.x);
    m_v3Target.f.y = (inverseZoom * m_nearZoom.m_v3Target.f.y) + (zoom * m_farZoom.m_v3Target.f.y);
    m_v3Target.f.z = (inverseZoom * m_nearZoom.m_v3Target.f.z) + (zoom * m_farZoom.m_v3Target.f.z);

    zoom = m_fZoom;
    inverseZoom = 1.0f - zoom;

    m_v3Camera.f.x = (inverseZoom * m_nearZoom.m_v3Camera.f.x) + (zoom * m_farZoom.m_v3Camera.f.x);
    m_v3Camera.f.y = (inverseZoom * m_nearZoom.m_v3Camera.f.y) + (zoom * m_farZoom.m_v3Camera.f.y);
    m_v3Camera.f.z = (inverseZoom * m_nearZoom.m_v3Camera.f.z) + (zoom * m_farZoom.m_v3Camera.f.z);

    m_fFOV = Interpolate(m_nearZoom.m_CameraData->fov, m_farZoom.m_CameraData->fov, m_fZoom);

    glMatrixLookAt(m_matView, m_v3Camera, m_v3Target, mUpVector);
}

/**
 * Offset/Address/Size: 0x874 | 0x801A9EB4 | size: 0x58
 */
void GameplayCamera::Reactivate()
{
    m_nearZoom.CalcDesiredTarget();
    m_nearZoom.m_fDampenedTargetX = m_nearZoom.m_fDesiredTargetX;
    m_nearZoom.m_fDampenedTargetY = m_nearZoom.m_fDesiredTargetY;

    m_farZoom.CalcDesiredTarget();
    m_farZoom.m_fDampenedTargetX = m_farZoom.m_fDesiredTargetX;
    m_farZoom.m_fDampenedTargetY = m_farZoom.m_fDesiredTargetY;
}

/**
 * Offset/Address/Size: 0x470 | 0x801A9AB0 | size: 0x404
 * TODO: 89.01% match - this in r29 vs r28 (100 diffs), -inline deferred vs -inline auto limitation
 */
void GameplayCameraZoomLevel::CalcDesiredTarget()
{
    int i;
    nlVector3 v3OOIPos = { 0.0f, 0.0f, 0.0f };
    float fKnotTableBlendWeights[3];
    float fCurrWeight;
    float fDampenedBlendRiser;
    float fAccumulatedWeight;
    float fBlendPercent;
    float* pKnotTableBlendWeights = fKnotTableBlendWeights;

    if (gGameplayCameraInReplay == false)
    {
        cBall* pBall = g_pBall;
        if (pBall != NULL)
        {
            cPlayer* pBallOwner = pBall->m_pOwner;
            if (pBallOwner != NULL)
            {
                v3OOIPos = pBallOwner->m_v3Position;
            }
            else
            {
                v3OOIPos = pBall->m_v3Position;
            }
        }
    }
    else
    {
        ReplayManager* pReplayManager = ReplayManager::Instance();
        if (pReplayManager->mRender != NULL)
        {
            v3OOIPos = ReplayManager::Instance()->mRender->mBall.mPosition;
        }
    }

    pKnotTableBlendWeights[0] = 0.0f;
    pKnotTableBlendWeights[1] = 0.0f;
    pKnotTableBlendWeights[2] = 0.0f;

    fCurrWeight = 1.0f;
    for (int j = 0; j < 5; j++)
    {
        fDampenedBlendRiser = m_KnotTableBlendQueue[j].fBlendRiser;
        pKnotTableBlendWeights[m_KnotTableBlendQueue[j].nKnotTable] += fCurrWeight * fDampenedBlendRiser;
        if (1.0f == fDampenedBlendRiser)
        {
            break;
        }
        fCurrWeight *= 1.0f - fDampenedBlendRiser;
    }

    fAccumulatedWeight = 0.0f;
    m_fDesiredTargetX = 0.0f;
    m_fDesiredTargetY = 0.0f;

    for (i = 0; i < 3; i++, pKnotTableBlendWeights++)
    {
        if (*pKnotTableBlendWeights > 0.0f)
        {
            fAccumulatedWeight += *pKnotTableBlendWeights;

            const float* pFieldKnotsX = m_CameraData->fieldKnotsX[i];
            const float* pTargetKnotsX = m_CameraData->targetKnotsX[i];
            int nNumKnotsX = m_CameraData->numKnotsX;

            float fFieldPosX = v3OOIPos.f.x;
            float fXMin = pFieldKnotsX[0];
            float fXMax = pFieldKnotsX[nNumKnotsX - 1] - 0.001f;

            if (fFieldPosX < fXMin)
            {
                fFieldPosX = fXMin;
            }

            if (fFieldPosX > fXMax)
            {
                fFieldPosX = fXMax;
            }

            int nXKnot = 0;
            for (int n = nNumKnotsX - 1; n > 0; n--)
            {
                if (fFieldPosX < pFieldKnotsX[nXKnot + 1])
                {
                    break;
                }
                nXKnot++;
            }

            float fXKnotPercent;
            if (pFieldKnotsX[nXKnot] == pFieldKnotsX[nXKnot + 1])
            {
                fXKnotPercent = 0.0f;
            }
            else
            {
                fXKnotPercent = (fFieldPosX - pFieldKnotsX[nXKnot]) / (pFieldKnotsX[nXKnot + 1] - pFieldKnotsX[nXKnot]);
            }

            float fMappedX = Interpolate(pTargetKnotsX[nXKnot], pTargetKnotsX[nXKnot + 1], fXKnotPercent);

            const float* pFieldKnotsY = m_CameraData->fieldKnotsY[i];
            const float* pTargetKnotsY = m_CameraData->targetKnotsY[i];
            int nNumKnotsY = m_CameraData->numKnotsY;

            float fFieldPosY = v3OOIPos.f.y;
            float fYMin = pFieldKnotsY[0];
            float fYMax = pFieldKnotsY[nNumKnotsY - 1] - 0.001f;

            if (fFieldPosY < fYMin)
            {
                fFieldPosY = fYMin;
            }

            if (fFieldPosY > fYMax)
            {
                fFieldPosY = fYMax;
            }

            int nYKnot = 0;
            for (int n = nNumKnotsY - 1; n > 0; n--)
            {
                if (fFieldPosY < pFieldKnotsY[nYKnot + 1])
                {
                    break;
                }
                nYKnot++;
            }

            float fYKnotPercent;
            if (pFieldKnotsY[nYKnot] == pFieldKnotsY[nYKnot + 1])
            {
                fYKnotPercent = 0.0f;
            }
            else
            {
                fYKnotPercent = (fFieldPosY - pFieldKnotsY[nYKnot]) / (pFieldKnotsY[nYKnot + 1] - pFieldKnotsY[nYKnot]);
            }

            float fMappedY = Interpolate(pTargetKnotsY[nYKnot], pTargetKnotsY[nYKnot + 1], fYKnotPercent);
            fBlendPercent = *pKnotTableBlendWeights / fAccumulatedWeight;

            m_fDesiredTargetX = Interpolate(m_fDesiredTargetX, fMappedX, fBlendPercent);
            m_fDesiredTargetY = Interpolate(m_fDesiredTargetY, fMappedY, fBlendPercent);
        }
    }
}

/**
 * Offset/Address/Size: 0x5C | 0x801A969C | size: 0x414
 * TODO: 99.58% decomp.me match (100% instruction match, offset-only diffs from
 * standalone CalcCurrentKnotTable copy). Real build should be 100%.
 */
static inline void CalcCurrentKnotTable(GameplayCameraZoomLevel* self, bool forceNeutral)
{
    cPlayer* pBallOwner;
    int nNewKnotTable;
    int i;

    if (g_pBall != NULL)
    {
        pBallOwner = g_pBall->m_pOwner;
    }
    else
    {
        pBallOwner = NULL;
    }

    if (pBallOwner == NULL)
    {
        pBallOwner = g_pBall->m_pPassTarget;
    }

    if (pBallOwner != NULL && !forceNeutral)
    {
        if (pBallOwner->m_pTeam->GetOtherNet()->m_baseLocation.f.x > 0.0f)
        {
            nNewKnotTable = 1;
        }
        else
        {
            nNewKnotTable = 2;
        }
    }
    else
    {
        nNewKnotTable = 0;
    }

    if (nNewKnotTable != self->m_KnotTableBlendQueue[0].nKnotTable)
    {
        for (i = 4; i > 0; i--)
        {
            self->m_KnotTableBlendQueue[i] = self->m_KnotTableBlendQueue[i - 1];
        }

        self->m_KnotTableBlendQueue[0].nKnotTable = nNewKnotTable;
        self->m_KnotTableBlendQueue[0].fBlendRiser = 0.0f;
    }
}

void GameplayCameraZoomLevel::Update(float fDeltaT, bool forceNeutral)
{
    float fSin;
    float fCos;
    float fOrientSin;
    float fOrientCos;
    float fXYDist;
    int i;

    if (gGameplayCameraInReplay)
    {
        forceNeutral = true;
    }

    CalcCurrentKnotTable(this, forceNeutral);

    if (forceNeutral)
    {
        m_KnotTableBlendQueue[0].fBlendRiser = 1.0f;
    }

    float t = fDeltaT / 0.75f;
    for (i = 0; i < 5; i++)
    {
        m_KnotTableBlendQueue[i].fBlendRiser += t;
        if (m_KnotTableBlendQueue[i].fBlendRiser >= 1.0f)
        {
            m_KnotTableBlendQueue[i].fBlendRiser = 1.0f;
            break;
        }
    }

    CalcDesiredTarget();

    {
        float omega = 2.0f / m_fTargetSeekTime;
        float x = omega * fDeltaT;
        float exp = 1.0f / ((x * (0.235f * x * x)) + ((0.48f * x * x) + (1.0f + x)));
        float change = m_fDampenedTargetX - m_fDesiredTargetX;
        float currentVelocity = m_fTargetSeekSpeedX;
        float temp = fDeltaT * ((change * omega) + currentVelocity);

        m_fTargetSeekSpeedX = exp * (currentVelocity - (omega * temp));
        m_fDampenedTargetX = (exp * (change + temp)) + m_fDesiredTargetX;
    }

    {
        float omega = 2.0f / m_fTargetSeekTime;
        float x = omega * fDeltaT;
        float exp = 1.0f / ((x * (0.235f * x * x)) + ((0.48f * x * x) + (1.0f + x)));
        float change = m_fDampenedTargetY - m_fDesiredTargetY;
        float currentVelocity = m_fTargetSeekSpeedY;
        float temp = fDeltaT * ((change * omega) + currentVelocity);

        m_fTargetSeekSpeedY = exp * (currentVelocity - (omega * temp));
        m_fDampenedTargetY = (exp * (change + temp)) + m_fDesiredTargetY;
    }

    nlSinCos(&fSin, &fCos, ((s32)(65536.0f * m_CameraData->pitch)) / 360);
    nlSinCos(&fOrientSin, &fOrientCos, ((s32)(65536.0f * m_CameraData->orientation)) / 360);

    fXYDist = fCos * m_CameraData->distance;

    m_v3Camera.f.x = (fOrientCos * fXYDist) + m_fDampenedTargetX;
    m_v3Camera.f.y = (fOrientSin * fXYDist) + m_fDampenedTargetY;
    m_v3Camera.f.z = fSin * m_CameraData->distance;

    float dampenedY = m_fDampenedTargetY;
    float dampenedX = m_fDampenedTargetX;
    m_v3Target.f.x = dampenedX;
    m_v3Target.f.y = dampenedY;
    m_v3Target.f.z = 0.0f;
}

/**
 * Offset/Address/Size: 0x0 | 0x801A9640 | size: 0x5C
 */
GameplayCamera::~GameplayCamera()
{
}
