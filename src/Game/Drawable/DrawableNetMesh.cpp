#include "Game/Drawable/DrawableNetMesh.h"
#include "Game/Render/ShootToScoreArrow.h"
#include "Game/Replay.h"
#include "Game/Field.h"
#include "Game/Net.h"
#include "Game/World.h"
#include "NL/gl/gl.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"

static unsigned long UnlitProgram = glGetProgram("3d unlit");
static unsigned long LitProgram = glGetProgram("3d pointlit");
static unsigned long LightTexture = glGetTexture("global/lightramp");
static unsigned long BlackTexture = glGetTexture("global/black");
static unsigned long WhiteTexture = glGetTexture("global/white");
static unsigned long NetMeshTexture = glGetTexture("global/netmesh");
static unsigned long CheckerTexture = glGetTexture("global/checker");
static int siInvisiblePlaneAlpha;
/**
 * Offset/Address/Size: 0xC4C | 0x80114BA8 | size: 0x24
 */
DrawableNetMesh::DrawableNetMesh(bool isPositiveXNet)
{
    miNetIndex = isPositiveXNet ? 0 : 1;
    mNumQuads = 0;
    mNumVertices = 0;
    mbInitialized = false;
}

/**
 * Offset/Address/Size: 0xB80 | 0x80114ADC | size: 0xCC
 */
DrawableNetMesh::~DrawableNetMesh()
{
    if (mbInitialized)
    {
        delete[] mpPosition;
    }

    if (sbStaticInitialized[miNetIndex])
    {
        delete[] spTexcoord[miNetIndex];
        delete[] spTriIndices[miNetIndex];
        delete[] spColour[miNetIndex];
        sbStaticInitialized[miNetIndex] = false;
    }

    mNumQuads = 0;
    mNumVertices = 0;
    mbInitialized = false;
}

/**
 * Offset/Address/Size: 0x91C | 0x80114878 | size: 0x264
 * TODO: 99.61% match - netHeight and half-height still use swapped floating-point registers.
 */
void DrawableNetMesh::RenderInvisiblePlanes() const
{
    float goalLineX = cField::GetGoalLineX(1U);
    float netWidth = cNet::m_fNetWidth;
    float netHeight = cNet::m_fNetHeight;

    glSetDefaultState(true);
    glSetRasterState(GLS_DepthWrite, 1);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_Culling, 0);
    glSetCurrentRasterState(glHandleizeRasterState());

    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTextureState(glHandleizeTextureState());

    nlMatrix4 matrix;
    nlMakeRotationMatrixY(matrix, 1.5707964f);

    float netPlaneX;

    nlColour c = { 0xFF, 0xFF, 0xFF, 0x00 };
    c.c[3] = (u8)siInvisiblePlaneAlpha;

    glQuad3 quad;

    netPlaneX = goalLineX - 0.05f;
    matrix.m[3][0] = netPlaneX;
    matrix.m[3][1] = 0.0f;
    matrix.m[3][2] = 0.5f * netHeight;
    matrix.m[3][3] = 1.0f;
    quad.SetupRotatedRectangle(netHeight, netWidth, matrix, false, false);
    quad.SetColour(c);
    glAttachQuad3(GLV_InvisiblePlane, 1, &quad, true);

    netPlaneX = 0.05f + goalLineX;
    matrix.m[3][0] = netPlaneX;
    matrix.m[3][1] = 0.0f;
    matrix.m[3][2] = 0.5f * netHeight;
    matrix.m[3][3] = 1.0f;
    quad.SetupRotatedRectangle(netHeight, netWidth, matrix, false, false);
    quad.SetColour(c);
    glAttachQuad3(GLV_InvisiblePlane, 1, &quad, true);

    netPlaneX = -goalLineX - 0.05f;
    matrix.m[3][0] = netPlaneX;
    matrix.m[3][1] = 0.0f;
    matrix.m[3][2] = 0.5f * netHeight;
    matrix.m[3][3] = 1.0f;
    quad.SetupRotatedRectangle(netHeight, netWidth, matrix, false, false);
    quad.SetColour(c);
    glAttachQuad3(GLV_InvisiblePlane, 1, &quad, true);

    netPlaneX = 0.05f - goalLineX;
    matrix.m[3][0] = netPlaneX;
    matrix.m[3][1] = 0.0f;
    matrix.m[3][2] = 0.5f * netHeight;
    matrix.m[3][3] = 1.0f;
    quad.SetupRotatedRectangle(netHeight, netWidth, matrix, false, false);
    quad.SetColour(c);
    glAttachQuad3(GLV_InvisiblePlane, 1, &quad, true);

    glSetDefaultState(false);
}

