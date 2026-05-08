#include "Game/Debug/ShapeRender.h"

#include "Game/GL/gluMeshWriter.h"

#include "NL/gl/glDraw2.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"

extern const u32 WhiteTexture;
static unsigned char g_bLit;
static unsigned long LitProgram;

/**
 * Offset/Address/Size: 0x149C | 0x801FC72C | size: 0x418
 * TODO: 80.73% match - FPR allocation and instruction scheduling still differ in inner-loop trig/normal code.
 */
void ShapeRender::CreateHemisphereGeometry(PrimitiveShape& prim)
{
    nlVector3 vNormal;
    nlVector3* pdst;
    nlVector3* ndst;
    nlVector2* tdst;
    int nRing;
    float z0;
    float z1;
    int nSegment;
    float x0;
    float y0;
    float x1;
    float y1;
    float ring0;
    float ring1;
    float z0Sq;
    float z1Sq;

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

        angle0 = (int)(10430.378f * ((float)nRing * 0.31415927f));
        z0 = 0.5f * nlSin((u16)angle0);

        angle1 = (int)(10430.378f * ((float)(nRing + 1) * 0.31415927f));
        z1 = 0.5f * nlSin((u16)angle1);

        ring0 = nlSin((u16)((u16)angle0 + 0x4000));
        ring1 = nlSin((u16)((u16)angle1 + 0x4000));

        z0Sq = z0 * z0;
        z1Sq = z1 * z1;

        for (nSegment = 0; nSegment < 0x10; nSegment++)
        {
            int angle;
            int angle90;
            float x0Sq;
            float y0Sq;
            float x1Sq;
            float y1Sq;
            float invLen;

            angle = (int)(10430.378f * ((float)nSegment * 0.41887903f));

            x0 = 0.5f * (ring0 * nlSin((u16)angle));

            angle90 = (u16)angle + 0x4000;
            y0 = 0.5f * (ring0 * nlSin((u16)angle90));

            x1 = 0.5f * (ring1 * nlSin((u16)angle));
            y1 = 0.5f * (ring1 * nlSin((u16)angle90));

            x0Sq = x0 * x0;
            y0Sq = y0 * y0;
            invLen = nlRecipSqrt(z0Sq + (x0Sq + y0Sq), true);

            pdst->f.x = x0;
            pdst->f.y = y0;
            pdst->f.z = z0;

            vNormal.f.x = invLen * x0;
            vNormal.f.y = invLen * y0;
            vNormal.f.z = invLen * z0;
            *ndst = vNormal;

            tdst->f.x = (float)nSegment / 15.0f;
            tdst->f.y = (float)nRing / 5.0f;

            x1Sq = x1 * x1;
            y1Sq = y1 * y1;
            invLen = nlRecipSqrt(z1Sq + (x1Sq + y1Sq), true);

            pdst[1].f.x = x1;
            pdst[1].f.y = y1;
            pdst[1].f.z = z1;

            vNormal.f.x = invLen * x1;
            vNormal.f.y = invLen * y1;
            vNormal.f.z = invLen * z1;
            ndst[1] = vNormal;

            tdst[1].f.x = (float)nSegment / 15.0f;
            tdst[1].f.y = (float)(nRing + 1) / 5.0f;

            pdst += 2;
            ndst += 2;
            tdst += 2;
        }
    }
}

/**
 * Offset/Address/Size: 0x11C0 | 0x801FC450 | size: 0x2DC
 * TODO: 66.07% match - register allocation and constant ordering in the segment loop still differ.
 */
void ShapeRender::CreateFlatCylinderEndGeometry(PrimitiveShape& prim)
{
    nlVector3 vNormal;
    int nSegment;
    nlVector3* pdst;
    nlVector3* ndst;
    nlVector2* tdst;
    float x0;
    float y0;
    float x1;
    float y1;

    prim.vertCount = 0x20;
    prim.position = (nlVector3*)glResourceAlloc(0x180, GLM_VertexData);
    prim.normal = (nlVector3*)glResourceAlloc(0x180, GLM_VertexData);
    prim.texcoord = (nlVector2*)glResourceAlloc(0x100, GLM_VertexData);

    pdst = prim.position;
    ndst = prim.normal;
    tdst = prim.texcoord;

    for (nSegment = 0; nSegment < 0x10; nSegment++)
    {
        u16 angle = (u16)(10430.378f * ((float)nSegment * 0.3926991f));

        x0 = 2.0f * (0.5f * nlSin(angle));
        y0 = 2.0f * (0.5f * nlSin((u16)(angle + 0x4000)));
        x1 = 2.0f * (1.0f * nlSin(angle));
        y1 = 2.0f * (1.0f * nlSin((u16)(angle + 0x4000)));

        pdst->f.x = x0;
        pdst->f.y = y0;
        pdst->f.z = 0.5f;

        {
            float invLen = nlRecipSqrt((0.5f * 0.5f) + (x0 * x0 + y0 * y0), true);
            vNormal.f.x = invLen * x0;
            vNormal.f.y = invLen * y0;
            vNormal.f.z = invLen * 0.5f;
        }
        *ndst = vNormal;

        tdst->f.x = (float)nSegment / 16.0f;
        tdst->f.y = 0.5f;

        pdst[1].f.x = x1;
        pdst[1].f.y = y1;
        pdst[1].f.z = 0.5f;

        {
            float invLen = nlRecipSqrt((0.5f * 0.5f) + (x1 * x1 + y1 * y1), true);
            vNormal.f.x = invLen * x1;
            vNormal.f.y = invLen * y1;
            vNormal.f.z = invLen * 0.5f;
        }
        ndst[1] = vNormal;

        tdst[1].f.x = (float)nSegment / 16.0f;
        tdst[1].f.y = 1.0f;

        pdst += 2;
        ndst += 2;
        tdst += 2;
    }
}

