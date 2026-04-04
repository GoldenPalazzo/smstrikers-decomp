#include "Game/ObjectBlur.h"

#include "NL/nlMath.h"
#include "NL/nlString.h"
#include "NL/nlMemory.h"

#include "Game/Camera/CameraMan.h"
#include "Game/GL/gluMeshWriter.h"

#include "NL/gl/glState.h"
#include "NL/gl/glMatrix.h"

#include "Game/ReplayManager.h"

BlurHandler* BlurManager::m_activeBlurHandler = NULL;

SlotPool<BlurHandler> BlurManager::m_BlurHandlerSlotPool;

f32 fFlimmerOffset = 0.01f;

const eGLStream stream_decl_[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };

/**
 * Offset/Address/Size: 0x0 | 0x801627D4 | size: 0x3C
 */
void BlurManager::Shutdown()
{
    nlDeleteDLRing<BlurHandler>(&m_activeBlurHandler);
    m_activeBlurHandler = NULL;
    SlotPoolBase::BaseFreeBlocks(&m_BlurHandlerSlotPool, 0x4C);
}

/**
 * Offset/Address/Size: 0x3C | 0x80162810 | size: 0x128
 */
void BlurManager::Update(float deltaTime)
{
    if (m_activeBlurHandler == NULL)
        return;

    bool isEnd;
    BlurHandler* current = nlDLRingGetStart<BlurHandler>(m_activeBlurHandler);
    do
    {
        isEnd = nlDLRingIsEnd<BlurHandler>(m_activeBlurHandler, current);
        BlurHandler* next = current->m_next;

        if (current->m_bDying)
        {
            current->m_fDyingTimer -= deltaTime;
            if (current->m_fDyingTimer <= 0.0f)
            {
                current->m_fDyingTimer = 0.0f;
            }
        }

        bool shouldRemove;
        if (current->m_bDying && current->m_fDyingTimer <= 0.0f)
        {
            shouldRemove = true;
        }
        else
        {
            shouldRemove = false;
        }

        if (shouldRemove)
        {
            nlDLRingRemove<BlurHandler>(&m_activeBlurHandler, current);
            if (current != NULL)
            {
                delete[] current->m_pointRingBuffer;
                current->m_next = (BlurHandler*)m_BlurHandlerSlotPool.m_FreeList;
                m_BlurHandlerSlotPool.m_FreeList = (SlotPoolEntry*)current;
            }
        }
        else
        {
            current->RenderMesh(current->m_uTexHashID);
        }

        current = next;
    } while (!isEnd);
}

/**
 * Offset/Address/Size: 0x164 | 0x80162938 | size: 0x78
 */
void BlurManager::DestroyHandler(BlurHandler* handler, float timeToDie)
{
    if (timeToDie == 0.0f)
    {
        nlDLRingRemove<BlurHandler>(&m_activeBlurHandler, handler);
        if (handler != NULL)
        {
            delete[] handler->m_pointRingBuffer;
            handler->m_next = (BlurHandler*)m_BlurHandlerSlotPool.m_FreeList;
            m_BlurHandlerSlotPool.m_FreeList = (SlotPoolEntry*)handler;
        }
    }
    else
    {
        handler->m_bDying = true;
        handler->m_fDyingTimer = timeToDie;
        handler->m_fTimeToDie = timeToDie;
    }
}

/**
 * Offset/Address/Size: 0x1DC | 0x801629B0 | size: 0xD0
 */
BlurHandler* BlurManager::GetNewHandler(const char* szTextureName, float fLineWidth, int maxPositionEntries, bool bAdditive)
{
    BlurHandler* handler = nullptr;

    if (m_BlurHandlerSlotPool.m_FreeList == nullptr)
    {
        SlotPoolBase::BaseAddNewBlock(&m_BlurHandlerSlotPool, 0x4C);
    }

    if (m_BlurHandlerSlotPool.m_FreeList != nullptr)
    {
        handler = (BlurHandler*)m_BlurHandlerSlotPool.m_FreeList;
        m_BlurHandlerSlotPool.m_FreeList = (SlotPoolEntry*)handler->m_next;
    }

    if (handler != nullptr)
    {
        handler->m_bAdditive = bAdditive;
        handler->m_fLineWidth = fLineWidth;
        handler->m_pLastPoint = nullptr;
        handler->m_bDying = false;
        handler->m_maxPositionEntries = maxPositionEntries;
        handler->m_nInsertIndex = 0;
        handler->m_nTrailEndPointer = 0;
        handler->m_uTexHashID = nlStringLowerHash(szTextureName);
        handler->m_pointRingBuffer = (BlurPointEntry*)nlMalloc(maxPositionEntries * sizeof(BlurPointEntry), 8, false);
    }

    nlDLRingAddEnd<BlurHandler>(&m_activeBlurHandler, handler);

    return handler;
}

