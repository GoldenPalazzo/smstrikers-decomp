#include "Game/Drawable/DrawableModel.h"
#include "Game/Debug/ShapeRender.h"
#include "Game/Render/RenderShadow.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glState.h"
#include "NL/gl/glAppAttach.h"
#include "NL/gl/glUserData.h"
#include "NL/nlString.h"
#include "Game/GameObjectLighting.h"
#include "Game/GL/GLInventory.h"
#include "Game/GL/GLVertexAnim.h"
#include "Game/Render/Jumbotron.h"
#include "Game/Render/CrowdManager.h"

bool g_bShadowVolumes = true;
bool g_bEnableDrawableSkinModel = true;
bool g_bSkinModelTextureLighting = true;
const u32 GLTT_BumpLocal_bit = 1 << (int)GLTT_BumpLocal;

static unsigned long UnlitProgram;
static unsigned long LitProgram;
static unsigned long LightTexture;
static unsigned long BlackTexture;
static unsigned long WhiteTexture;
static unsigned long UnlitCrowdProgram;
static unsigned long LitCrowdProgram;
static unsigned long BallModelID;

static float sfCoPlanarZ = 0.1f;
static float sfCoPlanar0Z = 0.041666668f;
static float sfPlanarShadowOpacity = 0.3f;

bool g_bEnableDrawableModel = true;
bool g_bLightDynamicObjects = true;
bool g_bDrawLitObjects = true;
bool g_bDrawSpecularObjects = true;
bool g_bCalculateFresnel = true;
bool g_bDrawObjectsWithPlanarShadows = true;
bool g_bBallGlow = true;
float g_fBallGlowH = 1.0f;
float g_fBallGlowR0 = 2.0f;
int g_nBallGlowA0 = 100;
int g_nBallGlowA1 = 20;
int g_nBallGlowRed = 255;
int g_nBallGlowGreen = 255;
int g_nBallGlowBlue = 255;
float g_fBallShadowH = 3.0f;
float g_fBallShadowR0 = 0.275f;
float g_fBallShadowR1 = 0.625f;
int g_nBallShadowA0 = 128;
int g_nBallShadowA1 = 72;
float g_fBallGlowR1;
unsigned char DrawableModel::sbBallShadowDisabled;
static nlAVLTreeSlotPool<unsigned long, AABBDimensions, DefaultKeyCompare<unsigned long> > boundingBoxCache;

extern "C"
{
    void __ct__12SlotPoolBaseFv(void*);
    void* __register_global_object(void* object, void* destructor, void* registration);
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8011FE0C | size: 0x3C
//  */
// void CleanBoundingBoxCache()
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x8011FE48 | size: 0x26C
//  */
// void RenderBoundingBox(const glModel*, const nlMatrix4&)
// {
// }

// /**
//  * Offset/Address/Size: 0x2A8 | 0x801200B4 | size: 0x8
//  */
// float GetCoPlanar0Z()
// {
//     return sfCoPlanar0Z;
// }

// /**
//  * Offset/Address/Size: 0x2B0 | 0x801200BC | size: 0x8
//  */
// void SetCoPlanarZ(float z)
// {
//     sfCoPlanarZ = z;
// }

// /**
//  * Offset/Address/Size: 0x2B8 | 0x801200C4 | size: 0x8
//  */
// float GetCoPlanarZ()
// {
//     return sfCoPlanarZ;
// }

// /**
//  * Offset/Address/Size: 0x2C0 | 0x801200CC | size: 0x8
//  */
// void SetPlanarShadowOpacity(float opacity)
// {
//     sfPlanarShadowOpacity = opacity;
// }

// /**
//  * Offset/Address/Size: 0x2C8 | 0x801200D4 | size: 0x8
//  */
// float GetPlanarShadowOpacity()
// {
//     return sfPlanarShadowOpacity;
// }

// /**
//  * Offset/Address/Size: 0x2D0 | 0x801200DC | size: 0x460
//  */
// void DrawPlanarShadow(const glModel*, const nlMatrix4&, float, bool, bool, bool, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x730 | 0x8012053C | size: 0x234
//  */
// void DrawCoPlanarReference(eGLView, const glModel&, const nlMatrix4&, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x964 | 0x80120770 | size: 0x290
//  */
// void GetShadowBoundingSquare(const glModel*, const nlMatrix4&, float&, float&, float&, float&, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBF4 | 0x80120A00 | size: 0x84
//  */
// void DrawableModel::DrawPlanarShadow()
// {
//     glModel* model = glModelDupNoStreams(m_pModel, true, false);
//     f32 opacity = sfPlanarShadowOpacity;
//     const nlMatrix4& worldMtx = GetWorldMatrix();
//     ::DrawPlanarShadow(model, worldMtx, opacity * m_translucency, true, false, true, (unsigned long)this);
// }

// /**
//  * Offset/Address/Size: 0xC78 | 0x80120A84 | size: 0x78
//  */
// void DrawableShadow::Draw()
// {
//     if (g_bShadowVolumes)
//     {
//         u32 mtx;
//         nlMatrix4& worldMtx = GetWorldMatrix();
//         mtx = glAllocMatrix();
//         if (mtx + 0x10000 != 0xFFFF)
//         {
//             glSetMatrix(mtx, worldMtx);
//         }
//         RenderShadowModel(2, m_pModel, mtx);
//     }
// }

// /**
//  * Offset/Address/Size: 0xCF0 | 0x80120AFC | size: 0x28
//  * TODO: Instruction scheduling differs - li r5,0 should come before lwz r3,0x9c(r3)
//  */
// void DrawableModel::GetAABBDimensions(AABBDimensions& dims, bool) const
// {
//     GetAABBDimensions(GetModel(), dims, 0);
// }

// /**
//  * Offset/Address/Size: 0xD18 | 0x80120B24 | size: 0x38C
//  */
// void GetAABBDimensions(const glModel*, AABBDimensions&, unsigned long)
// {
//     FORCE_DONT_INLINE;
// }

// /**
//  * Offset/Address/Size: 0x10A4 | 0x80120EB0 | size: 0x1C8
//  */
// DrawableObject* DrawableModel::Clone() const
// {
//     return NULL;
// }

// /**
//  * Offset/Address/Size: 0x126C | 0x80121078 | size: 0x38
//  */
// "art/effects/templates.fx"
// /**
//  * Offset/Address/Size: 0x12A4 | 0x801210B0 | size: 0x598
//  */
// void DrawableModel::DrawModel(const nlMatrix4&)
// {
// }

// /**
//  * Offset/Address/Size: 0x183C | 0x80121648 | size: 0x598
//  */
// void Fresnelify(glModelPacket*, eGLView)
// {
// }

// /**
//  * Offset/Address/Size: 0x1DD4 | 0x80121BE0 | size: 0x214
//  */
// void DrawBallShadow(const nlVector3&, const BallShadowParams&, bool)
// {
// }

// // /**
// //  * Offset/Address/Size: 0x1FE8 | 0x80121DF4 | size: 0x60
// //  */
// // DrawableModel::~DrawableModel()
// // {
// // }

// /**
//  * Offset/Address/Size: 0x0 | 0x80121E54 | size: 0x60
//  */
// DrawableShadow::~DrawableShadow()
// {
// }

/**
 * Offset/Address/Size: 0x60 | 0x80121EB4 | size: 0x60
 */
// void nlAVLTreeSlotPool<unsigned long, AABBDimensions, DefaultKeyCompare<unsigned long>>::~nlAVLTreeSlotPool()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80121F14 | size: 0x58
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::Clear()
// {
// }

/**
 * Offset/Address/Size: 0x58 | 0x80121F6C | size: 0x64
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::DestroyTree(void (AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AABBDimensions>*))
// {
// }

/**
 * Offset/Address/Size: 0xBC | 0x80121FD0 | size: 0x758
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::PostorderTraversal(AVLTreeEntry<unsigned long, AABBDimensions>*, void (AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AABBDimensions>*))
// {
// }

/**
 * Offset/Address/Size: 0x814 | 0x80122728 | size: 0x8
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::CastUp(AVLTreeNode*) const
// {
// }

/**
 * Offset/Address/Size: 0x81C | 0x80122730 | size: 0x10
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::DeleteEntry(AVLTreeEntry<unsigned long, AABBDimensions>*)
// {
// }

/**
 * Offset/Address/Size: 0x8B4 | 0x80122740 | size: 0x12C
 * TODO: 90.11% match - still differs in r30/r31 aliasing and relocation-backed
 *       symbol loads for vtable/destructor labels in the cache init block.
 */
extern "C" void __sinit_DrawableModel_cpp()
{
    UnlitProgram = glGetProgram("3d unlit");
    LitProgram = glGetProgram("3d pointlit");
    LightTexture = glGetTexture("global/lightramp");
    BlackTexture = glGetTexture("global/black");
    WhiteTexture = glGetTexture("global/white");
    UnlitCrowdProgram = glGetProgram("3d crowd");
    LitCrowdProgram = glGetProgram("3d crowd lit");

    unsigned long* cache = (unsigned long*)&boundingBoxCache;
    unsigned long* cache30 = cache;
    unsigned long* cache31 = cache;

    cache[0] = 0x802A4B5C;
    cache[0] = 0x802AECF0;

    __ct__12SlotPoolBaseFv((void*)(cache + 1));

    cache31[9] = 0;
    cache31[7] = 0;
    cache31[8] = 0;
    cache30[0] = 0x802AECDC;
    cache30[1] = 0x10;

    SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)(cache30 + 1), 0x34);

    cache30[2] = 0x10;

    __register_global_object((void*)cache30, (void*)0x80121EB4, (void*)0x80321F88);

    BallModelID = nlStringHash("gameplay/ball");
}

