#include "Game/Audio/AudioScriptEventMgr.h"
#include "Game/Audio/SoundEventScript.h"
#include "Game/Game.h"
#include "Game/Goalie.h"
#include "Game/Physics/PhysicsNet.h"
#include "Game/Sys/eventman.h"

#include "NL/nlBSearch.h"
#include "NL/nlConfig.h"
#include "NL/nlList.h"
#include "NL/nlListSlotPool.h"
#include "NL/nlQSort.h"
#include "NL/nlSlotPool.h"
#include "NL/nlString.h"

class cTeam
{
public:
    /* 0x00 */ int m_nSide;
    /* 0x04 */ cPlayer* m_pPlayers[5];
    /* 0x18 */ cFielder* m_pAIOrderedFielders[4];
    /* 0x28 */ cFielder* m_pBallInterceptOrderedFielders[4];
    /* 0x38 */ void* m_pNet;
    /* 0x3C */ int m_nScore;
};

enum eVariantType
{
    FT_UNSPECIFIED = -1,
    FT_BOOL = 0,
    FT_CHAR = 1,
    FT_SHORT = 2,
    FT_INT = 3,
    FT_U32 = 4,
    FT_FLOAT = 5,
    FT_PLAYER = 6,
    FT_TEAM = 7,
    FT_VECTOR = 8,
    NUM_V_TYPES = 9,
};

class Variant
{
public:
    virtual void Reset()
    {
        mType = FT_UNSPECIFIED;
        mData.vector.f.x = 0.0f;
        mData.vector.f.y = 0.0f;
        mData.vector.f.z = 0.0f;
    }

    /* 0x04 */ eVariantType mType;
    /* 0x08 */ union
    {
        bool b;
        char c;
        signed short s;
        float f;
        int i;
        unsigned long u;
        cPlayer* pPlayer;
        cTeam* pTeam;
        nlVector3 vector;
    } mData;
};

class FuzzyVariant : public Variant
{
public:
    virtual void Reset()
    {
        mType = FT_UNSPECIFIED;
        mData.vector.f.x = 0.0f;
        mData.vector.f.y = 0.0f;
        mData.vector.f.z = 0.0f;
        ExtraData.Reset();
        Confidence = 0.0f;
        SelectionChance = 1.0f;
    }

    /* 0x14 */ float Confidence;
    /* 0x18 */ float SelectionChance;
    /* 0x1C */ Variant ExtraData;
};

class Fuzzy
{
public:
    static FuzzyVariant GoodToShoot(cFielder*);
};

extern cTeam* g_pTeams[];
float OnBreakaway(cFielder*);

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

struct AUDIO_SCRIPT_SETTINGS
{
    /* 0x00 */ unsigned long HalfTime;
    /* 0x04 */ unsigned long LastPeriod;
    /* 0x08 */ unsigned long FinalSeconds;
    /* 0x0C */ float GoodToShootThreshold;
    /* 0x10 */ float OnBreakawayThreshold;
    /* 0x14 */ unsigned long MinGoodPositionPeriod;
    /* 0x18 */ float TimeToBored;
    /* 0x1C */ float BoredPeriod;
    /* 0x20 */ unsigned long MaxFreeBallTime;
};

static AUDIO_SCRIPT_SETTINGS g_ScriptSettings;

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

    operator unsigned long() const { return hash; }
};

extern NIS_EVENT_LOOKUP g_NisEventLookup[4];

extern char* AUDIO_EVENT_FUNC_NAMES[];

typedef ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > > AudioEventList;

template class ListContainerBase<AUDIO_EVENT_RECORD, BasicSlotPool<ListEntry<AUDIO_EVENT_RECORD> > >;

extern AudioEventList g_PendingEvents;
extern EventHandler* g_pAudioEventHandler;
extern _AudioEventRaiser g_AudioEventRaiser;

void AudioScriptEventHandler(Event*, void*);
static void Poll();

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

/**
 * Offset/Address/Size: 0x2248 | 0x8014B39C | size: 0xD0
 */
template <typename T>
static AudioScriptEventMgr::AUDIO_EVENT_TEAM GetEventTeam(Event* pEvent, bool Invert)
{
    T* pEventData;
    if ((s32)pEvent->m_data.GetID() == -1)
    {
        nlPrintf("Error: Trying to get event data on event with none!\n");
        pEventData = NULL;
    }
    else
    {
        if ((s32)pEvent->m_data.GetID() != (s32)T::ID)
        {
            nlPrintf("Error: GetData() failed! Data types do not match!\n");
            pEventData = NULL;
        }
        else
        {
            pEventData = (T*)&pEvent->m_data;
        }
    }

    AudioScriptEventMgr::AUDIO_EVENT_TEAM team = AudioScriptEventMgr::AET_Home;
    if (pEventData->uTeamIndex != 0)
    {
        team = AudioScriptEventMgr::AET_Away;
    }

    s32 invertMask = Invert ? (s32)AudioScriptEventMgr::AET_Special : 0;
    return (AudioScriptEventMgr::AUDIO_EVENT_TEAM)((s32)team ^ invertMask);
}

