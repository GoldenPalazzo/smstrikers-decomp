#include "Game/Render/RenderShadow.h"

#include "Game/Debug/ShapeRender.h"
#include "Game/Drawable/DrawableModel.h"
#include "Game/GL/GLInventory.h"
#include "Game/WorldManager.h"
#include "NL/gl/glAppAttach.h"
#include "NL/gl/glDraw3.h"
#include "NL/nlString.h"

const unsigned long UnlitProgram = glGetProgram("3d unlit");
const unsigned long LitProgram = glGetProgram("3d pointlit");
const unsigned long LightTexture = glGetTexture("global/lightramp");
const unsigned long BlackTexture = glGetTexture("global/black");
const unsigned long WhiteTexture = glGetTexture("global/white");
int MaxProjectedShadows;

extern GLInventory glInventory;
extern unsigned long ResolvedWhiteTexture;

static u8 g_bPreview;
static u8 g_bShadowBlobs;
static u8 g_bSubdivideShadow;
static u8 g_bShadowBounds;
static u8 g_bShadowPositionOverride;
static u8 g_bCoPlanarProjectedShadows;

const unsigned long FourTexture = glGetTexture("global/four");

static glModel* pCylinder;
static glModel* pBox;

static bool g_bProjectDirectional = true;
static bool g_bShadowDirectional = true;
static int g_AlphaRef = 0x00000020;
static float g_fProjectionAdjust = 0.125f;
static eGLView g_CharacterShadowView = GLV_WorldShadowed;
static float g_AntiFlimmer = 0.015625f;

static nlVector3 g_vShadowPosition = { 0.0f, 17.0f, 13.0f };
static int g_Alpha[3] = { 180, 80, 32 };

/**
 * Offset/Address/Size: 0x16B4 | 0x801246E8 | size: 0x80
 */
void ShadowStartup()
{
    pCylinder = glInventory.GetModel(nlStringHash("debug/cylinder"));
    pBox = glInventory.GetModel(nlStringHash("debug/box"));
    const nlVector4& result = glConstantGet("target/pshadow_num");
    MaxProjectedShadows = (int)result.f.x;
}

/**
 * Offset/Address/Size: 0x1558 | 0x8012458C | size: 0x15C
 * TODO: 97.13% match - preview loop pkt register r28 vs target r26,
 *       and MWCC schedules li r3 after conditional li r4 for first glSetRasterState(GLS_Culling, v).
 */
void RenderShadowModel(unsigned long flags, glModel* model, unsigned long matrix)
{
    unsigned long fourTex;
    eGLView view = GLV_Shadow1;
    if (flags & 1)
        view = GLV_Shadow0;

    if (matrix == 0xFFFFFFFF)
        matrix = glGetIdentityMatrix();

    if (g_bPreview)
    {
        glModelPacket* pkt = model->packets;
        while (pkt < model->packets + model->numPackets)
        {
            glModelPacket* dup = glModelPacketDup(pkt, true);
            dup->state.texture[0] = ResolvedWhiteTexture;
            dup->state.matrix = matrix;
            glViewAttachPacket(GLV_Unshadowed, dup);
            pkt++;
        }
    }
    else
    {
        glModelPacket* pkt;
        glModelPacket* dup;
        s32 pass = 0;
        fourTex = FourTexture;
        do
        {
            pkt = model->packets;
            while (pkt < model->packets + model->numPackets)
            {
                dup = glModelPacketDup(pkt, true);
                dup->state.texture[0] = fourTex;
                dup->state.matrix = matrix;
                glUnHandleizeRasterState(dup->state.raster);

                u32 v = 1;
                if (pass == 0)
                    v = 2;
                glSetRasterState(GLS_Culling, v);
                glSetRasterState(GLS_DepthWrite, 0);

                glSetRasterState(GLS_AlphaBlend, (pass == 0) ? 2 : 7);
                glSetRasterState(GLS_ColourWrite, 2);

                dup->state.raster = glHandleizeRasterState();
                glViewAttachPacket(view, dup);
                pkt++;
            }
            pass++;
        } while (pass < 2);
    }
}

/**
 * Offset/Address/Size: 0x14DC | 0x80124510 | size: 0x7C
 */
