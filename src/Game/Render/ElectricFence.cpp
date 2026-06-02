#include "Game/Render/ElectricFence.h"

#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EmissionController.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Field.h"
#include "Game/Game.h"
#include "Game/GameTweaks.h"
#include "Game/GL/GLMeshWriter.h"
#include "NL/gl/gluMeshWriter.h"
#include "Game/Net.h"
#include "Game/Render/SidelineExplodable.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"
#include "NL/gl/glView.h"
#include "NL/nlMath.h"
#include "NL/nlTask.h"

#include <math.h>

static f32 sfStartAngle = 180.0f;
static f32 sfElectricFenceDisplayAngle;
static bool sbIsElectricFenceBeingDisplayed;
static float sfAngleAnimationRate = 100.0f;
static float sfTimeBetweenEffects = 0.02f;
static int sNumRevolutionsToDisplay = 2;
static float sfAngleRandomOffset = 10.0f;
static bool sbUseSparksDuringElectricFenceFlyBy = true;

const unsigned long UnlitProgram = glGetProgram("3d unlit");
const unsigned long LitProgram = glGetProgram("3d pointlit");
const unsigned long LightTexture = glGetTexture("global/lightramp");
const unsigned long BlackTexture = glGetTexture("global/black");
const unsigned long WhiteTexture = glGetTexture("global/white");
const unsigned long GridTexture = glGetTexture("global/grid");

nlList<ElectricFenceData> ElectricFenceData::sActiveElectricFences((ElectricFenceData*)NULL, (ElectricFenceData*)NULL);
SlotPool<ElectricFenceData> ElectricFenceData::sElectricFenceDataPool(16, 16);
SlotPool<ElectricFenceGeometry> ElectricFenceGeometry::sElectricFenceGeometryPool(4, 4);

// /**
//  * Offset/Address/Size: 0x9C | 0x8016C6EC | size: 0x2C
//  */
// void nlListAddEnd<ElectricFenceData>(ElectricFenceData**, ElectricFenceData**, ElectricFenceData*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8016C650 | size: 0x9C
 */
template ElectricFenceData* nlListRemoveElement<ElectricFenceData>(ElectricFenceData**, ElectricFenceData*, ElectricFenceData**);

/**
 * Offset/Address/Size: 0x1370 | 0x8016C3A0 | size: 0x2B0
 */
