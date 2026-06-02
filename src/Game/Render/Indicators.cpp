#include "Game/Render/Indicators.h"

#include "Game/AI/AiUtil.h"
#include "Game/CharacterTemplate.h"
#include "Game/CharacterTweaks.h"
#include "Game/Game.h"
#include "Game/Player.h"
#include "Game/ReplayManager.h"
#include "NL/gl/glDraw2.h"
#include "NL/gl/glState.h"
#include "NL/gl/glTexture.h"
#include "NL/gl/glView.h"
#include "NL/globalpad.h"
#include "NL/nlColour.h"
#include "NL/nlMath.h"
#include "types.h"

static float s_fOverheadSize = 35.0f;
static float s_fAdditiveBlendingIntensity = 0.55f;
static float s_fAdditiveTextureScale = 1.25f;
static float s_fPulseRate = 2.0f;
static unsigned char s_bPulseGlowTexture = 1;
static float s_fGlowIntensityScale;
static unsigned char s_bGlowIsRising;

typedef struct
{
    char pad0[0x19C];
    float mSwitchScale;
    char pad1[0x28];
    void* mPossessionObject;
} cPlayerIndicatorState;

typedef struct
{
    void* vtbl;
    void* mGameTweaks;
} cGameOverlay;

typedef struct
{
    char pad0[0x2B4];
    float mVerticalOffset;
    float mProjectionYOffset;
} GameTweaksOverlay;

typedef struct
{
    nlVector3 mPosition;
    char pad0[0x4C];
} ReplayIndicatorCharacter;

typedef struct
{
    char pad0[0x68];
    ReplayIndicatorCharacter mCharacters[10];
} ReplayIndicatorSnapshot;

unsigned long uIndicatorTexID[4];
unsigned long uGlowTexID[4];

namespace
{
struct IndicatorsInit
{
    IndicatorsInit()
    {
        uIndicatorTexID[0] = nlStringLowerHash("fe/controller_1_indicator");
        uIndicatorTexID[1] = nlStringLowerHash("fe/controller_2_indicator");
        uIndicatorTexID[2] = nlStringLowerHash("fe/controller_3_indicator");
        uIndicatorTexID[3] = nlStringLowerHash("fe/controller_4_indicator");
        uGlowTexID[0] = nlStringLowerHash("fe/controller_1_indicator_E");
        uGlowTexID[1] = nlStringLowerHash("fe/controller_2_indicator_E");
        uGlowTexID[2] = nlStringLowerHash("fe/controller_3_indicator_E");
        uGlowTexID[3] = nlStringLowerHash("fe/controller_4_indicator_E");
    }
};
IndicatorsInit s_indicatorsInit;
} // namespace

float fMaxAlpha = 0.9f;
float fOpacityFadePerSecond = 1.2f;

class IndicatorInfo
{
public:
    float m_fOpacity;

    inline void IncrementOnscreenTimer(float fTimeDelta)
    {
        m_fOpacity -= fTimeDelta * fOpacityFadePerSecond;
        if (m_fOpacity < 0.0f)
        {
            m_fOpacity = 0.0f;
        }
    }

    inline void IncrementOffscreenTimer(float fTimeDelta)
    {
        m_fOpacity += fTimeDelta * fOpacityFadePerSecond;
        if (m_fOpacity > fMaxAlpha)
        {
            m_fOpacity = fMaxAlpha;
        }
    }
};

IndicatorInfo indicatorInfo[10];

/**
 * Offset/Address/Size: 0x868 | 0x8015FACC | size: 0x440
 * TODO: 95.82% match - f29/f30 swap (dt vs x callee-saved register allocation),
 * r3/r4 swap (g_pGame/drawIndicator), mr vs li constant propagation for insideXY
 */
