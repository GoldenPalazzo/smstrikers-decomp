#include "Game/Debug/ShapeRender.h"

#include "Game/GL/gluMeshWriter.h"

#include "NL/gl/glDraw2.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"

ShapeRender g_ShapeRenderer;
static unsigned long UnlitProgram = glGetProgram("3d unlit");
static unsigned long LitProgram = glGetProgram("3d pointlit");
const u32 WhiteTexture = glGetTexture("global/white");

static unsigned char g_bLit;

/**
 * Offset/Address/Size: 0x149C | 0x801FC72C | size: 0x418
 * TODO: 98.78% match - constant and angle temporaries still use different saved registers.
 */
void ShapeRender::CreateHemisphereGeometry(PrimitiveShape& prim)
{
    nlVector3 vNormal;
    nlVector3* pdst;
    nlVector3* ndst;
    nlVector2* tdst;
    int nRing;
    float y1;
    float ring0;
    float ring1;
    float z0Sq;
    float z1Sq;
    float z0;
    float z1;
    float x0;
    float y0;
    float x1;
    float x0Sq;
    float y0Sq;
    float x1Sq;
    float y1Sq;
    float invLen;
    int nSegment;

    prim.vertCount = 0xA0;
    prim.position = (nlVector3*)glResourceAlloc(0x780, GLM_VertexData);
    prim.normal = (nlVector3*)glResourceAlloc(0x780, GLM_VertexData);
    prim.texcoord = (nlVector2*)glResourceAlloc(0x500, GLM_VertexData);

    pdst = prim.position;
    ndst = prim.normal;
    tdst = prim.texcoord;

    for (nRing = 0; nRing < 5; nRing++)
    {
        int angle0;
        int angle1;
        float fAngle;

        fAngle = (float)nRing;
        fAngle *= 0.31415927f;
        angle0 = (int)(fAngle * 10430.378f);
        z0 = 0.5f * nlSin((u16)angle0);

        fAngle = (float)(nRing + 1);
        fAngle *= 0.31415927f;
        angle1 = (int)(fAngle * 10430.378f);
        z1 = 0.5f * nlSin((u16)angle1);

        ring0 = nlSin((u16)((u16)angle0 + 0x4000));
        ring1 = nlSin((u16)((u16)angle1 + 0x4000));

        z0Sq = z0 * z0;
        z1Sq = z1 * z1;

        for (nSegment = 0; nSegment < 0x10; nSegment++)
        {
            int angle;
            int angle90;
            float fSegmentAngle;

            fSegmentAngle = (float)nSegment;
            angle = (int)((fSegmentAngle *= 0.41887903f) * 10430.378f);

            x0 = 0.5f * (ring0 * nlSin((u16)angle));

            angle90 = (u16)angle + 0x4000;
            y0 = 0.5f * (ring0 * nlSin((u16)(s32)angle90));

            x1 = ring1 * (0.5f * nlSin((u16)angle));
            y1 = ring1 * (0.5f * nlSin((u16)(s32)angle90));

            x0Sq = x0 * x0;
            y0Sq = y0 * y0;

            vNormal.f.x = x0;
            vNormal.f.y = y0;
            vNormal.f.z = z0;

            invLen = nlRecipSqrt(z0Sq + (x0Sq + y0Sq), true);

            pdst->f.x = x0;
            nlVec3Scale(vNormal, invLen);
            pdst->f.y = y0;
            pdst->f.z = z0;
            *ndst = vNormal;

            tdst->f.x = (float)nSegment / 15.0f;
            tdst->f.y = (float)nRing / 5.0f;

            pdst++;
            ndst++;
            tdst++;

            x1Sq = x1 * x1;
            y1Sq = y1 * y1;

            vNormal.f.x = x1;
            vNormal.f.y = y1;
            vNormal.f.z = z1;

            invLen = nlRecipSqrt(z1Sq + (x1Sq + y1Sq), true);

            pdst->f.x = x1;
            nlVec3Scale(vNormal, invLen);
            pdst->f.y = y1;
            pdst->f.z = z1;
            *ndst = vNormal;

            tdst->f.x = (float)nSegment / 15.0f;
            tdst->f.y = (float)(nRing + 1) / 5.0f;

            pdst++;
            ndst++;
            tdst++;
        }
    }
}