void GetWallPoint(const nlVector3& impactPosition, float xOffset, float zOffset, nlVector3& outPosition)
{
    /**
     * TODO: 99.71% match - r29/r31 register allocation swap (impactPosition ptr vs yIsPositive).
     */
    extern float sfGridTextureSize;
    extern float sfNumGridSquares;
    extern float sfAlignmentOffset2;

    const nlVector3* const pPoint = &impactPosition;

    float cornerRadius = cField::GetCornerRadius();
    float goalLineX = cField::GetGoalLineX(1U);
    float sidelineY = cField::GetSidelineY(1U);
    u8 xIsPositive = pPoint->f.x > 0.0f;
    u8 yIsPositive = pPoint->f.y > 0.0f;

    nlVector3 impactPositionPositive = { };
    double absX = __fabs(pPoint->f.x);
    double absY = __fabs(((volatile const nlVector3*)pPoint)->f.y);
    impactPositionPositive.f.x = (float)absX;
    impactPositionPositive.f.y = (float)absY;
    impactPositionPositive.f.z = pPoint->f.z;

    float straightLength = goalLineX - cornerRadius;
    float cornerCircumference = cornerRadius;
    cornerCircumference *= 1.5707964f;

    float inCoordinate;
    if (impactPositionPositive.f.x >= straightLength && impactPositionPositive.f.y >= sidelineY - cornerRadius)
    {
        inCoordinate = nlATan2f(impactPositionPositive.f.y - (sidelineY - cornerRadius), impactPositionPositive.f.x - straightLength);
        inCoordinate = inCoordinate * cornerRadius;
    }
    else if (impactPositionPositive.f.x < straightLength)
    {
        inCoordinate = cornerCircumference + (straightLength - impactPositionPositive.f.x);
    }
    else
    {
        inCoordinate = -((sidelineY - cornerRadius) - impactPositionPositive.f.y);
    }

    float increment = sfGridTextureSize / sfNumGridSquares;
    float alignedDistance = sfAlignmentOffset2 + (increment * (float)floor(inCoordinate / increment));
    float outCoordinate = alignedDistance + xOffset;

    if (outCoordinate <= 0.0f)
    {
        outPosition.f.x = goalLineX;
        outPosition.f.y = outCoordinate + (sidelineY - cornerRadius);
        outPosition.f.z = impactPositionPositive.f.z + zOffset;
    }
    else if (outCoordinate >= cornerCircumference)
    {
        float linearOffset = outCoordinate - cornerCircumference;
        outPosition.f.x = straightLength - linearOffset;
        outPosition.f.y = sidelineY;
        outPosition.f.z = impactPositionPositive.f.z + zOffset;
    }
    else
    {
        s32 angle = (s32)(10430.378f * (outCoordinate / cornerRadius));
        float sinAngle = nlSin((u16)angle);
        float cosAngle = nlSin((u16)((u16)angle + 0x4000));

        outPosition.f.x = straightLength + (cornerRadius * cosAngle);
        outPosition.f.y = (sidelineY - cornerRadius) + (cornerRadius * sinAngle);
        outPosition.f.z = impactPositionPositive.f.z + zOffset;
    }

    if (xIsPositive == 0)
    {
        outPosition.f.x = -outPosition.f.x;
    }

    if (yIsPositive == 0)
    {
        outPosition.f.y = -outPosition.f.y;
    }
}

/**
 * Offset/Address/Size: 0x12CC | 0x8016C2FC | size: 0xA4
 */
void ElectricFenceFinished(EmissionController& controller)
{
    ElectricFenceData* node = ElectricFenceData::sActiveElectricFences.m_pStart;
    while (node != NULL)
    {
        if (node->mpEmissionController == &controller)
        {
            if (node != NULL)
            {
                nlListRemoveElement<ElectricFenceData>(&ElectricFenceData::sActiveElectricFences.m_pStart, node, &ElectricFenceData::sActiveElectricFences.m_pEnd);
                ElectricFenceData::numAllocated--;
                ElectricFenceGeometry* geom = node->mpGeometry;
                if (geom != NULL)
                {
                    SlotPoolEntry* oldFree = ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList;
                    *(SlotPoolEntry**)geom = oldFree;
                    ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList = (SlotPoolEntry*)geom;
                }
                SlotPoolEntry* oldFree2 = ElectricFenceData::sElectricFenceDataPool.m_FreeList;
                *(SlotPoolEntry**)node = oldFree2;
                ElectricFenceData::sElectricFenceDataPool.m_FreeList = (SlotPoolEntry*)node;
            }
            return;
        }
        node = node->next;
    }
}

/**
 * Offset/Address/Size: 0xEAC | 0x8016BEDC | size: 0x420
 */
static inline void RenderElectricFenceFlat(const nlVector3& position, const nlVector3& normal, float intensity)
{
    extern float sfGridTextureSize;
    extern const unsigned long GridTexture;

    nlMatrix4 matrix;
    nlMakeRotationMatrixX(matrix, 1.5707964f);

    float angle = nlATan2f(normal.f.y, normal.f.x);
    nlMatrix4 matrix2;
    nlMakeRotationMatrixZ(matrix2, 0.0000958738f * (float)(u16)(s32)(10430.378f * angle));
    nlMultMatrices(matrix, matrix, matrix2);

    matrix.f.m41 = position.f.x;
    matrix.f.m42 = position.f.y;
    matrix.f.m43 = position.f.z;
    matrix.f.m44 = 1.0f;

    glQuad3 quad;
    quad.SetupRotatedRectangle(sfGridTextureSize, sfGridTextureSize, matrix, false, false);

    u8 lightenAmount = (u8)(255.0f * intensity);
    quad.SetColour(lightenAmount, lightenAmount, lightenAmount, 0xFF);
    glAttachQuad3(GLV_ElectricFence, 1, &quad, true);
}

