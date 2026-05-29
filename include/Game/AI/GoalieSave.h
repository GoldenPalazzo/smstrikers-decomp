#ifndef _GOALIESAVE_H_
#define _GOALIESAVE_H_

#include "NL/nlMath.h"
#include "NL/nlList.h"
#include "NL/nlAVLTree.h"

// void InsertSorted(nlDLListContainer<MyMiniData*>&, MyMiniData*);
// void nlListAddStart<ListEntry<SaveData*>>(ListEntry<SaveData*>**, ListEntry<SaveData*>*, ListEntry<SaveData*>**);
// void nlDLRingIsStart<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>*, DLListEntry<MyMiniData*>*);
// void nlDLRingIsEnd<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>*, DLListEntry<MyMiniData*>*);
// void nlDLRingGetStart<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>*);
// void nlDLRingAddEnd<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>**, DLListEntry<MyMiniData*>*);
// void nlDLRingAddStart<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>**, DLListEntry<MyMiniData*>*);
// void nlDLRingInsert<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>**, DLListEntry<MyMiniData*>*, DLListEntry<MyMiniData*>*);
// void 0x8028D298..0x8028D29C | size: 0x4;

class SavePositionData
{
public:
    /* 0x00 */ int mnAnimID;
    /* 0x04 */ float mfAnimDistance;
    /* 0x08 */ float mfAnimTime;
    /* 0x0C */ float mfAnimVelocity;
}; // total size: 0x10

class SaveData
{
public:
    /* 0x00 */ int mnAnimID;
    /* 0x04 */ SaveData* mpFailAnimData;
    /* 0x08 */ int mnRecoverAnimID;
    /* 0x0C */ unsigned int muSaveType;
    /* 0x10 */ nlVector3 mv3SavePos;
    /* 0x1C */ nlVector3 mv3TakeoffPos;
    /* 0x28 */ float mfDuration;
    /* 0x2C */ float mfMilestonePercent[5];
    /* 0x40 */ float mfFatigueValue;
    /* 0x44 */ SaveData* mpConnectedSaveData[4];
    /* 0x54 */ nlVector3 mv3GroupMinCoords;
    /* 0x60 */ nlVector3 mv3GroupMaxCoords;
    /* 0x6C */ char mszName[16];
    /* 0x7C */ int muIndex;
}; // total size: 0x80

struct SaveBlendInfo // most probably not the right place for this
{
public:
    /* 0x0, */ float mfStartTime;
    /* 0x4, */ float mfMilestoneTime[5];
    /* 0x18 */ float mfMilestoneScale[4][5];
    /* 0x68 */ float mfSaveBlendPrimary;
    /* 0x6C */ float mfSaveBlendSecondary;
    /* 0x70 */ float mfSaveBlendComposite;
    /* 0x74 */ class SaveData* mpSaveData[4];
    /* 0x84 */ class nlVector3 mv3BlendedSavePos;
}; // total size: 0x90

class Goalie;

class GoalieSave
{
public:
    static void ClearData();
    static void InitData(Goalie*);
    static SaveData* FindBestSave(SaveBlendInfo& blendInfo, const nlVector3& v3LocalPos, float fTime, bool bDoNearSearch, unsigned int uSaveType, bool bFromTakeoff);
    static SaveData* FindBestInList(SaveBlendInfo&, nlListContainer<SaveData*>&, const nlVector3&, float, unsigned int, bool);
    static SaveData* GetClosestBlendedPos(SaveBlendInfo&, const nlVector3&, SaveData*);
    static SaveData* GetMissChipSaveData(bool, bool);
    static SaveData* GetRandomSTSMissData(bool);
    static SaveData* GetSTSSpinMissData(bool);
    static SaveData* GetRandomSTSSaveData();
    static bool TriggerCallback(float, float, unsigned long, float, void*);
    static void AddAreaToGrid(SaveData*);
    static void AddSegmentToGrid(SaveData*, SaveData*);
    static void AddChainToGrid(SaveData*, bool);
    static void AddToGrid(SaveData*);

    static float mfCatchAllowDistSq;
    static SaveData* mpSaveTable;
    static unsigned int muSTSMissIndexStart;
    static unsigned int muSTSGoalIndexStart;
    static unsigned int muSTSGoalCount;
    static unsigned int muSTSSaveIndexStart;
    static unsigned int muSTSSaveCount;
    static unsigned int muMissChipIndexStart;
    static float mfCrouchDuration;
    static SavePositionData* mpPositionTable;
    static unsigned char mbInitialized;
};

// class AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>
// {
// public:
//     void AllocateEntry(void*, void*);
//     void CompareKey(void*, AVLTreeNode*);
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void ~AVLTreeBase();
//     void CastUp(AVLTreeNode*) const;
//     void PostorderTraversal(AVLTreeEntry<int, SaveData*>*, void (AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::*)(AVLTreeEntry<int, SaveData*>*));
//     void DestroyTree(void (AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::*)(AVLTreeEntry<int, SaveData*>*));
//     void Clear();
//     void DeleteEntry(AVLTreeEntry<int, SaveData*>*);
// };

// class DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<MyMiniData*>*);
// };

// class ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>
// {
// public:
//     void DeleteEntry(ListEntry<SaveData*>*);
// };

// class nlListContainer<SaveData*>
// {
// public:
//     void nlListContainer();
//     void ~nlListContainer();
// };

// class nlAVLTree<int, SaveData*, DefaultKeyCompare<int>>
// {
// public:
//     void ~nlAVLTree();
// };

// class nlWalkList<ListEntry<SaveData*>, ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>>(ListEntry<SaveData*>*, ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>*, void (ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>
// {
// public:
//     void *)(ListEntry<SaveData*>*));
// };

// class nlWalkDLRing<DLListEntry<MyMiniData*>, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>>(DLListEntry<MyMiniData*>*, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>*, void (DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>
// {
// public:
//     void *)(DLListEntry<MyMiniData*>*));
// };

// class nlWalkRing<DLListEntry<MyMiniData*>, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>>(DLListEntry<MyMiniData*>*, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>*, void (DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>
// {
// public:
//     void *)(DLListEntry<MyMiniData*>*));
// };

#endif // _GOALIESAVE_H_
