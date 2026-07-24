#ifndef _FESCENEMANAGER_H_
#define _FESCENEMANAGER_H_

#include "types.h"

#include "NL/nlSingleton.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlDLRing.h"

#include "Game/BaseSceneHandler.h"
#include "Game/FE/feScene.h"

// void nlDLRingRemoveStart<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**);
// void nlDLRingIsEnd<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>*, DLListEntry<PackagePushPopMessage*>*);
// void nlDLRingIsEnd<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>*, DLListEntry<BaseSceneHandler*>*);
// void nlDLRingGetStart<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>*);
// void nlDLRingGetStart<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>*);
// void nlDLRingRemove<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**, DLListEntry<PackagePushPopMessage*>*);
// void nlDLRingRemove<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>**, DLListEntry<BaseSceneHandler*>*);
// void nlDLRingAddEnd<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**, DLListEntry<PackagePushPopMessage*>*);
// void nlDLRingAddStart<DLListEntry<PackagePushPopMessage*>>(DLListEntry<PackagePushPopMessage*>**, DLListEntry<PackagePushPopMessage*>*);
// void nlDLRingAddStart<DLListEntry<BaseSceneHandler*>>(DLListEntry<BaseSceneHandler*>**, DLListEntry<BaseSceneHandler*>*);
class BaseSceneHandler;

struct PackagePushPopMessage
{
    /* 0x0 */ bool m_bPush;
    /* 0x4 */ BaseSceneHandler* m_pSceneHandler;
    /* 0x8 */ char m_szFilename[64];
}; // total size: 0x48

class FESceneManager : public nlSingleton<FESceneManager>
{
public:
    void Update(float);
    void RenderActiveScenes();
    void QueueScenePop();
    void QueueScenePush(BaseSceneHandler*, const char*);
    void LoadScene(const char*, BaseSceneHandler*);
    void ProcessPushPopQueue();
    BaseSceneHandler* GetSceneHandler(unsigned long);
    void ForceImmediateStackProcessing();
    bool AreAllScenesValid();

    ~FESceneManager();
    FESceneManager();

    /* 0x00 */ nlDLListSlotPool<BaseSceneHandler*> m_sceneHandlerStack;
    /* 0x1C */ BaseSceneHandler* m_topMostScene;
    /* 0x20 */ unsigned long m_uDefaultRenderView;
};

// class DLListContainerBase<BaseSceneHandler*, BasicSlotPool<DLListEntry<BaseSceneHandler*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<BaseSceneHandler*>*);
// };

// class DLListContainerBase<PackagePushPopMessage*, BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<PackagePushPopMessage*>*);
// };

// class nlDLListSlotPool<PackagePushPopMessage*>
// {
// public:
//     void ~nlDLListSlotPool();
// };

// class SlotPool<PackagePushPopMessage>
// {
// public:
//     void ~SlotPool();
// };

// class nlWalkDLRing<DLListEntry<BaseSceneHandler*>, DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>>(DLListEntry<BaseSceneHandler*>*, DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>*, void (DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>
// {
// public:
//     void *)(DLListEntry<BaseSceneHandler*>*));
// };

// class nlWalkDLRing<DLListEntry<PackagePushPopMessage*>, DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>>(DLListEntry<PackagePushPopMessage*>*, DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>*, void (DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>
// {
// public:
//     void *)(DLListEntry<PackagePushPopMessage*>*));
// };

// class nlWalkRing<DLListEntry<BaseSceneHandler*>, DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>>(DLListEntry<BaseSceneHandler*>*, DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>*, void (DLListContainerBase<BaseSceneHandler*,
// BasicSlotPool<DLListEntry<BaseSceneHandler*>>>
// {
// public:
//     void *)(DLListEntry<BaseSceneHandler*>*));
// };

// class nlWalkRing<DLListEntry<PackagePushPopMessage*>, DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>>(DLListEntry<PackagePushPopMessage*>*, DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>*, void (DLListContainerBase<PackagePushPopMessage*,
// BasicSlotPool<DLListEntry<PackagePushPopMessage*>>>
// {
// public:
//     void *)(DLListEntry<PackagePushPopMessage*>*));
// };

#endif // _FESCENEMANAGER_H_