/**
 * Offset/Address/Size: 0x4B8 | 0x80114414 | size: 0x464
 * TODO: 77.01% match - early guard branch polarity and stream-table/vtable
 *       setup ordering still differ from target.
 */
enum eGLPrimitive
{
    GLP_TriList = 0,
    GLP_TriStrip = 1,
    GLP_TriFan = 2,
    GLP_QuadList = 3,
    GLP_LineList = 4,
    GLP_LineStrip = 5,
    GLP_Num = 6,
};

enum eGLStream
{
    GLStream_Position = 0,
    GLStream_Colour = 2,
    GLStream_Diffuse = 3,
};

enum eGLMemory
{
    GLM_Header = 0,
    GLM_Matrix = 1,
    GLM_IndexData = 2,
    GLM_VertexData = 3,
    GLM_TextureData = 4,
    GLM_Target = 5,
    GLM_Num = 6,
};

class GLMeshWriterCore
{
public:
    GLMeshWriterCore();
    ~GLMeshWriterCore();

    virtual bool Begin(int, eGLPrimitive, int, const eGLStream*, bool);
    virtual bool End();
    virtual void Colour(const nlColour&);
    virtual void ColourPlat(unsigned long);
    virtual void Normal(const nlVector3&) = 0;
    virtual void Texcoord(const nlVector2&);
    virtual void Vertex(const nlVector3&);
    virtual void Vertex(const nlVector4&);

    glModel* GetModel();

    glModel* pModel;
    glModelStream stream[15];
    int currentIndex;
    int maximumVerts;
    int elementCount;
};

class GLMeshWriter : public GLMeshWriterCore
{
public:
    GLMeshWriter()
        : GLMeshWriterCore()
    {
    }

    virtual bool End();
    virtual void Normal(const nlVector3&);
    virtual void Texcoord(const nlVector2&);
    void Texcoord(short, short);
};

struct DisplayList;

void* glFrameAlloc(unsigned long, eGLMemory);
DisplayList* dlMakeDisplayList(const glModelPacket*, bool);
void nlZeroMemory(void*, unsigned long);
void DCFlushRange(void*, unsigned long);

static const int gl_stream_stride[15] = {
    12, 3, 4, 4, 4, 4, 4, 4, 4, 12, 12, 12, 1, 16, 16
};

/**
 * Offset/Address/Size: 0x3DC | 0x80114414 | size: 0x464
 * TODO: 93.96% match - vtable setup ordering after GLMeshWriterCore
 * construction, stream stride load sequence, and loop index/register
 * allocation differences remain.
 */
