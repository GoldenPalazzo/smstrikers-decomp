#include "Game/AI/GoalieSave.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/FilteredRandom.h"
#include "Game/Field.h"

static nlAVLTree<int, SaveData*, DefaultKeyCompare<int> > gSaveMap;
static nlListContainer<SaveData*> gSaveGrid[7][5];

struct MyMiniData;

extern "C"
{
    typedef void (*ConstructorDestructor)(void*);
    void* __register_global_object(void* object, void* destructor, void* registration);
    void __construct_array(void* ptr, ConstructorDestructor ctor, ConstructorDestructor dtor, unsigned long size, unsigned long n);

    void __vt__18AVLTreeUntemplated(void);
    void vtAVLTreeBaseSave(void);
    void vtNlAVLTreeSave(void);
    void SaveMapDtor(void*, int);
    void SaveGridCtor(void*);
    void SaveGridDtor(void*);
    void SaveGridArrayDtor(void*, int);
}

#pragma alias vtAVLTreeBaseSave "__vt__91AVLTreeBase<i,P8SaveData,40NewAdapter<26AVLTreeEntry<i,P8SaveData>>,20DefaultKeyCompare<i>>"
#pragma alias vtNlAVLTreeSave "__vt__46nlAVLTree<i,P8SaveData,20DefaultKeyCompare<i>>"
#pragma alias SaveMapDtor "__dt__46nlAVLTree<i,P8SaveData,20DefaultKeyCompare<i>>Fv"
#pragma alias SaveGridCtor "__ct__27nlListContainer<P8SaveData>Fv"
#pragma alias SaveGridDtor "__dt__27nlListContainer<P8SaveData>Fv"
#pragma alias SaveGridArrayDtor "__arraydtor$565"

struct GoalieSaveDestructorChain
{
    GoalieSaveDestructorChain* next;
    void* destructor;
    void* object;
};

/**
 * Offset/Address/Size: 0x0 | 0x80056D60 | size: 0xA4
 * TODO: 96.10% match - relocation/symbol label diffs remain for @564/@566
 * chain objects and templated vtable/ctor/dtor symbols.
 */