/**
 * Offset/Address/Size: 0x958 | 0x8012286C | size: 0x7C
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::~AVLTreeBase()
// {
// }

/**
 * Offset/Address/Size: 0x9D4 | 0x801228E8 | size: 0x2C
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

/**
 * Offset/Address/Size: 0xA00 | 0x80122914 | size: 0x2C
 */
// void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::CompareKey(void*, AVLTreeNode*)
// {
// }

/**
 * Offset/Address/Size: 0xA2C | 0x80122940 | size: 0xE0
 */
AVLTreeNode* AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions> >, DefaultKeyCompare<unsigned long> >::
    AllocateEntry(void* key, void* value)
{
    AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions> >, DefaultKeyCompare<unsigned long> >* self = this;
    unsigned long* keyPtr = (unsigned long*)key;
    AABBDimensions* valuePtr = (AABBDimensions*)value;
    AVLTreeEntry<unsigned long, AABBDimensions>* entry = 0;

    if (self->m_Allocator.m_FreeList == 0)
    {
        SlotPoolBase::BaseAddNewBlock(&self->m_Allocator, 0x34);
    }

    if (self->m_Allocator.m_FreeList != 0)
    {
        entry = (AVLTreeEntry<unsigned long, AABBDimensions>*)self->m_Allocator.m_FreeList;
        self->m_Allocator.m_FreeList = ((SlotPoolEntry*)entry)->m_next;
    }

    entry->node.left = 0;
    entry->node.right = 0;
    entry->node.heavy = 0;
    entry->key = *keyPtr;
    entry->value = *valuePtr;

    return (AVLTreeNode*)entry;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80122A20 | size: 0x8
//  */
// bool DrawableModel::IsDrawableModel()
// {
//     return true;
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x80122A28 | size: 0x4
//  */
// DrawableModel* DrawableModel::AsDrawableModel()
// {
//     return NULL;
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x80122A28 | size: 0x4
//  */
//  void DrawableModel::AsDrawableModel()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x0 | 0x80122A20 | size: 0x8
//   */
//  void DrawableModel::IsDrawableModel()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0xA2C | 0x80122940 | size: 0xE0
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::AllocateEntry(void*, void*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0xA00 | 0x80122914 | size: 0x2C
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::CompareKey(void*, AVLTreeNode*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x9D4 | 0x801228E8 | size: 0x2C
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x958 | 0x8012286C | size: 0x7C
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::~AVLTreeBase()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x81C | 0x80122730 | size: 0x10
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::DeleteEntry(AVLTreeEntry<unsigned long, AABBDimensions>*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x814 | 0x80122728 | size: 0x8
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::CastUp(AVLTreeNode*) const
//  {
//  }

//  /**
//   * Offset/Address/Size: 0xBC | 0x80121FD0 | size: 0x758
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::PostorderTraversal(AVLTreeEntry<unsigned long, AABBDimensions>*, void (AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AABBDimensions>*))
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x58 | 0x80121F6C | size: 0x64
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::DestroyTree(void (AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AABBDimensions>*))
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x0 | 0x80121F14 | size: 0x58
//   */
//  void AVLTreeBase<unsigned long, AABBDimensions, BasicSlotPool<AVLTreeEntry<unsigned long, AABBDimensions>>, DefaultKeyCompare<unsigned long>>::Clear()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x60 | 0x80121EB4 | size: 0x60
//   */
//  void nlAVLTreeSlotPool<unsigned long, AABBDimensions, DefaultKeyCompare<unsigned long>>::~nlAVLTreeSlotPool()
//  {
//  }

// /**
//  * Offset/Address/Size: 0x0 | 0x80121E54 | size: 0x60
//  */
// DrawableShadow::~DrawableShadow()
// {
// }

// /**
//  * Offset/Address/Size: 0x1FE8 | 0x80121DF4 | size: 0x60
//  */
// DrawableModel::~DrawableModel()
// {
// }

/**
 * Offset/Address/Size: 0x1DD4 | 0x80121BE0 | size: 0x214
 * TODO: 90.73% match - fRadius stays in f6 (target f7), causing coordinate
 *       block register mismatch (x/y loads and stfs order). Remaining drift
 *       starts from an early-hoisted lfs in prologue.
 */
void DrawBallShadow(const nlVector3& vPosition, const BallShadowParams& p, bool bGlow)
{
    f32 fT = vPosition.f.z / p.fReferenceHeight;
    if (fT < 0.0f)
    {
        fT = 0.0f;
    }
    if (fT > 1.0f)
    {
        fT = 1.0f;
    }

    f32 fRadius = (1.0f - fT) * p.fRadius0 + fT * p.fRadius1;
    f32 fAlpha = (1.0f - fT) * (f32)p.nAlpha0 + fT * (f32)p.nAlpha1;
    s32 nAlpha = (s32)fAlpha;
    if (nAlpha < 0)
    {
        nAlpha = 0;
    }
    if (nAlpha > 255)
    {
        nAlpha = 255;
    }

    nlColour colour = p.colour;
    f32 fY0 = vPosition.f.y - fRadius;
    f32 fX0 = vPosition.f.x - fRadius;
    f32 fY1 = vPosition.f.y + fRadius;
    f32 fX1 = vPosition.f.x + fRadius;

    glQuad3 quad;
    colour.c[3] = (u8)nAlpha;

    quad.m_pos[0].f.x = fX0;
    quad.m_pos[0].f.y = fY0;
    quad.m_pos[0].f.z = -0.5f;
    quad.m_pos[1].f.x = fX0;
    quad.m_pos[1].f.y = fY1;
    quad.m_pos[1].f.z = -0.5f;
    quad.m_pos[2].f.x = fX1;
    quad.m_pos[2].f.y = fY1;
    quad.m_pos[2].f.z = -0.5f;
    quad.m_pos[3].f.x = fX1;
    quad.m_pos[3].f.y = fY0;
    quad.m_pos[3].f.z = -0.5f;

    quad.m_uv[0].f.x = 1.0f;
    quad.m_uv[0].f.y = 1.0f;
    quad.m_uv[1].f.x = 0.0f;
    quad.m_uv[1].f.y = 1.0f;
    quad.m_uv[2].f.x = 0.0f;
    quad.m_uv[2].f.y = 0.0f;
    quad.m_uv[3].f.x = 1.0f;
    quad.m_uv[3].f.y = 0.0f;

    quad.m_colour[3] = colour;
    quad.m_colour[2] = colour;
    quad.m_colour[1] = colour;
    quad.m_colour[0] = colour;

    glSetDefaultState(true);

    glSetRasterState(GLS_AlphaBlend, bGlow ? 3 : 1);
    glSetRasterState(GLS_AlphaTest, 1);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetCurrentRasterState(glHandleizeRasterState());

    glSetCurrentTexture(glGetTexture(bGlow ? "global/ballshadowglow" : "global/ballshadow"), GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 3);
    glSetCurrentTextureState(glHandleizeTextureState());

    const glModel* pModel = quad.GetModel(true);
    void* pUserData = glAppGetNoFogUserData();
    glUserAttach(pUserData, pModel->packets, false);
    glViewAttachModel(GLV_Unshadowed, pModel);
}

static inline void* FindStream(glModelPacket* pPacket, int streamID)
{
    for (int i = 0; i < pPacket->numStreams; i++)
    {
        if (pPacket->streams[i].id == streamID)
        {
            return (void*)pPacket->streams[i].address;
        }
    }
    return NULL;
}

/**
 * Offset/Address/Size: 0x183C | 0x80121648 | size: 0x598
 * TODO: 99.75% match - 9 commutative operand swaps on mullw/add in display list vertex indexing
 */
void Fresnelify(glModelPacket* pPacket, eGLView view)
{
    extern void* dlGetStruct(unsigned long);

    nlMatrix4 viewMat;
    nlMatrix4 objectMat;
    nlMatrix4 modelview;

    u32 glossInt = (u8)glGetTextureState(pPacket->state.texturestate, GLTS_GlossLevel);
    f32 glossLevel = (f32)glossInt * (1.0f / 63.0f);
    if (glossLevel > 0.85f)
    {
        return;
    }

    s8* pNormals = (s8*)FindStream(pPacket, 1);
    nlColour* pColours = (nlColour*)FindStream(pPacket, 2);

    glViewGetViewMatrix(view, viewMat);
    glGetMatrix(pPacket->state.matrix, objectMat);
    nlMultMatrices(modelview, objectMat, viewMat);
    nlInvertMatrix(modelview, modelview);
    nlTransposeMatrix(modelview, modelview);

    void* pList = dlGetStruct(pPacket->indexBuffer);
    s32 index = 0;
    s32 numVerts = pPacket->numVertices;

    if (glossLevel > 0.5f)
    {
        while (index < numVerts)
        {
            u16* pVert;
            if (*(u16*)((u8*)pList + 0x0E) != 0)
            {
                u16 ns = *(u16*)((u8*)pList + 0x0C);
                int offset = index * ((ns - 1) * 2 + 1) + 4;
                pVert = (u16*)((u8*)*(u32*)((u8*)pList + 0x04) + offset);
            }
            else
            {
                u16 ns = *(u16*)((u8*)pList + 0x0C);
                int offset = index * (ns * 2) + 3;
                pVert = (u16*)((u8*)*(u32*)((u8*)pList + 0x04) + offset);
            }
            int vertIndex = *pVert;
            s8* pNormal = pNormals + vertIndex * 3;
            nlColour* pColour = pColours + vertIndex;
            nlVector3 normal = { 0 };
            normal.f.x = (f32)pNormal[0] * (1.0f / 64.0f);
            normal.f.y = (f32)pNormal[1] * (1.0f / 64.0f);
            normal.f.z = (f32)pNormal[2] * (1.0f / 64.0f);
            f32 dot = normal.f.x * modelview.m[0][2] + normal.f.y * modelview.m[1][2] + normal.f.z * modelview.m[2][2];
            if (dot < 0.0f)
            {
                dot = 0.0f;
            }
            f32 fresnel = 1.0f - dot;
            fresnel = fresnel * fresnel * 255.5f;
            pColour->c[3] = (u8)(s32)fresnel;
            index++;
        }
    }
    else if (glossLevel > 0.25f)
    {
        while (index < numVerts)
        {
            u16* pVert;
            if (*(u16*)((u8*)pList + 0x0E) != 0)
            {
                u16 ns = *(u16*)((u8*)pList + 0x0C);
                int offset = index * ((ns - 1) * 2 + 1) + 4;
                pVert = (u16*)((u8*)*(u32*)((u8*)pList + 0x04) + offset);
            }
            else
            {
                u16 ns = *(u16*)((u8*)pList + 0x0C);
                int offset = index * (ns * 2) + 3;
                pVert = (u16*)((u8*)*(u32*)((u8*)pList + 0x04) + offset);
            }
            int vertIndex = *pVert;
            s8* pNormal = pNormals + vertIndex * 3;
            nlColour* pColour = pColours + vertIndex;
            nlVector3 normal = { 0 };
            normal.f.x = (f32)pNormal[0] * (1.0f / 64.0f);
            normal.f.y = (f32)pNormal[1] * (1.0f / 64.0f);
            normal.f.z = (f32)pNormal[2] * (1.0f / 64.0f);
            f32 dot = normal.f.x * modelview.m[0][2] + normal.f.y * modelview.m[1][2] + normal.f.z * modelview.m[2][2];
            if (dot < 0.0f)
            {
                dot = 0.0f;
            }
            f32 fresnel = 1.0f - dot;
            fresnel *= fresnel * fresnel;
            pColour->c[3] = (u8)(s32)(255.5f * fresnel);
            index++;
        }
    }
    else
    {
        while (index < numVerts)
        {
            u16* pVert;
            if (*(u16*)((u8*)pList + 0x0E) != 0)
            {
                u16 ns = *(u16*)((u8*)pList + 0x0C);
                int offset = index * ((ns - 1) * 2 + 1) + 4;
                pVert = (u16*)((u8*)*(u32*)((u8*)pList + 0x04) + offset);
            }
            else
            {
                u16 ns = *(u16*)((u8*)pList + 0x0C);
                int offset = index * (ns * 2) + 3;
                pVert = (u16*)((u8*)*(u32*)((u8*)pList + 0x04) + offset);
            }
            int vertIndex = *pVert;
            s8* pNormal = pNormals + vertIndex * 3;
            nlColour* pColour = pColours + vertIndex;
            nlVector3 normal = { 0 };
            normal.f.x = (f32)pNormal[0] * (1.0f / 64.0f);
            normal.f.y = (f32)pNormal[1] * (1.0f / 64.0f);
            normal.f.z = (f32)pNormal[2] * (1.0f / 64.0f);
            f32 dot = normal.f.x * modelview.m[0][2] + normal.f.y * modelview.m[1][2] + normal.f.z * modelview.m[2][2];
            if (dot < 0.0f)
            {
                dot = 0.0f;
            }
            f32 fresnel = 1.0f - dot;
            fresnel *= fresnel * fresnel * fresnel;
            pColour->c[3] = (u8)(s32)(255.5f * fresnel);
            index++;
        }
    }
}

/**
 * Offset/Address/Size: 0x12A4 | 0x801210B0 | size: 0x598
 * TODO: 97.61% match - r30/r31 register swap for this/worldMatrix (scratch context),
 *       World offset diffs from missing m_pPlayerNISLightData in World.h,
 *       linker address mode diffs for glInventory/CrowdManager
 */
void DrawableModel::DrawModel(const nlMatrix4& worldMatrix)
{
    void* pLightData = NULL;
    void* pSpecularData = NULL;
    void* pEnvData = NULL;
    void* pTransData = NULL;
    void* pNoFogData = NULL;
    unsigned long LightTexture;
    unsigned char bJumbotron;
    unsigned char bCrowd;
    unsigned long litProgram;
    unsigned long unlitProgram;
    unsigned char bLight;
    unsigned char bSpec;
    glModel* newModel;

    if (!g_bEnableDrawableModel)
        return;

    if (!g_bDrawObjectsWithPlanarShadows && m_bRenderPlanarShadow)
        return;

    if (m_uObjectCreationFlags & 0x00100000)
    {
        pNoFogData = glAppGetNoFogUserData();
    }

    unsigned char bTransparent = (m_uObjectCreationFlags & 0x0000F000) != 0;
    if (bTransparent)
    {
        float f31 = m_translucency;
        if (f31 == 0.0f)
            return;
        if (f31 == 1.0f)
            bTransparent = 0;
        if (bTransparent)
        {
            pTransData = glUserAlloc(GLUD_Translucent, 4, false);
            float* pFloat = (float*)glUserGetData(pTransData);
            *pFloat = f31;
        }
    }

    eGLView view;
    if (m_uObjectFlags & 0x00000008)
    {
        view = (eGLView)0x16;
    }
    else
    {
        eGLView v = (eGLView)3;
        if (m_uObjectCreationFlags & 0x00000004)
            v = (eGLView)7;
        view = v;
        if (m_uObjectCreationFlags & 0x00000100)
            v = (eGLView)2;
        view = v;
        if (m_uObjectFlags & 0x00000040)
            v = (eGLView)0x13;
        view = v;
    }

    bCrowd = (m_uObjectCreationFlags >> 17) & 1;
    bJumbotron = (m_uObjectCreationFlags >> 16) & 1;

    if (bCrowd)
    {
        litProgram = LitCrowdProgram;
        unlitProgram = UnlitCrowdProgram;
    }
    else
    {
        litProgram = LitProgram;
        unlitProgram = UnlitProgram;
    }

    if (m_uObjectFlags & 0x00000100)
    {
        LightTexture = GetGameObjectLightRamp();
    }
    else
    {
        LightTexture = m_pWorldContext->m_LightRampTexA;
    }

    bLight = ((m_uObjectCreationFlags >> 7) & 1) ^ 1;
    if (bLight)
    {
        unsigned char temp = 1;
        if (m_uObjectFlags & 0x00000004)
            temp = g_bLightDynamicObjects;
        bLight = temp;
    }
    if (bLight)
    {
        if (!g_bDrawLitObjects)
            return;
    }

    bSpec = (m_uObjectCreationFlags >> 3) & 1;
    if (bSpec)
    {
        if (!g_bDrawSpecularObjects)
            return;
    }

    if (m_bVertexAnimated)
    {
        GLVertexAnim* pAnim = glInventory.GetVertexAnim(m_pModel->id);
        newModel = pAnim->GetModel(-1);
    }
    else
    {
        newModel = glModelDupNoStreams(m_pModel, true, false);
    }

    if (m_pModel->id == BallModelID && !sbBallShadowDisabled && worldMatrix.m[3][2] >= 0.0f)
    {
        BallShadowParams p;
        p.fReferenceHeight = g_fBallShadowH;
        p.fRadius0 = g_fBallShadowR0;
        p.fRadius1 = g_fBallShadowR1;
        p.nAlpha0 = g_nBallShadowA0;
        p.nAlpha1 = g_nBallShadowA1;
        p.colour.c[0] = 0xFF;
        p.colour.c[1] = 0xFF;
        p.colour.c[2] = 0xFF;
        p.colour.c[3] = 0xFF;
        DrawBallShadow(*(const nlVector3*)&worldMatrix.m[3][0], p, false);

        if (g_bBallGlow)
        {
            p.fReferenceHeight = g_fBallGlowH;
            p.fRadius0 = g_fBallGlowR0;
            p.fRadius1 = g_fBallGlowR1;
            p.nAlpha0 = g_nBallGlowA0;
            p.nAlpha1 = g_nBallGlowA1;
            p.colour.c[0] = g_nBallGlowRed;
            p.colour.c[1] = g_nBallGlowGreen;
            p.colour.c[2] = g_nBallGlowBlue;
            p.colour.c[3] = 0xFF;
            DrawBallShadow(*(const nlVector3*)&worldMatrix.m[3][0], p, true);
        }
    }

    if ((m_uObjectCreationFlags & 0x00000010) && g_bCalculateFresnel)
    {
        pEnvData = glUserAlloc(GLUD_EnvDiffuse, 0, false);
    }

    if (bLight)
    {
        if (m_uObjectFlags & 0x00000100)
        {
            pLightData = GetInGameLightData();
        }
        else
        {
            pLightData = m_pWorldContext->m_pIntensityPerm;
        }

        if (m_uObjectCreationFlags & 0x00000008)
        {
            pSpecularData = m_pWorldContext->m_pSTSIntensity;
        }
    }
    else
    {
        if (bSpec)
        {
            pSpecularData = m_pWorldContext->m_pSTSIntensity;
        }
    }

    unsigned long matHandle = glAllocMatrix();
    if (matHandle + 0x10000 != 0xFFFF)
    {
        glSetMatrix(matHandle, worldMatrix);
    }

    if (m_uObjectCreationFlags & 0x00200000)
    {
        for (glModelPacket* pPacket = newModel->packets; pPacket < newModel->packets + newModel->numPackets; pPacket++)
        {
            void* fresnelData = glAppGetOnePassFresnelUserData();
            glUserAttach(fresnelData, pPacket, false);
        }
    }

    for (glModelPacket* pPacket = newModel->packets; pPacket < newModel->packets + newModel->numPackets; pPacket++)
    {
        pPacket->state.matrix = matHandle;

        if (bJumbotron)
        {
            u32 tex = Jumbotron::instance.m_CurrentTexture;
            if (tex + 0x10000 != 0xFFFF)
            {
                pPacket->state.texture[0] = tex;
            }
        }

        if (bCrowd)
        {
            u32 tex = CrowdManager::instance.GetTextureHandle(newModel->id);
            if (tex + 0x10000 != 0xFFFF)
            {
                pPacket->state.texture[0] = tex;
            }
        }

        if (pNoFogData)
            glUserAttach(pNoFogData, pPacket, false);
        if (pTransData)
            glUserAttach(pTransData, pPacket, false);
        if (pEnvData)
        {
            glUserAttach(pEnvData, pPacket, false);
            Fresnelify(pPacket, view);
        }

        pPacket->state.program = unlitProgram;
        if (pLightData)
        {
            pPacket->state.program = litProgram;
            glUserAttach(pLightData, pPacket, false);

            if (!(pPacket->state.texconfig & 0x20))
            {
                pPacket->state.texture[5] = LightTexture;
                pPacket->state.texconfig |= GLTT_BumpLocal_bit;
            }

            if (pSpecularData && (pPacket->state.texconfig & 0x10))
            {
                void* specData = m_pWorldContext->GetCustomSpecularData(pPacket, false);
                glUserAttach(specData, pPacket, false);
            }
        }
        else
        {
            if (pSpecularData && (pPacket->state.texconfig & 0x10))
            {
                void* specData = m_pWorldContext->GetCustomSpecularData(pPacket, false);
                glUserAttach(specData, pPacket, false);
            }
        }
    }

    if (m_CB)
    {
        newModel = m_CB(newModel, view, m_uRenderLayer);
        if (!newModel)
            return;
    }

    glViewAttachModel(view, m_uRenderLayer, newModel);

    if (m_bRenderPlanarShadow)
    {
        DrawPlanarShadow();
    }
}

/**
 * Offset/Address/Size: 0x126C | 0x80121078 | size: 0x38
 */
void DrawableModel::Draw()
{
    DrawModel(GetWorldMatrix());
}

/**
 * Offset/Address/Size: 0x10A4 | 0x80120EB0 | size: 0x1C8
 * TODO: 59.34% match - inlined copy-constructor register allocation differs
 *       (target alternates r4/r5 loads and uses r0 for vtable addi destination),
 *       causing a wide instruction-order mismatch across the object field copy.
 */
DrawableObject* DrawableModel::Clone() const
{
    DrawableObject* pClone = new (nlMalloc(sizeof(DrawableModel), 8, false)) DrawableModel(*this);

    pClone->m_uObjectFlags |= 0x1;
    pClone->m_uObjectFlags |= 0x4;

    return pClone;
}

/**
 * Offset/Address/Size: 0xD18 | 0x80120B24 | size: 0x38C
 * TODO: 98.70% match - register allocation diff: dimensions r22 vs target r23,
 *       bbCache r30 vs target r28. Cascading register diffs and commutative
 *       operand order swaps in mullw/add.
 */
void GetAABBDimensions(const glModel* model, AABBDimensions& dimensions, unsigned long boundingBoxCacheKey)
{
    extern void* dlGetStruct(unsigned long);

    AABBDimensions* foundValue = NULL;
    AVLTreeEntry<unsigned long, AABBDimensions>* node = boundingBoxCache.m_Root;

    while (node != NULL)
    {
        int cmpResult;
        if (boundingBoxCacheKey == node->key)
        {
            cmpResult = 0;
        }
        else if (boundingBoxCacheKey < node->key)
        {
            cmpResult = -1;
        }
        else
        {
            cmpResult = 1;
        }

        if (cmpResult == 0)
        {
            if (&foundValue != NULL)
            {
                foundValue = &node->value;
            }
            break;
        }

        if (cmpResult < 0)
        {
            node = (AVLTreeEntry<unsigned long, AABBDimensions>*)node->node.left;
        }
        else
        {
            node = (AVLTreeEntry<unsigned long, AABBDimensions>*)node->node.right;
        }
    }

    if (foundValue != NULL)
    {
        dimensions = *foundValue;
        return;
    }

    u8* packets = (u8*)model->packets;
    unsigned int numPackets = model->numPackets;
    unsigned char first = 1;
    unsigned int packetIndex = 0;
    int packetOffset = 0;
    int vertexIndex;
    nlVector3 min;
    nlVector3 max;

    while (packetIndex < numPackets)
    {
        glModelPacket* packet = (glModelPacket*)(packets + packetOffset);
        void* list = dlGetStruct(packet->indexBuffer);
        vertexIndex = 0;

        while (vertexIndex < packet->numVertices)
        {
            u16* pVert;
            if (*(u16*)((u8*)list + 0x0E) != 0)
            {
                u16 ns = *(u16*)((u8*)list + 0x0C);
                int offset = ((ns - 1) * 2 + 1) * vertexIndex + 4;
                pVert = (u16*)((u8*)*(u32*)((u8*)list + 0x04) + offset);
            }
            else
            {
                u16 ns = *(u16*)((u8*)list + 0x0C);
                int offset = (ns * 2) * vertexIndex + 3;
                pVert = (u16*)((u8*)*(u32*)((u8*)list + 0x04) + offset);
            }

            glModelStream* stream = packet->streams;
            u16 vert = *pVert;
            u8 stride = stream->stride;
            nlVector3 point;

            if (stride == 12)
            {
                memcpy(&point, (u8*)stream->address + vert * stride, 12);
            }
            else
            {
                s16* src = (s16*)((u8*)stream->address + vert * stride);
                float scale = 0.01f;

                point.f.x = (float)src[0] * scale;
                point.f.y = (float)src[1] * scale;
                point.f.z = (float)src[2] * scale;
            }

            if (point.f.x < min.f.x || first)
            {
                min.f.x = point.f.x;
            }

            if (point.f.y < min.f.y || first)
            {
                min.f.y = point.f.y;
            }

            if (point.f.z < min.f.z || first)
            {
                min.f.z = point.f.z;
            }

            if (point.f.x > max.f.x || first)
            {
                max.f.x = point.f.x;
            }

            if (point.f.y > max.f.y || first)
            {
                max.f.y = point.f.y;
            }

            if (point.f.z > max.f.z || first)
            {
                max.f.z = point.f.z;
            }

            first = 0;
            vertexIndex++;
        }

        packetOffset += sizeof(glModelPacket);
        packetIndex++;
    }

    dimensions.mMin = min;
    dimensions.mMax = max;
    float dx = dimensions.mMax.f.x - dimensions.mMin.f.x;
    float dy = dimensions.mMax.f.y - dimensions.mMin.f.y;
    float dz = dimensions.mMax.f.z - dimensions.mMin.f.z;
    dimensions.mDim.f.x = dx;
    dimensions.mDim.f.y = dy;
    dimensions.mDim.f.z = dz;

    if (boundingBoxCacheKey != 0)
    {
        AVLTreeNode* existingNode;

        boundingBoxCache.AddAVLNode((AVLTreeNode**)&boundingBoxCache.m_Root, &boundingBoxCacheKey, &dimensions, &existingNode, boundingBoxCache.m_NumElements);

        if (existingNode == NULL)
        {
            boundingBoxCache.m_NumElements++;
        }
    }
}

/**
 * Offset/Address/Size: 0xCF0 | 0x80120AFC | size: 0x28
 * TODO: 94% match - instruction scheduling: target has li r5,0 before lwz r3,0x9c(r3),
 *       but compiler places lwz first. TU-level scheduling effect, not reproducible in scratch.
 */
void DrawableModel::GetAABBDimensions(AABBDimensions& dims, bool param) const
{
    ::GetAABBDimensions(m_pModel, dims, 0);
}

/**
 * Offset/Address/Size: 0xC78 | 0x80120A84 | size: 0x78
 */
void DrawableShadow::Draw()
{
    if (g_bShadowVolumes)
    {
        u32 mtx;
        nlMatrix4& worldMtx = GetWorldMatrix();
        mtx = glAllocMatrix();
        if (mtx + 0x10000 != 0xFFFF)
        {
            glSetMatrix(mtx, worldMtx);
        }
        RenderShadowModel(2, m_pModel, mtx);
    }
}

/**
 * Offset/Address/Size: 0xBF4 | 0x80120A00 | size: 0x84
 */
void DrawableModel::DrawPlanarShadow()
{
    glModel* model = glModelDupNoStreams(m_pModel, true, false);
    f32 opacity = sfPlanarShadowOpacity;
    const nlMatrix4& worldMtx = GetWorldMatrix();
    ::DrawPlanarShadow(model, worldMtx, opacity * m_translucency, true, false, true, (unsigned long)this);
}

/**
 * Offset/Address/Size: 0x964 | 0x80120770 | size: 0x290
 * TODO: 87.04% match - float register allocation/scheduling still differs in
 *       AABB corner initialization and projected matrix coefficient setup.
 */
void GetShadowBoundingSquare(const glModel* model, const nlMatrix4& matrix, float& x0, float& x1, float& y0, float& y1, unsigned long userData)
{
    extern World* s_World__12WorldManager;

    AABBDimensions dimensions;
    GetAABBDimensions(model, dimensions, userData);

    float zero = 0.0f;
    nlVector4 points[8];
    nlVector4* point = points;
    int i = 0;

    float minX = dimensions.mMin.f.x;
    float minY = dimensions.mMin.f.y;
    float minZ = dimensions.mMin.f.z;
    float maxZ = dimensions.mMax.f.z;
    float maxY = dimensions.mMax.f.y;
    float maxX = dimensions.mMax.f.x;

    points[0].f.x = minX;
    points[0].f.y = minY;
    float m13 = matrix.f.m13;
    points[0].f.z = minZ;
    float m11 = matrix.f.m11;
    points[0].f.w = zero;
    float m23 = matrix.f.m23;

    points[1].f.x = minX;
    float m21 = matrix.f.m21;
    points[1].f.y = minY;
    float m33 = matrix.f.m33;
    points[1].f.z = maxZ;
    float m31 = matrix.f.m31;
    points[1].f.w = zero;
    float m43 = matrix.f.m43;

    points[2].f.x = minX;
    float m41 = matrix.f.m41;
    points[2].f.y = maxY;
    float m12 = matrix.f.m12;
    points[2].f.z = minZ;
    float m22 = matrix.f.m22;
    points[2].f.w = zero;
    float m32 = matrix.f.m32;

    points[3].f.x = minX;
    points[3].f.y = maxY;
    float one = 1.0f;
    points[3].f.z = maxZ;
    points[3].f.w = zero;

    points[4].f.x = maxX;
    points[4].f.y = minY;
    points[4].f.z = minZ;
    points[4].f.w = zero;

    points[5].f.x = maxX;
    points[5].f.y = minY;
    points[5].f.z = maxZ;
    points[5].f.w = zero;

    points[6].f.x = maxX;
    points[6].f.y = maxY;
    points[6].f.z = minZ;
    points[6].f.w = zero;

    points[7].f.x = maxX;
    points[7].f.y = maxY;
    points[7].f.z = maxZ;
    points[7].f.w = zero;

    u32 lightPtr = *(u32*)((u8*)s_World__12WorldManager + 0x138);
    float lightX = *(float*)(lightPtr + 4);
    float lightY = *(float*)(lightPtr + 8);
    float lightZ = *(float*)(lightPtr + 0xC);

    lightX = -lightX;
    lightY = -lightY;

    nlMatrix4 projected;
    projected.f.m13 = one;
    float xOverZ = lightX / lightZ;
    projected.f.m23 = one;
    projected.f.m33 = one;
    projected.f.m43 = one;
    projected.f.m14 = one;
    projected.f.m24 = one;
    float yOverZ = lightY / lightZ;
    projected.f.m34 = one;
    projected.f.m44 = zero;

    projected.f.m13 = m11 + xOverZ * m13;
    projected.f.m23 = m21 + xOverZ * m23;
    projected.f.m33 = m31 + xOverZ * m33;
    projected.f.m43 = m41 + xOverZ * m43;
    projected.f.m14 = m12 + yOverZ * m13;
    projected.f.m24 = m22 + yOverZ * m23;
    projected.f.m34 = m32 + yOverZ * m33;
    projected.f.m44 = matrix.f.m42 + yOverZ * m43;

    for (; i < 8; i++, point++)
    {
        nlMultVectorMatrix(*point, *point, projected);

        if (i == 0 || point->f.x < x0)
        {
            x0 = point->f.x;
        }

        if (i == 0 || point->f.x > x1)
        {
            x1 = point->f.x;
        }

        if (i == 0 || point->f.y < y0)
        {
            y0 = point->f.y;
        }

        if (i == 0 || point->f.y > y1)
        {
            y1 = point->f.y;
        }
    }
}

/**
 * Offset/Address/Size: 0x730 | 0x8012053C | size: 0x234
 * TODO: 98.40% match - culling-state setup still schedules `li r3, 6` after
 *       the visibility branch instead of before `cmplwi`.
 */
void DrawCoPlanarReference(eGLView view, const glModel& model, const nlMatrix4& mtx, unsigned long userData)
{
    extern unsigned char g_bCoPlanarReferenceVis;
    extern unsigned long ResolvedWhiteTexture;

    float z;
    if (view == GLV_CoPlanar0)
    {
        z = sfCoPlanar0Z;
    }
    else
    {
        z = sfCoPlanarZ;
    }

    float x0;
    float x1;
    float y0;
    float y1;
    GetShadowBoundingSquare(&model, mtx, x0, x1, y0, y1, userData);

    nlVector3 points[4] = { };
    points[0].f.x = x0;
    points[0].f.y = y0;
    points[0].f.z = z;
    points[1].f.x = x1;
    points[1].f.y = y0;
    points[1].f.z = z;
    points[2].f.x = x1;
    points[2].f.y = y1;
    points[2].f.z = z;
    points[3].f.x = x0;
    points[3].f.y = y1;
    points[3].f.z = z;

    if (x0 + x1 < 0.0f)
    {
        points[0].f.x = x1;
        points[1].f.x = x0;
        points[2].f.x = x0;
        points[3].f.x = x1;
    }

    glSetDefaultState(false);

    eGLState cullState = GLS_Culling;
    unsigned long cullMode = 3;
    if (g_bCoPlanarReferenceVis)
    {
        cullMode = 0;
    }

    glSetRasterState(cullState, cullMode);
    glSetRasterState(GLS_DepthTest, 0);
    glSetRasterState(GLS_DepthWrite, 0);

    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTexture(ResolvedWhiteTexture, GLTT_Diffuse);

    glQuad3 quad;
    quad.m_pos[0] = points[0];
    quad.m_uv[0].f.x = 0.0f;
    quad.m_uv[0].f.y = 0.0f;

    quad.m_pos[1] = points[1];
    quad.m_uv[1].f.x = 0.0f;
    quad.m_uv[1].f.y = 0.0f;

    quad.m_pos[2] = points[2];
    quad.m_uv[2].f.x = 0.0f;
    quad.m_uv[2].f.y = 0.0f;

    quad.m_pos[3] = points[3];
    quad.m_uv[3].f.x = 0.0f;
    quad.m_uv[3].f.y = 0.0f;

    quad.SetColour(0xAA, 0xAA, 0xAA, 0xFF);
    quad.Attach(view, 0, true);
}

/**
 * Offset/Address/Size: 0x2D0 | 0x801200DC | size: 0x460
 * TODO: 79.67% match - GPR register allocation shifted by 2 (model=r23 vs r25,
 *       worldMatrix=r24 vs r26, ignorePacketMatrices=r29 vs r31), causing:
 *       1) f29 callee-saved (frame 0x1B0 vs 0x1A0) - named float temps needed
 *          for correct fmadds/fdivs scheduling trigger 3rd callee-saved FPR
 *       2) interleaved 2-at-a-time struct copy vs bulk 16-at-once (too few free GPRs)
 *       3) clrlwi. (combined mask+test) inside loop vs hoisted clrlwi + cmplwi
 */
static inline void ComputeShadowMtx(nlMatrix4& dst, const nlMatrix4& src, u32 lightPtr)
{
    float m13 = src.f.m13;
    float m11 = src.f.m11;
    float lightX = *(float*)(lightPtr + 4);
    float lightY = *(float*)(lightPtr + 8);
    float xRatioNeg = -lightX;
    float lightZ = *(float*)(lightPtr + 0xC);
    float yRatioNeg = -lightY;
    float zero = 0.0f;
    float one = 1.0f;
    float xOverZ = xRatioNeg / lightZ;
    float m23 = src.f.m23;
    float m21 = src.f.m21;
    float m33 = src.f.m33;
    float m31 = src.f.m31;
    float m43 = src.f.m43;
    float m41 = src.f.m41;
    float m12 = src.f.m12;
    float yOverZ = yRatioNeg / lightZ;
    float m22 = src.f.m22;
    float m32 = src.f.m32;
    float m42 = src.f.m42;

    dst.f.m11 = xOverZ * m13 + m11;
    dst.f.m21 = xOverZ * m23 + m21;
    dst.f.m13 = zero;
    dst.f.m23 = zero;
    dst.f.m31 = xOverZ * m33 + m31;
    dst.f.m41 = xOverZ * m43 + m41;
    dst.f.m12 = yOverZ * m13 + m12;
    dst.f.m22 = yOverZ * m23 + m22;
    dst.f.m32 = yOverZ * m33 + m32;
    dst.f.m42 = yOverZ * m43 + m42;
    dst.f.m33 = zero;
    dst.f.m43 = zero;
    dst.f.m14 = zero;
    dst.f.m24 = zero;
    dst.f.m34 = zero;
    dst.f.m44 = one;
}

void DrawPlanarShadow(const glModel* model, const nlMatrix4& worldMatrix, float shadowTranslucency, bool ignorePacketMatrices, bool isModelPosed, bool bFieldOnlyShadow, unsigned long boundingBoxCacheKey)
{
    extern unsigned char g_bDrawBoundingBoxes;
    extern unsigned char g_bDrawPlanarShadows;
    extern unsigned char g_bCoPlanarPerObject;
    extern unsigned char g_bCoPlanarIgnoreIdentity;
    extern unsigned long ResolvedBlackTexture;
    extern World* s_World__12WorldManager;

    nlMatrix4 packetMatrix;
    nlMatrix4 packetShadowMatrix;
    nlMatrix4 transformedPacketMatrix;
    nlMatrix4 packetMat;
    nlMatrix4 mat;
    glModelPacket* pPacketEnd;
    glModelPacket* pPacket;
    void* pTransData;
    unsigned long program;

    if (g_bDrawBoundingBoxes)
    {
        RenderBoundingBox(model, worldMatrix);
    }

    if (!g_bDrawPlanarShadows)
    {
        return;
    }

    eGLView view = GLV_CoPlanar;
    if (bFieldOnlyShadow)
    {
        view = GLV_CoPlanar0;
    }

    pTransData = glUserAlloc(GLUD_Translucent, 4, false);
    *(float*)glUserGetData(pTransData) = shadowTranslucency;

    if (g_bCoPlanarPerObject)
    {
        if (g_bCoPlanarIgnoreIdentity && worldMatrix.f.m41 == 0.0f && worldMatrix.f.m42 == 0.0f && worldMatrix.f.m43 == 0.0f)
        {
            return;
        }

        if (ignorePacketMatrices)
        {
            mat = worldMatrix;
        }
        else
        {
            glGetMatrix(model->packets->state.matrix, packetMat);
            nlMultMatrices(mat, worldMatrix, packetMat);
        }

        DrawCoPlanarReference(view, *model, mat, boundingBoxCacheKey);
    }

    pPacket = model->packets;
    pPacketEnd = pPacket + model->numPackets;
    program = UnlitProgram;

    while (pPacket < pPacketEnd)
    {
        if (!ignorePacketMatrices)
        {
            glGetMatrix(pPacket->state.matrix, packetMatrix);

            if (isModelPosed)
            {
                transformedPacketMatrix = packetMatrix;
            }
            else
            {
                nlMultMatrices(transformedPacketMatrix, worldMatrix, packetMatrix);
            }

            ComputeShadowMtx(packetShadowMatrix, transformedPacketMatrix, *(u32*)((u8*)s_World__12WorldManager + 0x138));
        }
        else
        {
            ComputeShadowMtx(packetShadowMatrix, worldMatrix, *(u32*)((u8*)s_World__12WorldManager + 0x138));
        }

        unsigned long shadowMatrix = glAllocMatrix();
        if (shadowMatrix + 0x10000 != 0xFFFF)
        {
            glSetMatrix(shadowMatrix, packetShadowMatrix);
        }

        pPacket->state.matrix = shadowMatrix;
        pPacket->state.texture[0] = ResolvedBlackTexture;
        pPacket->state.program = program;

        glUserAttach(pTransData, pPacket, false);
        glUserDetach(GLUD_Light, pPacket);
        glSetRasterState(pPacket->state.raster, GLS_AlphaBlend, 1);

        pPacket++;
    }

    glViewAttachModel(view, model);
}

/**
 * Offset/Address/Size: 0x2C8 | 0x801200D4 | size: 0x8
 */
float GetPlanarShadowOpacity()
{
    return sfPlanarShadowOpacity;
}

/**
 * Offset/Address/Size: 0x2C0 | 0x801200CC | size: 0x8
 */
void SetPlanarShadowOpacity(float opacity)
{
    sfPlanarShadowOpacity = opacity;
}

/**
 * Offset/Address/Size: 0x2B8 | 0x801200C4 | size: 0x8
 */
float GetCoPlanarZ()
{
    return sfCoPlanarZ;
}

/**
 * Offset/Address/Size: 0x2B0 | 0x801200BC | size: 0x8
 */
void SetCoPlanarZ(float z)
{
    sfCoPlanarZ = z;
}

/**
 * Offset/Address/Size: 0x2A8 | 0x801200B4 | size: 0x8
 */
float GetCoPlanar0Z()
{
    return sfCoPlanar0Z;
}

/**
 * Offset/Address/Size: 0x3C | 0x8011FE48 | size: 0x26C
 * TODO: 99.35% match - float register allocation cycle (f4/f6/f2/f3) in AABB corner setup and remaining literal pool symbol mismatch for colour load
 */
void RenderBoundingBox(const glModel* model, const nlMatrix4& matrix)
{
    AABBDimensions dimensions;
    GetAABBDimensions(model, dimensions, 0);

    nlVector4 points[8];

    nlVector4* p1 = &points[1];
    nlVector4* p2 = &points[2];
    nlVector4* p3 = &points[3];
    nlVector4* p5;
    nlVector4* p4;
    p4 = &points[4];
    p5 = &points[5];
    nlVector4* p6 = &points[6];
    nlVector4* p7 = &points[7];
    nlVector4* p0 = &points[0];
    int i = 0;

    nlVec4Set(points[0], dimensions.mMin.f.x, dimensions.mMin.f.y, dimensions.mMin.f.z, 1.0f);
    nlVec4Set(*p1, dimensions.mMin.f.x, dimensions.mMin.f.y, dimensions.mMax.f.z, 1.0f);
    nlVec4Set(*p2, dimensions.mMin.f.x, dimensions.mMax.f.y, dimensions.mMax.f.z, 1.0f);
    nlVec4Set(*p3, dimensions.mMin.f.x, dimensions.mMax.f.y, dimensions.mMin.f.z, 1.0f);
    nlVec4Set(*p4, dimensions.mMax.f.x, dimensions.mMin.f.y, dimensions.mMin.f.z, 1.0f);
    nlVec4Set(*p5, dimensions.mMax.f.x, dimensions.mMin.f.y, dimensions.mMax.f.z, 1.0f);
    nlVec4Set(*p6, dimensions.mMax.f.x, dimensions.mMax.f.y, dimensions.mMax.f.z, 1.0f);
    nlVec4Set(*p7, dimensions.mMax.f.x, dimensions.mMax.f.y, dimensions.mMin.f.z, 1.0f);

    for (; i < 8; i++, p0++)
    {
        nlMultVectorMatrix(*p0, *p0, matrix);
    }

    const nlColour colour = { 0xFF, 0xFF, 0xFF, 0xFF };

    g_ShapeRenderer.DrawLine3D((nlVector3&)points[0], (nlVector3&)*p1, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p1, (nlVector3&)*p2, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p2, (nlVector3&)*p3, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p3, (nlVector3&)points[0], colour, true);

    g_ShapeRenderer.DrawLine3D((nlVector3&)*p4, (nlVector3&)*p5, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p5, (nlVector3&)*p6, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p6, (nlVector3&)*p7, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p7, (nlVector3&)*p4, colour, true);

    g_ShapeRenderer.DrawLine3D((nlVector3&)*p4, (nlVector3&)points[0], colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p5, (nlVector3&)*p1, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p6, (nlVector3&)*p2, colour, true);
    g_ShapeRenderer.DrawLine3D((nlVector3&)*p7, (nlVector3&)*p3, colour, true);
}

/**
 * Offset/Address/Size: 0x0 | 0x8011FE0C | size: 0x3C
 */
void CleanBoundingBoxCache()
{
    boundingBoxCache.Clear();
    SlotPoolBase::BaseFreeBlocks(&boundingBoxCache.m_Allocator, sizeof(AVLTreeEntry<unsigned long, AABBDimensions>));
}
