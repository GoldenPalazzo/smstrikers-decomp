#include "Game/FE/feRender.h"
#include "Game/FE/tlInstance.h"
#include "Game/FE/tlImageInstance.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/FE/feScene.h"
#include "Game/FE/fePackage.h"
#include "Game/FE/fePresentation.h"
#include "Game/FE/feImage.h"
#include "Game/FE/feTextureResource.h"
#include "NL/gl/glMatrixStack.h"
#include "NL/gl/glState.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/gluMeshWriter.h"
#include "NL/nlMemory.h"
#include "NL/nlColour.h"
#include "NL/platvmath.h"
#include "NL/gl/gl.h"

GLMatrixStack* FERender::m_pMatrixStack = nullptr;
FEScene* FERender::m_pRenderScene = nullptr;

static nlFloatColour s_currentAssetColour;
static unsigned long drawQuadProgram;
static unsigned long grabTex;

inline void ConvertFloatColourToColour(nlColour& out, const nlFloatColour& in)
{
    out.c[0] = in.c[0] * 255.0f;
    out.c[1] = in.c[1] * 255.0f;
    out.c[2] = in.c[2] * 255.0f;
    out.c[3] = in.c[3] * 255.0f;
}

inline void ConvertFloatColourToColour_(nlColour& out, const float r, const float g, const float b, const float a)
{
    out.c[0] = r;
    out.c[1] = g;
    out.c[2] = b;
    out.c[3] = a;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020A288 | size: 0x3BC
 * TODO: 95.73% match - 3-way register cycle (texconfig r28/r29, program r29/r31, pMap r31/r28), Begin-fail destructor inlined instead of separate block, 4 cosmetic static var index diffs
 */
inline void GLMeshWriterCore::Position(const nlVector3& v)
{
    Vertex(v);
}

/**
 * Offset/Address/Size: 0x0 | 0x8020A288 | size: 0x3BC
 * TODO: 99.58% match - r28/r29/r30 cyclic register swap for texconfig/pMap/matrixHandle.
 * MWCC allocator heuristic, not controllable from source.
 */
unsigned char FERender::RenderImageInstance(const TLImageInstance* pTLImageInstance)
{
    nlColour colour = pTLImageInstance->GetAssetColour();

    const FEImage* pFEImage = (const FEImage*)pTLImageInstance->m_component;
    FETextureResource* pTexRes = pFEImage->m_pFeTextureResource;

    if (!pTexRes->m_bValid)
        return 1;

    ConvertFloatColourToColour(colour, s_currentAssetColour);

    unsigned long textureHandle = pTexRes->m_glTextureHandle;
    if (!textureHandle)
        return 1;

    nlMatrix4 matTM;
    m_pMatrixStack->GetTop(matTM);
    nlMultMatrices(matTM, matTM, m_pRenderScene->m_matView);

    unsigned long matrixHandle = glAllocMatrix();
    if (matrixHandle != 0xFFFFFFFF)
        glSetMatrix(matrixHandle, matTM);

    nlVector2 pos[4];
    nlVector2 uv[4];

    uv[0].e[0] = 0.0f;
    uv[0].e[1] = 0.0f;
    uv[1].e[0] = 0.0f;
    uv[1].e[1] = 1.0f;
    uv[2].e[0] = 1.0f;
    uv[2].e[1] = 1.0f;
    uv[3].e[0] = 1.0f;
    uv[3].e[1] = 0.0f;

    pos[0].e[0] = -0.5f;
    pos[0].e[1] = 0.5f;
    pos[1].e[0] = -0.5f;
    pos[1].e[1] = -0.5f;
    pos[2].e[0] = 0.5f;
    pos[2].e[1] = -0.5f;
    pos[3].e[0] = 0.5f;
    pos[3].e[1] = 0.5f;

    glSetDefaultState(false);

    static signed char init;
    static unsigned char bAlpha;
    if (!init)
    {
        bAlpha = 1;
        init = 1;
    }

    glSetRasterState(GLS_Culling, 0);

    if (textureHandle != grabTex && bAlpha)
    {
        glSetRasterState(GLS_AlphaBlend, 1);
        glSetRasterState(GLS_AlphaTest, 1);
        glSetRasterState(GLS_AlphaTestRef, 0);
    }

    glSetTextureState(GLTS_DiffuseWrap, 3);

    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTextureState(glHandleizeTextureState());

    glSetCurrentTexture(textureHandle, GLTT_Diffuse);

    eGLStream streams[] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };

    GLMeshWriter meshWriter;

    u8 texconfig = gl_GetCurrentStateBundle()->texconfig;

    unsigned long program = glSetCurrentProgram(drawQuadProgram);
    matrixHandle = glSetCurrentMatrix(matrixHandle);

    static int stripmap[4];
    static int quadmap[4];

    int* pMap;
    eGLPrimitive prim;
    if (glHasQuads())
    {
        pMap = quadmap;
        prim = GLP_QuadList;
    }
    else
    {
        pMap = stripmap;
        prim = GLP_TriStrip;
    }

    if (meshWriter.Begin(4, prim, texconfig + 2, streams, false))
    {
        for (int i = 0; i < 4; i++)
        {
            int index = pMap[i];
            meshWriter.Colour(colour);
            if (texconfig)
                meshWriter.Texcoord(uv[index]);
            nlVector3 vertex;
            vertex.f.x = pos[index].e[0];
            vertex.f.y = pos[index].e[1];
            vertex.f.z = 0.0f;
            meshWriter.Position(vertex);
        }

        if (!meshWriter.End())
            return 0;

        eGLView view = (eGLView)m_pRenderScene->m_uRenderView;
        glViewAttachModel(view, 0, meshWriter.GetModel());
    }
    else
    {
        return 0;
    }

    glSetCurrentProgram(program);
    glSetCurrentMatrix(matrixHandle);

    return 1;
}

