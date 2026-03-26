#include "Game/Render/ElectricFence.h"

static f32 sfStartAngle;
static f32 sfElectricFenceDisplayAngle;
static bool sbIsElectricFenceBeingDisplayed;

class EmissionManager
{
public:
    static void DestroyAll(bool);
    static bool IsPlaying(unsigned long, const EffectsGroup*);
    static EmissionController* Create(EffectsGroup*, unsigned short);
};

class GameTweaks
{
public:
    /* 0x00 */ u8 _pad00[0x138];
    /* 0x138 */ float fBobombMediumRadius;
    /* 0x13C */ u8 _pad13C[0x154 - 0x13C];
    /* 0x154 */ float fPowerupExplosionRadius;
};

class cGame
{
public:
    virtual ~cGame();
    /* 0x04 */ GameTweaks* m_pGameTweaks;
    /* 0x08 */ u8 _pad08[0x40 - 0x08];
    /* 0x40 */ bool mbCaptainShotToScoreOn;
};

extern cGame* g_pGame;

class cField
{
public:
    static float GetGoalLineX(unsigned int);
};

class SidelineExplodableManager
{
public:
    static void TriggerExplosions(const nlVector3&, float);
};

// /**
//  * Offset/Address/Size: 0x0 | 0x8016C898 | size: 0x64
//  */
// void SlotPool<ElectricFenceData>::~SlotPool()
// {
// }

// /**
//  * Offset/Address/Size: 0x1E4 | 0x8016C834 | size: 0x64
//  */
// void SlotPool<ElectricFenceGeometry>::~SlotPool()
// {
// }

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

extern "C" float GetCornerRadius__6cFieldFv();
extern "C" float GetSidelineY__6cFieldFUi(unsigned int);
extern "C" double floor(double);

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

    float cornerRadius = GetCornerRadius__6cFieldFv();
    float goalLineX = cField::GetGoalLineX(1U);
    float sidelineY = GetSidelineY__6cFieldFUi(1U);
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
enum eGLState
{
    GLS_DepthTest = 0,
    GLS_DepthWrite = 1,
    GLS_AlphaBlend = 5,
    GLS_Culling = 6,
};

enum eGLTextureType
{
    GLTT_Diffuse = 0,
};

enum eGLTextureState
{
    GLTS_DiffuseWrap = 0,
};

enum eGLPrimitive
{
    GLP_TriStrip = 1,
};

enum eGLStream
{
    GLStream_Position = 0,
    GLStream_Colour = 2,
    GLStream_Diffuse = 3,
};

struct nlColour;
struct glModel;

struct glModelStream
{
    unsigned long address;
    unsigned char id;
    unsigned char stride;
};

class glQuad3
{
public:
    void SetupRotatedRectangle(float, float, const nlMatrix4&, bool, bool);
    void SetColour(unsigned char, unsigned char, unsigned char, unsigned char);

