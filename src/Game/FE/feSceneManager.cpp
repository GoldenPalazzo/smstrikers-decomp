#include "Game/FE/feSceneManager.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feRender.h"
#include "NL/nlDLRing.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlDebug.h"
#include "NL/nlString.h"

extern FEInput* g_pFEInput;

template <>
FESceneManager* nlSingleton<FESceneManager>::s_pInstance = 0;

SlotPool<PackagePushPopMessage> m_PushPopMessageSlotPool__21PackagePushPopMessage(0x14, 0);
nlDLListSlotPool<PackagePushPopMessage*> m_pushPopMessageQueue(0x14, 0);

/**
 * Offset/Address/Size: 0x0 | 0x8020D64C | size: 0xC0
 */
void FESceneManager::Update(float dt)
{
    DLListEntry<BaseSceneHandler*>* headEntry;
    DLListEntry<BaseSceneHandler*>* currentEntry;

    ProcessPushPopQueue();

    if (m_sceneHandlerStack.m_Head == nullptr)
    {
        return;
    }

    currentEntry = nlDLRingGetStart(m_sceneHandlerStack.m_Head);
    headEntry = m_sceneHandlerStack.m_Head;

    while (currentEntry != nullptr)
    {
        if (currentEntry->entry->m_pFEScene->m_bValid != false)
        {
            g_pFEInput->EnableInputIfSceneHasFocus(currentEntry->entry);
            currentEntry->entry->Update(dt);
        }

        if (nlDLRingIsEnd(headEntry, currentEntry) || currentEntry == nullptr)
        {
            currentEntry = nullptr;
        }
        else
        {
            currentEntry = currentEntry->m_next;
        }
    }
}

static inline bool IsObjectQueuedForPop(BaseSceneHandler* pSceneHandler)
{
    DLListEntry<PackagePushPopMessage*>* msgEntry = nlDLRingGetStart(m_pushPopMessageQueue.m_Head);
    DLListEntry<PackagePushPopMessage*>* msgHead = m_pushPopMessageQueue.m_Head;

    while (msgEntry != NULL)
    {
        PackagePushPopMessage* pMsg = msgEntry->entry;
        if (pMsg->m_pSceneHandler == pSceneHandler && pMsg->m_bPush == false)
        {
            return true;
        }

        if (nlDLRingIsEnd(msgHead, msgEntry) || msgEntry == NULL)
        {
            msgEntry = NULL;
        }
        else
        {
            msgEntry = msgEntry->m_next;
        }
    }

    return false;
}

static inline void FindSceneForPop(
    PackagePushPopMessage* msg, DLListEntry<BaseSceneHandler*>* headEntry, DLListEntry<BaseSceneHandler*>* sceneEntry)
{
    while (sceneEntry != NULL)
    {
        BaseSceneHandler* pSceneHandler = sceneEntry->entry;

        if (!IsObjectQueuedForPop(pSceneHandler))
        {
            msg->m_pSceneHandler = sceneEntry->entry;
            break;
        }

        if (nlDLRingIsEnd(headEntry, sceneEntry) || sceneEntry == NULL)
        {
            sceneEntry = NULL;
        }
        else
        {
            sceneEntry = sceneEntry->m_next;
        }
    }
}

static inline void FindSceneAndQueuePop(
    PackagePushPopMessage* msg,
    DLListEntry<BaseSceneHandler*>* headEntry,
    DLListEntry<BaseSceneHandler*>* sceneEntry,
    DLListEntry<PackagePushPopMessage*>** queueHead)
{
    DLListEntry<PackagePushPopMessage*>* entry;

    FindSceneForPop(msg, headEntry, sceneEntry);

    entry = NULL;

    if (m_pushPopMessageQueue.m_Allocator.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&m_pushPopMessageQueue.m_Allocator, sizeof(DLListEntry<PackagePushPopMessage*>));
    }

    if (m_pushPopMessageQueue.m_Allocator.m_FreeList != NULL)
    {
        entry = (DLListEntry<PackagePushPopMessage*>*)m_pushPopMessageQueue.m_Allocator.m_FreeList;
        m_pushPopMessageQueue.m_Allocator.m_FreeList = m_pushPopMessageQueue.m_Allocator.m_FreeList->next;
    }

    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->entry = msg;
    }

    nlDLRingAddEnd(queueHead, entry);
}

