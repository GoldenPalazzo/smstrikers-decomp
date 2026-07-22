#include "Game/Render/ElectricFence.h"

#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EmissionController.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Field.h"
#include "Game/Game.h"
#include "Game/GameTweaks.h"
#include "Game/GL/GLMeshWriter.h"
#include "Game/GL/gluMeshWriter.h"
#include "Game/Net.h"
#include "Game/Render/SidelineExplodable.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"
#include "NL/gl/glView.h"
#include "NL/nlMath.h"
#include "NL/nlTask.h"

#include <math.h>

static float sfGridTextureSize = 7.0f;
static float sfNumGridSquares = 16.0f;
static float sfFadeOutTime = 0.2f;
static float sfAlignmentOffset1 = 0.25f;
static float sfAlignmentOffset2 = 0.13f;
static float sfAngleAnimationRate = 100.0f;
static float sfTimeBetweenEffects = 0.02f;
static int sNumRevolutionsToDisplay = 2;
static float sfAngleRandomOffset = 10.0f;
static float sfStartAngle = 180.0f;
static bool sbUseSparksDuringElectricFenceFlyBy = true;
static bool sbIsElectricFenceBeingDisplayed;
static float sfElectricFenceDisplayAngle;

const unsigned long UnlitProgram = glGetProgram("3d unlit");
const unsigned long LitProgram = glGetProgram("3d pointlit");
const unsigned long LightTexture = glGetTexture("global/lightramp");
const unsigned long BlackTexture = glGetTexture("global/black");
const unsigned long WhiteTexture = glGetTexture("global/white");
const unsigned long GridTexture = glGetTexture("global/grid");

int ElectricFenceData::numAllocated;
nlList<ElectricFenceData> ElectricFenceData::sActiveElectricFences((ElectricFenceData*)NULL, (ElectricFenceData*)NULL);
SlotPool<ElectricFenceData> ElectricFenceData::sElectricFenceDataPool(16, 16);
SlotPool<ElectricFenceGeometry> ElectricFenceGeometry::sElectricFenceGeometryPool(4, 4);

// /**
//  * Offset/Address/Size: 0x9C | 0x8016C6EC | size: 0x2C
//  */
// void nlListAddEnd<ElectricFenceData>(ElectricFenceData**, ElectricFenceData**, ElectricFenceData*)
// {
// }

static void GetWallPoint(const nlVector3& impactPosition, float xOffset, float zOffset, nlVector3& outPosition);

/**
 * Offset/Address/Size: 0x1370 | 0x8016C3A0 | size: 0x2B0
 */
