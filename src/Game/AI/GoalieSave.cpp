#include "Game/AI/GoalieSave.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/FilteredRandom.h"
#include "Game/Field.h"

#pragma inline_depth(255)

static nlAVLTree<int, SaveData*, DefaultKeyCompare<int> > gSaveMap;
static nlListContainer<SaveData*> gSaveGrid[7][5];
static float fDefaultMilestoneValues[2] = { 0.4f, 0.7f };

struct MyMiniData;

/**
 * Offset/Address/Size: 0x0 | 0x80056BC8 | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before `stw r0, 0x24(r1)`.
 */
template void nlWalkDLRing<DLListEntry<MyMiniData*>, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > > >(
    DLListEntry<MyMiniData*>* head,
    DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > >* callback,
    void (DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > >::*callbackFunc)(DLListEntry<MyMiniData*>*));

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

    struct SAnimCtrlLite
    {
        void* vtable;
        void* children[3];
        int numChildren;
        SAnimLite* m_pSAnim;
        float m_fTime;
        bool m_bMirror;
        const void* m_pAnimRetarget;
        float m_fPrevTime;
    };

    struct SlotPoolLite
    {
        unsigned char pad[0xC];
        void* m_FreeList;
    };

    extern nlVector3 v3Zero;
    extern int gPositionAnimID[];
    extern SaveInfo gSaveInfo[];
    extern SlotPoolLite m_SAnimControllerSlotPool__19cPN_SAnimController;

    extern SAnimLite* GetAnim__14cAnimInventoryFi(void*, int);
    extern bool GetMirrored__14cAnimInventoryFi(void*, int);
    extern void* __ct__19cPN_SAnimControllerFP6cSAnimPC12AnimRetarget9ePlayModePFUiP19cPN_SAnimController_vUib(void*, void*, const void*, int, void*, unsigned int, bool);
    extern void GetRootTrans__9cPoseNodeFP9nlVector3Us(void*, nlVector3*, unsigned short);
    extern void BaseAddNewBlock__12SlotPoolBaseFP12SlotPoolBaseUi(void*, unsigned int);
    extern void GetAnimTriggerInfo__FP10cCharacteriPFffUlfPv_bPv(void*, int, bool (*)(float, float, unsigned long, float, void*), void*);
    extern void GetJointPositionFuture__10cCharacterFP9nlVector3iifbbb(void*, nlVector3*, int, int, float, bool, bool, bool);
    extern void* nlMalloc__FUlUib(unsigned long, unsigned int, bool);
    extern void* __construct_new_array(void*, void*, void*, unsigned long, unsigned long);
    extern void nlStrNCpy__FPcPCcUl(char*, const char*, unsigned long);

    unsigned int i;
    int nCount;
    unsigned int count;
    SaveData* pSaveData;

    if (mbInitialized)
    {
        return;
    }

    muNumSaveEntries = 0x45;
    void* mem = nlMalloc__FUlUib(0x2290, 8, false);
    mpSaveTable = (SaveData*)__construct_new_array(mem, NULL, NULL, 0x80, 0x45);

    muSTSGoalIndexStart = 0;
    muSTSGoalCount = 0;
    muSTSMissIndexStart = 0;
    muSTSMissCount = 0;
    muSTSSaveIndexStart = 0;
    muSTSSaveCount = 0;
    muMissChipIndexStart = 0;
    muMissChipCount = 0;

    void* pAnimInventory = *(void**)((unsigned char*)pGoalie + 0x80);
    SAnimLite* pAnim = GetAnim__14cAnimInventoryFi(pAnimInventory, 0x2E);
    u32 numKeys = pAnim->m_nNumKeys;

    nlVector3* pv3Zero = &v3Zero;
    u32 v3ZeroX = pv3Zero->as_u32[0];

    mfCrouchDuration = (float)numKeys / 30.0f;

    SaveInfo* pSaveInfoBase = gSaveInfo;
    SaveInfo* pSaveInfo = pSaveInfoBase;
    i = 0;
    u32 v3ZeroY = pv3Zero->as_u32[1];
    u32 v3ZeroZ = pv3Zero->as_u32[2];

    while (i < muNumSaveEntries)
    {
        pSaveData = &mpSaveTable[i];

        pSaveData->mnAnimID = pSaveInfo->mnAnimID;
        pSaveData->mnRecoverAnimID = pSaveInfo->mnRecoverAnimID;
        pSaveData->muSaveType = pSaveInfo->muSaveType;

        GoalieTweaksLite* pTweaks = *(GoalieTweaksLite**)((unsigned char*)pGoalie + 0x1C4);
        unsigned int uSaveType = pSaveInfo->muSaveType;

        float fFatigueValue;
        if (uSaveType & 0x1)
        {
            fFatigueValue = pTweaks->fShotFatigueStandCatch;
        }
        else if (uSaveType & 0x2)
        {
            fFatigueValue = pTweaks->fShotFatigueDiveCatch;
        }
        else if (uSaveType & 0x4)
        {
            fFatigueValue = pTweaks->fShotFatigueStandDeflect;
        }
        else if (uSaveType & 0x8)
        {
            fFatigueValue = pTweaks->fShotFatigueDiveDeflect;
        }
        else if (uSaveType & 0x10)
        {
            fFatigueValue = pTweaks->fShotFatigueStandPunch;
        }
        else if (uSaveType & 0x20)
        {
            fFatigueValue = pTweaks->fShotFatigueLegSave;
        }
        else if (uSaveType & 0x00010000)
        {
            fFatigueValue = pTweaks->fShotFatigueSTSStun;
        }
        else if (uSaveType & 0x00060000)
        {
            fFatigueValue = pTweaks->fShotFatigueSTSStun;
        }
        else
        {
            fFatigueValue = pTweaks->fShotFatigueDefault;
        }
        pSaveData->mfFatigueValue = fFatigueValue;

        pSaveData->mv3SavePos.as_u32[0] = v3ZeroX;
        pSaveData->mv3SavePos.as_u32[1] = v3ZeroY;
        pSaveData->mv3SavePos.as_u32[2] = v3ZeroZ;

        pSaveData->mfMilestonePercent[0] = 0.0f;
        pSaveData->mfMilestonePercent[1] = 0.0f;
        pSaveData->mfMilestonePercent[2] = 0.0f;
        pSaveData->mfMilestonePercent[3] = 0.0f;
        pSaveData->mfMilestonePercent[4] = 0.0f;

        pSaveData->mv3TakeoffPos.as_u32[0] = v3ZeroX;
        pSaveData->mv3TakeoffPos.as_u32[1] = v3ZeroY;
        pSaveData->mv3TakeoffPos.as_u32[2] = v3ZeroZ;

        pSaveData->mv3GroupMinCoords.as_u32[0] = v3ZeroX;
        pSaveData->mv3GroupMinCoords.as_u32[1] = v3ZeroY;
        pSaveData->mv3GroupMinCoords.as_u32[2] = v3ZeroZ;

        pSaveData->mv3GroupMaxCoords.as_u32[0] = v3ZeroX;
        pSaveData->mv3GroupMaxCoords.as_u32[1] = v3ZeroY;
        pSaveData->mv3GroupMaxCoords.as_u32[2] = v3ZeroZ;

        nlStrNCpy__FPcPCcUl(pSaveData->mszName, pSaveInfo->mszName, 16);
        pSaveData->muIndex = i;

        pSaveData = &mpSaveTable[i];
        unsigned int st = pSaveData->muSaveType;
        if (st & 0x00020000)
        {
            muSTSGoalCount++;
            if (muSTSGoalCount == 1)
            {
                muSTSGoalIndexStart = i;
            }
        }
        else if (st & 0x00040000)
        {
            muSTSMissCount++;
            if (muSTSMissCount == 1)
            {
                muSTSMissIndexStart = i;
            }
        }
        else if (st & 0x00010000)
        {
            muSTSSaveCount++;
            if (muSTSSaveCount == 1)
            {
                muSTSSaveIndexStart = i;
            }
        }
        else if (st & 0x00100000)
        {
            muMissChipCount++;
            if (muMissChipCount == 1)
            {
                muMissChipIndexStart = i;
            }
        }

        int animKey = pSaveData->mnAnimID;
        SaveData* pValue = pSaveData;
        AVLTreeNode* pExistingNode = NULL;
        gSaveMap.AddAVLNode((AVLTreeNode**)&gSaveMap.m_Root, &animKey, &pValue, &pExistingNode, gSaveMap.m_NumElements);
        if (pExistingNode == NULL)
        {
            gSaveMap.m_NumElements++;
        }

        pSaveInfo++;
        i++;
    }

    pSaveInfo = pSaveInfoBase;
    for (unsigned int j = 0; j < muNumSaveEntries; j++)
    {
        int failID = pSaveInfo->mnFailAnimID;
        SaveData* pEntry = &mpSaveTable[j];

        if (failID >= 0)
        {
            SaveData** ppFound;
            if (gSaveMap.FindGet(failID, &ppFound))
            {
                pEntry->mpFailAnimData = *ppFound;
            }
            else
            {
                pEntry->mpFailAnimData = NULL;
            }
        }
        else
        {
            pEntry->mpFailAnimData = NULL;
        }

        for (int k = 0; k < 4; k++)
        {
            int connID = pSaveInfo->mConnectedSaveID[k];
            if (connID >= 0)
            {
                SaveData** ppConn;
                if (gSaveMap.FindGet(connID, &ppConn))
                {
                    pEntry->mpConnectedSaveData[k] = *ppConn;
                }
                else
                {
                    pEntry->mpConnectedSaveData[k] = NULL;
                }
            }
            else
            {
                pEntry->mpConnectedSaveData[k] = NULL;
            }
        }

        pSaveInfo++;
    }

    muNumPositionEntries = 6;
    void* mem2 = nlMalloc__FUlUib(0x70, 8, false);
    mpPositionTable = (SavePositionData*)__construct_new_array(mem2, NULL, NULL, 0x10, 6);

    for (count = 0; count < muNumPositionEntries; count++)
    {
        SavePositionData* pPos = &mpPositionTable[count];
        int animID = gPositionAnimID[count];
        pPos->mnAnimID = animID;

        SAnimCtrlLite* pController = NULL;
        if (m_SAnimControllerSlotPool__19cPN_SAnimController.m_FreeList == NULL)
        {
            BaseAddNewBlock__12SlotPoolBaseFP12SlotPoolBaseUi(&m_SAnimControllerSlotPool__19cPN_SAnimController, 0x54);
        }
        if (m_SAnimControllerSlotPool__19cPN_SAnimController.m_FreeList != NULL)
        {
            pController = (SAnimCtrlLite*)m_SAnimControllerSlotPool__19cPN_SAnimController.m_FreeList;
            m_SAnimControllerSlotPool__19cPN_SAnimController.m_FreeList = *(void**)m_SAnimControllerSlotPool__19cPN_SAnimController.m_FreeList;
        }

        if (pController != NULL)
        {
            void* pAnimInv = *(void**)((unsigned char*)pGoalie + 0x80);
            bool bMirrored = GetMirrored__14cAnimInventoryFi(pAnimInv, animID);
            void* pSAnim = GetAnim__14cAnimInventoryFi(pAnimInv, animID);
            pController = (SAnimCtrlLite*)__ct__19cPN_SAnimControllerFP6cSAnimPC12AnimRetarget9ePlayModePFUiP19cPN_SAnimController_vUib(pController, pSAnim, NULL, 1, NULL, 0, bMirrored);
        }

        pController->m_fPrevTime = pController->m_fTime;
        pController->m_fTime = 1.0f;

        nlVector3 v3RootTrans;
        GetRootTrans__9cPoseNodeFP9nlVector3Us(pController, &v3RootTrans, 0);

        pPos->mfAnimDistance = v3RootTrans.f.y;
        pPos->mfAnimTime = (float)pController->m_pSAnim->m_nNumKeys / 30.0f;
        pPos->mfAnimVelocity = pPos->mfAnimDistance / pPos->mfAnimTime;

        if (pController != NULL)
        {
            typedef void (*VtableDestructor)(void*, int);
            VtableDestructor dtor = ((VtableDestructor*)pController->vtable)[2];
            dtor(pController, 1);
        }
    }

    {
        typedef ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*> > > SaveListBase;

        int row = 0;
        do
        {
            int j = 0;
            nlListContainer<SaveData*>* pEntry = &gSaveGrid[row][0];
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
            row++;
        } while (row < 7);
    }

    int nBallJointIndex = *(int*)((unsigned char*)pGoalie + 0x1AC);

    for (i = 0; i < muNumSaveEntries; i++)
    {
        pSaveData = &mpSaveTable[i];
        GetAnimTriggerInfo__FP10cCharacteriPFffUlfPv_bPv(pGoalie, pSaveData->mnAnimID, TriggerCallback, pSaveData);
        pSaveData->mfMilestonePercent[4] = 1.0f;
        GetJointPositionFuture__10cCharacterFP9nlVector3iifbbb(pGoalie, &pSaveData->mv3SavePos, pSaveData->mnAnimID, nBallJointIndex, pSaveData->mfMilestonePercent[2], true, true, false);
        if (pSaveData->mfMilestonePercent[1] > 0.0f)
        {
            GetJointPositionFuture__10cCharacterFP9nlVector3iifbbb(pGoalie, &pSaveData->mv3TakeoffPos, pSaveData->mnAnimID, -1, pSaveData->mfMilestonePercent[1], true, true, false);
        }
    }

    nCount = (int)muNumSaveEntries - 1;
    while (nCount >= 0)
    {
        SaveData* pSD = &mpSaveTable[nCount];
        if ((pSD->muSaveType & 0xFFFF) != 0)
        {
            AddToGrid(pSD);
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
 * TODO: 99.51% match - traversal after near-search list construction still
 * uses lower current/head registers and the final cleanup call target differs.
 */
SaveData* GoalieSave::FindBestSave(SaveBlendInfo& blendInfo, const nlVector3& v3LocalPos, float fTime, bool bDoNearSearch, unsigned int uSaveType, bool bFromTakeoff)
{
    int i;
    int j;
    SaveData* pSaveData;
    MyMiniListShim mylist;
    MyMiniData griddata[7][5];
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

    pSaveData = GoalieSave::FindBestInList(
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
            MyMiniData* gridRow = griddata[across];
            nlListContainer<SaveData*>* saveRow = gSaveGrid[across];
            for (up = 0; up < 5; up++)
            {
                int du = j - up;
                int dz = i - across;
                int testDist = dz * dz + du * du;

                if (testDist <= 8)
                {
                    gridRow[up].dist = testDist;
                    gridRow[up].list = &saveRow[up];
                    InsertSorted(*(nlDLListContainer<MyMiniData*>*)&mylist, &gridRow[up]);
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
                pSaveData = GoalieSave::FindBestInList(
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
 * TODO: 97.19% match - list cursor and milestone-scale loop registers still differ.
 */
SaveData* GoalieSave::FindBestInList(SaveBlendInfo& blendInfo, nlListContainer<SaveData*>& SaveList, const nlVector3& v3LocalPos, float fTime, unsigned int uSaveType, bool bFromTakeoff)
{
    float fClosest = 10000.0f;
    SaveBlendInfo tempBlendInfo;
    ListEntry<SaveData*>* pEntry;
    nlVector3 v3AdjLocalPos;
    float fSaveTime;
    SaveData* pConnected;
    float fLastTime;
    float fThisTime;
    unsigned char bEmptySpot;
    float fInvSegTime;

    int milestone = bFromTakeoff ? 1 : 0;
    SaveData* pClosest = NULL;

    pEntry = SaveList.m_Head;
    while (pEntry != NULL)
    {
        SaveData* pCur = pEntry->data;

        if (!(uSaveType & pCur->muSaveType))
            goto advance;

        fSaveTime = pCur->mfMilestonePercent[2] * pCur->mfDuration;
        {
            float fMilestoneVal = pCur->mfMilestonePercent[milestone];

            if (fMilestoneVal > 0.0f)
            {
                float fMilDur = fMilestoneVal * pCur->mfDuration;
                fSaveTime = fSaveTime - fMilDur;
                if (bFromTakeoff)
                    nlVec3Add(v3AdjLocalPos, v3LocalPos, pCur->mv3TakeoffPos);
                else
                    v3AdjLocalPos = v3LocalPos;
            }
            else
            {
                float fScale = 1.0f - fDefaultMilestoneValues[milestone];
                v3AdjLocalPos = v3LocalPos;
                fSaveTime = fSaveTime * fScale;
            }
        }

        if (fSaveTime <= fTime)
        {
            pCur = GoalieSave::GetClosestBlendedPos(tempBlendInfo, v3AdjLocalPos, pCur);

            fSaveTime = tempBlendInfo.mfMilestoneTime[2];
            {
                float fThisTime = tempBlendInfo.mfMilestoneTime[milestone];

                if (fThisTime > 0.0f)
                {
                    fSaveTime = fSaveTime - fThisTime;
                    if (bFromTakeoff)
                        nlVec3Add(v3AdjLocalPos, v3LocalPos, pCur->mv3TakeoffPos);
                }
                else
                {
                    float fScale = 1.0f - fDefaultMilestoneValues[milestone];
                    fSaveTime = fSaveTime * fScale;
                }
            }

            if (fSaveTime <= fTime)
            {
                float fDistY = v3AdjLocalPos.f.y - tempBlendInfo.mv3BlendedSavePos.f.y;
                float fDistZ = v3AdjLocalPos.f.z - tempBlendInfo.mv3BlendedSavePos.f.z;
                float fDistSq = fDistY * fDistY + fDistZ * fDistZ;

                if (fDistSq < fClosest)
                {
                    if (fDistSq < mfCatchAllowDistSq)
                        goto okUpdate;
                    if (pCur->muSaveType & 3)
                        goto advance;
                okUpdate:

                    fClosest = fDistSq;
                    pClosest = pCur;

                    blendInfo.mfStartTime = tempBlendInfo.mfStartTime;
                    __memcpy(blendInfo.mfMilestoneTime, tempBlendInfo.mfMilestoneTime, 20);

                    __memcpy(&blendInfo.mfMilestoneScale[0][0], &tempBlendInfo.mfMilestoneScale[0][0], 80);

                    blendInfo.mfSaveBlendPrimary = tempBlendInfo.mfSaveBlendPrimary;
                    blendInfo.mfSaveBlendSecondary = tempBlendInfo.mfSaveBlendSecondary;
                    blendInfo.mfSaveBlendComposite = tempBlendInfo.mfSaveBlendComposite;
                    __memcpy(&blendInfo.mpSaveData[0], &tempBlendInfo.mpSaveData[0], 28);

                    {
                        float fStartAdj = blendInfo.mfMilestoneTime[2] - fTime;
                        float fStartResult;
                        if (fStartAdj <= 0.0f)
                            fStartResult = 0.0f;
                        else
                            fStartResult = fStartAdj;
                        blendInfo.mfStartTime = fStartResult;
                    }

                    if (bFromTakeoff)
                    {
                        nlVec3Sub(blendInfo.mv3BlendedSavePos, blendInfo.mv3BlendedSavePos, pCur->mv3TakeoffPos);
                    }

                    if (fDistSq < 0.0025f)
                        break;
                }
            }
        }

    advance:
        pEntry = pEntry->next;
    }

    if (pClosest != NULL)
    {

        bEmptySpot = 0;

        {
            int i;

            for (i = 0; i < 4; i++)
            {
                pConnected = blendInfo.mpSaveData[i];
                if (pConnected == NULL)
                    continue;

                {
                    float fRunning = 0.0f;
                    int segment;

                    for (segment = 0; segment < 5; segment++)
                    {
                        fThisTime = pConnected->mfMilestonePercent[segment] * pConnected->mfDuration;
                        if (fThisTime > 0.0f)
                        {
                            blendInfo.mfMilestoneScale[i][segment] = fThisTime - fRunning;
                            fRunning = fThisTime;
                        }
                        else
                        {
                            blendInfo.mfMilestoneScale[i][segment] = -1.0f;
                            bEmptySpot = 1;
                        }
                    }
                }
            }
        }

        {
            int i;
            float fPrevTime = 0.0f;
            for (i = 0; i < 5; i++)
            {
                fThisTime = blendInfo.mfMilestoneTime[i];
                if (fThisTime > 0.0f)
                {
                    float fSegDuration = fThisTime - fPrevTime;
                    fInvSegTime = 1.0f / fSegDuration;
                    fPrevTime = fThisTime;

                    if (blendInfo.mpSaveData[0])
                        blendInfo.mfMilestoneScale[0][i] *= fInvSegTime;
                    if (blendInfo.mpSaveData[1])
                        blendInfo.mfMilestoneScale[1][i] *= fInvSegTime;
                    if (blendInfo.mpSaveData[2])
                        blendInfo.mfMilestoneScale[2][i] *= fInvSegTime;
                    if (blendInfo.mpSaveData[3])
                        blendInfo.mfMilestoneScale[3][i] *= fInvSegTime;
                }
            }
        }

        if ((unsigned char)bEmptySpot)
        {
            if (blendInfo.mfMilestoneTime[3] <= 0.0f)
            {
                blendInfo.mfMilestoneScale[0][3] = blendInfo.mfMilestoneScale[0][4];
                blendInfo.mfMilestoneScale[1][3] = blendInfo.mfMilestoneScale[1][4];
                blendInfo.mfMilestoneScale[2][3] = blendInfo.mfMilestoneScale[2][4];
                blendInfo.mfMilestoneScale[3][3] = blendInfo.mfMilestoneScale[3][4];
            }

            if (blendInfo.mfMilestoneTime[2] <= 0.0f)
            {
                blendInfo.mfMilestoneScale[0][2] = blendInfo.mfMilestoneScale[0][3];
                blendInfo.mfMilestoneScale[1][2] = blendInfo.mfMilestoneScale[1][3];
                blendInfo.mfMilestoneScale[2][2] = blendInfo.mfMilestoneScale[2][3];
                blendInfo.mfMilestoneScale[3][2] = blendInfo.mfMilestoneScale[3][3];
            }

            if (blendInfo.mfMilestoneTime[1] <= 0.0f)
            {
                blendInfo.mfMilestoneScale[0][1] = blendInfo.mfMilestoneScale[0][2];
                blendInfo.mfMilestoneScale[1][1] = blendInfo.mfMilestoneScale[1][2];
                blendInfo.mfMilestoneScale[2][1] = blendInfo.mfMilestoneScale[2][2];
                blendInfo.mfMilestoneScale[3][1] = blendInfo.mfMilestoneScale[3][2];
            }

            if (blendInfo.mfMilestoneTime[0] <= 0.0f)
            {
                blendInfo.mfMilestoneScale[0][0] = blendInfo.mfMilestoneScale[0][1];
                blendInfo.mfMilestoneScale[1][0] = blendInfo.mfMilestoneScale[1][1];
                blendInfo.mfMilestoneScale[2][0] = blendInfo.mfMilestoneScale[2][1];
                blendInfo.mfMilestoneScale[3][0] = blendInfo.mfMilestoneScale[3][1];
            }
        }

    } /* end if (pClosest != NULL) */

    return pClosest;
}

/**
 * Offset/Address/Size: 0xF90 | 0x800543B0 | size: 0xA8C
 * TODO: 97.94% match - pClosest/pEdge register allocation still diverges in
 * edge-selection paths.
 */
SaveData* GoalieSave::GetClosestBlendedPos(SaveBlendInfo& blendInfo, const nlVector3& v3TargetPos, SaveData* pClosest)
{
    SaveData* const pSaveData = pClosest;
    SaveData* pEdge = NULL;

    SaveData* pRightUp = NULL;
    SaveData* pLeft = NULL;
    SaveData* pLeftUp = NULL;
    SaveData* pRight = NULL;

    float fScaleLeft;
    float fScaleRight;

    blendInfo.mfSaveBlendPrimary = 0.0f;
    blendInfo.mfSaveBlendSecondary = 0.0f;
    blendInfo.mfSaveBlendComposite = 0.0f;

    if (pSaveData->mv3GroupMaxCoords.f.y > v3TargetPos.f.y)
    {
        if (pSaveData->mv3GroupMinCoords.f.y < v3TargetPos.f.y)
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

            unsigned char done = 0;
            while (!done)
            {
                if (v3TargetPos.f.y <= pLeft->mv3SavePos.f.y || v3TargetPos.f.y <= pRight->mv3SavePos.f.y)
                {
                    if (v3TargetPos.f.y >= pLeft->mv3SavePos.f.y || v3TargetPos.f.y >= pRight->mv3SavePos.f.y || pLeft->mpConnectedSaveData[3] == NULL)
                    {
                        pEdge = pLeft;
                        break;
                    }
                    else
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
                }
                else if (v3TargetPos.f.y >= pLeftUp->mv3SavePos.f.y || v3TargetPos.f.y >= pRightUp->mv3SavePos.f.y)
                {
                    if (v3TargetPos.f.y <= pLeftUp->mv3SavePos.f.y || v3TargetPos.f.y <= pRightUp->mv3SavePos.f.y || pLeftUp->mpConnectedSaveData[2] == NULL)
                    {
                        pEdge = pLeftUp;
                        break;
                    }
                    else
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

                    float fLefty = Interpolate(pLeft->mv3SavePos.f.y, pRight->mv3SavePos.f.y, fScaleLeft);
                    float fRighty = Interpolate(pLeftUp->mv3SavePos.f.y, pRightUp->mv3SavePos.f.y, fScaleRight);
                    float fComposite = (v3TargetPos.f.y - fLefty) / (fRighty - fLefty);

                    if (fComposite <= 0.001f)
                    {
                        pEdge = pLeft;
                        break;
                    }

                    if (fComposite >= 0.999f)
                    {
                        pEdge = pLeftUp;
                        break;
                    }

                    done = 1;
                    blendInfo.mfSaveBlendComposite = fComposite;

                    float fTimeLeft[5];
                    float fLeftZ;
                    float fTimeRight[5];
                    float fRightZ;

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

                                fTimeLeft[milestone] = (fTime0 <= 0.001f) ? 0.0f : Interpolate(fTime0, fTime1, fScaleLeft);
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

                                fTimeRight[milestone] = (fTime0 <= 0.001f) ? 0.0f : Interpolate(fTime0, fTime1, fScaleRight);
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
                    blendInfo.mv3BlendedSavePos.f.z = Interpolate(fLeftZ, fRightZ, fComposite);

                    for (milestone = 0; milestone < 5; milestone++)
                    {
                        float fRightTime = fTimeRight[milestone];
                        float fLeftTime = fTimeLeft[milestone];
                        blendInfo.mfMilestoneTime[milestone] = (fLeftTime <= 0.001f) ? 0.0f : Interpolate(fLeftTime, fRightTime, fComposite);
                    }

                    if (fComposite <= 0.5f)
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
        else
        {
            SaveData* pLast;
            SaveData* pCurEdge = pSaveData;
            while (pCurEdge != NULL)
            {
                pLast = pCurEdge;
                pCurEdge = pCurEdge->mpConnectedSaveData[3];
            }
            pEdge = pLast;
        }
    }
    else
    {
        SaveData* pLast;
        SaveData* pCurEdge = pSaveData;
        while (pCurEdge != NULL)
        {
            pLast = pCurEdge;
            pCurEdge = pCurEdge->mpConnectedSaveData[2];
        }
        pEdge = pLast;
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

                    blendInfo.mfMilestoneTime[milestone] = (fTime0 <= 0.001f) ? 0.0f : Interpolate(fTime0, fTime1, fPrimary);
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
                const float fNudge = 0.1f;

                if (fabsf(pDown->mv3SavePos.f.y - v3TargetPos.f.y) < fNudge)
                {
                    blendInfo.mv3BlendedSavePos.f.y = v3TargetPos.f.y;
                }
                else if (pDown->mv3SavePos.f.y > v3TargetPos.f.y)
                {
                    blendInfo.mv3BlendedSavePos.f.y -= fNudge;
                }
                else
                {
                    blendInfo.mv3BlendedSavePos.f.y += fNudge;
                }

                if (fabsf(pDown->mv3SavePos.f.z - v3TargetPos.f.z) < fNudge)
                {
                    blendInfo.mv3BlendedSavePos.f.z = v3TargetPos.f.z;
                }
                else if (pDown->mv3SavePos.f.z > v3TargetPos.f.z)
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
    return pClosest;
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

/**
 * Offset/Address/Size: 0x780 | 0x80053BA0 | size: 0x64C
 * TODO: 98.11% match - root save-data and row pointer registers remain shifted in nested grid traversal.
 */
void GoalieSave::AddAreaToGrid(SaveData* pSaveData)
{
    SaveData* pCur;
    nlVector3 v3TopRight;
    nlVector3 v3BotLeft;
    float yInc;
    float zInc;
    nlVector3 v3CurColPos;
    nlVector3 v3CurRowPos;
    SaveData* pCurBot;
    SaveData* pRightCorner;
    SaveData* pNextRight;
    SaveData* pNextNextRight;
    SaveData* pCurLeft;
    SaveData* pCurRight;
    SaveData* pCurUp;
    SaveData* pCurRightUp;
    SaveData* pClosest;
    float fCloseDist;

    pCur = pSaveData;
    while (pCur != NULL)
    {
        pCurRightUp = pCur;
        pCur = pCur->mpConnectedSaveData[3];
    }
    while (pCurRightUp != NULL)
    {
        pCurRightUp = pCurRightUp->mpConnectedSaveData[0];
    }

    pCur = pSaveData;
    while (pCur != NULL)
    {
        pCurUp = pCur;
        pCur = pCur->mpConnectedSaveData[2];
    }
    while (pCurUp != NULL)
    {
        pCurBot = pCurUp;
        pCurUp = pCurUp->mpConnectedSaveData[0];
    }

    {
        SaveData* end;
        pCur = pSaveData;
        while (pCur != NULL)
        {
            end = pCur;
            pCur = pCur->mpConnectedSaveData[3];
        }
        while (end != NULL)
        {
            pRightCorner = end;
            end = end->mpConnectedSaveData[1];
        }
    }

    {
        SaveData* end;
        pCur = pSaveData;
        while (pCur != NULL)
        {
            end = pCur;
            pCur = pCur->mpConnectedSaveData[2];
        }
        while (end != NULL)
        {
            end = end->mpConnectedSaveData[1];
        }
    }

    yInc = (float)(0.95 * (cField::GetNet(1.0f)->GetNetWidth() / 7.0f));
    zInc = (float)(0.95 * (cField::GetNet(1.0f)->GetNetHeight() / 5.0f));

    pCur = pCurBot;
    v3TopRight = pCurBot->mv3SavePos;
    while (pCur != NULL)
    {
        if (pCur->mv3SavePos.f.y > v3TopRight.f.y)
            v3TopRight.f.y = pCur->mv3SavePos.f.y;
        pCur = pCur->mpConnectedSaveData[1];
    }
    while (pCurBot != NULL)
    {
        if (pCurBot->mv3SavePos.f.z > v3TopRight.f.z)
            v3TopRight.f.z = pCurBot->mv3SavePos.f.z;
        pCurBot = pCurBot->mpConnectedSaveData[3];
    }

    pSaveData->mv3GroupMaxCoords = v3TopRight;
    float halfYInc = 0.51f * yInc;
    float halfZInc = 0.51f * zInc;
    v3TopRight.f.y += halfYInc;
    v3TopRight.f.z += halfZInc;

    pCur = pRightCorner;
    v3BotLeft = pRightCorner->mv3SavePos;
    while (pCur != NULL)
    {
        if (pCur->mv3SavePos.f.y < v3BotLeft.f.y)
            v3BotLeft.f.y = pCur->mv3SavePos.f.y;
        pCur = pCur->mpConnectedSaveData[0];
    }
    pCur = pRightCorner;
    while (pCur != NULL)
    {
        if (pCur->mv3SavePos.f.z < v3BotLeft.f.z)
            v3BotLeft.f.z = pCur->mv3SavePos.f.z;
        pCur = pCur->mpConnectedSaveData[2];
    }

    pSaveData->mv3GroupMinCoords = v3BotLeft;
    v3BotLeft.f.y -= halfYInc;
    v3BotLeft.f.z -= halfZInc;

    pNextRight = pRightCorner;
    pCurBot = pNextRight;
    pNextNextRight = pNextRight->mpConnectedSaveData[2];
    v3CurRowPos = v3BotLeft;

    while (v3CurRowPos.f.y < v3TopRight.f.y)
    {
        if (v3CurRowPos.f.y >= pNextRight->mv3SavePos.f.y && pNextNextRight != NULL)
        {
            pCurBot = pNextRight;
            pNextRight = pNextNextRight;
            pNextNextRight = pNextNextRight->mpConnectedSaveData[2];
        }
        pCurLeft = pCurBot;
        pCurRight = pNextRight;
        v3CurColPos = v3CurRowPos;

        while (v3CurColPos.f.z < v3TopRight.f.z)
        {
            pCurRightUp = pCurLeft;
            pCur = pCurLeft;
            for (; pCurRightUp != NULL && v3CurColPos.f.z > pCurRightUp->mv3SavePos.f.z; pCurRightUp = pCurRightUp->mpConnectedSaveData[0])
            {
                pCur = pCurRightUp;
            }
            for (; pCur != NULL && v3CurColPos.f.z < pCur->mv3SavePos.f.z; pCur = pCur->mpConnectedSaveData[1])
            {
                pCurRightUp = pCur;
            }
            if (pCur == NULL)
            {
                pCurLeft = pCurRightUp;
                pCurUp = pCurRightUp;
            }
            else if (pCurRightUp == NULL)
            {
                pCurLeft = pCur;
                pCurUp = pCur;
            }
            else
            {
                pCurUp = pCurRightUp;
                pCurLeft = pCur;
            }

            pCurRightUp = pCurRight;
            pCur = pCurRight;
            for (; pCurRightUp != NULL && v3CurColPos.f.z > pCurRightUp->mv3SavePos.f.z; pCurRightUp = pCurRightUp->mpConnectedSaveData[0])
            {
                pCur = pCurRightUp;
            }
            for (; pCur != NULL && v3CurColPos.f.z < pCur->mv3SavePos.f.z; pCur = pCur->mpConnectedSaveData[1])
            {
                pCurRightUp = pCur;
            }
            if (pCur == NULL)
            {
                pCurRight = pCurRightUp;
            }
            else if (pCurRightUp == NULL)
            {
                pCurRight = pCur;
                pCurRightUp = pCur;
            }
            else
            {
                pCurRight = pCur;
            }

            {
                float z = v3CurColPos.f.z;
                float y = v3CurColPos.f.y;
                float dy = pCurLeft->mv3SavePos.f.y - y;
                float dz = pCurLeft->mv3SavePos.f.z - z;
                fCloseDist = dy * dy + dz * dz;
                pClosest = pCurLeft;

                if (pCurLeft != pCurUp)
                {
                    float upDy = pCurUp->mv3SavePos.f.y - y;
                    float upDz = pCurUp->mv3SavePos.f.z - z;
                    float d = upDy * upDy + upDz * upDz;
                    if (d < fCloseDist)
                    {
                        fCloseDist = d;
                        pClosest = pCurUp;
                    }
                }

                if (pCurLeft != pCurRight)
                {
                    float rightDy = pCurRight->mv3SavePos.f.y - y;
                    float rightDz = pCurRight->mv3SavePos.f.z - z;
                    float d = rightDy * rightDy + rightDz * rightDz;
                    if (d < fCloseDist)
                    {
                        fCloseDist = d;
                        pClosest = pCurRight;
                    }
                    if (pCurRight != pCurRightUp)
                    {
                        float upRightDy = pCurRightUp->mv3SavePos.f.y - y;
                        float upRightDz = pCurRightUp->mv3SavePos.f.z - z;
                        float d2 = upRightDy * upRightDy + upRightDz * upRightDz;
                        if (d2 < fCloseDist)
                        {
                            fCloseDist = d2;
                            pClosest = pCurRightUp;
                        }
                    }
                }
            }

            pClosest->mv3GroupMinCoords = pSaveData->mv3GroupMinCoords;
            pClosest->mv3GroupMaxCoords = pSaveData->mv3GroupMaxCoords;
            AddPointToGrid(pClosest, v3CurColPos);
            v3CurColPos.f.z += zInc;
        }
        v3CurRowPos.f.y += yInc;
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
 * TODO: 98.77% match - save-data pointers, loop count, and grid-cell registers remain shifted in the inlined AddPointToGrid path.
 */
void GoalieSave::AddSegmentToGrid(SaveData* pSaveData1, SaveData* pSaveData2)
{
    int divisions;
    SaveData* pCurSaveData;
    int count;
    int i, j, m, n;
    nlVector3 v3CurPos;
    nlVector3 v3Delta;

    Local2GridCoords(pSaveData1->mv3SavePos.f.y, pSaveData1->mv3SavePos.f.z, i, j);
    Local2GridCoords(pSaveData2->mv3SavePos.f.y, pSaveData2->mv3SavePos.f.z, m, n);
    nlVec3Sub(v3Delta, pSaveData2->mv3SavePos, pSaveData1->mv3SavePos);
    divisions = abs(j - n) + abs(i - m);
    if (divisions > 0)
    {
        nlVec3Scale(v3Delta, v3Delta, 1.0f / (float)divisions);
    }
    v3CurPos = pSaveData1->mv3SavePos;
    for (count = 0; count <= divisions; count++)
    {
        float d2z = pSaveData2->mv3SavePos.f.z - v3CurPos.f.z;
        float d2y = pSaveData2->mv3SavePos.f.y - v3CurPos.f.y;
        float d1y = pSaveData1->mv3SavePos.f.y - v3CurPos.f.y;
        float d1z = pSaveData1->mv3SavePos.f.z - v3CurPos.f.z;
        if (d1z * d1z + d1y * d1y < d2z * d2z + d2y * d2y)
            pCurSaveData = pSaveData1;
        else
            pCurSaveData = pSaveData2;
        AddPointToGrid(pCurSaveData, v3CurPos);
        v3CurPos.f.z += v3Delta.f.z;
        v3CurPos.f.y += v3Delta.f.y;
        v3CurPos.f.x += v3Delta.f.x;
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
#pragma dont_inline on
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
#pragma dont_inline reset
