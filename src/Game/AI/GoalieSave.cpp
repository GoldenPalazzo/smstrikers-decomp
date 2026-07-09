#include "Game/AI/GoalieSave.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/FilteredRandom.h"
#include "Game/AnimInventory.h"
#include "Game/CharacterTriggers.h"
#include "Game/Field.h"
#include "Game/Goalie.h"
#include "Game/SAnim/pnSAnimController.h"
#include "NL/nlMemory.h"
#include "NL/nlString.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"

#pragma inline_depth(255)

static nlAVLTree<int, SaveData*, DefaultKeyCompare<int> > gSaveMap;
static nlListContainer<SaveData*> gSaveGrid[7][5];
static float fDefaultMilestoneValues[2] = { 0.4f, 0.7f };

template class nlListContainer<SaveData*>;
template class nlListContainer<int>;

struct MyMiniData;

struct SaveInfo
{
    int mnAnimID;
    int mnFailAnimID;
    int mnRecoverAnimID;
    unsigned int muSaveType;
    int mConnectedSaveID[4];
    char mszName[16];
};

inline void SaveData::PostInit(const SaveInfo& info)
{
    int failID = info.mnFailAnimID;
    SaveData* pFound;

    if (failID >= 0)
    {
        SaveData** ppFound;
        if (gSaveMap.FindGet(failID, &ppFound))
        {
            pFound = *ppFound;
        }
        else
        {
            pFound = NULL;
        }
    }
    else
    {
        pFound = NULL;
    }
    mpFailAnimData = pFound;

    for (int i = 0; i < 4; i++)
    {
        int connID = info.mConnectedSaveID[i];
        SaveData* pConn;
        if (connID >= 0)
        {
            SaveData** ppConn;
            if (gSaveMap.FindGet(connID, &ppConn))
            {
                pConn = *ppConn;
            }
            else
            {
                pConn = NULL;
            }
        }
        else
        {
            pConn = NULL;
        }
        mpConnectedSaveData[i] = pConn;
    }
}

