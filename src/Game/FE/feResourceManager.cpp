#include "Game/FE/FEResourceManager.h"
#include "Game/FE/feFontResource.h"
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
static nlAVLTreeSlotPool<unsigned long, FEResourceHandle*, DefaultKeyCompare<unsigned long> > s_loadedResourceList;
static nlDLListSlotPool<FEResourceHandle*> pendingResourceQueue;
static FEResourceHandle* s_pCurrentResourceBeingLoaded;
FESceneResource* s_pCurrentFESceneResourceContext;
static FESceneResource* s_pPermanentBundleSceneResource;

enum ResourceResult
{
    FERR_WaitingForResource = 0,
    FERR_AlreadyLoaded = 1,
};

extern "C"
{
    void __ct__12SlotPoolBaseFv(void*);
    void* __register_global_object(void* object, void* destructor, void* registration);
}

struct FeResourceManagerDestructorChain
{
    FeResourceManagerDestructorChain* next;
    void* destructor;
    void* object;
};

void LoadedResourceListDtor(void* obj, int)
{
    ((nlAVLTreeSlotPool<unsigned long, FEResourceHandle*, DefaultKeyCompare<unsigned long> >*)obj)
        ->~nlAVLTreeSlotPool<unsigned long, FEResourceHandle*, DefaultKeyCompare<unsigned long> >();
}

void PendingResourceQueueDtor(void* obj, int)
{
    ((nlDLListSlotPool<FEResourceHandle*>*)obj)->~nlDLListSlotPool<FEResourceHandle*>();
}

/**
 * Offset/Address/Size: 0x0 | 0x8020D394 | size: 0xFC
 * TODO: 90.21% match - still differs in r30/r31 alias usage and
 * destructor/registration relocation symbols.
 */
