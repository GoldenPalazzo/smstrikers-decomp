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
void RenderElectricFence(EmissionController&)
{
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
 */
ElectricFenceData::ElectricFenceData(EmissionController*)
{
    FORCE_DONT_INLINE;
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