/**
 * Offset/Address/Size: 0x3BC | 0x8020A644 | size: 0xD0
 * TODO: 98.85% match - 1 instruction scheduling diff: lfs f1,8(r4) scheduled before fmuls f3 instead of after. Compiler pipeline heuristic, not controllable from source.
 */
void FERender::RenderTextInstance(TLTextInstance* textInstance)
{
    nlMatrix4 combinedMatrix;

    m_pMatrixStack->GetTop(combinedMatrix);

    nlMultMatrices(combinedMatrix, combinedMatrix, m_pRenderScene->m_matView);

    textInstance->m_DrawInfo.pMatrix = &combinedMatrix;

    nlColour colour;
    ConvertColour(colour, s_currentAssetColour);

    textInstance->Render((eGLView)(m_pRenderScene->m_uRenderView), colour);
}

/**
 * Offset/Address/Size: 0x48C | 0x8020A714 | size: 0x64
 */
void FERender::RenderScene(FEScene* scene)
{
    if (scene == nullptr)
    {
        return;
    }

    m_pRenderScene = scene;
    m_pMatrixStack->LoadIdentity();

    s_currentAssetColour.c[0] = 1.0f;
    s_currentAssetColour.c[1] = 1.0f;
    s_currentAssetColour.c[2] = 1.0f;
    s_currentAssetColour.c[3] = 1.0f;

    FEPresentation* presentation = scene->m_pFEPackage->GetPresentation();
    RenderPresentation(presentation);

    m_pRenderScene = nullptr;
}

/**
 * Offset/Address/Size: 0x4F0 | 0x8020A778 | size: 0x38
 */
void FERender::RenderPresentation(const FEPresentation* presentation)
{
    if (presentation == nullptr)
    {
        return;
    }

    if (presentation->m_slides == nullptr)
    {
        return;
    }

    RenderSlide(presentation->m_currentSlide);
}

