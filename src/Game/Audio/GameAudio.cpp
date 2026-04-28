#include "Game/GameAudio.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Game.h"
#include "Game/Physics/PhysicsObject.h"
#include "Game/Sys/debug.h"
#include "NL/nlTask.h"
#include "NL/nlString.h"
#include "types.h"

SlotPool<SFXPlaySet> SFXPlaySet::m_TrackedSFXSlotPool(0x32, 0x10);

/**
 * Offset/Address/Size: 0x2128 | 0x8015366C | size: 0x30
 */
bool TrackedSFXPitchFreqTypeCheckCallback(unsigned long sfxID, cGameSFX* pGameSFX)
{
    const int volGrp = pGameSFX->mpSFX[sfxID].volGrp;
    return (bool)(volGrp >= 5 && volGrp <= 19);
}

inline bool IsVolGrpInRange(unsigned long sfxID, cGameSFX* pGameSFX)
{
    const int volGrp = pGameSFX->mpSFX[sfxID].volGrp;
    return (volGrp >= 5 && volGrp <= 19);
}

bool TrackedSFXFilterFreqTypeCheckCallback(unsigned long sfxID, cGameSFX* pGameSFX);
bool TrackedSFXPriorityCallback(SFXPlaySet* pSFXPlaySet, unsigned long param, cGameSFX* pGameSFX);

static inline bool IsGroupPitchType(unsigned long type, cGameSFX* pGameSFX)
{
    const int volGrp = pGameSFX->mpSFX[type].volGrp;
    return (volGrp >= 5 && volGrp <= 19);
}

static inline bool ShouldApplyGroupFilter(unsigned long type, cGameSFX* pGameSFX)
{
    if (pGameSFX->GetClassType() == WORLD && type == 0xBB)
    {
        return false;
    }

    if (IsGroupPitchType(type, pGameSFX))
    {
        return false;
    }

    return true;
}

static inline void ResetTrackedPlaySet(SFXPlaySet* pPlaySet)
{
    pPlaySet->type = (unsigned long)-1;
    pPlaySet->voiceID = Audio::GetSndIDError();
    pPlaySet->bIs3D = false;
    pPlaySet->emitter = NULL;
    pPlaySet->delay = -1.0f;
    pPlaySet->timeStamp = -1.0f;
    pPlaySet->sfxPriority = 0;
    pPlaySet->groupPriority = -1;
    pPlaySet->filterFreq = 0;
    pPlaySet->pitch = 0x2000;

    SlotPoolEntry* pFreeEntry = (SlotPoolEntry*)pPlaySet;
    pFreeEntry->m_next = SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList;
    SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList = pFreeEntry;
}

static inline SFXPlaySet* RemoveTrackedPlaySet(cGameSFX* pGameSFX, DLListEntry<SFXPlaySet*>* head, DLListEntry<SFXPlaySet*>* pEntry)
{
    SFXPlaySet* pPlaySet = pEntry->m_data;
    nlDLRingIsEnd(head, pEntry);
    nlDLRingRemove(&pGameSFX->mpCurPlaySet.m_Head, pEntry);
    delete pEntry;
    ResetTrackedPlaySet(pPlaySet);
    return pPlaySet;
}

static inline void UpdateGroupFilterStatusOnSFX(cGameSFX* pGameSFX, SFXPlaySet* pTrackedSFX)
{
    bool bResult = ShouldApplyGroupFilter(pTrackedSFX->type, pGameSFX);

    if (!bResult)
    {
        return;
    }

    if (pGameSFX->mbGroupFilterOn)
    {
        if (pTrackedSFX->bIs3D)
        {
            if (Audio::IsEmitterActive(pTrackedSFX->emitter))
            {
                pTrackedSFX->voiceID = Audio::GetEmitterVoiceID(pTrackedSFX->emitter);
            }
        }

        Audio::ActivateFilterOnSFX(pTrackedSFX->voiceID, true);

        if (pTrackedSFX->filterFreq != pGameSFX->muGroupFilterFreq)
        {
            bResult = Audio::SetFilterFreqOnSFX(pTrackedSFX->voiceID, pGameSFX->muGroupFilterFreq);
            if (!bResult)
            {
                tDebugPrintManager::Print(
                    DC_SOUND,
                    "SetFilterFreqOnSFX() returned false on type %d, voice ID %d, with frequency %d.\n",
                    pTrackedSFX->type,
                    pTrackedSFX->voiceID,
                    pGameSFX->muGroupFilterFreq);
            }
            pTrackedSFX->filterFreq = pGameSFX->muGroupFilterFreq;
        }
    }
    else
    {
        pGameSFX->muGroupFilterFreq = 0;
        Audio::ActivateFilterOnSFX(pTrackedSFX->voiceID, false);
        Audio::SetFilterFreqOnSFX(pTrackedSFX->voiceID, pGameSFX->muGroupFilterFreq);
        pTrackedSFX->filterFreq = pGameSFX->muGroupFilterFreq;
    }
}

static inline void UpdateGroupPitchStatusOnSFX(cGameSFX* pGameSFX, SFXPlaySet* pTrackedSFX)
{
    bool bResult = IsGroupPitchType(pTrackedSFX->type, pGameSFX);

    if (!bResult)
    {
        return;
    }

    bResult = Audio::SetPitchBendOnSFX(pTrackedSFX->voiceID, pGameSFX->muGroupPitch);
    if (!bResult)
    {
        tDebugPrintManager::Print(
            DC_SOUND,
            "SetPitchBendOnSFX() returned false on type %d, voice ID %d, with pitch %d.\n",
            pTrackedSFX->type,
            pTrackedSFX->voiceID,
            pGameSFX->muGroupPitch);
    }

    pTrackedSFX->pitch = pGameSFX->muGroupPitch;
}

static inline void ResetTrackedEmitter(SFXEmitter* pSFXEmitter)
{
    pSFXEmitter->bKeepTrack = true;
    pSFXEmitter->soundType = (unsigned long)-1;
    pSFXEmitter->fTimeStamp = -1.0f;
    pSFXEmitter->bIsStopping = false;
    pSFXEmitter->bInUse = false;
    pSFXEmitter->bIsFilterOn = false;
    pSFXEmitter->m_unk_0x5F = false;
    pSFXEmitter->pPhysObj = NULL;
    pSFXEmitter->pOwner = NULL;
    pSFXEmitter->pos.pvPos = NULL;
    pSFXEmitter->dir.pvDir = NULL;
    pSFXEmitter->pos.vPos.f.x = 0.0f;
    pSFXEmitter->pos.vPos.f.y = 0.0f;
    pSFXEmitter->pos.vPos.f.z = 0.0f;
    pSFXEmitter->dir.vDir.f.x = 0.0f;
    pSFXEmitter->dir.vDir.f.y = 0.0f;
    pSFXEmitter->dir.vDir.f.z = 0.0f;
    pSFXEmitter->posUpdateMethod = NONE;

    if (pSFXEmitter->pMIDIControllerInfo != NULL)
    {
        if (pSFXEmitter->pMIDIControllerInfo->paraArray != NULL)
        {
            delete[] pSFXEmitter->pMIDIControllerInfo->paraArray;
        }
        delete pSFXEmitter->pMIDIControllerInfo;
    }

    pSFXEmitter->pMIDIControllerInfo = NULL;
}

