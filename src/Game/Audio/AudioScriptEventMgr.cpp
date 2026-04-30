#include "Game/Audio/AudioScriptEventMgr.h"
#include "Game/Audio/SoundEventScript.h"
#include "Game/Game.h"
#include "Game/Sys/eventman.h"

#include "NL/nlBSearch.h"
#include "NL/nlList.h"
#include "NL/nlQSort.h"
#include "NL/nlSlotPool.h"
#include "NL/nlString.h"

class cTeam;

struct AUDIO_SCRIPT_POLL_STATE
{
    /* 0x00 */ float NextPossibleGoodPositionTime;
    /* 0x04 */ cTeam* pLastBallOwnerTeam;
    /* 0x08 */ float NoOwnerTime;
    /* 0x0C */ unsigned long GameTimeToggle;
    /* 0x10 */ float LastExcitementTime;
    /* 0x14 */ unsigned char AmBored : 1;
    /* 0x18 */ cTeam* LastOwningTeam;
};

extern AUDIO_SCRIPT_POLL_STATE g_ScriptPollState;

struct AUDIO_EVENT_RECORD
{
    /* 0x0 */ AudioScriptEventMgr::AUDIO_EVENT Event : 16;
    /* 0x2 */ AudioScriptEventMgr::AUDIO_EVENT_TEAM Team : 16;
};

struct NIS_EVENT_LOOKUP
{
    /* 0x0 */ unsigned long hash;
    /* 0x4 */ const char* Name;
    /* 0x8 */ AUDIO_EVENT_RECORD Event;
};

extern NIS_EVENT_LOOKUP g_NisEventLookup[4];

extern char* AUDIO_EVENT_FUNC_NAMES[];

typedef ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > > AudioEventList;

template class ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >;

extern AudioEventList g_PendingEvents;
extern EventHandler* g_pAudioEventHandler;
extern _AudioEventRaiser g_AudioEventRaiser;

void Poll();

template <typename KeyType, typename EntryType, typename CallbackType>
class WalkHelper
{
public:
    CallbackType* m_CBClass;
    void (CallbackType::*m_CB)(KeyType*);
    void Callback(EntryType*);
};

typedef WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser> AudioEventWalkHelper;

// /**
//  * Offset/Address/Size: 0xD0 | 0x8014B7EC | size: 0x2C
//  */
// void nlListAddEnd<ListEntry<AUDIO_EVENT_RECORD> >(ListEntry<AUDIO_EVENT_RECORD>**, ListEntry<AUDIO_EVENT_RECORD>**, ListEntry<AUDIO_EVENT_RECORD>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x8014B784 | size: 0x68
//  */
// void nlWalkList<ListEntry<AUDIO_EVENT_RECORD>, ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > > >(ListEntry<AUDIO_EVENT_RECORD>*, ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >*, void (ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >::*)(ListEntry<AUDIO_EVENT_RECORD>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B71C | size: 0x68
//  */
// void nlWalkList<ListEntry<AUDIO_EVENT_RECORD>, WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser> >(ListEntry<AUDIO_EVENT_RECORD>*, WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>*, void (WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>::*)(ListEntry<AUDIO_EVENT_RECORD>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B65C | size: 0xC0
//  */
// void 0x8014B71C..0x8014B818 | size : 0xFC
// {
// }

// /**
//  * Offset/Address/Size: 0xE0 | 0x8014B590 | size: 0xCC
//  */
// void nlListSlotPool<AUDIO_EVENT_RECORD>::~nlListSlotPool()
// {
// }

// /**
//  * Offset/Address/Size: 0xB4 | 0x8014B564 | size: 0x2C
//  */
// void nlDefaultQSortComparer<NIS_EVENT_LOOKUP>(const NIS_EVENT_LOOKUP*, const NIS_EVENT_LOOKUP*)
// {
// }

/**
 * Offset/Address/Size: 0x8C | 0x8014B53C | size: 0x28
 */
// void nlQSort<NIS_EVENT_LOOKUP>(NIS_EVENT_LOOKUP*, int, int (*)(const NIS_EVENT_LOOKUP*, const NIS_EVENT_LOOKUP*))
// instantiated via AudioScriptEventMgr_stub below

/**
 * Offset/Address/Size: 0x0 | 0x8014B4B0 | size: 0x8C
 */
// nlBSearch<NIS_EVENT_LOOKUP, unsigned long> instantiated via AudioScriptEventMgr_stub below

// /**
//  * Offset/Address/Size: 0x0 | 0x8014B4A0 | size: 0x10
//  */
// void ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >::DeleteEntry(ListEntry<AUDIO_EVENT_RECORD>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8014B46C | size: 0x34
 */
