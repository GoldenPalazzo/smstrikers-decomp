#include "NL/glx/glxSend.h"

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
#include "NL/gl/glState.h"
#include "NL/gl/glUserData.h"
#include "NL/glx/glxGX.h"
#include "NL/glx/glxMatrix.h"
#include "NL/glx/glxTexture.h"
#include "dolphin/gx/GXTexture.h"
#include "NL/nlColour.h"
#include "NL/platvmath.h"
#include "types.h"
#include <string.h>

// Forward declarations for global variables (these would need to be declared elsewhere)
extern bool glx_ReloadPointLights;
extern bool g_bAllowLighting;
extern u32 glx_prevLightMask;
extern u32 glx_NumIndices;

static u32 gxLights[4] = {
    0x00000001,
    0x00000002,
    0x00000004,
    0x00000008
};

// static bool glx_allowSpecular;            // size: 0x1, address: 0x80397FC8
// static bool glx_ReloadPointLights; // size: 0x1, address: 0x80397FC9
// static u32 glx_prevLightMask;      // size: 0x4, address: 0x80397FCC
// static u32 glx_prevSpecMask;              // size: 0x4, address: 0x80397FD0
// static struct _GXColor rshadow_colour[2]; // size: 0x8, address: 0x80397FD4

static GXTexObj glx_texobj[6];
static GXTlutObj glx_tlutobj[6];
static u32 glx_texture[6];
static u32 glx_texdirty;
static u32 gx_vtxfmt;
static GXColor rshadow_colour[2];
static nlColour world_ambient;
static nlVector4 water_Scale;
static nlVector4 water_Trans;
static nlVector4 water_Follow;
static f32 glx_IndDivisor;
static _GXTevScale glx_tevscale;
static nlColour nlBlack;
static nlColour nlWhite;
static u32 glx_prevSpecMask;
static u32 glx_DirtyFlags;
static bool glx_ReloadSpecLights;
static bool glx_allowSpecular;
static bool glx_envdiffuse;
static bool glx_mobilediffuse;
static bool glx_constantcolour;
static bool glx_viewport;
static bool glx_CoPlanar;
static u32 glx_program;
static u32 prog_2d_movie;
static f32 glx_konstlevel[4];
static int glx_aniso;
static u32 glx_texconfig;
static bool glx_translucent;
static bool glx_norasterizedalpha;
static s32 glx_RasterizedAlphaStage;
static s32 glx_RasterizedAlphaArg;
static s32 glx_GlossMapStage;
static s32 glx_GlossMapCoord;
static bool glx_NoFog;
// static unsigned long glx_NumIndices;
/**
 * Offset/Address/Size: 0x0 | 0x801B9B00 | size: 0x538
 */
void glx_SendFrame_cb(eGLView, unsigned long, const glModelPacket*)
{
}

/**
 * Offset/Address/Size: 0x538 | 0x801BA038 | size: 0xB20
 */
void glx_DrawPacket(const glModelPacket*)
{
}

/**
 * Offset/Address/Size: 0x1058 | 0x801BAB58 | size: 0x964
 */
void glx_SwitchUserData(const glModelPacket*)
{
    FORCE_DONT_INLINE;
}

static eGLView prev_view;
static u8 glx_InvXpose;
static u8 glx_InvXposeChar;
static bool g_bFastSkinPath;
static bool g_bMtxSkinMath;
static bool glx_IsCoPlanarView;
static Mtx gx_mview;
static nlMatrix4 mview;

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
 * TODO: 99.13% match - block 1 FPR allocation (y,x,z loads vs target z,y,x),
 *       block 2 store order swap at 120/124, GXColor stack offset 0xC vs 0x08
 */
void glud_Specular(void* pData)
{
    static float SpecularFudge;
    static signed char init;

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
                    float worldZ = pLight->worldDirection.f.z;
                    float worldY = pLight->worldDirection.f.y;
                    float worldX = pLight->worldDirection.f.x;

                    worldDir.f.z = recipLength * worldZ;
                    worldDir.f.x = recipLength * worldX;
                    worldDir.f.y = recipLength * worldY;
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
                    float halfExponent = pLight->exponent;
                    halfExponent *= 0.5f;
                    GXInitLightAttn(&light, 0.0f, 0.0f, 1.0f, halfExponent, 0.0f, 1.0f - halfExponent);
                }

                GXLoadLightObjImm(&light, lightID);
            }

            pLight++;
            index += 1;
        }

        glx_ReloadSpecLights = 0;
        if (glx_allowSpecular)
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
void glx_LoadLight(GLLightUserData* pLight, _GXLightID lightId)
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
            float recipLength = nlRecipSqrt(worldDir.f.x * worldDir.f.x + worldDir.f.y * worldDir.f.y + worldDir.f.z * worldDir.f.z, true);

            worldDir.f.z = recipLength * worldDir.f.z;
            worldDir.f.y = recipLength * worldDir.f.y;
            worldDir.f.x = recipLength * worldDir.f.x;
        }

        nlMultDirVectorMatrix(viewDir, worldDir, mview);

        viewDir.f.z = -1.0f * viewDir.f.z;
        viewDir.f.y = -1.0f * viewDir.f.y;
        viewDir.f.x = -1.0f * viewDir.f.x;

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

