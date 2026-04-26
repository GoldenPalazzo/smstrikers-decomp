#include "Game/Audio/AudioStream.h"
#include "Game/Sys/PlatStream.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/PriorityStream.h"
#include "NL/nlMemory.h"
#include "NL/nlConfig.h"
#include "NL/nlString.h"
#include "NL/nlPrint.h"
#include "NL/nlMath.h"
#include "Game/GameInfo.h"
#include "Game/Team.h"
#include <stdlib.h>

#include "Game/FE/feHelpFuncs.h"

extern cTeam* g_pTeams[2];
extern unsigned int nlDefaultSeed;

// namespace AudioStreamTrack
// {

// class TrackManagerBase
// {
// public:
//     virtual ~TrackManagerBase();
//     virtual void StopAllTracks(unsigned long);

//     // StreamFileLookup m_FileLookup;       // offset 0x4, size 0x14
//     // FadeManager m_FadeMgr;               // offset 0x18, size 0x20
//     // SlotPool m_StreamPool;               // offset 0x38, size 0x18
//     // nlDLListSlotPool m_StreamDeleteList; // offset 0x50, size 0x1C
// };

// } // namespace AudioStreamTrack

extern AudioStreamTrack::TrackManagerBase* g_pTrackManager;
extern PriorityStream* g_pPriorityStream;

// Structure for last scorer info
struct LastScorerInfo
{
    /* 0x00 */ void* unk_0x0;
    /* 0x04 */ u32 scorerID;
};

extern LastScorerInfo* g_pLastScorer__5Audio;

template <typename CharT>
int nlStrNICmp(const CharT*, const CharT*, unsigned long);

// Team name strings
static const char* s_TeamNames[] = {
    "Daisy",      // TEAM_DAISY = 0
    "DonkeyKong", // TEAM_DONKEYKONG = 1
    "Luigi",      // TEAM_LUIGI = 2
    "Mario",      // TEAM_MARIO = 3
    "Peach",      // TEAM_PEACH = 4
    "Waluigi",    // TEAM_WALUIGI = 5
    "Wario",      // TEAM_WARIO = 6
    "Yoshi",      // TEAM_YOSHI = 7
    "Mystery"     // TEAM_MYSTERY = 8
};

// static const char* GetTeamNameString(eTeamID team)
// {
//     if (team >= 0 && team < NUM_TEAMS)
//     {
//         return s_TeamNames[team];
//     }
//     return "Unknown";
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014C388 | size: 0x1C
//  */
// void MemFun<PriorityStream, void>(void (PriorityStream::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014C350 | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*>(Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014C2B8 | size: 0x98
//  */
// void nlStrNICmp<char>(const char*, const char*, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8014C25C | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*> >::~FunctorImpl()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8014BFE8 | size: 0x274
 * TODO: 98.62% match - m_Track load scheduling, m_OrigStreamId/m_CapChant store order, stack temporary allocation order
 */
PriorityStream::PriorityStream(AudioStreamTrack::StreamTrack& track)
    : m_InPause(false)
    , m_Track(track)
    , m_HasCrowdStream(0)
    , m_PStream(m_Track)
    , m_CapChant(m_Track)
{
    m_PStream.m_OrigStreamId = 0;
    Function0<void> f0(Bind<void>(MemFun<PriorityStream, void>(&PriorityStream::TrackIdleCB), this));
    AudioStreamTrack::StreamTrack& trackRef = m_Track;
    trackRef.m_IdleCallback = Function<FnVoidVoid>(f0);
}

// /**
//  * Offset/Address/Size: 0xE4 | 0x8014BFB8 | size: 0x30
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*> >::operator()()
// {
// }

// /**
//  * Offset/Address/Size: 0x6C | 0x8014BF40 | size: 0x78
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*> >::Clone() const
// {
// }

/**
 * Offset/Address/Size: 0x69C | 0x8014BEB4 | size: 0x20
 */
void Audio::InitStreaming()
{
    PlatAudio::InitStreaming();
}

/**
 * Offset/Address/Size: 0x65C | 0x8014BE74 | size: 0x40
 */
void Audio::StopStreaming()
{
    if (AudioLoader::gbDisableAudio == false)
    {
        g_pTrackManager->StopAllTracks(0);
    }
}

// Stadium name strings (based on WorldLoader)
static const char* s_StadiumNames[] = {
    "Mario_Stadium", // 0
    "The_Palace",    // 1
    "DK_Daisy",      // 2
    "Wario_Stadium", // 3
    "Yoshi_Stadium", // 4
    "Super_Stadium", // 5
    "Forbidden_Dome" // 6
};