/**
 * Offset/Address/Size: 0x2AC | 0x80162A80 | size: 0x514
 * TODO: 88.92% match - MWCC in decomp.me devirtualizes Texcoord calls (direct bl)
 * while target uses virtual dispatch (lwz+lwz+mtctr+bctrl). 12 extra target instructions
 * from vtable dispatch + 4 from v3Bottom pointer pre-computation + lwz vs li for 0xFFFFFFFF.
 */
void BlurHandler::RenderMesh(unsigned long uTexID)
{
    u32 count;
    u32 uPointIndex;
    u32 matHandle;
    nlColour colour;
    f32 nonAdditiveAlpha;
    BlurPointEntry* BPEntry;

    count = 0;
    if (m_nTrailEndPointer != m_nInsertIndex)
    {
        if (m_nTrailEndPointer < m_nInsertIndex)
        {
            count = m_nInsertIndex - m_nTrailEndPointer;
        }
        else
        {
            count = m_maxPositionEntries - m_nTrailEndPointer;
            count += m_nInsertIndex;
        }
    }

    if (count < 2)
    {
        return;
    }

    eGLStream stream_decl[3];
    stream_decl[0] = stream_decl_[0];
    stream_decl[1] = stream_decl_[1];
    stream_decl[2] = stream_decl_[2];

    GLMeshWriter mesh;

    glSetDefaultState(true);

    nlMatrix4 matWorld;
    matWorld.SetIdentity();
    matWorld.f.m43 += fFlimmerOffset;

    matHandle = glAllocMatrix();
    if (matHandle != (u32)-1)
    {
        glSetMatrix(matHandle, matWorld);
    }
    glSetCurrentMatrix(matHandle);

    if (m_bAdditive)
    {
        glSetRasterState(GLS_AlphaBlend, 2);
    }
    else
    {
        glSetRasterState(GLS_AlphaBlend, 1);
    }
    glSetRasterState(GLS_AlphaTest, 1);
    glSetRasterState(GLS_AlphaTestRef, 0);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetRasterState(GLS_Culling, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTexture(uTexID, GLTT_Diffuse);
    glSetCurrentProgram(glGetProgram("3d unlit"));

    nonAdditiveAlpha = 0.0f;
    *(u32*)&colour.c[0] = 0xFFFFFFFF;

    if (m_bAdditive)
    {
        nlColourSet(colour, 0xFF, 0xFF, 0xFF, 0xFF);
        if (m_bDying)
        {
            int fadeVal = 255.0f * (m_fDyingTimer / m_fTimeToDie);
            nlColourSeReversed(colour, fadeVal, fadeVal, fadeVal, fadeVal);
        }
    }
    else if (m_bDying)
    {
        nonAdditiveAlpha = 90.0f * (m_fDyingTimer / m_fTimeToDie);
    }
    else
    {
        nonAdditiveAlpha = 90.0f;
    }

    uPointIndex = m_nTrailEndPointer;
    f32 texU = 0.0f;
    f32 texUIncrement = 1.0f / (f32)count;

    if (mesh.Begin(count * 2, GLP_TriStrip, 3, stream_decl, false))
    {
        for (u32 i = 0; i < count; i++)
        {
            BPEntry = &m_pointRingBuffer[uPointIndex];
            if (!m_bAdditive)
            {
                colour.c[3] = nonAdditiveAlpha * texU;
            }

            if ((i == 0 || i == count - 1) && !m_bDying)
            {
                int pointIndexA, pointIndexB;
                if (i == 0)
                {
                    pointIndexB = uPointIndex + 1;
                    pointIndexA = uPointIndex;
                    if (pointIndexB == m_maxPositionEntries)
                    {
                        pointIndexB = 0;
                    }
                }
                else
                {
                    pointIndexA = uPointIndex - 1;
                    pointIndexB = uPointIndex;
                    if (pointIndexA < 0)
                    {
                        pointIndexA = m_maxPositionEntries - 1;
                    }
                }

                f32 blendPct = ReplayManager::Instance()->mRender->mFrameBlendPercent;
                BlurPointEntry* pB = &m_pointRingBuffer[pointIndexB];
                BlurPointEntry* pA = &m_pointRingBuffer[pointIndexA];

                f32 invBlend = 1.0f - blendPct;

                nlVector3 v3Top;
                v3Top.f.x = blendPct * pB->v3Top.f.x + invBlend * pA->v3Top.f.x;
                v3Top.f.y = blendPct * pB->v3Top.f.y + invBlend * pA->v3Top.f.y;
                v3Top.f.z = blendPct * pB->v3Top.f.z + invBlend * pA->v3Top.f.z;

                nlVector3 v3Bottom;
                v3Bottom.f.x = blendPct * pB->v3Bottom.f.x + invBlend * pA->v3Bottom.f.x;
                v3Bottom.f.y = blendPct * pB->v3Bottom.f.y + invBlend * pA->v3Bottom.f.y;
                v3Bottom.f.z = blendPct * pB->v3Bottom.f.z + invBlend * pA->v3Bottom.f.z;

                mesh.Colour(colour);
                nlVector2 tc0;
                tc0.f.x = texU;
                tc0.f.y = 0.0f;
                mesh.Texcoord(tc0);
                mesh.Vertex(v3Top);

                mesh.Colour(colour);
                nlVector2 tc1;
                tc1.f.x = texU;
                tc1.f.y = 1.0f;
                mesh.Texcoord(tc1);
                mesh.Vertex(v3Bottom);
            }
            else
            {
                mesh.Colour(colour);
                nlVector2 tc0;
                tc0.f.x = texU;
                tc0.f.y = 0.0f;
                mesh.Texcoord(tc0);
                mesh.Vertex(BPEntry->v3Top);

                mesh.Colour(colour);
                nlVector2 tc1;
                tc1.f.x = texU;
                tc1.f.y = 1.0f;
                mesh.Texcoord(tc1);
                mesh.Vertex(BPEntry->v3Bottom);
            }

            uPointIndex++;
            if (uPointIndex == (u32)m_maxPositionEntries)
            {
                uPointIndex = 0;
            }
            texU += texUIncrement;
        }

        if (!mesh.End())
        {
            return;
        }
        glViewAttachModel(GLV_Particles, 5, mesh.GetModel());
    }
}

/**
 * Offset/Address/Size: 0x7C0 | 0x80162F94 | size: 0x14
 */
void BlurHandler::Die(float timeToDie)
{
    m_bDying = true;
    m_fDyingTimer = timeToDie;
    m_fTimeToDie = timeToDie;
}

/**
 * Offset/Address/Size: 0x7D4 | 0x80162FA8 | size: 0x1B0
 */
void BlurHandler::AddViewOrientedPoint(const nlVector3& position, const nlVector3& forwardVector)
{
    nlVector3 topPoint, bottomPoint;

    if (ConstructViewOrientedPoints(topPoint, bottomPoint, position, forwardVector))
    {
        m_pointFinal.v3Top = topPoint;
        m_pointFinal.v3Bottom = bottomPoint;

        if (m_pLastPoint != nullptr)
        {
            nlVector3 delta;
            nlVec3Set(delta, m_pLastPoint->v3Top.f.x - m_pointFinal.v3Top.f.x, m_pLastPoint->v3Top.f.y - m_pointFinal.v3Top.f.y, m_pLastPoint->v3Top.f.z - m_pointFinal.v3Top.f.z);

            if ((delta.f.x * delta.f.x) + (delta.f.y * delta.f.y) + (delta.f.z * delta.f.z) < 0.0025000002f)
            {
                return;
            }
        }

        BlurPointEntry* entry = &m_pointRingBuffer[m_nInsertIndex];
        entry->v3Top = m_pointFinal.v3Top;
        entry->v3Bottom = m_pointFinal.v3Bottom;

        m_pLastPoint = &m_pointRingBuffer[m_nInsertIndex];
        m_nInsertIndex = m_nInsertIndex + 1;
        m_nInsertIndex %= m_maxPositionEntries;

        if (m_nInsertIndex == m_nTrailEndPointer)
        {
            m_nTrailEndPointer = m_nTrailEndPointer + 1;
            m_nTrailEndPointer %= m_maxPositionEntries;
        }
    }
    else
    {
        if (m_nInsertIndex != m_nTrailEndPointer)
        {
            m_nTrailEndPointer = m_nTrailEndPointer + 1;
            m_nTrailEndPointer %= m_maxPositionEntries;
        }
    }
}

inline void nlVec3CrossProduct(nlVector3& result, const nlVector3& a, const nlVector3& b)
{
    result.f.x = (a.f.y * b.f.z) - (a.f.z * b.f.y);
    result.f.y = (a.f.z * b.f.x) - (a.f.x * b.f.z);
    result.f.z = (a.f.x * b.f.y) - (a.f.y * b.f.x);
}

inline float nlVec3DotProduct(const nlVector3& a, const nlVector3& b)
{
    return (a.f.x * b.f.x) + (a.f.y * b.f.y) + (a.f.z * b.f.z);
}

inline float nlVec3LengthSquared(const nlVector3& a)
{
    return (a.f.x * a.f.x) + (a.f.y * a.f.y) + (a.f.z * a.f.z);
}

inline float nlVec3Length(const nlVector3& a)
{
    return sqrtf(nlVec3LengthSquared(a));
}

/**
 * Offset/Address/Size: 0x984 | 0x80163158 | size: 0x1C4
 * TODO: 98.0% match - remaining volatile FP register allocation diffs:
 * viewVector x/y load order (f4/f6 swap), cascading to perp f5/f6/f4
 * vs target f6/f7/f8 in both branches.
 */
/**
 * Offset/Address/Size: 0x3DC | 0x80163158 | size: 0x1C4
 * TODO: 98.63% match - volatile FPR allocation offset for perp values (f5,f6,f4 vs f6,f7,f8)
 */
bool BlurHandler::ConstructViewOrientedPoints(nlVector3& topPoint, nlVector3& bottomPoint, nlVector3 position, const nlVector3& forwardVector)
{
    nlVector3 viewVector;
    nlVector3 perp;

    float sLen1 = nlVec3LengthSquared(forwardVector);
    if (sLen1 < 0.01f)
    {
        return false;
    }

    float invLen = nlRecipSqrt(sLen1, 1);
    float normX;
    float normZ;
    float normY;
    normZ = invLen * forwardVector.f.z;
    normY = invLen * forwardVector.f.y;
    normX = invLen * forwardVector.f.x;

    cCameraManager::GetViewVector(viewVector);
    if (viewVector.f.x * normX + viewVector.f.y * normY + viewVector.f.z * normZ < 0.99f)
    {
        float crossX = (normY * viewVector.f.z) - (normZ * viewVector.f.y);
        float crossY = (-normX * viewVector.f.z) + (normZ * viewVector.f.x);
        float crossZ = (normX * viewVector.f.y) - (normY * viewVector.f.x);

        float invLen2 = nlRecipSqrt((crossZ * crossZ) + ((crossX * crossX) + (crossY * crossY)), 1);

        nlVec3Set(perp, m_fLineWidth * (invLen2 * crossX), m_fLineWidth * (invLen2 * crossY), m_fLineWidth * (invLen2 * crossZ));
    }
    else
    {
        if (m_pLastPoint != NULL)
        {
            nlVec3Set(perp, 0.5f * (m_pLastPoint->v3Top.f.x - m_pLastPoint->v3Bottom.f.x), 0.5f * (m_pLastPoint->v3Top.f.y - m_pLastPoint->v3Bottom.f.y), 0.5f * (m_pLastPoint->v3Top.f.z - m_pLastPoint->v3Bottom.f.z));
        }
        else
        {
            return false;
        }
    }
    float pY_5 = position.f.x;
    float pX_2 = position.f.y;
    float pZ_2 = position.f.z;
    topPoint.f.x = pY_5 + perp.f.x;
    topPoint.f.y = pX_2 + perp.f.y;
    topPoint.f.z = pZ_2 + perp.f.z;
    bottomPoint.f.x = pY_5 - perp.f.x;
    bottomPoint.f.y = pX_2 - perp.f.y;
    bottomPoint.f.z = pZ_2 - perp.f.z;
    return true;
}

/**
 * Offset/Address/Size: 0xC58 | 0x8016342C | size: 0x94
 */
template <>
void nlDeleteRing<BlurHandler>(BlurHandler** head)
{
    extern SlotPool<BlurHandler> m_BlurHandlerSlotPool__11BlurHandler;
    BlurHandler* element;
    BlurHandler* next;

    BlurHandler* headPtr = *head;
    if (headPtr != NULL)
    {
        element = headPtr->m_next;
        for (;;)
        {
            next = element->m_next;
            if (element != NULL)
            {
                delete[] element->m_pointRingBuffer;
                element->m_next = (BlurHandler*)m_BlurHandlerSlotPool__11BlurHandler.m_FreeList;
                m_BlurHandlerSlotPool__11BlurHandler.m_FreeList = (SlotPoolEntry*)element;
            }
            if (element == *head)
            {
                break;
            }
            element = next;
        }
        *head = NULL;
    }
}
