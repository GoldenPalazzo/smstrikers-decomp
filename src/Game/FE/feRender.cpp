#include "Game/FE/feRender.h"
#include "Game/FE/tlInstance.h"
#include "Game/FE/tlImageInstance.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/FE/feScene.h"
#include "Game/FE/fePackage.h"
#include "Game/FE/fePresentation.h"
#include "Game/FE/feImage.h"
#include "Game/FE/feTextureResource.h"
#include "Game/GL/gluMeshWriter.h"
#include "NL/gl/glMatrixStack.h"
#include "NL/gl/glState.h"
#include "NL/gl/glMatrix.h"
#include "NL/nlMemory.h"
#include "NL/nlColour.h"
#include "NL/platvmath.h"
#include "NL/gl/gl.h"

GLMatrixStack* FERender::m_pMatrixStack = nullptr;
FEScene* FERender::m_pRenderScene = nullptr;

static nlFloatColour s_currentAssetColour;
static unsigned long drawQuadProgram = glGetProgram("2d unlit");
static unsigned long grabTex = nlStringLowerHash("target/grab_texture");

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
 */
void FERender::RenderTextInstance(TLTextInstance* textInstance)
{
    nlMatrix4 combinedMatrix;

    m_pMatrixStack->GetTop(combinedMatrix);

    nlMultMatrices(combinedMatrix, combinedMatrix, m_pRenderScene->m_matView);

    textInstance->m_DrawInfo.pMatrix = &combinedMatrix;

    nlColour colour;
    colour.c[0] = (s32)(s_currentAssetColour.c[0] * 255.0f);
    colour.c[1] = (s32)(s_currentAssetColour.c[1] * 255.0f);
    colour.c[2] = (s32)(s_currentAssetColour.c[2] * 255.0f);
    colour.c[3] = (s32)(s_currentAssetColour.c[3] * 255.0f);

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
 * TODO: 92.2% match - same MWCC register-allocator coloring as RenderSlide:
 * outer-loop instance/nextInstance and inner-loop child/nextChild swap pairs
 * vs target, plus v3Pos/oldChildColour stack-slot swap. No opcode diffs.
 * ConvertColour kept as a real call via #pragma inline_depth(0)/() around the
 * TLAT_TEXT call site; grandchild operator= is forced via the extern __as__
 * declaration above. The combination of pragma here + pragma in RenderSlide is
 * the local optimum; removing either regresses the other by more than it gains.
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
#pragma inline_depth(0)
                ConvertColour(colour, s_currentAssetColour);
#pragma inline_depth()

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
 * TODO: 98.84% match - MWCC register allocation: instance r30 vs target r28,
 * nextInstance r29 vs target r27, child r28 vs target r29, nextChild r27 vs
 * target r30, grandchild r23/r22 swapped. Stack: v3Pos at 0x2c vs target 0x1c,
 * oldChildColour at 0x1c vs target 0x28 (v3Pos and oldChildColour swapped in
 * frame). No opcode or control-flow diffs -- purely allocator choices.
 * ConvertColour kept as a real call via #pragma inline_depth(0)/() around the
 * TLAT_TEXT call site (call-site-local rather than body-level FDI).
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
            nlFloatColour oldChildColour;
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
#pragma inline_depth(0)
                ConvertColour(colour, s_currentAssetColour);
#pragma inline_depth()

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
                    *(u32*)&oldChildColour.c[0] = *(u32*)&s_currentAssetColour.c[0];
                    *(u32*)&oldChildColour.c[1] = *(u32*)&s_currentAssetColour.c[1];
                    *(u32*)&oldChildColour.c[2] = *(u32*)&s_currentAssetColour.c[2];
                    *(u32*)&oldChildColour.c[3] = *(u32*)&s_currentAssetColour.c[3];
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

                    u32* childDst = (u32*)&s_currentAssetColour;
                    u32* childSrc = (u32*)&oldChildColour;
                    childDst[0] = childSrc[0];
                    childDst[1] = childSrc[1];
                    childDst[2] = childSrc[2];
                    childDst[3] = childSrc[3];

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
 * TODO: 98.82% match - 131 pure register-allocation diffs: pTLInstance r26 vs
 * target r31, colour loop base r31 vs target r27, cascading through all
 * nested loop variables. Stack offsets shifted accordingly. No opcode or
 * control-flow diffs.
 */
void FERender::RenderTimeLineAsset(TLInstance* pTLInstance, float fCurrentTime)
{
    if (!pTLInstance->IsValidAtTime(fCurrentTime))
    {
        return;
    }

    if (!pTLInstance->m_bVisible)
    {
        return;
    }

    nlMatrix4 rotationMatrix;
    nlMatrix4 scaleMatrix;
    nlMatrix4 combinedMatrix;

    const feVector3& rotZ = pTLInstance->GetRotation();
    const feVector3& rotY = pTLInstance->GetRotation();
    nlMakeRotationMatrixEulerAngles(
        rotationMatrix,
        pTLInstance->GetRotation().f.x,
        rotY.f.y,
        rotZ.f.z);

    const feVector3& scaleZ = pTLInstance->GetScale();
    const feVector3& scaleY = pTLInstance->GetScale();
    nlMakeScaleMatrix(
        scaleMatrix,
        pTLInstance->GetScale().f.x,
        scaleY.f.y,
        scaleZ.f.z);

    nlMultMatrices(combinedMatrix, scaleMatrix, rotationMatrix);

    const feVector3& pos = pTLInstance->GetPosition();
    float x;
    float negOne = -1.0f;
    float y;
    float z = pos.f.z;
    y = pos.f.y;
    x = pos.f.x;
    combinedMatrix.f.m43 = z;
    combinedMatrix.f.m41 = x;
    combinedMatrix.f.m42 = y;
    combinedMatrix.f.m44 = 1.0f;
    combinedMatrix.f.m43 = z * negOne;

    m_pMatrixStack->PushMatrix();
    m_pMatrixStack->MultMatrix(combinedMatrix);

    for (u32 i = 0; i < 4; i++)
    {
        s_currentAssetColour.c[i] = (pTLInstance->GetColour().c[i] * s_currentAssetColour.c[i]) / 255.0f;
    }

    switch (pTLInstance->m_type)
    {
    case TLAT_IMAGE:
        RenderImageInstance((const TLImageInstance*)pTLInstance);
        break;
    case TLAT_TEXT:
    {
        nlMatrix4 textMatrix;
        m_pMatrixStack->GetTop(textMatrix);
        nlMultMatrices(textMatrix, textMatrix, m_pRenderScene->m_matView);
        ((TLTextInstance*)pTLInstance)->m_DrawInfo.pMatrix = &textMatrix;

        nlColour colour;
        colour.c[0] = (s32)(s_currentAssetColour.c[0] * 255.0f);
        colour.c[1] = (s32)(s_currentAssetColour.c[1] * 255.0f);
        colour.c[2] = (s32)(s_currentAssetColour.c[2] * 255.0f);
        colour.c[3] = (s32)(s_currentAssetColour.c[3] * 255.0f);

        ((TLTextInstance*)pTLInstance)->Render((eGLView)m_pRenderScene->m_uRenderView, colour);
        break;
    }
    case TLAT_COMPONENT:
    {
        TLComponent* compRef = pTLInstance->m_component;
        if (compRef != 0)
        {
            TLSlide* slide = compRef->m_pActiveSlide;
            if (slide != 0)
            {
                if (slide != 0 && slide->m_instances != 0)
                {
                    TLInstance* curr = slide->m_instances->m_next;

                    while (true)
                    {
                        nlFloatColour oldSlideColour = s_currentAssetColour;
                        TLInstance* next = curr->m_next;

                        float slideTime = slide->GetCurrentTime();
                        if (curr->IsValidAtTime(slideTime) && curr->IsVisible())
                        {
                            PushTransformMatrix(curr);
                            CalculateCurrentAssetColour(curr);

                            switch (curr->GetType())
                            {
                            case TLAT_IMAGE:
                                RenderImageInstance((const TLImageInstance*)curr);
                                break;
                            case TLAT_TEXT:
#pragma inline_depth(0)
                                RenderTextInstance((TLTextInstance*)curr);
#pragma inline_depth()
                                break;
                            case TLAT_COMPONENT:
#pragma inline_depth(0)
                                RenderComponentInstance((TLComponentInstance*)curr);
#pragma inline_depth()
                                break;
                            default:
                                break;
                            }

                            if (curr->pChildren != 0)
                            {
                                TLInstance* child = curr->pChildren->m_next;

                                while (true)
                                {
                                    TLInstance* nextChild = child->m_next;
                                    nlFloatColour oldChildColour = s_currentAssetColour;

                                    RenderTimeLineAsset(child, slideTime);

#pragma inline_depth(0)
                                    s_currentAssetColour = oldChildColour;
#pragma inline_depth()

                                    if (child == curr->pChildren)
                                    {
                                        break;
                                    }

                                    child = nextChild;
                                }
                            }

                            PopTransformMatrix();
                        }

                        {
                            u32* slideDst = (u32*)&s_currentAssetColour;
                            u32* slideSrc = (u32*)&oldSlideColour;
                            slideDst[0] = slideSrc[0];
                            slideDst[1] = slideSrc[1];
                            slideDst[2] = slideSrc[2];
                            slideDst[3] = slideSrc[3];
                        }

                        if (curr == slide->m_instances)
                        {
                            break;
                        }

                        curr = next;
                    }
                }
            }
        }
        break;
    }
    default:
        break;
    }

    if (pTLInstance->pChildren != 0)
    {
        TLInstance* curr = pTLInstance->pChildren->m_next;

        while (true)
        {
            nlFloatColour colour = s_currentAssetColour;
            TLInstance* next = curr->m_next;

            if (curr->IsValidAtTime(fCurrentTime) && curr->m_bVisible)
            {
                nlMatrix4 rotMatrix;
                nlMatrix4 sclMatrix;
                nlMatrix4 combMatrix;

                const feVector3& rZ = curr->GetRotation();
                const feVector3& rY = curr->GetRotation();
                nlMakeRotationMatrixEulerAngles(
                    rotMatrix,
                    curr->GetRotation().f.x,
                    rY.f.y,
                    rZ.f.z);

                const feVector3& sZ = curr->GetScale();
                const feVector3& sY = curr->GetScale();
                nlMakeScaleMatrix(
                    sclMatrix,
                    curr->GetScale().f.x,
                    sY.f.y,
                    sZ.f.z);

                nlMultMatrices(combMatrix, sclMatrix, rotMatrix);

                nlVector3 v3Pos;
                curr->GetPosition().GetNLVector3(v3Pos);
                combMatrix.SetTranslation(v3Pos);
                combMatrix.f.m43 *= -1.0f;

                m_pMatrixStack->PushMatrix();
                m_pMatrixStack->MultMatrix(combMatrix);

                for (u32 j = 0; j < 4; j++)
                {
                    s_currentAssetColour.c[j] = (curr->GetColour().c[j] * s_currentAssetColour.c[j]) / 255.0f;
                }

                switch (curr->m_type)
                {
                case TLAT_IMAGE:
                    RenderImageInstance((const TLImageInstance*)curr);
                    break;
                case TLAT_TEXT:
                {
                    nlMatrix4 textMat;
                    m_pMatrixStack->GetTop(textMat);
                    nlMultMatrices(textMat, textMat, m_pRenderScene->GetCameraMatrix());
                    ((TLTextInstance*)curr)->SetMatrix(&textMat);

                    nlColour col;
#pragma inline_depth(0)
                    ConvertColour(col, s_currentAssetColour);
#pragma inline_depth()

                    ((TLTextInstance*)curr)->Render(m_pRenderScene->GetRenderView(), col);
                    break;
                }
                case TLAT_COMPONENT:
                {
                    TLComponent* compRef = (TLComponent*)curr->GetLibRefObject();
                    if (compRef != 0)
                    {
                        if (compRef->GetActiveSlide() != 0)
                        {
                            RenderSlide(compRef->GetActiveSlide());
                        }
                    }
                    break;
                }
                default:
                    break;
                }

                if (curr->pChildren != 0)
                {
                    TLInstance* grandchild = curr->pChildren->m_next;

                    while (true)
                    {
                        nlFloatColour oldGrandColour = s_currentAssetColour;
                        TLInstance* nextGrand = grandchild->m_next;

                        if (grandchild->IsValidAtTime(fCurrentTime) && grandchild->IsVisible())
                        {
                            PushTransformMatrix(grandchild);
                            CalculateCurrentAssetColour(grandchild);

                            switch (grandchild->GetType())
                            {
                            case TLAT_IMAGE:
                                RenderImageInstance((const TLImageInstance*)grandchild);
                                break;
                            case TLAT_TEXT:
#pragma inline_depth(0)
                                RenderTextInstance((TLTextInstance*)grandchild);
#pragma inline_depth()
                                break;
                            case TLAT_COMPONENT:
#pragma inline_depth(0)
                                RenderComponentInstance((TLComponentInstance*)grandchild);
#pragma inline_depth()
                                break;
                            default:
                                break;
                            }

                            if (grandchild->pChildren != 0)
                            {
                                TLInstance* greatGrand = grandchild->pChildren->m_next;

                                while (true)
                                {
                                    TLInstance* nextGreat = greatGrand->m_next;
                                    nlFloatColour oldGreatColour = s_currentAssetColour;

                                    RenderTimeLineAsset(greatGrand, fCurrentTime);

#pragma inline_depth(0)
                                    s_currentAssetColour = oldGreatColour;
#pragma inline_depth()

                                    if (greatGrand == grandchild->pChildren)
                                    {
                                        break;
                                    }

                                    greatGrand = nextGreat;
                                }
                            }

                            PopTransformMatrix();
                        }

                        {
                            u32* grandDst = (u32*)&s_currentAssetColour;
                            u32* grandSrc = (u32*)&oldGrandColour;
                            grandDst[0] = grandSrc[0];
                            grandDst[1] = grandSrc[1];
                            grandDst[2] = grandSrc[2];
                            grandDst[3] = grandSrc[3];
                        }

                        if (grandchild == curr->pChildren)
                        {
                            break;
                        }

                        grandchild = nextGrand;
                    }
                }

                m_pMatrixStack->PopMatrix();
            }

            {
                u32* colDst = (u32*)&s_currentAssetColour;
                u32* colSrc = (u32*)&colour;
                colDst[0] = colSrc[0];
                colDst[1] = colSrc[1];
                colDst[2] = colSrc[2];
                colDst[3] = colSrc[3];
            }

            if (curr == pTLInstance->pChildren)
            {
                break;
            }

            curr = next;
        }
    }

    m_pMatrixStack->PopMatrix();
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

    const feVector3& tlPosition = instance->GetPosition();
    float x;
    float negOne = -1.0f;
    float y;
    float z = tlPosition.f.z;
    y = tlPosition.f.y;
    x = tlPosition.f.x;
    combinedMatrix.f.m43 = z;
    combinedMatrix.f.m41 = x;
    combinedMatrix.f.m42 = y;
    combinedMatrix.f.m44 = 1.0f;
    combinedMatrix.f.m43 = z * negOne;

    m_pMatrixStack->PushMatrix();
    m_pMatrixStack->MultMatrix(combinedMatrix);
}

/**
 * Offset/Address/Size: 0x1650 | 0x8020B8D8 | size: 0x50
 */
#pragma inline_depth(255)
void FERender::Initialize()
{
    if (m_pMatrixStack == nullptr)
    {
        m_pMatrixStack = new (8, false) GLMatrixStack(16);
    }
}
#pragma inline_depth()

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

/**
 * Offset/Address/Size: 0x0 | 0x8020BA14 | size: 0x8
 */
eTimeLineAssetType TLInstance::GetType() const
{
    return m_type;
}

/**
 * Offset/Address/Size: 0x8 | 0x8020BA1C | size: 0x8
 */
bool TLInstance::IsVisible() const
{
    return m_bVisible;
}

/**
 * Offset/Address/Size: 0x10 | 0x8020BA24 | size: 0x8
 */
FELibObject* TLInstance::GetLibRefObject() const
{
    return m_component;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020BA2C | size: 0x8
 */
TLSlide* TLComponent::GetActiveSlide()
{
    return m_pActiveSlide;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020BA34 | size: 0x8
 */
eGLView FEScene::GetRenderView() const
{
    return (eGLView)m_uRenderView;
}

/**
 * Offset/Address/Size: 0x8 | 0x8020BA3C | size: 0x8
 */
const nlMatrix4& FEScene::GetCameraMatrix() const
{
    return m_matView;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020BA44 | size: 0x70
 */

/**
 * Offset/Address/Size: 0x0 | 0x8020BAB4 | size: 0x8
 */
void TLTextInstance::SetMatrix(nlMatrix4* pMatrix)
{
    m_DrawInfo.pMatrix = pMatrix;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020BABC | size: 0x24
 */
void nlMatrix4::SetTranslation(const nlVector3& v)
{
    f.m41 = v.f.x;
    f.m42 = v.f.y;
    f.m43 = v.f.z;
    f.m44 = 1.0f;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020BAE0 | size: 0x1C
 */
void feVector3::GetNLVector3(nlVector3& out) const
{
    out.f.x = f.x;
    out.f.y = f.y;
    out.f.z = f.z;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020BAFC | size: 0x8
 */
f32 TLSlide::GetCurrentTime() const
{
    return m_time;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8020BB04 | size: 0x3C
//  */
// void 0x8028D570..0x8028D574 | size: 0x4
// {
// }

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler
 * lays out the related fields to match the original binary.
 */
void feRender_stub()
{
    void (nlFloatColour::* volatile forceAssign)(const nlFloatColour&) = &nlFloatColour::operator=;
    void (*volatile forceConvertColour)(nlColour&, const nlFloatColour&) = &ConvertColour;
    (void)forceAssign;
    (void)forceConvertColour;
}