/**
 * Offset/Address/Size: 0x11C0 | 0x801FC450 | size: 0x2DC
 * TODO: 98.52% match - saved-FPR allocation for pre-loop zero and angle constants still differs.
 */
void ShapeRender::CreateFlatCylinderEndGeometry(PrimitiveShape& prim)
{
    nlVector3 vNormal;
    int angle;
    int angle90;
    nlVector3* pdst;
    nlVector3* ndst;
    nlVector2* tdst;
    int nSegment;
    float z0;
    float z0Sq;
    float angleFactor;
    float segmentFactor;
    float half;
    float one;
    float texDenom;
    float x0;
    float y0;
    float x1;
    float y1;
    float x0Sq;
    float y0Sq;
    float x1Sq;
    float y1Sq;
    float invLen;

    prim.vertCount = 0x20;
    prim.position = (nlVector3*)glResourceAlloc(0x180, GLM_VertexData);
    prim.normal = (nlVector3*)glResourceAlloc(0x180, GLM_VertexData);
    prim.texcoord = (nlVector2*)glResourceAlloc(0x100, GLM_VertexData);

    z0 = 0.0f;
    z0Sq = z0;
    z0 *= z0Sq;
    angleFactor = 10430.378f;
    segmentFactor = 0.41887903f;
    half = 0.5f;
    one = 1.0f;
    texDenom = 15.0f;

    pdst = prim.position;
    ndst = prim.normal;
    tdst = prim.texcoord;

    for (nSegment = 0; nSegment < 0x10; nSegment++)
    {
        angle = (int)(angleFactor * ((float)nSegment * segmentFactor));

        x0 = half * (one * nlSin((u16)angle));

        angle90 = (u16)angle + 0x4000;
        y0 = half * (one * nlSin((u16)angle90));

        x1 = z0 * (half * nlSin((u16)angle));
        y1 = z0 * (half * nlSin((u16)angle90));

        x0Sq = x0 * x0;
        y0Sq = y0 * y0;

        vNormal.f.x = x0;
        vNormal.f.y = y0;
        vNormal.f.z = z0;

        invLen = nlRecipSqrt(z0Sq + (x0Sq + y0Sq), true);

        pdst->f.x = x0;
        vNormal.f.y = invLen * vNormal.f.y;
        vNormal.f.x = invLen * vNormal.f.x;
        pdst->f.y = y0;
        pdst->f.z = z0;
        vNormal.f.z = invLen * vNormal.f.z;
        *ndst = vNormal;

        tdst->f.x = (float)nSegment / texDenom;
        tdst->f.y = z0;

        x1Sq = x1 * x1;
        y1Sq = y1 * y1;

        vNormal.f.x = x1;
        vNormal.f.y = y1;
        vNormal.f.z = z0;

        invLen = nlRecipSqrt(z0Sq + (x1Sq + y1Sq), true);

        pdst[1].f.x = x1;
        pdst[1].f.y = y1;
        pdst[1].f.z = z0;
        nlVec3Scale(vNormal, invLen);
        ndst[1] = vNormal;

        tdst[1].f.x = (float)nSegment / texDenom;
        tdst[1].f.y = one;

        pdst += 2;
        ndst += 2;
        tdst += 2;
    }
}

/**
 * Offset/Address/Size: 0xE14 | 0x801FC0A4 | size: 0x3AC
 * TODO: 99.11% match - saved-FPR allocation for constants and ring-square temporaries still differs.
 */