static inline void DrawPrimitive(const ElectricFenceGeometry& prim, const nlMatrix4& objMatrix, int nPrimType, unsigned long textureHandle, float intensity)
{
    extern const unsigned long UnlitProgram;

    const eGLStream stream_decl[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
    GLMeshWriter mesh;
    nlVector3* pPosition;
    nlVector2* pTexcoord = (nlVector2*)prim.texcoord;

    glSetDefaultState(true);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetRasterState(GLS_AlphaBlend, 2);
    glSetCurrentRasterState(glHandleizeRasterState());

    unsigned long matrixHandle = glAllocMatrix();
    if (matrixHandle != (unsigned long)-1)
    {
        glSetMatrix(matrixHandle, objMatrix);
    }
    glSetCurrentMatrix(matrixHandle);

    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTexture(textureHandle, GLTT_Diffuse);
    glSetCurrentTextureState(glHandleizeTextureState());
    glSetCurrentProgram(UnlitProgram);

    nlColour c = { 0, 0, 0, 0xFF };
    u8 lightenAmount = (u8)(255.0f * intensity);
    c.c[0] = lightenAmount;
    c.c[1] = lightenAmount;
    c.c[2] = lightenAmount;

    if (mesh.Begin(prim.vertCount, (eGLPrimitive)nPrimType, 3, stream_decl, false))
    {
        pPosition = (nlVector3*)prim.position;
        int index = 0;
        while (index < prim.vertCount)
        {
            mesh.Colour(c);
            mesh.Texcoord(*pTexcoord);
            mesh.Vertex(*pPosition);
            pTexcoord++;
            pPosition++;
            index++;
        }

        if (!mesh.End())
        {
            return;
        }

        glViewAttachModel(GLV_ElectricFence, mesh.GetModel());
    }
}

/**
 * Offset/Address/Size: 0x89C | 0x8016B8CC | size: 0x420
 * TODO: 96.4% match - remaining register allocation diffs in the active-fence
 *       lookup and inlined DrawPrimitive pointer/index assignments.
 */
void RenderElectricFence(EmissionController& ec)
{
    extern float sfFadeOutTime;
    extern float sfGridTextureSize;
    extern const unsigned long GridTexture;

    EmissionController* pController = &ec;
    ElectricFenceData* pElectricFenceData = NULL;
    ElectricFenceData* p = ElectricFenceData::sActiveElectricFences.m_pStart;
    while (p != NULL)
    {
        if (p->mpEmissionController == pController)
        {
            pElectricFenceData = p;
            break;
        }
        p = p->next;
    }

    float intensity = 1.0f;
    float remainingTime = pController->GetRemainingTime();
    if (remainingTime < sfFadeOutTime)
    {
        intensity = remainingTime / sfFadeOutTime;
    }

    if (pElectricFenceData == NULL)
    {
        ElectricFenceData* data = NULL;

        if (ElectricFenceData::sElectricFenceDataPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&ElectricFenceData::sElectricFenceDataPool, sizeof(ElectricFenceData));
        }

        SlotPoolEntry* freeSlot = ElectricFenceData::sElectricFenceDataPool.m_FreeList;
        if (freeSlot != NULL)
        {
            data = (ElectricFenceData*)freeSlot;
            ElectricFenceData::sElectricFenceDataPool.m_FreeList = freeSlot->m_next;
        }

        if (data != NULL)
        {
            data = new (data) ElectricFenceData(pController);
        }

        pElectricFenceData = data;
    }

    if (pElectricFenceData == NULL)
    {
        return;
    }

    if (pElectricFenceData->mbIsFlat)
    {
        nlVector3 normal;
        ((unsigned long*)&normal)[0] = ((unsigned long*)&pElectricFenceData->mNormal)[0];
        ((unsigned long*)&normal)[1] = ((unsigned long*)&pElectricFenceData->mNormal)[1];
        ((unsigned long*)&normal)[2] = ((unsigned long*)&pElectricFenceData->mNormal)[2];

        glSetDefaultState(true);
        glSetRasterState(GLS_AlphaBlend, 2);
        glSetRasterState(GLS_Culling, 0);
        glSetRasterState(GLS_DepthWrite, 0);
        glSetRasterState(GLS_DepthTest, 1);
        glSetCurrentRasterState(glHandleizeRasterState());
        glSetCurrentTexture(GridTexture, GLTT_Diffuse);
        glSetTextureState(GLTS_DiffuseWrap, 0);
        glSetCurrentTextureState(glHandleizeTextureState());

        RenderElectricFenceFlat(pElectricFenceData->mPosition, normal, intensity);

        glSetDefaultState(false);
        return;
    }

    nlMatrix4 matrix;
    matrix.SetIdentity();

    DrawPrimitive(*pElectricFenceData->mpGeometry, matrix, GLP_TriStrip, GridTexture, intensity);
}