void DrawableNetMesh::Render() const
{
    extern unsigned char sbRenderAnimatedNetMesh;
    extern unsigned char sbAccelerateMeshWriter;
    extern unsigned char sbCopyVertices;
    extern unsigned char sbUseCheckerTexture;
    extern unsigned char sbUseDisplayLists;

    if (!sbRenderAnimatedNetMesh || !mbInitialized || !NetMesh::s_bAnimatedNetMeshEnabled)
    {
        return;
    }

    if (World::sbIsHyperShootToScoreRenderingEnabled)
    {
        int netIndex = miNetIndex;
        if ((netIndex == 1 && World::sbShowPositiveXNetDuringHyperStrike) || (netIndex == 0 && !World::sbShowPositiveXNetDuringHyperStrike))
        {
            return;
        }
    }

    eGLStream streamDecl[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
    GLMeshWriter meshWriter;
    nlVector3* pPosition = mpPosition;
    shortVector2* pTexcoord = spTexcoord[miNetIndex];

    glSetDefaultState(true);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_AlphaTest, 1);
    glSetRasterState(GLS_DepthTest, 1);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentMatrix(glGetIdentityMatrix());

    unsigned long texture = NetMesh::sNetTextureHandle;
    if (sbUseCheckerTexture)
    {
        texture = CheckerTexture;
    }
    glSetCurrentTexture(texture, GLTT_Diffuse);
    glSetCurrentProgram(UnlitProgram);

    unsigned short* pTriIndices = spTriIndices[miNetIndex];

    if (sbAccelerateMeshWriter)
    {
        glModel* pModel = (glModel*)glFrameAlloc(0x10, GLM_Header);
        nlZeroMemory(pModel, 0x10);
        glModelPacket* pPacket = (glModelPacket*)glFrameAlloc(0x4A, GLM_Header);
        nlZeroMemory(pPacket, 0x4A);
        glModelStream* pStreams = (glModelStream*)glFrameAlloc(0x12, GLM_Header);
        nlZeroMemory(pStreams, 0x12);

        pModel->id = (u32)-1;
        pModel->numPackets = 1;
        pModel->packets = pPacket;
        pPacket->indexBuffer = (u32)pTriIndices;
        pPacket->numStreams = 3;
        pPacket->numVertices = (u16)m_unk18;
        pPacket->primType = 1;
        pPacket->streams = pStreams;
        glStateSave(pPacket->state);

        if (sbCopyVertices)
        {
            unsigned long nBytes = (unsigned long)mJolt * (unsigned long)sizeof(nlVector3);
            nlVector3* pNewPosition = (nlVector3*)glFrameAlloc(nBytes, GLM_VertexData);
            memcpy(pNewPosition, pPosition, nBytes);
            pPosition = pNewPosition;
        }
        if (sbUseDisplayLists)
        {
            if ((u32)((DrawableNetMesh*)this)->mNumVertices == 0)
            {
                ((DrawableNetMesh*)this)->mNumQuads = pPacket->indexBuffer;
                ((DrawableNetMesh*)this)->mNumVertices = (int)dlMakeDisplayList(pPacket, true);
            }
            pPacket->indexBuffer = (u32)((DrawableNetMesh*)this)->mNumVertices;
        }
        DCFlushRange(pPosition, (unsigned long)mJolt * (unsigned long)sizeof(nlVector3));

        pStreams[0].id = GLStream_Position;
        pStreams[0].address = (u32)pPosition;
        pStreams[0].stride = (u8)gl_stream_stride[0];
        pStreams[1].id = GLStream_Colour;
        pStreams[1].address = (u32)spColour[miNetIndex];
        pStreams[1].stride = (u8)gl_stream_stride[2];
        pStreams[2].id = GLStream_Diffuse;
        pStreams[2].address = (u32)pTexcoord;
        pStreams[2].stride = (u8)gl_stream_stride[3];

        void* pUserDataHandle = glUserAlloc(GLUD_ConstantColour, 4, false);
        u8* pColourData = (u8*)glUserGetData(pUserDataHandle);
        WorldDarkening& wd = WorldDarkening::Instance();
        float darkScale = 255.0f;
        float darkBase = 1.0f - wd.mPos;
        u8 dark = (u8)(int)(darkScale * darkBase);
        pColourData[0] = dark;
        pColourData[1] = dark;
        pColourData[2] = dark;
        pColourData[3] = dark;
        glUserAttach(pUserDataHandle, pPacket, false);
        glViewAttachModel(GLV_UnsortedPerspective, pModel);
    }
    else
    {
        if (meshWriter.Begin(m_unk18, GLP_TriStrip, 3, streamDecl, false))
        {
            int i;
            unsigned short* pIndex = pTriIndices;
            for (i = 0; i < ((const volatile DrawableNetMesh*)this)->m_unk18; pIndex++, i++)
            {
                unsigned short index = *pIndex;
                float darkPos = WorldDarkening::Instance().mPos;
                u8 dark = (u8)(int)((1.0f - darkPos) * 255.0);
                shortVector2* pUV = &pTexcoord[index];
                meshWriter.Texcoord(pUV->e[0], pUV->e[1]);
                nlColour c;
                c.c[0] = dark;
                c.c[1] = dark;
                c.c[2] = dark;
                c.c[3] = 0xFF;
                ((GLMeshWriterCore*)&meshWriter)->Colour(c);
                meshWriter.Vertex(pPosition[index]);
            }
            if (!meshWriter.End())
            {
                return;
            }
            glViewAttachModel(GLV_UnsortedPerspective, meshWriter.GetModel());
        }
    }
    RenderInvisiblePlanes();
}

/**
 * Offset/Address/Size: 0x2C0 | 0x8011421C | size: 0x1F8
 */
void DrawableNetMesh::Grab(NetMesh& netMesh)
{
    mpNetMesh = &netMesh;

    if (!netMesh.mbInitialized)
        return;

    if (!mbInitialized)
    {
        int numTriIdx = netMesh.m_NumTriStripIndices;
        mJolt = netMesh.m_NumParticles;
        m_unk18 = numTriIdx;

        int numVerts = mJolt;
        int numIndices = m_unk18;

        mpPosition = (nlVector3*)nlMalloc(numVerts * sizeof(nlVector3), 8, false);

        if (!sbStaticInitialized[miNetIndex])
        {
            spTriIndices[miNetIndex] = (unsigned short*)nlMalloc(numIndices * 2, 8, false);

            int allocSize = numVerts * 4;

            spTexcoord[miNetIndex] = (shortVector2*)nlMalloc(allocSize, 8, false);
            spColour[miNetIndex] = (unsigned long*)nlMalloc(allocSize, 8, false);

            memset(spColour[miNetIndex], 0xFF, allocSize);

            sbStaticInitialized[miNetIndex] = true;
            sNumVertices[miNetIndex] = numVerts;
        }

        mbInitialized = true;
        mJoltCache = 0.0f;
    }

    unsigned short* pTriIndices = spTriIndices[miNetIndex];
    shortVector2* pTexcoord = spTexcoord[miNetIndex];

    for (int i = 0; i < netMesh.m_NumTriStripIndices; i++)
    {
        *pTriIndices++ = netMesh.m_TriStripIndices[i];
    }

    {
        shortVector2* pDst = pTexcoord;
        for (int i = 0; i < netMesh.m_NumParticles; i++)
        {
            mpPosition[i] = netMesh.m_v3Position[i];
            *pDst++ = netMesh.m_v2TextureCoords[i];
        }
    }
}