void ShapeRender::CreateCylinderGeometry(PrimitiveShape& prim)
{
    nlVector3 vNormal;
    nlVector3* pdst;
    nlVector3* ndst;
    nlVector2* tdst;
    int nRing;
    int angle;
    int angle90;
    float z0Sq;
    float z1Sq;
    float z0;
    float z1;
    float x0;
    float y0;
    float x1;
    float y1;
    float x0Sq;
    float y0Sq;
    float x1Sq;
    float y1Sq;
    float invLen;

    prim.vertCount = 0x40;
    prim.position = (nlVector3*)glResourceAlloc(0x300, GLM_VertexData);
    prim.normal = (nlVector3*)glResourceAlloc(0x300, GLM_VertexData);
    prim.texcoord = (nlVector2*)glResourceAlloc(0x200, GLM_VertexData);

    pdst = prim.position;
    ndst = prim.normal;
    tdst = prim.texcoord;

    for (nRing = 0; nRing < 2; nRing++)
    {
        z0 = -0.5f + (float)nRing * 0.5f;
        z1 = -0.5f + (float)(nRing + 1) * 0.5f;

        nlSin((u16)((u16)(int)(10430.378f * ((float)nRing * 0.5f)) + 0x4000));
        nlSin((u16)((u16)(int)(10430.378f * ((float)(nRing + 1) * 0.5f)) + 0x4000));

        z0Sq = z0 * z0;
        z1Sq = z1 * z1;

        for (int nSegment = 0; nSegment < 0x10; nSegment++)
        {
            float fSegmentAngle;

            fSegmentAngle = (float)nSegment;
            angle = (int)((fSegmentAngle *= 0.41887903f) * 10430.378f);

            x0 = 0.5f * nlSin((u16)angle);

            angle90 = (u16)angle + 0x4000;
            y0 = 0.5f * nlSin((u16)angle90);

            x1 = 0.5f * nlSin((u16)angle);
            y1 = 0.5f * nlSin((u16)angle90);

            x0Sq = x0 * x0;
            y0Sq = y0 * y0;

            vNormal.f.x = x0;
            vNormal.f.y = y0;
            vNormal.f.z = z0;

            invLen = nlRecipSqrt(z0Sq + (x0Sq + y0Sq), true);

            pdst->f.x = x0;
            nlVec3Scale(vNormal, invLen);
            pdst->f.y = y0;
            pdst->f.z = z0;
            *ndst = vNormal;

            tdst->f.x = (float)nSegment / 15.0f;
            tdst->f.y = (float)nRing / 2.0f;

            x1Sq = x1 * x1;
            y1Sq = y1 * y1;

            vNormal.f.x = x1;
            vNormal.f.y = y1;
            vNormal.f.z = z1;

            invLen = nlRecipSqrt(z1Sq + (x1Sq + y1Sq), true);

            pdst[1].f.x = x1;
            nlVec3Scale(vNormal, invLen);
            pdst[1].f.y = y1;
            pdst[1].f.z = z1;
            ndst[1] = vNormal;

            tdst[1].f.x = (float)nSegment / 15.0f;
            tdst[1].f.y = (float)(nRing + 1) / 2.0f;

            pdst += 2;
            ndst += 2;
            tdst += 2;
        }
    }
}

/**
 * Offset/Address/Size: 0xAC0 | 0x801FBD50 | size: 0x354
 * TODO: 99.95% match - lit-program register assignment in the second packet loop still differs.
 */
