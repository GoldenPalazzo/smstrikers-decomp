#include "Game/Camera/animcam.h"

#include "NL/nlFile.h"
#include "NL/nlMemory.h"
#include "Game/Render/depthoffield.h"
#include "Game/FixedUpdateTask.h"

/**
 * Offset/Address/Size: 0x113C | 0x801A5D30 | size: 0x2C
 */
void EnableDofDebug()
{
    if (DepthOfFieldManager::instance.m_bDebugView)
    {
        DepthOfFieldManager::instance.m_bDebugView = 0;
    }
    else
    {
        DepthOfFieldManager::instance.m_bDebugView = 1;
    }
}

static inline void* nlGetChunkData(nlChunk* chunk)
{
    u32 alignField = chunk->m_ID & 0x7F000000;
    u32 isAligned = ((-alignField) | alignField) >> 31;
    if (isAligned != 0)
    {
        u32 alignment = 1u << (alignField >> 24);
        u32 ptr = (u32)chunk + alignment;
        ptr += 7;
        ptr &= ~(alignment - 1);
        return (void*)ptr;
    }
    return (void*)((u8*)chunk + 8);
}

static inline nlChunk* nlGetNextChunk(nlChunk* chunk)
{
    return (nlChunk*)((u8*)chunk + chunk->m_Size + 8);
}

/**
 * Offset/Address/Size: 0xCA4 | 0x801A5898 | size: 0x498
 * TODO: 96.07% match - register allocation diffs in nlGetChunkData inline
 * expansion for vector/quaternion copy cases (andc destination r3 vs r5,
 * li initialization order r6/r7 swap)
 */
bool LoadAnimCameraData(nlChunk* outerChunk, nlChunk* outerEnd, cCameraData* pAnimCameraData, bool ownsKeyData)
{
    pAnimCameraData->ownsKeyData = ownsKeyData;
    while (outerChunk < outerEnd)
    {
        u32 id = outerChunk->m_ID;
        u32 type = id & 0x80FFFFFF;
        switch (type)
        {
        case 0x15508:
            pAnimCameraData->m_uKeyCount = *(u32*)nlGetChunkData(outerChunk);
            if (ownsKeyData)
            {
                pAnimCameraData->cameraPos = (nlVector3*)nlMalloc(pAnimCameraData->m_uKeyCount * sizeof(nlVector3), 8, false);
                pAnimCameraData->targetPos = (nlVector3*)nlMalloc(pAnimCameraData->m_uKeyCount * sizeof(nlVector3), 8, false);
                pAnimCameraData->cameraRot = (nlQuaternion*)nlMalloc(pAnimCameraData->m_uKeyCount * sizeof(nlQuaternion), 8, false);
                pAnimCameraData->fFOV = (float*)nlMalloc(pAnimCameraData->m_uKeyCount * sizeof(float), 8, false);
                pAnimCameraData->fFocalLength = (float*)nlMalloc(pAnimCameraData->m_uKeyCount * sizeof(float), 8, false);
            }
            else
            {
                pAnimCameraData->cameraPos = NULL;
                pAnimCameraData->targetPos = NULL;
                pAnimCameraData->cameraRot = NULL;
                pAnimCameraData->fFOV = NULL;
                pAnimCameraData->fFocalLength = NULL;
            }
            break;
        case 0x15509:
        {
            nlVector3* v3Pos = (nlVector3*)nlGetChunkData(outerChunk);
            if (ownsKeyData)
            {
                unsigned long offset = 0;
                unsigned long i = 0;
                while (i < pAnimCameraData->m_uKeyCount)
                {
                    *(nlVector3*)((u8*)pAnimCameraData->cameraPos + offset) = *v3Pos;
                    i++;
                    offset += sizeof(nlVector3);
                    v3Pos++;
                }
            }
            else
                pAnimCameraData->cameraPos = v3Pos;
            break;
        }
        case 0x1550C:
        {
            nlVector3* v3Pos = (nlVector3*)nlGetChunkData(outerChunk);
            if (ownsKeyData)
            {
                unsigned long offset = 0;
                unsigned long i = 0;
                while (i < pAnimCameraData->m_uKeyCount)
                {
                    *(nlVector3*)((u8*)pAnimCameraData->targetPos + offset) = *v3Pos;
                    i++;
                    offset += sizeof(nlVector3);
                    v3Pos++;
                }
            }
            else
                pAnimCameraData->targetPos = v3Pos;
            break;
        }
        case 0x15511:
        {
            nlQuaternion* rot = (nlQuaternion*)nlGetChunkData(outerChunk);
            if (ownsKeyData)
            {
                unsigned long offset = 0;
                unsigned long i = 0;
                while (i < pAnimCameraData->m_uKeyCount)
                {
                    *(nlQuaternion*)((u8*)pAnimCameraData->cameraRot + offset) = *rot;
                    i++;
                    offset += sizeof(nlQuaternion);
                    rot++;
                }
            }
            else
                pAnimCameraData->cameraRot = rot;
            break;
        }
        case 0x1550F:
            if (ownsKeyData)
            {
                unsigned long i = 0;
                while (i < pAnimCameraData->m_uKeyCount)
                {
                    pAnimCameraData->fFOV[i] = ((float*)nlGetChunkData(outerChunk))[i];
                    i++;
                }
            }
            else
                pAnimCameraData->fFOV = (float*)nlGetChunkData(outerChunk);
            break;
        case 0x15510:
            if (ownsKeyData)
            {
                unsigned long i = 0;
                while (i < pAnimCameraData->m_uKeyCount)
                {
                    pAnimCameraData->fFocalLength[i] = ((float*)nlGetChunkData(outerChunk))[i];
                    i++;
                }
            }
            else
                pAnimCameraData->fFocalLength = (float*)nlGetChunkData(outerChunk);
            break;
        }
        outerChunk = nlGetNextChunk(outerChunk);
    }
    return true;
}