/**
 * Offset/Address/Size: 0x0 | 0x80151544 | size: 0x8D8
 */
void cGameSFX::UpdateAllTrackedSFX(float)
{
    DLListEntry<SFXPlaySet*>* head;
    DLListEntry<SFXPlaySet*>* current;
    DLListEntry<SFXPlaySet*>* iter;
    SFXPlaySet* pTrackedSFX;
    float currTime;

    if (!mbCurPlaySetIsValid)
    {
        return;
    }

    current = nlDLRingGetStart(mpCurPlaySet.m_Head);
    head = mpCurPlaySet.m_Head;

    while (current != NULL)
    {
        iter = current;
        pTrackedSFX = current->m_data;

        if (nlDLRingIsEnd(head, current) || current == NULL)
        {
            current = NULL;
        }
        else
        {
            current = current->m_next;
        }

        if (pTrackedSFX->type == (unsigned long)-1)
        {
            RemoveTrackedPlaySet(this, head, iter);
            continue;
        }

        currTime = Audio::GetAudioTimer();

        if (pTrackedSFX->delay >= 0.0f)
        {
            if (currTime > pTrackedSFX->timeStamp + pTrackedSFX->delay + 15.0f)
            {
                RemoveTrackedPlaySet(this, head, iter);
            }
            continue;
        }

        if (pTrackedSFX->bIs3D)
        {
            if (pTrackedSFX->emitter != NULL)
            {
                if (pTrackedSFX->type != pTrackedSFX->emitter->soundType || pTrackedSFX->emitter->pOwner != this)
                {
                    RemoveTrackedPlaySet(this, head, iter);
                    continue;
                }

                if (Audio::IsEmitterActive(pTrackedSFX->emitter))
                {
                    unsigned long sndIDError = Audio::GetSndIDError();
                    if (Audio::GetEmitterVoiceID(pTrackedSFX->emitter) != sndIDError)
                    {
                        if (g_pGame != NULL)
                        {
                            pTrackedSFX->voiceID = Audio::GetEmitterVoiceID(pTrackedSFX->emitter);
                            UpdateGroupFilterStatusOnSFX(this, pTrackedSFX);
                            UpdateGroupPitchStatusOnSFX(this, pTrackedSFX);
                        }
                        continue;
                    }
                }

                if (!Audio::IsEmitterActive(pTrackedSFX->emitter) && pTrackedSFX->timeStamp > -1.0f && currTime > pTrackedSFX->timeStamp + 0.1f)
                {
                    UpdateGroupFilterStatusOnSFX(this, pTrackedSFX);
                    UpdateGroupPitchStatusOnSFX(this, pTrackedSFX);

                    ResetTrackedEmitter(pTrackedSFX->emitter);
                    RemoveTrackedPlaySet(this, head, iter);
                }

                continue;
            }

            if (pTrackedSFX->voiceID != Audio::GetSndIDError())
            {
                if (!Audio::IsSFXPlaying(pTrackedSFX->voiceID))
                {
                    RemoveTrackedPlaySet(this, head, iter);
                }
                continue;
            }

            nlPrintf("Could not get valid emitter or voice ID to check sound type %d\n", pTrackedSFX->type);
            continue;
        }

        if (pTrackedSFX->voiceID != Audio::GetSndIDError())
        {
            if (!Audio::IsSFXPlaying(pTrackedSFX->voiceID))
            {
                RemoveTrackedPlaySet(this, head, iter);
            }
            continue;
        }

        RemoveTrackedPlaySet(this, head, iter);
        nlPrintf("Could not get valid emitter or voice ID to check sound type %d\n", pTrackedSFX->type);
    }
}

/**
 * Offset/Address/Size: 0x8D8 | 0x80151E1C | size: 0x9C
 */
void cGameSFX::StopPlayingAllTrackedSFX()
{
    FORCE_DONT_INLINE;
    if (!mbCurPlaySetIsValid)
    {
        return;
    }

    DLListEntry<SFXPlaySet*>* start = nlDLRingGetStart(mpCurPlaySet.m_Head);
    DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
    DLListEntry<SFXPlaySet*>* current = start;

    while (current != NULL)
    {
        struct
        {
            DLListEntry<SFXPlaySet*>* m_head;
            DLListEntry<SFXPlaySet*>* m_current;
        } iter;

        iter.m_head = head;
        iter.m_current = current;

        if (nlDLRingIsEnd(head, current) || current == NULL)
        {
            current = NULL;
        }
        else
        {
            current = current->m_next;
        }

        StopTrackedSFX(reinterpret_cast<nlDLListIterator<SFXPlaySet*>*>(&iter));
    }
}

/**
 * Offset/Address/Size: 0x974 | 0x80151EB8 | size: 0x24C
 */