static inline void RenderSceneStack(FESceneManager* pSceneManager)
{
    DLListEntry<BaseSceneHandler*>* sceneEntry = nlDLRingGetStart(pSceneManager->m_sceneHandlerStack.m_Head);
    DLListEntry<BaseSceneHandler*>* sceneHead = pSceneManager->m_sceneHandlerStack.m_Head;

    while (sceneEntry != NULL)
    {
        BaseSceneHandler* pSceneHandler = sceneEntry->entry;

        if (pSceneHandler != pSceneManager->m_topMostScene)
        {
            if (!IsObjectQueuedForPop(sceneEntry->entry))
            {
                if (pSceneHandler->m_pFEScene->m_bValid && pSceneHandler->m_bVisible)
                {
                    FERender::RenderScene(pSceneHandler->m_pFEScene);
                }
            }
        }

        if (nlDLRingIsEnd(sceneHead, sceneEntry) || sceneEntry == NULL)
        {
            sceneEntry = NULL;
        }
        else
        {
            sceneEntry = sceneEntry->m_next;
        }
    }
}

/**
 * Offset/Address/Size: 0xC0 | 0x8020D70C | size: 0x1C4
 */
void FESceneManager::RenderActiveScenes()
{
    if (m_topMostScene != NULL)
    {
        if (!IsObjectQueuedForPop(m_topMostScene))
        {
            if (m_topMostScene->m_pFEScene->m_bValid && m_topMostScene->m_bVisible)
            {
                FERender::RenderScene(m_topMostScene->m_pFEScene);
            }
        }
    }

    RenderSceneStack(this);
}

/**
 * Offset/Address/Size: 0x284 | 0x8020D8D0 | size: 0x1A8
 */
void FESceneManager::QueueScenePop()
{
    PackagePushPopMessage* msg;
    DLListEntry<PackagePushPopMessage*>** queueHead;

    msg = NULL;

    if (m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&m_PushPopMessageSlotPool__21PackagePushPopMessage, sizeof(PackagePushPopMessage));
    }

    if (m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList != NULL)
    {
        msg = (PackagePushPopMessage*)m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList;
        m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList = m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList->next;
    }

    msg->m_szFilename[0] = 0;
    msg->m_pSceneHandler = NULL;
    msg->m_bPush = false;

    DLListEntry<BaseSceneHandler*>* sceneEntry = nlDLRingGetStart(m_sceneHandlerStack.m_Head);
    queueHead = &m_pushPopMessageQueue.m_Head;

    FindSceneAndQueuePop(msg, m_sceneHandlerStack.m_Head, sceneEntry, queueHead);
}

/**
 * Offset/Address/Size: 0x42C | 0x8020DA78 | size: 0x114
 */
void FESceneManager::QueueScenePush(BaseSceneHandler* pSceneHandler, const char* szFilename)
{
    PackagePushPopMessage* msg = nullptr;

    if (m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&m_PushPopMessageSlotPool__21PackagePushPopMessage, sizeof(PackagePushPopMessage));
    }

    if (m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList != NULL)
    {
        msg = (PackagePushPopMessage*)m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList;
        m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList = m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList->next;
    }

    msg->m_bPush = true;
    msg->m_pSceneHandler = pSceneHandler;
    nlStrNCpy<char>(msg->m_szFilename, szFilename, 0x40);
    msg->m_pSceneHandler->m_uHashID = nlStringLowerHash(szFilename);

    DLListEntry<PackagePushPopMessage*>* entry = nullptr;

    if (m_pushPopMessageQueue.m_Allocator.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&m_pushPopMessageQueue.m_Allocator, sizeof(DLListEntry<PackagePushPopMessage*>));
    }

    if (m_pushPopMessageQueue.m_Allocator.m_FreeList != NULL)
    {
        entry = (DLListEntry<PackagePushPopMessage*>*)m_pushPopMessageQueue.m_Allocator.m_FreeList;
        m_pushPopMessageQueue.m_Allocator.m_FreeList = m_pushPopMessageQueue.m_Allocator.m_FreeList->next;
    }

    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->entry = msg;
    }

    nlDLRingAddEnd(&m_pushPopMessageQueue.m_Head, entry);
}