static const nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };
int gPositionAnimID[6] = { 25, 28, 20, 23, 34, 35 };
SaveInfo gSaveInfo[70] = {
    { 47, 88, -1, 0x00000001, { -1, 48, 52, 57 }, "Ctch Ctr XHiJmp" },
    { 48, 89, -1, 0x00000001, { 47, 49, 53, 58 }, "Ctch Ctr Hi Jmp" },
    { 49, 90, -1, 0x00000001, { 48, 50, 54, 59 }, "Catch Ctr Hi" },
    { 50, 91, -1, 0x00000001, { 49, 51, 55, 60 }, "Catch Ctr Med" },
    { 51, 92, -1, 0x00000001, { 50, -1, 56, 61 }, "Catch Ctr Lo" },
    { 52, 93, -1, 0x00000001, { -1, 53, -1, 47 }, "Catch Rt XHiJmp" },
    { 53, 94, -1, 0x00000001, { 52, 54, -1, 48 }, "Catch Rt Hi Jmp" },
    { 54, 95, -1, 0x00000001, { 53, 55, -1, 49 }, "Catch Rt Hi" },
    { 55, 96, -1, 0x00000001, { 54, 56, -1, 50 }, "Catch Rt Med" },
    { 56, 97, -1, 0x00000001, { 55, -1, -1, 51 }, "Catch Rt Lo" },
    { 57, 98, -1, 0x00000001, { -1, 58, 47, -1 }, "Catch Lf XHiJmp" },
    { 58, 99, -1, 0x00000001, { 57, 59, 48, -1 }, "Catch Lft HiJmp" },
    { 59, 100, -1, 0x00000001, { 58, 60, 49, -1 }, "Catch Lft Hi" },
    { 60, 101, -1, 0x00000001, { 59, 61, 50, -1 }, "Catch Lft Med" },
    { 61, 101, -1, 0x00000001, { 60, -1, 51, -1 }, "Catch Lft Lo" },
    { 106, 79, -1, 0x00000002, { -1, -1, -1, -1 }, "DiveCatchRHiSpc" },
    { 107, 85, -1, 0x00000002, { -1, -1, -1, -1 }, "DiveCatchLHiSpc" },
    { 62, 76, 132, 0x00000002, { -1, 63, 68, -1 }, "Dive Catch R Hi" },
    { 63, 75, 132, 0x00000002, { 62, 64, 69, -1 }, "Dive Catch R Md" },
    { 64, 74, 132, 0x00000002, { 63, -1, 70, -1 }, "Dive Catch R Lo" },
    { 65, 82, 133, 0x00000002, { -1, 66, -1, 71 }, "Dive Catch L Hi" },
    { 66, 81, 133, 0x00000002, { 65, 67, -1, 72 }, "Dive Catch L Md" },
    { 67, 80, 133, 0x00000002, { 66, -1, -1, 73 }, "Dive Catch L Lo" },
    { 68, 79, 132, 0x00000002, { -1, 69, -1, 62 }, "DiveCtch RHiFar" },
    { 69, 78, 132, 0x00000002, { 68, 70, -1, 63 }, "DiveCtch RMdFar" },
    { 70, 77, 132, 0x00000002, { 69, -1, -1, 64 }, "DiveCtch RLoFar" },
    { 71, 85, 133, 0x00000002, { -1, 72, 65, -1 }, "DiveCtch LHiFar" },
    { 72, 84, 133, 0x00000002, { 71, 73, 66, -1 }, "DiveCtch LMdFar" },
    { 73, 83, 133, 0x00000002, { 72, -1, 67, -1 }, "DiveCtch LLoFar" },
    { 88, -1, -1, 0x00000004, { -1, 89, 93, 98 }, "Deflect XHi Jmp" },
    { 89, -1, -1, 0x00000004, { 88, 90, 94, 99 }, "Deflect Hi Jmp" },
    { 90, -1, -1, 0x00000004, { 89, 91, 95, 100 }, "Deflect Hi" },
    { 91, -1, -1, 0x00000004, { 90, 92, 96, 101 }, "Deflect Med" },
    { 92, -1, -1, 0x00000004, { 91, -1, 97, 102 }, "Deflect Lo" },
    { 93, -1, -1, 0x00000004, { -1, 94, -1, 88 }, "Defl R XHi Jmp" },
    { 94, -1, -1, 0x00000004, { 93, 95, -1, 89 }, "Defl R Hi Jmp" },
    { 95, -1, -1, 0x00000004, { 94, 96, -1, 90 }, "Deflect R Hi" },
    { 96, -1, -1, 0x00000004, { 95, 97, -1, 91 }, "Deflect R Med" },
    { 97, -1, -1, 0x00000004, { 96, -1, -1, 92 }, "Deflect R Lo" },
    { 98, -1, -1, 0x00000004, { -1, 99, 88, -1 }, "Defl L XHi Jmp" },
    { 99, -1, -1, 0x00000004, { 98, 100, 89, -1 }, "Defl L Hi Jmp" },
    { 100, -1, -1, 0x00000004, { 99, 101, 90, -1 }, "Deflect L Hi" },
    { 101, -1, -1, 0x00000004, { 100, 102, 91, -1 }, "Deflect L Med" },
    { 102, -1, -1, 0x00000004, { 101, -1, 92, -1 }, "Deflect L Lo" },
    { 76, -1, 134, 0x00000008, { -1, 75, 79, -1 }, "Dive Dfl R Hi" },
    { 75, -1, 134, 0x00000008, { 76, 74, 78, -1 }, "Dive Dfl R Med" },
    { 74, -1, 134, 0x00000008, { 75, -1, 77, -1 }, "Dive Dfl R Lo" },
    { 79, -1, 134, 0x00000008, { -1, 78, -1, 76 }, "DiveDfl R HiFar" },
    { 78, -1, 134, 0x00000008, { 79, 77, -1, 75 }, "DiveDfl R MdFar" },
    { 77, -1, 134, 0x00000008, { 78, -1, -1, 74 }, "DiveDfl R LoFar" },
    { 82, -1, 135, 0x00000008, { -1, 81, -1, 85 }, "Dive Dfl L Hi" },
    { 81, -1, 135, 0x00000008, { 82, 80, -1, 84 }, "Dive Dfl L Med" },
    { 80, -1, 135, 0x00000008, { 81, -1, -1, 83 }, "Dive Dfl L Lo" },
    { 85, -1, 135, 0x00000008, { -1, 84, 82, -1 }, "DiveDfl L HiFar" },
    { 84, -1, 135, 0x00000008, { 85, 83, 81, -1 }, "DiveDfl L MdFar" },
    { 83, -1, 135, 0x00000008, { 84, -1, 80, -1 }, "DiveDfl L LoFar" },
    { 86, -1, -1, 0x00000020, { -1, -1, -1, -1 }, "Leg R Lo" },
    { 87, -1, -1, 0x00000020, { -1, -1, -1, -1 }, "Leg L Lo" },
    { 103, -1, -1, 0x00000010, { -1, -1, 104, 105 }, "Punch Hi" },
    { 104, -1, -1, 0x00000010, { -1, -1, -1, 103 }, "Punch R Hi" },
    { 105, -1, -1, 0x00000010, { -1, -1, 103, -1 }, "Punch L Hi" },
    { 108, -1, -1, 0x00020000, { -1, -1, -1, -1 }, "S2S Blast Net" },
    { 114, -1, 116, 0x00040000, { -1, -1, -1, -1 }, "S2S Spin L" },
    { 115, -1, 117, 0x00040000, { -1, -1, -1, -1 }, "S2S Spin R" },
    { 110, -1, 111, 0x00010000, { -1, -1, -1, -1 }, "S2S Save Stun" },
    { 128, -1, 134, 0x00100000, { -1, -1, -1, -1 }, "Miss Chip R" },
    { 129, -1, 135, 0x00100000, { -1, -1, -1, -1 }, "Miss Chip L" },
    { 130, -1, 134, 0x00100000, { -1, -1, -1, -1 }, "MissChipShort R" },
    { 131, -1, 135, 0x00100000, { -1, -1, -1, -1 }, "MissChipShort L" },
    { -1, 0, 0, 0x00000000, { -1, -1, -1, -1 }, "Empty" },
};