typedef bool (*LoadAnimCameraDataFn)(nlChunk*, nlChunk*, cCameraData*, const char*);

/**
 * Offset/Address/Size: 0xBF8 | 0x801A57EC | size: 0xAC
 */
bool cAnimCamera::LoadCameraAnimation(nlChunk* begin, nlChunk* end, const char* cameraName, bool ownsKeyData)
{
    cCameraData* pData;
    void* mem = nlMalloc(0x24, 8, false);
    pData = (cCameraData*)mem;
    if (mem != NULL)
    {
        ((cCameraData*)mem)->next = NULL;
        ((cCameraData*)mem)->m_uHashID = 0;
        ((cCameraData*)mem)->m_uKeyCount = 0;
        ((cCameraData*)mem)->cameraPos = NULL;
        ((cCameraData*)mem)->targetPos = NULL;
        ((cCameraData*)mem)->cameraRot = NULL;
        ((cCameraData*)mem)->fFOV = NULL;
        ((cCameraData*)mem)->fFocalLength = NULL;
    }
    pData->m_uHashID = nlStringLowerHash((const char*)end);
    bool result = ((LoadAnimCameraDataFn)LoadAnimCameraData)((nlChunk*)this, begin, pData, cameraName);
    nlListAddStart(&m_cameraDataList, pData, (cCameraData**)NULL);
    return result;
}

/**
 * Offset/Address/Size: 0xB14 | 0x801A5708 | size: 0xE4
 */
