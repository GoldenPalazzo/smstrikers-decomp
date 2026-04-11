#include "Game/FE/feSceneManager.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feRender.h"
#include "NL/nlDLRing.h"
#include "NL/nlDLListSlotPool.h"
#include "NL/nlDebug.h"
#include "NL/nlString.h"

extern FEInput* g_pFEInput;

extern "C"
{
    void __ct__12SlotPoolBaseFv(void*);
    void* __register_global_object(void* object, void* destructor, void* registration);
}

struct FeSceneManagerDestructorChain
{
    FeSceneManagerDestructorChain* next;
    void* destructor;
    void* object;
};

void PushPopMessageSlotPoolDtor(void* obj, int)
{
    ((SlotPool<PackagePushPopMessage>*)obj)->~SlotPool<PackagePushPopMessage>();
}

void PushPopMessageQueueDtor(void* obj, int)
{
    ((nlDLListSlotPool<PackagePushPopMessage*>*)obj)->~nlDLListSlotPool<PackagePushPopMessage*>();
}

static SlotPool<PackagePushPopMessage> m_PushPopMessageSlotPool__21PackagePushPopMessage;
static nlDLListSlotPool<PackagePushPopMessage*> m_pushPopMessageQueue;

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
        if (currentEntry->m_data->m_pFEScene->m_bValid != false)
        {
            g_pFEInput->EnableInputIfSceneHasFocus(currentEntry->m_data);
            currentEntry->m_data->Update(dt);
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
        PackagePushPopMessage* pMsg = msgEntry->m_data;
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

/**
 * Offset/Address/Size: 0xC0 | 0x8020D70C | size: 0x1C4
 * TODO: 98.63% match - register allocation differences only (r diffs).
 * this=r31(target)/r29(current), sceneEntry=r28/r31, queueAddr=r30/r28, msgEntry=r29/r30.
 * Known MWCC issue: inlined IsObjectQueuedForPop accesses static m_pushPopMessageQueue,
 * causing hoisted static address register to shift caller variable allocation.
 * Same issue as QueueScenePop (98.82%).
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

    DLListEntry<BaseSceneHandler*>* sceneEntry = nlDLRingGetStart(m_sceneHandlerStack.m_Head);
    DLListEntry<BaseSceneHandler*>* sceneHead = m_sceneHandlerStack.m_Head;

    while (sceneEntry != NULL)
    {
        BaseSceneHandler* pSceneHandler = sceneEntry->m_data;

        if (pSceneHandler != m_topMostScene)
        {
            if (!IsObjectQueuedForPop(pSceneHandler))
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
 * Offset/Address/Size: 0x284 | 0x8020D8D0 | size: 0x1A8
 * TODO: 99.34% match - register allocation differences only (r diffs).
 * headEntry remains in r26 instead of r28; inlined IsObjectQueuedForPop locals rotate
 * (pSceneHandler r25/r29, msgHead r29/r26, msgEntry r28/r25).
 */
void FESceneManager::QueueScenePop()
{
    PackagePushPopMessage* msg;
    DLListEntry<PackagePushPopMessage*>* entry;
    DLListEntry<PackagePushPopMessage*>** queueHead;

    msg = NULL;

    if (m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&m_PushPopMessageSlotPool__21PackagePushPopMessage, sizeof(PackagePushPopMessage));
    }

    if (m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList != NULL)
    {
        msg = (PackagePushPopMessage*)m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList;
        m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList = m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList->m_next;
    }

    msg->m_szFilename[0] = 0;
    msg->m_pSceneHandler = NULL;
    msg->m_bPush = false;

    DLListEntry<BaseSceneHandler*>* sceneEntry = nlDLRingGetStart(m_sceneHandlerStack.m_Head);
    DLListEntry<BaseSceneHandler*>* headEntry = m_sceneHandlerStack.m_Head;
    queueHead = &m_pushPopMessageQueue.m_Head;

    while (sceneEntry != NULL)
    {
        BaseSceneHandler* pSceneHandler = sceneEntry->m_data;

        if (!IsObjectQueuedForPop(pSceneHandler))
        {
            msg->m_pSceneHandler = sceneEntry->m_data;
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

    entry = NULL;

    if (m_pushPopMessageQueue.m_Allocator.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&m_pushPopMessageQueue.m_Allocator, sizeof(DLListEntry<PackagePushPopMessage*>));
    }

    if (m_pushPopMessageQueue.m_Allocator.m_FreeList != NULL)
    {
        entry = (DLListEntry<PackagePushPopMessage*>*)m_pushPopMessageQueue.m_Allocator.m_FreeList;
        m_pushPopMessageQueue.m_Allocator.m_FreeList = m_pushPopMessageQueue.m_Allocator.m_FreeList->m_next;
    }

    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->m_data = msg;
    }

    nlDLRingAddEnd(queueHead, entry);
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
        m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList = m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList->m_next;
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
        m_pushPopMessageQueue.m_Allocator.m_FreeList = m_pushPopMessageQueue.m_Allocator.m_FreeList->m_next;
    }

    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->m_data = msg;
    }

    nlDLRingAddEnd(&m_pushPopMessageQueue.m_Head, entry);
}