template AudioScriptEventMgr::AUDIO_EVENT_TEAM GetEventTeam<GoalScoredData>(Event*, bool);

/**
 * Offset/Address/Size: 0x2178 | 0x8014B2CC | size: 0xD0
 */
template AudioScriptEventMgr::AUDIO_EVENT_TEAM GetEventTeam<CollisionBallGoalpostData>(Event*, bool);

/**
 * Offset/Address/Size: 0x1BAC | 0x8014AD00 | size: 0x5CC
 */
void AudioScriptEventMgr::Init()
{
    g_pAudioEventHandler = g_pEventManager->AddEventHandler(AudioScriptEventHandler, NULL, (unsigned long)-1);

    AUDIO_SCRIPT_SETTINGS& settings = g_ScriptSettings;

    settings.HalfTime = GetConfigInt(Config::Global(), "HalfTime", 50);
    settings.LastPeriod = GetConfigInt(Config::Global(), "LastPeriod", 75);
    settings.FinalSeconds = GetConfigInt(Config::Global(), "FinalSeconds", 93);
    settings.GoodToShootThreshold = GetConfigFloat(Config::Global(), "GoodToShoot", 0.5f);
    settings.OnBreakawayThreshold = GetConfigFloat(Config::Global(), "OnBreakaway", 0.5f);
    settings.MinGoodPositionPeriod = (unsigned long)GetConfigFloat(Config::Global(), "MinGoodPositionPeriod", 5.0f);
    settings.TimeToBored = GetConfigFloat(Config::Global(), "TimeToBored", 12.0f);
    settings.BoredPeriod = GetConfigFloat(Config::Global(), "BoredPeriod", 12.0f);
    settings.MaxFreeBallTime = (unsigned long)(GetConfigFloat(Config::Global(), "MaxFreeBallTime", 1000.0f) / 1000.0f);

    memset(&g_ScriptPollState, 0, sizeof(AUDIO_SCRIPT_POLL_STATE));
}

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

/**
 * Offset/Address/Size: 0x10F8 | 0x8014A24C | size: 0x80C
 * TODO: 95.18% match - two literal symbol references differ (@2283 and @1485)
 */