void GetShadowPartitionIndex()
{
    static s32 index = 0;
    static s32 prevFrame = 0;

    s32 currentFrame = glGetCurrentFrame();
    if ((u32)prevFrame != (u32)currentFrame)
    {
        prevFrame = currentFrame;
        index = 0;
    }

    index++;
}

/**
 * Offset/Address/Size: 0x140C | 0x80124440 | size: 0xD0
 */
u8 ShouldShadowBeUpdated(const ProjectedShadowParams& params)
{
    float fRadius = 2.0f * params.fRadius;
    nlMatrix4 mWorld;
    mWorld.SetIdentity();
    mWorld.f.m41 = params.vPosition.f.x;
    mWorld.f.m42 = params.vPosition.f.y;
    mWorld.f.m43 = params.vPosition.f.z;
    mWorld.f.m44 = 1.0f;
    mWorld.f.m43 += 0.5f * params.fHeight;

    u8 isVisible = WorldManager::s_World->IsSphereInFrustum(mWorld, fRadius);
    u32 interval;
    if (isVisible)
    {
        interval = params.nVisibleInterval;
    }
    else
    {
        interval = params.nInvisibleInterval;
    }

    u32 currentFrame = (u32)glGetCurrentFrame();
    u32 frame = (u32)params.nPartitionIndex + currentFrame;
    if (frame % interval != 0)
    {
        return 0;
    }
    return 1;
}

/**
 * Offset/Address/Size: 0xF9C | 0x80123FD0 | size: 0x470
 * TODO: 99.00% match - remaining diffs are local/static symbol labels
 *       (e.g. @343/init$345 numbering) and associated address-label references.
 */