static inline void AddSceneHandlerEntry(
    BasicSlotPool<DLListEntry<BaseSceneHandler*> >& alloc,
    DLListEntry<BaseSceneHandler*>** head,
    BaseSceneHandler* const& pHandler)
{
    DLListEntry<BaseSceneHandler*>* entry = NULL;
    BaseSceneHandler* localData = pHandler;

    if (alloc.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&alloc, sizeof(DLListEntry<BaseSceneHandler*>));
    }

    if (alloc.m_FreeList != NULL)
    {
        entry = (DLListEntry<BaseSceneHandler*>*)alloc.m_FreeList;
        alloc.m_FreeList = alloc.m_FreeList->next;
    }

    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->entry = localData;
    }

    nlDLRingAddStart(head, entry);
}

static inline void LoadSceneInline(
    BaseSceneHandler* pHandler,
    const char* szFilename,
    FESceneManager* pInstance)
{
    FEScene* pFEScene = new (nlMalloc(0x70, 8, false)) FEScene();
    pFEScene->m_uHashID = nlStringLowerHash(szFilename);

    if (!pFEScene->LoadPackage(szFilename))
    {
        nlPrintf("Error: failed to load package!\n");
        nlBreak();
    }
    else
    {
        pFEScene->m_uRenderView = pInstance->m_uDefaultRenderView;
        pHandler->m_pFEScene = pFEScene;
        pHandler->SetPresentation(pFEScene->m_pFEPackage->GetPresentation());
        pHandler->SceneCreated();
        pHandler->InitializeSubHandlers();
    }
}

/**
 * Offset/Address/Size: 0x540 | 0x8020DB8C | size: 0x270
 * TODO: 99.94% match - the scene handler value uses r27 instead of r24 in the push block
 */
void FESceneManager::ProcessPushPopQueue()
{
    FESceneManager* pSceneManager = this;
    PackagePushPopMessage* pPackagePushPopMessage;
    DLListEntry<PackagePushPopMessage*>** queueHead = &m_pushPopMessageQueue.m_Head;

    while (*queueHead != NULL)
    {
        DLListEntry<PackagePushPopMessage*>* msgEntry = nlDLRingRemoveStart(queueHead);

        if (&pPackagePushPopMessage != NULL)
        {
            pPackagePushPopMessage = msgEntry->entry;
        }

        msgEntry->m_next = (DLListEntry<PackagePushPopMessage*>*)m_pushPopMessageQueue.m_Allocator.m_FreeList;
        m_pushPopMessageQueue.m_Allocator.m_FreeList = (SlotPoolEntry*)msgEntry;

        if (pPackagePushPopMessage->m_bPush != false)
        {
            AddSceneHandlerEntry(
                pSceneManager->m_sceneHandlerStack.m_Allocator,
                &pSceneManager->m_sceneHandlerStack.m_Head,
                pPackagePushPopMessage->m_pSceneHandler);

            LoadSceneInline(
                pPackagePushPopMessage->m_pSceneHandler,
                pPackagePushPopMessage->m_szFilename,
                nlSingleton<FESceneManager>::s_pInstance);
        }
        else
        {
            DLListEntry<BaseSceneHandler*>* headEntry;
            DLListEntry<BaseSceneHandler*>* sceneEntry;

            sceneEntry = nlDLRingGetStart(pSceneManager->m_sceneHandlerStack.m_Head);
            headEntry = pSceneManager->m_sceneHandlerStack.m_Head;

            while (sceneEntry != NULL)
            {
                if (sceneEntry->entry == pPackagePushPopMessage->m_pSceneHandler)
                {
                    nlDLRingIsEnd(headEntry, sceneEntry);
                    nlDLRingRemove(&pSceneManager->m_sceneHandlerStack.m_Head, sceneEntry);
                    sceneEntry->m_next = (DLListEntry<BaseSceneHandler*>*)pSceneManager->m_sceneHandlerStack.m_Allocator.m_FreeList;
                    pSceneManager->m_sceneHandlerStack.m_Allocator.m_FreeList = (SlotPoolEntry*)sceneEntry;
                    break;
                }

                if (nlDLRingIsEnd(headEntry, sceneEntry) || sceneEntry == NULL)
                {
                    sceneEntry = NULL;
                }
                else
                {
                    sceneEntry = sceneEntry->m_next;
                }
            }

            pPackagePushPopMessage->m_pSceneHandler->m_pFEScene->UnloadPackage();

            FEScene* pFEScene = pPackagePushPopMessage->m_pSceneHandler->m_pFEScene;

            delete pPackagePushPopMessage->m_pSceneHandler;
            delete pFEScene;
        }

        {
            SlotPoolEntry* freeEntry = (SlotPoolEntry*)pPackagePushPopMessage;
            freeEntry->next = m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList;
            m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList = freeEntry;
        }
    }
}

