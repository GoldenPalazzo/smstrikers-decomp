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
#include "NL/nlDLRing.h"
#include "NL/gl/gl.h"
#include "NL/gl/glConstant.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glFont.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glView.h"
#include "NL/glx/glxTexture.h"
#include "Game/GL/gluMeshWriter.h"

#include "Game/Camera/BaseCamera.h"
#include "Game/GameInfo.h"

class cCameraManager
{
public:
    static void Update(float);

    static cBaseCamera* m_cameraStack;
    static nlMatrix4 m_matView;
    static float m_fFOV;
    static int m_pBeginFrameCameraType;
};

extern glModel* (*m_LightingCallback__14ParticleSystem)(glModel*);

static inline float GetAspectRatio()
{
    if (GameInfoManager::s_pInstance->mUserInfo.mVisualOptions.mIsWidescreen)
    {
        return 1.666f;
    }
    bool bWide = false;
    u32 currState = nlTaskManager::m_pInstance->m_CurrState;
    if (currState == 0x100 || (currState == 1 && nlTaskManager::m_pInstance->m_PrevState == 0x100))
    {
        bWide = true;
    }
    if (bWide)
    {
        return 1.3323944f;
    }
    return 1.25f;
}

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
    nlMatrix4 ortho;
    glMatrixOrthographicCentered(ortho, 640.0f, 480.0f, 0.0f, 5000.0f);
    u32 hOrtho = glAllocMatrix();
    if (hOrtho != 0xFFFFFFFF)
    {
        glSetMatrix(hOrtho, ortho);
    }
    glViewSetProjectionMatrix(GLV_Anark, hOrtho);

    glMatrixOrthographic(ortho, 640.0f, 480.0f);
    u32 hFlat = glAllocMatrix();
    if (hFlat != 0xFFFFFFFF)
    {
        glSetMatrix(hFlat, ortho);
    }
    glViewSetProjectionMatrix(GLV_ShadowBlend0, hFlat);
    glViewSetProjectionMatrix(GLV_ShadowBlend1, hFlat);
    glViewSetProjectionMatrix(GLV_FrontEnd, hFlat);
    glViewSetProjectionMatrix(GLV_Debug, hFlat);
    glViewSetProjectionMatrix(GLV_Transitions, hFlat);
    glViewSetProjectionMatrix(GLV_WarbleBlend, hFlat);
    glViewSetProjectionMatrix(GLV_DepthOfField, hFlat);
    glViewSetProjectionMatrix(GLV_BigBlackPolygon, hFlat);
    glViewSetProjectionMatrix(GLV_UnsortedOrtho, hFlat);

    float fAspect = GetAspectRatio();

    nlMatrix4 proj;
    glMatrixPerspective(proj, 0.4712389f, fAspect, 0.25f, 1024.0f);
    u32 hPersp = glAllocMatrix();
    if (hPersp != 0xFFFFFFFF)
    {
        glSetMatrix(hPersp, proj);
    }
    glViewSetProjectionMatrix(GLV_Anark3D_BG, hPersp);
    glViewSetProjectionMatrix(GLV_Anark3D_FG, hPersp);
    glViewSetProjectionMatrix(GLV_Transitions3D, hPersp);

    nlVector3 at = { 0.0f, 0.0f, 0.0f };
    nlVector3 to = { 0.0f, 0.0f, -1.0f };
    nlVector3 up = { 0.0f, 1.0f, 0.0f };
    nlMatrix4 view;
    glMatrixLookAt(view, at, to, up);
    u32 hView1 = glAllocMatrix();
    if (hView1 != 0xFFFFFFFF)
    {
        glSetMatrix(hView1, view);
    }
    glViewSetViewMatrix(GLV_Transitions3D, hView1);

    nlVector3 at2 = { 0.0f, 12.0f, 0.0f };
    nlVector3 to2 = { 0.0f, 0.0f, -1.0f };
    nlVector3 up2 = { 0.0f, 0.0f, 1.0f };
    nlMatrix4 view2;
    glMatrixLookAt(view2, at2, to2, up2);
    u32 hView2 = glAllocMatrix();
    if (hView2 != 0xFFFFFFFF)
    {
        glSetMatrix(hView2, view2);
    }
    glViewSetViewMatrix(GLV_Anark3D_BG, hView2);
    glViewSetViewMatrix(GLV_Anark3D_FG, hView2);

    float fFOV = 3.1415927f * cCameraManager::m_fFOV / 180.0f;
    glMatrixPerspective(proj, fFOV, fAspect, 0.25f, 1024.0f);
    u32 hCamProj = glAllocMatrix();
    if (hCamProj != 0xFFFFFFFF)
    {
        glSetMatrix(hCamProj, proj);
    }
    glViewSetProjectionMatrix(GLV_Skybox, hCamProj);
    glViewSetProjectionMatrix(GLV_CameraSpace, hCamProj);
    glViewSetProjectionMatrix(GLV_Shadowed, hCamProj);
    glViewSetProjectionMatrix(GLV_Shadow0, hCamProj);
    glViewSetProjectionMatrix(GLV_Shadow1, hCamProj);
    glViewSetProjectionMatrix(GLV_WorldShadowed, hCamProj);
    glViewSetProjectionMatrix(GLV_Characters, hCamProj);
    glViewSetProjectionMatrix(GLV_Unshadowed, hCamProj);
    glViewSetProjectionMatrix(GLV_LingeringParticles, hCamProj);
    glViewSetProjectionMatrix(GLV_Particles, hCamProj);
    glViewSetProjectionMatrix(GLV_Warble, hCamProj);
    glViewSetProjectionMatrix(GLV_UnsortedPerspective, hCamProj);
    glViewSetProjectionMatrix(GLV_InvisiblePlane, hCamProj);
    glViewSetProjectionMatrix(GLV_ElectricFence, hCamProj);
    glViewSetProjectionMatrix(GLV_CoPlanar0, hCamProj);
    glViewSetProjectionMatrix(GLV_CoPlanar, hCamProj);

    u32 hCamView = glAllocMatrix();
    if (hCamView != 0xFFFFFFFF)
    {
        glSetMatrix(hCamView, cCameraManager::m_matView);
    }

    cBaseCamera* pCamera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
    if (pCamera != NULL)
    {
        pCamera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
        cCameraManager::m_pBeginFrameCameraType = pCamera->GetType();
    }

    glViewSetViewMatrix(GLV_Skybox, hCamView);
    glViewSetViewMatrix(GLV_Shadowed, hCamView);
    glViewSetViewMatrix(GLV_Shadow0, hCamView);
    glViewSetViewMatrix(GLV_Shadow1, hCamView);
    glViewSetViewMatrix(GLV_WorldShadowed, hCamView);
    glViewSetViewMatrix(GLV_Characters, hCamView);
    glViewSetViewMatrix(GLV_Unshadowed, hCamView);
    glViewSetViewMatrix(GLV_Warble, hCamView);
    glViewSetViewMatrix(GLV_LingeringParticles, hCamView);
    glViewSetViewMatrix(GLV_Particles, hCamView);
    glViewSetViewMatrix(GLV_UnsortedPerspective, hCamView);
    glViewSetViewMatrix(GLV_InvisiblePlane, hCamView);
    glViewSetViewMatrix(GLV_ElectricFence, hCamView);
    glViewSetViewMatrix(GLV_CoPlanar0, hCamView);
    glViewSetViewMatrix(GLV_CoPlanar, hCamView);

    glViewSetViewMatrix(GLV_CameraSpace, glGetIdentityMatrix());

    m_LightingCallback__14ParticleSystem = cb_ParticleLighting;
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
 * TODO: 98.62% match - stack offset diffs (0x18) on nlVector3 vars; MWCC not
 * generating ~GLMeshWriterCore() destructor cleanup, causing stack slot reuse
 * where target has 3 separate mesh/streams allocations
 */
