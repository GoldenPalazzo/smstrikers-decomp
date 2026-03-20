#include "NL/nlDebug.h"
#include "NL/nlMemory.h"
#include "NL/nlConfig.h"
#include "NL/gl/glState.h"
#include "NL/gl/glPlat.h"
#include "NL/gl/glConstant.h"
#include "NL/gl/glFont.h"
#include "NL/gl/glDraw2.h"
#include "NL/gl/glTarget.h"
#include "NL/gl/glAppAttach.h"
#include "NL/gl/glRenderList.h"
#include "NL/glx/glxSwap.h"
#include "NL/glx/glxMemory.h"
#include "NL/glx/glxTarget.h"
#include "NL/glx/glxSend.h"
#include "dolphin/vi/vifuncs.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXFrameBuffer.h"
#include "dolphin/gx/GXPixel.h"
#include "dolphin/gx/GXManage.h"
#include "dolphin/gx/GXPerf.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/os/OSThread.h"
#include "dolphin/os/OSReset.h"

// Performance metric string array
static const char* str_perf0[]
    = { "VERTICES", "CLIP_VTX", "CLIP_CLKS", "XF_WAIT_IN", "XF_WAIT_OUT", "XF_XFRM_CLKS", "XF_LIT_CLKS", "XF_BOT_CLKS", "XF_REGLD_CLKS", "XF_REGRD_CLKS", "CLIP_RATIO", "TRIANGLES", "TRIANGLES_CULLED", "TRIANGLES_PASSED", "TRIANGLES_SCISSORED", "TRIANGLES_0TEX", "TRIANGLES_1TEX", "TRIANGLES_2TEX", "TRIANGLES_3TEX", "TRIANGLES_4TEX", "TRIANGLES_5TEX", "TRIANGLES_6TEX", "TRIANGLES_7TEX", "TRIANGLES_8TEX", "TRIANGLES_0CLR", "TRIANGLES_1CLR", "TRIANGLES_2CLR", "QUAD_0CVG", "QUAD_NON0CVG", "QUAD_1CVG", "QUAD_2CVG", "QUAD_3CVG", "QUAD_4CVG", "AVG_QUAD_CNT", "CLOCKS" };

// Performance counter string array for GPU/texture metrics
static const char* str_perf1[] = { "TEXELS", "TX_IDLE", "TX_REGS", "TX_MEMSTALL", "TC_CHECK1_2", "TC_CHECK3_4", "TC_CHECK5_6", "TC_CHECK7_8", "TC_MISS", "VC_ELEMQ_FULL", "VC_MISSQ_FULL", "VC_MEMREQ_FULL", "VC_STATUS7", "VC_MISSREP_FULL", "VC_STREAMBUF_LOW", "VC_ALL_STALLS", "VERTICES", "FIFO_REQ", "CALL_REQ", "VC_MISS_REQ", "CP_ALL_REQ", "CLOCKS" };

GXRenderModeObj glx_rmode;

GXRenderModeObj glPal480IntDf = { VI_TVMODE_PAL_INT,
    640,
    480,
    542,
    40,
    16,
    640,
    542,
    VI_XFBMODE_SF,
    1,
    0,
    { 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 },
    { 6, 6, 8, 8, 10, 12, 10 } };

struct GXFrameBuffer
{
    void* unk0;
    void* unk4;
};

GXFrameBuffer glx_FrameBuffer;

bool glx_bFog = false;
bool glx_bProgressiveMode = false;
u32 glx_TargetFPS = 60;
s32 glx_FBSize;
GXPerf0 glx_perf0 = GX_PERF0_TRIANGLES;
GXPerf1 glx_perf1 = GX_PERF1_VERTICES;
bool glx_Virt = true;
bool glx_Perf = true;
bool glx_PerfSync = true;
s32 glx_ViewFence = -1;
u32 glx_NumVirtMisses = 0;
u32 glx_VirtLatency = 0;
f32 glx_CopyDispScaleFactor = 1.f;
eVideoMode glx_VideoMode;