extern "C" void __sinit_GoalieSave_cpp()
{
    static GoalieSaveDestructorChain chain1;
    static GoalieSaveDestructorChain chain2;

    u32* map = (u32*)&gSaveMap;
    map[0] = (u32)__vt__18AVLTreeUntemplated;
    map[0] = (u32)vtAVLTreeBaseSave;
    map[4] = 0;
    map[2] = 0;
    map[3] = 0;
    map[0] = (u32)vtNlAVLTreeSave;
    __register_global_object(map, (void*)SaveMapDtor, &chain1);

    __construct_array(gSaveGrid, SaveGridCtor, SaveGridDtor, 0xC, 0x23);
    __register_global_object(0, (void*)SaveGridArrayDtor, &chain2);
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80056D60 | size: 0xA4
//  */
// void 0x8028D298..0x8028D29C | size: 0x4
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80056D00 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<MyMiniData*>, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>>(DLListEntry<MyMiniData*>*, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>*, void (DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>::*)(DLListEntry<MyMiniData*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x10C | 0x80056CD4 | size: 0x2C
//  */
// void nlDLRingInsert<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>**, DLListEntry<MyMiniData*>*, DLListEntry<MyMiniData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xD4 | 0x80056C9C | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>**, DLListEntry<MyMiniData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x98 | 0x80056C60 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>**, DLListEntry<MyMiniData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x80056C48 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x60 | 0x80056C28 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>*, DLListEntry<MyMiniData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x80056C04 | size: 0x24
//  */
// void nlDLRingIsStart<DLListEntry<MyMiniData*>>(DLListEntry<MyMiniData*>*, DLListEntry<MyMiniData*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80056BC8 | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before `stw r0, 0x24(r1)`.
 */
template void nlWalkDLRing<DLListEntry<MyMiniData*>, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > > >(
    DLListEntry<MyMiniData*>* head,
    DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > >* callback,
    void (DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > >::*callbackFunc)(DLListEntry<MyMiniData*>*));

// /**
//  * Offset/Address/Size: 0x68 | 0x80056BA0 | size: 0x28
//  */
// void nlListAddStart<ListEntry<SaveData*>>(ListEntry<SaveData*>**, ListEntry<SaveData*>*, ListEntry<SaveData*>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80056B38 | size: 0x68
//  */
// void nlWalkList<ListEntry<SaveData*>, ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>>(ListEntry<SaveData*>*, ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>*, void (ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>::*)(ListEntry<SaveData*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x118 | 0x80056AD8 | size: 0x60
//  */
// void nlAVLTree<int, SaveData*, DefaultKeyCompare<int>>::~nlAVLTree()
// {
// }

// /**
//  * Offset/Address/Size: 0x108 | 0x80056AC8 | size: 0x10
//  */
// void nlListContainer<SaveData*>::nlListContainer()
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x80056A1C | size: 0xAC
//  */
// void nlListContainer<SaveData*>::~nlListContainer()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800569C0 | size: 0x24
//  */
// void ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*>>>::DeleteEntry(ListEntry<SaveData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8005699C | size: 0x24
//  */
// void DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*>>>::DeleteEntry(DLListEntry<MyMiniData*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8F4 | 0x80056938 | size: 0x64
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8C8 | 0x8005690C | size: 0x2C
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x89C | 0x800568E0 | size: 0x2C
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x840 | 0x80056884 | size: 0x5C
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x838 | 0x8005687C | size: 0x8
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0xE0 | 0x80056124 | size: 0x758
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::PostorderTraversal(AVLTreeEntry<int, SaveData*>*, void (AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::*)(AVLTreeEntry<int, SaveData*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x7C | 0x800560C0 | size: 0x64
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::DestroyTree(void (AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::*)(AVLTreeEntry<int, SaveData*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x24 | 0x80056068 | size: 0x58
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80056044 | size: 0x24
//  */
// void AVLTreeBase<int, SaveData*, NewAdapter<AVLTreeEntry<int, SaveData*>>, DefaultKeyCompare<int>>::DeleteEntry(AVLTreeEntry<int, SaveData*>*)
// {
// }

/**
 * Offset/Address/Size: 0x2B44 | 0x80055F64 | size: 0xE0
 * TODO: 91.39% match - register allocation/stack frame differs: compiler uses r26-r31 with 0x30 frame, target uses r25-r31 with 0x40 frame; merged NULL store still emits r31 for both head/tail
 */
/**
 * Offset/Address/Size: 0x3DC | 0x80055F64 | size: 0xE0
 * TODO: 91.4% match - MWCC coalesces headClr/tailClr into one register (6 callee-saved regs
 * instead of 7). Target uses r30=headClr, r31=tailClr separately. This shifts all register
 * assignments by +4 and reduces stack frame from 0x40 to 0x30 (stmw r25 vs stmw r26).
 */
void GoalieSave::ClearData()
{
    if (!mbInitialized)
    {
        return;
    }

    gSaveMap.Clear();

    typedef ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*> > > SaveListBase;

    int i = 0;

    do
    {
        int j = 0;
        nlListContainer<SaveData*>* pEntry = &gSaveGrid[i][0];
        ListEntry<SaveData*>* headClr = (ListEntry<SaveData*>*)(u32)j;
        ListEntry<SaveData*>* tailClr = (ListEntry<SaveData*>*)(u32)j;
        do
        {
            nlWalkList(pEntry->m_Head, (SaveListBase*)pEntry, &SaveListBase::DeleteEntry);
            pEntry->m_Head = headClr;
            j++;
            pEntry->m_Tail = tailClr;
            pEntry++;
        } while (j < 5);
        i++;
    } while (i < 7);

    if (mpSaveTable != NULL)
    {
        delete[] ((u8*)mpSaveTable - 0x10);
    }

    if (mpPositionTable != NULL)
    {
        delete[] ((u8*)mpPositionTable - 0x10);
    }

    mbInitialized = 0;
}

/**
 * Offset/Address/Size: 0x23F4 | 0x80055814 | size: 0x750
 */
void GoalieSave::InitData(Goalie* pGoalie)
{
    struct SaveInfo
    {
        int mnAnimID;
        int mnFailAnimID;
        int mnRecoverAnimID;
        unsigned int muSaveType;
        int mConnectedSaveID[4];
        char mszName[16];
    };

    struct GoalieTweaksLite
    {
        unsigned char mPad[0xB8];
        float fShotFatigueDefault;
        float fShotFatigueStandCatch;
        float fShotFatigueDiveCatch;
        float fShotFatigueStandDeflect;
        float fShotFatigueDiveDeflect;
        float fShotFatigueStandPunch;
        float fShotFatigueLegSave;
        float fShotFatigueSTSSave;
        float fShotFatigueSTSStun;
    };

    struct SAnimLite
    {
        unsigned char mPad[0x8];
        unsigned int m_nNumKeys;
    };

    struct SaveMapFindHelper
    {
        unsigned char m_Pad[0x8];
        AVLTreeEntry<int, SaveData*>* m_Root;

        inline bool FindGet(int key, SaveData*** foundValue) const
        {
            AVLTreeEntry<int, SaveData*>* node = m_Root;
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
                        *foundValue = &node->value;
                    }
                    return true;
                }

                if (cmpResult < 0)
                {
                    node = (AVLTreeEntry<int, SaveData*>*)node->node.left;
                }
                else
                {
                    node = (AVLTreeEntry<int, SaveData*>*)node->node.right;
                }
            }
            return false;
        }
    };

    extern unsigned int muNumSaveEntries__10GoalieSave;
    extern unsigned int muSTSMissCount__10GoalieSave;
    extern unsigned int muMissChipCount__10GoalieSave;
    extern unsigned int muNumPositionEntries__10GoalieSave;
    extern int gPositionAnimID[6];
    extern SaveInfo gSaveInfo[70];
    extern SAnimLite* GetAnim__14cAnimInventoryFi(void*, int);

    if (mbInitialized)
    {
        return;
    }

    muNumSaveEntries__10GoalieSave = 0x45;
    mpSaveTable = new SaveData[muNumSaveEntries__10GoalieSave];

    muSTSGoalIndexStart = 0;
    muSTSGoalCount = 0;
    muSTSMissIndexStart = 0;
    muSTSMissCount__10GoalieSave = 0;
    muSTSSaveIndexStart = 0;
    muSTSSaveCount = 0;
    muMissChipIndexStart = 0;
    muMissChipCount__10GoalieSave = 0;

    void* pAnimInventory = *(void**)((unsigned char*)pGoalie + 0x80);
    SAnimLite* pAnim = GetAnim__14cAnimInventoryFi(pAnimInventory, 0x2E);
    mfCrouchDuration = (float)pAnim->m_nNumKeys / 30.0f;

    GoalieTweaksLite* pTweaks = *(GoalieTweaksLite**)((unsigned char*)pGoalie + 0x1C4);

    for (unsigned int i = 0; i < muNumSaveEntries__10GoalieSave; i++)
    {
        SaveData* pSaveData = &mpSaveTable[i];
        const SaveInfo* pInfo = &gSaveInfo[i];

        pSaveData->mnAnimID = pInfo->mnAnimID;
        pSaveData->mnRecoverAnimID = pInfo->mnRecoverAnimID;
        pSaveData->muSaveType = pInfo->muSaveType;

        unsigned int uSaveType = pSaveData->muSaveType;
        if (uSaveType & 0x1)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueStandCatch;
        }
        else if (uSaveType & 0x2)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueDiveCatch;
        }
        else if (uSaveType & 0x4)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueStandDeflect;
        }
        else if (uSaveType & 0x8)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueDiveDeflect;
        }
        else if (uSaveType & 0x10)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueStandPunch;
        }
        else if (uSaveType & 0x20)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueLegSave;
        }
        else if (uSaveType & 0x00010000)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueSTSStun;
        }
        else if (uSaveType & 0x00060000)
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueSTSStun;
        }
        else
        {
            pSaveData->mfFatigueValue = pTweaks->fShotFatigueDefault;
        }

        pSaveData->mv3SavePos.f.x = 0.0f;
        pSaveData->mv3SavePos.f.y = 0.0f;
        pSaveData->mv3SavePos.f.z = 0.0f;

        pSaveData->mfMilestonePercent[0] = 0.0f;
        pSaveData->mfMilestonePercent[1] = 0.0f;
        pSaveData->mfMilestonePercent[2] = 0.0f;
        pSaveData->mfMilestonePercent[3] = 0.0f;
        pSaveData->mfMilestonePercent[4] = 0.0f;

        pSaveData->mv3TakeoffPos.f.x = 0.0f;
        pSaveData->mv3TakeoffPos.f.y = 0.0f;
        pSaveData->mv3TakeoffPos.f.z = 0.0f;

        pSaveData->mv3GroupMinCoords.f.x = 0.0f;
        pSaveData->mv3GroupMinCoords.f.y = 0.0f;
        pSaveData->mv3GroupMinCoords.f.z = 0.0f;

        pSaveData->mv3GroupMaxCoords.f.x = 0.0f;
        pSaveData->mv3GroupMaxCoords.f.y = 0.0f;
        pSaveData->mv3GroupMaxCoords.f.z = 0.0f;

        nlStrNCpy<char>(pSaveData->mszName, pInfo->mszName, 16);
        pSaveData->muIndex = i;

        if (uSaveType & 0x00020000)
        {
            muSTSGoalCount++;
            if (muSTSGoalCount == 1)
            {
                muSTSGoalIndexStart = i;
            }
        }
        else if (uSaveType & 0x00040000)
        {
            muSTSMissCount__10GoalieSave++;
            if (muSTSMissCount__10GoalieSave == 1)
            {
                muSTSMissIndexStart = i;
            }
        }
        else if (uSaveType & 0x00010000)
        {
            muSTSSaveCount++;
            if (muSTSSaveCount == 1)
            {
                muSTSSaveIndexStart = i;
            }
        }
        else if (uSaveType & 0x00100000)
        {
            muMissChipCount__10GoalieSave++;
            if (muMissChipCount__10GoalieSave == 1)
            {
                muMissChipIndexStart = i;
            }
        }

        AVLTreeNode* pExistingNode;
        gSaveMap.AddAVLNode((AVLTreeNode**)&gSaveMap.m_Root, &pSaveData->mnAnimID, &pSaveData, &pExistingNode, gSaveMap.m_NumElements);
        if (pExistingNode == NULL)
        {
            gSaveMap.m_NumElements++;
        }
    }

    SaveMapFindHelper* pMap = (SaveMapFindHelper*)&gSaveMap;

    for (unsigned int i = 0; i < muNumSaveEntries__10GoalieSave; i++)
    {
        SaveData* pSaveData = &mpSaveTable[i];
        const SaveInfo* pInfo = &gSaveInfo[i];

        pSaveData->mpFailAnimData = NULL;
        if (pInfo->mnFailAnimID >= 0)
        {
            SaveData** ppFail = NULL;
            if (pMap->FindGet(pInfo->mnFailAnimID, &ppFail))
            {
                pSaveData->mpFailAnimData = *ppFail;
            }
        }

        for (int j = 0; j < 4; j++)
        {
            int connectedID = pInfo->mConnectedSaveID[j];
            pSaveData->mpConnectedSaveData[j] = NULL;
            if (connectedID >= 0)
            {
                SaveData** ppConnected = NULL;
                if (pMap->FindGet(connectedID, &ppConnected))
                {
                    pSaveData->mpConnectedSaveData[j] = *ppConnected;
                }
            }
        }
    }

    muNumPositionEntries__10GoalieSave = 6;
    mpPositionTable = new SavePositionData[muNumPositionEntries__10GoalieSave];
    for (unsigned int i = 0; i < muNumPositionEntries__10GoalieSave; i++)
    {
        SavePositionData* pPos = &mpPositionTable[i];
        pPos->mnAnimID = gPositionAnimID[i];
        pPos->mfAnimDistance = 0.0f;
        pPos->mfAnimTime = 0.0f;
        pPos->mfAnimVelocity = 0.0f;
    }

    typedef ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*> > > SaveListBase;

    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            nlListContainer<SaveData*>* pEntry = &gSaveGrid[i][j];
            nlWalkList(pEntry->m_Head, (SaveListBase*)pEntry, &SaveListBase::DeleteEntry);
            pEntry->m_Head = NULL;
            pEntry->m_Tail = NULL;
        }
    }

    int nCount = (int)muNumSaveEntries__10GoalieSave - 1;
    while (nCount >= 0)
    {
        SaveData* pSaveData = &mpSaveTable[nCount];
        if ((pSaveData->muSaveType & 0xFFFF) != 0)
        {
            AddToGrid(pSaveData);
        }
        nCount--;
    }

    mbInitialized = 1;
}