/**
 * Offset/Address/Size: 0x7B0 | 0x8020DDFC | size: 0x98
 */
BaseSceneHandler* FESceneManager::GetSceneHandler(unsigned long hashID)
{
    DLListEntry<BaseSceneHandler*>* headEntry;
    DLListEntry<BaseSceneHandler*>* currentEntry;

    currentEntry = nlDLRingGetStart(m_sceneHandlerStack.m_Head);
    headEntry = m_sceneHandlerStack.m_Head;

    while (currentEntry != nullptr)
    {
        if (hashID == currentEntry->entry->m_uHashID)
        {
            return currentEntry->entry;
        }

        if (nlDLRingIsEnd(headEntry, currentEntry) || currentEntry == nullptr)
        {
            currentEntry = nullptr;
        }
        else
        {
            currentEntry = currentEntry->m_next;
        }
    }

    return nullptr;
}

/**
 * Offset/Address/Size: 0x848 | 0x8020DE94 | size: 0x20
 */
void FESceneManager::ForceImmediateStackProcessing()
{
    FORCE_DONT_INLINE;
    ProcessPushPopQueue();
}

/**
 * Offset/Address/Size: 0x868 | 0x8020DEB4 | size: 0xA4
 */
bool FESceneManager::AreAllScenesValid()
{
    DLListEntry<BaseSceneHandler*>* headEntry;
    DLListEntry<BaseSceneHandler*>* currentEntry;

    currentEntry = nlDLRingGetStart(m_sceneHandlerStack.m_Head);
    headEntry = m_sceneHandlerStack.m_Head;

    while (currentEntry != nullptr)
    {
        BaseSceneHandler* sceneHandler = currentEntry->entry;
        if (sceneHandler->m_pFEScene->m_bValid == false)
        {
            return false;
        }

        if (nlDLRingIsEnd(headEntry, currentEntry) || currentEntry == nullptr)
        {
            currentEntry = nullptr;
        }
        else
        {
            currentEntry = currentEntry->m_next;
        }
    }

    return (m_pushPopMessageQueue.m_Head == nullptr);
}

/**
 * Offset/Address/Size: 0x9C4 | 0x8020E010 | size: 0x70
 * 91.25% match - residual r30/r31 swap: target colors the outer `this` to r31
 * and the m_Allocator subobject `this` to r30; MWCC GC/2.5 does the reverse.
 * Same single-pool inlined-ctor coloring wall as FontManager ctor (also 91.25%);
 * appears irreducible with local source (likely an original-compiler-version
 * coloring artifact). FETweenManager (two pools) matches 100% via a different
 * allocation, so it is not a template-shape problem.
 *
 * IMPORTANT: this ctor MUST stay defined BEFORE ~FESceneManager. Compiling the
 * destructor first instantiates nlDLListSlotPool<BaseSceneHandler*> out-of-line,
 * after which MWCC calls the out-of-line ctor instead of inlining it here,
 * collapsing the match to ~59%.
 */
FESceneManager::FESceneManager()
    : m_sceneHandlerStack(0x14, 0)
{
    m_uDefaultRenderView = -1;
    m_topMostScene = nullptr;
    FERender::Initialize();
}

/**
 * Offset/Address/Size: 0x90C | 0x8020DF58 | size: 0xB8
 */
