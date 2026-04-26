#ifndef _WORLDANIM_H_
#define _WORLDANIM_H_

#include "NL/nlList.h"
#include "NL/nlAVLTree.h"
#include "Game/Inventory.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim.h"

class AnimationSet
{
public:
    /* 0x0 */ cInventory<cSAnim> m_animInventory;

    inline cSAnim* FindAnimationByHash(u32 hash) const
    {
        ListEntry<cSAnim*>* entry = m_animInventory.m_lItemList.m_Head;
        while (entry != NULL)
        {
            cSAnim* candidate = entry->data;
            if (hash == entry->data->m_uHashID)
            {
                return candidate;
            }
            entry = entry->next;
        }
        return nullptr;
    }
}; // total size: 0x1C

struct glModel;

class WorldAnimController
{
public:
    virtual ~WorldAnimController() { };
    virtual void Update(float) { };
    virtual glModel* GetUpdatedModel(unsigned long, void*) { return 0; };

    void SetAnimation(const char* szAnimationName, ePlayMode playMode);
    void SetAnimationTime(float fTime);
    float GetAnimationTime();
    float GetAnimationDuration();

public:
    /* 0x4, */ cPoseAccumulator* m_pPoseAccumulator;
    /* 0x8, */ cPN_SAnimController* m_pPoseTree;
    /* 0xC, */ float m_fSpeed;
    /* 0x10 */ AnimationSet* m_pAnimationSet;
    /* 0x14 */ bool m_bIsGanged;
}; // total size: 0x18

class WorldAnimManager
{
public:
    WorldAnimManager();
    ~WorldAnimManager();

    /* 0x0 */ cInventory<cSHierarchy>* m_pHierarchyInventory;                                               // offset 0x0, size 0x4
    /* 0x4 */ nlAVLTree<unsigned long, AnimationSet*, DefaultKeyCompare<unsigned long> > m_animationSetMap; // offset 0x4, size 0x14
}; // total size: 0x18

// class nlAVLTree<unsigned long, AnimationSet*, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTree();
// };

// class AVLTreeBase<unsigned long, AnimationSet*, NewAdapter<AVLTreeEntry<unsigned long, AnimationSet*>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void DeleteEntry(AVLTreeEntry<unsigned long, AnimationSet*>*);
//     void AllocateEntry(void*, void*);
//     void CompareKey(void*, AVLTreeNode*);
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CastUp(AVLTreeNode*) const;
//     void PostorderTraversal(AVLTreeEntry<unsigned long, AnimationSet*>*, void (AVLTreeBase<unsigned long, AnimationSet*, NewAdapter<AVLTreeEntry<unsigned long, AnimationSet*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AnimationSet*>*));
//     void DestroyTree(void (AVLTreeBase<unsigned long, AnimationSet*, NewAdapter<AVLTreeEntry<unsigned long, AnimationSet*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, AnimationSet*>*));
//     void Clear();
//     void ~AVLTreeBase();
// };

#endif // _WORLDANIM_H_
