#include "NL/glx/glxSend.h"

#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXLighting.h"
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTransform.h"
#include "dolphin/mtx.h"
#include "NL/gl/glConstant.h"
#include "NL/gl/glLightUserData.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glPlat.h"
#include "NL/gl/gl.h"
#include "NL/gl/glState.h"
#include "NL/gl/glUserData.h"
#include "NL/gl/glView.h"
#include "NL/glx/glxGX.h"
#include "NL/glx/glxMatrix.h"
#include "NL/glx/glxTexture.h"
#include "NL/glx/glxDisplayList.h"
#include "dolphin/gx/GXDispList.h"
#include "dolphin/gx/GXBump.h"
#include "dolphin/gx/GXCull.h"
#include "dolphin/gx/GXTexture.h"
#include "NL/nlColour.h"
#include "NL/platvmath.h"
#include "types.h"
#include "dolphin/gx/GXVert.h"
#include <string.h>

// .sdata: non-zero-initialized globals (target order)
static bool g_bAllowLighting = true;                      // .sdata:0x00
static bool g_bAllowSpecular = true;                      // .sdata:0x01
static nlColour nlBlack = { { 0, 0, 0, 0xFF } };          // .sdata:0x04
static nlColour nlWhite = { { 0xFF, 0xFF, 0xFF, 0xFF } }; // .sdata:0x08
static bool glx_AlwaysReloadLights = true;                // .sdata:0x0C
static bool glx_CompiledDraw = true;                      // .sdata:0x0D
static u8 glx_InvXpose = 1;                               // .sdata:0x0E
static bool glx_AllowUncompiledDraws = true;              // .sdata:0x0F
static bool g_bFastSkinPath = true;                       // .sdata:0x10
static bool g_bMtxSkinMath = true;                        // .sdata:0x11
static u32 glx_program = (u32)-1;                         // .sdata:0x14

// .sbss: zero-init globals (target order)
static eGLView prev_view;            // .sbss:0x00
static bool glx_IsCoPlanarView;      // .sbss:0x04
static u32 glx_texdirty;             // .sbss:0x08
static u8 glx_normals;               // .sbss:0x0C
static bool glx_envdiffuse;          // .sbss:0x0D
static bool glx_mobilediffuse;       // .sbss:0x0E
static bool glx_constantcolour;      // .sbss:0x0F
static bool glx_viewport;            // .sbss:0x10
static bool glx_CoPlanar;            // .sbss:0x11
static u32 glx_texconfig;            // .sbss:0x14
static u32 glx_NumIndices;           // .sbss:0x18
static u32 glx_DirtyFlags;           // .sbss:0x1C
static bool glx_translucent;         // .sbss:0x20
static bool glx_norasterizedalpha;   // .sbss:0x21
static s32 glx_RasterizedAlphaStage; // .sbss:0x24
static s32 glx_RasterizedAlphaArg;   // .sbss:0x28
static s32 glx_GlossMapStage;        // .sbss:0x2C
static s32 glx_GlossMapCoord;        // .sbss:0x30
static bool glx_NoFog;               // .sbss:0x34
static u32 gx_vtxfmt;                // .sbss:0x38
static bool glx_allowSpecular;       // .sbss:0x3C
static bool glx_ReloadPointLights;   // .sbss:0x3D
static bool glx_ReloadSpecLights;    // .sbss:0x3E
static u32 glx_prevLightMask;        // .sbss:0x40
static u32 glx_prevSpecMask;         // .sbss:0x44
static GXColor rshadow_colour[2];    // .sbss:0x48
static nlColour world_ambient;       // .sbss:0x50
static f32 glx_IndDivisor;           // .sbss:0x54
static _GXTevScale glx_tevscale;     // .sbss:0x58
static int glx_aniso;                // .sbss:0x5C
static u8 glx_InvXposeChar;          // .sbss:0x60

static void glud_Specular(void*);

// Program-handle statics (initialized at __sinit_)
static u32 prog_2d_unlit = glGetProgram("2d unlit");
static u32 prog_2d_movie = glGetProgram("2d movie");
static u32 prog_3d_unlit = glGetProgram("3d unlit");
static u32 prog_3d_unlit_2x = glGetProgram("3d unlit 2x");
static u32 prog_3d_pointlit = glGetProgram("3d pointlit");
static u32 prog_3d_pointlit_dirt = glGetProgram("3d pointlit dirt");
static u32 prog_3d_crowd = glGetProgram("3d crowd");
static u32 prog_3d_crowd_lit = glGetProgram("3d crowd lit");
static u32 ColourTargetTexture = glGetTexture("target/colour");

// .sdata2: forced via section attribute (target has these in .sdata2 with non-zero init)
static u32 glv_MatrixChanged __attribute__((section(".sdata2"))) = 0x20;
static u32 glv_TexConfigChanged __attribute__((section(".sdata2"))) = 0x80;

// .data: non-zero-initialized large arrays (target order)
static nlVector4 water_Scale = { 1.0f, 1.0f, 0.0f, 0.0f };
static nlVector4 water_Trans = { 0.0f, 0.0f, 0.0f, 0.0f };
static nlVector4 water_Follow = { 0.0f, 0.0f, 0.0f, 0.0f };
static f32 glx_konstlevel[4] = { -1.0f, -1.0f, -1.0f, -1.0f };

// .bss: zero-init large arrays (target order)
static nlMatrix4 mproj;
static nlMatrix4 mview;
static nlMatrix4 modelview;
static Mtx gx_mview;
static Mtx44 gx_proj;
static Mtx gx_modelview;
static GXTexObj glx_texobj[6];
static GXTlutObj glx_tlutobj[6];
static u32 glx_texture[6];

struct GLViewportUserData
{
    u16 x;
    u16 y;
    u16 w;
    u16 h;
    u32 view;
    u32 projection;
};

struct GLScissorUserData
{
    u16 xOrig;
    u16 yOrig;
    u16 wd;
    u16 ht;
};

static GLViewportUserData g_viewport;
static bool bDeferredEnvDiffuse;
static signed char init;

static void glx_SwitchTextureState(const glModelPacket*);
static unsigned long glx_SwitchTexConfig(const glModelPacket*);
static void glx_DrawPacket(const glModelPacket*);
static void glx_SwitchUserData(const glModelPacket*);
static void glx_LoadLight(GLLightUserData*, _GXLightID);
static void GetConstants();

/**
 * Offset/Address/Size: 0x0 | 0x801B9B00 | size: 0x538
 * TODO: 98.27% match - texture loop register permutation: texnum/i at r25/r24 vs target r29/r28, sliding texture/output pointers shifted.
 */
void glx_SendFrame_cb(eGLView view, unsigned long flags, const glModelPacket* p)
{
    static bool bEnabled;
    static signed char init;

    if (p != NULL)
    {
        if (glx_DirtyFlags != 0)
        {
            if (!init)
            {
                bEnabled = false;
                init = 1;
            }
            if (bEnabled)
            {
                GXSetTevSwapMode((GXTevStageID)0, (GXTevSwapSel)0, (GXTevSwapSel)0);
                gxSetTevAlphaOp(0, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
                gxSetTevAlphaOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
                bEnabled = false;
            }
            flags |= glx_DirtyFlags;
            glx_DirtyFlags = 0;
        }
    }

    if (flags & 0x7FF)
    {
        if (flags & 0x83)
        {
            if (flags & 1)
            {
                if (view != prev_view)
                {
                    prev_view = view;
                    glx_IsCoPlanarView = (view == GLV_CoPlanar0 || view == GLV_CoPlanar);
                    glViewGetProjectionMatrix(view, mproj);
                    glViewGetViewMatrix(view, mview);
                    glxCopyMatrix(gx_mview, mview);
                    glxCopyMatrix(gx_proj, mproj);
                    GXProjectionType projType;
                    if (mproj.m[3][2] == -1.0f)
                        projType = GX_PERSPECTIVE;
                    else
                        projType = GX_ORTHOGRAPHIC;
                    GXSetProjection(gx_proj, projType);
                    glx_SwitchUserData(NULL);

                    nlColour amb;
                    nlColour temp = { };
                    glx_ReloadPointLights = 1;
                    glx_ReloadSpecLights = 1;
                    nlColourSet(temp, world_ambient.c[0], world_ambient.c[1], world_ambient.c[2], world_ambient.c[3]);
                    amb = temp;
                    gxSetChanAmbColour(0, amb);
                    gxSetChanMatColour(0, nlWhite);
                    gxSetChanAmbColour(1, nlBlack);
                    gxSetChanMatColour(1, nlWhite);
                }
            }

            if (flags & 2)
            {
                u32 newProg = p->state.program;
                if (glx_program == prog_2d_movie && newProg != prog_2d_movie)
                {
                    GXSetTevKAlphaSel((GXTevStageID)1, (GXTevKAlphaSel)0);
                }
                u8 normals = 0;
                if (newProg == prog_3d_pointlit || newProg == prog_3d_pointlit_dirt || newProg == prog_3d_crowd_lit)
                {
                    normals = 1;
                }
                glx_normals = normals;
                glx_program = newProg;
            }

            if (flags & 0x80)
            {
                flags |= glx_SwitchTexConfig(p);
            }
        }

        if (flags & 0x100)
        {
            glx_SwitchUserData(p);
        }

        if (flags & 0x14)
        {
            int texnum = 0;
            int i;

            static u32 errorTextures[2] = { 0, 0 };
            static signed char errorTextures_init;
            if (!errorTextures_init)
            {
                errorTextures[0] = glGetTexture("global/white");
                errorTextures[1] = glGetTexture("global/magenta");
                errorTextures_init = 1;
            }

            for (i = 0; i < 6; i++)
            {
                if (!(glx_texconfig & (1 << i)))
                    continue;

                bool notDebug = (prev_view != GLV_Debug);
                PlatTexture* tex = glx_GetTex(p->state.texture[i], false, notDebug);

                if (tex == NULL || tex->m_bMissingTexture)
                {
                    s32 frame = glGetCurrentFrame();
                    u32 errHandle = errorTextures[(frame & 4) >> 2];
                    tex = glx_GetTex(errHandle, true, false);
                }

                memcpy(&glx_texobj[texnum], &tex->m_TexObj, sizeof(_GXTexObj));
                if (tex->m_nPaletteEntries != 0)
                {
                    memcpy(&glx_tlutobj[texnum], &tex->m_TlutObj, sizeof(_GXTlutObj));
                }

                glx_texture[texnum] = (u32)tex;
                glx_texdirty |= (1 << texnum);
                texnum++;
            }
            glx_SwitchTextureState(p);
        }

        if (flags & 0x08)
        {
            glx_SwitchRaster(p);
        }

        if (flags & 0x20)
        {
            unsigned long mtx = p->state.matrix;
            if (mtx == glGetIdentityMatrix())
            {
                modelview = mview;
            }
            else
            {
                nlMultMatrices(modelview, *(const nlMatrix4*)mtx, mview);
            }
            glxCopyMatrix(gx_modelview, modelview);
            GXLoadPosMtxImm(gx_modelview, 0);
            if (glx_normals)
            {
                if (glx_InvXpose)
                {
                    Mtx nrmMtx;
                    PSMTXInvXpose(gx_modelview, nrmMtx);
                    GXLoadNrmMtxImm(nrmMtx, 0);
                }
                else
                {
                    GXLoadNrmMtxImm(gx_modelview, 0);
                }
            }
            GXSetCurrentMtx(0);
        }

        if (flags & 0x40)
        {
            glx_SwitchStreams(p);
        }
    }

    if (flags & 0x800)
    {
        glx_DrawPacket(p);
    }
}

// Save/restore the alpha-in arg on the rasterized stage. enable=true at setup,
// false at restore (puts back whatever the previous value was).
inline void EnableTranslucent(bool enable)
{
    static _GXTevAlphaArg argSaved;
    static signed char init;
    if (!init)
    {
        argSaved = (_GXTevAlphaArg)5;
        init = 1;
    }
    if (glx_RasterizedAlphaStage >= 0 && glx_RasterizedAlphaArg >= 0)
    {
        if (enable)
        {
            argSaved = (_GXTevAlphaArg)gxSetTevAlphaIn(
                glx_RasterizedAlphaStage,
                glx_RasterizedAlphaArg,
                (glx_texconfig & 0x20) ? (_GXTevAlphaArg)3 : (_GXTevAlphaArg)3);
        }
        else
        {
            gxSetTevAlphaIn(glx_RasterizedAlphaStage, glx_RasterizedAlphaArg, argSaved);
        }
    }
}

inline void EnableNoRasterizedAlpha(bool enable)
{
    static _GXTevAlphaArg argSaved;
    static signed char init;
    if (!init)
    {
        argSaved = (_GXTevAlphaArg)5;
        init = 1;
    }
    if (glx_RasterizedAlphaStage >= 0 && glx_RasterizedAlphaArg >= 0)
    {
        if (enable)
        {
            argSaved = (_GXTevAlphaArg)gxSetTevAlphaIn(
                glx_RasterizedAlphaStage, glx_RasterizedAlphaArg, (_GXTevAlphaArg)3);
        }
        else
        {
            gxSetTevAlphaIn(glx_RasterizedAlphaStage, glx_RasterizedAlphaArg, argSaved);
        }
    }
}

inline void glud_Ambient(void* pData)
{
    nlColour colour;
    nlFloatColour* pColour = (nlFloatColour*)pData;
    colour.c[0] = (u8)(255.0f * pColour->c[0]);
    colour.c[1] = (u8)(255.0f * pColour->c[1]);
    colour.c[2] = (u8)(255.0f * pColour->c[2]);
    colour.c[3] = (u8)(255.0f * pColour->c[3]);
    gxSetChanAmbColour(0, colour);
}

inline void glud_Diffuse(void* pData)
{
    nlColour colour;
    nlFloatColour* pColour = (nlFloatColour*)pData;
    colour.c[0] = (u8)(255.0f * pColour->c[0]);
    colour.c[1] = (u8)(255.0f * pColour->c[1]);
    colour.c[2] = (u8)(255.0f * pColour->c[2]);
    colour.c[3] = (u8)(255.0f * pColour->c[3]);
    gxSetChanMatColour(0, colour);

    u32 lightMask = glx_prevLightMask;
    if (lightMask)
    {
        if (g_bAllowLighting)
        {
            glx_prevLightMask = lightMask;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, lightMask, GX_DF_CLAMP, GX_AF_SPOT);
        }
    }
}