template <typename T>
class nlSingleton
{
public:
    static T* s_pInstance;
};

class GameInfoManager
{
public:
    bool IsStunnedGoaliesOn() const;
};

struct MyMiniData
{
    int dist;
    nlListContainer<SaveData*>* list;
};

struct MyMiniListShim
{
    NewAdapter<DLListEntry<MyMiniData*> > m_Allocator;
    DLListEntry<MyMiniData*>* m_Head;
};

/**
 * Offset/Address/Size: 0x227C | 0x8005569C | size: 0x178
 */
static void InsertSorted(nlDLListContainer<MyMiniData*>& list, MyMiniData* data)
{
    DLListEntry<MyMiniData*>* head;
    DLListEntry<MyMiniData*>* current = nlDLRingGetStart(list.m_Head);
    head = list.m_Head;

    while (current != NULL)
    {
        if (current->m_data->dist > data->dist)
        {
            if (nlDLRingIsStart(head, current))
            {
                DLListEntry<MyMiniData*>* entry = (DLListEntry<MyMiniData*>*)nlMalloc(0xC, 8, 0);
                if (entry != NULL)
                {
                    entry->m_next = NULL;
                    entry->m_prev = NULL;
                    entry->m_data = data;
                }
                nlDLRingAddStart(&list.m_Head, entry);
                return;
            }

            if (nlDLRingIsStart(head, current))
            {
                head = NULL;
            }
            else
            {
                head = current->m_prev;
            }

            DLListEntry<MyMiniData*>* entry = (DLListEntry<MyMiniData*>*)nlMalloc(0xC, 8, 0);
            if (entry != NULL)
            {
                entry->m_next = NULL;
                entry->m_prev = NULL;
                entry->m_data = data;
            }
            nlDLRingInsert(&list.m_Head, head, entry);
            return;
        }

        if (nlDLRingIsEnd(head, current) || current == NULL)
        {
            current = NULL;
        }
        else
        {
            current = current->m_next;
        }
    }

    DLListEntry<MyMiniData*>* entry = (DLListEntry<MyMiniData*>*)nlMalloc(0xC, 8, 0);
    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->m_data = data;
    }
    nlDLRingAddEnd(&list.m_Head, entry);
}

