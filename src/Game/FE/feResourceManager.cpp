#include "Game/FE/FEResourceManager.h"
#include "Game/FE/feFontResource.h"
#include "Game/FE/feResourceManager.h"
#include "Game/FE/feScene.h"
#include "Game/FE/feSceneResource.h"
#include "Game/FE/feTextureResource.h"
#include "Game/Font/fontmanager.h"
#include "NL/nlMemory.h"
#include "NL/nlString.h"
#include "NL/gl/glTexture.h"
#include "NL/nlAVLTreeSlotPool.h"
#include "NL/nlDLListSlotPool.h"
#include "NL/glx/glxMemory.h"

static BundleFile* s_pOnDemandBundle;
static BundleFile* s_pPermanentBundle;
static unsigned char* s_pResourceLoadBuffer;
static nlAVLTreeSlotPool<unsigned long, FEResourceHandle*, DefaultKeyCompare<unsigned long> > s_loadedResourceList(0x100, 0);
static nlDLListSlotPool<FEResourceHandle*> pendingResourceQueue(0x80, 0);
static FEResourceHandle* s_pCurrentResourceBeingLoaded;
FESceneResource* s_pCurrentFESceneResourceContext;
static FESceneResource* s_pPermanentBundleSceneResource;

enum ResourceResult
{
    FERR_WaitingForResource = 0,
    FERR_AlreadyLoaded = 1,
};

/**
 * Offset/Address/Size: 0x0 | 0x8020D4A0 | size: 0xA8
 */
#pragma dont_inline on
void __force_dt()
{
    nlDLListSlotPool<FEResourceHandle*> x;
    x.~nlDLListSlotPool();
}
#pragma dont_inline off

/**
 * Offset/Address/Size: 0x108 | 0x8020D358 | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before saving LR; current MWCC output saves LR first.
 */
template void nlWalkDLRing<DLListEntry<FEResourceHandle*>,
    DLListContainerBase<FEResourceHandle*, BasicSlotPool<DLListEntry<FEResourceHandle*> > > >(
    DLListEntry<FEResourceHandle*>* head,
    DLListContainerBase<FEResourceHandle*, BasicSlotPool<DLListEntry<FEResourceHandle*> > >* callback,
    void (DLListContainerBase<FEResourceHandle*, BasicSlotPool<DLListEntry<FEResourceHandle*> > >::*)(DLListEntry<FEResourceHandle*>*));

/**
 * Offset/Address/Size: 0xD48 | 0x8020C888 | size: 0x1C
 */
FEResourceManager::FEResourceManager()
    : nlTask()
{
}

/**
 * Offset/Address/Size: 0xCEC | 0x8020C82C | size: 0x5C
 */
FEResourceManager::~FEResourceManager()
{
    Cleanup();
}

/**
 * Offset/Address/Size: 0xAF4 | 0x8020C634 | size: 0x1F8
 */
void FEResourceManager::Cleanup()
{
    if (s_pOnDemandBundle != NULL)
    {
        s_pOnDemandBundle->Close();
        delete s_pOnDemandBundle;
        s_pOnDemandBundle = NULL;
    }

    if (s_loadedResourceList.m_NumElements != 0)
    {
        nlPrintf("WARNING: Resources still loaded during FEResourceManager::Cleanup\n");
        nlPrintf("  Hash          Type\n");

        struct NodeStack
        {
            AVLTreeEntry<unsigned long, FEResourceHandle*>** data;
            u32 count;
        };

        NodeStack* stack = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);

        if (stack != NULL)
        {
            AVLTreeEntry<unsigned long, FEResourceHandle*>* node = s_loadedResourceList.m_Root;

            stack->data = (AVLTreeEntry<unsigned long, FEResourceHandle*>**)nlMalloc(
                (s_loadedResourceList.m_NumElements + 1) * sizeof(AVLTreeEntry<unsigned long, FEResourceHandle*>*), 8, false);
            stack->count = 0;

            if (node != NULL)
            {
                while (node->node.left != NULL)
                {
                    stack->data[stack->count] = node;
                    stack->count = stack->count + 1;
                    node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.left;
                }
                stack->data[stack->count] = node;
                stack->count = stack->count + 1;
            }
        }

        while (stack->count != 0)
        {
            AVLTreeEntry<unsigned long, FEResourceHandle*>* top = stack->data[stack->count - 1];
            FEResourceHandle* handle = top->value;
            nlPrintf("  0x%08X  %d\n", handle->m_hashID, handle->m_type);

            stack->count--;

            AVLTreeEntry<unsigned long, FEResourceHandle*>* current = stack->data[stack->count];
            AVLTreeEntry<unsigned long, FEResourceHandle*>* right = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)current->node.right;

            if (right != NULL)
            {
                while (right->node.left != NULL)
                {
                    stack->data[stack->count] = right;
                    stack->count = stack->count + 1;
                    right = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)right->node.left;
                }
                stack->data[stack->count] = right;
                stack->count = stack->count + 1;
            }
        }

        if (stack != NULL)
        {
            delete[] stack->data;
            delete stack;
        }
    }

    s_loadedResourceList.Clear();
}

