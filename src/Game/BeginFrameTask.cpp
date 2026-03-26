#include "Game/BeginFrameTask.h"

#include "Game/Debug/FrameCounter.h"
#include "Game/Drawable/DrawableModel.h"
#include "Game/NisPlayer.h"
#include "Game/Render/depthoffield.h"
#include "Game/main.h"
#include "NL/gl/glState.h"
#include "NL/gl/glUserData.h"
#include "Game/GameObjectLighting.h"

#include "NL/nlConfig.h"
#include "NL/nlFileGC.h"
#include "NL/gl/gl.h"
#include "NL/gl/glConstant.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glFont.h"
#include "NL/glx/glxTexture.h"

class cCameraManager
{
public:
    static void Update(float);
};

const u32 GLTT_BumpLocal_bit = 1 << (int)GLTT_BumpLocal;
static float dimx = 48.0f;
static float dimy = 28.0f;
static float offx;
static float offy;

u8 g_bCoPlanarRefVisible;
u8 g_bCoPlanarDepthTest;
u8 g_bCoPlanarDepthWrite;
u8 g_bFrameSmiler;
u8 g_bFrameStatsOnScreen;
static u8 g_bDrawSafeFrame;
static s32 g_nGridDisplaySpacing;
static s32 g_eWaitMode;
static u8 bGotWait;
static s8 init;
// /**
//  * Offset/Address/Size: 0xA8 | 0x80170214 | size: 0x84
//  */
// void Config::TagValuePair::Get<BasicString<char, Detail::TempStringAllocator>>() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8017016C | size: 0xA8
//  */
// void Config::Get<BasicString<char, Detail::TempStringAllocator>>(const char*, BasicString<char, Detail::TempStringAllocator>)
// {
// }

/**
 * Offset/Address/Size: 0x19E8 | 0x801700C8 | size: 0x98
 */
glModel* cb_ParticleLighting(glModel* pModel)
{
    glModelPacket* pPacket; // r27
    u32 LitProgram = glGetProgram("3d pointlit");
    u32 lightRamp = GetGameObjectLightRamp();
    void* pLightData = GetInGameLightData();
    pPacket = pModel->packets;

    while (pPacket < &pModel->packets[pModel->numPackets])
    {
        pPacket->state.program = LitProgram;
        pPacket->state.texture[5] = lightRamp;
        pPacket->state.texconfig |= GLTT_BumpLocal_bit;
        glUserAttach(pLightData, pPacket, 0);
        pPacket++;
    }
    return pModel;
}

/**
 * Offset/Address/Size: 0x14B8 | 0x8016FB98 | size: 0x530
 */
void SetupMatrices()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x1068 | 0x8016F748 | size: 0x450
 * TODO: 97.72% match - r30/r31 register swap in BasicString constructor (MWCC
 * allocates differently for temporary vs named variable), missing lwz r0 reload
 * and r0/r4 register difference after Config::Get return value copy-init.
 */
