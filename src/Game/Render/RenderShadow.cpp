#include "Game/Render/RenderShadow.h"

#include "Game/GL/GLInventory.h"
#include "Game/WorldManager.h"
#include "NL/gl/glAppAttach.h"
#include "NL/gl/glDraw3.h"
#include "NL/nlString.h"

extern GLInventory glInventory;
extern unsigned long ResolvedWhiteTexture;

static u8 g_bPreview;
static unsigned long FourTexture;
static glModel* pCylinder;
static glModel* pBox;
int MaxProjectedShadows;

static int g_Alpha[3];
static int g_AlphaRef;
static unsigned char g_bCoPlanarProjectedShadows;

// /**
//  * Offset/Address/Size: 0x1734 | 0x80124768 | size: 0x7C
//  */
// void 0x8028D2CC..0x8028D2D0 | size: 0x4
// {
// }

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
    extern u8 g_bProjectDirectional;
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
        glModelPacket* pPacket;
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
        glModelPacket* pPacket;
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
void RenderBlobShadow(const nlVector3&, const nlVector3*, int, const int*, const nlColour*)
{
}

/**
 * Offset/Address/Size: 0x0 | 0x80123034 | size: 0x750
 */
void RenderProjectedShadow(const ProjectedShadowParams&)
{
}