void RenderCharacterIntoTexture(const ProjectedShadowParams& params)
{
    extern u8 g_bShadowBlobs;
    extern u8 g_bShadowPositionOverride;
    extern nlVector3 g_vShadowPosition;
    extern unsigned long ResolvedBlackTexture;

    struct ShadowTextureUserData
    {
        s16 x;
        s16 y;
        s16 width;
        s16 height;
        u32 viewMatrix;
        u32 projectionMatrix;
    };

    nlVector3 targetPos;
    nlVector3 eyePos;
    nlVector3 shadowPos;
    nlVector3 up = { 0.0f, 0.0f, 1.0f };

    static float s_fFarPlane = 8.0f;
    static float s_fLightDist = -0.125f;

    if (g_bShadowBlobs)
    {
        return;
    }

    float radius = params.fRadius;
    if (g_bShadowPositionOverride)
    {
        shadowPos = g_vShadowPosition;
    }
    else
    {
        shadowPos.f.x = params.vLight.f.x;
        shadowPos.f.y = params.vLight.f.y;
        shadowPos.f.z = params.vLight.f.z;
    }

    targetPos = params.vPosition;

    if (g_bProjectDirectional)
    {
        float lx = -shadowPos.f.x;
        float ly = -shadowPos.f.y;
        float lz = -shadowPos.f.z;
        float lenSq = lx * lx + ly * ly + lz * lz;
        float len = nlSqrt(lenSq, true);
        float invLen = nlRecipSqrt(lenSq, false);

        float scale = len * (s_fLightDist * radius);
        eyePos.f.x = targetPos.f.x + scale * (invLen * lx);
        eyePos.f.y = targetPos.f.y + scale * (invLen * ly);
        eyePos.f.z = targetPos.f.z + scale * (invLen * lz);
    }
    else
    {
        float scale = s_fLightDist * radius;
        float tx = targetPos.f.x;
        float ty = targetPos.f.y;
        float tz = targetPos.f.z;
        float dx = tx - shadowPos.f.x;
        float dy = ty - shadowPos.f.y;
        float dz = tz - shadowPos.f.z;

        eyePos.f.x = dx;
        eyePos.f.y = dy;
        eyePos.f.z = dz;

        eyePos.f.x = tx + scale * eyePos.f.x;
        eyePos.f.y = ty + scale * eyePos.f.y;
        eyePos.f.z = tz + scale * eyePos.f.z;
    }

    float dx = targetPos.f.x - eyePos.f.x;
    float dy = targetPos.f.y - eyePos.f.y;
    float dz = targetPos.f.z - eyePos.f.z;
    float eyeDistance = nlSqrt(dx * dx + dy * dy + dz * dz, true);

    float ratio = radius / eyeDistance;
    float nearPlane = eyeDistance - radius;
    float farPlane = nearPlane + s_fFarPlane;

    float fovY;
    if ((float)__fabs(ratio) < 0.01f)
    {
        fovY = 1.0f;
    }
    else
    {
        u16 angle = (u16)(0x4000 - nlACos(ratio));
        fovY = 2.0f * (9.58738e-5f * (float)angle);
    }

    nlMatrix4 view;
    nlMatrix4 projection;

    glMatrixPerspective(projection, fovY, 1.0f, nearPlane, farPlane);
    glMatrixLookAt(view, eyePos, targetPos, up);

    u32 viewMatrix = glAllocMatrix();
    if (viewMatrix != 0xFFFFFFFF)
    {
        glSetMatrix(viewMatrix, view);
    }

    u32 projectionMatrix = glAllocMatrix();
    if (projectionMatrix != 0xFFFFFFFF)
    {
        glSetMatrix(projectionMatrix, projection);
    }

    void* userData = glUserAlloc(GLUD_Viewport, sizeof(ShadowTextureUserData), false);
    ShadowTextureUserData* pViewportData = (ShadowTextureUserData*)glUserGetData(userData);
    pViewportData->viewMatrix = viewMatrix;
    pViewportData->projectionMatrix = projectionMatrix;
    pViewportData->x = (s16)((params.nPartitionIndex % 4) * 0xA0);
    pViewportData->y = (s16)((params.nPartitionIndex / 4) * 0x94);
    pViewportData->width = 0xA0;
    pViewportData->height = 0x94;

    glModel* pModel = glModelDup(params.pModel, true);
    glModelPacket* pPacket = pModel->packets;

    while (pPacket < pModel->packets + pModel->numPackets)
    {
        if (glUserHasType(GLUD_Light, pPacket))
        {
            glUserDetach(GLUD_Light, pPacket);
        }

        if (userData)
        {
            glUserAttach(userData, pPacket, false);
        }

        pPacket->state.texture[0] = ResolvedBlackTexture;
        glSetTextureState(pPacket->state.texturestate, (eGLTextureState)0xC, 0x3F);

        pPacket++;
    }

    glViewAttachModel(GLV_ShadowTexture, pModel);

    char buffer[32];
    nlSNPrintf(buffer, 32, "target/pshadow_updated%02d", params.nPartitionIndex);

    nlVector4 v;
    v.f.x = 1.0f;
    v.f.y = 0.0f;
    v.f.z = 0.0f;
    v.f.w = 0.0f;
    glConstantSet(buffer, v);
}

/**
 * Offset/Address/Size: 0xF24 | 0x80123F58 | size: 0x78
 */
void SetCharacterShadowUpdated(int index, bool updated)
{
    char buffer[32];
    nlSNPrintf(buffer, 32, "target/pshadow_updated%02d", index);

    nlVector4 v;
    v.f.x = updated ? 1.0f : 0.0f;
    v.f.y = 0.0f;
    v.f.z = 0.0f;
    v.f.w = 0.0f;

    glConstantSet(buffer, v);
}

/**
 * Offset/Address/Size: 0xB44 | 0x80123B78 | size: 0x3E0
 * TODO: 99.82% match - r30 reuse for pPacket in second coplanar block (register allocation difference)
 */