inline void glx_LoadDirectionalLight(GLDirectionalLightUserData* pLight, GXLightID lightID)
{
    GXLightObj light;
    GXColor colour;
    {
        int value = (int)(pLight->colour.c[0] * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (u8)value;
        }
        colour.r = value;
    }
    {
        int value = (int)(pLight->colour.c[1] * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (u8)value;
        }
        colour.g = value;
    }
    {
        int value = (int)(pLight->colour.c[2] * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (u8)value;
        }
        colour.b = value;
    }
    {
        int value = (int)(pLight->colour.c[3] * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (u8)value;
        }
        colour.a = value;
    }

    GXInitLightColor(&light, colour);

    nlVector3 viewDir;
    nlMultDirVectorMatrix(viewDir, pLight->direction, mview);
    viewDir.f.z = 1048576.0f * viewDir.f.z;
    viewDir.f.x = 1048576.0f * viewDir.f.x;
    viewDir.f.y = 1048576.0f * viewDir.f.y;
    GXInitLightPos(&light, viewDir.f.x, viewDir.f.y, viewDir.f.z);

    GXInitLightAttnA(&light, 1.0f, 0.0f, 0.0f);
    GXInitLightDistAttn(&light, 1048576.0f, 1.0f, GX_DA_OFF);
    GXLoadLightObjImm(&light, lightID);
}

inline void glud_DirectionalLight(void* pData)
{
    static u32 gxLights[4] = { 1, 2, 4, 8 };

    u32 numLights = *(u32*)pData;
    glx_ReloadPointLights = true;
    if (numLights != 0)
    {
        GLDirectionalLightUserData* pLight = (GLDirectionalLightUserData*)((u8*)pData + 4);
        GLDirectionalLightUserData* pEndLight = pLight + numLights;
        u32* pGXLight = gxLights;
        u32 lightMask = 0;
        int index = 0;

        while (pLight < pEndLight)
        {
            if (index >= 4)
            {
                break;
            }

            GLDirectionalLightUserData* pCurrentLight = pLight;
            lightMask |= *pGXLight++;
            GXLightID lightID = (GXLightID)gxLights[index++];
            pLight++;

            glx_LoadDirectionalLight(pCurrentLight, lightID);
        }

        if (g_bAllowLighting != 0)
        {
            glx_prevLightMask = lightMask;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, lightMask, GX_DF_CLAMP, GX_AF_SPOT);
        }
    }
}