inline void FEResourceManager::LoadPermanentTextures()
{
    FETextureResource* pTextureResource;
    u32 fileCount = s_pPermanentBundle->m_pHeader->nNumFiles;
    u32 fileIndex;

    for (fileIndex = 0; fileIndex < fileCount; fileIndex++)
    {
        BundleFileDirectoryEntry fileDirectoryEntry;

        if (!s_pPermanentBundle->GetFileInfoByIndex(fileIndex, &fileDirectoryEntry))
        {
            nlPrintf("FEResourceManager Error: Failed to get file information in permanent bundle!\n");
        }
        else
        {
            u32 fileLength = fileDirectoryEntry.m_length;
            u32 fileHash = fileDirectoryEntry.m_hash;

            pTextureResource = new (nlMalloc(sizeof(FETextureResource), 8, false)) FETextureResource();
            pTextureResource->m_hashID = fileHash;

            s_pResourceLoadBuffer = (unsigned char*)nlMalloc(fileLength, 0x20, true);
            s_pPermanentBundle->ReadFileByIndex(fileIndex, s_pResourceLoadBuffer, fileLength);
            glTextureAdd(pTextureResource->m_hashID, s_pResourceLoadBuffer, fileLength);

            delete[] s_pResourceLoadBuffer;
            s_pResourceLoadBuffer = NULL;

            pTextureResource->m_glTextureHandle = pTextureResource->m_hashID;

            AVLTreeNode* existingNodeB;
            AVLTreeNode* existingNodeA;
            FEResourceHandle* valueA = pTextureResource;
            u32 keyA = pTextureResource->m_hashID;
            s_loadedResourceList.AddAVLNode(
                (AVLTreeNode**)&s_loadedResourceList.m_Root,
                &keyA,
                &valueA,
                &existingNodeB,
                s_loadedResourceList.m_NumElements);
            if (existingNodeB == NULL)
            {
                s_loadedResourceList.m_NumElements++;
            }
            pTextureResource->m_bValid = true;

            delete[] s_pResourceLoadBuffer;
            s_pResourceLoadBuffer = NULL;

            FEResourceHandle* valueB = pTextureResource;
            u32 keyB = pTextureResource->m_hashID;
            s_loadedResourceList.AddAVLNode(
                (AVLTreeNode**)&s_loadedResourceList.m_Root,
                &keyB,
                &valueB,
                &existingNodeA,
                s_loadedResourceList.m_NumElements);
            if (existingNodeA == NULL)
            {
                s_loadedResourceList.m_NumElements++;
            }
            pTextureResource->m_bValid = true;
        }
    }
}

/**
 * Offset/Address/Size: 0x848 | 0x8020C388 | size: 0x2AC
 */