void* glx_FIFOMem = nullptr;
GXFifoObj* glx_FIFO = nullptr;
u32 glx_FIFOSize = 393216;

bool glx_bFogAdjust = true;
u32 glx_FogType = 4;
bool glx_bFogSky = true;
f32 glx_FogStart = 5.f;
f32 glx_FogEnd = 160.f;
f32 glx_FogIntensity = 1.f;
GXColor glx_FogColour = { 0xFF, 0xFF, 0xFF, 0xFF };

static u32 fogtype[5] = { GX_FOG_PERSP_LIN, GX_FOG_PERSP_EXP, GX_FOG_PERSP_EXP2, GX_FOG_PERSP_REVEXP, GX_FOG_PERSP_REVEXP2 };

s32 glx_VIWidth = 0x00000294;
s32 prev_VIWidth = 0x00000294;

static u32 total_val0 = 0;
static u32 total_val1 = 0;

/**
 * Offset/Address/Size: 0x0 | 0x801B45F4 | size: 0xB0
 */
void glplatViewProjectPoint(eGLView view, const nlVector3& arg1, nlVector3& arg2)
{
    nlVector3 v_out;
    nlMatrix4* temp_r31 = glViewGetViewMatrix(view);
    nlMatrix4* temp_r30 = glViewGetProjectionMatrix(view);
    nlMultPosVectorMatrix(v_out, arg1, *temp_r31);
    nlMultPosVectorMatrix(arg2, v_out, *temp_r30);
    float temp_f1 = 1.f / -v_out.f.z;
    arg2.f.x = arg2.f.x * temp_f1;
    arg2.f.y = -arg2.f.y * temp_f1;
    arg2.f.z = arg2.f.z * temp_f1;
}

/**
 * Offset/Address/Size: 0xB0 | 0x801B46A4 | size: 0x4
 */
void glplatEndFrame()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0xB4 | 0x801B46A8 | size: 0x58
 */
void glplatBeginFrame()
{
    if (glx_VIWidth != prev_VIWidth)
    {
        s32 diff = 720 - glx_VIWidth;
        prev_VIWidth = glx_VIWidth;
        glx_rmode.viWidth = (s16)glx_VIWidth;
        glx_rmode.viXOrigin = (s16)(diff / 2);
        VIConfigure(&glx_rmode);
        VIFlush();
    }
}

/**
 * Offset/Address/Size: 0x10C | 0x801B4700 | size: 0x20
 */
void glplatFinish()
{
    glxSwapWaitDrawDone();
}

/**
 * Offset/Address/Size: 0x12C | 0x801B4720 | size: 0x34
 */
void glplatAbortFrame()
{
    glplatFrameAllocNextFrame();
    glx_NumVirtMisses = 0;
    glx_VirtLatency = 0;
    glxSwapWaitDrawDone();
    VIWaitForRetrace();
}

/**
 * Offset/Address/Size: 0x160 | 0x801B4754 | size: 0x2FC
 */