void SetupRenderInfo()
{
    extern s32 m_ModelType__10cCharacter;
    extern void* s_World__12WorldManager;
    extern s32 s_GameplaySkin__14BeginFrameTask;
    extern s32 s_ReplaySkin__14BeginFrameTask;
    extern u8* g_pGame;

    if (s_GameplaySkin__14BeginFrameTask != 2)
    {
        m_ModelType__10cCharacter = (s_GameplaySkin__14BeginFrameTask != 0);
    }
    else
    {
        if (s_World__12WorldManager != NULL)
        {
            s32 state = nlTaskManager::m_pInstance->m_CurrState;
            if (state == 1)
            {
                state = nlTaskManager::m_pInstance->m_PrevState;
            }

            switch (state)
            {
            case 0x10:
            case 0x100:
                m_ModelType__10cCharacter = 1;
                break;
            case 0x20000:
                if (s_ReplaySkin__14BeginFrameTask == 0)
                {
                    m_ModelType__10cCharacter = 0;
                }
                else
                {
                    m_ModelType__10cCharacter = 1;
                }
                break;
            default:
                m_ModelType__10cCharacter = 0;
                break;
            }
        }
        else
        {
            m_ModelType__10cCharacter = 0;
        }
    }

    if (g_pGame != NULL)
    {
        if (g_pGame[0x40] != 0)
        {
            m_ModelType__10cCharacter = 1;
        }
    }

    if (!init)
    {
        bGotWait = 0;
        init = 1;
    }

    if (!bGotWait)
    {
        BasicString<char, Detail::TempStringAllocator> waitMode = Config::Global().Get<BasicString<char, Detail::TempStringAllocator> >("wait_vblank", BasicString<char, Detail::TempStringAllocator>("default"));

        if (waitMode == "never")
        {
            g_eWaitMode = 1;
        }
        else if (waitMode == "always")
        {
            g_eWaitMode = 2;
        }
        else if (waitMode == "float" || waitMode == "floating")
        {
            g_eWaitMode = 3;
        }
        else
        {
            g_eWaitMode = 0;
        }

        bGotWait = 1;
    }

    nlVector4 vwait;

    if (g_eWaitMode == 0 || g_eWaitMode == 3)
    {
        if (BeginFrameTask::s_FramerateLocked && nlTaskManager::m_pInstance->m_CurrState == 0x100)
        {
            vwait.f.x = 2.0f;
            vwait.f.y = 2.0f;
            vwait.f.z = 2.0f;
            vwait.f.w = 2.0f;
        }
        else
        {
            float vwaitValue;
            if (g_eWaitMode == 3)
            {
                vwaitValue = 0.5f;
            }
            else
            {
                vwaitValue = 1.0f;
            }

            switch (nlTaskManager::m_pInstance->m_CurrState)
            {
            case 0x10:
            case 4:
            case 0x100:
                vwait.f.x = vwaitValue;
                vwait.f.y = vwaitValue;
                vwait.f.z = vwaitValue;
                vwait.f.w = vwaitValue;
                break;
            default:
                vwait.f.x = vwaitValue;
                vwait.f.y = vwaitValue;
                vwait.f.z = vwaitValue;
                vwait.f.w = vwaitValue;
                break;
            }
        }
    }
    else if (g_eWaitMode == 1)
    {
        vwait.f.x = 0.0f;
        vwait.f.y = 0.0f;
        vwait.f.z = 0.0f;
        vwait.f.w = 0.0f;
    }
    else
    {
        vwait.f.x = 1.0f;
        vwait.f.y = 1.0f;
        vwait.f.z = 1.0f;
        vwait.f.w = 1.0f;
    }

    glConstantSet("glxswap/vwait", vwait);
}

/**
 * Offset/Address/Size: 0x950 | 0x8016F030 | size: 0x718
 */
void DrawSafeFrame()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x410 | 0x8016EAF0 | size: 0x540
 */
void DrawGrid(int)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x0 | 0x8016E6E0 | size: 0x410
 * TODO: 99.85% match - remaining diffs are i-only symbol/offset immediates for
 * function-local statics in this file.
 */