bool cAnimCamera::LoadCameraAnimation(const char* szFilename, const char* szCameraName, bool ownsKeyData)
{
    bool result;
    unsigned long uSize = 0;
    nlChunk* end;
    nlChunk* begin;
    cCameraData* pCamData;
    void* pData = nlLoadEntireFile(szFilename, &uSize, 0x20, (eAllocType)0);
    if (pData == NULL)
    {
        return false;
    }
    begin = (nlChunk*)((u8*)pData + 8);
    end = (nlChunk*)((u8*)pData + ((nlChunk*)pData)->m_Size + 8);

    void* mem = nlMalloc(0x24, 8, false);
    pCamData = (cCameraData*)mem;
    if (mem != NULL)
    {
        ((cCameraData*)mem)->next = NULL;
        ((cCameraData*)mem)->m_uHashID = 0;
        ((cCameraData*)mem)->m_uKeyCount = 0;
        ((cCameraData*)mem)->cameraPos = NULL;
        ((cCameraData*)mem)->targetPos = NULL;
        ((cCameraData*)mem)->cameraRot = NULL;
        ((cCameraData*)mem)->fFOV = NULL;
        ((cCameraData*)mem)->fFocalLength = NULL;
    }
    pCamData->m_uHashID = nlStringLowerHash(szCameraName);
    result = LoadAnimCameraData(begin, end, pCamData, ownsKeyData);
    nlListAddStart(&m_cameraDataList, pCamData, (cCameraData**)NULL);
    operator delete(pData);
    return result;
}

/**
 * Offset/Address/Size: 0xAE8 | 0x801A56DC | size: 0x2C
 */
void cAnimCamera::FreeCameraAnimations()
{
    nlDeleteList(&m_cameraDataList);
    m_cameraDataList = NULL;
}

/**
 * Offset/Address/Size: 0xA54 | 0x801A5648 | size: 0x94
 */
cAnimCamera::cAnimCamera()
{
    m_bUseSimulationTime = false;
    m_LetManagerDoUpdate = true;
    m_bUnusedPad = false;
    m_fAnimationTime = 0.0f;
    m_fAnimationSpeed = 1.0f;
    m_fLastSimulationTime = -1.0f;
    m_pActiveCameraData = NULL;
    mFacingAngle = 0;
    m_EndOfAnimationCallback = NULL;
    nlVec3Set(m_vecTarget, 0.0f, 0.0f, 0.0f);
    nlVec3Set(m_OffsetPos, 0.0f, 0.0f, 0.0f);
    nlVec3Set(m_Mirror, 1.0f, 1.0f, 1.0f);
    m_pActiveCameraData = m_cameraDataList;
    m_bCyclic = true;
}

/**
 * Offset/Address/Size: 0x9F8 | 0x801A55EC | size: 0x5C
 */
cAnimCamera::~cAnimCamera()
{
}

/**
 * Offset/Address/Size: 0x41C | 0x801A5010 | size: 0x5DC
 */
void cAnimCamera::BuildAnimViewMatrix(nlMatrix4&)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x410 | 0x801A5004 | size: 0xC
 */
void cAnimCamera::UnselectCameraAnimation()
{
    m_pActiveCameraData = 0;
}

/**
 * Offset/Address/Size: 0x39C | 0x801A4F90 | size: 0x74
 */
void cAnimCamera::SelectCameraAnimation(const char* name)
{
    m_fLastSimulationTime = -1.0f;
    u32 hash = nlStringLowerHash(name);

    cCameraData* pData = m_cameraDataList;
    while (pData != nullptr)
    {
        if (pData->m_uHashID == hash)
        {
            m_pActiveCameraData = pData;
            return;
        }
        pData = pData->next;
    }

    m_fAnimationTime = 0.0f;
    BuildAnimViewMatrix(m_matView);
}

/**
 * Offset/Address/Size: 0x34C | 0x801A4F40 | size: 0x50
 */
bool cAnimCamera::CameraAnimationExists(const char* name) const
{
    u32 hash = nlStringLowerHash(name);
    cCameraData* data = m_cameraDataList;
    while (data != NULL)
    {
        if (data->m_uHashID == hash)
        {
            return true;
        }
        data = data->next;
    }
    return false;
}

/**
 * Offset/Address/Size: 0x2AC | 0x801A4EA0 | size: 0xA0
 */