/**
 * Offset/Address/Size: 0xC8C | 0x8016BCBC | size: 0x220
 */
void EmitElectricFenceBallEffect(const nlVector3& pos, const nlVector3& dir, unsigned long emitterID, bool bNoSpark)
{
    ElectricFenceData* data;
    const char* groupName;
    EmissionController* controller;

    if (g_pGame->mbCaptainShotToScoreOn)
        return;

    nlVector3 clampedPos;
    ((unsigned long*)&clampedPos)[0] = ((unsigned long*)&pos)[0];
    ((unsigned long*)&clampedPos)[1] = ((unsigned long*)&pos)[1];
    ((unsigned long*)&clampedPos)[2] = ((unsigned long*)&pos)[2];

    float goalLineX = cField::GetGoalLineX(1U);
    float absPosX = (float)__fabs(clampedPos.f.x);
    if ((float)__fabs(absPosX - goalLineX) < 0.2f)
    {
        if (clampedPos.f.x > 0.0f)
        {
            clampedPos.f.x = goalLineX;
        }
        else
        {
            clampedPos.f.x = -goalLineX;
        }
    }

    groupName = bNoSpark ? "electric_fence_nospark" : "electric_fence";

    if (!EmissionManager::IsPlaying(emitterID, fxGetGroup(groupName)))
    {
        controller = EmissionManager::Create(fxGetGroup(groupName), 0);
        controller->m_uUserData = emitterID;
        controller->SetPosition(clampedPos);

        float angle = nlATan2f(dir.f.y, dir.f.x);
        data = NULL;
        controller->m_aFacing = (u16)(10430.378f * angle);

        if (ElectricFenceData::sElectricFenceDataPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&ElectricFenceData::sElectricFenceDataPool, sizeof(ElectricFenceData));
        }
        SlotPoolEntry* freeSlot = ElectricFenceData::sElectricFenceDataPool.m_FreeList;
        if (freeSlot != NULL)
        {
            data = (ElectricFenceData*)freeSlot;
            ElectricFenceData::sElectricFenceDataPool.m_FreeList = freeSlot->m_next;
        }

        new (data) ElectricFenceData(controller);

        {
            Function<EmissionController&> updateCb;
            updateCb.mTag = FREE_FUNCTION;
            updateCb.mFreeFunction = RenderElectricFence;
            controller->SetUpdateCallback(updateCb);
        }

        Function<EmissionController&> finishedCb;
        finishedCb.mTag = FREE_FUNCTION;
        finishedCb.mFreeFunction = ElectricFenceFinished;
        controller->SetFinishedCallback(finishedCb);
    }
}

/**
 * Offset/Address/Size: 0xAB8 | 0x8016BAE8 | size: 0x1D4
 */