extern "C" void __sinit_feResourceManager_cpp()
{
    static FeResourceManagerDestructorChain chain1;
    static FeResourceManagerDestructorChain chain2;

    u32* loaded = (u32*)&s_loadedResourceList;
    u32* loaded31 = loaded;
    u32* loaded30 = loaded;

    loaded[0] = 0x802A4B5C;
    loaded[0] = 0x80303850;
    __ct__12SlotPoolBaseFv((void*)(loaded + 1));

    u32 loadedVt = 0x8030383C;
    u32 zero = 0;
    u32 initial = 0x100;

    loaded31[9] = zero;
    loaded31[7] = zero;
    loaded31[8] = zero;
    loaded30[0] = loadedVt;
    loaded30[1] = initial;
    SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)(loaded30 + 1), 0x14);
    loaded30[2] = 0;

    __register_global_object((void*)loaded30, (void*)LoadedResourceListDtor, &chain1);

    SlotPoolBase* queue = (SlotPoolBase*)&pendingResourceQueue;
    SlotPoolBase* queue31 = queue;
    SlotPoolBase* queue30 = queue;

    __ct__12SlotPoolBaseFv((void*)queue);

    u32 zeroQueue = 0;
    u32 initialQueue = 0x80;

    ((nlDLListSlotPool<FEResourceHandle*>*)queue30)->m_Head = 0;
    queue31->m_Initial = initialQueue;
    SlotPoolBase::BaseAddNewBlock(queue30, 0xC);
    queue31->m_Delta = zeroQueue;

    __register_global_object((void*)queue31, (void*)PendingResourceQueueDtor, &chain2);
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8020D5A8 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<FEResourceHandle*>, DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>>(DLListEntry<FEResourceHandle*>*, DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>*, void (DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>::*)(DLListEntry<FEResourceHandle*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xA8 | 0x8020D548 | size: 0x60
//  */
// void nlAVLTreeSlotPool<unsigned long, FEResourceHandle*, DefaultKeyCompare<unsigned long>>::~nlAVLTreeSlotPool()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020D4A0 | size: 0xA8
//  */
// void nlDLListSlotPool<FEResourceHandle*>::~nlDLListSlotPool()
// {
// }

// /**
//  * Offset/Address/Size: 0xFC | 0x8020D490 | size: 0x10
//  */
// void DLListContainerBase<FEResourceHandle*, BasicSlotPool<DLListEntry<FEResourceHandle*>>>::DeleteEntry(DLListEntry<FEResourceHandle*>*)
// {
// }

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

// /**
//  * Offset/Address/Size: 0xD0 | 0x8020D320 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**, DLListEntry<FEResourceHandle*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x94 | 0x8020D2E4 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**, DLListEntry<FEResourceHandle*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x50 | 0x8020D2A0 | size: 0x44
//  */
// void nlDLRingRemove<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**, DLListEntry<FEResourceHandle*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x8020D288 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020D250 | size: 0x38
//  */
// void nlDLRingRemoveStart<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020D244 | size: 0xC
//  */
// void FEResourceManager::GetName()
// {
// }

// /**
//  * Offset/Address/Size: 0x900 | 0x8020D1A4 | size: 0xA0
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8D4 | 0x8020D178 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8A8 | 0x8020D14C | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x82C | 0x8020D0D0 | size: 0x7C
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x824 | 0x8020D0C8 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0xCC | 0x8020C970 | size: 0x758
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::
//     PostorderTraversal(AVLTreeEntry<unsigned long, FEResourceHandle*>*,
//                        void (AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                                          DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, FEResourceHandle*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x8020C90C | size: 0x64
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::
//     DestroyTree(void (AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                                   DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, FEResourceHandle*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x10 | 0x8020C8B4 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8020C8A4 | size: 0x10
//  */
// void AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
//                  DefaultKeyCompare<unsigned long>>::DeleteEntry(AVLTreeEntry<unsigned long, FEResourceHandle*>*)
// {
// }

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

    u32 fileCount = s_pPermanentBundle->m_bundleHeader->m_entryCount;
    u32 fileIndex;
    AVLTreeNode** pRoot = (AVLTreeNode**)&s_loadedResourceList.m_Root;
    FETextureResource* pTextureResource;

    for (fileIndex = 0; fileIndex < fileCount; fileIndex++)
    {
        BundleFileDirectoryEntry fileDirectoryEntry;

        if (!s_pPermanentBundle->GetFileInfoByIndex(fileIndex, &fileDirectoryEntry))
        {
            nlPrintf("FEResourceManager Error: Failed to get file information in permanent bundle!\n");
        }
        else
        {
            u32 fileHash = fileDirectoryEntry.m_hash;
            u32 fileLength = fileDirectoryEntry.m_length;

            pTextureResource = new (nlMalloc(sizeof(FETextureResource), 8, false)) FETextureResource();
            pTextureResource->m_hashID = fileHash;

            s_pResourceLoadBuffer = (unsigned char*)nlMalloc(fileLength, 0x20, true);
            s_pPermanentBundle->ReadFileByIndex(fileIndex, s_pResourceLoadBuffer, fileLength);
            glTextureAdd(pTextureResource->m_hashID, s_pResourceLoadBuffer, fileLength);

            delete[] s_pResourceLoadBuffer;
            s_pResourceLoadBuffer = NULL;

            pTextureResource->m_glTextureHandle = pTextureResource->m_hashID;

            FEResourceHandle* valueA = pTextureResource;
            u32 keyA = pTextureResource->m_hashID;
            AVLTreeNode* existingNodeB;
            s_loadedResourceList.AddAVLNode(pRoot, &keyA, &valueA, &existingNodeB, s_loadedResourceList.m_NumElements);
            if (existingNodeB == NULL)
            {
                s_loadedResourceList.m_NumElements++;
            }
            pTextureResource->m_bValid = true;

            delete[] s_pResourceLoadBuffer;
            s_pResourceLoadBuffer = NULL;

            FEResourceHandle* valueB = pTextureResource;
            u32 keyB = pTextureResource->m_hashID;
            AVLTreeNode* existingNodeA;
            s_loadedResourceList.AddAVLNode(pRoot, &keyB, &valueB, &existingNodeA, s_loadedResourceList.m_NumElements);
            if (existingNodeA == NULL)
            {
                s_loadedResourceList.m_NumElements++;
            }
            pTextureResource->m_bValid = true;
        }
    }

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

/**
 * Offset/Address/Size: 0x344 | 0x8020BE84 | size: 0x2B0
 * TODO: 98.91% match - remaining stack slot/register allocation mismatch in
 * AVL search/remove temporaries (`foundValue`/`key`, r28/r30 ownership).
 */
void FEResourceManager::UnloadPermanentResourceBundle()
{
    s_pPermanentBundle = new (nlMalloc(sizeof(BundleFile), 8, false)) BundleFile();
    s_pPermanentBundle->Open(m_szPermanentBundleFileName);

    FESceneResource* pPermanentSceneResource = s_pPermanentBundleSceneResource;
    u32 fileCount = s_pPermanentBundle->m_bundleHeader->m_entryCount;

    switch (pPermanentSceneResource->m_type)
    {
    case FERT_TEXTURE:
    {
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

        u32 key = pPermanentSceneResource->m_hashID;
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

    u32 fileIndex = 0;
    AVLTreeNode** pRoot = (AVLTreeNode**)&s_loadedResourceList.m_Root;
    nlAVLTreeSlotPool<unsigned long, FEResourceHandle*, DefaultKeyCompare<unsigned long> >* pLoadedResourceList = &s_loadedResourceList;

    while (fileIndex < fileCount)
    {
        BundleFileDirectoryEntry fileDirectoryEntry;
        FEResourceHandle** foundValue;
        unsigned char found;

        s_pPermanentBundle->GetFileInfoByIndex(fileIndex, &fileDirectoryEntry);

        AVLTreeEntry<unsigned long, FEResourceHandle*>* node = (AVLTreeEntry<unsigned long, FEResourceHandle*>*)*pRoot;
        u32 searchKey = fileDirectoryEntry.m_hash;

        while (node != NULL)
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
                if (&foundValue != NULL)
                {
                    foundValue = (FEResourceHandle**)&node->value;
                }
                found = 1;
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

        found = 0;
    check_found:
        if ((u8)found)
        {
            u32 key = (*foundValue)->m_hashID;
            ::operator delete(*foundValue);

            AVLTreeNode* removedNode = s_loadedResourceList.RemoveAVLNode(
                pRoot,
                &key,
                pLoadedResourceList->m_NumElements);

            if (removedNode != NULL)
            {
                removedNode->left = (AVLTreeNode*)pLoadedResourceList->m_Allocator.m_FreeList;
                pLoadedResourceList->m_Allocator.m_FreeList = (SlotPoolEntry*)removedNode;
                pLoadedResourceList->m_NumElements--;
            }
        }

        fileIndex++;
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

/**
 * Offset/Address/Size: 0x0 | 0x8020BB40 | size: 0x29C
 * TODO: 96.09% match - missing switch-entry dead-store spills still keep
 * result in r4 (target uses r5), cascading through TEXTURE-case register
 * allocation and stack slot layout.
 */
void FEResourceManager::Update(float)
{
    FEResourceHandle* pFeResourceHandle;
    unsigned char found;
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
        result = FERR_WaitingForResource;
        s_pCurrentResourceBeingLoaded = pFeResourceHandle;

        switch (pFeResourceHandle->m_type)
        {
        case FERT_TEXTURE:
        {
            FEResourceHandle** pPreExistingResourceHandle;
            unsigned long key = pFeResourceHandle->m_hashID;
            AVLTreeEntry<unsigned long, FEResourceHandle*>* node = s_loadedResourceList.m_Root;
            found = 0;

            while (node != NULL)
            {
                unsigned long nodeKey = node->key;
                int cmpResult;

                if (key == nodeKey)
                {
                    cmpResult = 0;
                }
                else if (key < nodeKey)
                {
                    cmpResult = -1;
                }
                else
                {
                    cmpResult = 1;
                }

                if (cmpResult == 0)
                {
                    if (&pPreExistingResourceHandle != NULL)
                    {
                        pPreExistingResourceHandle = &node->value;
                    }
                    found = 1;
                    break;
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

            FEResourceHandle* pFeExistingResource;
            if (found)
            {
                pFeExistingResource = *pPreExistingResourceHandle;
            }
            else
            {
                pFeExistingResource = NULL;
            }

            if ((pFeExistingResource != NULL) && (pFeExistingResource->m_type == pFeResourceHandle->m_type))
            {
                ((FETextureResource*)pFeResourceHandle)->m_glTextureHandle = ((FETextureResource*)pFeExistingResource)->m_glTextureHandle;
                pFeResourceHandle->m_bValid = pFeExistingResource->m_bValid;
                result = FERR_AlreadyLoaded;
            }
            else
            {
                BundleFileDirectoryEntry fileDirectoryEntry;
                if (s_pOnDemandBundle->GetFileInfo(pFeResourceHandle->m_hashID, &fileDirectoryEntry, true))
                {
                    s_pResourceLoadBuffer = (unsigned char*)nlMalloc(fileDirectoryEntry.m_length, 0x20, true);
                    s_pOnDemandBundle->ReadFileAsync(
                        pFeResourceHandle->m_hashID,
                        s_pResourceLoadBuffer,
                        fileDirectoryEntry.m_length,
                        TextureResourceLoadComplete,
                        (unsigned long)pFeResourceHandle);
                }
                result = FERR_WaitingForResource;
            }
            break;
        }

        case FERT_SCENE:
        {
            if ((s_pCurrentFESceneResourceContext != NULL)
                && (s_pCurrentFESceneResourceContext != (FESceneResource*)pFeResourceHandle)
                && (s_pPermanentBundleSceneResource != s_pCurrentFESceneResourceContext))
            {
                s_pCurrentFESceneResourceContext->m_pFESceneContext->m_bValid = true;
                s_pCurrentFESceneResourceContext->m_pFESceneContext->AllResourcesLoadedCallback();
            }

            ((FESceneResource*)pFeResourceHandle)->m_glResourceMarker = glplatResourceMark();
            pFeResourceHandle->m_bValid = true;
            s_pCurrentFESceneResourceContext = (FESceneResource*)pFeResourceHandle;
            result = FERR_WaitingForResource;
            break;
        }

        case FERT_FONT:
        {
            ((FEFontResource*)pFeResourceHandle)->SetFontReference(FontManager::Instance()->GetFontByHashID(pFeResourceHandle->m_hashID));
            result = FERR_AlreadyLoaded;
            pFeResourceHandle->m_bValid = true;
            break;
        }

        default:
            break;
        }

        bQueueNextResource = (result == FERR_AlreadyLoaded);
    }
}