/**
 * Offset/Address/Size: 0x1FC0 | 0x800553E0 | size: 0x2BC
 */
/**
 * TODO: 98.97% match - r20-r24 cyclic register shift in nested loop
 * (dist/gSaveGrid/griddata base pointers). MWCC -inline deferred register
 * allocation quirk.
 */
SaveData* GoalieSave::FindBestSave(SaveBlendInfo& blendInfo, const nlVector3& v3LocalPos, float fTime, bool bDoNearSearch, unsigned int uSaveType, bool bFromTakeoff)
{
    typedef SaveData* (*FindBestInListFunc)(SaveBlendInfo&, nlListContainer<SaveData*>&, const nlVector3&, float, unsigned int, bool);

    int i;
    int j;
    SaveData* pSaveData;
    MyMiniListShim mylist;
    MyMiniData griddata[7][5];
    int dist;
    int dz;
    int across;
    int up;

    float y;
    float z;
    z = v3LocalPos.f.z;
    y = v3LocalPos.f.y;

    float netWidth = cField::GetNet(1.0f)->GetNetWidth();
    float netHeight = cField::GetNet(1.0f)->GetNetHeight();

    i = (int)(7.0f * (0.5f * netWidth + y) / netWidth);
    if (i < 0)
        i = 0;
    else if (i >= 7)
        i = 6;

    j = (int)(5.0f * z / netHeight);
    if (j < 0)
        j = 0;
    else if (j >= 5)
        j = 4;

    if (nlSingleton<GameInfoManager>::s_pInstance->IsStunnedGoaliesOn())
        uSaveType &= ~3;

    pSaveData = ((FindBestInListFunc)GoalieSave::FindBestInList)(
        blendInfo,
        gSaveGrid[i][j],
        v3LocalPos,
        fTime,
        uSaveType,
        bFromTakeoff);

    if (bDoNearSearch && pSaveData == NULL)
    {
        mylist.m_Head = NULL;

        for (across = 0; across < 7; across++)
        {
            dz = i - across;
            dist = dz * dz;

            for (up = 0; up < 5; up++)
            {
                int du = j - up;
                int testDist = dist + du * du;

                if (testDist <= 8)
                {
                    griddata[across][up].dist = testDist;
                    griddata[across][up].list = &gSaveGrid[across][up];
                    InsertSorted(*(nlDLListContainer<MyMiniData*>*)&mylist, &griddata[across][up]);
                }
            }
        }

        DLListEntry<MyMiniData*>* current = nlDLRingGetStart(mylist.m_Head);
        DLListEntry<MyMiniData*>* head = mylist.m_Head;

        if (nlDLRingIsEnd(head, current) || current == NULL)
            current = NULL;
        else
            current = current->m_next;

        while (current != NULL)
        {
            MyMiniData* data = current->m_data;
            nlListContainer<SaveData*>* cellList = data->list;

            if (cellList != NULL)
            {
                pSaveData = ((FindBestInListFunc)GoalieSave::FindBestInList)(
                    blendInfo,
                    *cellList,
                    v3LocalPos,
                    fTime,
                    uSaveType,
                    bFromTakeoff);

                if (pSaveData != NULL)
                    break;
            }

            if (nlDLRingIsEnd(head, current) || current == NULL)
                current = NULL;
            else
                current = current->m_next;
        }

        typedef DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > > MiniDataList;
        nlWalkDLRing<DLListEntry<MyMiniData*>, MiniDataList>(
            mylist.m_Head,
            (MiniDataList*)&mylist,
            &MiniDataList::DeleteEntry);
        mylist.m_Head = NULL;
    }

    return pSaveData;
}

/**
 * Offset/Address/Size: 0x1A1C | 0x80054E3C | size: 0x5A4
 */
void GoalieSave::FindBestInList(SaveBlendInfo&, nlListContainer<SaveData*>&, const nlVector3&, float, unsigned int, bool)
{
}

/**
 * Offset/Address/Size: 0xF90 | 0x800543B0 | size: 0xA8C
 * TODO: 81.83% match - branch structure and register usage still diverge in
 * vertical row traversal and edge-selection blending paths.
 */