void WalkHelper<AUDIO_EVENT_RECORD, ListEntry<AUDIO_EVENT_RECORD>, _AudioEventRaiser>::Callback(ListEntry<AUDIO_EVENT_RECORD>* listEntry)
{
    (m_CBClass->*m_CB)(&listEntry->data);
}

// /**
//  * Offset/Address/Size: 0x2248 | 0x8014B39C | size: 0xD0
//  */
// void GetEventTeam<GoalScoredData>(Event*, bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x2178 | 0x8014B2CC | size: 0xD0
//  */
// void GetEventTeam<CollisionBallGoalpostData>(Event*, bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x1BAC | 0x8014AD00 | size: 0x5CC
//  */
// void AudioScriptEventMgr::Init()
// {
// }

/**
 * Offset/Address/Size: 0x1B18 | 0x8014AC6C | size: 0x94
 */
void AudioScriptEventMgr::Purge()
{
    nlWalkList(g_PendingEvents.m_Head, &g_PendingEvents, &AudioEventList::DeleteEntry);
    g_PendingEvents.m_Head = NULL;
    g_PendingEvents.m_Tail = NULL;
    SlotPoolBase::BaseFreeBlocks(&g_PendingEvents.m_Allocator, sizeof(ListEntry<AUDIO_EVENT_RECORD>));

    if (g_pAudioEventHandler != NULL)
    {
        g_pEventManager->RemoveEventHandler(g_pAudioEventHandler);
    }
    g_pAudioEventHandler = NULL;
}

/**
 * Offset/Address/Size: 0x1A48 | 0x8014AB9C | size: 0xD0
 * TODO: 97.96% match - WalkHelper and Callback method pointer stack offsets swapped
 */
void AudioScriptEventMgr::Update()
{
    Poll();

    AudioEventWalkHelper helper;
    helper.m_CBClass = &g_AudioEventRaiser;
    helper.m_CB = &_AudioEventRaiser::RaiseEvent;

    nlWalkList(g_PendingEvents.m_Head, &helper, &AudioEventWalkHelper::Callback);
    nlWalkList(g_PendingEvents.m_Head, &g_PendingEvents, &AudioEventList::DeleteEntry);
    g_PendingEvents.m_Head = NULL;
    g_PendingEvents.m_Tail = NULL;
}

/**
 * Offset/Address/Size: 0x19B8 | 0x8014AB0C | size: 0x90
 */
void _AudioEventRaiser::RaiseEvent(AUDIO_EVENT_RECORD* pEvent)
{
    char FuncName[64];
    memcpy(FuncName, "Crowd", 5);
    nlStrNCpy(FuncName + 5, AUDIO_EVENT_FUNC_NAMES[pEvent->Event], 0x3b);
    if (pEvent->Team != 0)
    {
        const char* suffix = "Away";
        if (pEvent->Team == 1)
        {
            suffix = "Home";
        }
        nlStrNCat(FuncName, FuncName, suffix, 0x40);
    }
    SoundEventScript::Instance().Call(FuncName);
}

/**
 * Offset/Address/Size: 0x1904 | 0x8014AA58 | size: 0xB4
 */
void AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AUDIO_EVENT Event, AudioScriptEventMgr::AUDIO_EVENT_TEAM Team)
{
    AUDIO_EVENT_RECORD aer = { Event, Team };
    union
    {
        AUDIO_EVENT_RECORD val;
        u64 _align;
    } temp;
    temp.val = aer;

    ListEntry<AUDIO_EVENT_RECORD>* entry = NULL;
    g_PendingEvents.m_Allocator.Allocate(entry);

    if (entry != NULL)
    {
        entry->next = NULL;
        entry->data = temp.val;
    }

    nlListAddEnd(&g_PendingEvents.m_Head, &g_PendingEvents.m_Tail, entry);
}

// /**
//  * Offset/Address/Size: 0x10F8 | 0x8014A24C | size: 0x80C
//  */
// void Poll()
// {
// }

/**
 * Offset/Address/Size: 0xFF8 | 0x8014A14C | size: 0x100
 */
void RecordExcitingEvent()
{
    g_ScriptPollState.LastExcitementTime = g_pGame->GetGameTime();

    if (g_ScriptPollState.AmBored)
    {
        nlPrintf("END bored\n");
        g_ScriptPollState.AmBored = 0;
        AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AE_BoredEnd, AudioScriptEventMgr::AET_Neutral);
    }
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80149154 | size: 0xFF8
//  */
// void AudioScriptEventHandler(Event*, void*)
// {
// }

// REMOVE once real callers exist.
void AudioScriptEventMgr_stub()
{
    unsigned long k = 0;
    nlBSearch<NIS_EVENT_LOOKUP, unsigned long>(k, g_NisEventLookup, 4);
    nlQSort<NIS_EVENT_LOOKUP>(g_NisEventLookup, 4, &nlDefaultQSortComparer<NIS_EVENT_LOOKUP>);
}