void BeginFrameTask::Run(float dt)
{
    extern u8 g_bCoPlanarPerObject;
    extern volatile u8 m_AllowInFront__14ParticleSystem;

    g_FrameCounter.StartTimer(0);

    if (g_bFrameSmiler)
    {
        g_FrameCounter.DisplayFrameSmiler();
    }

    NisPlayer::Instance()->Update(dt);
    NisPlayer::Instance()->Render();
    cCameraManager::Update(dt);

    if ((nlTaskManager::m_pInstance->m_CurrState & 0x110) != 0)
    {
        DepthOfFieldManager::instance.TurnOn();
    }
    else
    {
        DepthOfFieldManager::instance.TurnOff();
    }

    glBeginFrame();
    SetupMatrices();
    SetupRenderInfo();
    nlServiceFileSystem();

    if (!g_bCoPlanarPerObject)
    {
        static u32 BlackTexture;
        static s8 init;

        if (!init)
        {
            BlackTexture = glGetTexture("global/black");
            init = 1;
        }

        float z = GetCoPlanarZ();
        float dx = dimx;
        float dy = dimy;
        float nx = -dx;
        float ox = offx;
        float oy = offy;
        float ny = -dy;

        float right = dx + ox;
        float left = nx + ox;
        float bottom = ny + oy;
        float top = dy + oy;

        nlVector3 points[4] = { 0 };

        points[0].f.x = left;
        points[0].f.y = bottom;
        points[0].f.z = z;
        points[1].f.x = right;
        points[1].f.y = bottom;
        points[1].f.z = z;
        points[2].f.x = right;
        points[2].f.y = top;
        points[2].f.z = z;
        points[3].f.x = left;
        points[3].f.y = top;
        points[3].f.z = z;

        glSetDefaultState(false);
        glSetRasterState(GLS_Culling, g_bCoPlanarRefVisible ? 1 : 3);
        glSetRasterState(GLS_DepthTest, g_bCoPlanarDepthTest);
        glSetRasterState(GLS_DepthWrite, g_bCoPlanarDepthWrite);
        glSetCurrentRasterState(glHandleizeRasterState());
        glSetCurrentTexture(BlackTexture, GLTT_Diffuse);

        glQuad3 q;
        q.m_pos[0] = points[0];
        q.m_uv[0].f.x = 0.0f;
        q.m_uv[0].f.y = 0.0f;
        q.m_pos[1] = points[1];
        q.m_uv[1].f.x = 0.0f;
        q.m_uv[1].f.y = 0.0f;
        q.m_pos[2] = points[2];
        q.m_uv[2].f.x = 0.0f;
        q.m_uv[2].f.y = 0.0f;
        q.m_pos[3] = points[3];
        q.m_uv[3].f.x = 0.0f;
        q.m_uv[3].f.y = 0.0f;

        q.SetColour(0xFF, 0xFF, 0xFF, 0xFF);
        q.Attach(GLV_CoPlanar, 0, true);

        z = GetCoPlanar0Z();
        q.m_pos[0].f.z = z;
        q.m_pos[1].f.z = z;
        q.m_pos[2].f.z = z;
        q.m_pos[3].f.z = z;
        q.Attach(GLV_CoPlanar0, 0, true);
    }

    switch (nlTaskManager::m_pInstance->m_CurrState)
    {
    case 0x10:
        m_AllowInFront__14ParticleSystem = 0;
        break;
    default:
        m_AllowInFront__14ParticleSystem = 1;
        break;
    }

    if (g_bDrawSafeFrame)
    {
        DrawSafeFrame();
    }

    if (g_nGridDisplaySpacing > 0)
    {
        DrawGrid(g_nGridDisplaySpacing);
    }

    static u8 showRegion;
    static s8 init;

    if (!init)
    {
        Config& cfg = Config::Global();
        TagValuePair& tvp = cfg.FindTvp("show_region");

        bool regionVisible;
        if (tvp.tag == 0)
        {
            cfg.Set("show_region", false);
            regionVisible = false;
        }
        else if (tvp.type == _BOOL)
        {
            regionVisible = LexicalCast<bool, bool>(tvp.value.b);
        }
        else if (tvp.type == _INT)
        {
            regionVisible = LexicalCast<bool, int>(tvp.value.i);
        }
        else if (tvp.type == _FLOAT)
        {
            regionVisible = LexicalCast<bool, float>(tvp.value.f);
        }
        else if (tvp.type == _STRING)
        {
            regionVisible = LexicalCast<bool, const char*>(tvp.value.s);
        }
        else
        {
            regionVisible = false;
        }

        showRegion = regionVisible;
        init = 1;
    }

    if (showRegion)
    {
        glFontBegin(false);
        nlColour white = { 0xFF, 0xFF, 0xFF, 0xFF };
        glFontPrintf((eGLView)0x21, 1, 1, white, "Region %d", *(int*)GetRegion());
        glFontEnd();
    }
}