void ShapeRender::DrawSpherePrimitive(const nlMatrix4& mat_world, float radius, const nlColour& colour) const
{
    nlMatrix4 mat_hemiTop;
    nlMatrix4 mat_hemiBottom;
    nlMatrix4 mat_rot;

    radius = radius / 100.0f;

    nlMakeScaleMatrix(mat_hemiTop, radius, radius, radius);
    nlMakeRotationMatrixX(mat_rot, 3.1415927f);
    nlMultMatrices(mat_hemiBottom, mat_hemiTop, mat_rot);
    nlMultMatrices(mat_hemiTop, mat_hemiTop, mat_world);
    nlMultMatrices(mat_hemiBottom, mat_hemiBottom, mat_world);

    {
        unsigned long matrix = glAllocMatrix();
        if (matrix + 0x10000 != 0xFFFF)
        {
            glSetMatrix(matrix, mat_hemiTop);
        }

        unsigned long litProgram;
        void* pLightData;
        glModelPacket* packet;
        void* pUserData;
        glModel* pModel = glModelDupNoStreams(m_Hemisphere.model, true, false);

        if (g_bLit)
        {
            void* pColourData;
            pUserData = glUserAlloc(GLUD_Diffuse, sizeof(nlFloatColour), false);
            pColourData = glUserGetData(pUserData);

            ((nlFloatColour*)pColourData)->c[0] = (float)colour.c[0] / 255.0f;
            ((nlFloatColour*)pColourData)->c[1] = (float)colour.c[1] / 255.0f;
            ((nlFloatColour*)pColourData)->c[2] = (float)colour.c[2] / 255.0f;
            ((nlFloatColour*)pColourData)->c[3] = (float)colour.c[3] / 255.0f;
        }
        else
        {
            void* pColourData;
            pUserData = glUserAlloc(GLUD_ConstantColour, sizeof(nlColour), false);
            pColourData = glUserGetData(pUserData);
            *(unsigned long*)pColourData = *(unsigned long*)&colour;
        }

        packet = pModel->packets;
        pLightData = m_pLightUserData;
        litProgram = LitProgram;

        while (packet < &pModel->packets[pModel->numPackets])
        {
            packet->state.matrix = matrix;
            glUserAttach(pUserData, packet, false);

            if (g_bLit && pLightData != NULL)
            {
                packet->state.program = litProgram;
                glUserAttach(pLightData, packet, false);
            }

            packet++;
        }

        glViewAttachModel(m_eView, pModel);
    }

    {
        unsigned long matrix = glAllocMatrix();
        if (matrix + 0x10000 != 0xFFFF)
        {
            glSetMatrix(matrix, mat_hemiBottom);
        }

        glModelPacket* packet;
        unsigned long litProgram;
        void* pLightData;
        void* pUserData;
        glModel* pModel = glModelDupNoStreams(m_Hemisphere.model, true, false);

        if (g_bLit)
        {
            void* pColourData;
            pUserData = glUserAlloc(GLUD_Diffuse, sizeof(nlFloatColour), false);
            pColourData = glUserGetData(pUserData);

            ((nlFloatColour*)pColourData)->c[0] = (float)colour.c[0] / 255.0f;
            ((nlFloatColour*)pColourData)->c[1] = (float)colour.c[1] / 255.0f;
            ((nlFloatColour*)pColourData)->c[2] = (float)colour.c[2] / 255.0f;
            ((nlFloatColour*)pColourData)->c[3] = (float)colour.c[3] / 255.0f;
        }
        else
        {
            void* pColourData;
            pUserData = glUserAlloc(GLUD_ConstantColour, sizeof(nlColour), false);
            pColourData = glUserGetData(pUserData);
            *(unsigned long*)pColourData = *(unsigned long*)&colour;
        }

        packet = pModel->packets;
        pLightData = m_pLightUserData;
        litProgram = LitProgram;

        while (packet < &pModel->packets[pModel->numPackets])
        {
            packet->state.matrix = matrix;
            glUserAttach(pUserData, packet, false);

            if (g_bLit && pLightData != NULL)
            {
                packet->state.program = litProgram;
                glUserAttach(pLightData, packet, false);
            }

            packet++;
        }

        glViewAttachModel(m_eView, pModel);
    }
}

/**
 * Offset/Address/Size: 0x930 | 0x801FBBC0 | size: 0x190
 */
void ShapeRender::DrawLine3D(const nlVector3& p0, const nlVector3& p1, const nlColour& colour, bool bWithDepth) const
{
    GLMeshWriter writer;

    glSetDefaultState(bWithDepth);
    glSetCurrentMatrix(glGetIdentityMatrix());
    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetCurrentProgram(glGetProgram("3d unlit"));

    const eGLStream streams[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };

    if (writer.Begin(2, GLP_LineList, 3, streams, false))
    {
        nlVector2 uv0;
        nlVector2 uv1;

        writer.Colour(colour);
        uv0.f.x = 0.0f;
        uv0.f.y = 0.0f;
        ((GLMeshWriterCore*)&writer)->Texcoord(uv0);
        writer.Vertex(p0);

        writer.Colour(colour);
        uv1.f.x = 0.0f;
        uv1.f.y = 0.0f;
        ((GLMeshWriterCore*)&writer)->Texcoord(uv1);
        writer.Vertex(p1);

        if (!writer.End())
        {
            return;
        }

        glViewAttachModel(m_eView, 2, writer.GetModel());
    }
}

