#include "NL/gl/glDraw2.h"

#include "NL/nlMath.h"

#include "NL/gl/gl.h"
#include "NL/gl/glState.h"
#include "NL/gl/glView.h"
#include "NL/gl/glStruct.h"
#include "NL/gl/glMatrix.h"

#include "NL/gl/glModel.h"
#include "NL/gl/glUserData.h"

#include "Game/GL/gluMeshWriter.h"

static int StripMap[4] = { 3, 0, 2, 1 };
static int QuadMap[4] = { 0, 1, 2, 3 };
static int TriListMap[6] = { 3, 0, 2, 0, 2, 1 };

static unsigned long _defaultProgram = glGetProgram("2d unlit");

/**
 * Offset/Address/Size: 0x610 | 0x801D7C10 | size: 0x210
 */

inline void GLMeshWriterCore::Position(const nlVector3& v)
{
    Vertex(v);
}

bool glPoly2::Attach(eGLView view, int layer, unsigned long* pMatrixHandle, unsigned long programHandle)
{
    eGLStream streamsDesc[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
    GLMeshWriter writer;

    if ((programHandle + 0x10000) == 0xFFFF)
    {
        programHandle = _defaultProgram;
    }

    unsigned long program;
    unsigned long matrix;
    unsigned long texconfig;

    texconfig = gl_GetCurrentStateBundle()->texconfig;
    program = glSetCurrentProgram(programHandle);
    matrix = pMatrixHandle ? *pMatrixHandle : glGetIdentityMatrix();
    matrix = glSetCurrentMatrix(matrix);

    eGLPrimitive prim;
    int* pMap;
    if (glHasQuads())
    {
        prim = GLP_QuadList;
        pMap = QuadMap;
    }
    else
    {
        prim = GLP_TriStrip;
        pMap = StripMap;
    }

    if (writer.Begin(4, prim, texconfig + 2, streamsDesc, false))
    {
        for (int i = 0; i < 4; i++)
        {
            int index = pMap[i];
            writer.Colour(m_colour[index]);
            if (texconfig != 0)
            {
                writer.Texcoord(m_uv[index]);
            }
            float fx = m_pos[index].f.x;
            float fy = m_pos[index].f.y;
            float fz = depth;
            nlVector3 pos;
            pos.f.x = fx;
            pos.f.y = fy;
            pos.f.z = fz;
            writer.Position(pos);
        }
        if (!writer.End())
        {
            return false;
        }

        glViewAttachModel(view, layer, writer.GetModel());
    }
    else
    {
        return false;
    }

    glSetCurrentProgram(program);
    glSetCurrentMatrix(matrix);

    return true;
}

/**
 * Offset/Address/Size: 0x33C | 0x801D793C | size: 0x2D4
 */
bool glAttachPoly2(eGLView view, unsigned long count, glPoly2* polys, unsigned long* pMatrix, const void* user)
{
    // TODO: Remaining mismatch is in fast-path register allocation and Begin-failure cleanup branch placement.
    if (gl_GetCurrentStateBundle()->texconfig == 1)
    {
        eGLStream streamsDesc[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
        GLMeshWriter writer;

        int* map;
        int vertsPerPoly;
        eGLPrimitive prim;
        if (glHasQuads())
        {
            map = QuadMap;
            vertsPerPoly = 4;
            prim = GLP_QuadList;
        }
        else
        {
            map = TriListMap;
            vertsPerPoly = 6;
            prim = GLP_TriList;
        }

        unsigned long oldProg = glSetCurrentProgram(_defaultProgram);
        unsigned long oldMat = glSetCurrentMatrix(pMatrix ? *pMatrix : glGetIdentityMatrix());

        if (writer.Begin((int)(vertsPerPoly * count), prim, 3, streamsDesc, false))
        {
            glPoly2* pPoly = polys;
            unsigned long i = 0;
            while (i < count)
            {
                int* pMap = map;
                int j = 0;
                while (j < vertsPerPoly)
                {
                    int index = *pMap;
                    writer.Colour(pPoly->m_colour[index]);
                    writer.Texcoord(pPoly->m_uv[index]);

                    nlVector3 pos;
                    float y = pPoly->m_pos[index].f.y;
                    float z = pPoly->depth;
                    float x = pPoly->m_pos[index].f.x;
                    pos.f.x = x;
                    pos.f.y = y;
                    pos.f.z = z;
                    writer.Position(pos);

                    pMap++;
                    j++;
                }

                i++;
                pPoly++;
            }

            if (!writer.End())
            {
                return false;
            }

            if (user == NULL)
            {
                glViewAttachModel(view, writer.GetModel());
            }
            else
            {
                glModel* pDup = glModelDupNoStreams(writer.GetModel(), true, false);
                glModelPacket* pPacket = pDup->packets;
                while (pPacket < (glModelPacket*)((unsigned char*)pDup->packets + pDup->numPackets * sizeof(glModelPacket)))
                {
                    glUserAttach(user, pPacket, false);
                    pPacket = (glModelPacket*)((unsigned char*)pPacket + sizeof(glModelPacket));
                }
                glViewAttachModel(view, pDup);
            }

            glSetCurrentProgram(oldProg);
            glSetCurrentMatrix(oldMat);
            return true;
        }

        return false;
    }

    glPoly2* pPoly = polys;
    unsigned long i = 0;
    while (i < count)
    {
        if (!pPoly->Attach(view, 0, 0, 0xFFFFFFFFu))
        {
            return false;
        }
        i++;
        pPoly++;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x250 | 0x801D7850 | size: 0xEC
 */
void glPoly2::FullCoverage(const nlColour& col, float z)
{
    const float width = glGetOrthographicWidth();
    const float height = glGetOrthographicHeight();
    gl_ScreenInfo* screenInfo = glGetScreenInfo();
    const float border = screenInfo->PixelCentre;

    nlVec2Set(m_pos[0], -border, -border);
    nlVec2Set(m_pos[1], -border, height - border);
    nlVec2Set(m_pos[2], width - border, height - border);
    nlVec2Set(m_pos[3], width - border, -border);

    depth = z;

    nlVec2Set(m_uv[0], 0.0f, 0.0f);
    nlVec2Set(m_uv[1], 0.0f, 1.0f);
    nlVec2Set(m_uv[2], 1.0f, 1.0f);
    nlVec2Set(m_uv[3], 1.0f, 0.0f);

    m_colour[0] = col;
    m_colour[1] = col;
    m_colour[2] = col;
    m_colour[3] = col;
}

/**
 * Offset/Address/Size: 0x1EC | 0x801D77EC | size: 0x64
 */
void glPoly2::SetupRectangle(float x, float y, float w, float h, float z)
{
    f32 bottom_y;
    f32 right_x;

    nlVec2Set(m_pos[0], x, y);
    nlVec2Set(m_pos[1], x, y + h);
    nlVec2Set(m_pos[2], x + w, y + h);
    nlVec2Set(m_pos[3], x + w, y);

    nlVec2Set(m_uv[0], 0.0f, 0.0f);
    nlVec2Set(m_uv[1], 0.0f, 1.0f);
    nlVec2Set(m_uv[2], 1.0f, 1.0f);
    nlVec2Set(m_uv[3], 1.0f, 0.0f);

    if (z != 1e10f)
    {
        depth = z;
    }
}

/**
 * Offset/Address/Size: 0x18 | 0x801D7618 | size: 0x1D4
 */
void glPoly2::SetupRotatedRectangle(float cx, float cy, float w, float h, float angle, float z)
{
    nlVector2 v;
    nlMatrix3 m;
    float f3 = 0.5f;
    f32 temp_f26;
    f32 temp_f25;
    f32 temp_f31;
    f32 temp_f29;

    nlMakeRotationMatrixZ(m, angle);

    temp_f26 = -w * f3;
    temp_f31 = -h * f3;
    nlVec2Set(v, temp_f26, temp_f31);
    nlMultVectorMatrix(v, v, m);
    nlVec2Set(m_pos[0], v.f.x + cx, v.f.y + cy);

    temp_f25 = h * f3;
    nlVec2Set(v, temp_f26, temp_f25);
    nlMultVectorMatrix(v, v, m);
    nlVec2Set(m_pos[1], v.f.x + cx, v.f.y + cy);

    temp_f29 = w * f3;
    nlVec2Set(v, temp_f29, temp_f25);
    nlMultVectorMatrix(v, v, m);
    nlVec2Set(m_pos[2], v.f.x + cx, v.f.y + cy);

    nlVec2Set(v, temp_f29, temp_f31);
    nlMultVectorMatrix(v, v, m);
    nlVec2Set(m_pos[3], v.f.x + cx, v.f.y + cy);

    nlVec2Set(m_uv[0], 0.0f, 0.0f);
    nlVec2Set(m_uv[1], 0.0f, 1.0f);
    nlVec2Set(m_uv[2], 1.0f, 1.0f);
    nlVec2Set(m_uv[3], 1.0f, 0.0f);

    if (z != 1e10f)
    {
        depth = z;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801D7600 | size: 0x18
 */
void glPoly2::SetColour(const nlColour& col)
{
    m_colour[0] = col;
    m_colour[1] = col;
    m_colour[2] = col;
    m_colour[3] = col;
}