void glplatSendFrame()
{
    glPoly2 sp1C;
    f32 sp18;
    f32 sp14;
    f32 sp10;
    f32 spC;
    nlColour sp8;
    s32 temp_r5;
    s32 var_r30;
    s32 var_r31;
    u32 temp_r30;
    u32 temp_r3;
    u32 temp_r4;

    glxSwapPre(true);
    var_r30 = 0;
    if (glx_Perf != false)
    {
        var_r30 = 1;
    }
    if (glx_Virt != false)
    {
        var_r30 += 1;
    }

    if (var_r30 != 0)
    {
        glFontVirtualPosToScreenCoordPos(0.f, 36.f, spC, sp10);
        glFontVirtualPosToScreenCoordPos(0.f, (f32)(var_r30 + 0x24), sp14, sp18);
        glSetDefaultState(false);
        sp1C.SetupRectangle(0.f, sp10 - 2.f, 640.f, 4.f + (sp18 - sp10), 10000000000.f);
        *(volatile u32*)&sp8 = *(volatile u32*)&glx_FogColour;
        sp8.c[0] = 0x3A;
        sp8.c[1] = 0x6E;
        sp8.c[2] = 0xA5;
        sp8.c[3] = 0xFF;
        // SetColour(&sp1C, &sp8, 0xA5, 0x6E, 0x3A);
        sp1C.SetColour(sp8);
        sp1C.Attach((eGLView)0x21, 0, 0, -1);
        var_r31 = 0;
        if ((u8)glx_Perf != 0)
        {
            var_r31 = 1;
            static u32 print_val0 = 0;
            static u32 print_val1 = 0;
            static u32 counter = 0;
            temp_r5 = counter + 1;
            counter = temp_r5;
            if (temp_r5 >= 0x14)
            {
                temp_r4 = total_val0;
                temp_r3 = total_val1;
                temp_r4 = temp_r4 / (u32)temp_r5;
                total_val0 = 0;
                total_val1 = 0;
                counter = 0;
                temp_r3 = temp_r3 / (u32)temp_r5;
                print_val0 = temp_r4;
                print_val1 = temp_r3;
            }
            glFontBegin(false);
            glFontPrintf((eGLView)0x21, 1, 0x24, "%u %s, %u %s", print_val0, str_perf0[glx_perf0], print_val1, str_perf1[glx_perf1]);
            glFontEnd();
        }

        if (glx_Virt)
        {
            static u32 print0 = 0;
            static u32 print1 = 0;
            static u32 print2 = 0;

            if ((u32)glx_NumVirtMisses != 0U)
            {
                print0 = glx_NumVirtMisses;
                print1 = glx_VirtLatency;
                print2 = glGetCurrentFrame();
            }
            temp_r30 = nlVirtualLargestBlock();
            glFontBegin(0);
            glFontPrintf((eGLView)0x21, 1, var_r31 + 0x24, "%uKB free : %u misses, %u us latency (frame %u)", temp_r30 >> 0xAU, print0, print1, print2);
            glFontEnd();
        }
    }
    glx_SendViews();
    glxSwapPost(true);
    glplatFrameAllocNextFrame();
    glx_NumVirtMisses = 0U;
    glx_VirtLatency = 0;
}

/**
 * Offset/Address/Size: 0x45C | 0x801B4A50 | size: 0x470
 */