void UpdateAndRenderOffScreenIndicators(float dt)
{
    nlVector3 worldPos = { 0 };
    float half;
    int i;

    half = 0.5f;

    for (i = 0; i < 10; i++)
    {
        nlVector3 projectedPos;
        ReplayManager* replay;

        replay = ReplayManager::Instance();
        if (replay->mRender != 0)
        {
            replay = ReplayManager::Instance();
            worldPos = replay->mRender->mCharacters[i].mPosition;
        }

        {
            float h = ((cPlayer*)g_pCharacters[i])->m_pTweaks->fPhysCapsuleHeight;
            worldPos.f.z += h * half;
        }

        glViewProjectPoint((eGLView)7, worldPos, projectedPos);
        ((cPlayer*)g_pCharacters[i])->m_v3ScreenPosition = projectedPos;

        if (((cPlayer*)g_pCharacters[i])->GetGlobalPad() != 0)
        {
            cPlayer* pChar;
            u8 insideXY;
            u8 insideXYZ;
            float fOne;
            u8 drawIndicator;

            pChar = (cPlayer*)g_pCharacters[i];
            insideXYZ = 0;
            fOne = 1.0f;
            insideXY = insideXYZ;

            if ((float)__fabs(pChar->m_v3ScreenPosition.f.x) <= fOne
                && (float)__fabs(pChar->m_v3ScreenPosition.f.y) <= fOne)
            {
                insideXY = 1;
            }

            if (insideXY)
            {
                if ((float)__fabs(pChar->m_v3ScreenPosition.f.z) <= fOne)
                {
                    insideXYZ = 1;
                }
            }

            if (insideXYZ)
                goto do_fadeout;

            drawIndicator = 0;
            if (g_pGame->m_eGameState == 4 || g_pGame->m_eGameState == 5)
            {
                drawIndicator = 1;
            }

            if (!drawIndicator)
            {
            do_fadeout:
                indicatorInfo[i].IncrementOnscreenTimer(dt);
            }
            else
            {
                float x;
                float y;
                cPlayer* pCharDraw;
                float absX;
                float absY;
                float size;
                unsigned long texID;
                int yPixels;
                int xPixels;

                indicatorInfo[i].IncrementOffscreenTimer(dt);

                x = 320.0f * projectedPos.f.x;
                y = 240.0f * projectedPos.f.y;
                pCharDraw = (cPlayer*)g_pCharacters[i];

                if (x < -288.0f)
                {
                    x = -288.0f;
                }
                else if (x > 288.0f)
                {
                    x = 288.0f;
                }

                if (y < -208.0f)
                {
                    y = -208.0f;
                }
                else if (y > 208.0f)
                {
                    y = 208.0f;
                }

                absX = (float)__fabs(projectedPos.f.x);
                absY = (float)__fabs(projectedPos.f.y);

                x = x + 320.0f;
                y = y + 240.0f;

                if (absX < absY)
                {
                    absX = absY;
                }

                size = InterpolateRangeClamped(1.0f, 0.5f, 0.0f, 2.0f, (float)__fabs(1.0f - absX));
                texID = uIndicatorTexID[pCharDraw->GetGlobalPad()->m_padIndex];
                yPixels = (int)y;
                xPixels = (int)x;

                size = 64.0f * size;
                {
                    float opacity = indicatorInfo[i].m_fOpacity;

                    if ((u8)glTextureLoad(texID))
                    {
                        static nlColour cInit;
                        glPoly2 quad;
                        nlColour c;

                        glSetDefaultState(0);
                        glSetRasterState(GLS_AlphaBlend, 1);
                        glSetRasterState(GLS_AlphaTest, 1);
                        glSetRasterState(GLS_AlphaTestRef, 0);
                        glSetCurrentRasterState(glHandleizeRasterState());
                        glSetCurrentTexture(texID, (eGLTextureType)0);
                        glTextureGetWidth();
                        glTextureGetHeight();

                        quad.SetupRotatedRectangle((float)xPixels, (float)yPixels, size, size, 0.0f, 10000000000.0f);

                        c = cInit;
                        c.c[0] = 0xFF;
                        c.c[1] = 0xFF;
                        c.c[2] = 0xFF;
                        c.c[3] = (unsigned char)(255.0f * opacity);
                        quad.SetColour(c);

                        quad.depth = -0.5f;
                        quad.Attach((eGLView)27, 0, 0, (unsigned long)-1);
                    }
                }
            }
        }
    }
}

static void DrawIndicator(int xCentre, int yCentre, float fPixelWidth, float fPixelHeight, float fOpacity, unsigned long uTexID,
    float rotationAngle, unsigned char additiveBlending)
{
    if ((u8)glTextureLoad(uTexID))
    {
        static nlColour cInit;
        glPoly2 poly;
        nlColour c;

        glSetDefaultState(0);
        glSetRasterState(GLS_AlphaBlend, additiveBlending);
        glSetRasterState(GLS_AlphaTest, 1);
        glSetRasterState(GLS_AlphaTestRef, 0);
        glSetCurrentRasterState(glHandleizeRasterState());
        glSetCurrentTexture(uTexID, (eGLTextureType)0);
        glTextureGetWidth();
        glTextureGetHeight();

        poly.SetupRotatedRectangle((float)xCentre, (float)yCentre, fPixelWidth, fPixelHeight, rotationAngle, 10000000000.0f);

        c = cInit;
        if (additiveBlending == 2)
        {
            unsigned char intensity = (unsigned char)(255.0f * fOpacity);
            c.c[0] = intensity;
            c.c[1] = intensity;
            c.c[2] = intensity;
            c.c[3] = 0xFF;
        }
        else
        {
            c.c[0] = 0xFF;
            c.c[1] = 0xFF;
            c.c[2] = 0xFF;
            c.c[3] = (unsigned char)(255.0f * fOpacity);
        }
        poly.SetColour(c);

        poly.depth = -0.5f;
        poly.Attach((eGLView)27, 0, 0, (unsigned long)-1);
    }
}

/**
 * Offset/Address/Size: 0xA8 | 0x8015F30C | size: 0x7C0
 * TODO: 98.26% match - remaining diffs are callee-saved integer register
 * allocation rotation (r25-r31 shifted by 2 positions).
 */