/**
 * Offset/Address/Size: 0x0 | 0x80056BC8 | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before `stw r0, 0x24(r1)`.
 */
template void nlWalkDLRing<DLListEntry<MyMiniData*>, DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > > >(
    DLListEntry<MyMiniData*>* head,
    DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > >* callback,
    void (DLListContainerBase<MyMiniData*, NewAdapter<DLListEntry<MyMiniData*> > >::*callbackFunc)(DLListEntry<MyMiniData*>*));

typedef ListContainerBase<SaveData*, NewAdapter<ListEntry<SaveData*> > > SaveListBase;

static inline void ClearSaveGrid()
{
    for (int i = 0; i < 7; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            nlWalkList(gSaveGrid[i][j].m_Head, (SaveListBase*)&gSaveGrid[i][j], &SaveListBase::DeleteEntry);
            gSaveGrid[i][j].m_Head = NULL;
            gSaveGrid[i][j].m_Tail = NULL;
        }
    }
}

/**
 * Offset/Address/Size: 0x2B44 | 0x80055F64 | size: 0xE0
 */
void GoalieSave::ClearData()
{
    if (!mbInitialized)
    {
        return;
    }

    gSaveMap.Clear();

    ClearSaveGrid();

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
    SaveInfo* pSaveInfo;
    unsigned int i;
    int nCount;
    unsigned int count;
    SaveData* pSaveData;

    if (mbInitialized)
    {
        return;
    }

    muNumSaveEntries = 0x45;
    void* mem = nlMalloc(0x2290, 8, false);
    mpSaveTable = (SaveData*)__construct_new_array(mem, NULL, NULL, 0x80, 0x45);

    muSTSGoalIndexStart = 0;
    muSTSGoalCount = 0;
    muSTSMissIndexStart = 0;
    muSTSMissCount = 0;
    muSTSSaveIndexStart = 0;
    muSTSSaveCount = 0;
    muMissChipIndexStart = 0;
    muMissChipCount = 0;

    cSAnim* pAnim = pGoalie->m_pAnimInventory->GetAnim(0x2E);
    u32 numKeys = pAnim->m_nNumKeys;

    mfCrouchDuration = (float)numKeys / 30.0f;

    pSaveInfo = gSaveInfo;
    for (i = 0; i < muNumSaveEntries; i++)
    {
        pSaveData = &mpSaveTable[i];

        pSaveData->mnAnimID = pSaveInfo->mnAnimID;
        pSaveData->mnRecoverAnimID = pSaveInfo->mnRecoverAnimID;
        pSaveData->muSaveType = pSaveInfo->muSaveType;

        GoalieTweaks* pTweaks = (GoalieTweaks*)pGoalie->m_pTweaks;
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

        pSaveData->mv3SavePos = v3Zero;

        pSaveData->mfMilestonePercent[0] = 0.0f;
        pSaveData->mfMilestonePercent[1] = 0.0f;
        pSaveData->mfMilestonePercent[2] = 0.0f;
        pSaveData->mfMilestonePercent[3] = 0.0f;
        pSaveData->mfMilestonePercent[4] = 0.0f;

        pSaveData->mv3TakeoffPos = v3Zero;

        pSaveData->mv3GroupMinCoords = v3Zero;

        pSaveData->mv3GroupMaxCoords = v3Zero;

        nlStrNCpy<char>(pSaveData->mszName, pSaveInfo->mszName, 16);
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
        AVLTreeNode* pExistingNode;
        gSaveMap.AddAVLNode((AVLTreeNode**)&gSaveMap.m_Root, &animKey, &pValue, &pExistingNode, gSaveMap.m_NumElements);
        if (pExistingNode == NULL)
        {
            gSaveMap.m_NumElements++;
        }

        pSaveInfo++;
    }

    pSaveInfo = gSaveInfo;
    for (unsigned int j = 0; j < muNumSaveEntries; j++)
    {
        SaveData* pEntry = &mpSaveTable[j];
        pEntry->PostInit(*pSaveInfo);

        pSaveInfo++;
    }

    muNumPositionEntries = 6;
    void* mem2 = nlMalloc(0x70, 8, false);
    mpPositionTable = (SavePositionData*)__construct_new_array(mem2, NULL, NULL, 0x10, 6);

    for (count = 0; count < muNumPositionEntries; count++)
    {
        SavePositionData* pPos = &mpPositionTable[count];
        int animID = gPositionAnimID[count];
        pPos->mnAnimID = animID;

        cPN_SAnimController* pController = NULL;
        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock(&cPN_SAnimController::m_SAnimControllerSlotPool, 0x54);
        }
        if (cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList != NULL)
        {
            pController = (cPN_SAnimController*)cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList;
            cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList = cPN_SAnimController::m_SAnimControllerSlotPool.m_FreeList->m_next;
        }

        if (pController != NULL)
        {
            cAnimInventory* pAnimInv = pGoalie->m_pAnimInventory;
            bool bMirrored = pAnimInv->GetMirrored(animID);
            cSAnim* pSAnim = pAnimInv->GetAnim(animID);
            pController = new (pController) cPN_SAnimController(pSAnim, NULL, PM_HOLD, NULL, 0, bMirrored);
        }

        pController->m_fPrevTime = pController->m_fTime;
        pController->m_fTime = 1.0f;

        nlVector3 v3RootTrans;
        pController->GetRootTrans(&v3RootTrans, 0);

        pPos->mfAnimDistance = v3RootTrans.f.y;
        pPos->mfAnimTime = (float)pController->m_pSAnim->m_nNumKeys / 30.0f;
        pPos->mfAnimVelocity = pPos->mfAnimDistance / pPos->mfAnimTime;

        delete pController;
    }

    ClearSaveGrid();

    int nBallJointIndex = pGoalie->m_nBallJointIndex;

    for (i = 0; i < muNumSaveEntries; i++)
    {
        pSaveData = &mpSaveTable[i];
        GetAnimTriggerInfo(pGoalie, pSaveData->mnAnimID, TriggerCallback, pSaveData);
        pSaveData->mfMilestonePercent[4] = 1.0f;
        pGoalie->GetJointPositionFuture(&pSaveData->mv3SavePos, pSaveData->mnAnimID, nBallJointIndex, pSaveData->mfMilestonePercent[2], true, true, false);
        if (pSaveData->mfMilestonePercent[1] > 0.0f)
        {
            pGoalie->GetJointPositionFuture(&pSaveData->mv3TakeoffPos, pSaveData->mnAnimID, -1, pSaveData->mfMilestonePercent[1], true, true, false);
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
        if (current->entry->dist > data->dist)
        {
            if (nlDLRingIsStart(head, current))
            {
                DLListEntry<MyMiniData*>* entry = (DLListEntry<MyMiniData*>*)nlMalloc(0xC, 8, 0);
                if (entry != NULL)
                {
                    entry->m_next = NULL;
                    entry->m_prev = NULL;
                    entry->entry = data;
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
                entry->entry = data;
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
        entry->entry = data;
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
            MyMiniData* data = current->entry;
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
 * TODO: 97.38% match - list cursor and milestone-scale loop registers still differ.
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
        SaveData* pCur = pEntry->entry;

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
                float fDistSq = fDistY * fDistY
                              + (v3AdjLocalPos.f.z - tempBlendInfo.mv3BlendedSavePos.f.z)
                                    * (v3AdjLocalPos.f.z - tempBlendInfo.mv3BlendedSavePos.f.z);

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
                    fLastTime = 0.0f;
                    int segment;

                    for (segment = 0; segment < 5; segment++)
                    {
                        fThisTime = pConnected->mfMilestonePercent[segment] * pConnected->mfDuration;
                        if (fThisTime > 0.0f)
                        {
                            blendInfo.mfMilestoneScale[i][segment] = fThisTime - fLastTime;
                            fLastTime = fThisTime;
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
            fLastTime = 0.0f;
            for (i = 0; i < 5; i++)
            {
                fThisTime = blendInfo.mfMilestoneTime[i];
                if (fThisTime > 0.0f)
                {
                    float fSegDuration = fThisTime - fLastTime;
                    fInvSegTime = 1.0f / fSegDuration;
                    fLastTime = fThisTime;

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
 * TODO: 98.04% match - pClosest/pEdge register allocation still diverges in
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
                pRight = pCur;
                pLeft = pPrev;
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
                            pRight = pNextCur;
                            pLeft = pNextPrev;
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
                            pRightUp = pNextCur;
                            pLeftUp = pNextPrev;
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
            pUp = pCur;
            pDown = pPrev;
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
            if (entry->entry == pSaveData)
                return;
            entry = entry->next;
        }
    }

    {
        ListEntry<SaveData*>* newEntry = (ListEntry<SaveData*>*)nlMalloc(sizeof(ListEntry<SaveData*>), 8, false);
        if (newEntry != NULL)
        {
            newEntry->next = NULL;
            newEntry->entry = pSaveData;
        }
        nlListAddStart<ListEntry<SaveData*> >(&cell.m_Head, newEntry, &cell.m_Tail);
    }
}

static inline void FindVerticalBoundingPoints(SaveData* pSaveData, const nlVector3& v3TargetPoint, SaveData** pLoPoint, SaveData** pHiPoint)
{
    SaveData* pHiSaveData = pSaveData;
    SaveData* pLoSaveData = pSaveData;

    while (pHiSaveData != NULL && v3TargetPoint.f.z > pHiSaveData->mv3SavePos.f.z)
    {
        pLoSaveData = pHiSaveData;
        pHiSaveData = pHiSaveData->mpConnectedSaveData[0];
    }
    while (pLoSaveData != NULL && v3TargetPoint.f.z < pLoSaveData->mv3SavePos.f.z)
    {
        pHiSaveData = pLoSaveData;
        pLoSaveData = pLoSaveData->mpConnectedSaveData[1];
    }
    if (pLoSaveData == NULL)
    {
        *pLoPoint = pHiSaveData;
        *pHiPoint = pHiSaveData;
    }
    else if (pHiSaveData == NULL)
    {
        *pLoPoint = pLoSaveData;
        *pHiPoint = pLoSaveData;
    }
    else
    {
        *pHiPoint = pHiSaveData;
        *pLoPoint = pLoSaveData;
    }
}

/**
 * Offset/Address/Size: 0x780 | 0x80053BA0 | size: 0x64C
 */
void GoalieSave::AddAreaToGrid(SaveData* pSaveData)
{
    SaveData* const pRoot = pSaveData;
    SaveData* pCur;
    nlVector3 v3TopRight;
    nlVector3 v3BotLeft;
    float yInc;
    float zInc;
    nlVector3 v3CurColPos;
    nlVector3 v3CurRowPos;
    SaveData* pNextRight;
    SaveData* pCurBot;
    SaveData* pRightCorner;
    SaveData* pNextNextRight;
    SaveData* pCurLeft;
    SaveData* pCurRight;
    SaveData* pCurUp;
    SaveData* pCurRightUp;
    SaveData* pClosest;
    float fCloseDist;

    pCur = pRoot;
    while (pCur != NULL)
    {
        pCurRightUp = pCur;
        pCur = pCur->mpConnectedSaveData[3];
    }
    while (pCurRightUp != NULL)
    {
        pCurRightUp = pCurRightUp->mpConnectedSaveData[0];
    }

    pCur = pRoot;
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
        pCur = pRoot;
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
        pCur = pRoot;
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

    pRoot->mv3GroupMaxCoords = v3TopRight;
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

    pRoot->mv3GroupMinCoords = v3BotLeft;
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
            FindVerticalBoundingPoints(pCurLeft, v3CurColPos, &pCurLeft, &pCurUp);

            FindVerticalBoundingPoints(pCurRight, v3CurColPos, &pCurRight, &pCurRightUp);

            {
                float dy = pCurLeft->mv3SavePos.f.y - v3CurColPos.f.y;
                float dz = pCurLeft->mv3SavePos.f.z - v3CurColPos.f.z;
                fCloseDist = nlGetLengthSquared2D(dy, dz);
                pClosest = pCurLeft;

                if (pCurLeft != pCurUp)
                {
                    float upDy = pCurUp->mv3SavePos.f.y - v3CurColPos.f.y;
                    float upDz = pCurUp->mv3SavePos.f.z - v3CurColPos.f.z;
                    float d = nlGetLengthSquared2D(upDy, upDz);
                    if (d < fCloseDist)
                    {
                        fCloseDist = d;
                        pClosest = pCurUp;
                    }
                }

                if (pCurLeft != pCurRight)
                {
                    float rightDy = pCurRight->mv3SavePos.f.y - v3CurColPos.f.y;
                    float rightDz = pCurRight->mv3SavePos.f.z - v3CurColPos.f.z;
                    float d = nlGetLengthSquared2D(rightDy, rightDz);
                    if (d < fCloseDist)
                    {
                        fCloseDist = d;
                        pClosest = pCurRight;
                    }
                    if (pCurRight != pCurRightUp)
                    {
                        float upRightDy = pCurRightUp->mv3SavePos.f.y - v3CurColPos.f.y;
                        float upRightDz = pCurRightUp->mv3SavePos.f.z - v3CurColPos.f.z;
                        float d2 = nlGetLengthSquared2D(upRightDy, upRightDz);
                        if (d2 < fCloseDist)
                        {
                            fCloseDist = d2;
                            pClosest = pCurRightUp;
                        }
                    }
                }
            }

            pClosest->mv3GroupMinCoords = pRoot->mv3GroupMinCoords;
            pClosest->mv3GroupMaxCoords = pRoot->mv3GroupMaxCoords;
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
 * TODO: 98.99% match - save-data pointers, loop count, and grid-cell registers remain shifted in the inlined AddPointToGrid path.
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
        nlVec3Add(v3CurPos, v3CurPos, v3Delta);
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
