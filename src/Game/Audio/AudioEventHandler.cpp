#include "Game/Audio/AudioEventHandler.h"

namespace PlatAudio
{
void InitEmitter(unsigned long);
bool RemoveEmitter(unsigned long);
} // namespace PlatAudio

struct cPlayer;
struct GoalScoredData : public EventData
{
    unsigned int uTeamIndex : 8;
    unsigned int uNumGoalsScored : 8;
    unsigned int uGoalType : 15;
    unsigned int uIsHyper : 1;
    unsigned int _v3ShotPosition[3];
    cPlayer* pScorer;
    cPlayer* pAssister;
    cPlayer* pLastTouch[2];

    virtual unsigned long GetID() { return 0x18A; }
};

namespace AudioStreamTrack
{
class TrackManagerBase
{
public:
    virtual ~TrackManagerBase();
    virtual void Update(float);
    virtual void CreateTrack(const char*, int);
    virtual void DestroyAllTracks();
    virtual void* GetTrack(unsigned long);
    virtual void StopAllTracks(unsigned long);
};
} // namespace AudioStreamTrack

class PriorityStream
{
public:
    void FakePause(unsigned long);
    void FakeResume(bool);
};

class AudioLoader
{
public:
    static void PlayPauseMenuMusic();
    static void StopPauseMenuMusic();
};

class FEAudio
{
public:
    static long PlayAnimAudioEvent(const char*, bool);
};

namespace CrowdMood
{
void RestartLoops();
void EnableCrowdDecay(bool);
void SetCrowdVolume(unsigned long, unsigned long);
void Purge(bool);
} // namespace CrowdMood

extern unsigned long g_MusicTrackPrePauseStreamId;
extern AudioStreamTrack::TrackManagerBase* g_pTrackManager;

namespace Audio
{
bool IsInited();
bool IsWorldSFXLoaded();
void Silence();
PriorityStream* GetPriorityStream();

enum eWorldSFX
{
    WORLDSFX_PLACEHOLDER = 0
};

class cWorldSFX
{
public:
    unsigned long Play(eWorldSFX, float, float, bool, float);
};

extern bool gbGameIsPaused;
extern bool gbStartingGame;
extern bool g_bHomeTeamHasJustScored;
extern cPlayer* g_pLastScorer;
extern cWorldSFX gWorldSFX;
} // namespace Audio

class cGame;
extern cGame* g_pGame;

/**
 * Offset/Address/Size: 0x0 | 0x801423B4 | size: 0x1A18
 * TODO: 3.69% match - jump-table dispatch and most event case bodies still need decomp.
 */
void Audio::AudioEventHandler(Event* pEvent, void*)
{
    if (!Audio::IsInited())
    {
        return;
    }

    if (g_pGame == NULL)
    {
        return;
    }

    switch (pEvent->m_uEventID)
    {
    case 0:
        Audio::gbGameIsPaused = true;
        Audio::GetPriorityStream()->FakePause(0);
        g_MusicTrackPrePauseStreamId = 0;
        g_pTrackManager->StopAllTracks(0);
        CrowdMood::SetCrowdVolume(0, 0xFA);
        CrowdMood::Purge(true);
        CrowdMood::EnableCrowdDecay(false);
        Audio::Silence();
        FEAudio::PlayAnimAudioEvent("sfx_screen_forward", false);
        AudioLoader::PlayPauseMenuMusic();
        break;

    case 1:
        Audio::gbGameIsPaused = false;
        Audio::GetPriorityStream()->FakeResume(true);
        AudioLoader::StopPauseMenuMusic();
        CrowdMood::RestartLoops();
        CrowdMood::SetCrowdVolume(0x7F, 0xFA);
        CrowdMood::EnableCrowdDecay(true);
        break;

    case 3:
    {
        int i;
        for (i = 0; i < 64; i++)
        {
            PlatAudio::RemoveEmitter((unsigned long)i);
            PlatAudio::InitEmitter((unsigned long)i);
        }
        break;
    }

    case 5:
    {
        GoalScoredData* pData;
        if ((int)pEvent->m_data.GetID() == -1)
        {
            pData = 0;
        }
        else if ((int)pEvent->m_data.GetID() != 0x18A)
        {
            pData = 0;
        }
        else
        {
            pData = (GoalScoredData*)&pEvent->m_data;
        }

        Audio::g_pLastScorer = pData->pLastTouch[pData->uTeamIndex];
        break;
    }

    case 6:
    case 9:
        if (Audio::gbGameIsPaused)
        {
            Audio::gbGameIsPaused = false;
        }
        break;

    case 10:
        if (Audio::IsWorldSFXLoaded())
        {
            Audio::gWorldSFX.Play((Audio::eWorldSFX)0x4F, 100.0f, -1.0f, true, 100.0f);

            if (Audio::g_bHomeTeamHasJustScored)
            {
                Audio::g_bHomeTeamHasJustScored = false;
            }

            if (Audio::gbStartingGame)
            {
                int i;
                for (i = 0; i < 64; i++)
                {
                    PlatAudio::RemoveEmitter((unsigned long)i);
                    PlatAudio::InitEmitter((unsigned long)i);
                }
                Audio::gbStartingGame = false;
            }
        }
        break;

    case 2:
    case 4:
    case 7:
    case 8:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
        break;

    default:
        break;
    }
}