inline void glud_ShadowVolume(void* pData)
{
    if (*(s32*)pData == 3)
    {
        static bool bEnabled;
        static signed char init;
        static GXColor c0 = {
            64,
            64,
            64,
            64,
        };

        glx_DirtyFlags = 0x80;

        if (!init)
        {
            bEnabled = false;
            init = 1;
        }

        GXSetTevSwapMode((GXTevStageID)0, (GXTevSwapSel)0, (GXTevSwapSel)3);
        bEnabled = true;
        gxSetNumTevStages(3);

        gxSetTevAlphaOp(0, (_GXTevOp)0xE, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
        gxSetTevAlphaOp(1, (_GXTevOp)1, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);

        GXSetTevColorIn((GXTevStageID)0, (GXTevColorArg)0xF, (GXTevColorArg)0xF, (GXTevColorArg)0xF, (GXTevColorArg)4);
        GXSetTevColorIn((GXTevStageID)1, (GXTevColorArg)0xF, (GXTevColorArg)0xF, (GXTevColorArg)0xF, (GXTevColorArg)0);
        GXSetTevColorIn((GXTevStageID)2, (GXTevColorArg)0xF, (GXTevColorArg)0xF, (GXTevColorArg)0xF, (GXTevColorArg)0);

        gxSetTevAlphaIn(0, (_GXTevAlphaArg)4, (_GXTevAlphaArg)1, (_GXTevAlphaArg)6, (_GXTevAlphaArg)7);
        gxSetTevAlphaIn(1, (_GXTevAlphaArg)7, (_GXTevAlphaArg)6, (_GXTevAlphaArg)0, (_GXTevAlphaArg)6);
        gxSetTevAlphaIn(2, (_GXTevAlphaArg)7, (_GXTevAlphaArg)0, (_GXTevAlphaArg)2, (_GXTevAlphaArg)7);

        GXSetTevColor((GXTevRegID)1, c0);
        GXSetTevColor((GXTevRegID)2, rshadow_colour[prev_view == GLV_ShadowBlend1]);
    }
}

inline void glud_Translucent(void* pData)
{
    int alpha = (int)(*(float*)pData * 255.5f);
    if (alpha < 0)
    {
        alpha = 0;
    }
    if (alpha > 255)
    {
        alpha = 255;
    }

    GXColor c = {
        (u8)alpha,
        (u8)alpha,
        (u8)alpha,
        (u8)alpha,
    };
    GXSetTevColor((GXTevRegID)3, c);
    glx_translucent = true;
}

inline void glud_NoRasterizedAlpha()
{
    GXColor c = {
        255,
        255,
        255,
        255,
    };
    GXSetTevColor((GXTevRegID)3, c);
    glx_norasterizedalpha = true;
}

inline void glud_Scissor(const GLScissorUserData* pScissor)
{
    u32 xOrig;
    u32 yOrig;
    u32 wd;
    u32 ht;

    if (pScissor == NULL)
    {
        xOrig = 0;
        yOrig = xOrig;
        wd = 640;
        ht = 448;
    }
    else
    {
        xOrig = pScissor->xOrig;
        yOrig = pScissor->yOrig;
        wd = pScissor->wd;
        ht = pScissor->ht;
    }
    GXSetScissor(xOrig, yOrig, wd, ht);
}

inline void glud_EnvDiffuse(bool bOn)
{
    if (!glx_allowSpecular)
    {
        return;
    }

    if (bDeferredEnvDiffuse)
    {
        glx_envdiffuse = bOn;
        return;
    }

    Mtx texs;
    Mtx text;
    Mtx invMat;
    Mtx envMat;

    PSMTXScale(texs, 0.5f, -0.5f, 0.0f);
    PSMTXTrans(text, 0.5f, 0.5f, 1.0f);
    PSMTXConcat(text, texs, envMat);
    GXLoadTexMtxImm(envMat, 0x5B, GX_MTX3x4);

    PSMTXInvXpose(gx_modelview, invMat);
    GXLoadTexMtxImm(invMat, 0x39, GX_MTX3x4);

    GXTexCoordID coord = (GXTexCoordID)glx_GlossMapCoord;
    GXTevStageID stage = (GXTevStageID)glx_GlossMapStage;
    GXSetTexCoordGen2(coord, GX_TG_MTX3x4, GX_TG_NRM, 0x39, GX_TRUE, 0x5B);
    GXSetTevOrder(stage, coord, (GXTexMapID)coord, GX_COLOR0A0);

    if (glx_texconfig & 0x20)
    {
        GXSetTevColorIn(stage, (GXTevColorArg)0xF, (GXTevColorArg)0xB, (GXTevColorArg)8, (GXTevColorArg)0xF);
    }
    else
    {
        GXSetTevColorIn(stage, (GXTevColorArg)0xF, (GXTevColorArg)0xB, (GXTevColorArg)8, (GXTevColorArg)0);
    }
}

inline void glud_Viewport(void* pData)
{
    glx_viewport = true;
    GLViewportUserData* pViewport = (GLViewportUserData*)pData;
    g_viewport.x = pViewport->x;
    g_viewport.y = pViewport->y;
    g_viewport.w = pViewport->w;
    g_viewport.h = pViewport->h;
    g_viewport.view = pViewport->view;
    g_viewport.projection = pViewport->projection;
}

/**
 * Offset/Address/Size: 0x538 | 0x801BA038 | size: 0xB20
 * TODO: 97.91% match - p/bFogWasDisabled/bIndirect registers differ; texture-dirty loop
 * pointer registers, env stage register, viewport height temp, and display-list pointer
 * register still differ.
 */
static void glx_DrawPacket(const glModelPacket* p)
{
    static float indMtx[2][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
    };
    static _GXPrimitive primitives[6] = {
        GX_TRIANGLES,
        GX_TRIANGLESTRIP,
        GX_TRIANGLEFAN,
        GX_QUADS,
        GX_LINES,
        GX_LINESTRIP,
    };

    u32 i;
    u32 j;
    bool bFogWasDisabled = false;
    bool bIndirect = false;
    u32 mask;
    _GXTlut tlutID;
    s32 vh;
    s32 vx;
    s32 vy;
    s32 vw;

    // === Block 1: WarbleBlend indirect-texture setup ===
    if ((prev_view == GLV_WarbleBlend) && (p->state.texture[0] == ColourTargetTexture))
    {
        PlatTexture* tex = glx_GetTex(glGetTexture("target/offset"), true, true);

        glx_texture[1] = (u32)tex;
        memcpy(&glx_texobj[1], &tex->m_TexObj, sizeof(GXTexObj));
        GXInitTexObjWrapMode(&glx_texobj[1], (GXTexWrapMode)0, (GXTexWrapMode)0);
        GXInitTexObjFilter(&glx_texobj[1], (GXTexFilter)1, (GXTexFilter)1);
        glx_texdirty |= 2;

        GXSetNumIndStages(1);
        GXSetIndTexOrder((GXIndTexStageID)0, (GXTexCoordID)0, (GXTexMapID)1);
        GXSetIndTexCoordScale((GXIndTexStageID)0, (GXIndTexScale)1, (GXIndTexScale)1);
        GXSetTevIndWarp((GXTevStageID)0, (GXIndTexStageID)0, 1, 0, (GXIndTexMtxID)1);

        {
            float scale = 1.0f / glx_IndDivisor;
            indMtx[0][0] = scale;
            indMtx[1][1] = scale;
        }

        GXSetIndTexMtx((GXIndTexMtxID)1, indMtx, 1);
        bIndirect = true;
    }

    // === Block 2: Texture dirty upload loop ===
    if (glx_texdirty != 0)
    {
        for (i = 0; i < 6; i++)
        {
            mask = (1 << i);
            if (glx_texdirty & mask)
            {
                PlatTexture* tex = (PlatTexture*)glx_texture[i];
                if (tex->m_nPaletteEntries != 0)
                {
                    GXInitTexObjTlut(&glx_texobj[i], (GXTlut)i);
                    GXLoadTlut(&glx_tlutobj[i], (GXTlut)i);
                }

                GXLoadTexObj(&glx_texobj[i], (GXTexMapID)i);
                glx_texdirty &= ~mask;
            }
        }
    }

    // === Block 3: CoPlanar + Fog disable ===
    gxSetCoPlanar(glx_CoPlanar);

    if (glx_NoFog && glx_GetFog())
    {
        bFogWasDisabled = true;
        glx_Fog(false);
    }

    // === Block 4: Env-diffuse / Mobile-diffuse matrix setup ===
    if (glx_envdiffuse)
    {
        Mtx finalMtx;
        Mtx invMtx;
        Mtx transMtx;
        Mtx scaleMtx;
        u32 glossMapStage;
        u32 glossMapCoord;

        PSMTXScale(scaleMtx, 0.5f, -0.5f, 0.0f);
        PSMTXTrans(transMtx, 0.5f, 0.5f, 1.0f);
        PSMTXConcat(transMtx, scaleMtx, finalMtx);
        GXLoadTexMtxImm(finalMtx, 0x5B, (_GXTexMtxType)0);

        PSMTXInvXpose(gx_modelview, invMtx);
        GXLoadTexMtxImm(invMtx, 0x39, (_GXTexMtxType)0);

        glossMapCoord = glx_GlossMapCoord;
        glossMapStage = glx_GlossMapStage;

        GXSetTexCoordGen2((GXTexCoordID)glossMapCoord, (GXTexGenType)0, (GXTexGenSrc)1, 0x39, true, 0x5B);
        GXSetTevOrder((GXTevStageID)glossMapStage, (GXTexCoordID)glossMapCoord, (GXTexMapID)glossMapCoord, (GXChannelID)4);

        if (glx_texconfig & 0x20)
        {
            GXSetTevColorIn((GXTevStageID)glossMapStage, (GXTevColorArg)15, (GXTevColorArg)11, (GXTevColorArg)8, (GXTevColorArg)15);
        }
        else
        {
            GXSetTevColorIn((GXTevStageID)glossMapStage, (GXTevColorArg)15, (GXTevColorArg)11, (GXTevColorArg)8, (GXTevColorArg)0);
        }
    }
    else if (glx_mobilediffuse)
    {
        Mtx transMtx;
        Mtx scaleMtx;
        Mtx mvCopy;
        static int n;
        static signed char init;

        gxSetTexCoordGen(0, (_GXTexGenType)0, (_GXTexGenSrc)1, 0x39);
        memcpy(mvCopy, gx_modelview, sizeof(Mtx));
        PSMTXScale(scaleMtx, 0.5f, -0.5f, 0.0f);
        PSMTXTrans(transMtx, 0.5f, 0.5f, 1.0f);
        PSMTXConcat(scaleMtx, mvCopy, mvCopy);
        PSMTXConcat(transMtx, mvCopy, mvCopy);

        if (!init)
        {
            n = 5;
            init = 1;
        }

        float invN = 1.0f / (float)n;
        u32 frame = glGetCurrentFrame();
        u32 frameDiv = frame / n;
        u32 frameMod = frame - frameDiv * n;
        float offset = invN * frameMod;
        offset = 2.0f * offset - 1.0f;
        mvCopy[0][2] = offset;
        mvCopy[1][2] = offset;

        GXLoadTexMtxImm(mvCopy, 0x39, (_GXTexMtxType)0);
    }

    // === Block 5: Alpha state (translucent / norasterized / constantcolour) ===
    if (glx_norasterizedalpha)
    {
        EnableNoRasterizedAlpha(true);
    }
    else if (glx_translucent)
    {
        EnableTranslucent(true);
    }
    else if (glx_constantcolour)
    {
        if (glx_texconfig == 1)
        {
            GXSetTevColorIn((GXTevStageID)0, (GXTevColorArg)15, (GXTevColorArg)6, (GXTevColorArg)8, (GXTevColorArg)15);
            gxSetTevAlphaIn(0, (_GXTevAlphaArg)7, (_GXTevAlphaArg)3, (_GXTevAlphaArg)4, (_GXTevAlphaArg)7);
        }
        else if (glx_texconfig == 0x21)
        {
            GXSetTevColorIn((GXTevStageID)1, (GXTevColorArg)15, (GXTevColorArg)6, (GXTevColorArg)8, (GXTevColorArg)15);
        }
    }

    // === Block 6: Viewport setup (when glx_viewport flag is set) ===
    if (glx_viewport)
    {
        Mtx mv34;
        Mtx44 proj44;
        nlMatrix4 srcView;
        nlMatrix4 srcProj;
        vx = g_viewport.x;
        vy = g_viewport.y;
        vw = g_viewport.w;
        vh = g_viewport.h;

        memcpy(&srcProj, (const void*)g_viewport.projection, sizeof(nlMatrix4));
        memcpy(&srcView, (const void*)g_viewport.view, sizeof(nlMatrix4));
        glxCopyMatrix(proj44, srcProj);
        glxCopyMatrix(mv34, srcView);
        GXLoadPosMtxImm(mv34, 0);

        _GXProjectionType setupProjType;
        if (-1.0f == srcProj.e[14])
        {
            setupProjType = (_GXProjectionType)0;
        }
        else
        {
            setupProjType = (_GXProjectionType)1;
        }
        GXSetProjection(proj44, setupProjType);
        GXSetCurrentMtx(0);
        GXSetViewport((float)vx, (float)vy, (float)vw, (float)vh, 0.0f, 1.0f);
        GXSetScissor(vx, vy, vw, vh);
    }

    // === Block 7: Draw - emit primitives or display list ===
    if (p->indexBuffer == 0)
    {
        GXBegin(primitives[p->primType], (_GXVtxFmt)gx_vtxfmt, p->numVertices);

        for (i = 0; i < p->numVertices; i++)
        {
            for (j = 0; j < glx_NumIndices; j++)
            {
                GXWGFifo.u16 = (u16)i;
            }
        }
    }
    else
    {
        if (glx_NumIndices == 0)
        {
            GXCallDisplayList(dlGetDisplayList(p->indexBuffer), dlGetSize(p->indexBuffer));
        }
        else if (glx_CompiledDraw && (glx_NumIndices == p->numStreams) && dlIsDisplayList(p->indexBuffer))
        {
            GXCallDisplayList(dlGetDisplayList(p->indexBuffer), dlGetSize(p->indexBuffer));
        }
        else if (glx_AllowUncompiledDraws && glGetRasterState(p->state.raster, (eGLState)8) != 1)
        {
            if (dlIsDisplayList(p->indexBuffer))
            {
                DisplayList* dl = dlGetStruct(p->indexBuffer);
                GXBegin(primitives[p->primType], (_GXVtxFmt)gx_vtxfmt, p->numVertices);
                for (j = 0; j < p->numVertices; j++)
                {
                    for (i = 0; i < glx_NumIndices; i++)
                    {
                        u16* ptr;
                        if (((u16*)&dl->indices)[1] != 0)
                        {
                            u16 ns = ((u16*)&dl->indices)[0];
                            s32 stride = (ns - 1) * 2 + 1;
                            s32 offset = stride * j;
                            u8* ptr8 = (u8*)dl->list + offset;
                            ptr = (u16*)ptr8;
                            ptr8 = (u8*)ptr;
                            ptr8 += 4;
                            ptr = (u16*)ptr8;
                        }
                        else
                        {
                            u16 ns = ((u16*)&dl->indices)[0];
                            s32 stride = ns * 2;
                            s32 offset = j * stride;
                            u8* ptr8 = (u8*)dl->list;
                            ptr8 += offset;
                            ptr = (u16*)(ptr8 + 3);
                        }
                        GXWGFifo.u16 = *ptr;
                    }
                }
            }
            else
            {
                u16* idxPtr = (u16*)p->indexBuffer;
                GXBegin(primitives[p->primType], (_GXVtxFmt)gx_vtxfmt, p->numVertices);
                for (i = 0; i < p->numVertices; i++)
                {
                    for (j = 0; j < glx_NumIndices; j++)
                    {
                        GXWGFifo.u16 = idxPtr[i];
                    }
                }
            }
        }
    }

    // === Block 8: Restore states ===
    if (bIndirect)
    {
        GXSetNumIndStages(0);
        GXSetTevDirect((GXTevStageID)0);
    }

    if (bFogWasDisabled)
    {
        glx_Fog(true);
    }

    // === Block 8b: Env-diffuse / Mobile-diffuse tex-coord-gen restore ===
    if (glx_envdiffuse)
    {
        if (glx_GlossMapStage >= 0)
        {
            gxSetTexCoordGen(glx_GlossMapStage, (_GXTexGenType)1, (_GXTexGenSrc)(glx_GlossMapStage + 4), 0x3C);
            glx_DirtyFlags |= glv_TexConfigChanged;
        }
    }
    else if (glx_mobilediffuse)
    {
        gxSetTexCoordGen(0, (_GXTexGenType)1, (_GXTexGenSrc)4, 0x3C);
    }

    // === Block 8c: Alpha state restore ===
    if (glx_norasterizedalpha)
    {
        EnableNoRasterizedAlpha(false);
    }
    else if (glx_translucent)
    {
        EnableTranslucent(false);
    }
    else if (glx_constantcolour)
    {
        if (glx_texconfig == 1)
        {
            GXSetTevColorIn((GXTevStageID)0, (GXTevColorArg)15, (GXTevColorArg)10, (GXTevColorArg)8, (GXTevColorArg)15);
            gxSetTevAlphaIn(0, (_GXTevAlphaArg)7, (_GXTevAlphaArg)5, (_GXTevAlphaArg)4, (_GXTevAlphaArg)7);
        }
        else
        {
            GXSetTevColorIn((GXTevStageID)1, (GXTevColorArg)15, (GXTevColorArg)10, (GXTevColorArg)8, (GXTevColorArg)15);
        }
    }

    // === Block 9: Viewport restore (when user-viewport was active) ===
    if (glx_viewport)
    {
        GXLoadPosMtxImm(gx_modelview, 0);

        _GXProjectionType projType;
        if (-1.0f == mproj.m[3][2])
        {
            projType = (_GXProjectionType)0;
        }
        else
        {
            projType = (_GXProjectionType)1;
        }
        GXSetProjection(gx_proj, projType);
        GXSetCurrentMtx(0);
        GXSetViewport(0.0f, 0.0f, 640.0f, 448.0f, 0.0f, 1.0f);
        GXSetScissor(0, 0, 0x280, 0x1C0);
    }

    GXSetCurrentMtx(0);
}

/**
 * Offset/Address/Size: 0x1058 | 0x801BAB58 | size: 0x964
 */
static void glx_SwitchUserData(const glModelPacket* p)
{
    if (!init)
    {
        bDeferredEnvDiffuse = true;
        init = 1;
    }

    if (glx_AlwaysReloadLights)
    {
        glx_ReloadPointLights = true;
        glx_ReloadSpecLights = true;
    }

    {
        nlColour amb;
        nlColour temp = { };
        nlColourSet(temp, world_ambient.c[0], world_ambient.c[1], world_ambient.c[2], world_ambient.c[3]);
        amb = temp;
        gxSetChanAmbColour(0, amb);
    }

    if (glx_prevLightMask)
    {
        GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, (GXLightID)glx_prevLightMask, GX_DF_NONE, GX_AF_NONE);
        glx_prevLightMask = 0;
    }

    if (glx_prevSpecMask)
    {
        gxSetNumChans(1);
        GXSetChanCtrl(GX_COLOR1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, (GXLightID)glx_prevSpecMask, GX_DF_NONE, GX_AF_NONE);
        glx_prevSpecMask = 0;
    }

    if (bDeferredEnvDiffuse)
    {
        glx_envdiffuse = false;
    }
    else if (glx_GlossMapStage >= 0)
    {
        gxSetTexCoordGen(glx_GlossMapStage, GX_TG_MTX2x4, (_GXTexGenSrc)(glx_GlossMapStage + 4), GX_IDENTITY);
        glx_DirtyFlags |= glv_TexConfigChanged;
    }

    glx_mobilediffuse = false;
    glx_constantcolour = false;
    glx_viewport = false;
    glx_translucent = false;
    glx_norasterizedalpha = false;
    glx_NoFog = false;
    GXSetScissor(0, 0, 640, 448);
    glx_CoPlanar = false;

    if (p == NULL)
    {
        return;
    }

    unsigned long* pTable = (unsigned long*)p->userData;
    if (pTable == NULL)
    {
        return;
    }

    if (prev_view == GLV_ShadowTexture)
    {
        if (pTable[GLUD_Skin] != 0)
        {
            if (pTable[GLUD_Viewport] != 0)
            {
                void* pViewportData = glUserGetData((void*)pTable[GLUD_Viewport]);
                memcpy(&mview, *(void**)((u8*)pViewportData + 8), 0x40);
                glxCopyMatrix(gx_mview, mview);
            }
        }
    }

    for (int i = 0; i < GLUD_Num; i++, pTable++)
    {
        if (*pTable == 0)
        {
            continue;
        }

        void* pData = glUserGetData((void*)*pTable);

        switch (i)
        {
        case GLUD_CoPlanar:
            glx_CoPlanar = true;
            break;

        case GLUD_NoFog:
            glx_NoFog = true;
            break;

        case GLUD_Ambient:
            glud_Ambient(pData);
            break;

        case GLUD_Diffuse:
            glud_Diffuse(pData);
            break;

        case GLUD_Light:
            glud_Light(pData);
            break;

        case GLUD_DirectionalLight:
            glud_DirectionalLight(pData);
            break;

        case GLUD_Specular:
            if (glx_allowSpecular)
            {
                glud_Specular(pData);
            }
            break;

        case GLUD_ShadowVolume:
            glud_ShadowVolume(pData);
            break;

        case GLUD_Translucent:
            glud_Translucent(pData);
            break;

        case GLUD_NoRasterizedAlpha:
            glud_NoRasterizedAlpha();
            break;

        case GLUD_Scissor:
            glud_Scissor((GLScissorUserData*)pData);
            break;

        case GLUD_EnvDiffuse:
            glud_EnvDiffuse(true);
            break;

        case GLUD_MobileDiffuse:
            glx_mobilediffuse = true;
            break;

        case GLUD_Skin:
            glud_Skin(pData, p);
            break;

        case GLUD_ConstantColour:
            glx_constantcolour = true;
            GXSetTevColor((GXTevRegID)3, *(GXColor*)pData);
            break;

        case GLUD_Viewport:
            glud_Viewport(pData);
            break;

        default:
            break;
        }
    }
}