void SubdivideAndRender(glQuad3& quad, eGLView view)
{
    nlVector3 p0;
    nlVector3 p1;
    nlVector2 uv0;
    nlVector2 uv1;
    nlColour c;
    glQuad3 q;
    glModelPacket* pPacket;

    p0.f.x = 0.5f * quad.m_pos[0].f.x + 0.5f * quad.m_pos[3].f.x;
    p0.f.y = 0.5f * quad.m_pos[0].f.y + 0.5f * quad.m_pos[3].f.y;
    p0.f.z = 0.5f * quad.m_pos[0].f.z + 0.5f * quad.m_pos[3].f.z;

    p1.f.x = 0.5f * quad.m_pos[1].f.x + 0.5f * quad.m_pos[2].f.x;
    p1.f.y = 0.5f * quad.m_pos[1].f.y + 0.5f * quad.m_pos[2].f.y;
    p1.f.z = 0.5f * quad.m_pos[1].f.z + 0.5f * quad.m_pos[2].f.z;

    uv0.f.x = 0.5f * quad.m_uv[0].f.x + 0.5f * quad.m_uv[3].f.x;
    uv0.f.y = 0.5f * quad.m_uv[0].f.y + 0.5f * quad.m_uv[3].f.y;

    uv1.f.x = 0.5f * quad.m_uv[1].f.x + 0.5f * quad.m_uv[2].f.x;
    uv1.f.y = 0.5f * quad.m_uv[1].f.y + 0.5f * quad.m_uv[2].f.y;

    c = quad.m_colour[0];
    c.c[3] = (unsigned char)g_Alpha[1];

    // First sub-quad (left half)
    q.m_pos[0] = quad.m_pos[0];
    q.m_pos[1] = quad.m_pos[1];
    q.m_pos[2] = p1;
    q.m_pos[3] = p0;
    q.m_uv[0] = quad.m_uv[0];
    q.m_uv[1] = quad.m_uv[1];
    q.m_uv[2] = uv1;
    q.m_uv[3] = uv0;
    q.m_colour[1] = quad.m_colour[0];
    q.m_colour[0] = quad.m_colour[0];
    q.m_colour[3] = c;
    q.m_colour[2] = c;

    if (g_bCoPlanarProjectedShadows)
    {
        const glModel* pModel = q.GetModel(true);
        pPacket = pModel->packets;
        glSetRasterState(pPacket->state.raster, GLS_AlphaTest, 1);
        glSetRasterState(pPacket->state.raster, GLS_AlphaTestRef, g_AlphaRef);
        glSetRasterState(pPacket->state.raster, GLS_DepthFunc, 3);
        glSetRasterState(pPacket->state.raster, GLS_DepthTest, 1);
        glSetRasterState(pPacket->state.raster, GLS_DepthWrite, 1);
        glUserAttach(glAppGetCoPlanarUserData(), pModel->packets, false);
        glViewAttachModel(GLV_CoPlanar0, 1, pModel);
    }
    else
    {
        q.Attach(view, 0, true);
    }

    // Second sub-quad (right half)
    q.m_pos[0] = p0;
    q.m_pos[1] = p1;
    q.m_pos[2] = quad.m_pos[2];
    q.m_pos[3] = quad.m_pos[3];
    q.m_uv[0] = uv0;
    q.m_uv[1] = uv1;
    q.m_uv[2] = quad.m_uv[2];
    q.m_uv[3] = quad.m_uv[3];
    q.m_colour[1] = c;
    q.m_colour[0] = c;
    q.m_colour[3] = quad.m_colour[2];
    q.m_colour[2] = quad.m_colour[2];

    if (g_bCoPlanarProjectedShadows)
    {
        const glModel* pModel = q.GetModel(true);
        pPacket = pModel->packets;
        glSetRasterState(pPacket->state.raster, GLS_AlphaTest, 1);
        glSetRasterState(pPacket->state.raster, GLS_AlphaTestRef, g_AlphaRef);
        glSetRasterState(pPacket->state.raster, GLS_DepthFunc, 3);
        glSetRasterState(pPacket->state.raster, GLS_DepthTest, 1);
        glSetRasterState(pPacket->state.raster, GLS_DepthWrite, 1);
        glUserAttach(glAppGetCoPlanarUserData(), pModel->packets, false);
        glViewAttachModel(GLV_CoPlanar0, 1, pModel);
    }
    else
    {
        q.Attach(view, 0, true);
    }
}

/**
 * Offset/Address/Size: 0x750 | 0x80123784 | size: 0x3F4
 */