void glx_SendViews()
{
    GLRenderList* renderList;
    nlVector4 const_v;
    nlVector4 local;
    u32 temp_r5;
    s32 var_r30;
    s32 var_r31;
    u32 temp_r30;
    u32 temp_r3;
    u32 temp_r4;
    bool cVar6;
    u32* iVar4;
    double dVar8;
    GXColor local_c8;
    u8 uStack_44;
    u8 uStack_34;
    u8 uStack_24;
    double local_48;
    double local_38;
    s32 local_40;
    double local_28;
    s32 iVar1;
    s32 local_30;
    s32 iVar2;
    s32 local_20;
    u32 local_c4;
    GXColor local_c0;
    u32* uVar5;
    u32 local_d8;
    u32 local_d4[1];
    nlMatrix4 auStack_88;
    GXFogAdjTable auStack_9c;

    glx_SendReset();
    renderList = gl_ViewGetRenderList((eGLView)9);
    if (renderList->IsEmpty())
    {
        glx_UpdateWarble();
    }

    if (glx_ViewFence < 0)
    {
        if (glx_PerfSync)
        {
            GXDrawDone();
        }
        GXClearGPMetric();
        GXSetGPMetric(glx_perf0, glx_perf1);
    }

    glConstantGet("dof/range", const_v);
    int iVar7 = 0;
    local = const_v;

    do
    {
        if (glViewGetEnable((eGLView)iVar7))
        {
            renderList = gl_ViewGetRenderList((eGLView)iVar7);
            cVar6 = renderList->IsEmpty();
            if (((cVar6 != false) && (glViewGetFilter((eGLView)iVar7) == 0)) && (iVar7 != 0x19))
            {
                if (iVar7 == 0)
                {
                    glx_ShadowTextureGrab();
                }
                goto switchD_801b4d2c_caseD_5;
            }

            if (glx_bFog == false)
            {
            LAB_801b4ca4:
                dVar8 = 0.0;
                local_c8 = glx_FogColour;
                GXSetFog(GX_FOG_NONE, 0.0, 0.0, 0.0, 0.0, local_c8);
            }
            else
            {
                if (iVar7 < 6)
                {
                    if (iVar7 == 3)
                    {
                    LAB_801b4b78:
                        cVar6 = true;
                    }
                    else if ((2 < iVar7) || (cVar6 = glx_bFogSky, iVar7 < 2))
                        goto LAB_801b4b88;
                }
                else
                {
                    if ((iVar7 == 0xb) || ((iVar7 < 0xb && (iVar7 < 8))))
                        goto LAB_801b4b78;
                LAB_801b4b88:
                    cVar6 = false;
                }
                if (cVar6 == false)
                    goto LAB_801b4ca4;

                // uStack_44 = glx_FogColour >> 0x18;
                // uStack_34 = glx_FogColour >> 0x10 & 0xff;
                // uStack_24 = glx_FogColour >> 8 & 0xff;
                // local_48 = 0x43300000;
                // local_38 = 0x43300000;
                // iVar4 = (int)(glx_FogIntensity * (float)((double)CONCAT44(0x43300000, uStack_44) - 0x4330000000000000));
                // local_40 = (longlong)iVar4;
                // local_28 = 0x43300000;
                // iVar1 = (int)(glx_FogIntensity * (float)((double)CONCAT44(0x43300000, uStack_34) - 0x4330000000000000));
                // local_30 = (longlong)iVar1;
                // iVar2 = (int)(glx_FogIntensity * (float)((double)CONCAT44(0x43300000, uStack_24) - 0x4330000000000000));
                // local_20 = (longlong)iVar2;
                // local_c4 = CONCAT22(CONCAT11((char)iVar4, (char)iVar1), CONCAT11((char)iVar2, (char)glx_FogColour));
                // local_c0 = local_c4;
                local_c0.r = 0x0;
                local_c0.g = 0x0;
                local_c0.b = 0x0;
                local_c0.a = 0xff;

                GXSetFog((GXFogType)fogtype[glx_FogType], glx_FogStart, glx_FogEnd, 0.25f, 130.0f, local_c0);
                if (glx_bFogAdjust == false)
                {
                    GXSetFogRangeAdj(0, 0, 0);
                }
                else
                {
                    glViewGetProjectionMatrix((eGLView)iVar7, auStack_88);
                    GXInitFogAdjTable(&auStack_9c, 0x280, auStack_88.m);
                    GXSetFogRangeAdj(1, 0x140, &auStack_9c);
                }
            }

            if ((iVar7 == glx_ViewFence) && (gld_ViewName((eGLView)iVar7), glx_Perf != false))
            {
                if (glx_PerfSync != false)
                {
                    GXDrawDone();
                }
                GXClearGPMetric();
                GXSetGPMetric(glx_perf0, glx_perf1);
            }

            switch (iVar7)
            {
            case 5:
                break;
            default:
                goto switchD_801b4d2c_caseD_6;
            case 9:
                glx_ColourGrab();
                goto switchD_801b4d2c_caseD_6;
            case 10:
                renderList = gl_ViewGetRenderList((eGLView)9);
                cVar6 = renderList->IsEmpty();
                if (cVar6 == false)
                {
                    glx_OffsetGrab();
                    goto switchD_801b4d2c_caseD_6;
                }
                break;
            case 0xf:
                renderList = gl_ViewGetRenderList((eGLView)0xe);
                cVar6 = renderList->IsEmpty();
                if (cVar6 == false)
                {
                    glx_ShadowGrab();
                    goto switchD_801b4d2c_caseD_6;
                }
                break;
            case 0x11:
                glx_DOFUpdate((double)local.f.x);
                glx_DOFGrab();
            switchD_801b4d2c_caseD_6:

                if (glViewGetDepthClear((eGLView)iVar7) != false)
                {
                    glx_ClearZBuffer();
                }
                gl_ViewIterate((eGLView)iVar7, glx_SendFrame_cb);

                if (iVar7 == 0)
                {
                    glx_ShadowTextureGrab();
                }

                if (glViewGetFilter((eGLView)iVar7) == (eGLFilter)6)
                {
                    cVar6 = glx_GetSharedLock();
                    if (cVar6 != false)
                    {
                        return;
                    }
                    uVar5 = (u32*)glGetTexture("target/grab_texture");
                    iVar4 = (u32*)glx_GetTex((u32)uVar5, false, true);
                    glGrabFrameBufferToTexture((u32)uVar5, *(u32*)(iVar4 + 4), *(u32*)(iVar4 + 6), 0, 0, 0x280, 0x1c0);
                }
            }
        }
    switchD_801b4d2c_caseD_5:
        iVar7 = iVar7 + 1;
        if (0x21 < iVar7)
        {
            glx_SendEnd();
            if (glx_ViewFence < 0)
            {
                if (glx_PerfSync != false)
                {
                    GXDrawDone();
                }
                GXReadGPMetric(&local_d8, local_d4);
                total_val0 = total_val0 + local_d8;
                total_val1 = total_val1 + local_d4[0];
            }
            return;
        }
    } while (true);
}