    nlVector3 m_pos[4];
    nlVector2 m_uv[4];
    unsigned char m_colour[4][4];
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

void glSetDefaultState(bool);
unsigned long glSetRasterState(eGLState, unsigned long);
unsigned long glHandleizeRasterState();
unsigned long glSetCurrentRasterState(unsigned long = 0);
unsigned long glSetCurrentTexture(unsigned long, eGLTextureType);
unsigned long glSetTextureState(eGLTextureState, unsigned long);
unsigned long long glHandleizeTextureState();
unsigned long long glSetCurrentTextureState(unsigned long long);
unsigned long glSetCurrentProgram(unsigned long);
unsigned long glSetCurrentMatrix(unsigned long);
unsigned long glAllocMatrix();
void glSetMatrix(unsigned long, const nlMatrix4&);
bool glAttachQuad3(eGLView, unsigned long, glQuad3*, bool);
void glViewAttachModel(eGLView, const glModel*);

/**
 * Offset/Address/Size: 0x89C | 0x8016B8CC | size: 0x420
 * TODO: 90.4% match - register allocation diffs (ec r28→r29, pElectricFenceData r31→r28)
 *       due to -inline deferred vs -inline auto; placement new extra beq; matrix stack offset 0xa4→0x24
 */
void RenderElectricFence(EmissionController& ec)
{
    extern float sfFadeOutTime;
    extern float sfGridTextureSize;
    extern unsigned long UnlitProgram;
    extern unsigned long GridTexture;

    ElectricFenceData* pElectricFenceData = NULL;
    ElectricFenceData* p = ElectricFenceData::sActiveElectricFences.m_pStart;

    while (p != NULL)
    {
        if (p->mpEmissionController == &ec)
        {
            pElectricFenceData = p;
            break;
        }

        p = p->next;
    }

    float intensity = 1.0f;
    float remainingTime = ec.GetRemainingTime();
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
            data = new (data) ElectricFenceData(&ec);
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

        nlMatrix4 matrix;
        nlMakeRotationMatrixX(matrix, 1.5707964f);

        float angle = nlATan2f(normal.f.y, normal.f.x);
        nlMatrix4 matrix2;
        nlMakeRotationMatrixZ(matrix2, 0.0000958738f * (float)(u16)(s32)(10430.378f * angle));
        nlMultMatrices(matrix, matrix, matrix2);

        matrix.f.m41 = pElectricFenceData->mPosition.f.x;
        matrix.f.m42 = pElectricFenceData->mPosition.f.y;
        matrix.f.m43 = pElectricFenceData->mPosition.f.z;
        matrix.f.m44 = 1.0f;

        glQuad3 quad;
        quad.SetupRotatedRectangle(sfGridTextureSize, sfGridTextureSize, matrix, false, false);

        u8 lightenAmount = (u8)(255.0f * intensity);
        quad.SetColour(lightenAmount, lightenAmount, lightenAmount, 0xFF);
        glAttachQuad3(GLV_ElectricFence, 1, &quad, true);

        glSetDefaultState(false);
        return;
    }

    nlMatrix4 matrix;
    matrix.SetIdentity();

    ElectricFenceGeometry* prim = pElectricFenceData->mpGeometry;
    const eGLStream streams[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
    GLMeshWriter meshWriter;

    glSetDefaultState(true);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetRasterState(GLS_AlphaBlend, 2);
    glSetCurrentRasterState(glHandleizeRasterState());

    unsigned long matrixHandle = glAllocMatrix();
    if (matrixHandle != -1)
    {
        glSetMatrix(matrixHandle, matrix);
    }

    glSetCurrentMatrix(matrixHandle);

    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTexture(GridTexture, GLTT_Diffuse);
    glSetCurrentTextureState(glHandleizeTextureState());
    glSetCurrentProgram(UnlitProgram);

    unsigned long colourWord = 0x000000FF;
    ((unsigned char*)&colourWord)[0] = (u8)(255.0f * intensity);
    ((unsigned char*)&colourWord)[1] = ((unsigned char*)&colourWord)[0];
    ((unsigned char*)&colourWord)[2] = ((unsigned char*)&colourWord)[0];

    if (meshWriter.Begin(prim->vertCount, GLP_TriStrip, 3, streams, false))
    {
        nlVector3* pPos = prim->position;
        nlVector2* pUv = prim->texcoord;

        for (int i = 0; i < prim->vertCount; i++)
        {
            meshWriter.Colour(*(nlColour*)&colourWord);
            meshWriter.Texcoord(*pUv);
            meshWriter.Vertex(*pPos);

            pUv++;
            pPos++;
        }

        if (meshWriter.End())
        {
            glViewAttachModel(GLV_ElectricFence, meshWriter.GetModel());
        }
    }
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
 * TODO: 93.7% match - register allocation + instruction scheduling diffs
 *   due to -inline deferred (target) vs -inline auto (decomp.me) flag difference.
 *   fabs temp register (f0 vs f24/f25), neg GPR (r27 vs r30), loop body scheduling.
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
    f32 distanceFromSideline = (f32)__fabs((f32)absY - GetSidelineY__6cFieldFUi(1U));

    f64 absX = __fabs(mPosition.f.x);
    f32 distanceFromGoal = (f32)__fabs((f32)absX - cField::GetGoalLineX(1U));

    float cornerDiameter = 2.0f * GetCornerRadius__6cFieldFv();
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
                    sideY = GetSidelineY__6cFieldFUi(1U);
                else
                    sideY = -GetSidelineY__6cFieldFUi(1U);
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

        GetCornerRadius__6cFieldFv();
        cField::GetGoalLineX(1U);
        AIsgn(impactPosition.f.x);
        GetSidelineY__6cFieldFUi(1U);
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
 */
void UpdateElectricFence(float)
{
}