// Scorer name strings (character names)
static const char* s_ScorerNames[] = {
    "Mario",      // 0
    "Luigi",      // 1
    "Peach",      // 2
    "Daisy",      // 3
    "Yoshi",      // 4
    "DonkeyKong", // 5
    "Wario",      // 6
    "Waluigi",    // 7
    "Bowser",     // 8
    "Toad",       // 9
    "Koopa",      // 10
    "ShyGuy",     // 11
    "Birdo"       // 12
};

/**
 * Offset/Address/Size: 0x2EC | 0x8014BB04 | size: 0x370
 */
bool Audio::TrackMgrFileNameParamLookup(const char* param, char* out, unsigned long size)
{
    s32 hash = nlStringLowerHash(param);
    const char* value;

    switch (hash)
    {
    case 0x3B762F28:
        value = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0));
        nlStrNCpy(out, value, size);
        break;

    case (s32)0xB59118F1:
    {
        u32 outValue = (u32)GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1));
        nlStrNCpy(out, (const char*)outValue, size);
        break;
    }

    case (s32)0xBDD1CBB6:
        switch (nlSingleton<GameInfoManager>::s_pInstance->GetStadium())
        {
        case 0:
            nlStrNCpy(out, "pipeline", size);
            break;
        case 1:
            nlStrNCpy(out, "palace", size);
            break;
        case 2:
            nlStrNCpy(out, "konga", size);
            break;
        case 3:
            nlStrNCpy(out, "underground", size);
            break;
        case 4:
            nlStrNCpy(out, "yoshi", size);
            break;
        case 5:
            nlStrNCpy(out, "super", size);
            break;
        case 6:
            nlStrNCpy(out, "forbidden", size);
            break;
        default:
            return false;
        }
        break;

    case (s32)0xAEDB83D0:
        switch (g_pLastScorer__5Audio->scorerID)
        {
        case 0:
        case 3:
        case 4:
        case 8:
        case 12:
            nlStrNCpy(out, "Side", size);
            break;

        case 1:
            nlStrNCpy(out, "Daisy", size);
            break;

        case 2:
            nlStrNCpy(out, "DonkeyKong", size);
            break;

        case 5:
            nlStrNCpy(out, "Luigi", size);
            break;

        case 6:
            nlStrNCpy(out, "Mario", size);
            break;

        case 7:
            nlStrNCpy(out, "Peach", size);
            break;

        case 9:
            nlStrNCpy(out, "Waluigi", size);
            break;

        case 10:
            nlStrNCpy(out, "Wario", size);
            break;

        case 11:
            nlStrNCpy(out, "Yoshi", size);
            break;

        default:
            return false;
        }
        break;

    case 0x3F76E1B9:
    {
        int sideInt = g_pTeams[0]->m_nScore < g_pTeams[1]->m_nScore;
        s16 side = (s16)sideInt;
        u32 outValue = (u32)GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(side));
        nlStrNCpy(out, (const char*)outValue, size);
        break;
    }

    default:
        if (nlStrNICmp<char>(param, "RAND", 4) == 0)
        {
            u32 num = atoi(param + 4);
            const char* format = "%02d";
            if (num < 10)
            {
                format = "%d";
            }
            nlSNPrintf(out, size, format, nlRandom(num, &nlDefaultSeed) + 1);
            break;
        }

        nlPrintf("Unknown stream param \"%s\"\n", param);
        return false;
    }

    return true;
}

/**
 * Offset/Address/Size: 0x27C | 0x8014BA94 | size: 0x70
 */
void Audio::CreatePriorityStreams()
{
    PriorityStream* stream = (PriorityStream*)nlMalloc(0x78, 8, false);
    stream = new (stream) PriorityStream(g_pTrackManager->CreateTrack("Priority", MasterVolume::VG_Special));
    g_pPriorityStream = stream;
}

/**
 * Offset/Address/Size: 0xDC | 0x8014B8F4 | size: 0x1A0
 * TODO: 98.98% match - stack temporary allocation swap (sp+0x08 vs sp+0x10 for
 * Function0<void> copy param and Function<FnVoidVoid> result)
 */
void Audio::DestroyPriorityStreams()
{
    PriorityStream* ps;
    if ((ps = g_pPriorityStream) != NULL)
    {
        AudioStreamTrack::StreamTrack& track = ps->m_Track;
        {
            Function0<void> f0;
            f0.mTag = EMPTY;
            track.m_IdleCallback = Function<FnVoidVoid>(f0);
        }
        delete ps;
    }
    g_pPriorityStream = NULL;
}

/**
 * Offset/Address/Size: 0xD4 | 0x8014B8EC | size: 0x8
 */
PriorityStream* Audio::GetPriorityStream()
{
    return g_pPriorityStream;
}

/**
 * Offset/Address/Size: 0x0 | 0x8014B818 | size: 0xD4
 */
void Audio::ConfigureStreamBuffers(unsigned long count)
{
    if (!GetConfigBool(Config::Global(), "no_stream", false))
    {
        PlatAudio::ConfigureStreamBuffers(count);
    }
}