static inline void meshTexcoord(GLMeshWriterCore& w, const nlVector2& tc)
{
    w.Texcoord(tc);
}

static inline void meshVertex(GLMeshWriterCore& w, const nlVector3& v)
{
    w.Vertex(v);
}

void DrawGrid(int spacing)
{
    nlColour gridColour = { 0x40, 0x40, 0xFF, 0xFF };
    nlColour centerColour = { 0xFF, 0x40, 0x40, 0xFF };

    for (int y = 0; y < 480; y += spacing)
    {
        GLMeshWriter mesh;
        eGLStream streams[] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
        glSetDefaultState(false);
        glSetCurrentTexture(glGetTexture("global/white"), GLTT_Diffuse);
        glSetCurrentProgram(glGetProgram("2d unlit"));

        if (mesh.Begin(2, GLP_LineList, 3, streams, false))
        {
            nlVector2 tc;
            tc.f.x = 0.0f;
            tc.f.y = 0.0f;
            meshTexcoord(mesh, tc);
            mesh.Colour(gridColour);
            nlVector3 v;
            v.f.x = 0.0f;
            v.f.y = (float)y;
            v.f.z = 0.0f;
            meshVertex(mesh, v);

            nlVector2 tc2;
            tc2.f.x = 0.0f;
            tc2.f.y = 0.0f;
            meshTexcoord(mesh, tc2);
            mesh.Colour(gridColour);
            nlVector3 v2;
            v2.f.x = 640.0f;
            v2.f.y = (float)y;
            v2.f.z = 0.0f;
            meshVertex(mesh, v2);

            if (mesh.End())
            {
                glViewAttachModel(GLV_Debug, 5, mesh.GetModel());
            }
        }
    }

    for (int x = 0; x < 640; x += spacing)
    {
        GLMeshWriter mesh;
        eGLStream streams[] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
        glSetDefaultState(false);
        glSetCurrentTexture(glGetTexture("global/white"), GLTT_Diffuse);
        glSetCurrentProgram(glGetProgram("2d unlit"));

        if (mesh.Begin(2, GLP_LineList, 3, streams, false))
        {
            nlVector2 tc;
            tc.f.x = 0.0f;
            tc.f.y = 0.0f;
            meshTexcoord(mesh, tc);
            mesh.Colour(gridColour);
            nlVector3 v;
            v.f.x = (float)x;
            v.f.y = 0.0f;
            v.f.z = 0.0f;
            meshVertex(mesh, v);

            nlVector2 tc2;
            tc2.f.x = 0.0f;
            tc2.f.y = 0.0f;
            meshTexcoord(mesh, tc2);
            mesh.Colour(gridColour);
            nlVector3 v2;
            v2.f.x = (float)x;
            v2.f.y = 480.0f;
            v2.f.z = 0.0f;
            meshVertex(mesh, v2);

            if (mesh.End())
            {
                glViewAttachModel(GLV_Debug, 5, mesh.GetModel());
            }
        }
    }

    {
        GLMeshWriter mesh;
        eGLStream streams[] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
        glSetDefaultState(false);
        glSetCurrentTexture(glGetTexture("global/white"), GLTT_Diffuse);
        glSetCurrentProgram(glGetProgram("2d unlit"));

        if (mesh.Begin(2, GLP_LineList, 3, streams, false))
        {
            nlVector2 tc;
            tc.f.x = 0.0f;
            tc.f.y = 0.0f;
            meshTexcoord(mesh, tc);
            mesh.Colour(centerColour);
            nlVector3 v;
            v.f.x = 320.0f;
            v.f.y = 0.0f;
            v.f.z = 0.0f;
            meshVertex(mesh, v);

            nlVector2 tc2;
            tc2.f.x = 0.0f;
            tc2.f.y = 0.0f;
            meshTexcoord(mesh, tc2);
            mesh.Colour(centerColour);
            nlVector3 v2;
            v2.f.x = 320.0f;
            v2.f.y = 480.0f;
            v2.f.z = 0.0f;
            meshVertex(mesh, v2);

            if (mesh.End())
            {
                glViewAttachModel(GLV_Debug, 5, mesh.GetModel());
            }
        }
    }
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