void GoalieSave::GetClosestBlendedPos(SaveBlendInfo& blendInfo, const nlVector3& v3TargetPos, SaveData* pSaveData)
{
    extern float Interpolate(float, float, float);

    SaveData* pClosest = pSaveData;
    SaveData* pEdge = NULL;

    SaveData* pLeft = NULL;
    SaveData* pRight = NULL;
    SaveData* pLeftUp = NULL;
    SaveData* pRightUp = NULL;

    float fScaleLeft;
    float fScaleRight;
    float fLeftY;
    float fRightY;
    float fTimeLeft[5];
    float fLeftZ;
    float fTimeRight[5];
    float fRightZ;

    blendInfo.mfSaveBlendPrimary = 0.0f;
    blendInfo.mfSaveBlendSecondary = 0.0f;
    blendInfo.mfSaveBlendComposite = 0.0f;

    if (pSaveData->mv3GroupMaxCoords.f.y <= v3TargetPos.f.y)
    {
        SaveData* pLast = pSaveData;
        while (pSaveData != NULL)
        {
            pLast = pSaveData;
            pSaveData = pSaveData->mpConnectedSaveData[2];
        }
        pEdge = pLast;
    }
    else if (pSaveData->mv3GroupMinCoords.f.y >= v3TargetPos.f.y)
    {
        SaveData* pLast = pSaveData;
        while (pSaveData != NULL)
        {
            pLast = pSaveData;
            pSaveData = pSaveData->mpConnectedSaveData[3];
        }
        pEdge = pLast;
    }
    else
    {
        SaveData* pPrev = pSaveData;
        SaveData* pCur = pSaveData;

        while (pCur != NULL && v3TargetPos.f.z > pCur->mv3SavePos.f.z)
        {
            pPrev = pCur;
            pCur = pCur->mpConnectedSaveData[0];
        }

        while (pPrev != NULL && v3TargetPos.f.z < pPrev->mv3SavePos.f.z)
        {
            pCur = pPrev;
            pPrev = pPrev->mpConnectedSaveData[1];
        }

        if (pPrev == NULL)
        {
            pLeft = pCur;
            pRight = pCur;
        }
        else if (pCur == NULL)
        {
            pLeft = pPrev;
            pRight = pPrev;
        }
        else
        {
            pLeft = pPrev;
            pRight = pCur;
        }

        pLeftUp = pLeft;
        pRightUp = pRight;

        bool done = false;
        while (!done)
        {
            if (v3TargetPos.f.y <= pLeft->mv3SavePos.f.y || v3TargetPos.f.y <= pRight->mv3SavePos.f.y)
            {
                if (v3TargetPos.f.y < pLeft->mv3SavePos.f.y && v3TargetPos.f.y < pRight->mv3SavePos.f.y && pLeft->mpConnectedSaveData[3] != NULL)
                {
                    SaveData* pNextRow = pLeft->mpConnectedSaveData[3];
                    SaveData* pNextPrev = pNextRow;
                    SaveData* pNextCur = pNextRow;

                    pLeftUp = pLeft;
                    pRightUp = pRight;

                    while (pNextCur != NULL && v3TargetPos.f.z > pNextCur->mv3SavePos.f.z)
                    {
                        pNextPrev = pNextCur;
                        pNextCur = pNextCur->mpConnectedSaveData[0];
                    }

                    while (pNextPrev != NULL && v3TargetPos.f.z < pNextPrev->mv3SavePos.f.z)
                    {
                        pNextCur = pNextPrev;
                        pNextPrev = pNextPrev->mpConnectedSaveData[1];
                    }

                    if (pNextPrev == NULL)
                    {
                        pLeft = pNextCur;
                        pRight = pNextCur;
                    }
                    else if (pNextCur == NULL)
                    {
                        pLeft = pNextPrev;
                        pRight = pNextPrev;
                    }
                    else
                    {
                        pLeft = pNextPrev;
                        pRight = pNextCur;
                    }
                }
                else
                {
                    pEdge = pLeft;
                    break;
                }
            }
            else if (v3TargetPos.f.y >= pLeftUp->mv3SavePos.f.y || v3TargetPos.f.y >= pRightUp->mv3SavePos.f.y)
            {
                if (v3TargetPos.f.y > pLeftUp->mv3SavePos.f.y && v3TargetPos.f.y > pRightUp->mv3SavePos.f.y && pLeftUp->mpConnectedSaveData[2] != NULL)
                {
                    SaveData* pNextRow = pLeftUp->mpConnectedSaveData[2];
                    SaveData* pNextPrev = pNextRow;
                    SaveData* pNextCur = pNextRow;

                    pLeft = pLeftUp;
                    pRight = pRightUp;

                    while (pNextCur != NULL && v3TargetPos.f.z > pNextCur->mv3SavePos.f.z)
                    {
                        pNextPrev = pNextCur;
                        pNextCur = pNextCur->mpConnectedSaveData[0];
                    }

                    while (pNextPrev != NULL && v3TargetPos.f.z < pNextPrev->mv3SavePos.f.z)
                    {
                        pNextCur = pNextPrev;
                        pNextPrev = pNextPrev->mpConnectedSaveData[1];
                    }

                    if (pNextPrev == NULL)
                    {
                        pLeftUp = pNextCur;
                        pRightUp = pNextCur;
                    }
                    else if (pNextCur == NULL)
                    {
                        pLeftUp = pNextPrev;
                        pRightUp = pNextPrev;
                    }
                    else
                    {
                        pLeftUp = pNextPrev;
                        pRightUp = pNextCur;
                    }
                }
                else
                {
                    pEdge = pLeftUp;
                    break;
                }
            }
            else
            {
                int milestone;

                fScaleLeft = 0.0f;
                fScaleRight = 0.0f;

                if (pLeft != pRight)
                {
                    fScaleLeft = (v3TargetPos.f.z - pLeft->mv3SavePos.f.z) / (pRight->mv3SavePos.f.z - pLeft->mv3SavePos.f.z);
                }

                if (pLeftUp != pRightUp)
                {
                    fScaleRight = (v3TargetPos.f.z - pLeftUp->mv3SavePos.f.z) / (pRightUp->mv3SavePos.f.z - pLeftUp->mv3SavePos.f.z);
                }

                fLeftY = Interpolate(pLeft->mv3SavePos.f.y, pRight->mv3SavePos.f.y, fScaleLeft);
                fRightY = Interpolate(pLeftUp->mv3SavePos.f.y, pRightUp->mv3SavePos.f.y, fScaleRight);
                blendInfo.mfSaveBlendComposite = (v3TargetPos.f.y - fLeftY) / (fRightY - fLeftY);

                if (blendInfo.mfSaveBlendComposite <= 0.001f)
                {
                    pEdge = pLeft;
                    break;
                }

                if (blendInfo.mfSaveBlendComposite >= 0.999f)
                {
                    pEdge = pLeftUp;
                    break;
                }

                done = true;

                blendInfo.mpSaveData[1] = NULL;
                if (fScaleLeft <= 0.999f)
                {
                    blendInfo.mpSaveData[0] = pLeft;
                    if (fScaleLeft >= 0.001f)
                    {
                        blendInfo.mpSaveData[1] = pRight;
                        pClosest = pLeft;
                        fLeftZ = v3TargetPos.f.z;
                        blendInfo.mfSaveBlendPrimary = fScaleLeft;

                        for (milestone = 0; milestone < 5; milestone++)
                        {
                            float fTime0 = pLeft->mfMilestonePercent[milestone] * pLeft->mfDuration;
                            float fTime1 = pRight->mfMilestonePercent[milestone] * pRight->mfDuration;

                            if (fTime0 <= 0.001f)
                                fTimeLeft[milestone] = 0.0f;
                            else
                                fTimeLeft[milestone] = Interpolate(fTime0, fTime1, fScaleLeft);
                        }
                    }
                    else
                    {
                        fLeftZ = pLeft->mv3SavePos.f.z;
                        for (milestone = 0; milestone < 5; milestone++)
                        {
                            fTimeLeft[milestone] = pLeft->mfMilestonePercent[milestone] * pLeft->mfDuration;
                        }
                    }
                }
                else
                {
                    blendInfo.mpSaveData[0] = pRight;
                    fLeftZ = pRight->mv3SavePos.f.z;
                    for (milestone = 0; milestone < 5; milestone++)
                    {
                        fTimeLeft[milestone] = pRight->mfMilestonePercent[milestone] * pRight->mfDuration;
                    }
                }

                blendInfo.mpSaveData[3] = NULL;
                if (fScaleRight <= 0.999f)
                {
                    blendInfo.mpSaveData[2] = pLeftUp;
                    if (fScaleRight >= 0.001f)
                    {
                        blendInfo.mpSaveData[3] = pRightUp;
                        fRightZ = v3TargetPos.f.z;
                        blendInfo.mfSaveBlendSecondary = fScaleRight;

                        for (milestone = 0; milestone < 5; milestone++)
                        {
                            float fTime0 = pLeftUp->mfMilestonePercent[milestone] * pLeftUp->mfDuration;
                            float fTime1 = pRightUp->mfMilestonePercent[milestone] * pRightUp->mfDuration;

                            if (fTime0 <= 0.001f)
                                fTimeRight[milestone] = 0.0f;
                            else
                                fTimeRight[milestone] = Interpolate(fTime0, fTime1, fScaleRight);
                        }
                    }
                    else
                    {
                        fRightZ = pLeftUp->mv3SavePos.f.z;
                        for (milestone = 0; milestone < 5; milestone++)
                        {
                            fTimeRight[milestone] = pLeftUp->mfMilestonePercent[milestone] * pLeftUp->mfDuration;
                        }
                    }
                }
                else
                {
                    blendInfo.mpSaveData[2] = pRightUp;
                    fRightZ = pRightUp->mv3SavePos.f.z;
                    for (milestone = 0; milestone < 5; milestone++)
                    {
                        fTimeRight[milestone] = pRightUp->mfMilestonePercent[milestone] * pRightUp->mfDuration;
                    }
                }

                blendInfo.mv3BlendedSavePos.f.y = v3TargetPos.f.y;
                blendInfo.mv3BlendedSavePos.f.z = Interpolate(fLeftZ, fRightZ, blendInfo.mfSaveBlendComposite);

                for (milestone = 0; milestone < 5; milestone++)
                {
                    if (fTimeLeft[milestone] <= 0.001f)
                        blendInfo.mfMilestoneTime[milestone] = 0.0f;
                    else
                        blendInfo.mfMilestoneTime[milestone] = Interpolate(fTimeLeft[milestone], fTimeRight[milestone], blendInfo.mfSaveBlendComposite);
                }

                if (blendInfo.mfSaveBlendComposite <= 0.5f)
                {
                    if (fScaleLeft <= 0.5f)
                        pClosest = pLeft;
                    else
                        pClosest = pRight;
                }
                else
                {
                    if (fScaleRight <= 0.5f)
                        pClosest = pLeftUp;
                    else
                        pClosest = pRightUp;
                }
            }
        }
    }

    if (pEdge != NULL)
    {
        SaveData* pPrev = pEdge;
        SaveData* pCur = pEdge;
        SaveData* pDown;
        SaveData* pUp;
        int milestone;

        while (pCur != NULL && v3TargetPos.f.z > pCur->mv3SavePos.f.z)
        {
            pPrev = pCur;
            pCur = pCur->mpConnectedSaveData[0];
        }

        while (pPrev != NULL && v3TargetPos.f.z < pPrev->mv3SavePos.f.z)
        {
            pCur = pPrev;
            pPrev = pPrev->mpConnectedSaveData[1];
        }

        if (pPrev == NULL)
        {
            pDown = pCur;
            pUp = pCur;
        }
        else if (pCur == NULL)
        {
            pDown = pPrev;
            pUp = pPrev;
        }
        else
        {
            pDown = pPrev;
            pUp = pCur;
        }

        blendInfo.mpSaveData[0] = pDown;
        blendInfo.mpSaveData[1] = NULL;
        blendInfo.mpSaveData[3] = NULL;
        blendInfo.mpSaveData[2] = NULL;

        if (pDown != pUp)
        {
            float fPrimary = (v3TargetPos.f.z - pDown->mv3SavePos.f.z) / (pUp->mv3SavePos.f.z - pDown->mv3SavePos.f.z);
            if (fPrimary >= 0.999f)
            {
                blendInfo.mv3BlendedSavePos = pUp->mv3SavePos;
                blendInfo.mpSaveData[0] = pUp;
                for (milestone = 0; milestone < 5; milestone++)
                {
                    blendInfo.mfMilestoneTime[milestone] = pUp->mfMilestonePercent[milestone] * pUp->mfDuration;
                }
            }
            else if (fPrimary <= 0.001f)
            {
                blendInfo.mv3BlendedSavePos = pDown->mv3SavePos;
                for (milestone = 0; milestone < 5; milestone++)
                {
                    blendInfo.mfMilestoneTime[milestone] = pDown->mfMilestonePercent[milestone] * pDown->mfDuration;
                }
            }
            else
            {
                blendInfo.mfSaveBlendPrimary = fPrimary;
                blendInfo.mv3BlendedSavePos.f.x = pDown->mv3SavePos.f.x;
                blendInfo.mv3BlendedSavePos.f.y = Interpolate(pDown->mv3SavePos.f.y, pUp->mv3SavePos.f.y, fPrimary);
                blendInfo.mv3BlendedSavePos.f.z = v3TargetPos.f.z;
                blendInfo.mpSaveData[1] = pUp;

                for (milestone = 0; milestone < 5; milestone++)
                {
                    float fTime0 = pDown->mfMilestonePercent[milestone] * pDown->mfDuration;
                    float fTime1 = pUp->mfMilestonePercent[milestone] * pUp->mfDuration;

                    if (fTime0 <= 0.001f)
                        blendInfo.mfMilestoneTime[milestone] = 0.0f;
                    else
                        blendInfo.mfMilestoneTime[milestone] = Interpolate(fTime0, fTime1, fPrimary);
                }
            }
        }
        else
        {
            blendInfo.mv3BlendedSavePos = pDown->mv3SavePos;
            for (milestone = 0; milestone < 5; milestone++)
            {
                blendInfo.mfMilestoneTime[milestone] = pDown->mfMilestonePercent[milestone] * pDown->mfDuration;
            }

            if (pDown->mpConnectedSaveData[1] == NULL && pDown->mpConnectedSaveData[0] == NULL)
            {
                float fTargetY = v3TargetPos.f.y;
                float fCurrentY = pDown->mv3SavePos.f.y;
                const float fNudge = 0.1f;

                if (fabs(fCurrentY - fTargetY) < fNudge)
                {
                    blendInfo.mv3BlendedSavePos.f.y = fTargetY;
                }
                else if (fCurrentY > fTargetY)
                {
                    blendInfo.mv3BlendedSavePos.f.y -= fNudge;
                }
                else
                {
                    blendInfo.mv3BlendedSavePos.f.y += fNudge;
                }

                float fTargetZ = v3TargetPos.f.z;
                float fCurrentZ = pDown->mv3SavePos.f.z;

                if (fabs(fCurrentZ - fTargetZ) < fNudge)
                {
                    blendInfo.mv3BlendedSavePos.f.z = fTargetZ;
                }
                else if (fCurrentZ > fTargetZ)
                {
                    blendInfo.mv3BlendedSavePos.f.z -= fNudge;
                }
                else
                {
                    blendInfo.mv3BlendedSavePos.f.z += fNudge;
                }
            }
        }

        if (blendInfo.mfSaveBlendPrimary < 0.5f)
            pClosest = blendInfo.mpSaveData[0];
        else
            pClosest = blendInfo.mpSaveData[1];
    }

    blendInfo.mv3BlendedSavePos.f.x = pClosest->mv3SavePos.f.x;
}