void FEResourceManager::LoadPermanentResourceBundle(const char* szBundleFileName)
{
    nlStrNCpy(m_szPermanentBundleFileName, szBundleFileName, 0x20);

    s_pPermanentBundleSceneResource = new (nlMalloc(sizeof(FESceneResource), 8, false)) FESceneResource();
    s_pPermanentBundleSceneResource->m_pFESceneContext = NULL;
    s_pPermanentBundleSceneResource->m_hashID = nlStringLowerHash("PermanentContext");
    s_pPermanentBundleSceneResource->m_next = NULL;
    s_pPermanentBundleSceneResource->m_prev = NULL;
    s_pPermanentBundleSceneResource->m_type = FERT_SCENE;

    FESceneResource* pPermanentSceneResource = s_pPermanentBundleSceneResource;

    if ((s_pCurrentFESceneResourceContext != NULL) && (s_pCurrentFESceneResourceContext != pPermanentSceneResource)
        && (pPermanentSceneResource != s_pCurrentFESceneResourceContext))
    {
        s_pCurrentFESceneResourceContext->m_pFESceneContext->m_bValid = true;
        s_pCurrentFESceneResourceContext->m_pFESceneContext->AllResourcesLoadedCallback();
    }

    pPermanentSceneResource->m_glResourceMarker = glplatResourceMark();
    pPermanentSceneResource->m_bValid = true;
    s_pCurrentFESceneResourceContext = pPermanentSceneResource;

    s_pPermanentBundle = new (nlMalloc(sizeof(BundleFile), 8, false)) BundleFile();
    s_pPermanentBundle->Open(m_szPermanentBundleFileName);

    LoadPermanentTextures();

    s_pPermanentBundle->Close();
    delete s_pPermanentBundle;
    s_pPermanentBundle = NULL;
}

/**
 * Offset/Address/Size: 0x7D0 | 0x8020C310 | size: 0x78
 */
bool FEResourceManager::OpenOnDemandResourceBundle(const char* szBundleFileName)
{
    nlStrNCpy(m_szOnDemandBundleFileName, szBundleFileName, 0x20);

    BundleFile* pBundle = new (nlMalloc(sizeof(BundleFile), 8, false)) BundleFile();
    s_pOnDemandBundle = pBundle;

    if (!pBundle->Open(szBundleFileName))
    {
        return false;
    }
    return true;
}

/**
 * Offset/Address/Size: 0x7CC | 0x8020C30C | size: 0x4
 */
void FEResourceManager::Initialize()
{
}

/**
 * Offset/Address/Size: 0x72C | 0x8020C26C | size: 0xA0
 */
void FEResourceManager::QueueResourceLoad(FEResourceHandle* pHandle)
{
    DLListEntry<FEResourceHandle*>* entry = NULL;

    if (pendingResourceQueue.m_Allocator.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&pendingResourceQueue.m_Allocator, sizeof(DLListEntry<FEResourceHandle*>));
    }

    SlotPoolEntry* freeEntry = pendingResourceQueue.m_Allocator.m_FreeList;
    if (freeEntry != NULL)
    {
        entry = (DLListEntry<FEResourceHandle*>*)freeEntry;
        pendingResourceQueue.m_Allocator.m_FreeList = freeEntry->m_next;
    }

    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->m_data = pHandle;
    }

    nlDLRingAddEnd(&pendingResourceQueue.m_Head, entry);
}

/**
 * Offset/Address/Size: 0x5F4 | 0x8020C134 | size: 0x138
 * TODO: 99.49% match - r5/r6 register swap for rootAddr vs nodeKey/cmpResult
 */
void FEResourceManager::UnloadResource(FEResourceHandle* pFeResourceHandle)
{
    switch (pFeResourceHandle->m_type)
    {
    case FERT_TEXTURE:
    {
        u32 key;
        FEResourceHandle** foundValue;
        u32 searchKey = pFeResourceHandle->m_hashID;
        AVLTreeEntry<unsigned long, FEResourceHandle*>* node = s_loadedResourceList.m_Root;

        while (node != nullptr)
        {
            unsigned long nodeKey = node->key;
            int cmpResult;
            if (searchKey == nodeKey)
            {
                cmpResult = 0;
            }
            else if (searchKey < nodeKey)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                if (&foundValue != nullptr)
                {
                    foundValue = (FEResourceHandle**)&node->value;
                }
                searchKey = 1;
                goto check_found;
            }
            else
            {
                if (cmpResult < 0)
                {
                    node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.left;
                }
                else
                {
                    node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.right;
                }
            }
        }

        searchKey = 0;
    check_found:
        if (!(u8)searchKey)
            break;
        if (*foundValue != pFeResourceHandle)
            break;

        key = pFeResourceHandle->m_hashID;
        {
            AVLTreeNode* removedNode = s_loadedResourceList.RemoveAVLNode(
                (AVLTreeNode**)&s_loadedResourceList.m_Root, &key, s_loadedResourceList.m_NumElements);

            if (removedNode != NULL)
            {
                removedNode->left = (AVLTreeNode*)s_loadedResourceList.m_Allocator.m_FreeList;
                s_loadedResourceList.m_Allocator.m_FreeList = (SlotPoolEntry*)removedNode;
                s_loadedResourceList.m_NumElements--;
            }
        }
        break;
    }
    case FERT_SCENE:
        glplatResourceRelease(*(unsigned long long*)((u8*)pFeResourceHandle + 0x18));
        break;
    }
}