void cAnimCamera::FreeCameraAnimation(const char* szCameraName)
{
    unsigned long uHashID = nlStringLowerHash(szCameraName);
    cCameraData* pCameraData = m_cameraDataList;
    while (pCameraData != NULL)
    {
        if (pCameraData->m_uHashID == uHashID)
        {
            nlListRemoveElement(&m_cameraDataList, pCameraData, (cCameraData**)NULL);
            if (pCameraData != NULL)
            {
                if (pCameraData->ownsKeyData)
                {
                    operator delete[](pCameraData->cameraPos);
                    operator delete[](pCameraData->targetPos);
                    operator delete[](pCameraData->cameraRot);
                    operator delete[](pCameraData->fFOV);
                    operator delete[](pCameraData->fFocalLength);
                }
                operator delete(pCameraData);
            }
            return;
        }
        pCameraData = pCameraData->next;
    }
}

static inline float GetSimulationDelta(float lastTime, float& simTime)
{
    simTime = FixedUpdateTask::mSimulationTime;
    return simTime - lastTime;
}

/**
 * Offset/Address/Size: 0x150 | 0x801A4D44 | size: 0x15C
 */
void cAnimCamera::Update(float dt)
{
    if (!m_LetManagerDoUpdate)
    {
        return;
    }

    if (m_bUseSimulationTime)
    {
        float lastTime = m_fLastSimulationTime;
        float duration = 1.0f;
        if (lastTime < duration)
        {
            m_fLastSimulationTime = FixedUpdateTask::mSimulationTime;
        }
        else
        {
            float simTime;
            float delta = GetSimulationDelta(lastTime, simTime);
            if (m_pActiveCameraData != NULL)
            {
                duration = (float)(m_pActiveCameraData->m_uKeyCount - 1) / 30.0f;
            }
            m_fAnimationTime += (delta * m_fAnimationSpeed) / duration;
            m_fLastSimulationTime = simTime;
        }
    }
    else
    {
        float duration;
        if (m_pActiveCameraData != NULL)
        {
            duration = (float)(m_pActiveCameraData->m_uKeyCount - 1) / 60.0f;
        }
        else
        {
            duration = 0.0f;
        }
        m_fAnimationTime += (dt * m_fAnimationSpeed) / duration;
    }

    if (m_fAnimationTime >= 1.0f)
    {
        if (m_bCyclic)
        {
            m_fAnimationTime -= 1.0f;
        }
        else
        {
            m_fAnimationTime = 1.0f;
        }
        if (m_EndOfAnimationCallback != NULL)
        {
            m_EndOfAnimationCallback();
        }
    }

    BuildAnimViewMatrix(m_matView);
}

/**
 * Offset/Address/Size: 0x0 | 0x801A4BF4 | size: 0x150
 */
void cAnimCamera::ManualUpdate(float dt)
{
    if (m_bUseSimulationTime)
    {
        float lastTime = m_fLastSimulationTime;
        float duration = 1.0f;
        if (lastTime < duration)
        {
            m_fLastSimulationTime = FixedUpdateTask::mSimulationTime;
        }
        else
        {
            float simTime = FixedUpdateTask::mSimulationTime;
            float delta = simTime - lastTime;
            if (m_pActiveCameraData != NULL)
            {
                duration = (float)(m_pActiveCameraData->m_uKeyCount - 1) / 60.0f;
            }
            m_fAnimationTime += (delta * m_fAnimationSpeed) / duration;
            m_fLastSimulationTime = simTime;
        }
    }
    else
    {
        float duration;
        if (m_pActiveCameraData != NULL)
        {
            duration = (float)(m_pActiveCameraData->m_uKeyCount - 1) / 60.0f;
        }
        else
        {
            duration = 1.0f;
        }
        m_fAnimationTime += (dt * m_fAnimationSpeed) / duration;
    }

    if (m_fAnimationTime >= 1.0f)
    {
        if (m_bCyclic)
        {
            m_fAnimationTime -= 1.0f;
        }
        else
        {
            m_fAnimationTime = 1.0f;
        }
        if (m_EndOfAnimationCallback != NULL)
        {
            m_EndOfAnimationCallback();
        }
    }

    BuildAnimViewMatrix(m_matView);
}