FESceneManager::~FESceneManager()
{
    ForceImmediateStackProcessing();
    FERender::Cleanup();
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8020E080 | size: 0x10
//  */
// void DLListContainerBase<BaseSceneHandler*, BasicSlotPool<DLListEntry<BaseSceneHandler*>>>::DeleteEntry(DLListEntry<BaseSceneHandler*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xD0 | 0x8020E150 | size: 0x10
//  */
// void DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>::DeleteEntry(DLListEntry<PackagePushPopMessage*>*)
// {
// }

/**
 * Offset/Address/Size: 0xE0 | 0x8020E160 | size: 0xA8
 */
#pragma dont_inline on
void __force_PackagePushPop_dt()
{
    nlDLListSlotPool<PackagePushPopMessage*> x;
    x.~nlDLListSlotPool();
}
#pragma dont_inline off

// /**
//  * Offset/Address/Size: 0x0 | 0x8020E208 | size: 0x64
//  */
// void SlotPool<PackagePushPopMessage>::~SlotPool()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8020E26C | size: 0x3C
 * TODO: 96% match - stw LR save scheduling differs by one slot
 * (target emits first lwz from callbackFunc before stw r0,0x24(r1)).
 */
template void nlWalkDLRing<DLListEntry<BaseSceneHandler*>,
    DLListContainerBase<BaseSceneHandler*, BasicSlotPool<DLListEntry<BaseSceneHandler*> > > >(
    DLListEntry<BaseSceneHandler*>* head,
    DLListContainerBase<BaseSceneHandler*, BasicSlotPool<DLListEntry<BaseSceneHandler*> > >* callback,
    void (DLListContainerBase<BaseSceneHandler*, BasicSlotPool<DLListEntry<BaseSceneHandler*> > >::*callbackFunc)(
        DLListEntry<BaseSceneHandler*>*));

// /**
//  * Offset/Address/Size: 0x3C | 0x8020E2A8 | size: 0x38
//  */
// void nlDLRingRemoveStart<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x74 | 0x8020E2E0 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>*, DLListEntry<PackagePushPopMessage*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x94 | 0x8020E300 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>*, DLListEntry<BaseSceneHandler*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB4 | 0x8020E320 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xCC | 0x8020E338 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xE4 | 0x8020E350 | size: 0x44
//  */
// void nlDLRingRemove<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**, DLListEntry<PackagePushPopMessage*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x128 | 0x8020E394 | size: 0x44
//  */
// void nlDLRingRemove<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>**, DLListEntry<BaseSceneHandler*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x16C | 0x8020E3D8 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**, DLListEntry<PackagePushPopMessage*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1A8 | 0x8020E414 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**, DLListEntry<PackagePushPopMessage*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1E0 | 0x8020E44C | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>**, DLListEntry<BaseSceneHandler*>*)
// {
// }

/**
 * Offset/Address/Size: 0x218 | 0x8020E484 | size: 0x3C
 * TODO: 96% match - stw LR save scheduling differs by one slot
 * (target emits first lwz from callbackFunc before stw r0,0x24(r1)).
 */
template void nlWalkDLRing<DLListEntry<PackagePushPopMessage*>,
    DLListContainerBase<PackagePushPopMessage*, BasicSlotPool<DLListEntry<PackagePushPopMessage*> > > >(
    DLListEntry<PackagePushPopMessage*>* head,
    DLListContainerBase<PackagePushPopMessage*, BasicSlotPool<DLListEntry<PackagePushPopMessage*> > >* callback,
    void (DLListContainerBase<PackagePushPopMessage*, BasicSlotPool<DLListEntry<PackagePushPopMessage*> > >::*callbackFunc)(
        DLListEntry<PackagePushPopMessage*>*));

// /**
//  * Offset/Address/Size: 0x0 | 0x8020E4C0 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<BaseSceneHandler*>, DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>>(DLListEntry<BaseSceneHandler*>*, DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>*, void (DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>::*)(DLListEntry<BaseSceneHandler*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x60 | 0x8020E520 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<PackagePushPopMessage*>, DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>>(DLListEntry<PackagePushPopMessage*>*, DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>*, void (DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>::*)(DLListEntry<PackagePushPopMessage*>*))
// {
// }