/**
 * Offset/Address/Size: 0x528 | 0x8020A7B0 | size: 0x42C
 * TODO: 94% match - MWCC inline budget causes grandchild operator= to be inlined
 * (5 extra instructions) instead of calling bl __as__13nlFloatColourFRC13nlFloatColour.
 * Register allocation cascade and stack offset diffs all trace to this root cause.
 * Also need IsVisible and GetLibRefObject body stripping in scratch context.
 */
void FERender::RenderComponentInstance(TLComponentInstance* componentInstance)
{
    TLComponent* component = ((TLInstance*)componentInstance)->m_component;
    if (component == 0)
    {
        return;
    }

    TLSlide* activeSlide = component->m_pActiveSlide;
    if (activeSlide == 0)
    {
        return;
    }

    if (activeSlide == 0)
    {
        return;
    }

    if (activeSlide->m_instances == 0)
    {
        return;
    }

    TLInstance* instance = activeSlide->m_instances->m_next;

    while (true)
    {
        float time = activeSlide->m_time;
        nlFloatColour oldSlideColour = s_currentAssetColour;
        TLInstance* nextInstance = instance->m_next;

        if (instance->IsValidAtTime(time) && instance->m_bVisible)
        {
            nlMatrix4 rotationMatrix;
            nlMatrix4 scaleMatrix;
            nlMatrix4 combinedMatrix;

            const feVector3& rotZ = instance->GetRotation();
            const feVector3& rotY = instance->GetRotation();
            nlMakeRotationMatrixEulerAngles(
                rotationMatrix,
                instance->GetRotation().f.x,
                rotY.f.y,
                rotZ.f.z);

            const feVector3& scaleZ = instance->GetScale();
            const feVector3& scaleY = instance->GetScale();
            nlMakeScaleMatrix(
                scaleMatrix,
                instance->GetScale().f.x,
                scaleY.f.y,
                scaleZ.f.z);

            nlMultMatrices(combinedMatrix, scaleMatrix, rotationMatrix);

            nlVector3 v3Pos;
            instance->GetPosition().GetNLVector3(v3Pos);
            combinedMatrix.SetTranslation(v3Pos);
            combinedMatrix.f.m43 *= -1.0f;

            m_pMatrixStack->PushMatrix();
            m_pMatrixStack->MultMatrix(combinedMatrix);

            nlFloatColour* curAssetColour = &s_currentAssetColour;
            for (u32 i = 0; i < 4; i++)
            {
                curAssetColour->c[i] = (instance->GetColour().c[i] * curAssetColour->c[i]) / 255.0f;
            }

            switch (instance->m_type)
            {
            case TLAT_IMAGE:
                RenderImageInstance((const TLImageInstance*)instance);
                break;
            case TLAT_TEXT:
            {
                nlMatrix4 textMatrix;
                m_pMatrixStack->GetTop(textMatrix);
                nlMultMatrices(textMatrix, textMatrix, m_pRenderScene->GetCameraMatrix());
                ((TLTextInstance*)instance)->SetMatrix(&textMatrix);

                nlColour colour;
                ConvertColour(colour, s_currentAssetColour);

                ((TLTextInstance*)instance)->Render((eGLView)m_pRenderScene->GetRenderView(), colour);
                break;
            }
            case TLAT_COMPONENT:
            {
                TLComponent* componentRef = (TLComponent*)instance->GetLibRefObject();
                if (componentRef != 0)
                {
                    if (componentRef->GetActiveSlide() != 0)
                    {
                        RenderSlide(componentRef->GetActiveSlide());
                    }
                }
                break;
            }
            default:
                break;
            }

            if (instance->pChildren != 0)
            {
                TLInstance* child = instance->pChildren->m_next;

                while (true)
                {
                    nlFloatColour oldChildColour = s_currentAssetColour;
                    TLInstance* nextChild = child->m_next;

                    if (child->IsValidAtTime(time) && child->IsVisible())
                    {
                        PushTransformMatrix(child);
                        CalculateCurrentAssetColour(child);

                        switch (child->GetType())
                        {
                        case TLAT_IMAGE:
                            RenderImageInstance((const TLImageInstance*)child);
                            break;
                        case TLAT_TEXT:
                            RenderTextInstance((TLTextInstance*)child);
                            break;
                        case TLAT_COMPONENT:
                            RenderComponentInstance((TLComponentInstance*)child);
                            break;
                        default:
                            break;
                        }

                        if (child->pChildren != 0)
                        {
                            TLInstance* grandchild = child->pChildren->m_next;

                            while (true)
                            {
                                TLInstance* nextGrandchild = grandchild->m_next;
                                nlFloatColour oldGrandchildColour = s_currentAssetColour;

                                RenderTimeLineAsset(grandchild, time);

                                s_currentAssetColour = oldGrandchildColour;

                                if (grandchild == child->pChildren)
                                {
                                    break;
                                }

                                grandchild = nextGrandchild;
                            }
                        }

                        PopTransformMatrix();
                    }

                    s_currentAssetColour = oldChildColour;

                    if (child == instance->pChildren)
                    {
                        break;
                    }

                    child = nextChild;
                }
            }

            m_pMatrixStack->PopMatrix();
        }

        s_currentAssetColour = oldSlideColour;

        if (instance == activeSlide->m_instances)
        {
            break;
        }

        instance = nextInstance;
    }
}

