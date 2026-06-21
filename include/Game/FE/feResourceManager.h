#ifndef _FERESOURCEMANAGER_H_
#define _FERESOURCEMANAGER_H_

#include "types.h"
#include "NL/nlTask.h"
#include "NL/nlSingleton.h"
#include "NL/nlBundleFile.h"

// void nlDLRingRemoveStart<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**);
// void nlDLRingGetStart<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>*);
// void nlDLRingRemove<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**, DLListEntry<FEResourceHandle*>*);
// void nlDLRingAddEnd<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**, DLListEntry<FEResourceHandle*>*);
// void nlDLRingAddStart<DLListEntry<FEResourceHandle*>>(DLListEntry<FEResourceHandle*>**, DLListEntry<FEResourceHandle*>*);

enum eFEResourceType
{
    FERT_UNKNOWN = -1,
    FERT_TEXTURE = 0,
    FERT_FONT = 1,
    FERT_SCENE = 2,
    FERT_RESOURCE_TYPE_COUNT = 3,
};

class FEResourceHandle
{
public:
    FEResourceHandle() { };
    ~FEResourceHandle() { };

    /* 0x00 */ FEResourceHandle* m_next;
    /* 0x04 */ FEResourceHandle* m_prev;
    /* 0x08 */ eFEResourceType m_type;
    /* 0x0C */ u32 m_hashID;
    /* 0x10 */ bool m_bValid;
}; // total size: 0x14

class FEResourceManager : public nlTask, public nlSingleton<FEResourceManager>
{
public:
    FEResourceManager();
    ~FEResourceManager();

    void Run(float dt)
    {
        Update(dt);
    }

    virtual const char* GetName() { return "FEResource Manager"; };

    void Cleanup();
    void LoadPermanentResourceBundle(const char*);
    void LoadPermanentTextures();
    bool OpenOnDemandResourceBundle(const char*);
    void Initialize();
    void Update(float);
    void QueueResourceLoad(FEResourceHandle*);
    void UnloadResource(FEResourceHandle*);
    void UnloadPermanentResourceBundle();
    static void TextureResourceLoadComplete(void*, unsigned long uReadSize, unsigned long uParam);

    // static nlSingleton<FEResourceManager> s_pInstance;

protected:
    /* 0x18 */ char m_szPermanentBundleFileName[32];
    /* 0x38 */ char m_szOnDemandBundleFileName[32];
}; // total size: 0x58

// class AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>,
// DefaultKeyCompare<unsigned long>>
// {
// public:
//     void AllocateEntry(void*, void*);
//     void CompareKey(void*, AVLTreeNode*);
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void ~AVLTreeBase();
//     void CastUp(AVLTreeNode*) const;
//     void PostorderTraversal(AVLTreeEntry<unsigned long, FEResourceHandle*>*, void (AVLTreeBase<unsigned long, FEResourceHandle*,
//     BasicSlotPool<AVLTreeEntry<unsigned long, FEResourceHandle*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long,
//     FEResourceHandle*>*)); void DestroyTree(void (AVLTreeBase<unsigned long, FEResourceHandle*, BasicSlotPool<AVLTreeEntry<unsigned long,
//     FEResourceHandle*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, FEResourceHandle*>*)); void Clear(); void
//     DeleteEntry(AVLTreeEntry<unsigned long, FEResourceHandle*>*);
// };

// class nlWalkDLRing<DLListEntry<FEResourceHandle*>, DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>>(DLListEntry<FEResourceHandle*>*, DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>*, void (DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>
// {
// public:
//     void *)(DLListEntry<FEResourceHandle*>*));
// };

// class DLListContainerBase<FEResourceHandle*, BasicSlotPool<DLListEntry<FEResourceHandle*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<FEResourceHandle*>*);
// };

// class nlDLListSlotPool<FEResourceHandle*>
// {
// public:
//     void ~nlDLListSlotPool();
// };

// class nlAVLTreeSlotPool<unsigned long, FEResourceHandle*, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTreeSlotPool();
// };

// class nlWalkRing<DLListEntry<FEResourceHandle*>, DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>>(DLListEntry<FEResourceHandle*>*, DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>*, void (DLListContainerBase<FEResourceHandle*,
// BasicSlotPool<DLListEntry<FEResourceHandle*>>>
// {
// public:
//     void *)(DLListEntry<FEResourceHandle*>*));
// };

#endif // _FERESOURCEMANAGER_H_