static inline void EmitElectricFenceCharacterEffectImpl(const nlVector3& pos, const nlVector3& dir, unsigned long emitterID)
{
    if (!EmissionManager::IsPlaying(emitterID, fxGetGroup("fx_electric_fence_char")))
    {
        EmissionController* controller = EmissionManager::Create(fxGetGroup("fx_electric_fence_char"), 0);
        controller->m_uUserData = emitterID;
        controller->SetPosition(pos);

        float angle = nlATan2f(dir.f.y, dir.f.x);
        ElectricFenceData* data = NULL;
        controller->m_aFacing = (u16)(10430.378f * angle);

        if (ElectricFenceData::sElectricFenceDataPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&ElectricFenceData::sElectricFenceDataPool, sizeof(ElectricFenceData));
        }
        SlotPoolEntry* freeSlot = ElectricFenceData::sElectricFenceDataPool.m_FreeList;
        if (freeSlot != NULL)
        {
            data = (ElectricFenceData*)freeSlot;
            ElectricFenceData::sElectricFenceDataPool.m_FreeList = freeSlot->m_next;
        }

        new (data) ElectricFenceData(controller);

        Function<EmissionController&> finishedCb;

        {
            Function<EmissionController&> updateCb;
            updateCb.mTag = FREE_FUNCTION;
            updateCb.mFreeFunction = RenderElectricFence;
            controller->SetUpdateCallback(updateCb);
        }

        finishedCb.mTag = FREE_FUNCTION;
        finishedCb.mFreeFunction = ElectricFenceFinished;
        controller->SetFinishedCallback(finishedCb);
    }

    SidelineExplodableManager::TriggerExplosions(pos, g_pGame->m_pGameTweaks->fBobombMediumRadius * g_pGame->m_pGameTweaks->fPowerupExplosionRadius);
}

void EmitElectricFenceCharacterEffect(const nlVector3& pos, const nlVector3& dir, unsigned long emitterID)
{
    if (g_pGame->mbCaptainShotToScoreOn)
        return;

    EmitElectricFenceCharacterEffectImpl(pos, dir, emitterID);
}

/**
 * Offset/Address/Size: 0xAB4 | 0x8016BAE4 | size: 0x4
 */
void InitializeElectricFence()
{
}

/**
 * Offset/Address/Size: 0x9E8 | 0x8016BA18 | size: 0xCC
 */
void FreeElectricFence()
{
    ElectricFenceData* node;
    while ((node = ElectricFenceData::sActiveElectricFences.m_pStart) != NULL)
    {
        if (node != NULL)
        {
            nlListRemoveElement<ElectricFenceData>(&ElectricFenceData::sActiveElectricFences.m_pStart, node, &ElectricFenceData::sActiveElectricFences.m_pEnd);
            ElectricFenceData::numAllocated--;
            ElectricFenceGeometry* geom = node->mpGeometry;
            if (geom != NULL)
            {
                SlotPoolEntry* oldFree = ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList;
                *(SlotPoolEntry**)geom = oldFree;
                ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList = (SlotPoolEntry*)geom;
            }
            SlotPoolEntry* oldFree2 = ElectricFenceData::sElectricFenceDataPool.m_FreeList;
            *(SlotPoolEntry**)node = oldFree2;
            ElectricFenceData::sElectricFenceDataPool.m_FreeList = (SlotPoolEntry*)node;
        }
    }
    SlotPoolBase::BaseFreeBlocks(&ElectricFenceData::sElectricFenceDataPool, sizeof(ElectricFenceData));
    SlotPoolBase::BaseFreeBlocks(&ElectricFenceGeometry::sElectricFenceGeometryPool, sizeof(ElectricFenceGeometry));
}

/**
 * Offset/Address/Size: 0x5A0 | 0x8016B5D0 | size: 0x448
 * TODO: 95.3% match - neg GPR r27 vs r30, loop body GPR/FPR swap (r27/r29,
 *   f24-f27 reversed) and instruction scheduling diffs in corner-case branch.
 */