/**
 * Offset/Address/Size: 0x978 | 0x8020AC00 | size: 0x418
 * TODO: 98.47% match - MWCC register allocation: instance r30 vs target r28,
 * child r28 vs target r29, grandchild r23/r22 swapped. Stack layout also differs
 * (v3Pos at 0x2c vs target 0x1c, oldChildColour at 0x1c vs target 0x28).
 * Root cause: MWCC inline budget causes mixed operator= inlining (grandchild bl,
 * child/outer inline) which can't be reproduced without manual 4-word copies.
 */
void FERender::RenderSlide(const TLSlide* slide)
{
    if (slide == nullptr)
    {
        return;
    }

    if (slide->m_instances == nullptr)
    {
        return;
    }

    TLInstance* instance = slide->m_instances->m_next;

    while (true)
    {
        float time = slide->m_time;
        nlFloatColour oldSlideColour = s_currentAssetColour;
        TLInstance* nextInstance = instance->m_next;

        if (instance->IsValidAtTime(time) && instance->m_bVisible)
        {
            nlMatrix4 rotationMatrix;
            nlMatrix4 scaleMatrix;
            nlMatrix4 combinedMatrix;

            const feVector3& rotZ = instance->GetRotation();
            const feVector3& rotY = instance->GetRotation();
            nlMakeRotationMatrixEulerAngles(
                rotationMatrix,
                instance->GetRotation().f.x,
                rotY.f.y,
                rotZ.f.z);

            const feVector3& scaleZ = instance->GetScale();
            const feVector3& scaleY = instance->GetScale();
            nlMakeScaleMatrix(
                scaleMatrix,
                instance->GetScale().f.x,
                scaleY.f.y,
                scaleZ.f.z);

            nlMultMatrices(combinedMatrix, scaleMatrix, rotationMatrix);

            nlVector3 v3Pos;
            instance->GetPosition().GetNLVector3(v3Pos);
            combinedMatrix.SetTranslation(v3Pos);
            combinedMatrix.f.m43 *= -1.0f;

            m_pMatrixStack->PushMatrix();
            m_pMatrixStack->MultMatrix(combinedMatrix);

            nlFloatColour* curAssetColour = &s_currentAssetColour;
            for (u32 i = 0; i < 4; i++)
            {
                curAssetColour->c[i] = (instance->GetColour().c[i] * curAssetColour->c[i]) / 255.0f;
            }

            switch (instance->m_type)
            {
            case TLAT_IMAGE:
                RenderImageInstance((const TLImageInstance*)instance);
                break;
            case TLAT_TEXT:
            {
                nlMatrix4 textMatrix;
                m_pMatrixStack->GetTop(textMatrix);
                nlMultMatrices(textMatrix, textMatrix, m_pRenderScene->GetCameraMatrix());
                ((TLTextInstance*)instance)->SetMatrix(&textMatrix);

                nlColour colour;
                ConvertColour(colour, s_currentAssetColour);

                ((TLTextInstance*)instance)->Render((eGLView)m_pRenderScene->GetRenderView(), colour);
                break;
            }
            case TLAT_COMPONENT:
            {
                TLComponent* componentRef = (TLComponent*)instance->GetLibRefObject();
                if (componentRef != 0)
                {
                    if (componentRef->GetActiveSlide() != 0)
                    {
                        RenderSlide(componentRef->GetActiveSlide());
                    }
                }
                break;
            }
            default:
                break;
            }

            if (instance->pChildren != 0)
            {
                TLInstance* child = instance->pChildren->m_next;

                while (true)
                {
                    nlFloatColour oldChildColour = s_currentAssetColour;
                    TLInstance* nextChild = child->m_next;

                    if (child->IsValidAtTime(time) && child->IsVisible())
                    {
                        PushTransformMatrix(child);
                        CalculateCurrentAssetColour(child);

                        switch (child->GetType())
                        {
                        case TLAT_IMAGE:
                            RenderImageInstance((const TLImageInstance*)child);
                            break;
                        case TLAT_TEXT:
                            RenderTextInstance((TLTextInstance*)child);
                            break;
                        case TLAT_COMPONENT:
                            RenderComponentInstance((TLComponentInstance*)child);
                            break;
                        default:
                            break;
                        }

                        if (child->pChildren != 0)
                        {
                            TLInstance* grandchild = child->pChildren->m_next;

                            while (true)
                            {
                                TLInstance* nextGrandchild = grandchild->m_next;
                                nlFloatColour oldGrandchildColour = s_currentAssetColour;

                                RenderTimeLineAsset(grandchild, time);

                                s_currentAssetColour = oldGrandchildColour;

                                if (grandchild == child->pChildren)
                                {
                                    break;
                                }

                                grandchild = nextGrandchild;
                            }
                        }

                        PopTransformMatrix();
                    }

                    *(u32*)&s_currentAssetColour.c[0] = *(u32*)&oldChildColour.c[0];
                    *(u32*)&s_currentAssetColour.c[1] = *(u32*)&oldChildColour.c[1];
                    *(u32*)&s_currentAssetColour.c[2] = *(u32*)&oldChildColour.c[2];
                    *(u32*)&s_currentAssetColour.c[3] = *(u32*)&oldChildColour.c[3];

                    if (child == instance->pChildren)
                    {
                        break;
                    }

                    child = nextChild;
                }
            }

            m_pMatrixStack->PopMatrix();
        }

        *(u32*)&s_currentAssetColour.c[0] = *(u32*)&oldSlideColour.c[0];
        *(u32*)&s_currentAssetColour.c[1] = *(u32*)&oldSlideColour.c[1];
        *(u32*)&s_currentAssetColour.c[2] = *(u32*)&oldSlideColour.c[2];
        *(u32*)&s_currentAssetColour.c[3] = *(u32*)&oldSlideColour.c[3];

        if (instance == slide->m_instances)
        {
            break;
        }

        instance = nextInstance;
    }
}