/**
 * Offset/Address/Size: 0x8CC | 0x801B4EC0 | size: 0x110
 */
void glx_Fog(bool enable)
{
    if (enable)
    {
        s32 r = (s32)(glx_FogIntensity * glx_FogColour.r);
        s32 g = (s32)(glx_FogIntensity * glx_FogColour.g);
        s32 b = (s32)(glx_FogIntensity * glx_FogColour.b);
        GXColor fogColour;
        fogColour.r = r;
        fogColour.g = g;
        fogColour.b = b;
        fogColour.a = glx_FogColour.a;
        GXSetFog((GXFogType)fogtype[glx_FogType], glx_FogStart, glx_FogEnd, 0.25f, 130.f, fogColour);
    }
    else
    {
        GXSetFog(GX_FOG_NONE, 0.f, 0.f, 0.f, 0.f, glx_FogColour);
    }
}

/**
 * Offset/Address/Size: 0x9DC | 0x801B4FD0 | size: 0x8
 */
bool glx_GetFog()
{
    return glx_bFog;
}

/**
 * Offset/Address/Size: 0x9E4 | 0x801B4FD8 | size: 0x24
 */
bool glplatPostStartup()
{
    glxPostInitTargets();
    return true;
}

/**
 * Offset/Address/Size: 0xA08 | 0x801B4FFC | size: 0x524
 */