/**
 * Offset/Address/Size: 0x2690 | 0x801BC190 | size: 0x48C
 * TODO: 95.8% match - stack offset diffs for GXColor locals (MWCC allocates
 * 2d_movie GXColors at lower offsets than konst GXColor work area, target has
 * them reversed). All instructions and registers correct.
 */
void glx_SwitchTextureState(const glModelPacket* p)
{
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
        return;
    }

    u8 raw;
    raw = (u8)glGetTextureState(GLTS_DiffuseLevel);
    f32 level = (f32)raw * (1.0f / 63.0f);
    if (level != glx_konstlevel[0])
    {
        int val = (int)(255.5f * level);
        GXColor c = { (u8)val, (u8)val, (u8)val, (u8)val };
        GXSetTevKColor(GX_KCOLOR0, c);
        glx_konstlevel[0] = level;
    }

    raw = (u8)glGetTextureState(GLTS_ShadowLevel);
    level = (f32)raw * (1.0f / 63.0f);
    if (level != glx_konstlevel[1])
    {
        int val = (int)(255.5f * level);
        GXColor c = { (u8)val, (u8)val, (u8)val, (u8)val };
        GXSetTevKColor(GX_KCOLOR1, c);
        glx_konstlevel[1] = level;
    }

    raw = (u8)glGetTextureState(GLTS_ShadowLevel);
    level = 1.0f - (f32)raw * (1.0f / 63.0f);
    if (level != glx_konstlevel[2])
    {
        int val = (int)(255.5f * level);
        GXColor c = { (u8)val, (u8)val, (u8)val, (u8)val };
        GXSetTevKColor(GX_KCOLOR2, c);
        glx_konstlevel[2] = level;
    }

    raw = (u8)glGetTextureState(GLTS_GlossLevel);
    level = (f32)raw * (1.0f / 63.0f);
    if (level != glx_konstlevel[3])
    {
        int val = (int)(255.5f * level);
        GXColor c = { (u8)val, (u8)val, (u8)val, (u8)val };
        GXSetTevKColor(GX_KCOLOR3, c);
        glx_konstlevel[3] = level;
    }

    int texnum = 0;
    for (int bit = 0; bit < 6; bit++)
    {
        if (!(glx_texconfig & (1 << bit)))
            continue;

        GXTexWrapMode mode[2];
        if (bit == 5)
        {
            mode[1] = GX_CLAMP;
            mode[0] = GX_CLAMP;
        }
        else
        {
            eGLTextureMode tmode = (eGLTextureMode)glGetTextureState((eGLTextureState)bit);
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

        eGLTextureFilter filter = (eGLTextureFilter)glGetTextureState((eGLTextureState)(bit + 6));
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
            GXTexFilter minFilt = GX_NEAR_MIP_NEAR;
            if (tex->m_Levels == 1)
                minFilt = GX_NEAR;
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
 */
void glx_SwitchTexConfig(const glModelPacket*)
{
}

/**
 * Offset/Address/Size: 0x4C70 | 0x801BE770 | size: 0x24
 */
void glx_SendEnd()
{
    glx_SwitchUserData(nullptr);
}

/**
 * Offset/Address/Size: 0x4C94 | 0x801BE794 | size: 0x200
 * TODO: 99.8% match - 4 register diffs (first loop counter r28 vs r31), 1 label diff (@142 vs @419)
 */
void glx_SendReset()
{
    prev_view = GLV_Num;

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);

    for (s32 i = 0; i < 6; i++)
    {
        GXSetVtxAttrFmt(GX_VTXFMT0, (GXAttr)(GX_VA_TEX0 + i), GX_TEX_ST, GX_S16, 10);
    }

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
 * TODO: 99.18% match - 30 stack offset diffs from compiler GXColor temp ordering
 *       (decomp.me interleaves col/tmp per scope, target groups all cols then all tmps)
 */
void GetConstants()
{
    nlVector4 vMult;
    nlVector4 vTexel;
    Mtx crowdMatrix;

    {
        const nlVector4& v = glConstantGet("shadow/pass0_colour");
        rshadow_colour[0] = makeColor(v.f.x, v.f.y, v.f.z, v.f.w);
    }

    {
        const nlVector4& v = glConstantGet("shadow/pass1_colour");
        rshadow_colour[1] = makeColor(v.f.x, v.f.y, v.f.z, v.f.w);
    }

    {
        const nlVector4& v = glConstantGet("lighting/ambient_colour");
        GXColor tmp = makeColor(v.f.x, v.f.y, v.f.z, v.f.w);
        world_ambient.c[0] = tmp.r;
        world_ambient.c[1] = tmp.g;
        world_ambient.c[2] = tmp.b;
        world_ambient.c[3] = tmp.a;
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