/**
 * Offset/Address/Size: 0xF4C | 0x8005436C | size: 0x44
 */
SaveData* GoalieSave::GetMissChipSaveData(bool bLeft, bool bFar)
{
    u32 v1 = (bFar != 0);
    u32 v2 = (bLeft != 0);
    int index = muMissChipIndexStart + (v1 ? 0 : 2) + v2;
    return &mpSaveTable[index];
}

/**
 * Offset/Address/Size: 0xEBC | 0x800542DC | size: 0x90
 */
SaveData* GoalieSave::GetRandomSTSMissData(bool bParam)
{
    int index = muSTSGoalIndexStart;
    if (!bParam)
    {
        if ((u32)muSTSGoalCount > 1)
        {
            static FilteredRandomRange randgen;
            index += randgen.genrand(muSTSGoalCount);
        }
    }
    return &mpSaveTable[index];
}

/**
 * Offset/Address/Size: 0xE98 | 0x800542B8 | size: 0x24
 */
SaveData* GoalieSave::GetSTSSpinMissData(bool bParam)
{
    u32 index = muSTSMissIndexStart + ((!bParam) ? 1 : 0);
    return &mpSaveTable[index];
}

/**
 * Offset/Address/Size: 0xE24 | 0x80054244 | size: 0x74
 */