/**
 * Offset/Address/Size: 0xE14 | 0x801FC0A4 | size: 0x3AC
 * TODO: 64.9% match - compiler CSEs texcoord Y floats to callee-saved FPRs
 * instead of recomputing from GPR xoris values each inner iteration.
 */
void ShapeRender::CreateCylinderGeometry(PrimitiveShape& prim)
{
    nlVector3 vNormal;
    nlVector3* pdst;
    nlVector3* ndst;
    nlVector2* tdst;
    int nRing;
    float z0;
    float z1;
    int nSegment;
    float x0;
    float y0;
    float x1;
    float y1;

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

        for (nSegment = 0; nSegment < 0x10; nSegment++)
        {
            int angle = (int)(10430.378f * ((float)nSegment * 0.3926991f));

            x0 = 0.5f * nlSin((u16)angle);
            y0 = 0.5f * nlSin((u16)((u16)angle + 0x4000));
            x1 = 0.5f * nlSin((u16)angle);
            y1 = 0.5f * nlSin((u16)((u16)angle + 0x4000));

            pdst->f.x = x0;
            pdst->f.y = y0;
            pdst->f.z = z0;

            {
                float invLen = nlRecipSqrt(z0 * z0 + (x0 * x0 + y0 * y0), true);
                vNormal.f.x = invLen * x0;
                vNormal.f.y = invLen * y0;
                vNormal.f.z = invLen * z0;
            }
            *ndst = vNormal;

            tdst->f.x = (float)nSegment / 16.0f;
            tdst->f.y = (float)nRing * 0.5f;

            pdst[1].f.x = x1;
            pdst[1].f.y = y1;
            pdst[1].f.z = z1;

            {
                float invLen = nlRecipSqrt(z1 * z1 + (x1 * x1 + y1 * y1), true);
                vNormal.f.x = invLen * x1;
                vNormal.f.y = invLen * y1;
                vNormal.f.z = invLen * z1;
            }
            ndst[1] = vNormal;

            tdst[1].f.x = (float)nSegment / 16.0f;
            tdst[1].f.y = (float)(nRing + 1) * 0.5f;

            pdst += 2;
            ndst += 2;
            tdst += 2;
        }
    }
}

