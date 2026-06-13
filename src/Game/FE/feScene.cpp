#include "Game/FE/feScene.h"

#include "NL/nlDebug.h"
#include "NL/nlFileGC.h"
#include "NL/nlMemory.h"
#include "NL/gl/glMatrix.h"
#include "NL/nlDLRing.h"

extern bool gSebringLoadPackageToVirtualMemory;

struct FE_FILE_HEADER
{
    char Thumbprint[4];
    unsigned int Version;
    unsigned int DataLength;
    unsigned int PointerTableLength;
};

/**
 * Offset/Address/Size: 0x0 | 0x80209D74 | size: 0x24
 */
void FEScene::Update(float dt)
{
    m_pFEPackage->Update(dt);
}

/**
 * Offset/Address/Size: 0x24 | 0x80209D98 | size: 0x4
 */
void FEScene::AllResourcesLoadedCallback()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x28 | 0x80209D9C | size: 0x24
 */
void QueueResourceLoadCallback::Callback(FEResourceHandle* handle)
{
    m_resourceManager->QueueResourceLoad(handle);
}

/**
 * Offset/Address/Size: 0x4C | 0x80209DC0 | size: 0x70
 */
void FEScene::UnloadPackage()
{
    UnloadResourceCallback unloadResourceCallback;
    unloadResourceCallback.m_resourceManager = FEResourceManager::Instance();
    nlWalkRing<FEResourceHandle, UnloadResourceCallback>(m_pFEPackage->m_pResourceList, &unloadResourceCallback, &UnloadResourceCallback::Callback);
    FEResourceManager::Instance()->UnloadResource(&m_feSceneResourceHandle);
}

/**
 * Offset/Address/Size: 0xBC | 0x80209E30 | size: 0x24
 */
void UnloadResourceCallback::Callback(FEResourceHandle* handle)
{
    m_resourceManager->UnloadResource(handle);
}

static inline void RelocatePointer(unsigned long* pPointer, void* pData)
{
    unsigned long value = *pPointer;
    unsigned long mask = ~((value + 1) | ((unsigned long)-1 - value));
    unsigned long sum = value + (unsigned long)pData;
    mask = (unsigned long)((long)mask >> 31);
    *pPointer = sum & ~mask;
}

/**
 * Offset/Address/Size: 0xE0 | 0x80209E54 | size: 0x26C
 */
bool FEScene::LoadPackage(const char* szPackageFileName)
{
    nlFile* file;
    FE_FILE_HEADER FenHdr ATTRIBUTE_ALIGN(32);
    void* pData;
    unsigned long* pPointerLocation;
    unsigned long* pLastPointer;
    unsigned long* pCurrentPointer;
    unsigned long* pPointer;

    file = nlOpen(szPackageFileName);
    nlRead(file, &FenHdr, 0x10);

    if (gSebringLoadPackageToVirtualMemory)
    {
        pData = nlVirtualAlloc(FenHdr.DataLength, false);
        if (pData == NULL)
        {
            nlBreak();
        }
        nlReadToVirtualMemory(file, pData, FenHdr.DataLength, 0x4000);
    }
    else
    {
        pData = nlMalloc(FenHdr.DataLength, 0x20, false);
        nlRead(file, pData, FenHdr.DataLength);
    }

    pPointerLocation = (unsigned long*)nlMalloc(FenHdr.PointerTableLength, 0x20, true);
    nlRead(file, pPointerLocation, FenHdr.PointerTableLength);
    nlClose(file);

    m_pFEPackage = (FEPackage*)pData;

    pLastPointer = (unsigned long*)((unsigned char*)pPointerLocation + (FenHdr.PointerTableLength & ~3));
    for (pCurrentPointer = pPointerLocation; pCurrentPointer < pLastPointer; pCurrentPointer++)
    {
        pPointer = (unsigned long*)((unsigned char*)pData + *pCurrentPointer);
        RelocatePointer(pPointer, pData);
    }

    nlFree(pPointerLocation);

    file = (nlFile*)m_pFEPackage;
    QueueResourceLoadCallback cb;

    cb.m_resourceManager = FEResourceManager::Instance();
    m_feSceneResourceHandle.m_pFESceneContext = this;
    m_feSceneResourceHandle.m_hashID = m_uHashID;
    m_feSceneResourceHandle.m_next = 0;
    m_feSceneResourceHandle.m_prev = 0;
    m_feSceneResourceHandle.m_type = FERT_SCENE;

    FEResourceManager::Instance()->QueueResourceLoad(&m_feSceneResourceHandle);
    nlWalkRing<FEResourceHandle, QueueResourceLoadCallback>(((FEPackage*)file)->m_pResourceList, &cb, &QueueResourceLoadCallback::Callback);
    return true;
}

/**
 * Offset/Address/Size: 0x34C | 0x8020A0C0 | size: 0x7C
 */
FEScene::~FEScene()
{
    if (m_pFEPackage != NULL)
    {
        if (gSebringLoadPackageToVirtualMemory)
        {
            nlVirtualFree(m_pFEPackage);
        }
        else
        {
            delete[] m_pFEPackage;
        }
        m_pFEPackage = NULL;
        m_uHashID = 0;
    }
}

/**
 * Offset/Address/Size: 0x3C8 | 0x8020A13C | size: 0x8C
 */
FEScene::FEScene()
    : m_pFEPackage(NULL)
    , m_uHashID(0)
    , m_bValid(false)
    , m_uRenderView(0)
{
    nlVector3 FROM;
    nlVec3Set(FROM, 0.0f, 0.0f, 600.0f);
    nlVector3 TO;
    nlVec3Set(TO, 0.0f, 0.0f, 0.0f);
    nlVector3 UP;
    nlVec3Set(UP, 0.0f, 1.0f, 0.0f);
    glMatrixLookAt(m_matView, FROM, TO, UP);
}