ElectricFenceData::ElectricFenceData(EmissionController* pEmissionController)
{
    extern float sfGridTextureSize;
    extern float sfNumGridSquares;
    extern float sfAlignmentOffset1;
    extern float AIsgn(float);

    float half, grid, z_val, negHalf, posHalf, zTop, zBottom, step;

    mpEmissionController = pEmissionController;
    mfIntensity = 0.0f;
    mpGeometry = NULL;

    nlListAddEnd<ElectricFenceData>(&sActiveElectricFences.m_pStart, &sActiveElectricFences.m_pEnd, this);
    numAllocated++;

    mPosition = pEmissionController->GetPosition();

    f64 absY = __fabs(mPosition.f.y);
    f64 diffY = __fabs((f32)absY - cField::GetSidelineY(1U));
    f32 distanceFromSideline = (f32)diffY;

    f64 absX = __fabs(mPosition.f.x);
    f64 diffX = __fabs((f32)absX - cField::GetGoalLineX(1U));
    f32 distanceFromGoal = (f32)diffX;

    float cornerDiameter = 2.0f * cField::GetCornerRadius();
    if (distanceFromGoal > cornerDiameter || distanceFromSideline > cornerDiameter)
    {
        mbIsFlat = true;

        if (distanceFromGoal < distanceFromSideline)
        {
            u8 isXPositive = mPosition.f.x > 0.0f;
            if (isXPositive)
            {
                f32 goalX;
                if (isXPositive)
                    goalX = cField::GetGoalLineX(1U);
                else
                    goalX = -cField::GetGoalLineX(1U);
                mPosition.f.x = goalX;
            }

            mNormal.f.x = 0.0f;
            mNormal.f.y = 1.0f;
            mNormal.f.z = 0.0f;

            float increment = sfGridTextureSize / sfNumGridSquares;
            u8 neg = false;
            if (mPosition.f.y < 0.0f)
            {
                mPosition.f.y = -mPosition.f.y;
                neg = true;
            }

            mPosition.f.y = (increment * (float)floor(mPosition.f.y / increment)) + sfAlignmentOffset1;
            if (neg)
                mPosition.f.y = -mPosition.f.y;

            mPosition.f.z = increment * (float)floor(mPosition.f.z / increment);
        }
        else
        {
            u8 isYPositive = mPosition.f.y > 0.0f;
            if (isYPositive)
            {
                f32 sideY;
                if (isYPositive)
                    sideY = cField::GetSidelineY(1U);
                else
                    sideY = -cField::GetSidelineY(1U);
                mPosition.f.y = sideY;
            }

            mNormal.f.x = 1.0f;
            mNormal.f.y = 0.0f;
            mNormal.f.z = 0.0f;

            float increment = sfGridTextureSize / sfNumGridSquares;
            mPosition.f.x = increment * (float)floor(mPosition.f.x / increment);
            mPosition.f.z = increment * (float)floor(mPosition.f.z / increment);
        }
    }
    else
    {
        mbIsFlat = false;

        float increment = sfGridTextureSize / sfNumGridSquares;
        mPosition.f.z = increment * (float)floor(mPosition.f.z / increment);

        if (ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList == NULL)
            SlotPoolBase::BaseAddNewBlock(&ElectricFenceGeometry::sElectricFenceGeometryPool, sizeof(ElectricFenceGeometry));

        ElectricFenceGeometry* geom = NULL;
        SlotPoolEntry* freeSlot = ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList;
        if (freeSlot != NULL)
        {
            geom = (ElectricFenceGeometry*)freeSlot;
            ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList = freeSlot->m_next;
        }

        mpGeometry = geom;

        nlVector3 impactPosition = mPosition;

        cField::GetCornerRadius();
        cField::GetGoalLineX(1U);
        AIsgn(impactPosition.f.x);
        cField::GetSidelineY(1U);
        AIsgn(impactPosition.f.y);

        geom->vertCount = 32;

        nlVector3* outPosition = geom->position;
        nlVector2* outTexcoord = (nlVector2*)&geom->texcoord;

        half = 0.5f;
        grid = sfGridTextureSize;
        z_val = impactPosition.f.z;
        negHalf = half * (-grid);
        posHalf = half * grid;
        zTop = z_val + posHalf;
        zBottom = z_val + negHalf;
        step = (posHalf - negHalf) / 15.0f;

        for (s32 i = 0; i < 16; i++)
        {
            nlVector3 wallPoint;
            GetWallPoint(impactPosition, (((float)i) * step) + negHalf, 0.0f, wallPoint);

            float t = (float)i / 15.0f;

            outPosition[0].f.x = wallPoint.f.x;
            outPosition[0].f.y = wallPoint.f.y;
            outPosition[0].f.z = zBottom;
            outTexcoord[0].f.x = t;
            outTexcoord[0].f.y = 0.0f;

            outPosition[1].f.x = wallPoint.f.x;
            outPosition[1].f.y = wallPoint.f.y;
            outPosition[1].f.z = zTop;
            outTexcoord[1].f.x = t;
            outTexcoord[1].f.y = 1.0f;

            outPosition += 2;
            outTexcoord += 2;
        }
    }
}