/**
 * Offset/Address/Size: 0xD90 | 0x8020B018 | size: 0x7A8
 */
void FERender::RenderTimeLineAsset(TLInstance*, float)
{
}

/**
 * Offset/Address/Size: 0x1538 | 0x8020B7C0 | size: 0x24
 */
void FERender::PopTransformMatrix()
{
    m_pMatrixStack->PopMatrix();
}

/**
 * Offset/Address/Size: 0x155C | 0x8020B7E4 | size: 0xF4
 */
void FERender::PushTransformMatrix(const TLInstance* instance)
{
    nlMatrix4 combinedMatrix;
    nlMatrix4 scaleMatrix;
    nlMatrix4 rotationMatrix;

    const feVector3& rotZ = instance->GetRotation();
    const feVector3& rotY = instance->GetRotation();
    nlMakeRotationMatrixEulerAngles(rotationMatrix,
        instance->GetRotation().f.x,
        rotY.f.y,
        rotZ.f.z);

    const feVector3& scaleZ = instance->GetScale();
    const feVector3& scaleY = instance->GetScale();
    nlMakeScaleMatrix(scaleMatrix,
        instance->GetScale().f.x,
        scaleY.f.y,
        scaleZ.f.z);

    nlMultMatrices(combinedMatrix, scaleMatrix, rotationMatrix);

    const feVector3& pos = instance->GetPosition();
    float z = pos.f.z;
    float negOne = -1.0f;
    nlVec3Set(combinedMatrix.GetTranslation(), pos.f.x, pos.f.y, z);
    combinedMatrix.f.m44 = 1.0f;
    combinedMatrix.f.m43 = z * negOne;

    m_pMatrixStack->PushMatrix();
    m_pMatrixStack->MultMatrix(combinedMatrix);
}