bool cGameSFX::StopTrackedSFX(nlDLListIterator<SFXPlaySet*>* pIterator)
{
    struct EntryIterator
    {
        DLListEntry<SFXPlaySet*>* m_head;
        DLListEntry<SFXPlaySet*>* m_current;
    };
    EntryIterator* pIter = reinterpret_cast<EntryIterator*>(pIterator);

    SFXEmitter* pEmitter;
    SFXPlaySet* pPlaySet = pIter->m_current->m_data;

    if (pPlaySet->delay >= 0.0f)
    {
        int delayedIndex = Audio::IsDelayedCharSFX(pPlaySet->type, this);
        if (delayedIndex == -1)
        {
            goto cleanup_list;
        }

        Audio::RemoveDelayedSFX(delayedIndex);
        return true;
    }

    if (pPlaySet->bIs3D != 0)
    {
        pEmitter = pPlaySet->emitter;
        if (pEmitter != NULL && pEmitter->soundType == pPlaySet->type && pEmitter->pOwner == this)
        {
            bool removed = Audio::Remove3DSFXEmitter(pEmitter);
            pEmitter->bIsStopping = true;

            if (!removed)
            {
                goto cleanup_list;
            }

            if (Audio::IsEmitterActive(pEmitter))
            {
                goto cleanup_list;
            }

            pEmitter->bKeepTrack = true;
            pEmitter->soundType = (unsigned long)-1;
            pEmitter->fTimeStamp = -1.0f;
            pEmitter->bIsStopping = false;
            pEmitter->bInUse = false;
            pEmitter->bIsFilterOn = false;
            pEmitter->m_unk_0x5F = false;
            pEmitter->pPhysObj = NULL;
            pEmitter->pOwner = NULL;
            pEmitter->pos.pvPos = NULL;
            pEmitter->dir.pvDir = NULL;
            pEmitter->pos.vPos.f.x = 0.0f;
            pEmitter->pos.vPos.f.y = 0.0f;
            pEmitter->pos.vPos.f.z = 0.0f;
            pEmitter->dir.vDir.f.x = 0.0f;
            pEmitter->dir.vDir.f.y = 0.0f;
            pEmitter->dir.vDir.f.z = 0.0f;
            pEmitter->posUpdateMethod = NONE;

            if (pEmitter->pMIDIControllerInfo != NULL)
            {
                if (pEmitter->pMIDIControllerInfo->paraArray != NULL)
                {
                    delete[] pEmitter->pMIDIControllerInfo->paraArray;
                }
                delete pEmitter->pMIDIControllerInfo;
            }
            pEmitter->pMIDIControllerInfo = NULL;
            goto cleanup_list;
        }

        if (pPlaySet->voiceID != Audio::GetSndIDError())
        {
            Audio::StopSFX(pPlaySet->voiceID);
            goto cleanup_list;
        }

        nlPrintf("Stopping stop sound type %d\n which no longer has a valid emitter.\n", pPlaySet->type);
        goto cleanup_list;
    }

    Audio::StopSFX(pPlaySet->voiceID);

cleanup_list:
    DLListEntry<SFXPlaySet*>* pNextEntry = pIter->m_current;
    pPlaySet = pNextEntry->m_data;

    if (nlDLRingIsEnd(pIter->m_head, pNextEntry) || pIter->m_current == NULL)
    {
        pIter->m_current = NULL;
    }
    else
    {
        pIter->m_current = pIter->m_current->m_next;
    }

    nlDLRingRemove(&mpCurPlaySet.m_Head, pNextEntry);
    delete pNextEntry;

    pPlaySet->type = (unsigned long)-1;
    pPlaySet->voiceID = Audio::GetSndIDError();
    pPlaySet->bIs3D = false;
    pPlaySet->emitter = NULL;
    pPlaySet->delay = -1.0f;
    pPlaySet->timeStamp = -1.0f;
    pPlaySet->sfxPriority = 0;
    pPlaySet->groupPriority = -1;
    pPlaySet->filterFreq = 0;
    pPlaySet->pitch = 0x2000;

    SlotPoolEntry* pFreeEntry = (SlotPoolEntry*)pPlaySet;
    pFreeEntry->m_next = SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList;
    SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList = pFreeEntry;

    return true;
}

/**
 * Offset/Address/Size: 0xBC0 | 0x80152104 | size: 0xC4
 */