static void Poll()
{
    GameTweaks* pGameTweaks = g_pGame->m_pGameTweaks;
    float gameTime = g_pGame->GetGameTime();
    unsigned long gameTimePercentage = (unsigned long)((100.0f * gameTime) / pGameTweaks->fGameDuration);
    AudioScriptEventMgr::AUDIO_EVENT event = (AudioScriptEventMgr::AUDIO_EVENT)-1;
    AudioScriptEventMgr::AUDIO_EVENT_TEAM team = AudioScriptEventMgr::AET_Neutral;

    if (gameTimePercentage == g_ScriptSettings.HalfTime)
    {
        event = AudioScriptEventMgr::AE_Halftime;
        g_ScriptPollState.GameTimeToggle++;
    }
    else if (gameTimePercentage == g_ScriptSettings.LastPeriod)
    {
        event = AudioScriptEventMgr::AE_LastPeriod;
        g_ScriptPollState.GameTimeToggle++;
    }
    else if (gameTimePercentage == g_ScriptSettings.FinalSeconds)
    {
        cTeam* pHome = g_pTeams[0];
        cTeam* pAway = g_pTeams[1];
        int awayScore = pAway->m_nScore;
        int homeScore = pHome->m_nScore;

        if (homeScore == awayScore)
        {
            event = AudioScriptEventMgr::AE_FinalSecondsTie;
        }
        else
        {
            event = AudioScriptEventMgr::AE_FinalSeconds;
            int winnerTeam = 2;
            if (homeScore > awayScore)
            {
                winnerTeam = 1;
            }
            team = (AudioScriptEventMgr::AUDIO_EVENT_TEAM)winnerTeam;
        }

        g_ScriptPollState.GameTimeToggle++;
    }
    else if (g_pGame->m_eGameState != GS_OVERTIME && g_pGame->m_eGameState == GS_END_GAME)
    {
        if (++g_ScriptPollState.GameTimeToggle == 1)
        {
            g_pEventManager->CreateValidEvent(4, 0x14);
        }
    }
    else
    {
        g_ScriptPollState.GameTimeToggle = 0;
    }

    if (g_ScriptPollState.GameTimeToggle == 1 && event != (AudioScriptEventMgr::AUDIO_EVENT)-1)
    {
        AudioScriptEventMgr::FireEvent(event, team);
    }

    cFielder* pBallOwner = g_pBall->GetOwnerFielder();
    if (pBallOwner != NULL)
    {
        cTeam* pOwnerTeam = ((cPlayer*)pBallOwner)->m_pTeam;

        if (pOwnerTeam != g_ScriptPollState.pLastBallOwnerTeam)
        {
            g_ScriptPollState.pLastBallOwnerTeam = pOwnerTeam;
            g_ScriptPollState.NextPossibleGoodPositionTime = g_pGame->GetGameTime() - 0.0001f;
        }

        if (Fuzzy::GoodToShoot(pBallOwner).mData.f >= g_ScriptSettings.GoodToShootThreshold || OnBreakaway(pBallOwner) >= g_ScriptSettings.OnBreakawayThreshold)
        {
            if (g_ScriptPollState.NextPossibleGoodPositionTime <= g_pGame->GetGameTime())
            {
                g_ScriptPollState.NextPossibleGoodPositionTime = g_pGame->GetGameTime() + (float)g_ScriptSettings.MinGoodPositionPeriod;
                AudioScriptEventMgr::AUDIO_EVENT_TEAM eventTeam = AudioScriptEventMgr::AET_Home;

                if (pOwnerTeam->m_nSide != 0)
                {
                    eventTeam = AudioScriptEventMgr::AET_Away;
                }

                AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AE_GoodPosition, eventTeam);
                g_ScriptPollState.LastExcitementTime = g_pGame->GetGameTime();

                if (g_ScriptPollState.AmBored)
                {
                    nlPrintf("END bored\n");
                    g_ScriptPollState.AmBored = 0;
                    AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AE_BoredEnd, AudioScriptEventMgr::AET_Neutral);
                }
            }
        }
    }

    if (g_pBall->m_pOwner != NULL)
    {
        cTeam* pBallTeam = g_pBall->m_pOwner->m_pTeam;
        if (pBallTeam != g_ScriptPollState.LastOwningTeam)
        {
            g_ScriptPollState.LastExcitementTime = g_pGame->GetGameTime();

            if (g_ScriptPollState.AmBored)
            {
                nlPrintf("END bored\n");
                g_ScriptPollState.AmBored = 0;
                AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AE_BoredEnd, AudioScriptEventMgr::AET_Neutral);
            }

            g_ScriptPollState.LastOwningTeam = pBallTeam;

            if (g_pGame->m_eGameState > GS_END_GAME)
            {
                AudioScriptEventMgr::AUDIO_EVENT_TEAM eventTeam = AudioScriptEventMgr::AET_Home;
                if (pBallTeam->m_nSide != 0)
                {
                    eventTeam = AudioScriptEventMgr::AET_Away;
                }
                AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AE_GotPossession, eventTeam);
            }

            g_ScriptPollState.NoOwnerTime = -1.0f;
        }
    }
    else if (g_ScriptPollState.NoOwnerTime < 0.0f)
    {
        g_ScriptPollState.NoOwnerTime = g_pGame->GetGameTime();
    }

    float timeSinceExcitement = g_pGame->GetGameTime() - g_ScriptPollState.LastExcitementTime;
    if (timeSinceExcitement > g_ScriptSettings.TimeToBored)
    {
        if (!g_ScriptPollState.AmBored)
        {
            g_ScriptPollState.AmBored = 1;
            g_ScriptPollState.LastExcitementTime = g_pGame->GetGameTime();
            nlPrintf("START bored %.2f\n", timeSinceExcitement);
            AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AE_BoredStart, AudioScriptEventMgr::AET_Neutral);
        }
    }
    else if (timeSinceExcitement > g_ScriptSettings.BoredPeriod)
    {
        if (g_ScriptPollState.AmBored)
        {
            g_ScriptPollState.LastExcitementTime = g_pGame->GetGameTime();
            nlPrintf("PERIOD bored %.2f\n", timeSinceExcitement);
            AudioScriptEventMgr::FireEvent(AudioScriptEventMgr::AE_BoredPeriod, AudioScriptEventMgr::AET_Neutral);
        }
    }
}

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

    static nlListSlotPool<AUDIO_EVENT_RECORD> s_inst;
}