/**
 * Offset/Address/Size: 0xAC | 0x80114008 | size: 0x214
 * TODO: 88.68% scratch - init loads lhs fields in swapped order; blend loops
 *       still use different pointer/index registers, and the 0.0f/1.0f
 *       literal labels differ.
 */
void DrawableNetMesh::Blend(float blendFactor, const DrawableNetMesh& lhs, const DrawableNetMesh& rhs)
{
    nlVector3* pDst;
    nlVector3* pSrc;

    if (!lhs.mbInitialized || !rhs.mbInitialized)
        return;

    if (!mbInitialized)
    {
        int numTriIdx = lhs.m_unk18;
        mJolt = lhs.mJolt;
        m_unk18 = numTriIdx;

        int numVerts = mJolt;
        int numIndices = m_unk18;

        mpPosition = (nlVector3*)nlMalloc(numVerts * sizeof(nlVector3), 8, false);

        if (!sbStaticInitialized[miNetIndex])
        {
            spTriIndices[miNetIndex] = (unsigned short*)nlMalloc(numIndices * 2, 8, false);

            int allocSize = numVerts * 4;

            spTexcoord[miNetIndex] = (shortVector2*)nlMalloc(allocSize, 8, false);
            spColour[miNetIndex] = (unsigned long*)nlMalloc(allocSize, 8, false);

            memset(spColour[miNetIndex], 0xFF, allocSize);

            sbStaticInitialized[miNetIndex] = true;
            sNumVertices[miNetIndex] = numVerts;
        }

        mbInitialized = true;
        mJoltCache = 0.0f;
    }

    float oneMinusBlend = 1.0f - blendFactor;

    for (int i = 0, offset = 0; i < mJolt; i++, offset += sizeof(nlVector3))
    {
        pSrc = (nlVector3*)((char*)lhs.mpPosition + offset);
        pDst = (nlVector3*)((char*)mpPosition + offset);
        float x = oneMinusBlend * pSrc->f.x;
        float y = pSrc->f.y;
        float z = pSrc->f.z;
        y = oneMinusBlend * y;
        z = oneMinusBlend * z;
        pDst->f.x = x;
        pDst->f.y = y;
        pDst->f.z = z;
    }

    for (int i = 0; i < mJolt; i++)
    {
        pDst = &mpPosition[i];
        pSrc = &rhs.mpPosition[i];
        float x = pDst->f.x + blendFactor * pSrc->f.x;
        float z = pDst->f.z + blendFactor * pSrc->f.z;
        float y = pDst->f.y + blendFactor * pSrc->f.y;
        pDst->f.x = x;
        pDst->f.y = y;
        pDst->f.z = z;
    }
}

/**
 * Offset/Address/Size: 0x38 | 0x80113F94 | size: 0x74
 */
void DrawableNetMesh::Replay(LoadFrame& frame)
{
    float joltValue = 0.0f;
    Replayable<0, LoadFrame, float>(frame, joltValue);

    if (joltValue != mJoltCache)
    {
        mJoltCache = joltValue;
        if (mpNetMesh != nullptr)
        {
            if (mJoltCache > 0.0f)
            {
                mpNetMesh->JoltNet(mJoltCache);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80113F5C | size: 0x38
 */
void DrawableNetMesh::Replay(SaveFrame& frame)
{
    mJoltCache = mpNetMesh->mJolt;
    Replayable<0, SaveFrame, float>(frame, mJoltCache);
}

/**
 * Offset/Address/Size: 0x0 | 0x80114BCC | size: 0x44
 */
#pragma dont_inline on
template <>
void Replayable<0, LoadFrame, float>(LoadFrame& frame, float& value)
{
    memcpy(&value, frame.mStream.mStorage, sizeof(float));
    frame.mStream.mStorage += sizeof(float);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x44 | 0x80114C10 | size: 0x40
 */
#pragma dont_inline on
template <>
void Replayable<0, SaveFrame, float>(SaveFrame& frame, float& value)
{
    memcpy(frame.mStream.mStorage, &value, sizeof(float));
    frame.mStream.mStorage += sizeof(float);
}
#pragma dont_inline reset