static inline bool FindLoadedResourceByRoot(
    AVLTreeNode** root, const unsigned long& key, FEResourceHandle*** foundValue)
{
    AVLTreeEntry<unsigned long, FEResourceHandle*>* node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)*root;

    while (node != NULL)
    {
        int cmpResult;
        if (key == node->key)
        {
            cmpResult = 0;
        }
        else if (key < node->key)
        {
            cmpResult = -1;
        }
        else
        {
            cmpResult = 1;
        }

        if (cmpResult == 0)
        {
            if (foundValue != NULL)
            {
                *foundValue = (FEResourceHandle**)&node->value;
            }
            return true;
        }
        else
        {
            if (cmpResult < 0)
            {
                node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.left;
            }
            else
            {
                node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.right;
            }
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x344 | 0x8020BE84 | size: 0x2B0
 */
void FEResourceManager::UnloadPermanentResourceBundle()
{
    s_pPermanentBundle = new (nlMalloc(sizeof(BundleFile), 8, false)) BundleFile();
    s_pPermanentBundle->Open(m_szPermanentBundleFileName);

    FESceneResource* pPermanentSceneResource = s_pPermanentBundleSceneResource;
    unsigned long fileCount = s_pPermanentBundle->m_pHeader->nNumFiles;
    BundleFileDirectoryEntry fileDirectoryEntry;
    AVLTreeNode** pRoot;
    unsigned long fileIndex;
    FEResourceHandle** pLoadedResourceHandle;
    u32 hashtodelete;

    switch (pPermanentSceneResource->m_type)
    {
    case FERT_TEXTURE:
    {
        u32 key;
        FEResourceHandle** foundValue;
        u32 searchState = pPermanentSceneResource->m_hashID;
        AVLTreeEntry<unsigned long, FEResourceHandle*>* node = s_loadedResourceList.m_Root;

        while (node != NULL)
        {
            unsigned long nodeKey = node->key;
            int cmpResult;

            if (searchState == nodeKey)
            {
                cmpResult = 0;
            }
            else if (searchState < nodeKey)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                if (&foundValue != NULL)
                {
                    foundValue = (FEResourceHandle**)&node->value;
                }
                searchState = 1;
                goto check_scene_found;
            }
            else
            {
                if (cmpResult < 0)
                {
                    node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.left;
                }
                else
                {
                    node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.right;
                }
            }
        }

        searchState = 0;
    check_scene_found:
        if (!(u8)searchState)
            break;
        if (*foundValue != pPermanentSceneResource)
            break;

        key = pPermanentSceneResource->m_hashID;
        AVLTreeNode* removedNode = s_loadedResourceList.RemoveAVLNode(
            (AVLTreeNode**)&s_loadedResourceList.m_Root,
            &key,
            s_loadedResourceList.m_NumElements);

        if (removedNode != NULL)
        {
            removedNode->left = (AVLTreeNode*)s_loadedResourceList.m_Allocator.m_FreeList;
            s_loadedResourceList.m_Allocator.m_FreeList = (SlotPoolEntry*)removedNode;
            s_loadedResourceList.m_NumElements--;
        }

        break;
    }
    case FERT_SCENE:
        glplatResourceRelease(pPermanentSceneResource->m_glResourceMarker);
        break;
    }

    pRoot = (AVLTreeNode**)&s_loadedResourceList.m_Root;
    for (fileIndex = 0; fileIndex < fileCount; fileIndex++)
    {
        s_pPermanentBundle->GetFileInfoByIndex(fileIndex, &fileDirectoryEntry);

        bool found = FindLoadedResourceByRoot(pRoot, fileDirectoryEntry.m_hash, &pLoadedResourceHandle);
        if (found)
        {
            hashtodelete = (*pLoadedResourceHandle)->m_hashID;
            ::operator delete(*pLoadedResourceHandle);

            AVLTreeNode* removedNode = s_loadedResourceList.RemoveAVLNode(
                pRoot,
                &hashtodelete,
                s_loadedResourceList.m_NumElements);

            if (removedNode != NULL)
            {
                removedNode->left = (AVLTreeNode*)s_loadedResourceList.m_Allocator.m_FreeList;
                s_loadedResourceList.m_Allocator.m_FreeList = (SlotPoolEntry*)removedNode;
                s_loadedResourceList.m_NumElements--;
            }
        }
    }

    s_pPermanentBundle->Close();
    delete s_pPermanentBundle;
    s_pPermanentBundle = NULL;

    ::operator delete(s_pPermanentBundleSceneResource);
    s_pPermanentBundleSceneResource = NULL;
}

/**
 * Offset/Address/Size: 0x29C | 0x8020BDDC | size: 0xA8
 */
void FEResourceManager::TextureResourceLoadComplete(void*, unsigned long uReadSize, unsigned long uParam)
{
    FEResourceHandle* pHandle = (FEResourceHandle*)uParam;

    glTextureAdd(pHandle->m_hashID, s_pResourceLoadBuffer, uReadSize);

    delete[] s_pResourceLoadBuffer;
    s_pResourceLoadBuffer = NULL;

    ((FETextureResource*)pHandle)->m_glTextureHandle = pHandle->m_hashID;

    u32 key;
    FEResourceHandle* value;
    AVLTreeNode* existingNode;

    value = pHandle;
    key = pHandle->m_hashID;

    s_loadedResourceList.AddAVLNode(
        (AVLTreeNode**)&s_loadedResourceList.m_Root,
        &key,
        &value,
        &existingNode,
        s_loadedResourceList.m_NumElements);

    if (existingNode == NULL)
    {
        s_loadedResourceList.m_NumElements++;
    }

    pHandle->m_bValid = true;
}

static FEResourceHandle* FindExistingResourceInResourceList_Inline(FEResourceHandle* pFEResourceHandle)
{
    FEResourceHandle** pPreExistingResourceHandle;
    unsigned long key = pFEResourceHandle->m_hashID;
    AVLTreeEntry<unsigned long, FEResourceHandle*>* node = s_loadedResourceList.m_Root;
    DefaultKeyCompare<unsigned long> compare;

    while (node != NULL)
    {
        int cmpResult = compare(node->key, key);

        if (cmpResult == 0)
        {
            if (&pPreExistingResourceHandle != NULL)
            {
                pPreExistingResourceHandle = &node->value;
            }
            key = 1;
            goto check_found;
        }

        if (cmpResult < 0)
        {
            node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.left;
        }
        else
        {
            node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)node->node.right;
        }
    }

    key = 0;
check_found:
    if ((u8)key)
    {
        return *pPreExistingResourceHandle;
    }

    return NULL;
}

