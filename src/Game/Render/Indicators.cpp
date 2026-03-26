#include "Game/Render/Indicators.h"
#include "types.h"

static float s_fPulseRate;
static unsigned char s_bPulseGlowTexture;
static float s_fGlowIntensityScale;
static unsigned char s_bGlowIsRising;

typedef struct
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        } f;
        unsigned long as_u32[3];
    } u;
} nlVector3;

typedef struct
{
    unsigned char c[4];
} nlColour;

typedef struct
{
    char pad0[0x50];
    float depth;
} glPoly2;

typedef struct
{
    char pad0[0x44];
    float fPhysCapsuleHeight;
} PlayerTweaks;

typedef struct
{
    char pad0[0x120];
    nlVector3 m_v3ScreenPosition;
    char pad1[0x98];
    PlayerTweaks* m_pTweaks;
} cPlayer;

typedef struct
{
    void* vtbl;
    int m_padIndex;
} cGlobalPad;

typedef struct
{
    char pad0[4];
    nlVector3 mPosition;
    char pad1[0x48];
} DrawableCharacter;

typedef struct
{
    char pad0[0x4C];
    DrawableCharacter mCharacters[10];
} RenderSnapshot;

typedef struct
{
    char pad0[0x5048];
    RenderSnapshot* mRender;
} ReplayManager;

typedef struct
{
    char pad0[0x24];
    int m_eGameState;
} cGame;

extern void* g_pCharacters[10];
extern cGame* g_pGame;

extern ReplayManager* Instance__13ReplayManagerFv(void);
extern cGlobalPad* GetGlobalPad__7cPlayerFv(cPlayer*);
extern void glViewProjectPoint__F7eGLViewRC9nlVector3R9nlVector3(int, const nlVector3*, nlVector3*);
extern float InterpolateRangeClamped__Ffffff(float, float, float, float, float);
extern int glTextureLoad__FUl(unsigned long);
extern void glSetDefaultState__Fb(int);
extern unsigned long glSetRasterState__F8eGLStateUl(int, unsigned long);
extern unsigned long glHandleizeRasterState__Fv(void);
extern unsigned long glSetCurrentRasterState__FUl(unsigned long);
extern unsigned long glSetCurrentTexture__FUl14eGLTextureType(unsigned long, int);
extern unsigned long glTextureGetWidth__Fv(void);
extern unsigned long glTextureGetHeight__Fv(void);
extern void SetupRotatedRectangle__7glPoly2Fffffff(glPoly2*, float, float, float, float, float, float);
extern void SetColour__7glPoly2FRC8nlColour(glPoly2*, const nlColour*);
extern int Attach__7glPoly2F7eGLViewiPUlUl(glPoly2*, int, int, unsigned long*, unsigned long);

unsigned long uIndicatorTexID[4];
float indicatorInfo[10];
float fMaxAlpha = 0.9f;
float fOpacityFadePerSecond = 1.2f;

/**
 * Offset/Address/Size: 0x868 | 0x8015FACC | size: 0x440
 * TODO: 91.55% match - remaining 15 diffs are -inline deferred artifacts:
 * f29/f30 swap (dt vs x register allocation), r3/r4 swap (g_pGame/drawIndicator),
 * mr vs li constant propagation, and branch offset consequence.
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

        replay = Instance__13ReplayManagerFv();
        if (replay->mRender != 0)
        {
            replay = Instance__13ReplayManagerFv();
            worldPos = replay->mRender->mCharacters[i].mPosition;
        }

        {
            float h = ((cPlayer*)g_pCharacters[i])->m_pTweaks->fPhysCapsuleHeight;
            worldPos.u.f.z += h * half;
        }

        glViewProjectPoint__F7eGLViewRC9nlVector3R9nlVector3(7, &worldPos, &projectedPos);
        ((cPlayer*)g_pCharacters[i])->m_v3ScreenPosition = projectedPos;

        if (GetGlobalPad__7cPlayerFv((cPlayer*)g_pCharacters[i]) != 0)
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

            if ((float)__fabs(pChar->m_v3ScreenPosition.u.f.x) <= fOne
                && (float)__fabs(pChar->m_v3ScreenPosition.u.f.y) <= fOne)
            {
                insideXY = 1;
            }

            if (insideXY)
            {
                if ((float)__fabs(pChar->m_v3ScreenPosition.u.f.z) <= fOne)
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
                indicatorInfo[i] -= dt * fOpacityFadePerSecond;
                if (indicatorInfo[i] < 0.0f)
                {
                    indicatorInfo[i] = 0.0f;
                }
            }
            else
            {
                float x;
                float y;
                float absX;
                float absY;
                float size;
                unsigned long texID;
                int xPixels;
                int yPixels;

                indicatorInfo[i] += dt * fOpacityFadePerSecond;
                if (indicatorInfo[i] > fMaxAlpha)
                {
                    indicatorInfo[i] = fMaxAlpha;
                }

                x = 320.0f * projectedPos.u.f.x;
                y = 240.0f * projectedPos.u.f.y;

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

                absX = (float)__fabs(projectedPos.u.f.x);
                absY = (float)__fabs(projectedPos.u.f.y);

                x = x + 320.0f;
                y = y + 240.0f;

                if (!(absX >= absY))
                {
                    absX = absY;
                }

                size = InterpolateRangeClamped__Ffffff(1.0f, 0.5f, 0.0f, 2.0f, (float)__fabs(1.0f - absX));
                texID = uIndicatorTexID[GetGlobalPad__7cPlayerFv((cPlayer*)g_pCharacters[i])->m_padIndex];
                yPixels = (int)y;
                xPixels = (int)x;

                size = 64.0f * size;

                if ((u8)glTextureLoad__FUl(texID))
                {
                    static nlColour cInit;
                    glPoly2 quad;
                    nlColour c;

                    glSetDefaultState__Fb(0);
                    glSetRasterState__F8eGLStateUl(5, 1);
                    glSetRasterState__F8eGLStateUl(3, 1);
                    glSetRasterState__F8eGLStateUl(4, 0);
                    glSetCurrentRasterState__FUl(glHandleizeRasterState__Fv());
                    glSetCurrentTexture__FUl14eGLTextureType(texID, 0);
                    glTextureGetWidth__Fv();
                    glTextureGetHeight__Fv();

                    SetupRotatedRectangle__7glPoly2Fffffff(&quad, (float)xPixels, (float)yPixels, size, size, 0.0f, 10000000000.0f);

                    c = cInit;
                    c.c[0] = 0xFF;
                    c.c[1] = 0xFF;
                    c.c[2] = 0xFF;
                    c.c[3] = (unsigned char)(255.0f * indicatorInfo[i]);
                    SetColour__7glPoly2FRC8nlColour(&quad, &c);

                    quad.depth = -0.5f;
                    Attach__7glPoly2F7eGLViewiPUlUl(&quad, 27, 0, 0, (unsigned long)-1);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xA8 | 0x8015F30C | size: 0x7C0
 */
void UpdateAndRenderPlayerIndicators(float)
{
    FORCE_DONT_INLINE;
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