bool glplatStartup(gl_ScreenInfo* arg0)
{
    GXColor sp8;
    GXRenderModeObj* var_r29;
    f32 var_f1;
    void* temp_r3_3;
    void* temp_r3_4;
    u32* var_r5;
    u32* var_r5_2;
    s32 temp_r0;
    void* temp_r31;
    s32 temp_r3_2;
    s32 var_r28_2;
    s32 var_r6;
    s32 var_r6_2;
    u32 temp_r29;
    u32 temp_r4;
    u32 var_r28;
    void* temp_r3;

    if (glxInitMemory() == 0)
    {
        return false;
    }
    Config::Global();

    // TODO: Implement this
    // if (Exists__6ConfigCFPCc(CONFIG_FIFO_SIZE_KEY) != 0)
    // {
    //     temp_r31 = Global__6ConfigFv();
    //     temp_r3 = FindTvp__6ConfigFPCc(CONFIG_FIFO_SIZE_KEY);
    //     if (temp_r3->unk0 == 0)
    //     {
    //         Set__6ConfigFPCcf(temp_r31, CONFIG_FIFO_SIZE_KEY, 1.0f);
    //         var_f1 = 1.0f;
    //     }
    //     else
    //     {
    //         temp_r0 = temp_r3->unk4;
    //         switch (temp_r0)
    //         { /* irregular */
    //         case 0:
    //             var_f1 = LexicalCast_f_b(temp_r3 + 8);
    //             break;
    //         case 1:
    //             var_f1 = LexicalCast_f_i(temp_r3 + 8);
    //             break;
    //         case 2:
    //             var_f1 = LexicalCast_f_f(temp_r3 + 8);
    //             break;
    //         case 3:
    //             var_f1 = LexicalCast_f_PCc(temp_r3 + 8);
    //             break;
    //         default:
    //             var_f1 = 1.0f;
    //             break;
    //         }
    //     }
    //     glx_FIFOSize = 256 * (256 * var_f1);
    // }

    arg0->ScreenWidth = 640;
    arg0->ScreenHeight = 448;
    arg0->ColourDepth[0] = 6;
    arg0->ColourDepth[1] = 6;
    arg0->ColourDepth[2] = 6;
    arg0->ColourDepth[3] = 6;
    arg0->ZDepth = 24;
    arg0->StencilDepth = 0;
    arg0->PixelCentre = 1.0f;
    arg0->FSAA = false;
    glx_CopyDispScaleFactor = 1.0f;

    // temp_r3_2 = VIGetTvFormat(24, 6, 448, 1.0f);
    temp_r3_2 = VIGetTvFormat();

    switch (temp_r3_2)
    {       /* switch 1; irregular */
    case 0: /* switch 1 */
        glx_VideoMode = VideoMode_NTSC;
        var_r29 = &GXNtsc480IntDf;
        break;
    case 5: /* switch 1 */
    case 1: /* switch 1 */
        glx_VideoMode = VideoMode_PAL;
        var_r29 = &glPal480IntDf;
        break;
    case 2: /* switch 1 */
        glx_VideoMode = VideoMode_MPAL;
        var_r29 = &GXMpal480IntDf;
        break;
    default: /* switch 1 */
        nlBreak();
        break;
    }
    if (((OSGetResetCode() >> 0x1F) != 0) && (OSGetResetCode() == 0x17) && (VIGetDTVStatus() != 0))
    {
        glx_bProgressiveMode = true;
        var_r29 = &GXNtsc480Prog;
    }
    else
    {
        glx_bProgressiveMode = false;
    }
    GXAdjustForOverscan(var_r29, &glx_rmode, 0, 16);
    if (glx_VideoMode == VideoMode_PAL)
    {
        glx_rmode.efbHeight = 448;
        glx_CopyDispScaleFactor = GXGetYScaleFactor(448, glx_rmode.xfbHeight);
        glx_TargetFPS = 50;
    }
    else
    {
        glx_TargetFPS = 60;
    }
    temp_r4 = 720 - glx_VIWidth;
    glx_rmode.viWidth = (s16)glx_VIWidth;
    glx_rmode.viXOrigin = (s16)((s32)((temp_r4 >> 0x1F) + temp_r4) >> 1);
    VIConfigure(&glx_rmode);
    VIFlush();
    VIConfigure(&glx_rmode);

    glx_FIFOMem = nlMalloc(glx_FIFOSize, 32, false);
    if (glx_FIFOMem == NULL)
    {
        return false;
    }
    glx_FIFO = GXInit(glx_FIFOMem, glx_FIFOSize);

    var_r28 = ((glx_rmode.fbWidth + 15) & 0xFFF0) * glx_rmode.xfbHeight * 2;
    if (var_r28 < 0x9F600)
    {
        var_r28 = 0x9F600;
    }
    temp_r29 = var_r28 * 2;
    temp_r3_4 = nlMalloc(temp_r29, 32, false);
    glx_FrameBuffer.unk4 = (u8*)temp_r3_4 + var_r28;
    var_r5 = (u32*)temp_r3_4;
    glx_FrameBuffer.unk0 = temp_r3_4;
    var_r6 = 0;
    glx_FBSize = var_r28;
loop_40:
    if (var_r6 < glx_FBSize)
    {
        *var_r5 = 0x10801080;
        var_r6 += 4;
        var_r5 += 1;
        goto loop_40;
    }
    DCFlushRange(var_r5, var_r6);
    var_r6_2 = 0;
    var_r5_2 = (u32*)glx_FrameBuffer.unk4;
loop_43:
    if (var_r6_2 < glx_FBSize)
    {
        *var_r5_2 = 0x10801080;
        var_r6_2 += 4;
        var_r5_2 += 1;
        goto loop_43;
    }
    DCFlushRange(var_r5_2, var_r6_2);
    // Print(1, DEBUG_MEMORY_MSG, temp_r29 >> 10, glx_FIFOSize >> 10);
    gxInit();
    GXSetViewport(0.0f, 0.0f, (f32)glx_rmode.fbWidth, (f32)glx_rmode.efbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, glx_rmode.fbWidth, glx_rmode.efbHeight);
    GXSetDispCopySrc(0, 0, glx_rmode.fbWidth, glx_rmode.efbHeight);
    GXSetDispCopyDst(glx_rmode.fbWidth, glx_rmode.xfbHeight);
    GXSetDispCopyYScale(glx_CopyDispScaleFactor);
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    gxSetDither(true);
    gxSetColourUpdate(true);
    gxSetAlphaUpdate(true);
    sp8.r = 0x00;
    sp8.g = 0x00;
    sp8.b = 0x00;
    sp8.a = 0x40;
    // { 0x00, 0x00, 0x00, 0x40 };
    GXSetCopyClear(sp8, 0xFFFFFF);
    GXSetDispCopyGamma(GX_GM_1_0);
    var_r28_2 = 0;
    do
    {
        gxSetTevColourOp(var_r28_2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
        gxSetTevAlphaOp(var_r28_2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
        var_r28_2 += 1;
    } while (var_r28_2 < 16);
    GXFlush();
    VISetNextFrameBuffer(glx_FrameBuffer.unk0);
    glxSwapSetBlack(true);
    VIFlush();
    VIWaitForRetrace();
    if (glx_rmode.viTVmode & 1)
    {
        VIWaitForRetrace();
    }
    glxInitSwap(glx_FrameBuffer.unk0, glx_FrameBuffer.unk4);
    glxInitTex();
    glxInitTargets();
    // VMSetLogStatsCallback(&virt_cb);
    return true;
}

/**
 * Offset/Address/Size: 0xF2C | 0x801B5520 | size: 0x2C
 */
void glx_SetPal50Mode()
{
    glx_SwitchVideoMode(&glPal480IntDf, VideoMode_PAL);
}

/**
 * Offset/Address/Size: 0xF58 | 0x801B554C | size: 0x2C
 */
void glx_SetRGB60Mode()
{
    glx_SwitchVideoMode(&GXEurgb60Hz480IntDf, VideoMode_PAL60);
}

/**
 * Offset/Address/Size: 0xF84 | 0x801B5578 | size: 0x34
 */
void glx_SetInterlacedMode()
{
    glx_SwitchVideoMode(&GXNtsc480IntDf, VideoMode_NTSC);
    glx_bProgressiveMode = 0;
}

/**
 * Offset/Address/Size: 0xFB8 | 0x801B55AC | size: 0x34
 */
void glx_SetProgressiveMode()
{
    glx_SwitchVideoMode(&GXNtsc480Prog, VideoMode_NTSC);
    glx_bProgressiveMode = 1;
}

/**
 * Offset/Address/Size: 0xFEC | 0x801B55E0 | size: 0x1C
 */
u32 glx_GetResetCode()
{
    // return 0x17 & ((s32) (-(s32) glx_bProgressiveMode | glx_bProgressiveMode) >> 0x1F);
    return glx_bProgressiveMode ? 0x17 : 0;
}

static inline void glx_SetVIWidth(const s32 currentWidth, const s32 maxWidth)
{
    const s32 diff = maxWidth - currentWidth;
    glx_rmode.viWidth = (s16)currentWidth;
    glx_rmode.viXOrigin = (s16)(((s32)((u32)diff >> 31) + diff) >> 1);
}

/**
 * Offset/Address/Size: 0x1008 | 0x801B55FC | size: 0x18C
 */
void glx_SwitchVideoMode(_GXRenderModeObj* rmode, eVideoMode mode)
{
    glx_VideoMode = mode;
    GXAdjustForOverscan(rmode, &glx_rmode, 0, 0x10);
    if (mode == 1)
    {
        glx_rmode.efbHeight = 448;
        glx_CopyDispScaleFactor = GXGetYScaleFactor(448, glx_rmode.xfbHeight);
        glx_TargetFPS = 50;
    }
    else
    {
        glx_TargetFPS = 60;
        glx_CopyDispScaleFactor = 1.f;
    }
    VISetBlack(1);
    VIFlush();
    VIWaitForRetrace();

    glx_SetVIWidth(glx_VIWidth, 720);

    VIConfigure(&glx_rmode);
    VIFlush();
    VIConfigure(&glx_rmode);
    VIFlush();
    VIWaitForRetrace();

    for (int i = 0; i < 60; i++)
    {
        OSYieldThread();
        VIWaitForRetrace();
    }

    VISetBlack(0);
    VIFlush();
    VIWaitForRetrace();

    GXSetViewport(0.f, 0.f, (f32)glx_rmode.fbWidth, (f32)glx_rmode.efbHeight, 0.f, 1.f);
    GXSetScissor(0, 0, glx_rmode.fbWidth, glx_rmode.efbHeight);
    GXSetDispCopySrc(0, 0, glx_rmode.fbWidth, glx_rmode.efbHeight);
    GXSetDispCopyDst(glx_rmode.fbWidth, glx_rmode.xfbHeight);
    GXSetDispCopyYScale(glx_CopyDispScaleFactor);
}

/**
 * Offset/Address/Size: 0x1194 | 0x801B5788 | size: 0x8
 */
bool glplatPreStartup()
{
    return true;
}

/**
 * Offset/Address/Size: 0x119C | 0x801B5790 | size: 0x1C
 */
void virt_cb(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, int arg4)
{
    glx_NumVirtMisses += 1;
    glx_VirtLatency += arg3;
}

/**
 * Offset/Address/Size: 0x11B8 | 0x801B57AC | size: 0x4C
 */
void glx_ClearXFB(void* cache)
{
    u8* var_r5 = (u8*)cache;
    s32 var_r6 = 0;

    while (var_r6 < (s32)glx_FBSize)
    {
        *(u32*)var_r5 = 0x10801080;
        var_r6 += 4;
        var_r5 += 4;
    }
    DCFlushRange(cache, glx_FBSize);
}

/**
 * Offset/Address/Size: 0x1204 | 0x801B57F8 | size: 0x8
 */
u32 glx_GetTargetFPS()
{
    return glx_TargetFPS;
}

/**
 * Offset/Address/Size: 0x120C | 0x801B5800 | size: 0x8
 */
u32 glx_GetScaledXFBWidth()
{
    return glx_VIWidth;
}

/**
 * Offset/Address/Size: 0x1214 | 0x801B5808 | size: 0x110
 */
void glx_SetFog(int type)
{
    if (type < 0)
    {
        glx_bFog = false;
        return;
    }

    if (type == 0)
    {
        glx_bFog = 1;
        glx_bFogSky = 1;
        glx_FogType = 0;
        glx_FogColour.r = 0xAE;
        glx_FogColour.g = 0x73;
        glx_FogColour.b = 0x55;
        glx_FogIntensity = 1.f;
        glx_FogStart = 5.f;
        glx_FogEnd = 130.f;
        return;
    }
    if (type == 1)
    {
        glx_bFog = 1;
        glx_bFogSky = 0;
        glx_FogType = 4;
        glx_FogColour.r = 0x2C;
        glx_FogColour.g = 0xB9;
        glx_FogColour.b = 0xFF;
        glx_FogIntensity = .5f;
        glx_FogStart = 10.f;
        glx_FogEnd = 125.f;
        return;
    }

    if (type != 2)
    {
        return;
    }

    glx_bFog = 1;
    glx_bFogSky = 1;
    glx_FogType = 2;
    glx_FogColour.r = 0xFA;
    glx_FogColour.g = 0xE6;
    glx_FogColour.b = 0xB9;
    glx_FogIntensity = 1.f;
    glx_FogStart = 45.f;
    glx_FogEnd = 160.f;
}