/**
 * Offset/Address/Size: 0x540 | 0x8020DB8C | size: 0x270
 * TODO: 99.62% match - remaining r24/r27 swap for szFilename/pFEScene in push block,
 * and r26/r27 mismatch for pop-block msg pointer
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
            pPackagePushPopMessage = msgEntry->m_data;
        }

        msgEntry->m_next = (DLListEntry<PackagePushPopMessage*>*)m_pushPopMessageQueue.m_Allocator.m_FreeList;
        m_pushPopMessageQueue.m_Allocator.m_FreeList = (SlotPoolEntry*)msgEntry;

        if (pPackagePushPopMessage->m_bPush != false)
        {
            BaseSceneHandler* pHandler;
            DLListEntry<BaseSceneHandler*>* sceneEntry = NULL;
            FESceneManager* pInstance;
            char* szFilename;
            BaseSceneHandler* pSceneHandler = pPackagePushPopMessage->m_pSceneHandler;

            if (pSceneManager->m_sceneHandlerStack.m_Allocator.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&pSceneManager->m_sceneHandlerStack, sizeof(DLListEntry<BaseSceneHandler*>));
            }

            if (pSceneManager->m_sceneHandlerStack.m_Allocator.m_FreeList != NULL)
            {
                sceneEntry = (DLListEntry<BaseSceneHandler*>*)pSceneManager->m_sceneHandlerStack.m_Allocator.m_FreeList;
                pSceneManager->m_sceneHandlerStack.m_Allocator.m_FreeList = pSceneManager->m_sceneHandlerStack.m_Allocator.m_FreeList->m_next;
            }

            if (sceneEntry != NULL)
            {
                sceneEntry->m_next = NULL;
                sceneEntry->m_prev = NULL;
                sceneEntry->m_data = pSceneHandler;
            }

            nlDLRingAddStart(&pSceneManager->m_sceneHandlerStack.m_Head, sceneEntry);

            pInstance = nlSingleton<FESceneManager>::s_pInstance;
            pHandler = pPackagePushPopMessage->m_pSceneHandler;
            szFilename = pPackagePushPopMessage->m_szFilename;

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
        else
        {
            DLListEntry<BaseSceneHandler*>* headEntry;
            DLListEntry<BaseSceneHandler*>* sceneEntry;
            PackagePushPopMessage* msg;

            sceneEntry = nlDLRingGetStart(pSceneManager->m_sceneHandlerStack.m_Head);
            headEntry = pSceneManager->m_sceneHandlerStack.m_Head;
            msg = pPackagePushPopMessage;

            while (sceneEntry != NULL)
            {
                if (sceneEntry->m_data == msg->m_pSceneHandler)
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
            freeEntry->m_next = m_PushPopMessageSlotPool__21PackagePushPopMessage.m_FreeList;
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
        if (hashID == currentEntry->m_data->m_uHashID)
        {
            return currentEntry->m_data;
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
        BaseSceneHandler* sceneHandler = currentEntry->m_data;
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
 * Offset/Address/Size: 0x90C | 0x8020DF58 | size: 0xB8
 */
FESceneManager::~FESceneManager()
{
    ForceImmediateStackProcessing();
    FERender::Cleanup();
}

/**
 * Offset/Address/Size: 0x9C4 | 0x8020E010 | size: 0x70
 * TODO: 92.68% match - r30/r31 register swap in inlined nlDLListSlotPool ctor.
 * Known MWCC quirk: inlined template constructor assigns r31 to subobject this
 * instead of outer this. Same issue as FontManager ctor (91.25%) and
 * FETweenManager ctor (94.62%).
 */
FESceneManager::FESceneManager()
    : m_sceneHandlerStack(0x14, 0)
{
    m_uDefaultRenderView = -1;
    m_topMostScene = nullptr;
    FERender::Initialize();
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8020E080 | size: 0x10
//  */
// void DLListContainerBase<BaseSceneHandler*, BasicSlotPool<DLListEntry<BaseSceneHandler*>>>::DeleteEntry(DLListEntry<BaseSceneHandler*>*)
// {
// }

/**
 * Offset/Address/Size: 0xA34 | 0x8020E090 | size: 0xC0
 * TODO: 98.75% match - r30/r31 assignment order differs for queue aliases and
 * relocation symbols differ for destructor/registration chain labels.
 */
extern "C" void __sinit_feSceneManager_cpp()
{
    static FeSceneManagerDestructorChain chain1;
    static FeSceneManagerDestructorChain chain2;
    SlotPoolBase* messagePoolBase = (SlotPoolBase*)&m_PushPopMessageSlotPool__21PackagePushPopMessage;

    __ct__12SlotPoolBaseFv(messagePoolBase);
    messagePoolBase->m_Initial = 0x14;
    SlotPoolBase::BaseAddNewBlock(messagePoolBase, sizeof(PackagePushPopMessage));
    messagePoolBase->m_Delta = 0;
    __register_global_object(messagePoolBase, (void*)PushPopMessageSlotPoolDtor, &chain1);

    volatile SlotPoolBase* queueBaseCopy = (SlotPoolBase*)&m_pushPopMessageQueue;
    SlotPoolBase* messageQueueBase = (SlotPoolBase*)queueBaseCopy;

    __ct__12SlotPoolBaseFv((void*)queueBaseCopy);
    ((nlDLListSlotPool<PackagePushPopMessage*>*)queueBaseCopy)->m_Head = 0;
    messageQueueBase->m_Initial = 0x14;
    SlotPoolBase::BaseAddNewBlock(messageQueueBase, sizeof(DLListEntry<PackagePushPopMessage*>));
    ((SlotPoolBase*)queueBaseCopy)->m_Delta = 0;
    __register_global_object((void*)queueBaseCopy, (void*)PushPopMessageQueueDtor, &chain2);
}

// /**
//  * Offset/Address/Size: 0xD0 | 0x8020E150 | size: 0x10
//  */
// void DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>::DeleteEntry(DLListEntry<PackagePushPopMessage*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xE0 | 0x8020E160 | size: 0xA8
//  */
// void nlDLListSlotPool<PackagePushPopMessage*>::~nlDLListSlotPool()
// {
// }

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