static ResourceResult IssueTextureLoadRequest_Inline(FETextureResource* pFeTextureResource)
{
    BundleFileDirectoryEntry fileDirectoryEntry;
    FEResourceHandle* pFeExistingResource = FindExistingResourceInResourceList_Inline((FEResourceHandle*)pFeTextureResource);

    if ((pFeExistingResource != NULL) && (pFeExistingResource->m_type == pFeTextureResource->m_type))
    {
        pFeTextureResource->m_glTextureHandle = ((FETextureResource*)pFeExistingResource)->m_glTextureHandle;
        pFeTextureResource->m_bValid = pFeExistingResource->m_bValid;
        return FERR_AlreadyLoaded;
    }

    if (s_pOnDemandBundle->GetFileInfo(pFeTextureResource->m_hashID, &fileDirectoryEntry, true))
    {
        s_pResourceLoadBuffer = (unsigned char*)nlMalloc(fileDirectoryEntry.m_length, 0x20, true);
        s_pOnDemandBundle->ReadFileAsync(
            pFeTextureResource->m_hashID,
            s_pResourceLoadBuffer,
            fileDirectoryEntry.m_length,
            FEResourceManager::TextureResourceLoadComplete,
            (unsigned long)pFeTextureResource);
    }

    return FERR_WaitingForResource;
}