void UpdateAndRenderPlayerIndicators(float)
{
    static int whoHadBall;
    static signed char init;
    cGlobalPad* pController;
    nlVector3 v3Position;
    nlVector3 v3ScreenPosition;
    float fX;
    float fY;
    float fOpacity;
    int whoHasBall;
    int i;
    float fDistInPixels;
    float switchScale;
    float sizeScale;

    whoHasBall = -1;
    if (!init)
    {
        whoHadBall = -1;
        init = 1;
    }

    for (i = 0; i < 10; i++)
    {
        ReplayManager* pReplay;
        ReplayIndicatorSnapshot* pSnapshot;
        GameTweaksOverlay* pTweaks;
        unsigned long indicatorTexID;
        unsigned long glowTexID;

        if (((cPlayer*)g_pCharacters[i])->GetGlobalPad() == 0)
        {
            continue;
        }

        fOpacity = 1.0f - indicatorInfo[i].m_fOpacity;
        if (fOpacity <= 0.011764706f)
        {
            continue;
        }

        pController = ((cPlayer*)g_pCharacters[i])->GetGlobalPad();
        indicatorTexID = uIndicatorTexID[pController->m_padIndex];
        pController = ((cPlayer*)g_pCharacters[i])->GetGlobalPad();
        glowTexID = uGlowTexID[pController->m_padIndex];

        pReplay = ReplayManager::Instance();
        pSnapshot = (ReplayIndicatorSnapshot*)pReplay->mRender;
        v3Position = pSnapshot->mCharacters[i].mPosition;

        pTweaks = (GameTweaksOverlay*)((cGameOverlay*)g_pGame)->mGameTweaks;
        switchScale = pTweaks->mProjectionYOffset;
        v3Position.f.z += pTweaks->mVerticalOffset;

        glViewProjectPoint((eGLView)7, v3Position, v3ScreenPosition);

        fY = 240.0f * v3ScreenPosition.f.y + 240.0f;
        fX = 320.0f * v3ScreenPosition.f.x + 320.0f;
        fY -= switchScale;

        if (((cPlayerIndicatorState*)g_pCharacters[i])->mSwitchScale < 0.5f)
        {
            switchScale = (0.5f - ((cPlayerIndicatorState*)g_pCharacters[i])->mSwitchScale) / 0.5f;
            sizeScale = Interpolate(1.0f, 2.0f, switchScale);

            DrawIndicator((int)fX, (int)fY, s_fAdditiveTextureScale * (s_fOverheadSize * sizeScale), s_fAdditiveTextureScale * (s_fOverheadSize * sizeScale), s_fAdditiveBlendingIntensity * switchScale, glowTexID, 0.0f, 2);

            fDistInPixels = s_fOverheadSize * sizeScale;
            DrawIndicator((int)fX, (int)fY, fDistInPixels, fDistInPixels, fOpacity, indicatorTexID, 0.0f, 1);
        }
        else if (((cPlayerIndicatorState*)g_pCharacters[i])->mPossessionObject)
        {
            whoHasBall = i;

            if (whoHadBall == -1)
            {
                s_fGlowIntensityScale = 0.0f;
            }

            switchScale = 1.0f;
            if (s_bPulseGlowTexture)
            {
                switchScale = s_fGlowIntensityScale;
            }

            fDistInPixels = s_fOverheadSize;
            DrawIndicator((int)fX, (int)fY, s_fAdditiveTextureScale * fDistInPixels, s_fAdditiveTextureScale * fDistInPixels, s_fAdditiveBlendingIntensity * switchScale, glowTexID, 0.0f, 2);
            DrawIndicator((int)fX, (int)fY, fDistInPixels, fDistInPixels, fOpacity, indicatorTexID, 0.0f, 1);
        }
        else
        {
            fDistInPixels = s_fOverheadSize;
            DrawIndicator((int)fX, (int)fY, fDistInPixels, fDistInPixels, fOpacity, indicatorTexID, 0.0f, 1);
        }
    }

    whoHadBall = whoHasBall;
}

/**
 * Offset/Address/Size: 0x0 | 0x8015F264 | size: 0xA8
 */
void UpdateAndRenderIndicators(float dt)
{
    UpdateAndRenderOffScreenIndicators(dt);
    UpdateAndRenderPlayerIndicators(dt);

    if (s_bPulseGlowTexture)
    {
        if (s_bGlowIsRising)
        {
            s_fGlowIntensityScale += s_fPulseRate * dt;
            if (s_fGlowIntensityScale > 1.0f)
            {
                s_fGlowIntensityScale = 1.0f;
                s_bGlowIsRising = 0;
            }
        }
        else
        {
            s_fGlowIntensityScale -= s_fPulseRate * dt;
            if (s_fGlowIntensityScale < 0.0f)
            {
                s_fGlowIntensityScale = 0.0f;
                s_bGlowIsRising = 1;
            }
        }
    }
}