struct GLSkinUserData
{
    int reg;
    float mat[12];
};

/**
 * Offset/Address/Size: 0x19BC | 0x801BB4BC | size: 0x1D0
 */
void glud_Skin(void* pData, const glModelPacket* pPacket)
{
    float mSkinConcat[3][4];
    float mNormFast[3][4];
    nlMatrix4 nlMat;
    float tempMtx[3][4];
    float gxMat[3][4];
    float mNorm[3][4];
    u32 numMatrices;
    u32 i;
    int slot;
    GLSkinUserData* pSkin;
    u8 bInvXpose;

    numMatrices = *(u32*)pData;
    pSkin = (GLSkinUserData*)((u8*)pData + 4);

    if (prev_view == GLV_Characters)
        bInvXpose = glx_InvXposeChar;
    else
        bInvXpose = glx_InvXpose;

    if (g_bFastSkinPath && g_bMtxSkinMath && !glx_IsCoPlanarView && pPacket->state.matrix == glGetIdentityMatrix())
    {
        // Fast path - use gx_mview directly
        for (i = 0; i < numMatrices; i++, pSkin++)
        {
            PSMTXConcat(gx_mview, *(const Mtx*)pSkin->mat, mSkinConcat);
            slot = pSkin->reg + 99;
            GXLoadPosMtxImm(mSkinConcat, (u32)slot);
            if (bInvXpose)
            {
                PSMTXInvXpose(mSkinConcat, mNormFast);
                GXLoadNrmMtxImm(mNormFast, (u32)slot);
            }
            else
            {
                GXLoadNrmMtxImm(mSkinConcat, (u32)slot);
            }
        }
    }
    else
    {
        // Slow path
        for (i = 0; i < numMatrices; i++, pSkin++)
        {
            if (g_bMtxSkinMath && !glx_IsCoPlanarView)
            {
                glxCopyMatrix(gxMat, *(nlMatrix4*)pPacket->state.matrix);
                PSMTXConcat(gxMat, gx_mview, tempMtx);
                PSMTXConcat(tempMtx, *(const Mtx*)pSkin->mat, mSkinConcat);
            }
            else
            {
                nlMultMatrices(nlMat, *(nlMatrix4*)pPacket->state.matrix, mview);
                glxCopyMatrix(tempMtx, nlMat);
                PSMTXConcat(tempMtx, *(const Mtx*)pSkin->mat, mSkinConcat);
            }
            slot = pSkin->reg + 99;
            GXLoadPosMtxImm(mSkinConcat, (u32)slot);
            if (bInvXpose)
            {
                PSMTXInvXpose(mSkinConcat, mNorm);
                GXLoadNrmMtxImm(mNorm, (u32)slot);
            }
            else
            {
                GXLoadNrmMtxImm(mSkinConcat, (u32)slot);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1B8C | 0x801BB68C | size: 0x320
 * TODO: 99.91% match - viewDir x/y store order at 0x120/0x124 and
 *       GXColor stack copy direction still differ.
 */
static void glud_Specular(void* pData)
{
    static float SpecularFudge;
    static signed char init;
    static u32 gxLights[4] = { 1, 2, 4, 8 };

    unsigned long* p32 = (unsigned long*)pData;
    unsigned long numLights = *p32;
    unsigned long lightMask;
    int index;
    GLSpecularUserData* pLight;
    GLSpecularUserData* pEndLight;

    if (numLights != 0)
    {
        pLight = (GLSpecularUserData*)((unsigned char*)pData + 4);
        index = 0;
        lightMask = 0;
        pEndLight = &pLight[numLights];

        while (pLight < pEndLight)
        {
            if (index >= 4)
            {
                break;
            }

            GXLightID lightID = (GXLightID)gxLights[index];
            lightMask |= lightID;

            if (glx_ReloadSpecLights)
            {
                GXLightObj light;
                nlVector3 viewDir;
                nlVector3 worldDir;
                GXColor colour;

                if (!init)
                {
                    init = 1;
                    SpecularFudge = 1.25f;
                }

                {
                    float recipLength = nlRecipSqrt(
                        pLight->worldDirection.f.x * pLight->worldDirection.f.x + pLight->worldDirection.f.y * pLight->worldDirection.f.y + pLight->worldDirection.f.z * pLight->worldDirection.f.z,
                        false);
                    nlVec3Set(worldDir, recipLength * pLight->worldDirection.f.x, recipLength * pLight->worldDirection.f.y, recipLength * pLight->worldDirection.f.z);
                }

                nlMultDirVectorMatrix(viewDir, worldDir, mview);

                {
                    float recipLength = nlRecipSqrt(
                        viewDir.f.x * viewDir.f.x + viewDir.f.y * viewDir.f.y + viewDir.f.z * viewDir.f.z,
                        false);

                    viewDir.f.z = recipLength * viewDir.f.z;
                    viewDir.f.y = recipLength * viewDir.f.y;
                    viewDir.f.x = recipLength * viewDir.f.x;
                }

                {
                    int value = (int)(pLight->colour.c[0] * pLight->intensity * SpecularFudge * 255.5f);
                    if (value < 0)
                    {
                        value = 0;
                    }
                    else if (value > 255)
                    {
                        value = 255;
                    }
                    else
                    {
                        value = (unsigned char)value;
                    }
                    colour.r = value;
                }
                {
                    int value = (int)(pLight->colour.c[1] * pLight->intensity * SpecularFudge * 255.5f);
                    if (value < 0)
                    {
                        value = 0;
                    }
                    else if (value > 255)
                    {
                        value = 255;
                    }
                    else
                    {
                        value = (unsigned char)value;
                    }
                    colour.g = value;
                }
                {
                    int value = (int)(pLight->colour.c[2] * pLight->intensity * SpecularFudge * 255.5f);
                    if (value < 0)
                    {
                        value = 0;
                    }
                    else if (value > 255)
                    {
                        value = 255;
                    }
                    else
                    {
                        value = (unsigned char)value;
                    }
                    colour.b = value;
                }
                {
                    int value = (int)(pLight->colour.c[3] * pLight->intensity * SpecularFudge * 255.5f);
                    if (value < 0)
                    {
                        value = 0;
                    }
                    else if (value > 255)
                    {
                        value = 255;
                    }
                    else
                    {
                        value = (unsigned char)value;
                    }
                    colour.a = value;
                }

                GXInitLightColor(&light, colour);
                GXInitSpecularDir(&light, viewDir.f.x, viewDir.f.y, viewDir.f.z);

                {
                    float half = 0.5f;
                    float halfExponent = pLight->exponent * half;
                    GXInitLightAttn(&light, 0.0f, 0.0f, 1.0f, halfExponent, 0.0f, 1.0f - halfExponent);
                }

                GXLoadLightObjImm(&light, lightID);
            }

            pLight++;
            index += 1;
        }

        glx_ReloadSpecLights = 0;
        if (g_bAllowSpecular)
        {
            glx_prevSpecMask = lightMask;
            gxSetNumChans(2);
            GXSetChanCtrl(GX_COLOR1, GX_TRUE, GX_SRC_REG, GX_SRC_REG, lightMask, GX_DF_NONE, GX_AF_SPEC);
        }
    }
}

struct LightData
{
    u32 numLights;
    GLLightUserData* lights;
};

/**
 * Offset/Address/Size: 0x1EAC | 0x801BB9AC | size: 0xC0
 */
void glud_Light(void* pUserData)
{
    static u32 gxLights[4] = { 1, 2, 4, 8 };

    LightData* lightData = (LightData*)pUserData;
    u32 lightMask;
    s32 light_id;
    GLLightUserData* pLight;
    GLLightUserData* pEndLight;

    if (lightData->numLights != 0)
    {
        pLight = (GLLightUserData*)((u8*)pUserData + 4);
        light_id = 0;
        lightMask = 0;
        pEndLight = &pLight[lightData->numLights];
        while (pLight < pEndLight)
        {
            if (light_id >= 4)
            {
                break;
            }
            lightMask |= gxLights[light_id];
            if (glx_ReloadPointLights != 0)
            {
                glx_LoadLight(pLight, (GXLightID)gxLights[light_id]);
            }
            pLight++;
            light_id += 1;
        }

        glx_ReloadPointLights = 0;
        if (g_bAllowLighting != 0)
        {
            glx_prevLightMask = lightMask;
            GXSetChanCtrl(GX_COLOR0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, lightMask, GX_DF_CLAMP, GX_AF_SPOT);
        }
    }
}

/**
 * Offset/Address/Size: 0x1F6C | 0x801BBA6C | size: 0x334
 * TODO: 96.22% match - directional branch register/order mismatch in worldDir normalization
 *       and viewDir scaling by -1.0f; static local/sdata constant references also differ.
 */
static void glx_LoadLight(GLLightUserData* pLight, _GXLightID lightId)
{
    static float refMult;
    static signed char init;
    static float refBright;
    static signed char init_0;
    static int refFunc;
    static signed char init_1;
    static GXDistAttnFn dist_func[3] = {
        GX_DA_GENTLE,
        GX_DA_MEDIUM,
        GX_DA_STEEP,
    };

    GXLightObj light;
    nlVector3 viewPos;
    GXColor colour;
    nlVector3 viewDir;
    nlVector3 worldDir;

    if (!init)
    {
        init = 1;
        refMult = 1.0f;
    }

    if (!init_0)
    {
        init_0 = 1;
        refBright = 0.5f;
    }

    if (!init_1)
    {
        refFunc = 2;
        init_1 = 1;
    }

    {
        int value = (int)(pLight->colour.c[0] * pLight->intensity * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (unsigned char)value;
        }
        colour.r = value;
    }

    {
        int value = (int)(pLight->colour.c[1] * pLight->intensity * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (unsigned char)value;
        }
        colour.g = value;
    }

    {
        int value = (int)(pLight->colour.c[2] * pLight->intensity * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (unsigned char)value;
        }
        colour.b = value;
    }

    {
        int value = (int)(pLight->colour.c[3] * pLight->intensity * 255.5f);
        if (value < 0)
        {
            value = 0;
        }
        else if (value > 255)
        {
            value = 255;
        }
        else
        {
            value = (unsigned char)value;
        }
        colour.a = value;
    }

    GXInitLightColor(&light, colour);

    if (0.0f == pLight->outerRadius)
    {
        nlVector3 origin = {
            0.0f,
            0.0f,
            0.0f,
        };

        float worldY = pLight->worldPosition.f.y - origin.f.y;
        float worldX = pLight->worldPosition.f.x - origin.f.x;
        float worldZ = pLight->worldPosition.f.z - origin.f.z;

        worldDir.f.x = worldX;
        worldDir.f.y = worldY;
        worldDir.f.z = worldZ;

        {
            float lengthSq = worldDir.f.x * worldDir.f.x;
            lengthSq += worldDir.f.y * worldDir.f.y;
            lengthSq += worldDir.f.z * worldDir.f.z;
            float recipLength = nlRecipSqrt(lengthSq, true);

            nlVec3Scale(worldDir, recipLength);
        }

        nlMultDirVectorMatrix(viewDir, worldDir, mview);

        nlVec3Scale(viewDir, -1.0f);

        GXInitLightPos(&light, viewDir.f.x, viewDir.f.y, viewDir.f.z);
        GXInitLightAttnA(&light, 1.0f, 0.0f, 0.0f);
        GXInitLightDistAttn(&light, -1.0f, 1.0f, GX_DA_OFF);
    }
    else
    {
        nlMultPosVectorMatrix(viewPos, pLight->worldPosition, mview);
        GXInitLightPos(&light, viewPos.f.x, viewPos.f.y, viewPos.f.z);
        GXInitLightAttnA(&light, 1.0f, 0.0f, 0.0f);
        GXInitLightDistAttn(&light, refMult * pLight->outerRadius, refBright, dist_func[refFunc]);
    }

    GXLoadLightObjImm(&light, lightId);
}

static u32 gx_texattr[] = {
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
};

/**
 * Offset/Address/Size: 0x22A0 | 0x801BBDA0 | size: 0x194
 */
void glx_SwitchStreams(const glModelPacket* pPacket)
{
    static u32 gx_streams[] = {
        9,
        10,
        11,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        26,
        0xFF,
    };

    glModelStream* stream = pPacket->streams;
    glModelStream* end = stream + pPacket->numStreams;

    GXClearVtxDesc();
    glx_NumIndices = 0;

    while (stream < end)
    {
        if (stream->id == 12)
        {
            GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
            glx_NumIndices++;
            stream++;
            continue;
        }

        s32 attr = gx_streams[stream->id];
        if (attr == 0xFF)
        {
            attr = gx_texattr[stream->id - 3];
        }

        if (attr != 0xFF)
        {
            if (stream->address == 0)
            {
                GXSetVtxDesc((GXAttr)attr, GX_DIRECT);
            }
            else
            {
                GXSetArray((GXAttr)attr, (void*)stream->address, stream->stride);
                GXSetVtxDesc((GXAttr)attr, GX_INDEX16);
                glx_NumIndices++;
            }
        }

        if (stream->id == 1)
        {
            if (stream->stride == 12)
            {
                GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
            }
            else
            {
                GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 6);
            }
        }

        if (stream->id == 0)
        {
            if (stream->stride == 12)
            {
                GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
            }
            else
            {
                GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 8);
            }
        }

        stream++;
    }
}

/**
 * Offset/Address/Size: 0x2434 | 0x801BBF34 | size: 0x25C
 */
void glx_SwitchRaster(const glModelPacket* p)
{
    static _GXCompare gx_DepthFunc[] = {
        GX_ALWAYS,
        GX_LEQUAL,
        GX_EQUAL,
        GX_LESS,
    };
    static _GXCompare gx_AlphaTest[] = {
        GX_ALWAYS,
        GX_GREATER,
    };
    static _GXCullMode gx_Culling[] = {
        GX_CULL_NONE,
        GX_CULL_FRONT,
        GX_CULL_BACK,
        GX_CULL_ALL,
    };

    unsigned long DepthTest;
    unsigned long DepthWrite;
    int DepthFunc;
    int AlphaTest;
    unsigned long AlphaTestRef;
    unsigned long AlphaBlend;
    int Culling;
    int ColourWrite;

    glUnHandleizeRasterState(p->state.raster);

    DepthTest = glGetRasterState(GLS_DepthTest);
    DepthWrite = glGetRasterState(GLS_DepthWrite);
    DepthFunc = glGetRasterState(GLS_DepthFunc);
    gxSetZMode((bool)DepthTest, gx_DepthFunc[DepthFunc], (bool)DepthWrite);

    AlphaTest = glGetRasterState(GLS_AlphaTest);
    AlphaTestRef = glGetRasterState(GLS_AlphaTestRef);
    gxSetAlphaCompare(gx_AlphaTest[AlphaTest], (unsigned char)AlphaTestRef);

    if (AlphaTest != 0)
    {
        gxSetZCompLoc(false);
    }
    else
    {
        gxSetZCompLoc(true);
    }

    AlphaBlend = glGetRasterState(GLS_AlphaBlend);

    switch (AlphaBlend)
    {
    case 0:
        gxSetBlendMode(false, (_GXBlendFactor)1, (_GXBlendFactor)0, false);
        break;
    case 1:
        gxSetBlendMode(true, (_GXBlendFactor)4, (_GXBlendFactor)5, false);
        break;
    case 2:
        gxSetBlendMode(true, (_GXBlendFactor)1, (_GXBlendFactor)1, false);
        break;
    case 3:
        gxSetBlendMode(true, (_GXBlendFactor)4, (_GXBlendFactor)1, false);
        break;
    case 4:
        gxSetBlendMode(true, (_GXBlendFactor)2, (_GXBlendFactor)0, false);
        break;
    case 5:
        gxSetBlendMode(true, (_GXBlendFactor)3, (_GXBlendFactor)1, false);
        break;
    case 6:
        gxSetBlendMode(true, (_GXBlendFactor)1, (_GXBlendFactor)0, false);
        break;
    case 7:
        gxSetBlendMode(true, (_GXBlendFactor)2, (_GXBlendFactor)0, true);
        break;
    }

    Culling = glGetRasterState(GLS_Culling);
    gxSetCullMode(gx_Culling[Culling]);

    ColourWrite = glGetRasterState(GLS_ColourWrite);
    switch (ColourWrite)
    {
    case 0:
        gxSetColourUpdate(false);
        gxSetAlphaUpdate(false);
        break;
    case 1:
        gxSetColourUpdate(true);
        gxSetAlphaUpdate(false);
        break;
    case 2:
        gxSetColourUpdate(false);
        gxSetAlphaUpdate(true);
        break;
    case 3:
        gxSetColourUpdate(true);
        if (prev_view == GLV_ShadowTexture)
        {
            gxSetAlphaUpdate(true);
        }
        else
        {
            gxSetAlphaUpdate(false);
        }
        break;
    }
}

static inline GXColor makeKonstColor(f32 level)
{
    int val = (int)(255.5f * level);
    GXColor c = { (u8)val, (u8)val, (u8)val, (u8)val };
    return c;
}

/**
 * Offset/Address/Size: 0x2690 | 0x801BC190 | size: 0x48C
 * TODO: 95.8% match - stack offset diffs for GXColor locals (MWCC allocates
 * 2d_movie GXColors at lower offsets than konst GXColor work area, target has
 * them reversed). All instructions and registers correct.
 */
static void glx_SwitchTextureState(const glModelPacket* p)
{
    int bit;
    int texnum;
    GXTexWrapMode mode[2];
    eGLTextureMode tmode;
    eGLTextureFilter filter;

    glUnHandleizeTextureState(p->state.texturestate);

    if (glx_program == prog_2d_movie)
    {
        GXColor c0 = { 0x00, 0x00, 0xE2, 0x58 };
        GXSetTevKColor(GX_KCOLOR0, c0);
        GXColor c1 = { 0xB3, 0x00, 0x00, 0xB6 };
        GXSetTevKColor(GX_KCOLOR1, c1);
        GXColor c2 = { 0xFF, 0x00, 0xFF, 0x80 };
        GXSetTevKColor(GX_KCOLOR2, c2);
        glx_konstlevel[0] = -1.0f;
        glx_konstlevel[1] = -1.0f;
        glx_konstlevel[2] = -1.0f;
        glx_konstlevel[3] = -1.0f;
    }
    else
    {
        u8 raw;
        f32 level;
        raw = (u8)glGetTextureState(GLTS_DiffuseLevel);
        level = (f32)raw * (1.0f / 63.0f);
        if (level != glx_konstlevel[0])
        {
            GXColor c = makeKonstColor(level);
            GXSetTevKColor(GX_KCOLOR0, c);
            glx_konstlevel[0] = level;
        }

        raw = (u8)glGetTextureState(GLTS_ShadowLevel);
        level = (f32)raw * (1.0f / 63.0f);
        if (level != glx_konstlevel[1])
        {
            GXColor c = makeKonstColor(level);
            GXSetTevKColor(GX_KCOLOR1, c);
            glx_konstlevel[1] = level;
        }

        raw = (u8)glGetTextureState(GLTS_ShadowLevel);
        {
            f32 shadowLevel = (f32)raw * (1.0f / 63.0f);
            level = 1.0f - shadowLevel;
        }
        if (level != glx_konstlevel[2])
        {
            GXColor c = makeKonstColor(level);
            GXSetTevKColor(GX_KCOLOR2, c);
            glx_konstlevel[2] = level;
        }

        raw = (u8)glGetTextureState(GLTS_GlossLevel);
        level = (f32)raw * (1.0f / 63.0f);
        if (level != glx_konstlevel[3])
        {
            GXColor c = makeKonstColor(level);
            GXSetTevKColor(GX_KCOLOR3, c);
            glx_konstlevel[3] = level;
        }
    }

    texnum = 0;
    for (bit = 0; bit < 6; bit++)
    {
        if (!(glx_texconfig & (1 << bit)))
            continue;

        if (bit == 5)
        {
            mode[1] = GX_CLAMP;
            mode[0] = GX_CLAMP;
        }
        else
        {
            tmode = (eGLTextureMode)glGetTextureState((eGLTextureState)bit);
            switch (tmode)
            {
            case GLTM_WrapWrap:
                mode[0] = GX_REPEAT;
                mode[1] = GX_REPEAT;
                break;
            case GLTM_WrapClamp:
                mode[0] = GX_REPEAT;
                mode[1] = GX_CLAMP;
                break;
            case GLTM_ClampWrap:
                mode[0] = GX_CLAMP;
                mode[1] = GX_REPEAT;
                break;
            case GLTM_ClampClamp:
                mode[0] = GX_CLAMP;
                mode[1] = GX_CLAMP;
                break;
            default:
                break;
            }
        }

        GXInitTexObjWrapMode(&glx_texobj[texnum], mode[0], mode[1]);

        filter = (eGLTextureFilter)glGetTextureState((eGLTextureState)(bit + 6));
        switch (filter)
        {
        case GLTF_Linear:
        {
            PlatTexture* tex = (PlatTexture*)glx_texture[texnum];
            if (tex->m_Levels == 1)
                GXInitTexObjFilter(&glx_texobj[texnum], GX_LINEAR, GX_LINEAR);
            else if (tex->m_Format == GXTex_CI8)
                GXInitTexObjFilter(&glx_texobj[texnum], GX_LIN_MIP_NEAR, GX_LINEAR);
            else
            {
                static GXAnisotropy aniso[] = { GX_ANISO_1, GX_ANISO_2, GX_ANISO_4 };
                GXInitTexObjFilter(&glx_texobj[texnum], GX_LIN_MIP_LIN, GX_LINEAR);
                GXInitTexObjMaxAniso(&glx_texobj[texnum], aniso[glx_aniso]);
            }
            break;
        }
        case GLTF_Point:
        {
            PlatTexture* tex = (PlatTexture*)glx_texture[texnum];
            GXTexFilter minFilt;
            if (tex->m_Levels == 1)
                minFilt = GX_NEAR;
            else
                minFilt = GX_NEAR_MIP_NEAR;
            GXInitTexObjFilter(&glx_texobj[texnum], minFilt, GX_NEAR);
            break;
        }
        }

        glx_texdirty |= (1 << texnum);
        texnum++;
    }
}

/**
 * Offset/Address/Size: 0x2B1C | 0x801BC61C | size: 0x2154
 * TODO: 99.51% match - gx_vtxfmt setup and texture-attribute loop still use
 * different temp registers and branch counter shape.
 */
static unsigned long glx_SwitchTexConfig(const glModelPacket* p)
{
    int texnum;
    unsigned long texconfig;
    int bit;
    unsigned long extra;

#define SET_TEV_ORDER(stage, coord, map, chan) \
    GXSetTevOrder((GXTevStageID)(stage), (GXTexCoordID)(coord), (GXTexMapID)(map), (GXChannelID)(chan))
#define SET_TEV_COLOR_IN(stage, a, b, c, d) \
    GXSetTevColorIn((GXTevStageID)(stage), (GXTevColorArg)(a), (GXTevColorArg)(b), (GXTevColorArg)(c), (GXTevColorArg)(d))
#define SET_TEV_ALPHA_IN(stage, a, b, c, d) \
    gxSetTevAlphaIn((int)(stage), (_GXTevAlphaArg)(a), (_GXTevAlphaArg)(b), (_GXTevAlphaArg)(c), (_GXTevAlphaArg)(d))
#define SET_TEV_KCOLOR_SEL(stage, sel) \
    GXSetTevKColorSel((GXTevStageID)(stage), (GXTevKColorSel)(sel))
#define SET_TEV_KALPHA_SEL(stage, sel) \
    GXSetTevKAlphaSel((GXTevStageID)(stage), (GXTevKAlphaSel)(sel))
#define SET_TEX_GEN(stage, type, src, mtx) \
    gxSetTexCoordGen((int)(stage), (_GXTexGenType)(type), (_GXTexGenSrc)(src), (u32)(mtx))

    texconfig = p->state.texconfig;
    glx_texconfig = texconfig;
    extra = 0x40;

    if (texconfig & 0x10)
    {
        if (glx_normals == 0)
        {
            extra = glv_MatrixChanged | 0x40;
            glx_normals = 1;
        }
        glx_allowSpecular = 1;
    }
    else
    {
        glx_allowSpecular = 0;
    }

    {
        int newVtxFmt = (int)gx_vtxfmt + 1;
        gx_texattr[0] = GX_VA_NULL;
        gx_texattr[1] = GX_VA_NULL;
        gx_texattr[2] = GX_VA_NULL;
        gx_texattr[3] = GX_VA_NULL;
        gx_texattr[4] = GX_VA_NULL;
        gx_texattr[5] = GX_VA_NULL;
        if (newVtxFmt >= 1)
            newVtxFmt = 0;
        gx_vtxfmt = (_GXVtxFmt)newVtxFmt;
    }

    glx_NumIndices = 0;

    texnum = 0;
    bit = 0;
    while (true)
    {
        if (bit >= 6)
            break;
        if (texconfig & (1 << bit))
        {
            gx_texattr[bit] = (GXAttr)(texnum + 13);
            texnum++;
        }
        bit++;
    }

    gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
    gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
    gxSetTevColourOp(2, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
    gxSetTevColourOp(3, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);

    gxSetTevAlphaOp(0, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
    gxSetTevAlphaOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
    gxSetTevAlphaOp(2, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
    gxSetTevAlphaOp(3, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);

    SET_TEX_GEN(0, 1, 4, 0x3C);
    SET_TEX_GEN(1, 1, 5, 0x3C);
    SET_TEX_GEN(2, 1, 6, 0x3C);
    SET_TEX_GEN(3, 1, 7, 0x3C);

    glx_RasterizedAlphaStage = -1;
    glx_RasterizedAlphaArg = -1;
    glx_GlossMapStage = -1;
    glx_GlossMapCoord = -1;

    if (glx_program == prog_3d_crowd || glx_program == prog_3d_crowd_lit)
    {
        switch (glx_texconfig)
        {
        case 0x01:
            gxSetNumTexGens(1);
            gxSetNumTevStages(1);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEX_GEN(0, 1, 4, 0x36);
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x21:
            gxSetNumTexGens(2);
            gxSetNumTevStages(2);
            SET_TEV_ORDER(0, 1, 1, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEX_GEN(0, 1, 4, 0x36);
            SET_TEX_GEN(1, 10, 19, 0x3C);
            gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_ALPHA_IN(0, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(1, 7, 6, 4, 7);
            glx_RasterizedAlphaStage = 1;
            glx_RasterizedAlphaArg = 1;
            break;
        }
    }
    else
    {
        switch (glx_texconfig)
        {
        case 0x00:
            gxSetNumTexGens(0);
            gxSetNumTevStages(1);
            SET_TEV_ORDER(0, 0xFF, 0xFF, 4);
            SET_TEV_COLOR_IN(0, 15, 10, 12, 15);
            SET_TEV_ALPHA_IN(0, 7, 5, 6, 7);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x01:
            gxSetNumTexGens(1);
            gxSetNumTevStages(1);
            SET_TEV_ORDER(0, 0, 0, 4);
            if (glx_program == prog_3d_unlit_2x)
            {
                gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)1, true, (_GXTevRegID)1);
            }
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x03:
            gxSetNumTexGens(2);
            gxSetNumTevStages(3);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 0xFF);
            SET_TEV_ORDER(2, 0xFF, 0xFF, 4);
            SET_TEV_KCOLOR_SEL(0, 0x0C);
            SET_TEV_KCOLOR_SEL(1, 0x0C);
            SET_TEV_KALPHA_SEL(0, 0x1C);
            SET_TEV_KALPHA_SEL(1, 0x1C);
            SET_TEV_COLOR_IN(0, 15, 8, 14, 15);
            SET_TEV_COLOR_IN(1, 8, 15, 14, 0);
            SET_TEV_COLOR_IN(2, 15, 0, 10, 15);
            SET_TEV_ALPHA_IN(0, 7, 4, 6, 7);
            SET_TEV_ALPHA_IN(1, 4, 7, 6, 0);
            SET_TEV_ALPHA_IN(2, 7, 0, 5, 7);
            glx_RasterizedAlphaStage = 2;
            glx_RasterizedAlphaArg = 2;
            break;
        case 0x07:
            if (glx_program == prog_2d_movie)
            {
                gxSetNumTexGens(3);
                gxSetNumTevStages(4);
                SET_TEV_ORDER(0, 1, 1, 0xFF);
                SET_TEV_ORDER(1, 1, 2, 0xFF);
                SET_TEV_ORDER(2, 0, 0, 0xFF);
                SET_TEV_ORDER(3, 0xFF, 0xFF, 0xFF);
                SET_TEV_COLOR_IN(0, 15, 8, 14, 2);
                gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, false, (_GXTevRegID)0);
                SET_TEV_ALPHA_IN(0, 7, 4, 6, 1);
                gxSetTevAlphaOp(0, (_GXTevOp)1, (_GXTevBias)0, (_GXTevScale)0, false, (_GXTevRegID)0);
                SET_TEV_KCOLOR_SEL(0, 0x0C);
                SET_TEV_KALPHA_SEL(0, 0x1C);
                SET_TEV_COLOR_IN(1, 15, 8, 14, 0);
                gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)1, false, (_GXTevRegID)0);
                SET_TEV_ALPHA_IN(1, 7, 4, 6, 0);
                gxSetTevAlphaOp(1, (_GXTevOp)1, (_GXTevBias)0, (_GXTevScale)0, false, (_GXTevRegID)0);
                SET_TEV_KCOLOR_SEL(1, 0x0D);
                SET_TEV_KALPHA_SEL(1, 0x1D);
                SET_TEV_COLOR_IN(2, 15, 8, 12, 0);
                gxSetTevColourOp(2, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
                SET_TEV_ALPHA_IN(2, 4, 7, 7, 0);
                gxSetTevAlphaOp(2, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
                SET_TEV_COLOR_IN(3, 1, 0, 14, 15);
                gxSetTevColourOp(3, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
                SET_TEV_ALPHA_IN(3, 7, 7, 7, 7);
                gxSetTevAlphaOp(3, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)0);
                SET_TEV_KCOLOR_SEL(3, 0x0E);
                {
                    GXColorS10 movieColour = { (s16)0xFFA6, 0, (s16)0xFF8E, 0x87 };
                    GXSetTevColorS10((GXTevRegID)1, movieColour);
                }
            }
            else
            {
                gxSetNumTexGens(3);
                gxSetNumTevStages(6);
                SET_TEV_ORDER(0, 0, 0, 4);
                SET_TEV_ORDER(1, 1, 1, 0xFF);
                SET_TEV_ORDER(2, 0xFF, 0xFF, 4);
                SET_TEV_ORDER(3, 2, 2, 0xFF);
                SET_TEV_ORDER(4, 0xFF, 0xFF, 0xFF);
                SET_TEV_KCOLOR_SEL(0, 0x0C);
                SET_TEV_KCOLOR_SEL(1, 0x0C);
                SET_TEV_KCOLOR_SEL(3, 0x0D);
                SET_TEV_KCOLOR_SEL(4, 0x0E);
                SET_TEV_KALPHA_SEL(0, 0x1C);
                SET_TEV_KALPHA_SEL(1, 0x1C);
                SET_TEV_KALPHA_SEL(3, 0x1D);
                SET_TEV_KALPHA_SEL(4, 0x1E);
                gxSetTevColourOp(2, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
                SET_TEV_COLOR_IN(0, 15, 8, 14, 15);
                SET_TEV_COLOR_IN(1, 8, 15, 14, 0);
                SET_TEV_COLOR_IN(2, 15, 0, 10, 15);
                SET_TEV_COLOR_IN(3, 15, 14, 8, 15);
                SET_TEV_COLOR_IN(4, 15, 12, 0, 14);
                SET_TEV_COLOR_IN(5, 15, 2, 0, 15);
                SET_TEV_ALPHA_IN(0, 7, 4, 6, 7);
                SET_TEV_ALPHA_IN(1, 4, 7, 6, 0);
                SET_TEV_ALPHA_IN(2, 7, 0, 5, 7);
                SET_TEV_ALPHA_IN(3, 7, 7, 7, 0);
                SET_TEV_ALPHA_IN(4, 7, 7, 7, 0);
                SET_TEV_ALPHA_IN(5, 7, 7, 7, 0);
                glx_RasterizedAlphaStage = 2;
                glx_RasterizedAlphaArg = 2;
            }
            break;
        case 0x11:
            gxSetNumTexGens(2);
            gxSetNumTevStages(2);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 5);
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 10, 8, 0);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            glx_GlossMapStage = 1;
            glx_GlossMapCoord = 1;
            break;
        case 0x13:
            gxSetNumTexGens(3);
            gxSetNumTevStages(4);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 0xFF);
            SET_TEV_ORDER(2, 0xFF, 0xFF, 4);
            SET_TEV_ORDER(3, 2, 2, 5);
            SET_TEV_KCOLOR_SEL(0, 0x0C);
            SET_TEV_KCOLOR_SEL(1, 0x0C);
            SET_TEV_KALPHA_SEL(0, 0x1C);
            SET_TEV_KALPHA_SEL(1, 0x1C);
            SET_TEV_COLOR_IN(0, 15, 8, 14, 15);
            SET_TEV_COLOR_IN(1, 8, 15, 14, 0);
            SET_TEV_COLOR_IN(2, 15, 0, 10, 15);
            SET_TEV_COLOR_IN(3, 15, 10, 8, 0);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(2, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(3, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            glx_GlossMapStage = 3;
            glx_GlossMapCoord = 2;
            break;
        case 0x05:
            gxSetNumTexGens(2);
            gxSetNumTevStages(2);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 0xFF);
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x15:
            gxSetNumTexGens(3);
            gxSetNumTevStages(3);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 0xFF);
            SET_TEV_ORDER(2, 2, 2, 5);
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(2, 15, 10, 8, 0);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            glx_GlossMapStage = 2;
            glx_GlossMapCoord = 2;
            break;
        case 0x09:
            gxSetNumTexGens(2);
            gxSetNumTevStages(2);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 0xFF);
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 12, 8);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x19:
            gxSetNumTexGens(3);
            gxSetNumTevStages(3);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 0xFF);
            SET_TEV_ORDER(2, 2, 2, 5);
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 12, 8);
            SET_TEV_COLOR_IN(2, 15, 10, 8, 0);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(2, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            glx_GlossMapStage = 2;
            glx_GlossMapCoord = 2;
            break;
        case 0x0D:
            gxSetNumTexGens(3);
            gxSetNumTevStages(3);
            SET_TEV_ORDER(0, 0, 0, 4);
            SET_TEV_ORDER(1, 1, 1, 0xFF);
            SET_TEV_ORDER(2, 2, 2, 0xFF);
            SET_TEV_COLOR_IN(0, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(2, 15, 0, 12, 8);
            SET_TEV_ALPHA_IN(0, 7, 5, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(2, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 0;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x21:
            gxSetNumTexGens(2);
            gxSetNumTevStages(2);
            SET_TEV_ORDER(0, 1, 1, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEX_GEN(1, 10, 19, 0x3C);
            gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_ALPHA_IN(0, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(1, 7, 6, 4, 7);
            glx_RasterizedAlphaStage = 1;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x29:
            gxSetNumTexGens(3);
            gxSetNumTevStages(3);
            SET_TEV_ORDER(0, 2, 2, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEV_ORDER(2, 1, 1, 0xFF);
            SET_TEX_GEN(2, 10, 19, 0x3C);
            gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(2, 15, 12, 8, 0);
            SET_TEV_ALPHA_IN(0, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(1, 7, 6, 4, 7);
            SET_TEV_ALPHA_IN(2, 7, 6, 6, 0);
            glx_RasterizedAlphaStage = 1;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x25:
            gxSetNumTexGens(3);
            gxSetNumTevStages(3);
            SET_TEV_ORDER(0, 2, 2, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEV_ORDER(2, 1, 1, 0xFF);
            SET_TEX_GEN(2, 10, 19, 0x3C);
            gxSetTevColourOp(2, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(2, 15, 0, 8, 15);
            SET_TEV_ALPHA_IN(0, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(1, 7, 6, 4, 7);
            SET_TEV_ALPHA_IN(1, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 1;
            glx_RasterizedAlphaArg = 1;
            break;
        case 0x23:
            if (glx_program == prog_3d_pointlit_dirt)
            {
                gxSetNumTexGens(3);
                gxSetNumTevStages(4);
                SET_TEV_ORDER(0, 2, 2, 4);
                SET_TEV_ORDER(1, 1, 1, 0xFF);
                SET_TEV_ORDER(2, 0, 0, 0xFF);
                SET_TEV_ORDER(3, 0xFF, 0xFF, 0xFF);
                SET_TEX_GEN(2, 10, 19, 0x3C);
                gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
                gxSetTevColourOp(3, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
                SET_TEV_KCOLOR_SEL(1, 0x0C);
                SET_TEV_KCOLOR_SEL(2, 0x0C);
                SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
                SET_TEV_COLOR_IN(1, 15, 12, 8, 14);
                SET_TEV_COLOR_IN(2, 15, 8, 0, 15);
                SET_TEV_COLOR_IN(3, 15, 0, 2, 15);
                SET_TEV_ALPHA_IN(0, 7, 7, 7, 7);
                SET_TEV_ALPHA_IN(1, 7, 7, 7, 7);
                SET_TEV_ALPHA_IN(2, 7, 7, 7, 4);
                SET_TEV_ALPHA_IN(3, 7, 7, 7, 0);
            }
            else
            {
                gxSetNumTexGens(3);
                gxSetNumTevStages(4);
                SET_TEV_ORDER(0, 2, 2, 4);
                SET_TEV_ORDER(1, 0, 0, 0xFF);
                SET_TEV_ORDER(2, 1, 1, 0xFF);
                SET_TEV_ORDER(3, 0xFF, 0xFF, 0xFF);
                SET_TEX_GEN(2, 10, 19, 0x3C);
                gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)1);
                SET_TEV_KCOLOR_SEL(1, 0x0C);
                SET_TEV_KCOLOR_SEL(2, 0x0C);
                SET_TEV_KALPHA_SEL(1, 0x1C);
                SET_TEV_KALPHA_SEL(2, 0x1C);
                SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
                SET_TEV_COLOR_IN(1, 15, 8, 14, 15);
                SET_TEV_COLOR_IN(2, 8, 15, 14, 0);
                SET_TEV_COLOR_IN(3, 15, 0, 2, 15);
                SET_TEV_ALPHA_IN(0, 7, 7, 7, 7);
                SET_TEV_ALPHA_IN(1, 7, 4, 6, 7);
                SET_TEV_ALPHA_IN(2, 4, 7, 6, 0);
                SET_TEV_ALPHA_IN(3, 7, 7, 7, 0);
            }
            break;
        case 0x27:
            gxSetNumTexGens(4);
            gxSetNumTevStages(7);
            SET_TEV_ORDER(0, 3, 3, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEV_ORDER(2, 1, 1, 0xFF);
            SET_TEV_ORDER(3, 0xFF, 0xFF, 0xFF);
            SET_TEV_ORDER(4, 2, 2, 0xFF);
            SET_TEV_ORDER(5, 0xFF, 0xFF, 0xFF);
            SET_TEX_GEN(3, 10, 19, 0x3C);
            gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)1);
            SET_TEV_KCOLOR_SEL(1, 0x0C);
            SET_TEV_KCOLOR_SEL(2, 0x0C);
            SET_TEV_KCOLOR_SEL(4, 0x0D);
            SET_TEV_KCOLOR_SEL(5, 0x0E);
            SET_TEV_KALPHA_SEL(1, 0x1C);
            SET_TEV_KALPHA_SEL(2, 0x1C);
            SET_TEV_KALPHA_SEL(4, 0x1D);
            SET_TEV_KALPHA_SEL(5, 0x1E);
            gxSetTevColourOp(3, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 8, 14, 15);
            SET_TEV_COLOR_IN(2, 8, 15, 14, 0);
            SET_TEV_COLOR_IN(3, 15, 0, 2, 15);
            SET_TEV_COLOR_IN(4, 15, 14, 8, 15);
            SET_TEV_COLOR_IN(5, 15, 12, 0, 14);
            SET_TEV_COLOR_IN(6, 15, 2, 0, 15);
            SET_TEV_ALPHA_IN(0, 7, 7, 7, 7);
            SET_TEV_ALPHA_IN(1, 7, 4, 6, 7);
            SET_TEV_ALPHA_IN(2, 4, 7, 6, 0);
            SET_TEV_ALPHA_IN(3, 7, 0, 5, 7);
            SET_TEV_ALPHA_IN(4, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(5, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(6, 7, 7, 7, 0);
            glx_RasterizedAlphaStage = 3;
            glx_RasterizedAlphaArg = 2;
            break;
        case 0x2B:
            gxSetNumTexGens(4);
            gxSetNumTevStages(5);
            SET_TEV_ORDER(0, 3, 3, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEV_ORDER(2, 1, 1, 0xFF);
            SET_TEV_ORDER(3, 0xFF, 0xFF, 0xFF);
            SET_TEV_ORDER(4, 2, 2, 0xFF);
            SET_TEX_GEN(3, 10, 19, 0x3C);
            gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)1);
            SET_TEV_KCOLOR_SEL(1, 0x0C);
            SET_TEV_KCOLOR_SEL(2, 0x0C);
            SET_TEV_KALPHA_SEL(1, 0x1C);
            SET_TEV_KALPHA_SEL(2, 0x1C);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 8, 14, 15);
            SET_TEV_COLOR_IN(2, 8, 15, 14, 0);
            SET_TEV_COLOR_IN(3, 15, 0, 2, 15);
            SET_TEV_COLOR_IN(4, 15, 0, 12, 8);
            SET_TEV_ALPHA_IN(0, 7, 7, 7, 7);
            SET_TEV_ALPHA_IN(1, 7, 4, 6, 7);
            SET_TEV_ALPHA_IN(2, 4, 7, 6, 0);
            SET_TEV_ALPHA_IN(3, 7, 7, 7, 0);
            SET_TEV_ALPHA_IN(4, 7, 7, 7, 0);
            break;
        case 0x31:
            gxSetNumTexGens(3);
            gxSetNumTevStages(4);
            SET_TEV_ORDER(0, 2, 2, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEV_ORDER(2, 1, 1, 5);
            SET_TEV_ORDER(3, 0xFF, 0xFF, 0xFF);
            SET_TEV_KCOLOR_SEL(3, 0x0F);
            SET_TEV_KALPHA_SEL(3, 0x1F);
            gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            gxSetTevAlphaOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            SET_TEX_GEN(2, 10, 19, 0x3C);
            gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(2, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(3, 15, 14, 0, 2);
            SET_TEV_ALPHA_IN(0, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(1, 7, 6, 4, 7);
            SET_TEV_ALPHA_IN(2, 7, 7, 7, 7);
            SET_TEV_ALPHA_IN(3, 7, 6, 0, 1);
            glx_RasterizedAlphaStage = 1;
            glx_RasterizedAlphaArg = 1;
            glx_GlossMapStage = 2;
            glx_GlossMapCoord = 1;
            break;
        case 0x39:
            gxSetNumTexGens(4);
            gxSetNumTevStages(5);
            SET_TEV_ORDER(0, 3, 3, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEV_ORDER(2, 2, 2, 5);
            SET_TEV_ORDER(3, 1, 1, 0xFF);
            SET_TEV_ORDER(4, 0xFF, 0xFF, 0xFF);
            SET_TEV_KCOLOR_SEL(4, 0x0F);
            SET_TEV_KALPHA_SEL(4, 0x1F);
            gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            gxSetTevAlphaOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            SET_TEX_GEN(3, 10, 19, 0x3C);
            gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(2, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(3, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(4, 15, 14, 0, 2);
            SET_TEV_ALPHA_IN(0, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(1, 7, 6, 4, 7);
            SET_TEV_ALPHA_IN(2, 7, 7, 7, 7);
            SET_TEV_ALPHA_IN(3, 7, 7, 7, 7);
            SET_TEV_ALPHA_IN(4, 7, 6, 0, 1);
            glx_GlossMapStage = 2;
            glx_GlossMapCoord = 2;
            break;
        case 0x35:
            gxSetNumTexGens(4);
            gxSetNumTevStages(5);
            SET_TEV_ORDER(0, 3, 3, 4);
            SET_TEV_ORDER(1, 0, 0, 0xFF);
            SET_TEV_ORDER(2, 1, 1, 0xFF);
            SET_TEV_ORDER(3, 2, 2, 5);
            SET_TEV_ORDER(4, 0xFF, 0xFF, 0xFF);
            SET_TEV_KCOLOR_SEL(4, 0x0F);
            SET_TEV_KALPHA_SEL(4, 0x1F);
            gxSetTevColourOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            gxSetTevAlphaOp(1, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            gxSetTevColourOp(2, (_GXTevOp)0, (_GXTevBias)0, (_GXTevScale)0, true, (_GXTevRegID)1);
            SET_TEX_GEN(3, 10, 19, 0x3C);
            gxSetTevColourOp(0, (_GXTevOp)0, (_GXTevBias)0, glx_tevscale, true, (_GXTevRegID)0);
            SET_TEV_COLOR_IN(0, 15, 12, 8, 15);
            SET_TEV_COLOR_IN(1, 15, 0, 8, 15);
            SET_TEV_COLOR_IN(2, 15, 2, 8, 15);
            SET_TEV_COLOR_IN(3, 15, 10, 8, 15);
            SET_TEV_COLOR_IN(4, 15, 14, 0, 2);
            SET_TEV_ALPHA_IN(0, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(1, 7, 6, 4, 7);
            SET_TEV_ALPHA_IN(2, 7, 6, 6, 7);
            SET_TEV_ALPHA_IN(3, 7, 7, 7, 7);
            SET_TEV_ALPHA_IN(4, 7, 6, 0, 1);
            glx_GlossMapStage = 3;
            glx_GlossMapCoord = 2;
            break;
        case 0x02:
        case 0x04:
        case 0x06:
        case 0x08:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x12:
        case 0x14:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x22:
        case 0x24:
        case 0x26:
        case 0x28:
        case 0x2A:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
        case 0x30:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x36:
        case 0x37:
        case 0x38:
            break;
        default:
            break;
        }
    }

#undef SET_TEV_ORDER
#undef SET_TEV_COLOR_IN
#undef SET_TEV_ALPHA_IN
#undef SET_TEV_KCOLOR_SEL
#undef SET_TEV_KALPHA_SEL
#undef SET_TEX_GEN

    return extra;
}

/**
 * Offset/Address/Size: 0x4C70 | 0x801BE770 | size: 0x24
 */
void glx_SendEnd()
{
    glx_SwitchUserData(nullptr);
}

static inline void glx_SetVtxAttr()
{
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);

    for (s32 j = 0; j < 6; j++)
    {
        GXSetVtxAttrFmt(GX_VTXFMT0, (GXAttr)(GX_VA_TEX0 + j), GX_TEX_ST, GX_S16, 10);
    }
}

/**
 * Offset/Address/Size: 0x4C94 | 0x801BE794 | size: 0x200
 */
void glx_SendReset()
{
    prev_view = GLV_Num;
    glx_SetVtxAttr();

    glx_texdirty = 0;

    for (s32 i = 0; i < 6; i++)
    {
        memset(&glx_texobj[i], 0, sizeof(GXTexObj));
        memset(&glx_tlutobj[i], 0, sizeof(GXTlutObj));
        glx_texture[i] = 0;
    }

    gx_vtxfmt = 0;

    GetConstants();

    {
        nlColour amb;
        nlColour temp = { };
        nlColourSet(temp, world_ambient.c[0], world_ambient.c[1], world_ambient.c[2], world_ambient.c[3]);
        amb = temp;
        gxSetChanAmbColour(0, amb);
    }
    gxSetChanMatColour(0, nlWhite);
    gxSetChanAmbColour(1, nlBlack);
    gxSetChanMatColour(1, nlWhite);

    GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_RED);

    if (glx_prevSpecMask != 0)
    {
        gxSetNumChans(1);
        GXSetChanCtrl(GX_COLOR1, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, glx_prevSpecMask, GX_DF_NONE, GX_AF_NONE);
    }

    glx_DirtyFlags = 0;
    glx_ReloadPointLights = true;
    glx_ReloadSpecLights = true;
    glx_prevLightMask = 0;
    glx_prevSpecMask = 0;
    glx_allowSpecular = 0;
    glx_envdiffuse = false;
    glx_mobilediffuse = false;
    glx_constantcolour = false;
    glx_viewport = false;
    glx_CoPlanar = false;

    gxSetCoPlanar(false);

    glx_translucent = false;
    glx_norasterizedalpha = false;
    glx_RasterizedAlphaStage = -1;
    glx_RasterizedAlphaArg = -1;
    glx_GlossMapStage = -1;
    glx_GlossMapCoord = -1;
    glx_NoFog = false;
}

static inline GXColor makeColor(f32 r, f32 g, f32 b, f32 a)
{
    GXColor c;
    c.r = (u8)(r * 255.0f);
    c.g = (u8)(g * 255.0f);
    c.b = (u8)(b * 255.0f);
    c.a = (u8)(a * 255.0f);
    return c;
}

/**
 * Offset/Address/Size: 0x4E94 | 0x801BE994 | size: 0x310
 */
static void GetConstants()
{
    nlVector4 vMult;
    nlVector4 vTexel;
    Mtx crowdMatrix;
    GXColor shadow0;
    GXColor shadow1;
    GXColor ambient;

    {
        const nlVector4& v = glConstantGet("shadow/pass0_colour");
        shadow0 = makeColor(v.f.x, v.f.y, v.f.z, v.f.w);
        rshadow_colour[0] = shadow0;
    }

    {
        const nlVector4& v = glConstantGet("shadow/pass1_colour");
        shadow1 = makeColor(v.f.x, v.f.y, v.f.z, v.f.w);
        rshadow_colour[1] = shadow1;
    }

    {
        const nlVector4& v = glConstantGet("lighting/ambient_colour");
        ambient = makeColor(v.f.x, v.f.y, v.f.z, v.f.w);
        world_ambient.c[0] = ambient.r;
        world_ambient.c[1] = ambient.g;
        world_ambient.c[2] = ambient.b;
        world_ambient.c[3] = ambient.a;
    }

    glConstantGet("water/scale", water_Scale);
    glConstantGet("water/trans", water_Trans);
    glConstantGet("water/follow", water_Follow);

    {
        const nlVector4& warbleDivisor = glConstantGet("warble/divisor");
        glx_IndDivisor = warbleDivisor.f.x;
    }

    vMult = glConstantGet("lighting/range");
    if (vMult.f.x == 0.0f)
    {
        glx_tevscale = (_GXTevScale)0;
    }
    else
    {
        glx_tevscale = (_GXTevScale)1;
    }

    if (glConstantGet("texture/density", vTexel))
    {
        glx_SetGridMode(vTexel.f.x == 0.0f);
    }

    {
        const nlVector4& crowdFrame = glConstantGet("crowd/frame");
        f32 crowdOffsetV = crowdFrame.f.x;
        PSMTXIdentity(crowdMatrix);
        crowdMatrix[1][3] = crowdOffsetV;
        GXLoadTexMtxImm(crowdMatrix, 0x36, (_GXTexMtxType)1);
    }
}

/**
 * Offset/Address/Size: 0x51A4 | 0x801BECA4 | size: 0xAC
//  */
// void 0x8028D51C..0x8028D520 | size: 0x4
// {
// }