static void GetWallPoint(const nlVector3& impactPosition, float xOffset, float zOffset, nlVector3& outPosition)
{
    /**
     * TODO: 99.71% match - r29/r31 register allocation swap (impactPosition ptr vs yIsPositive).
     */

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
static void ElectricFenceFinished(EmissionController& controller)
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

static inline void RenderElectricFenceFlat(const nlVector3& position, const nlVector3& normal, float intensity)
{
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
 * TODO: 99.05% match - allocation temp and DrawPrimitive geometry/matrix
 *       registers differ.
 */
static inline ElectricFenceData* FindElectricFenceData(EmissionController* pEmissionController)
{
    ElectricFenceData* data = ElectricFenceData::sActiveElectricFences.m_pStart;
    while (data != NULL)
    {
        if (data->mpEmissionController == pEmissionController)
        {
            return data;
        }
        data = data->next;
    }
    return NULL;
}

static inline ElectricFenceData* CreateAndAssignElectricFenceData(EmissionController* pController)
{
    ElectricFenceData* data = NULL;

    if (ElectricFenceData::sElectricFenceDataPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&ElectricFenceData::sElectricFenceDataPool, sizeof(ElectricFenceData));
    }

    if (ElectricFenceData::sElectricFenceDataPool.m_FreeList != NULL)
    {
        data = (ElectricFenceData*)ElectricFenceData::sElectricFenceDataPool.m_FreeList;
        ElectricFenceData::sElectricFenceDataPool.m_FreeList = ElectricFenceData::sElectricFenceDataPool.m_FreeList->next;
    }
    data = new ((u8*)data) ElectricFenceData(pController);
    return data;
}

static void RenderElectricFence(EmissionController& ec);
/**
 * Offset/Address/Size: 0xEAC | 0x8016BEDC | size: 0x420
 */
static void RenderElectricFence(EmissionController& ec)
{
    extern const unsigned long GridTexture;

    EmissionController* pController = &ec;
    ElectricFenceData* pElectricFenceData = FindElectricFenceData(pController);

    float intensity = 1.0f;
    float remainingTime = pController->GetRemainingTime();
    if (remainingTime < sfFadeOutTime)
    {
        intensity = remainingTime / sfFadeOutTime;
    }

    if (pElectricFenceData == NULL)
    {
        pElectricFenceData = CreateAndAssignElectricFenceData(pController);
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
        controller->m_aFacing = (u16)(10430.378f * angle);

        data = ElectricFenceData::sElectricFenceDataPool.Allocate();

        new (data) ElectricFenceData(controller);

        controller->SetUpdateCallback(RenderElectricFence);
        controller->SetFinishedCallback(ElectricFenceFinished);
    }
}

/**
 * Offset/Address/Size: 0xAB8 | 0x8016BAE8 | size: 0x1D4
 */
void EmitElectricFenceCharacterEffect(const nlVector3& pos, const nlVector3& dir, unsigned long emitterID)
{
    if (g_pGame->mbCaptainShotToScoreOn)
        return;

    if (!EmissionManager::IsPlaying(emitterID, fxGetGroup("fx_electric_fence_char")))
    {
        EmissionController* controller = EmissionManager::Create(fxGetGroup("fx_electric_fence_char"), 0);
        controller->m_uUserData = emitterID;
        controller->SetPosition(pos);

        float angle = nlATan2f(dir.f.y, dir.f.x);
        controller->m_aFacing = (u16)(10430.378f * angle);

        ElectricFenceData* data = ElectricFenceData::sElectricFenceDataPool.Allocate();

        new (data) ElectricFenceData(controller);

        controller->SetUpdateCallback(Function<EmissionController&>(RenderElectricFence));
        controller->SetFinishedCallback(Function<EmissionController&>(ElectricFenceFinished));
    }

    SidelineExplodableManager::TriggerExplosions(pos, g_pGame->m_pGameTweaks->fBobombMediumRadius * g_pGame->m_pGameTweaks->fPowerupExplosionRadius);
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

static inline void CreateElectricFenceGeometry(ElectricFenceGeometry& prim, const nlVector3& impactPosition)
{
    prim.vertCount = 32;

    nlVector3* pdst = prim.position;
    nlVector2* tdst = prim.texcoord;

    float startOffset = 0.5f * -sfGridTextureSize;
    float endOffset = 0.5f * sfGridTextureSize;
    float fDeltaSegmentOffset = (endOffset - startOffset) / 15.0f;
    float z0 = impactPosition.f.z + startOffset;
    float z1 = impactPosition.f.z + endOffset;

    for (int nSegment = 0; nSegment < 16;)
    {
        nlVector3 wallPoint;
        GetWallPoint(impactPosition, (((float)nSegment) * fDeltaSegmentOffset) + startOffset, 0.0f, wallPoint);

        int segment = nSegment;
        float wallY = wallPoint.f.y;
        float wallX = wallPoint.f.x;

        pdst[0].f.x = wallX;
        pdst[0].f.y = wallY;
        pdst[0].f.z = z0;
        tdst[0].f.x = (float)segment / 15.0f;
        tdst[0].f.y = 0.0f;

        float wY2 = wallPoint.f.y;
        float wX2 = wallPoint.f.x;
        pdst[1].f.x = wX2;
        pdst[1].f.y = wY2;
        pdst[1].f.z = z1;
        tdst[1].f.x = (float)segment / 15.0f;
        tdst[1].f.y = 1.0f;
        nSegment++;

        pdst += 2;
        tdst += 2;
    }
}

/**
 * Offset/Address/Size: 0x5A0 | 0x8016B5D0 | size: 0x448
 * 100% match.
 */
ElectricFenceData::ElectricFenceData(EmissionController* pEmissionController)
{
    extern float AIsgn(float);

    float zTop, zBottom, step, negHalf, posHalf, z_val, grid, half;

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

        ElectricFenceGeometry* geom = NULL;

        if (ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList == NULL)
            SlotPoolBase::BaseAddNewBlock(&ElectricFenceGeometry::sElectricFenceGeometryPool, sizeof(ElectricFenceGeometry));

        SlotPoolEntry* freeSlot = ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList;
        if (freeSlot != NULL)
        {
            geom = (ElectricFenceGeometry*)freeSlot;
            ElectricFenceGeometry::sElectricFenceGeometryPool.m_FreeList = freeSlot->next;
        }

        mpGeometry = geom;

        nlVector3 impactPosition = mPosition;
        ElectricFenceGeometry* activeGeom = mpGeometry;

        cField::GetCornerRadius();
        cField::GetGoalLineX(1U);
        AIsgn(impactPosition.f.x);
        cField::GetSidelineY(1U);
        AIsgn(impactPosition.f.y);

        CreateElectricFenceGeometry(*activeGeom, impactPosition);
        return;
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
 * TODO: 99.97% match - r0/r4 register swap while incrementing counter
 *   and computing useNoSpark.
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
        nlVector3 normal;
        u16 sinArg = (u16)(s32)(10430.378f * (3.1415927f * (sfElectricFenceDisplayAngle + randomAngleOffset) / 180.0f));
        pos.f.x = nlSin(sinArg);
        sinArg = (u16)(s32)(10430.378f * (3.1415927f * (sfElectricFenceDisplayAngle + randomAngleOffset) / 180.0f));
        pos.f.y = nlSin((u16)(sinArg + 0x4000));
        float scale;
        if (pos.f.x == 0.0f)
        {
            scale = sideLineY;
            normal.f.x = 0.0f;
            normal.f.y = 1.0f;
        }
        else if ((float)pos.f.y == 0.0f)
        {
            scale = goalLineX;
            normal.f.x = 1.0f;
            normal.f.y = 0.0f;
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
                normal.f.x = 1.0f;
                normal.f.y = 0.0f;
            }
            else
            {
                scale = sideLineScale;
                normal.f.x = 0.0f;
                normal.f.y = 1.0f;
            }
        }
        nlVec3Scale(pos, pos, scale);
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
        EmitElectricFenceBallEffect(pos, normal, counter++, !sbUseSparksDuringElectricFenceFlyBy);
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