SaveData* GoalieSave::GetRandomSTSSaveData()
{
    static FilteredRandomRange randgen;
    int index = muSTSSaveIndexStart + randgen.genrand(muSTSSaveCount);
    return &mpSaveTable[index];
}

/**
 * Offset/Address/Size: 0xDCC | 0x800541EC | size: 0x58
 */
bool GoalieSave::TriggerCallback(float fTime, float fDuration, unsigned long uEventID, float, void* pUserData)
{
    SaveData* pSaveData = (SaveData*)pUserData;

    if ((uEventID + 0x307C0000) == 0xE7CD)
    {
        pSaveData->mfMilestonePercent[2] = fTime;
        pSaveData->mfDuration = fDuration;
    }
    else if ((uEventID - 0x56260000) == 0x4BBE)
    {
        pSaveData->mfMilestonePercent[0] = fTime;
    }
    else if ((uEventID - 0x0F950000) == 0x24BA)
    {
        pSaveData->mfMilestonePercent[1] = fTime;
    }
    else if ((uEventID - 0x04540000) == 0x24B9)
    {
        pSaveData->mfMilestonePercent[3] = fTime;
    }
    return true;
}

/**
 * Offset/Address/Size: 0x780 | 0x80053BA0 | size: 0x64C
 */
void GoalieSave::AddAreaToGrid(SaveData*)
{
    FORCE_DONT_INLINE;
}

static inline void AddPointToGrid(SaveData* pSaveData, const nlVector3& v3Point)
{
    float z = v3Point.f.z;
    float y = v3Point.f.y;

    float netWidth = cField::GetNet(1.0f)->GetNetWidth();
    float netHeight = cField::GetNet(1.0f)->GetNetHeight();

    int i = (int)(7.0f * (0.5f * netWidth + y) / netWidth);
    if (i < 0)
        i = 0;
    else if (i >= 7)
        i = 6;

    int j = (int)(5.0f * z / netHeight);
    if (j < 0)
        j = 0;
    else if (j >= 5)
        j = 4;

    nlListContainer<SaveData*>& cell = gSaveGrid[i][j];

    ListEntry<SaveData*>* entry = cell.m_Head;
    if (entry != NULL)
    {
        while (entry != NULL)
        {
            if (entry->data == pSaveData)
                return;
            entry = entry->next;
        }
    }

    {
        ListEntry<SaveData*>* newEntry = (ListEntry<SaveData*>*)nlMalloc(sizeof(ListEntry<SaveData*>), 8, false);
        if (newEntry != NULL)
        {
            newEntry->next = NULL;
            newEntry->data = pSaveData;
        }
        nlListAddStart<ListEntry<SaveData*> >(&cell.m_Head, newEntry, &cell.m_Tail);
    }
}