/**
 * Offset/Address/Size: 0x58C | 0x8016B5BC | size: 0x14
 */
void DisplayElectricFence()
{
    sbIsElectricFenceBeingDisplayed = true;
    sfElectricFenceDisplayAngle = sfStartAngle;
}

/**
 * Offset/Address/Size: 0x560 | 0x8016B590 | size: 0x2C
 */
void StopDisplayingElectricFence()
{
    sbIsElectricFenceBeingDisplayed = false;
    EmissionManager::DestroyAll(true);
}

/**
 * Offset/Address/Size: 0x0 | 0x8016B030 | size: 0x560
 * TODO: 96.22% match - register/literal allocation and instruction ordering diffs
 *   in the scale/sideLineY else-branch, an extra reload from cNet::m_fNetWidth,
 *   and a duplicate beq around the placement-new of ElectricFenceData.
 */
void UpdateElectricFence(float fDeltaT)
{
    static unsigned long counter;
    static signed char init;
    static float timeSinceLastEffect;
    static signed char init2;
    if (nlTaskManager::m_pInstance->m_CurrState == 1)
        return;
    if (!sbIsElectricFenceBeingDisplayed)
        return;
    if (!init)
    {
        counter = 1;
        init = 1;
    }
    if (!init2)
    {
        timeSinceLastEffect = 0.0f;
        init2 = 1;
    }
    while (timeSinceLastEffect > sfTimeBetweenEffects)
    {
        float goalLineX = cField::GetGoalLineX(1U);
        float sideLineY = cField::GetSidelineY(1U);
        float randomAngleOffset = nlRandomf(-sfAngleRandomOffset, sfAngleRandomOffset, &nlDefaultSeed);
        nlVector3 pos = { 0.0f, 0.0f, 0.0f };
        u16 sinArg = (u16)(s32)(10430.378f * (3.1415927f * (sfElectricFenceDisplayAngle + randomAngleOffset) / 180.0f));
        pos.f.x = nlSin(sinArg);
        sinArg = (u16)(s32)(10430.378f * (3.1415927f * (sfElectricFenceDisplayAngle + randomAngleOffset) / 180.0f));
        pos.f.y = nlSin((u16)(sinArg + 0x4000));
        float scale;
        if (pos.f.x == 0.0f)
        {
            scale = sideLineY;
            sideLineY = 0.0f;
            randomAngleOffset = 1.0f;
        }
        else if ((float)pos.f.y == 0.0f)
        {
            scale = goalLineX;
            randomAngleOffset = 0.0f;
            sideLineY = 1.0f;
        }
        else
        {
            float goalLineScale = goalLineX / pos.f.x;
            float sideLineScale = sideLineY / pos.f.y;
            if (goalLineScale < 0.0f)
                goalLineScale = -goalLineScale;
            if (sideLineScale < 0.0f)
                sideLineScale = -sideLineScale;
            if (goalLineScale < sideLineScale)
            {
                scale = goalLineScale;
                randomAngleOffset = 0.0f;
                sideLineY = 1.0f;
            }
            else
            {
                scale = sideLineScale;
                randomAngleOffset = 1.0f;
                sideLineY = 0.0f;
            }
        }
        float zComp = pos.f.z;
        float yComp = pos.f.y;
        pos.f.x = scale * pos.f.x;
        pos.f.y = scale * yComp;
        pos.f.z = scale * zComp;
        pos.f.z = nlRandomf(0.0f, 5.0f, &nlDefaultSeed);
        if ((counter & 1) == 0)
        {
            pos.f.x = -pos.f.x;
        }
        float netWidth = cNet::m_fNetWidth;
        float netHeight = cNet::m_fNetHeight;
        if ((float)__fabs(pos.f.x - goalLineX) < 0.01)
        {
            if ((float)__fabs(pos.f.y) < netWidth)
            {
                pos.f.z = nlRandomf(netHeight, 5.0f, &nlDefaultSeed);
            }
        }
        u8 useNoSpark = !sbUseSparksDuringElectricFenceFlyBy;
        unsigned long counterVal = counter;
        counter = counterVal + 1;
        if (g_pGame->mbCaptainShotToScoreOn)
            goto next;
        {
            nlVector3 clampedPos;
            ((u32*)&clampedPos)[0] = ((u32*)&pos)[0];
            ((u32*)&clampedPos)[1] = ((u32*)&pos)[1];
            ((u32*)&clampedPos)[2] = ((u32*)&pos)[2];
            float goalLineX2 = cField::GetGoalLineX(1U);
            if ((float)__fabs((float)__fabs(clampedPos.f.x) - goalLineX2) < 0.2f)
            {
                if (clampedPos.f.x > 0.0f)
                {
                    clampedPos.f.x = goalLineX2;
                }
                else
                {
                    clampedPos.f.x = -goalLineX2;
                }
            }
            const char* groupName = useNoSpark != 0 ? "electric_fence_nospark" : "electric_fence";
            if (!EmissionManager::IsPlaying(counterVal, fxGetGroup(groupName)))
            {
                EmissionController* controller = EmissionManager::Create(fxGetGroup(groupName), 0);
                controller->m_uUserData = counterVal;
                controller->SetPosition(clampedPos);
                float atan = nlATan2f(randomAngleOffset, sideLineY);
                ElectricFenceData* data = NULL;
                controller->m_aFacing = (u16)(s32)(10430.378f * atan);
                if (ElectricFenceData::sElectricFenceDataPool.m_FreeList == NULL)
                {
                    SlotPoolBase::BaseAddNewBlock(&ElectricFenceData::sElectricFenceDataPool, sizeof(ElectricFenceData));
                }
                SlotPoolEntry* freeSlot = ElectricFenceData::sElectricFenceDataPool.m_FreeList;
                if (freeSlot != NULL)
                {
                    data = (ElectricFenceData*)freeSlot;
                    ElectricFenceData::sElectricFenceDataPool.m_FreeList = freeSlot->m_next;
                }
                new (data) ElectricFenceData(controller);
                {
                    Function<EmissionController&> updateCb;
                    updateCb.mTag = FREE_FUNCTION;
                    updateCb.mFreeFunction = RenderElectricFence;
                    controller->SetUpdateCallback(updateCb);
                }
                Function<EmissionController&> finishedCb;
                finishedCb.mTag = FREE_FUNCTION;
                finishedCb.mFreeFunction = ElectricFenceFinished;
                controller->SetFinishedCallback(finishedCb);
            }
        }
    next:
        timeSinceLastEffect = timeSinceLastEffect - sfTimeBetweenEffects;
    }
    timeSinceLastEffect = timeSinceLastEffect + fDeltaT;
    sfElectricFenceDisplayAngle = sfElectricFenceDisplayAngle + sfAngleAnimationRate * fDeltaT;
    float endAngle = sfStartAngle + 180.0f * (float)(s32)sNumRevolutionsToDisplay;
    if (sfElectricFenceDisplayAngle > endAngle)
    {
        sbIsElectricFenceBeingDisplayed = false;
    }
}