/**
 * Offset/Address/Size: 0x7EC | 0x801FBA7C | size: 0x144
 */
void ShapeRender::DrawRectangle2D(float x, float y, float w, float h, float z, const nlColour& colour, int view) const
{
    glSetDefaultState(false);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_AlphaTest, 1);
    glSetRasterState(GLS_AlphaTestRef, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTexture(glGetTexture("global/white"), GLTT_Diffuse);

    float right = y + h;
    float bottom = x + w;

    glPoly2 poly;
    poly.m_pos[0].f.x = x;
    poly.m_pos[0].f.y = y;

    glPoly2* pPoly = &poly;

    poly.m_pos[1].f.x = x;
    poly.m_pos[1].f.y = right;
    poly.m_pos[2].f.x = bottom;
    poly.m_pos[2].f.y = right;
    poly.m_pos[3].f.x = bottom;
    poly.m_pos[3].f.y = y;

    poly.m_colour[3] = colour;
    poly.m_colour[2] = colour;
    poly.m_colour[1] = colour;
    poly.m_colour[0] = colour;

    poly.depth = z;

    eGLView v = (eGLView)view;
    if (view == -1)
    {
        v = GLV_Debug;
    }
    pPoly->Attach(v, 0, NULL, -1);
}

static inline void BuildShapeMesh(PrimitiveShape& shape)
{
    extern unsigned long UnlitProgram;
    eGLStream stream_decl[4] = { GLStream_Position, GLStream_Colour, GLStream_Normal, GLStream_Diffuse };
    GLMeshWriter mesh;
    nlColour colour;
    shape.model = NULL;
    colour.c[0] = 0xFF;
    colour.c[1] = 0xFF;
    colour.c[2] = 0xFF;
    colour.c[3] = 0xFF;

    nlVector3* pPosition = shape.position;
    nlVector2* pTexcoord = shape.texcoord;
    nlVector3* pNormal = shape.normal;

    glSetDefaultState(true);
    glSetCurrentMatrix(glGetIdentityMatrix());
    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetCurrentProgram(UnlitProgram);

    if (mesh.Begin(shape.vertCount, GLP_TriStrip, 4, stream_decl, true))
    {
        int index = 0;
        while (index < shape.vertCount)
        {
            mesh.Colour(colour);
            mesh.Normal(*pNormal);
            mesh.Texcoord(*pTexcoord);
            mesh.Vertex(*pPosition);

            pNormal++;
            pTexcoord++;
            pPosition++;
            index++;
        }

        if (mesh.End())
        {
            shape.model = mesh.GetModel();
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801FB290 | size: 0x7EC
 * TODO: 92.59% match - face-building loop and mesh blocks still use different
 * saved registers for this and the loop temporaries.
 */
void ShapeRender::Initialize()
{
    static int ind_vert[24] = {
        0,
        2,
        3,
        1,
        4,
        5,
        7,
        6,
        0,
        1,
        5,
        4,
        1,
        3,
        7,
        5,
        3,
        2,
        6,
        7,
        2,
        0,
        4,
        6,
    };
    static int ind_uv[24] = {
        1,
        3,
        2,
        0,
        0,
        1,
        3,
        2,
        0,
        1,
        3,
        2,
        0,
        1,
        3,
        2,
        0,
        1,
        3,
        2,
        0,
        1,
        3,
        2,
    };
    static nlVector3 data_vert[8] = {
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { -0.5f, 0.5f, -0.5f },
        { 0.5f, 0.5f, -0.5f },
        { -0.5f, -0.5f, 0.5f },
        { 0.5f, -0.5f, 0.5f },
        { -0.5f, 0.5f, 0.5f },
        { 0.5f, 0.5f, 0.5f },
    };
    static nlVector2 data_uv[4] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
    };
    static nlVector3 data_norm[24] = {
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 0.0f, -1.0f },

        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },

        { 0.0f, -1.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f },

        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },

        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },

        { -1.0f, 0.0f, 0.0f },
        { -1.0f, 0.0f, 0.0f },
        { -1.0f, 0.0f, 0.0f },
        { -1.0f, 0.0f, 0.0f },
    };
    static int tri_map[6] = { 0, 1, 2, 3, 0, 2 };

    if (!m_Initialized)
    {
        m_Initialized = true;

        m_Box.position = (nlVector3*)glResourceAlloc(0x1B0, GLM_VertexData);
        m_Box.normal = (nlVector3*)glResourceAlloc(0x1B0, GLM_VertexData);
        m_Box.texcoord = (nlVector2*)glResourceAlloc(0x120, GLM_VertexData);
        m_Box.vertCount = 0x24;

        nlVector3* pos = m_Box.position;
        nlVector3* norm = m_Box.normal;
        nlVector2* uv = m_Box.texcoord;

        for (int iQuad = 0; iQuad < 6; iQuad++)
        {
            nlVector2* faceUVs[4];
            nlVector3* faceNorms[4];
            nlVector3* faceVerts[4];

            int base = iQuad * 4;
            int idx;

            idx = base + 0;
            faceVerts[0] = &data_vert[ind_vert[idx]];
            faceUVs[0] = &data_uv[ind_uv[idx]];
            faceNorms[0] = &data_norm[idx];

            idx = base + 1;
            faceVerts[1] = &data_vert[ind_vert[idx]];
            faceUVs[1] = &data_uv[ind_uv[idx]];
            faceNorms[1] = &data_norm[idx];

            idx = base + 2;
            faceVerts[2] = &data_vert[ind_vert[idx]];
            faceUVs[2] = &data_uv[ind_uv[idx]];
            faceNorms[2] = &data_norm[idx];

            idx = base + 3;
            faceVerts[3] = &data_vert[ind_vert[idx]];
            faceUVs[3] = &data_uv[ind_uv[idx]];
            faceNorms[3] = &data_norm[idx];

            for (int j = 0; j < 3; j++)
            {
                *pos = *faceVerts[tri_map[j * 2 + 0]];
                *norm = *faceNorms[tri_map[j * 2 + 0]];
                *uv = *faceUVs[tri_map[j * 2 + 0]];

                pos[1] = *faceVerts[tri_map[j * 2 + 1]];
                norm[1] = *faceNorms[tri_map[j * 2 + 1]];
                uv[1] = *faceUVs[tri_map[j * 2 + 1]];

                pos += 2;
                norm += 2;
                uv += 2;
            }
        }

        CreateCylinderGeometry(m_Cylinder);
        CreateHemisphereGeometry(m_Hemisphere);
        CreateFlatCylinderEndGeometry(m_FlatCylinderEnd);

        BuildShapeMesh(m_Box);
        BuildShapeMesh(m_Cylinder);
        BuildShapeMesh(m_Hemisphere);
        BuildShapeMesh(m_FlatCylinderEnd);

        struct ShapeRenderLightUserData
        {
            u32 type;
            nlVector3 worldPosition;
            nlFloatColour colour;
            float intensity;
            float innerRadius;
            float outerRadius;
        };

        void* pUser = glUserAlloc(GLUD_Light, sizeof(ShapeRenderLightUserData), true);
        ShapeRenderLightUserData* pLight = (ShapeRenderLightUserData*)glUserGetData(pUser);

        pLight->type = 1;
        pLight->worldPosition.f.x = 0.5f;
        pLight->worldPosition.f.y = 0.5f;
        pLight->worldPosition.f.z = 10.0f;
        pLight->colour.c[0] = 0.5f;
        pLight->colour.c[1] = 0.5f;
        pLight->colour.c[2] = 1.0f;
        pLight->colour.c[3] = 0.0f;
        pLight->intensity = 1.0f;
        pLight->innerRadius = 0.0f;
        pLight->outerRadius = 30.0f;

        m_pLightUserData = pUser;
        m_eView = (eGLView)7;
    }
}