/**
 * Offset/Address/Size: 0x1650 | 0x8020B8D8 | size: 0x50
 */
void FERender::Initialize()
{
    if (m_pMatrixStack == nullptr)
    {
        m_pMatrixStack = new (nlMalloc(sizeof(GLMatrixStack), 8, false)) GLMatrixStack(16);
    }
}

/**
 * Offset/Address/Size: 0x16A0 | 0x8020B928 | size: 0x38
 */
void FERender::Cleanup()
{
    if (m_pMatrixStack != nullptr)
    {
        delete m_pMatrixStack;
        m_pMatrixStack = nullptr;
    }
}

/**
 * Offset/Address/Size: 0x16D8 | 0x8020B960 | size: 0xB4
 */
void FERender::CalculateCurrentAssetColour(const TLInstance* instance)
{
    nlFloatColour* curAssetColour = &s_currentAssetColour;
    for (u32 i = 0; i < 4; i++)
    {
        curAssetColour->c[i] = (instance->GetColour().c[i] * curAssetColour->c[i]) / 255.0f;
    }
}

// {
//     const nlColour& colour = instance->GetColour();

//     for (int i = 0; i < 4; i++)
//     {
//         float component = (float)colour.c[i];
//         s_currentAssetColour.c[i] = (s_currentAssetColour.c[i] * component) / 255.0f;
//     }
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BA14 | size: 0x8
//  */
// void TLInstance::GetType() const
// {
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x8020BA1C | size: 0x8
//  */
// void TLInstance::IsVisible() const
// {
// }

// /**
//  * Offset/Address/Size: 0x10 | 0x8020BA24 | size: 0x8
//  */
// void TLInstance::GetLibRefObject() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BA2C | size: 0x8
//  */
// void TLComponent::GetActiveSlide()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BA34 | size: 0x8
//  */
// void FEScene::GetRenderView() const
// {
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x8020BA3C | size: 0x8
//  */
// void FEScene::GetCameraMatrix() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BA44 | size: 0x70
//  */
// void ConvertColour(nlColour&, const nlFloatColour&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BAB4 | size: 0x8
//  */
// void TLTextInstance::SetMatrix(nlMatrix4*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BABC | size: 0x24
//  */
// void nlMatrix4::SetTranslation(const nlVector3&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BAE0 | size: 0x1C
//  */
// void feVector3::GetNLVector3(nlVector3&) const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BAFC | size: 0x8
//  */
// void TLSlide::GetCurrentTime() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BB04 | size: 0x3C
//  */
// void 0x8028D570..0x8028D574 | size: 0x4
// {
// }
