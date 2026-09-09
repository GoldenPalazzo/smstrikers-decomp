#include "Game/FE/feScene.h"

#include "Game/FE/feLibObject.h"
#include "Game/FE/feResourceManager.h"
#include "NL/nlDebug.h"
#include "NL/nlFileGC.h"
#include "NL/nlMemory.h"
#include "NL/gl/glMatrix.h"
#include "NL/nlDLRing.h"
#include "compat_shims/endian.h"

bool gSebringLoadPackageToVirtualMemory = false;

struct FE_FILE_HEADER
{
    u8 Thumbprint[4];
    port::be<u32> Version;
    port::be<u32> DataLength;
    port::be<u32> PointerTableLength;
};

class QueueResourceLoadCallback
{
public:
    void Callback(FEResourceHandle*);
    FEResourceManager* m_resourceManager;
};

class UnloadResourceCallback
{
public:
    void Callback(FEResourceHandle*);
    FEResourceManager* m_resourceManager;
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

// it doesn't relocate anymore, just byteswaps the offset
static inline void RelocatePointer(u32* pPointer, void* /*pData*/)
{
    *pPointer = bswap(*pPointer);
}

/**
 * Offset/Address/Size: 0xE0 | 0x80209E54 | size: 0x26C
 */
bool FEScene::LoadPackage(const char* szPackageFileName)
{
    nlFile* file;
    FE_FILE_HEADER FenHdr ATTRIBUTE_ALIGN(32);
    void* pData;
    u32* pPointerLocation;
    u32* pLastPointer;
    u32* pCurrentPointer;
    u32* pPointer;

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

    pPointerLocation = (u32*)nlMalloc(FenHdr.PointerTableLength, 0x20, true);
    nlRead(file, pPointerLocation, FenHdr.PointerTableLength);
    nlClose(file);

    unsigned long numPointerEntries = FenHdr.PointerTableLength / sizeof(u32);
    for (unsigned long i = 0; i < numPointerEntries; i++)
    {
        pPointerLocation[i] = bswap(pPointerLocation[i]);
    }

    pLastPointer = (u32*)((unsigned char*)pPointerLocation + (FenHdr.PointerTableLength & ~3));
    for (pCurrentPointer = pPointerLocation; pCurrentPointer < pLastPointer; pCurrentPointer++)
    {
        pPointer = (u32*)((unsigned char*)pData + *pCurrentPointer);
        RelocatePointer(pPointer, pData);
    }

    u8* pkgBase = (u8*)pData;
    u32 offsetComponentList = *(u32*)(pkgBase+0x00);
    u32 offsetFEPresentation = *(u32*)(pkgBase+0x04);
    u32 offsetResourceList = *(u32*)(pkgBase+0x08);
    u32 offsetFEObjectLibrary = *(u32*)(pkgBase+0x0C);
    u32 uUniqueID = bswap(*(u32*)(pkgBase+0x10));
    u32 uResourceCount = bswap(*(u32*)(pkgBase+0x14));

    TLComponent* pComponentList = (offsetComponentList != 0xFFFFFFFFu) ? (TLComponent*)(pkgBase + offsetComponentList) : nullptr;
    FEPresentation* pFEPresentation = (offsetFEPresentation != 0xFFFFFFFFu) ? (FEPresentation*)(pkgBase + offsetFEPresentation) : nullptr;
    FEResourceHandle* pResourceListHead = (offsetResourceList != 0xFFFFFFFFu) ? (FEResourceHandle*)(pkgBase + offsetResourceList) : nullptr;
    FELibObject* pFEObjectLibrary = (offsetFEObjectLibrary != 0xFFFFFFFFu) ? (FELibObject*)(pkgBase + offsetFEObjectLibrary) : nullptr;

#define OFFSETOF_M_NEXT_ON_DISK 0x00
#define OFFSETOF_M_PREV_ON_DISK 0x04
#define OFFSETOF_M_TYPE_ON_DISK 0x08
#define OFFSETOF_M_HASHID_ON_DISK 0x0C
#define OFFSETOF_M_BVALID_ON_DISK 0x10
    FEResourceHandle* head = nullptr;
    FEResourceHandle* tail = nullptr;
    u32 startOffset = offsetResourceList;
    u32 curOffset = startOffset;

    if (curOffset != 0xFFFFFFFFu)
    {
        do
        {
            u8* fileNode = pkgBase + curOffset;
            u32 nextOffset = *(u32*)(fileNode + OFFSETOF_M_NEXT_ON_DISK);

            eFEResourceType type = (eFEResourceType)bswap(*(u32*)(fileNode + OFFSETOF_M_TYPE_ON_DISK));
            unsigned long hashID = bswap(*(u32*)(fileNode + OFFSETOF_M_HASHID_ON_DISK));
            bool bValid = *(fileNode + OFFSETOF_M_BVALID_ON_DISK) != 0;

            FEResourceHandle* newNode = (FEResourceHandle*)nlMalloc(sizeof(FEResourceHandle), 8, false);
            newNode->m_next = nullptr;
            newNode->m_prev = nullptr;
            newNode->m_type = type;
            newNode->m_hashID = hashID;
            newNode->m_bValid = bValid;

            if (tail)
            {
                tail->m_next = newNode;
                newNode->m_prev = tail;
            }
            else
                head = newNode;
            tail = newNode;
            curOffset = nextOffset;
        } while (curOffset != 0xFFFFFFFFu && curOffset != startOffset);

        if (tail && head)
        {
            tail->m_next = head;
            head->m_prev = tail;
        }
    }


    nlFree(pPointerLocation);

    m_pFEPackage = (FEPackage*)nlMalloc(sizeof(FEPackage), 8, false);
    m_pFEPackage->m_pComponentList = pComponentList;
    m_pFEPackage->m_pFEPresentation = pFEPresentation;
    m_pFEPackage->m_pResourceList = tail;
    m_pFEPackage->m_pFEObjectLibrary = pFEObjectLibrary;
    m_pFEPackage->m_uUniqueID = uUniqueID;
    m_pFEPackage->m_uResourceCount = uResourceCount;
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