void RenderBlobShadow(const nlVector3& vPosition, const nlVector3* pPoints, int index, const int* uvOrder, const nlColour* pColour)
{
    static float half_w = 0.625f;
    static float half_h = 0.625f;
    static int alpha = 0x80;

    glQuad3 quad;
    nlColour c;
    nlColour cfade;
    unsigned long texture;

    float posX, posY, antiFlimmer;
    antiFlimmer = g_AntiFlimmer;
    posY = vPosition.f.y;
    posX = vPosition.f.x;

    if (pColour == NULL)
    {
        nlColourSet(c, 0xFF, 0xFF, 0xFF, (u8)alpha);
        cfade = c;
    }
    else
    {
        cfade = *pColour;
        c = *pColour;
        cfade.c[3] = (u8)g_Alpha[2];
    }

    if (pPoints == NULL)
    {
        texture = glGetTexture("global/shadeblob");
        float hw = half_w;
        float hh = half_h;

        quad.m_pos[0].f.x = posX - hw;
        quad.m_pos[0].f.y = posY - hh;
        quad.m_pos[0].f.z = antiFlimmer;
        quad.m_pos[1].f.x = posX - hw;
        quad.m_pos[1].f.y = posY + hh;
        quad.m_pos[1].f.z = antiFlimmer;
        quad.m_pos[2].f.x = posX + hw;
        quad.m_pos[2].f.y = posY + hh;
        quad.m_pos[2].f.z = antiFlimmer;
        quad.m_pos[3].f.x = posX + hw;
        quad.m_pos[3].f.y = posY - hh;
        quad.m_pos[3].f.z = antiFlimmer;

        quad.m_uv[0].f.x = 1.0f;
        quad.m_uv[0].f.y = 1.0f;
        quad.m_uv[1].f.x = 0.0f;
        quad.m_uv[1].f.y = 1.0f;
        quad.m_uv[2].f.x = 0.0f;
        quad.m_uv[2].f.y = 0.0f;
        quad.m_uv[3].f.x = 1.0f;
        quad.m_uv[3].f.y = 0.0f;
    }
    else
    {
        char texturename[32] = "target/pshadow00";
        texturename[14] = '0' + (index / 10);
        texturename[15] = '0' + (index % 10);
        texture = glGetTexture(texturename);

        quad.m_pos[0] = pPoints[0];
        quad.m_pos[1] = pPoints[1];
        quad.m_pos[2] = pPoints[2];
        quad.m_pos[3] = pPoints[3];

        int idx;
        nlVector2* pUV;

        if (uvOrder == NULL)
        {
            idx = 0;
        }
        else
        {
            idx = uvOrder[0];
        }
        pUV = &quad.m_uv[idx];
        pUV->f.x = 0.75f;
        pUV->f.y = 0.666f;

        if (uvOrder == NULL)
        {
            idx = 1;
        }
        else
        {
            idx = uvOrder[1];
        }
        pUV = &quad.m_uv[idx];
        pUV->f.x = 0.25f;
        pUV->f.y = 0.666f;

        if (uvOrder == NULL)
        {
            idx = 2;
        }
        else
        {
            idx = uvOrder[2];
        }
        pUV = &quad.m_uv[idx];
        pUV->f.x = 0.25f;
        pUV->f.y = 0.125f;

        if (uvOrder == NULL)
        {
            idx = 3;
        }
        else
        {
            idx = uvOrder[3];
        }
        pUV = &quad.m_uv[idx];
        pUV->f.x = 0.75f;
        pUV->f.y = 0.125f;
    }

    quad.m_colour[1] = c;
    quad.m_colour[0] = c;
    quad.m_colour[3] = cfade;
    quad.m_colour[2] = cfade;

    glSetDefaultState(true);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTexture(texture, GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 3);
    glSetCurrentTextureState(glHandleizeTextureState());

    if (pPoints == NULL || !g_bSubdivideShadow)
    {
        quad.Attach(g_CharacterShadowView, 0, true);
    }
    else
    {
        SubdivideAndRender(quad, g_CharacterShadowView);
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80123034 | size: 0x750
 * TODO: 80.78% match - register allocation shift in vTemp copy section (height f29 vs f27,
 *       width f27 vs f24) cascades through quad corner computation scheduling
 */
void RenderProjectedShadow(const ProjectedShadowParams& params)
{
    nlVector3 vTemp;
    nlVector3 vUp = { 0.0f, 0.0f, 1.0f };
    nlVector3 p[4];
    nlVector3 vLight;
    float width;
    float height;
    nlColour c;
    int i;
    nlVector3 dir;
    nlColour colour;
    nlMatrix4 mLight;

    if (g_bShadowBlobs)
    {
        RenderBlobShadow(params.vPosition, NULL, -1, NULL, NULL);
        return;
    }

    height = params.fScalar * params.fHeight;
    width = params.fScalar * params.fWidth;

    if (g_bShadowPositionOverride)
    {
        vLight = g_vShadowPosition;
    }
    else
    {
        vLight.f.x = params.vLight.f.x;
        vLight.f.y = params.vLight.f.y;
        vLight.f.z = params.vLight.f.z;
    }

    vTemp = params.vPosition;

    {
        float dy = vTemp.f.y - vLight.f.y;
        float dx = vTemp.f.x - vLight.f.x;
        float dz = 0.0f;
        float halfH = 0.5f * height;
        vTemp.f.z += halfH;

        float invLen = nlRecipSqrt(dy * dy + dx * dx + dz * dz, false);

        float vx = invLen * dx;
        float vy = invLen * dy;
        float vz = invLen * dz;

        float crossX = vy * vUp.f.z - vz * vUp.f.y;
        float crossY = vz * vUp.f.x - vx * vUp.f.z;
        float crossZ = vx * vUp.f.y - vy * vUp.f.x;

        float halfW = 0.5f * width;
        float negHalfW = -0.5f * width;
        float negHalfH = -0.5f * height;

        vTemp = params.vPosition;
        vTemp.f.z += halfH;

        p[0].f.x = vTemp.f.x + halfW * crossX + negHalfH * vUp.f.x;
        p[0].f.y = vTemp.f.y + halfW * crossY + negHalfH * vUp.f.y;
        p[0].f.z = vTemp.f.z + halfW * crossZ + negHalfH * vUp.f.z;

        p[1].f.x = vTemp.f.x + negHalfW * crossX + negHalfH * vUp.f.x;
        p[1].f.y = vTemp.f.y + negHalfW * crossY + negHalfH * vUp.f.y;
        p[1].f.z = vTemp.f.z + negHalfW * crossZ + negHalfH * vUp.f.z;

        p[2].f.x = vTemp.f.x + negHalfW * crossX + halfH * vUp.f.x;
        p[2].f.y = vTemp.f.y + negHalfW * crossY + halfH * vUp.f.y;
        p[2].f.z = vTemp.f.z + negHalfW * crossZ + halfH * vUp.f.z;

        p[3].f.x = vTemp.f.x + halfW * crossX + halfH * vUp.f.x;
        p[3].f.y = vTemp.f.y + halfW * crossY + halfH * vUp.f.y;
        p[3].f.z = vTemp.f.z + halfW * crossZ + halfH * vUp.f.z;
    }

    nlVector3* p1 = &p[1];
    nlVector3* p2 = &p[2];
    nlVector3* p3 = &p[3];

    if (g_bShadowBounds)
    {
        c.c[0] = 0x40;
        c.c[1] = 0x40;
        c.c[2] = 0xFF;
        c.c[3] = 0xFF;
        g_ShapeRenderer.DrawLine3D(p[0], *p1, c, false);
        g_ShapeRenderer.DrawLine3D(*p1, *p2, c, false);
        g_ShapeRenderer.DrawLine3D(*p2, *p3, c, false);
        g_ShapeRenderer.DrawLine3D(*p3, p[0], c, false);
    }

    {
        nlVector3* pPoint = p;
        for (i = 0; i < 4; i++, pPoint++)
        {
            float lx = -vLight.f.x;
            float ly = -vLight.f.y;
            float lz = -vLight.f.z;

            if (g_bShadowDirectional)
            {
                float pz = pPoint->f.z;
                float py = pPoint->f.y;
                float px = pPoint->f.x;
                float invLen = nlRecipSqrt(ly * ly + lx * lx + lz * lz, false);
                float dirX = invLen * lx;
                float dirY = invLen * ly;
                float dirZ = invLen * lz;

                float Vx = 0.0f;
                float Vy = 0.0f;
                float Vz = 1.0f;
                float num = Vx * px + Vy * py + Vz * pz;
                float den = Vx * dirX + Vy * dirY + Vz * dirZ;
                float t = -(num / den);

                pPoint->f.x = px + t * dirX;
                pPoint->f.y = py + t * dirY;
                pPoint->f.z = pz + t * dirZ;
            }
            else
            {
                nlVector4 V = { 0.0f, 0.0f, 1.0f, 0.0f };
                nlVector4 Q = { 0.0f, 0.0f, 0.0f, 1.0f };
                nlVector4 L = { 0.0f, 0.0f, 0.0f, 0.0f };

                Q.f.x = pPoint->f.x;
                Q.f.y = pPoint->f.y;
                Q.f.z = pPoint->f.z;

                L.f.x = vLight.f.x;
                L.f.y = vLight.f.y;
                L.f.z = vLight.f.z;

                float qx = Q.f.x - L.f.x;
                float qy = Q.f.y - L.f.y;
                float qz = Q.f.z - L.f.z;

                float qDot = V.f.x * Q.f.x + V.f.y * Q.f.y + V.f.z * Q.f.z + V.f.w * Q.f.w;
                float lDot = V.f.x * qx + V.f.y * qy + V.f.z * qz + V.f.w * L.f.w;
                float t = -(qDot / lDot);

                pPoint->f.x = Q.f.x + t * qx;
                pPoint->f.y = Q.f.y + t * qy;
                pPoint->f.z = Q.f.z + t * qz;
            }

            pPoint->f.z = g_AntiFlimmer;
        }
    }

    dir = vLight;
    dir.f.z = 0.0f;

    {
        float invLen = nlRecipSqrt(dir.f.x * dir.f.x + dir.f.y * dir.f.y + dir.f.z * dir.f.z, false);
        dir.f.x = dir.f.x * invLen;
        dir.f.y = dir.f.y * invLen;
        dir.f.z = dir.f.z * invLen;
    }

    {
        float xAdjust = g_fProjectionAdjust * dir.f.x;
        float yAdjust = g_fProjectionAdjust * dir.f.y;

        p[0].f.x += xAdjust;
        p[0].f.y += yAdjust;
        p[1].f.x += xAdjust;
        p[1].f.y += yAdjust;
        p[2].f.x += xAdjust;
        p[2].f.y += yAdjust;
        p[3].f.x += xAdjust;
        p[3].f.y += yAdjust;
    }

    if (g_bShadowBounds)
    {
        c.c[0] = 0x40;
        c.c[2] = 0x40;
        c.c[1] = 0xFF;
        c.c[3] = 0xFF;
        g_ShapeRenderer.DrawLine3D(p[0], *p1, c, false);
        g_ShapeRenderer.DrawLine3D(*p1, *p2, c, false);
        g_ShapeRenderer.DrawLine3D(*p2, *p3, c, false);
        g_ShapeRenderer.DrawLine3D(*p3, p[0], c, false);
    }

    {
        const static u32 zeroColour = 0x0;
        float newAntiFlimmer = GetCoPlanarZ();
        float oldAntiFlimmer = g_AntiFlimmer;
        g_AntiFlimmer = newAntiFlimmer;

        *(u32*)&colour = zeroColour;
        // nlColourSet(colour, 0, 0, 0, g_Alpha[0]);
        colour.c[3] = (u8)g_Alpha[0];

        RenderBlobShadow(params.vPosition, p, params.nPartitionIndex, NULL, &colour);
        g_AntiFlimmer = oldAntiFlimmer;
    }

    if (g_bShadowBounds)
    {
        mLight.SetIdentity();
        mLight.f.m41 = vLight.f.x;
        mLight.f.m42 = vLight.f.y;
        mLight.f.m43 = vLight.f.z;
        mLight.f.m44 = 1.0f;

        c.c[0] = 0xFF;
        c.c[1] = 0xFF;
        c.c[2] = 0x40;
        c.c[3] = 0xFF;
        g_ShapeRenderer.DrawSpherePrimitive(mLight, 0.5f, c);
    }
}