/**
 * Offset/Address/Size: 0xAC0 | 0x801FBD50 | size: 0x354
 * TODO: 98.85% match - callee-saved register assignment differs in both packet loops.
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

        glModel* pModel = glModelDupNoStreams(m_Hemisphere.model, true, false);
        void* pUserData;

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

        glModelPacket* packet = pModel->packets;
        void* pLightData = m_pLightUserData;
        unsigned long litProgram = LitProgram;

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

        glModel* pModel = glModelDupNoStreams(m_Hemisphere.model, true, false);
        void* pUserData;

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

        glModelPacket* packet = pModel->packets;
        void* pLightData = m_pLightUserData;
        unsigned long litProgram = LitProgram;

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

/**
 * Offset/Address/Size: 0x0 | 0x801FB290 | size: 0x7EC
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

        for (int i = 0; i < 6; i++)
        {
            int base = i * 4;
            nlVector3* faceVerts[4];
            nlVector3* faceNorms[4];
            nlVector2* faceUVs[4];

            faceVerts[0] = &data_vert[ind_vert[base + 0]];
            faceNorms[0] = &data_norm[base + 0];
            faceUVs[0] = &data_uv[ind_uv[base + 0]];

            faceVerts[1] = &data_vert[ind_vert[base + 1]];
            faceNorms[1] = &data_norm[base + 1];
            faceUVs[1] = &data_uv[ind_uv[base + 1]];

            faceVerts[2] = &data_vert[ind_vert[base + 2]];
            faceNorms[2] = &data_norm[base + 2];
            faceUVs[2] = &data_uv[ind_uv[base + 2]];

            faceVerts[3] = &data_vert[ind_vert[base + 3]];
            faceNorms[3] = &data_norm[base + 3];
            faceUVs[3] = &data_uv[ind_uv[base + 3]];

            for (int j = 0; j < 3; j++)
            {
                int index0 = tri_map[j * 2 + 0];
                int index1 = tri_map[j * 2 + 1];

                *pos = *faceVerts[index0];
                *norm = *faceNorms[index0];
                *uv = *faceUVs[index0];

                pos[1] = *faceVerts[index1];
                norm[1] = *faceNorms[index1];
                uv[1] = *faceUVs[index1];

                pos += 2;
                norm += 2;
                uv += 2;
            }
        }

        CreateCylinderGeometry(m_Cylinder);
        CreateHemisphereGeometry(m_Hemisphere);
        CreateFlatCylinderEndGeometry(m_FlatCylinderEnd);

        {
            extern unsigned long UnlitProgram;
            GLMeshWriter mesh;
            eGLStream stream_decl[4] = { GLStream_Position, GLStream_Colour, GLStream_Normal, GLStream_Diffuse };
            nlColour colour;
            colour.c[0] = 0xFF;
            colour.c[1] = 0xFF;
            colour.c[2] = 0xFF;
            colour.c[3] = 0xFF;
            m_Box.model = NULL;

            nlVector3* pPosition = m_Box.position;
            nlVector3* pNormal = m_Box.normal;
            nlVector2* pTexcoord = m_Box.texcoord;

            glSetDefaultState(true);
            glSetCurrentMatrix(glGetIdentityMatrix());
            glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
            glSetCurrentProgram(UnlitProgram);

            if (mesh.Begin(m_Box.vertCount, GLP_TriStrip, 4, stream_decl, true))
            {
                int index = 0;
                while (index < m_Box.vertCount)
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
                    m_Box.model = mesh.GetModel();
                }
            }
        }

        {
            extern unsigned long UnlitProgram;
            GLMeshWriter mesh;
            eGLStream stream_decl[4] = { GLStream_Position, GLStream_Colour, GLStream_Normal, GLStream_Diffuse };
            nlColour colour;
            colour.c[0] = 0xFF;
            colour.c[1] = 0xFF;
            colour.c[2] = 0xFF;
            colour.c[3] = 0xFF;
            m_Cylinder.model = NULL;

            nlVector3* pPosition = m_Cylinder.position;
            nlVector3* pNormal = m_Cylinder.normal;
            nlVector2* pTexcoord = m_Cylinder.texcoord;

            glSetDefaultState(true);
            glSetCurrentMatrix(glGetIdentityMatrix());
            glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
            glSetCurrentProgram(UnlitProgram);

            if (mesh.Begin(m_Cylinder.vertCount, GLP_TriStrip, 4, stream_decl, true))
            {
                int index = 0;
                while (index < m_Cylinder.vertCount)
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
                    m_Cylinder.model = mesh.GetModel();
                }
            }
        }

        {
            extern unsigned long UnlitProgram;
            GLMeshWriter mesh;
            eGLStream stream_decl[4] = { GLStream_Position, GLStream_Colour, GLStream_Normal, GLStream_Diffuse };
            nlColour colour;
            colour.c[0] = 0xFF;
            colour.c[1] = 0xFF;
            colour.c[2] = 0xFF;
            colour.c[3] = 0xFF;
            m_Hemisphere.model = NULL;

            nlVector3* pPosition = m_Hemisphere.position;
            nlVector3* pNormal = m_Hemisphere.normal;
            nlVector2* pTexcoord = m_Hemisphere.texcoord;

            glSetDefaultState(true);
            glSetCurrentMatrix(glGetIdentityMatrix());
            glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
            glSetCurrentProgram(UnlitProgram);

            if (mesh.Begin(m_Hemisphere.vertCount, GLP_TriStrip, 4, stream_decl, true))
            {
                int index = 0;
                while (index < m_Hemisphere.vertCount)
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
                    m_Hemisphere.model = mesh.GetModel();
                }
            }
        }

        {
            extern unsigned long UnlitProgram;
            GLMeshWriter mesh;
            eGLStream stream_decl[4] = { GLStream_Position, GLStream_Colour, GLStream_Normal, GLStream_Diffuse };
            nlColour colour;
            colour.c[0] = 0xFF;
            colour.c[1] = 0xFF;
            colour.c[2] = 0xFF;
            colour.c[3] = 0xFF;
            m_FlatCylinderEnd.model = NULL;

            nlVector3* pPosition = m_FlatCylinderEnd.position;
            nlVector3* pNormal = m_FlatCylinderEnd.normal;
            nlVector2* pTexcoord = m_FlatCylinderEnd.texcoord;

            glSetDefaultState(true);
            glSetCurrentMatrix(glGetIdentityMatrix());
            glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
            glSetCurrentProgram(UnlitProgram);

            if (mesh.Begin(m_FlatCylinderEnd.vertCount, GLP_TriStrip, 4, stream_decl, true))
            {
                int index = 0;
                while (index < m_FlatCylinderEnd.vertCount)
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
                    m_FlatCylinderEnd.model = mesh.GetModel();
                }
            }
        }

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