static ResourceResult IssueSceneContextSwitch_Inline(FESceneResource* pFeSceneResource)
{
    if ((s_pCurrentFESceneResourceContext != NULL)
        && (s_pCurrentFESceneResourceContext != pFeSceneResource)
        && (s_pPermanentBundleSceneResource != s_pCurrentFESceneResourceContext))
    {
        s_pCurrentFESceneResourceContext->m_pFESceneContext->m_bValid = true;
        s_pCurrentFESceneResourceContext->m_pFESceneContext->AllResourcesLoadedCallback();
    }

    pFeSceneResource->m_glResourceMarker = glplatResourceMark();
    pFeSceneResource->m_bValid = true;
    s_pCurrentFESceneResourceContext = pFeSceneResource;

    return FERR_WaitingForResource;
}

static ResourceResult IssueFontLoadRequest_Inline(FEFontResource* pFeFontResource)
{
    FEResourceHandle* pFeResourceHandle = (FEResourceHandle*)pFeFontResource;
    nlFont* pExistingFont = FontManager::Instance()->GetFontByHashID(pFeResourceHandle->m_hashID);
    pFeFontResource->SetFontReference(pExistingFont);
    pFeResourceHandle->m_bValid = true;
    return FERR_AlreadyLoaded;
}

static ResourceResult IssueResourceLoadRequest_Inline(FEResourceHandle* pFeResourceHandle,
    DLListEntry<FEResourceHandle*>* pQueueEntry,
    DLListEntry<FEResourceHandle*>* pQueueHead)
{
    ResourceResult resourceRequestResult = FERR_WaitingForResource;
    DLListEntry<FEResourceHandle*>* volatile pQueueHeadSpill;
    DLListEntry<FEResourceHandle*>* volatile pQueueEntrySpill;
    pQueueEntrySpill = pQueueEntry;
    pQueueHeadSpill = pQueueHead;

    switch (pFeResourceHandle->m_type)
    {
    case FERT_TEXTURE:
        resourceRequestResult = IssueTextureLoadRequest_Inline((FETextureResource*)pFeResourceHandle);
        break;

    case FERT_SCENE:
        resourceRequestResult = IssueSceneContextSwitch_Inline((FESceneResource*)pFeResourceHandle);
        break;

    case FERT_FONT:
        resourceRequestResult = IssueFontLoadRequest_Inline((FEFontResource*)pFeResourceHandle);
        break;

    default:
        break;
    }

    return resourceRequestResult;
}

/**
 * Offset/Address/Size: 0x0 | 0x8020BB40 | size: 0x29C
 * TODO: 99.36% match - compare-result branch/register mismatch remains in
 * inlined AVL loaded-resource lookup
 */
void FEResourceManager::Update(float)
{
    FEResourceHandle* pFeResourceHandle;
    ResourceResult result;
    DLListEntry<FEResourceHandle*>** pPendingHead = &pendingResourceQueue.m_Head;
    bool bQueueNextResource = true;

    while (bQueueNextResource)
    {
        FEResourceHandle* pCurrentResource = s_pCurrentResourceBeingLoaded;
        if ((pCurrentResource != NULL) && !pCurrentResource->m_bValid)
        {
            return;
        }

        if (pCurrentResource != NULL)
        {
            DLListEntry<FEResourceHandle*>* removedEntry = nlDLRingRemoveStart(pPendingHead);
            removedEntry->m_next = (DLListEntry<FEResourceHandle*>*)pendingResourceQueue.m_Allocator.m_FreeList;
            pendingResourceQueue.m_Allocator.m_FreeList = (SlotPoolEntry*)removedEntry;
            s_pCurrentResourceBeingLoaded = NULL;

            if (*pPendingHead == NULL)
            {
                s_pCurrentFESceneResourceContext->m_pFESceneContext->m_bValid = true;
                s_pCurrentFESceneResourceContext = NULL;
            }
        }
        DLListEntry<FEResourceHandle*>* pQueueHead = *pPendingHead;
        if (pQueueHead == NULL)
        {
            return;
        }
        DLListEntry<FEResourceHandle*>* pQueueEntry = nlDLRingGetStart(pQueueHead);
        pFeResourceHandle = pQueueEntry->m_data;
        s_pCurrentResourceBeingLoaded = pFeResourceHandle;
        result = IssueResourceLoadRequest_Inline(pFeResourceHandle, pQueueEntry, *pPendingHead);
        bQueueNextResource = (result == FERR_AlreadyLoaded);
    }
}

void feResourceManager_stub()
{
    FEResourceManager::Instance()->GetName();
}