bool cGameSFX::StopTrackedSFX(SFXPlaySet* pPlaySet)
{
    FORCE_DONT_INLINE;
    if (!mbCurPlaySetIsValid)
    {
        return true;
    }

    struct
    {
        DLListEntry<SFXPlaySet*>* m_head;
        DLListEntry<SFXPlaySet*>* m_current;
    } iter;

    DLListEntry<SFXPlaySet*>* start = nlDLRingGetStart(mpCurPlaySet.m_Head);
    iter.m_head = mpCurPlaySet.m_Head;
    iter.m_current = start;

    while (iter.m_current != NULL)
    {
        if (pPlaySet == iter.m_current->m_data)
        {
            return StopTrackedSFX(reinterpret_cast<nlDLListIterator<SFXPlaySet*>*>(&iter));
        }

        if (nlDLRingIsEnd(iter.m_head, iter.m_current) || iter.m_current == NULL)
        {
            iter.m_current = NULL;
        }
        else
        {
            iter.m_current = iter.m_current->m_next;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0xC84 | 0x801521C8 | size: 0x150
 */
void cGameSFX::StopEmitter(SFXEmitter* pSFXEmitter, unsigned long type)
{
    if (pSFXEmitter == NULL)
    {
        return;
    }

    for (s32 i = 0; i < 0x40; i++)
    {
        SFXEmitter* pEmitter = Audio::GetEmitter(i);
        if (pEmitter == NULL || pSFXEmitter != pEmitter)
        {
            continue;
        }

        if ((type != 0 && pSFXEmitter->soundType != type) || pSFXEmitter->pOwner != this)
        {
            tDebugPrintManager::Print(DC_SOUND, "StopEmitter - emitter found but wrong owner or type\n");
            return;
        }

        // Match found
        pSFXEmitter->bIsStopping = true;
        unsigned char bResult = Audio::Remove3DSFXEmitter(pSFXEmitter);
        if (!bResult)
        {
            return;
        }

        pSFXEmitter->bKeepTrack = true;
        pSFXEmitter->soundType = (unsigned long)-1;
        pSFXEmitter->fTimeStamp = -1.0f;
        pSFXEmitter->bIsStopping = false;
        pSFXEmitter->bInUse = false;
        pSFXEmitter->bIsFilterOn = false;
        pSFXEmitter->m_unk_0x5F = false;
        pSFXEmitter->pPhysObj = NULL;
        pSFXEmitter->pOwner = NULL;
        pSFXEmitter->pos.pvPos = NULL;
        pSFXEmitter->dir.pvDir = NULL;
        pSFXEmitter->pos.vPos.f.x = 0.0f;
        pSFXEmitter->pos.vPos.f.y = 0.0f;
        pSFXEmitter->pos.vPos.f.z = 0.0f;
        pSFXEmitter->dir.vDir.f.x = 0.0f;
        pSFXEmitter->dir.vDir.f.y = 0.0f;
        pSFXEmitter->dir.vDir.f.z = 0.0f;
        pSFXEmitter->posUpdateMethod = (PosUpdateMethod)0;

        if (pSFXEmitter->pMIDIControllerInfo != NULL)
        {
            if (pSFXEmitter->pMIDIControllerInfo->paraArray != NULL)
            {
                delete[] pSFXEmitter->pMIDIControllerInfo->paraArray;
            }
            delete pSFXEmitter->pMIDIControllerInfo;
        }
        pSFXEmitter->pMIDIControllerInfo = NULL;
        return;
    }
}

/**
 * Offset/Address/Size: 0xDD4 | 0x80152318 | size: 0x12C
 */
void cGameSFX::Stop(unsigned long soundID, cGameSFX::StopFlag stopFlag)
{
    if (!mbCurPlaySetIsValid)
    {
        return;
    }

    f32 oldestTimeStamp = -1.0f;

    struct
    {
        DLListEntry<SFXPlaySet*>* m_head;
        DLListEntry<SFXPlaySet*>* m_current;
    } bestIter;

    DLListEntry<SFXPlaySet*>* start1 = nlDLRingGetStart(mpCurPlaySet.m_Head);
    bestIter.m_head = mpCurPlaySet.m_Head;
    bestIter.m_current = start1;

    struct
    {
        DLListEntry<SFXPlaySet*>* m_head;
        DLListEntry<SFXPlaySet*>* m_current;
    } iter;

    DLListEntry<SFXPlaySet*>* start2 = nlDLRingGetStart(bestIter.m_head);
    DLListEntry<SFXPlaySet*>* loopHead = mpCurPlaySet.m_Head;
    DLListEntry<SFXPlaySet*>* current = start2;

    while (current != NULL)
    {
        SFXPlaySet* playSet = current->m_data;

        iter.m_head = loopHead;
        iter.m_current = current;

        if (nlDLRingIsEnd(loopHead, current) || current == NULL)
        {
            current = NULL;
        }
        else
        {
            current = current->m_next;
        }

        if (playSet->type != soundID)
        {
            continue;
        }

        if (stopFlag == SFX_STOP_OLDEST)
        {
            if (oldestTimeStamp != -1.0f && !(playSet->timeStamp < oldestTimeStamp))
            {
                continue;
            }
            oldestTimeStamp = playSet->timeStamp;
            bestIter.m_head = iter.m_head;
            bestIter.m_current = iter.m_current;
            continue;
        }

        StopTrackedSFX(reinterpret_cast<nlDLListIterator<SFXPlaySet*>*>(&iter));

        if (stopFlag != SFX_STOP_ALL)
        {
            break;
        }
    }

    if (stopFlag == SFX_STOP_OLDEST && oldestTimeStamp != -1.0f)
    {
        StopTrackedSFX(reinterpret_cast<nlDLListIterator<SFXPlaySet*>*>(&bestIter));
    }
}

/**
 * Offset/Address/Size: 0xF00 | 0x80152444 | size: 0x204
 */
SFXPlaySet* cGameSFX::KeepTrack(SFXEmitter* pEmitter, const Audio::SoundAttributes& attrs, unsigned long sndID)
{
    if (!mbCurPlaySetIsValid)
    {
        return NULL;
    }

    if (attrs.mb_Is3D && !attrs.mb_Update3DContinuously)
    {
        return NULL;
    }

    SFXPlaySet* slot = NULL;

    if (SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&SFXPlaySet::m_TrackedSFXSlotPool, sizeof(SFXPlaySet));
    }
    if (SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList != NULL)
    {
        slot = (SFXPlaySet*)SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList;
        SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList = SFXPlaySet::m_TrackedSFXSlotPool.m_FreeList->m_next;
    }

    slot->type = (unsigned long)-1;
    slot->voiceID = Audio::GetSndIDError();
    slot->bIs3D = 0;
    slot->emitter = NULL;
    slot->delay = 0.0f;
    slot->timeStamp = 0.0f;
    slot->sfxPriority = 0;
    slot->groupPriority = -1;
    slot->filterFreq = 0;
    slot->pitch = 0x2000;
    slot->type = attrs.mu_Type;
    if (!attrs.mf_ReturnEmitterOnPlay)
    {
        slot->voiceID = sndID;
    }

    slot->delay = attrs.mf_DelayTime;
    if (attrs.mb_Is3D)
    {
        slot->bIs3D = 1;
    }
    else
    {
        slot->bIs3D = 0;
    }

    slot->timeStamp = Audio::GetAudioTimer();

    if (attrs.mb_Is3D && attrs.mb_Update3DContinuously && 0.0f == attrs.mf_DelayTime)
    {
        slot->emitter = pEmitter;
        pEmitter->bKeepTrack = true;
        if (attrs.posUpdateMethod == PHYSOBJ)
        {
            pEmitter->pPhysObj = attrs.mp_PhysObj;
        }
        else if (attrs.posUpdateMethod == PTRS_TO_VECTORS)
        {
            pEmitter->pos.pvPos = attrs.pos.pvPos;
            pEmitter->dir.pvDir = attrs.dir.pvDir;
        }
        else if (attrs.posUpdateMethod == VECTORS)
        {
            pEmitter->pos.vPos = attrs.pos.vPos;
            pEmitter->dir.vDir = attrs.dir.vDir;
        }
    }

    DLListEntry<SFXPlaySet*>* entry = (DLListEntry<SFXPlaySet*>*)nlMalloc(sizeof(DLListEntry<SFXPlaySet*>), 8, false);
    if (entry != NULL)
    {
        entry->m_next = NULL;
        entry->m_prev = NULL;
        entry->m_data = slot;
    }
    nlDLRingAddStart(&mpCurPlaySet.m_Head, entry);

    return slot;
}

/**
 * Offset/Address/Size: 0x1104 | 0x80152648 | size: 0xACC
 * TODO: 71.42% match - prologue frame/register assignment differs and 3D setup path still has stack-slot/register ordering diffs.
 */
unsigned long cGameSFX::Play(Audio::SoundAttributes& attributes)
{
    SFXEmitter* pSFXEmitter;
    unsigned long emitterIndex;
    SFXPlaySet* pTrackedSFX;
    float currTime;
    float fVolume;
    float fVolReverb;
    unsigned long voiceID;
    nlVector3 vPos;
    nlVector3 vDir;
    EmitterStartInfo info;
    unsigned long sfxID;
    int priority;

    pSFXEmitter = NULL;
    emitterIndex = Audio::GetSndIDError();

    if (!Audio::IsInited() || !mbInited)
    {
        return Audio::GetSndIDError();
    }

    sfxID = mpSFX[attributes.mu_Type].musyxID;

    if (!attributes.m_unk_0x7B && g_pGame != NULL && g_pGame->mbCaptainShotToScoreOn && nlTaskManager::m_pInstance->m_CurrState != 1)
    {
        if (attributes.me_ClassType == WORLD)
        {
            if (attributes.mu_Type != 0xB5 && attributes.mu_Type != 0xBC && attributes.mu_Type != 0xBD && attributes.mu_Type != 0x94
                && attributes.mu_Type != 0x95 && attributes.mu_Type != 0x96 && attributes.mu_Type != 0x92 && attributes.mu_Type != 0x93
                && attributes.mu_Type != 0xCA && attributes.mu_Type != 0x1C)
            {
                return Audio::GetSndIDError();
            }
        }
        else if (attributes.me_ClassType == CHAR)
        {
            if (attributes.mu_Type != 0x4F && attributes.mu_Type != 0x4E && attributes.mu_Type != 0x4F && attributes.mu_Type != 0x50
                && attributes.mu_Type != 0x3B && attributes.mu_Type != 0x3A && attributes.mu_Type != 0x3C && attributes.mu_Type != 0x62
                && attributes.mu_Type != 0x5D)
            {
                return Audio::GetSndIDError();
            }
        }
    }

    pTrackedSFX = NULL;

    if (attributes.mb_NoPhasingFilter)
    {
        if (mbCurPlaySetIsValid)
        {
            DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
            DLListEntry<SFXPlaySet*>* current = nlDLRingGetStart(head);

            while (current != NULL)
            {
                SFXPlaySet* tracked = current->m_data;

                if (tracked->type == attributes.mu_Type)
                {
                    if (tracked->bIs3D)
                    {
                        if (tracked->emitter != NULL && Audio::IsEmitterActive(tracked->emitter))
                        {
                            tracked->voiceID = Audio::GetEmitterVoiceID(tracked->emitter);
                        }
                    }

                    if ((tracked->delay < 0.0f && tracked->bIs3D && tracked->voiceID == (unsigned long)Audio::GetSndIDError()) || Audio::IsSFXPlaying(tracked->voiceID)
                        || tracked->delay >= 0.0f)
                    {
                        pTrackedSFX = tracked;
                        break;
                    }
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
        }

        if (pTrackedSFX != NULL)
        {
            currTime = Audio::GetAudioTimer();

            if (currTime - pTrackedSFX->timeStamp < 0.01f && attributes.mf_DelayTime == pTrackedSFX->delay)
            {
                tDebugPrintManager::Print(DC_SOUND, "SFX Repeat Filter prevented a duplicate instance of type %d playing at %0.2f\n", attributes.mu_Type, currTime);
                tDebugPrintManager::Print(
                    DC_SOUND, "Last instance of type %d played at %0.2f. Threshold time is %0.2f\n", attributes.mu_Type, pTrackedSFX->timeStamp, 0.01f);
                return Audio::GetSndIDError();
            }
        }
    }

    {
        SoundStrToIDNode sfxInfo = mpSFX[attributes.mu_Type];
        if (attributes.mf_Volume == 100.0f)
        {
            fVolume = sfxInfo.fVolume;
        }
        else
        {
            fVolume = attributes.mf_Volume;
        }

        SoundStrToIDNode sfxInfo2 = mpSFX[attributes.mu_Type];
        if (attributes.mf_VolReverb == 100.0f)
        {
            fVolReverb = sfxInfo2.fVolReverb;
        }
        else
        {
            fVolReverb = attributes.mf_VolReverb;
        }

        if (fVolReverb == 0.0f)
        {
            fVolReverb = 100.0f;
        }

        fVolume += attributes.mf_VolAdjustment;

        if (sfxInfo.volGrp > -1)
        {
            Audio::SetSFXVolumeGroup(sfxID, sfxInfo.volGrp);
        }

        priority = attributes.mi_GroupPriority;
        if (priority >= 0)
        {
            bool hasHigherPriority = false;
            if (mbCurPlaySetIsValid)
            {
                DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
                DLListEntry<SFXPlaySet*>* current = nlDLRingGetStart(head);
                while (current != NULL)
                {
                    SFXPlaySet* tracked = current->m_data;
                    if (tracked->type != (unsigned long)-1)
                    {
                        int trackedPriority = mpSFX[tracked->type].sfxPriority;
                        if (trackedPriority > 0 && trackedPriority < priority)
                        {
                            if (tracked->bIs3D)
                            {
                                if (tracked->emitter != NULL && Audio::IsEmitterActive(tracked->emitter))
                                {
                                    tracked->voiceID = Audio::GetEmitterVoiceID(tracked->emitter);
                                    hasHigherPriority = true;
                                }
                            }
                            else if (Audio::IsSFXPlaying(tracked->voiceID))
                            {
                                hasHigherPriority = true;
                            }
                            if (hasHigherPriority)
                            {
                                break;
                            }
                        }
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
            }

            if (hasHigherPriority)
            {
                return Audio::GetSndIDError();
            }

            if (mbCurPlaySetIsValid)
            {
                DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
                DLListEntry<SFXPlaySet*>* current = nlDLRingGetStart(head);
                while (current != NULL)
                {
                    SFXPlaySet* tracked = current->m_data;

                    if (nlDLRingIsEnd(head, current) || current == NULL)
                    {
                        current = NULL;
                    }
                    else
                    {
                        current = current->m_next;
                    }

                    if (tracked->type == (unsigned long)-1)
                    {
                        continue;
                    }

                    TrackedSFXPriorityCallback(tracked, (unsigned long)priority, this);
                }
            }
        }

        attributes.mi_SFXPriority = mpSFX[attributes.mu_Type].sfxPriority;
    }

    priority = attributes.mi_SFXPriority;
    if (priority > 0 && attributes.mf_DelayTime < 0.0f)
    {
        bool hasHigherPriority = false;
        if (mbCurPlaySetIsValid)
        {
            DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
            DLListEntry<SFXPlaySet*>* current = nlDLRingGetStart(head);
            while (current != NULL)
            {
                SFXPlaySet* tracked = current->m_data;
                if (tracked->type != (unsigned long)-1)
                {
                    int trackedPriority = mpSFX[tracked->type].sfxPriority;
                    if (trackedPriority > 0 && trackedPriority < priority)
                    {
                        if (tracked->bIs3D)
                        {
                            if (tracked->emitter != NULL && Audio::IsEmitterActive(tracked->emitter))
                            {
                                tracked->voiceID = Audio::GetEmitterVoiceID(tracked->emitter);
                                hasHigherPriority = true;
                            }
                        }
                        else if (Audio::IsSFXPlaying(tracked->voiceID))
                        {
                            hasHigherPriority = true;
                        }

                        if (hasHigherPriority)
                        {
                            break;
                        }
                    }
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
        }

        if (hasHigherPriority)
        {
            return Audio::GetSndIDError();
        }

        if (mbCurPlaySetIsValid)
        {
            DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
            DLListEntry<SFXPlaySet*>* current = nlDLRingGetStart(head);
            while (current != NULL)
            {
                SFXPlaySet* tracked = current->m_data;

                if (nlDLRingIsEnd(head, current) || current == NULL)
                {
                    current = NULL;
                }
                else
                {
                    current = current->m_next;
                }

                if (tracked->type == (unsigned long)-1)
                {
                    continue;
                }

                TrackedSFXPriorityCallback(tracked, (unsigned long)priority, this);
            }
        }
    }

    attributes.mp_OwnerSFX = this;
    if (priority >= 5 && priority <= 19)
    {
        attributes.mu_Pitch = 0x2000;
    }
    else
    {
        attributes.mb_FilterOn = false;
        attributes.mu_FilterFreq = 0;
    }

    if (attributes.mf_DelayTime != -1.0f)
    {
        return Audio::AddDelayedSFX(attributes, sfxID, fVolume, fVolReverb, this);
    }

    if (!attributes.mb_Is3D)
    {
        voiceID = Audio::PlaySFXbyID(attributes, sfxID, fVolume, fVolReverb, priority);
        mpSFX[attributes.mu_Type].lastVoiceID = voiceID;
        return voiceID;
    }

    {
        const nlVector3 zeroVector = { 0.0f, 0.0f, 0.0f };
        vDir = zeroVector;
    }

    if (attributes.posUpdateMethod == PHYSOBJ)
    {
        attributes.mp_PhysObj->GetPosition(&vPos);
    }
    else if (attributes.posUpdateMethod == PTRS_TO_VECTORS)
    {
        vPos = *attributes.pos.pvPos;
    }
    else if (attributes.posUpdateMethod == VECTORS)
    {
        vPos = attributes.pos.vPos;
    }

    if (attributes.mb_Update3DContinuously)
    {
        pSFXEmitter = Audio::GetFreeEmitter(emitterIndex);
        if (pSFXEmitter == NULL)
        {
            return Audio::GetSndIDError();
        }

        if (attributes.posUpdateMethod == PHYSOBJ)
        {
            if (attributes.mp_PhysObj->m_geomID != NULL)
            {
                attributes.mp_PhysObj->GetLinearVelocity(&vDir);
            }
            pSFXEmitter->pPhysObj = attributes.mp_PhysObj;
        }
        else if (attributes.posUpdateMethod == PTRS_TO_VECTORS)
        {
            vDir = *attributes.dir.pvDir;
            pSFXEmitter->pos.pvPos = attributes.pos.pvPos;
            pSFXEmitter->dir.pvDir = attributes.dir.pvDir;
        }
        else if (attributes.posUpdateMethod == VECTORS)
        {
            vDir = attributes.dir.vDir;
            pSFXEmitter->pos.vPos = vPos;
            pSFXEmitter->dir.vDir = vDir;
        }

        pSFXEmitter->posUpdateMethod = attributes.posUpdateMethod;
        pSFXEmitter->soundType = attributes.mu_Type;
        pSFXEmitter->pOwner = this;
    }

    info.pSFXEmitter = NULL;
    info.uSFXID = (unsigned long)-1;
    info.groupID = 0;
    info.position.f.x = 0.0f;
    info.position.f.y = 0.0f;
    info.position.f.z = 0.0f;
    info.direction.f.x = 0.0f;
    info.direction.f.y = 0.0f;
    info.direction.f.z = 0.0f;
    info.maxDist = 100.0f;
    info.comp = 0.0f;
    info.minVol = 0.0f;
    info.maxVol = 1.0f;
    info.fVolReverb = 100.0f;
    info.bContinuous = true;
    info.bRestartable = false;
    info.bPausable = false;
    info.bUseDoppler = false;
    info.bHardStart = true;
    info.bActivateFilter = false;
    info.filterFreq = 0;
    info.pitch = 0x2000;

    if (attributes.mb_Update3DContinuously)
    {
        info.pSFXEmitter = pSFXEmitter;
    }

    info.uSFXID = sfxID;
    info.groupID = attributes.mi_EmitterGroup;
    info.position = vPos;
    info.direction = vDir;
    info.maxDist = g_pGame->m_pGameTweaks->unk200;
    info.comp = g_pGame->m_pGameTweaks->unk204;
    info.minVol = 0.0f;
    info.maxVol = fVolume;
    info.fVolReverb = fVolReverb;
    info.bContinuous = attributes.mb_Update3DContinuously ? 1 : 0;
    info.bHardStart = attributes.m_unk_0x7C;

    if (priority >= 5 && priority <= 19)
    {
        info.pitch = 0x2000;
    }
    else
    {
        info.bActivateFilter = false;
        info.filterFreq = 0;
    }

    typedef unsigned long (*Add3DEmitterFn)(const EmitterStartInfo&);
    voiceID = ((Add3DEmitterFn)Audio::Add3DSFXEmitter)(info);

    if (attributes.mb_Update3DContinuously)
    {
        pSFXEmitter->fTimeStamp = Audio::GetAudioTimer();
    }

    mpSFX[attributes.mu_Type].pLastEmitter = pSFXEmitter;

    if (attributes.mf_ReturnEmitterOnPlay)
    {
        voiceID = (unsigned long)pSFXEmitter;
    }

    if (attributes.mb_KeepTrack)
    {
        KeepTrack(pSFXEmitter, attributes, voiceID);
    }

    return voiceID;
}

/**
 * Offset/Address/Size: 0x1BD0 | 0x80153114 | size: 0xA8
 */
bool TrackedSFXPriorityCallback(SFXPlaySet* pSFXPlaySet, unsigned long param, cGameSFX* pGameSFX)
{
    if (pSFXPlaySet->bIs3D != 0)
    {
        if (Audio::IsEmitterActive(pSFXPlaySet->emitter) && pSFXPlaySet->sfxPriority > param)
        {
            return pGameSFX->StopTrackedSFX(pSFXPlaySet);
        }
    }
    else
    {
        if (Audio::IsSFXPlaying(pSFXPlaySet->voiceID) && pSFXPlaySet->sfxPriority > param)
        {
            return pGameSFX->StopTrackedSFX(pSFXPlaySet);
        }
    }
    return true;
}

/**
 * Offset/Address/Size: 0x1C78 | 0x801531BC | size: 0x154
 */
bool cGameSFX::SetPitchBendOnAllDialogueSFX(unsigned short pitch)
{
    bool result;

    if (pitch > 0x3FFF)
    {
        pitch = 0x3FFF;
    }

    if (!mbCurPlaySetIsValid)
    {
        result = true;
        goto epilogue;
    }

    {
        SFXPlaySet* pPlaySet;
        DLListEntry<SFXPlaySet*>* start = nlDLRingGetStart(mpCurPlaySet.m_Head);
        DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
        DLListEntry<SFXPlaySet*>* current = start;
        unsigned short clampedPitch = (unsigned short)pitch;

        while (current != NULL)
        {
            pPlaySet = current->m_data;

            if (nlDLRingIsEnd(head, current) || current == NULL)
            {
                current = NULL;
            }
            else
            {
                current = current->m_next;
            }

            if (pPlaySet->type == (unsigned long)-1)
            {
                continue;
            }

            if (TrackedSFXPitchFreqTypeCheckCallback != NULL)
            {
                if (!TrackedSFXPitchFreqTypeCheckCallback(pPlaySet->type, this))
                {
                    continue;
                }
            }

            if (pPlaySet->bIs3D)
            {
                if (!Audio::IsEmitterActive(pPlaySet->emitter))
                {
                    continue;
                }
                pPlaySet->voiceID = Audio::GetEmitterVoiceID(pPlaySet->emitter);
            }

            if (pPlaySet->voiceID == Audio::GetSndIDError())
            {
                continue;
            }

            if (!Audio::IsSFXPlaying(pPlaySet->voiceID))
            {
                continue;
            }

            if (pPlaySet->pitch == clampedPitch)
            {
                continue;
            }

            pPlaySet->pitch = clampedPitch;
            Audio::SetPitchBendOnSFX(pPlaySet->voiceID, clampedPitch);
        }
    }

    result = true;

epilogue:
    muGroupPitch = pitch;
    return result;
}

/**
 * Offset/Address/Size: 0x1DCC | 0x80153310 | size: 0x194
 */
bool cGameSFX::SetFilterFreqOnAllTrackedSFX(unsigned short freq)
{
    bool result;

    if (freq > 0x3FFF)
    {
        freq = 0x3FFF;
    }

    if (!mbCurPlaySetIsValid)
    {
        result = true;
        goto epilogue;
    }

    {
        SFXPlaySet* pPlaySet;
        DLListEntry<SFXPlaySet*>* start = nlDLRingGetStart(mpCurPlaySet.m_Head);
        DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
        DLListEntry<SFXPlaySet*>* current = start;
        unsigned short clampedFreq = (unsigned short)freq;
        unsigned long sfxType;

        while (current != NULL)
        {
            pPlaySet = current->m_data;

            if (nlDLRingIsEnd(head, current) || current == NULL)
            {
                current = NULL;
            }
            else
            {
                current = current->m_next;
            }

            sfxType = pPlaySet->type;
            if (sfxType == (unsigned long)-1)
            {
                continue;
            }

            if (TrackedSFXFilterFreqTypeCheckCallback != NULL)
            {
                bool shouldApply;
                eClassType classType = GetClassType();
                if (classType == WORLD && sfxType == 0xBB)
                {
                    shouldApply = false;
                }
                else
                {
                    bool isInRange = IsVolGrpInRange(sfxType, this);
                    if (isInRange)
                    {
                        shouldApply = false;
                    }
                    else
                    {
                        shouldApply = true;
                    }
                }
                if (!shouldApply)
                {
                    continue;
                }
            }

            if (pPlaySet->bIs3D)
            {
                if (!Audio::IsEmitterActive(pPlaySet->emitter))
                {
                    continue;
                }
                pPlaySet->voiceID = Audio::GetEmitterVoiceID(pPlaySet->emitter);
            }

            if (pPlaySet->voiceID == Audio::GetSndIDError())
            {
                continue;
            }

            if (!Audio::IsSFXPlaying(pPlaySet->voiceID))
            {
                continue;
            }

            if (pPlaySet->filterFreq == clampedFreq)
            {
                continue;
            }

            pPlaySet->filterFreq = clampedFreq;
            Audio::SetFilterFreqOnSFX(pPlaySet->voiceID, clampedFreq);
        }
    }

    result = true;

epilogue:
    muGroupFilterFreq = freq;
    return result;
}

/**
 * Offset/Address/Size: 0x1F60 | 0x801534A4 | size: 0x12C
 */
bool cGameSFX::ActivateFilterOnAllTrackedSFX(bool bActivate)
{
    bool bParam = bActivate;
    u8 bActivateU8 = (u8)bActivate;
    u32 bNeg = (u32)(-(s32)bActivateU8);
    u32 bOr = bNeg | bActivateU8;
    u32 bNorm = bOr >> 31;
    bool result;

    if (!mbCurPlaySetIsValid)
    {
        result = true;
        goto epilogue;
    }

    {
        SFXPlaySet* pPlaySet;
        bool filterActive;
        DLListEntry<SFXPlaySet*>* start = nlDLRingGetStart(mpCurPlaySet.m_Head);
        DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
        DLListEntry<SFXPlaySet*>* current = start;

        while (current != NULL)
        {
            pPlaySet = current->m_data;

            if (nlDLRingIsEnd(head, current) || current == NULL)
            {
                current = NULL;
            }
            else
            {
                current = current->m_next;
            }

            if (pPlaySet->type == (unsigned long)-1)
            {
                continue;
            }

            filterActive = false;
            if (bNorm == 1)
            {
                filterActive = true;
            }

            if (pPlaySet->bIs3D)
            {
                if (!Audio::IsEmitterActive(pPlaySet->emitter))
                {
                    continue;
                }
                pPlaySet->voiceID = Audio::GetEmitterVoiceID(pPlaySet->emitter);
            }

            unsigned long sndIDError = Audio::GetSndIDError();
            if (pPlaySet->voiceID != sndIDError)
            {
                if (Audio::IsSFXPlaying(pPlaySet->voiceID))
                {
                    Audio::ActivateFilterOnSFX(pPlaySet->voiceID, filterActive);
                }
            }
        }
    }

    result = true;

epilogue:
    if (bParam)
    {
        mbGroupFilterOn = true;
    }
    else
    {
        mbGroupFilterOn = false;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x208C | 0x801535D0 | size: 0x9C
 */
bool TrackedSFXFilterFreqTypeCheckCallback(unsigned long sfxID, cGameSFX* pGameSFX)
{
    if (pGameSFX->GetClassType() == WORLD && sfxID == 0xBB)
    {
        return false;
    }

    if (IsVolGrpInRange(sfxID, pGameSFX))
    {
        return false;
    }
    return true;
}

/**
 * Offset/Address/Size: 0x2158 | 0x8015369C | size: 0x150
 */
bool cGameSFX::IsKeepingTrackOf(unsigned long type, SFXPlaySet** pGrabTrackedSFX)
{
    if (pGrabTrackedSFX != NULL)
    {
        *pGrabTrackedSFX = NULL;
    }

    if (!mbCurPlaySetIsValid)
    {
        return false;
    }

    DLListEntry<SFXPlaySet*>* head = mpCurPlaySet.m_Head;
    DLListEntry<SFXPlaySet*>* current = nlDLRingGetStart(head);
    head = mpCurPlaySet.m_Head;

    while (current != NULL)
    {
        SFXPlaySet* pTrackedSFX = current->m_data;

        if (pTrackedSFX->type == type)
        {
            if (pTrackedSFX->bIs3D && pTrackedSFX->emitter != NULL && Audio::IsEmitterActive(pTrackedSFX->emitter))
            {
                pTrackedSFX->voiceID = Audio::GetEmitterVoiceID(pTrackedSFX->emitter);
            }

            if ((pTrackedSFX->delay < 0.0f && pTrackedSFX->bIs3D && pTrackedSFX->voiceID == (unsigned long)Audio::GetSndIDError()) || Audio::IsSFXPlaying(pTrackedSFX->voiceID))
            {
                if (pGrabTrackedSFX != NULL)
                {
                    *pGrabTrackedSFX = pTrackedSFX;
                }
                return true;
            }

            if (pTrackedSFX->delay >= 0.0f)
            {
                if (pGrabTrackedSFX != NULL)
                {
                    *pGrabTrackedSFX = pTrackedSFX;
                }
                return true;
            }
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

    return false;
}

/**
 * Offset/Address/Size: 0x22A8 | 0x801537EC | size: 0x14
 */
float cGameSFX::GetSFXVolReverb(unsigned long index) const
{
    return mpSFX[index].fVolReverb;
}

/**
 * Offset/Address/Size: 0x22BC | 0x80153800 | size: 0x14
 */
float cGameSFX::GetSFXVol(unsigned long index) const
{
    return mpSFX[index].fVolume;
}

/**
 * Offset/Address/Size: 0x22D0 | 0x80153814 | size: 0x218
 */
void cGameSFX::SetSFX(SoundPropAccessor* pSoundPropAccessor)
{
    if (!mbInited)
    {
        nlPrintf("cGameSFX::SetSFX - not initialized\n");
    }

    if (mNumSFX == 0)
    {
        mNumSFX = pSoundPropAccessor->GetNumSFX();
    }
    else
    {
        mNumSFX += pSoundPropAccessor->GetNumSFX();
    }

    for (unsigned long i = 1; i < mNumSFXTypes; i++)
    {
        for (int j = 0; j < pSoundPropAccessor->GetNumSFX(); j++)
        {
            const SoundProperties* pProp = pSoundPropAccessor->GetSoundProperty(j);

            if (nlStrCmp(mpSoundStrTable[i], pProp->typeStr) == 0)
            {
                eClassType classType = GetClassType();

                if (classType == WORLD)
                {
                    AudioLoader::GetWorldSFXTypeFromStr(pProp->typeStr);
                }
                else if (classType == CHAR)
                {
                    AudioLoader::GetCharSFXTypeFromStr(pProp->typeStr);
                }

                SoundStrToIDNode* pNode = NULL;
                unsigned long musyxID = AudioLoader::GetSFXIDFromStr(pProp->musyxStr, &pNode);

                if (classType == CHAR)
                {
                    mpSFX[i].typeID = i;
                    mpSFX[i].typeStr = pProp->typeStr;
                }

                mpSFX[i].musyxStr = pProp->musyxStr;
                mpSFX[i].musyxID = musyxID;
                mpSFX[i].fVolume = pProp->fVolume;
                mpSFX[i].fDelay = pProp->fDelay;
                mpSFX[i].fVolReverb = pProp->fVolReverb;
                mpSFX[i].volGrp = pProp->volumeGroup;
                mpSFX[i].sfxPriority = pProp->priority;
                mpSFX[i].pSoundPropAccessor = pSoundPropAccessor;
                mpSFX[i].pSoundProp = pProp;
                mpSFX[i].pOwner = this;

                break;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x24E8 | 0x80153A2C | size: 0x6C
 */
void cGameSFX::ShutdownPlaySet()
{
    FORCE_DONT_INLINE;
    mbCurPlaySetIsValid = false;
    StopPlayingAllTrackedSFX();

    nlWalkDLRing<DLListEntry<SFXPlaySet*>, DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*> > > >(
        mpCurPlaySet.m_Head, &mpCurPlaySet, &DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*> > >::DeleteEntry);

    mpCurPlaySet.m_Head = NULL;
}

/**
 * Offset/Address/Size: 0x2554 | 0x80153A98 | size: 0x4C
 */
void cGameSFX::DeInit()
{
    ShutdownPlaySet();
    mpSFX = NULL;
    mpSoundStrTable = 0;
    mbGroupFilterOn = false;
    muGroupFilterFreq = 0;
    muGroupPitch = 0x2000;
    mbInited = false;
}

/**
 * Offset/Address/Size: 0x25A0 | 0x80153AE4 | size: 0x10
 */
void cGameSFX::Init()
{
    mbInited = true;
    mbCurPlaySetIsValid = true;
}

/**
 * Offset/Address/Size: 0x25B0 | 0x80153AF4 | size: 0xB0
 */
cGameSFX::~cGameSFX()
{
    DeInit();
}

/**
 * Offset/Address/Size: 0x2660 | 0x80153BA4 | size: 0x4C
 */
cGameSFX::cGameSFX()
{
    mbInited = false;
    mNumSFX = 0;
    mNumSFXTypes = 0;
    mpSFX = NULL;
    mpCurPlaySet.m_Head = NULL;
    mbCurPlaySetIsValid = false;
    mfTrackedSFXCheckInterval = 0.0f;
    mpSoundStrTable = NULL;
    meClassType = GAME;
    mbGroupFilterOn = false;
    muGroupFilterFreq = 0;
    muGroupPitch = 0x2000;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80153BF0 | size: 0x24
//  */
// void DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*>>>::DeleteEntry(DLListEntry<SFXPlaySet*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80153C14 | size: 0x3C
 * TODO: 96.00% match - prologue scheduling mismatch remains.
 * Target orders `lwz r7, 0(r5)` before `stw r0, 0x24(r1)`.
 */
template <>
void nlWalkDLRing<DLListEntry<SFXPlaySet*>, DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*> > > >(
    DLListEntry<SFXPlaySet*>* head,
    DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*> > >* callback,
    void (DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*> > >::*callbackFunc)(DLListEntry<SFXPlaySet*>*))
{
    FORCE_DONT_INLINE;
    void (DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*> > >::*func)(DLListEntry<SFXPlaySet*>*) = callbackFunc;
    nlWalkRing(head, callback, func);
}

// /**
//  * Offset/Address/Size: 0x3C | 0x80153C50 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>*, DLListEntry<SFXPlaySet*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x80153C70 | size: 0x18
//  */
// void nlDLRingGetStart<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x74 | 0x80153C88 | size: 0x44
//  */
// void nlDLRingRemove<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>**, DLListEntry<SFXPlaySet*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB8 | 0x80153CCC | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>**, DLListEntry<SFXPlaySet*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80153D04 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<SFXPlaySet*>, DLListContainerBase<SFXPlaySet*,
// NewAdapter<DLListEntry<SFXPlaySet*>>>>(DLListEntry<SFXPlaySet*>*, DLListContainerBase<SFXPlaySet*,
// NewAdapter<DLListEntry<SFXPlaySet*>>>*, void (DLListContainerBase<SFXPlaySet*,
// NewAdapter<DLListEntry<SFXPlaySet*>>>::*)(DLListEntry<SFXPlaySet*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x80153DCC | size: 0x64
//  */
// void SlotPool<SFXPlaySet>::~SlotPool()
// {
// }