static inline void Local2GridCoords(float y, float z, int& i, int& j)
{
    float netWidth = cField::GetNet(1.0f)->GetNetWidth();
    float netHeight = cField::GetNet(1.0f)->GetNetHeight();
    i = (int)(7.0f * (0.5f * netWidth + y) / netWidth);
    if (i < 0)
        i = 0;
    else if (i >= 7)
        i = 6;
    j = (int)(5.0f * z / netHeight);
    if (j < 0)
        j = 0;
    else if (j >= 5)
        j = 4;
}

/**
 * Offset/Address/Size: 0x390 | 0x800537B0 | size: 0x3F0
 * TODO: 95.81% match - r27/r28 vs r28/r29 register shift for pSaveData1/pSaveData2.
 * MWCC -inline deferred register allocation quirk (same issue as FindBestSave).
 */
void GoalieSave::AddSegmentToGrid(SaveData* pSaveData1, SaveData* pSaveData2)
{
    int divisions;
    SaveData* pCurSaveData;
    int count;
    int i, j, m, n;
    nlVector3 v3CurPos;

    Local2GridCoords(pSaveData1->mv3SavePos.f.y, pSaveData1->mv3SavePos.f.z, i, j);
    Local2GridCoords(pSaveData2->mv3SavePos.f.y, pSaveData2->mv3SavePos.f.z, m, n);
    float dz = pSaveData2->mv3SavePos.f.z - pSaveData1->mv3SavePos.f.z;
    float dy = pSaveData2->mv3SavePos.f.y - pSaveData1->mv3SavePos.f.y;
    float dx = pSaveData2->mv3SavePos.f.x - pSaveData1->mv3SavePos.f.x;
    divisions = abs(j - n) + abs(i - m);
    if (divisions > 0)
    {
        float inv = 1.0f / (float)divisions;
        dx *= inv;
        dy *= inv;
        dz *= inv;
    }
    v3CurPos = pSaveData1->mv3SavePos;
    for (count = 0; count <= divisions; count++)
    {
        float d2z = pSaveData2->mv3SavePos.f.z - v3CurPos.f.z;
        float d1z = pSaveData1->mv3SavePos.f.z - v3CurPos.f.z;
        float d2y = pSaveData2->mv3SavePos.f.y - v3CurPos.f.y;
        float d1y = pSaveData1->mv3SavePos.f.y - v3CurPos.f.y;
        if (d1z * d1z + d1y * d1y < d2z * d2z + d2y * d2y)
            pCurSaveData = pSaveData1;
        else
            pCurSaveData = pSaveData2;
        AddPointToGrid(pCurSaveData, v3CurPos);
        v3CurPos.f.z += dz;
        v3CurPos.f.y += dy;
        v3CurPos.f.x += dx;
    }
}

/**
 * Offset/Address/Size: 0x20C | 0x8005362C | size: 0x184
 */
void GoalieSave::AddChainToGrid(SaveData* pSaveData, bool bVertical)
{
    SaveData* pEnd;
    int dir;
    int oppdir;

    if (bVertical)
    {
        dir = 0;
        oppdir = 1;
    }
    else
    {
        dir = 2;
        oppdir = 3;
    }

    SaveData* p = pSaveData;
    while (p != NULL)
    {
        pEnd = p;
        p = p->mpConnectedSaveData[oppdir];
    }

    pSaveData->mv3GroupMaxCoords = pSaveData->mv3SavePos;
    pSaveData->mv3GroupMinCoords = pSaveData->mv3SavePos;

    SaveData* pCur = pEnd;
    SaveData* pLast;

    while (pCur != NULL)
    {
        if (pCur->mv3SavePos.f.x > pSaveData->mv3GroupMaxCoords.f.x)
            pSaveData->mv3GroupMaxCoords.f.x = pCur->mv3SavePos.f.x;
        if (pCur->mv3SavePos.f.y > pSaveData->mv3GroupMaxCoords.f.y)
            pSaveData->mv3GroupMaxCoords.f.y = pCur->mv3SavePos.f.y;
        if (pCur->mv3SavePos.f.z > pSaveData->mv3GroupMaxCoords.f.z)
            pSaveData->mv3GroupMaxCoords.f.z = pCur->mv3SavePos.f.z;

        if (pCur->mv3SavePos.f.x < pSaveData->mv3GroupMinCoords.f.x)
            pSaveData->mv3GroupMinCoords.f.x = pCur->mv3SavePos.f.x;
        if (pCur->mv3SavePos.f.y < pSaveData->mv3GroupMinCoords.f.y)
            pSaveData->mv3GroupMinCoords.f.y = pCur->mv3SavePos.f.y;
        if (pCur->mv3SavePos.f.z < pSaveData->mv3GroupMinCoords.f.z)
            pSaveData->mv3GroupMinCoords.f.z = pCur->mv3SavePos.f.z;

        pLast = pCur;
        SaveData* next = pCur->mpConnectedSaveData[dir];
        pCur = next;
        if (next != NULL)
        {
            AddSegmentToGrid(pLast, next);
        }
    }

    while (pLast != NULL)
    {
        pLast->mv3GroupMaxCoords = pSaveData->mv3GroupMaxCoords;
        pLast->mv3GroupMinCoords = pSaveData->mv3GroupMinCoords;
        pLast = pLast->mpConnectedSaveData[oppdir];
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80053420 | size: 0x20C
 */
void GoalieSave::AddToGrid(SaveData* pSaveData)
{
    if (pSaveData->mpConnectedSaveData[1] != NULL || pSaveData->mpConnectedSaveData[0] != NULL)
    {
        if (pSaveData->mpConnectedSaveData[2] != NULL || pSaveData->mpConnectedSaveData[3] != NULL)
        {
            AddAreaToGrid(pSaveData);
            return;
        }
        AddChainToGrid(pSaveData, true);
        return;
    }

    if (pSaveData->mpConnectedSaveData[2] != NULL || pSaveData->mpConnectedSaveData[3] != NULL)
    {
        AddChainToGrid(pSaveData, false);
        return;
    }

    AddPointToGrid(pSaveData, pSaveData->mv3SavePos);
    pSaveData->mv3GroupMinCoords = pSaveData->mv3SavePos;
    pSaveData->mv3GroupMaxCoords = pSaveData->mv3SavePos;
}
