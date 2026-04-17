
#include "Game/Sys/audio.h"
#include "Game/Sys/debug.h"
#include "Game/Game.h"
#include "Game/GameAudio.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/TransitionTask.h"
#include "Game/BasicStadium.h"

#include "NL/nlList.h"
#include "NL/nlMemory.h"
#include "NL/nlSortedSlot.h"
#include "NL/nlTimer.h"
#include "NL/plat/plataudio.h"

// Include PlatStream.h after plataudio.h to get the PlatAudio class
// The namespace PlatAudio and class PlatAudio can coexist
#include "Game/Sys/PlatStream.h"

struct FadeAudioData // TODO: this should be a ListEntry<T>
{
    /* 0x00 */ char padding[0x28];
    /* 0x28 */ FadeAudioData* next; // Pointer to next node for nlDeleteList
};

struct DelayTimer
{
    float _unk_0x0;
    // todo: implement
};

FadeAudioData* g_pFadeList;

bool gbFilterOn = false;
bool gbPitchBent = false;
bool gbUseHiQualityReverb = false;
bool gbListenerInit = false;

bool g_bAudioInitialized = false;
bool g_bAudioInGameLoaded = false;
bool g_bWorldSFXInitialized = false;

static f32 gfVolumeGroups[0x18];

extern Audio::SoundAttributes gDelayedSFX[15];

extern SoundPropAccessor* gpWORLDSoundPropAccessor;
extern SoundPropAccessor* gpPWRUPSoundPropAccessor;
extern SoundPropAccessor* gpSTADGENSoundPropAccessor;
extern SoundPropAccessor* gpCROWDSoundPropAccessor;

namespace AudioScriptEventMgr
{
void Update();
}

class cBaseCamera;
class cCameraManager
{
public:
    static cBaseCamera* m_cameraStack;
    static void GetViewVector(nlVector3&);
    static void GetUpVector(nlVector3&);
};

extern bool gbTestPrintout;
extern float gfSilenceTimer;
extern unsigned long uCurrentSFXVolume;
extern unsigned long uSFXVolume;

static const nlVector3 sListenerZero = { 0.0f, 0.0f, 0.0f };

/**
 * Offset/Address/Size: 0xF0 | 0x801413B0 | size: 0x28
 */
template <>
void nlListAddStart<FadeAudioData>(FadeAudioData** head, FadeAudioData* entry, FadeAudioData** tail)
{
    if (tail != 0)
    {
        if (*head == 0)
        {
            *tail = entry;
        }
    }

    entry->next = *head;
    *head = entry;
}

namespace Audio
{

cWorldSFX gWorldSFX;
cWorldSFX gPowerupSFX;
cWorldSFX gStadGenSFX;
cWorldSFX gCrowdSFX;

SND_LISTENER gListener;
float gChantDelayTimer;
bool gbGameIsPaused = false;
bool gbStartingGame = true;
bool g_bHomeTeamHasJustScored = false;
float g_fAudioTimer = 0.0f;

/**
 * Offset/Address/Size: 0x0 | 0x8013C514 | size: 0x10
 */
float MasterVolume::GetVoiceVolume()
{
    return gfVolumeGroups[3];
}

/**
 * Offset/Address/Size: 0x10 | 0x8013C524 | size: 0x208
 */
void MasterVolume::SetVoiceVolume(float volume, int time)
{
    Audio::SetVolGroupVolume(5, volume * gfVolumeGroups[8], time);
    Audio::SetVolGroupVolume(6, volume * gfVolumeGroups[9], time);
    Audio::SetVolGroupVolume(7, volume * gfVolumeGroups[10], time);
    Audio::SetVolGroupVolume(8, volume * gfVolumeGroups[11], time);
    Audio::SetVolGroupVolume(9, volume * gfVolumeGroups[12], time);
    Audio::SetVolGroupVolume(10, volume * gfVolumeGroups[13], time);
    Audio::SetVolGroupVolume(11, volume * gfVolumeGroups[14], time);
    Audio::SetVolGroupVolume(12, volume * gfVolumeGroups[15], time);
    Audio::SetVolGroupVolume(13, volume * gfVolumeGroups[16], time);
    Audio::SetVolGroupVolume(14, volume * gfVolumeGroups[17], time);
    Audio::SetVolGroupVolume(15, volume * gfVolumeGroups[18], time);
    Audio::SetVolGroupVolume(16, volume * gfVolumeGroups[19], time);
    Audio::SetVolGroupVolume(17, volume * gfVolumeGroups[20], time);
    Audio::SetVolGroupVolume(18, volume * gfVolumeGroups[21], time);
    Audio::SetVolGroupVolume(19, volume * gfVolumeGroups[22], time);
    Audio::SetVolGroupVolume(4, volume * gfVolumeGroups[7], time);
    gfVolumeGroups[3] = volume;
}

/**
 * Offset/Address/Size: 0x218 | 0x8013C72C | size: 0x14
 * TODO: 97% match - register allocation (r3/r4 swap)
 */
void MasterVolume::SetVolume(MasterVolume::VOLUME_GROUP group, float volume)
{
    gfVolumeGroups[group] = volume;
}

/**
 * Offset/Address/Size: 0x22C | 0x8013C740 | size: 0x14
 * TODO: 97% match - register allocation (r3/r4 swap)
 */
float MasterVolume::GetVolume(MasterVolume::VOLUME_GROUP group)
{
    return gfVolumeGroups[group];
}

/**
 * Offset/Address/Size: 0x240 | 0x8013C754 | size: 0x1A0
 */
void FadeFilterFromCurrentToZero()
{
    FadeAudioData* node;

    f32 t = (f32)gWorldSFX.muGroupFilterFreq / 16383.0f;
    if (t > 0.0f)
    {
        node = g_pFadeList;
        while (node != NULL)
        {
            s32 type = *(s32*)node;
            if (!(type != 2 && type != 3 && !(type == 2 && ((s32*)node)[1] == 0)))
            {
                nlListRemoveElement<FadeAudioData>(&g_pFadeList, node, NULL);
                FadeAudioData* next = node->next;
                delete node;
                node = next;
            }
            else
            {
                node = node->next;
            }
        }
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        FadeFilter(t, tweaks->unk210, tweaks->unk20C, 0.0f);
    }

    if (gbPitchBent)
    {
        t = (f32)gWorldSFX.muGroupPitch / 8191.0f;
        node = g_pFadeList;
        while (node != NULL)
        {
            s32 type = *(s32*)node;
            if (!(type != 2 && type != 3 && !(type == 3 && ((s32*)node)[1] == 0)))
            {
                nlListRemoveElement<FadeAudioData>(&g_pFadeList, node, NULL);
                FadeAudioData* next = node->next;
                delete node;
                node = next;
            }
            else
            {
                node = node->next;
            }
        }
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        PitchBend(t, tweaks->unk224, tweaks->unk20C, 0.0f);
    }
}

/**
 * Offset/Address/Size: 0x3E0 | 0x8013C8F4 | size: 0x94
 */
void FadeFilterToFullStrength()
{
    if (!gbFilterOn)
    {
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        FadeFilter(tweaks->unk210, tweaks->unk214, tweaks->unk208, 0.0f);
    }

    if (!gbPitchBent)
    {
        f32 t = (f32)gWorldSFX.muGroupPitch / 16383.0f;
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        PitchBend(t, tweaks->unk220, tweaks->unk208, 0.0f);
    }
}

/**
 * Offset/Address/Size: 0x474 | 0x8013C988 | size: 0x354
 * TODO: 97.61% match - register allocation in team/player loops (same as FadeFilter),
 *       pitchBend u16 load register swap (r28/r0), search loop beq vs bne+b compiler peephole.
 *       #pragma inline_depth(0) needed in decomp.me scratch to prevent nlListAddStart inlining.
 */
void PitchBend(float param1, float param2, float param3, float param4)
{
    TransitionTask* pTask = TransitionTask::sm_pGlobalTask;
    TRANSITION_STATE state;
    if (pTask != NULL)
    {
        state = pTask->m_TransitionState;
    }
    else
    {
        state = (TRANSITION_STATE)0;
    }
    if (state == eTS_Destroying)
    {
        return;
    }

    float diff = param2 - param1;
    float fadePerFrame;
    if (param3 <= 0.0f)
    {
        fadePerFrame = diff;
        if (0.0f != param4)
        {
            goto createFade;
        }
        if (0.5f == param2)
        {
            // Reset pitch bend to center (0x2000)
            if (!gbPitchBent)
            {
                return;
            }
            if (g_pGame != NULL)
            {
                for (int t = 0; t < 2; t++)
                {
                    cTeam* team = g_pTeams[t];
                    for (int p = 0; p < 5; p++)
                    {
                        team->GetPlayer(p)->m_pCharacterSFX->SetPitchBendOnAllDialogueSFX(0x2000);
                    }
                }
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->SetPitchBendOnAllDialogueSFX(0x2000);
            }
            gbPitchBent = false;
        }
        else
        {
            // Set pitch bend from game tweaks
            if (gbPitchBent)
            {
                return;
            }
            u16 pitchBend = (u16)(s32)(16384.0f * g_pGame->m_pGameTweaks->unk220);
            if (pitchBend > 0x3FFF)
            {
                pitchBend = 0x3FFF;
            }
            if (g_pGame != NULL)
            {
                for (int t = 0; t < 2; t++)
                {
                    cTeam* team = g_pTeams[t];
                    for (int p = 0; p < 5; p++)
                    {
                        team->GetPlayer(p)->m_pCharacterSFX->SetPitchBendOnAllDialogueSFX(pitchBend);
                    }
                }
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->SetPitchBendOnAllDialogueSFX(pitchBend);
            }
            if (pitchBend != 0x2000)
            {
                gbPitchBent = true;
            }
            else
            {
                gbPitchBent = false;
            }
        }
        return;
    }
    else
    {
        fadePerFrame = diff / param3;
    }
createFade:
    if (pTask != NULL)
    {
        state = pTask->m_TransitionState;
    }
    else
    {
        state = (TRANSITION_STATE)0;
    }
    if (state == eTS_Destroying)
    {
        return;
    }

    // Search for existing fade with same type (3=pitch bend) and target
    FadeAudioData* existing = g_pFadeList;
    while (existing != NULL)
    {
        if (*(s32*)existing == 3)
        {
            if (*(float*)((char*)existing + 0x14) != param2)
            {
            }
            else
            {
                goto foundExisting;
            }
        }
        existing = existing->next;
    }
    existing = NULL;
foundExisting:
    if (existing != NULL)
    {
        nlListRemoveElement<FadeAudioData>(&g_pFadeList, existing, NULL);
        delete existing;
    }

    // Allocate new FadeAudioData
    FadeAudioData* newFade = (FadeAudioData*)nlMalloc(0x2C, 8, false);

    // Constructor-like initialization (default values with dead stores)
    *(s32*)((char*)newFade) = 0;
    *(s32*)((char*)newFade + 0x04) = -1;
    *(s32*)((char*)newFade + 0x04) = 0;
    *(float*)((char*)newFade + 0x08) = 0.0f;
    *(float*)((char*)newFade + 0x0C) = 0.0f;
    *(float*)((char*)newFade + 0x10) = 0.0f;
    *(float*)((char*)newFade + 0x14) = 0.0f;
    *(float*)((char*)newFade + 0x18) = -1.0f;
    *(s32*)((char*)newFade + 0x18) = 0;
    *((char*)newFade + 0x1C) = 0;
    *((char*)newFade + 0x1D) = 0;
    *((char*)newFade + 0x1E) = 0;
    *((char*)newFade + 0x1F) = 0;
    *((char*)newFade + 0x20) = 0;
    *((char*)newFade + 0x21) = 0;
    *(float*)((char*)newFade + 0x24) = -1.0f;

    // Set actual fade values
    *(s32*)((char*)newFade) = 3;                     // type = 3 (pitch bend)
    *(float*)((char*)newFade + 0x08) = fadePerFrame; // fade rate per frame
    *(float*)((char*)newFade + 0x0C) = param4;       // delay
    *(float*)((char*)newFade + 0x10) = param3;       // time
    *(float*)((char*)newFade + 0x14) = param2;       // target
    *(float*)((char*)newFade + 0x18) = param1;       // from/current

    // Set direction flags based on target vs game tweaks
    if (param2 == g_pGame->m_pGameTweaks->unk224)
    {
        *((char*)newFade + 0x20) = 0;
        *((char*)newFade + 0x1C) = 1;
    }
    else
    {
        *((char*)newFade + 0x20) = 1;
        *((char*)newFade + 0x1C) = 0;
    }

    newFade->next = NULL;
    nlListAddStart<FadeAudioData>(&g_pFadeList, newFade, NULL);
}

/**
 * Offset/Address/Size: 0x7C8 | 0x8013CCDC | size: 0x510
 * TODO: 97.99% match - register allocation in team/player loops (r28-r31 swapped),
 *       while loop beq vs bne+b branch pattern
 */
void FadeFilter(float currentVal, float fadeToVal, float fadeDuration, float fadeTimeStart)
{
    TransitionTask* pTask = TransitionTask::sm_pGlobalTask;
    TRANSITION_STATE state;
    if (pTask != NULL)
    {
        state = pTask->m_TransitionState;
    }
    else
    {
        state = (TRANSITION_STATE)0;
    }
    if (state == eTS_Destroying)
    {
        return;
    }

    float volDiff = fadeToVal - currentVal;
    float stepSize;
    if (fadeDuration <= 0.0f)
    {
        stepSize = volDiff;
        if (0.0f != fadeTimeStart)
        {
            goto createFade;
        }
        if (0.0f == fadeToVal)
        {
            if (!gbFilterOn)
            {
                return;
            }
            gWorldSFX.ActivateFilterOnAllTrackedSFX(false);
            gPowerupSFX.ActivateFilterOnAllTrackedSFX(false);
            gStadGenSFX.ActivateFilterOnAllTrackedSFX(false);
            gCrowdSFX.ActivateFilterOnAllTrackedSFX(false);
            if (g_pGame != NULL)
            {
                for (int t = 0; t < 2; t++)
                {
                    cTeam* team = g_pTeams[t];
                    for (int p = 0; p < 5; p++)
                    {
                        team->GetPlayer(p)->m_pCharacterSFX->ActivateFilterOnAllTrackedSFX(false);
                    }
                }
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->ActivateFilterOnAllTrackedSFX(false);
            }
            CrowdMood::ActivateLPF(false);
            gbFilterOn = false;
            gWorldSFX.SetFilterFreqOnAllTrackedSFX(0);
            gPowerupSFX.SetFilterFreqOnAllTrackedSFX(0);
            gStadGenSFX.SetFilterFreqOnAllTrackedSFX(0);
            gCrowdSFX.SetFilterFreqOnAllTrackedSFX(0);
            if (g_pGame != NULL)
            {
                for (int t = 0; t < 2; t++)
                {
                    cTeam* team = g_pTeams[t];
                    for (int p = 0; p < 5; p++)
                    {
                        team->GetPlayer(p)->m_pCharacterSFX->SetFilterFreqOnAllTrackedSFX(0);
                    }
                }
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->SetFilterFreqOnAllTrackedSFX(0);
            }
            CrowdMood::SetLPF(0);
        }
        else
        {
            if (gbFilterOn)
            {
                return;
            }
            gWorldSFX.ActivateFilterOnAllTrackedSFX(true);
            gPowerupSFX.ActivateFilterOnAllTrackedSFX(true);
            gStadGenSFX.ActivateFilterOnAllTrackedSFX(true);
            gCrowdSFX.ActivateFilterOnAllTrackedSFX(true);
            if (g_pGame != NULL)
            {
                for (int t = 0; t < 2; t++)
                {
                    cTeam* team = g_pTeams[t];
                    for (int p = 0; p < 5; p++)
                    {
                        team->GetPlayer(p)->m_pCharacterSFX->ActivateFilterOnAllTrackedSFX(true);
                    }
                }
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->ActivateFilterOnAllTrackedSFX(true);
            }
            CrowdMood::ActivateLPF(true);
            gbFilterOn = true;
            gWorldSFX.SetFilterFreqOnAllTrackedSFX(0x3FFF);
            gPowerupSFX.SetFilterFreqOnAllTrackedSFX(0x3FFF);
            gStadGenSFX.SetFilterFreqOnAllTrackedSFX(0x3FFF);
            gCrowdSFX.SetFilterFreqOnAllTrackedSFX(0x3FFF);
            if (g_pGame != NULL)
            {
                for (int t = 0; t < 2; t++)
                {
                    cTeam* team = g_pTeams[t];
                    for (int p = 0; p < 5; p++)
                    {
                        team->GetPlayer(p)->m_pCharacterSFX->SetFilterFreqOnAllTrackedSFX(0x3FFF);
                    }
                }
                BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->SetFilterFreqOnAllTrackedSFX(0x3FFF);
            }
            CrowdMood::SetLPF(0x3FFF);
        }
        return;
    }
    else
    {
        stepSize = volDiff / fadeDuration;
    }
createFade:
    if (pTask != NULL)
    {
        state = pTask->m_TransitionState;
    }
    else
    {
        state = (TRANSITION_STATE)0;
    }
    if (state == eTS_Destroying)
    {
        return;
    }

    FadeAudioData* existing = g_pFadeList;
    while (existing != NULL)
    {
        if (*(s32*)existing == 2)
        {
            if (*(float*)((char*)existing + 0x14) == fadeToVal)
            {
                goto foundExisting;
            }
        }
        existing = existing->next;
    }
    existing = NULL;
foundExisting:
    if (existing != NULL)
    {
        nlListRemoveElement<FadeAudioData>(&g_pFadeList, existing, NULL);
        delete existing;
    }

    FadeAudioData* newFade = (FadeAudioData*)nlMalloc(0x2C, 8, false);

    *(s32*)((char*)newFade) = 0;
    *(s32*)((char*)newFade + 0x04) = -1;
    *(s32*)((char*)newFade + 0x04) = 0;
    *(float*)((char*)newFade + 0x08) = 0.0f;
    *(float*)((char*)newFade + 0x0C) = 0.0f;
    *(float*)((char*)newFade + 0x10) = 0.0f;
    *(float*)((char*)newFade + 0x14) = 0.0f;
    *(float*)((char*)newFade + 0x18) = -1.0f;
    *(s32*)((char*)newFade + 0x18) = 0;
    *((char*)newFade + 0x1C) = 0;
    *((char*)newFade + 0x1D) = 0;
    *((char*)newFade + 0x1E) = 0;
    *((char*)newFade + 0x1F) = 0;
    *((char*)newFade + 0x20) = 0;
    *((char*)newFade + 0x21) = 0;
    *(float*)((char*)newFade + 0x24) = -1.0f;

    *(s32*)((char*)newFade) = 2;
    *(float*)((char*)newFade + 0x08) = stepSize;
    *(float*)((char*)newFade + 0x0C) = fadeTimeStart;
    *(float*)((char*)newFade + 0x10) = fadeDuration;
    *(float*)((char*)newFade + 0x14) = fadeToVal;
    *(float*)((char*)newFade + 0x18) = currentVal;

    if (0.0f == fadeToVal)
    {
        *((char*)newFade + 0x1E) = 0;
        *((char*)newFade + 0x1C) = 1;
    }
    else
    {
        *((char*)newFade + 0x1E) = 1;
        *((char*)newFade + 0x1C) = 0;
    }

    newFade->next = NULL;
#pragma inline_depth(0)
    nlListAddStart<FadeAudioData>(&g_pFadeList, newFade, NULL);
#pragma inline_depth
}

/**
 * Offset/Address/Size: 0xCD8 | 0x8013D1EC | size: 0x2C
 */
void ClearFadeData()
{
    nlDeleteList<FadeAudioData>(&g_pFadeList);
    g_pFadeList = NULL;
}

/**
 * Offset/Address/Size: 0xD04 | 0x8013D218 | size: 0x20
 */
bool IsEmitterActive(SFXEmitter* emitter)
{
    return PlatAudio::IsEmitterActive(emitter);
}

/**
 * Offset/Address/Size: 0xD24 | 0x8013D238 | size: 0x20
 */
u32 GetEmitterVoiceID(SFXEmitter* emitter)
{
    return PlatAudio::GetEmitterVoiceID(emitter);
}

/**
 * Offset/Address/Size: 0xD44 | 0x8013D258 | size: 0x20
 */
bool Remove3DSFXEmitter(SFXEmitter* emitter)
{
    return PlatAudio::RemoveEmitter(emitter);
}

/**
 * Offset/Address/Size: 0xD64 | 0x8013D278 | size: 0x20
 */
void Add3DSFXEmitter(const EmitterStartInfo& emitterStartInfo)
{
    PlatAudio::Add3DSFXEmitter(emitterStartInfo);
}

/**
 * Offset/Address/Size: 0xD84 | 0x8013D298 | size: 0x20
 */
SFXEmitter* GetFreeEmitter(unsigned long& id)
{
    return PlatAudio::GetFreeEmitter(id);
}

/**
 * Offset/Address/Size: 0xDA4 | 0x8013D2B8 | size: 0x20
 */
SFXEmitter* GetEmitter(unsigned long id)
{
    return PlatAudio::GetSFXEmitter(id);
}

/**
 * Offset/Address/Size: 0xDC4 | 0x8013D2D8 | size: 0x8
 */
void SetListenerActive(bool active)
{
    gbListenerInit = active;
}

/**
 * Offset/Address/Size: 0xDCC | 0x8013D2E0 | size: 0x8
 */
bool IsListenerActive()
{
    return gbListenerInit;
}

/**
 * Offset/Address/Size: 0xDCC | 0x8013D2E0 | size: 0x8
 */
void SetOutputMode(MusyXOutputType outputType)
{
    PlatAudio::SetOutputMode(outputType);
}

/**
 * Offset/Address/Size: 0xDF4 | 0x8013D308 | size: 0x78
 */
bool SetPitchBendOnSFX(unsigned long uVoiceID, unsigned short pitch)
{
    bool isPlaying;
    if (g_bAudioInitialized)
    {
        isPlaying = PlatAudio::IsSFXPlaying(uVoiceID);
    }
    else
    {
        isPlaying = false;
    }
    if (!isPlaying)
    {
        return true;
    }
    if (pitch > 0x3FFF)
    {
        pitch = 0x3FFF;
    }
    return PlatAudio::SetPitchBendOnSFX(uVoiceID, pitch);
}

/**
 * Offset/Address/Size: 0xE6C | 0x8013D380 | size: 0x78
 */
bool SetFilterFreqOnSFX(unsigned long uVoiceID, unsigned short freq)
{
    bool isPlaying;
    if (g_bAudioInitialized)
    {
        isPlaying = PlatAudio::IsSFXPlaying(uVoiceID);
    }
    else
    {
        isPlaying = false;
    }
    if (!isPlaying)
    {
        return true;
    }
    if (freq > 0x3FFF)
    {
        freq = 0x3FFF;
    }
    return PlatAudio::SetFilterFreqOnSFX(uVoiceID, freq);
}

/**
 * Offset/Address/Size: 0xEE4 | 0x8013D3F8 | size: 0x88
 */
bool ActivateFilterOnSFX(unsigned long uVoiceID, bool bOn)
{
    bool isPlaying;
    if (::g_bAudioInitialized)
    {
        isPlaying = PlatAudio::IsSFXPlaying(uVoiceID);
    }
    else
    {
        isPlaying = false;
    }
    if (!isPlaying)
    {
        return true;
    }
    if (bOn)
    {
        return PlatAudio::SetMIDIControllerVal14Bit(uVoiceID, 0x4F, 0x2000);
    }
    else
    {
        return PlatAudio::SetMIDIControllerVal14Bit(uVoiceID, 0x4F, 0x1FFF);
    }
}

/**
 * Offset/Address/Size: 0xF6C | 0x8013D480 | size: 0xBC
 */
void Audio::SetPitchBendOnAllDialogueSFX(unsigned short pitch)
{
    if (pitch > 0x3FFF)
    {
        pitch = 0x3FFF;
    }
    if (g_pGame != NULL)
    {
        for (int i = 0; i < 2; i++)
        {
            cTeam* pTeam = g_pTeams[i];
            for (int j = 0; j < 5; j++)
            {
                pTeam->GetPlayer(j)->m_pCharacterSFX->SetPitchBendOnAllDialogueSFX(pitch);
            }
        }
        BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->SetPitchBendOnAllDialogueSFX(pitch);
    }
    gbPitchBent = (pitch != 0x2000);
}

/**
 * Offset/Address/Size: 0x1028 | 0x8013D53C | size: 0xE0
 */
// void ActivateFilterOnAllCurrentSFX(bool)
// {
// }

/**
 * Offset/Address/Size: 0x1108 | 0x8013D61C | size: 0x1CC
 */
void SetVolGroupVolume(int volGroup, float fVol, int fadeTime)
{
    unsigned char group;
    unsigned short fade;

    volGroup = (volGroup >= 0) ? volGroup : 0;
    volGroup = (volGroup > 255) ? 255 : volGroup;
    group = (unsigned char)volGroup;

    fadeTime = (fadeTime >= 0) ? fadeTime : 0;
    fadeTime = (fadeTime > 65535) ? 65535 : fadeTime;
    fade = (unsigned short)fadeTime;

    if (group == 0x20)
    {
        PlatAudio::SetVolGroupVolume(0x1e, fVol, fade);
        PlatAudio::SetVolGroupVolume(0x1f, fVol, fade);
        PlatAudio::SetVolGroupVolume(0x04, fVol, fade);
    }
    else if (group == 0x04)
    {
        PlatAudio::SetVolGroupVolume(0x04, fVol, fade);
    }
    else
    {
        u8 inRange = ((int)group >= 5 && (int)group <= 0x13);
        if (inRange)
        {
            PlatAudio::SetVolGroupVolume(group, fVol, fade);
        }
        else
        {
            switch (group)
            {
            case 2:
                PlatAudio::SetVolGroupVolume(2, fVol, fade);
                break;
            case 1:
            case 0x1e:
            {
                float vol = fVol;
                if (group == 0x1e)
                    vol = fVol * gfVolumeGroups[2];
                PlatAudio::SetVolGroupVolume(1, 0.9f * vol, fade);
                if (group == 1)
                    return;
            }
            case 3:
            {
                float vol = fVol;
                if (group == 0x1e)
                    vol = fVol * gfVolumeGroups[6];
                PlatAudio::SetVolGroupVolume(3, vol, fade);
                if (group == 3)
                    return;
            }
            }

            if (group != 0x1e)
                PlatAudio::SetVolGroupVolume(group, fVol, fade);
        }
    }
}

/**
 * Offset/Address/Size: 0x12D4 | 0x8013D7E8 | size: 0x4C
 */
// void SetSFXVolumeGroup(unsigned long, int)
// {
// }

/**
 * Offset/Address/Size: 0x1320 | 0x8013D834 | size: 0x20
 */
void SetSFXVolume(unsigned long voiceID, float volume)
{
    PlatAudio::SetSFXVolume(voiceID, volume);
}

/**
 * Offset/Address/Size: 0x1340 | 0x8013D854 | size: 0x25C
 * TODO: 99.88% match - store order swap in PHYSOBJ velocity copy (stw r4/r0 at 0x13c/0x140)
 */
void Update3DSFXEmitters()
{
    SFXEmitter* emitter;
    int i;
    float fVol;
    nlVector3 vel;

    for (i = 0; i < 64; i++)
    {
        emitter = PlatAudio::GetSFXEmitter(i);
        if (!emitter)
            continue;

        if (!PlatAudio::IsEmitterActive(emitter))
        {
            if (emitter->pOwner)
            {
                emitter->pOwner = NULL;
            }
            continue;
        }

        if (!emitter->pOwner)
            continue;
        bool stopping = emitter->bIsStopping;
        cGameSFX* pSFXOwner = (cGameSFX*)emitter->pOwner;

        if (!stopping)
        {
            fVol = pSFXOwner->GetSFXVol(emitter->soundType);

            if (emitter->m_unk_0x5F)
            {
                cBaseCamera* pCamera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
                nlVector3 pos = pCamera->GetCameraPosition();
                nlVector3 camVel = { 0.0f, 0.0f, 0.0f };
                PlatAudio::Update3DSFXEmitter(emitter, pos, camVel, fVol);
            }
            else if (emitter->posUpdateMethod == PHYSOBJ)
            {
                nlVector3 vel = { 0.0f, 0.0f, 0.0f };
                if (emitter->pPhysObj->m_bodyID)
                {
                    nlVector3& linVel = emitter->pPhysObj->GetLinearVelocity();
                    u32 b = linVel.as_u32[1];
                    u32 a = linVel.as_u32[0];
                    vel.as_u32[1] = b;
                    vel.as_u32[0] = a;
                    vel.as_u32[2] = linVel.as_u32[2];
                }
                PlatAudio::Update3DSFXEmitter(emitter, emitter->pPhysObj->GetPosition(), vel, fVol);
            }
            else if (emitter->posUpdateMethod == PTRS_TO_VECTORS)
            {
                PlatAudio::Update3DSFXEmitter(emitter, *emitter->pos.pvPos, *emitter->dir.pvDir, fVol);
                if (gbTestPrintout)
                {
                    nlPrintf("3D Sound Type: %d, Pos: %0.2f,%0.2f,%0.2f Heading: %0.2f,%0.2f,%0.2f Up: %0.2f,%0.2f,%0.2f\n",
                        emitter->soundType,
                        emitter->pos.pvPos->f.x,
                        emitter->pos.pvPos->f.y,
                        emitter->pos.pvPos->f.z);
                }
            }
            else if (emitter->posUpdateMethod == VECTORS)
            {
                PlatAudio::Update3DSFXEmitter(emitter, emitter->pos.vPos, emitter->dir.vDir, fVol);
            }
        }
        else
        {
            if (emitter->bKeepTrack)
            {
                switch (pSFXOwner->GetClassType())
                {
                case WORLD:
                    pSFXOwner->StopEmitter(emitter, 0);
                    break;
                case CHAR:
                {
                    cCharacterSFX* pCharSFX = (cCharacterSFX*)pSFXOwner;
                    pCharSFX->Stop((eCharSFX)emitter->soundType, cGameSFX::SFX_STOP_FIRST);
                    break;
                }
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x159C | 0x8013DAB0 | size: 0xA34
 */
// void UpdateFades(float)
// {
// }

/**
 * Offset/Address/Size: 0x1FD0 | 0x8013E4E4 | size: 0x354
 * TODO: 99.46% match - extra mr r30,r0 for gDelayedSFX address load (MWCC register coalescing)
 */
void Update(float fDeltaT)
{
    int i;
    register SoundAttributes* delayed;
    cTeam* pTeam;
    int j;
    cPlayer* pPlayer;

    if (::g_bAudioInitialized == false)
    {
        return;
    }

    g_fAudioTimer += fDeltaT;
    if (uSFXVolume != uCurrentSFXVolume)
    {
        sndVolume((u8)uSFXVolume, 0x1F4, 0xFE);
        uCurrentSFXVolume = uSFXVolume;
    }

    UpdateFades(fDeltaT);
    g_pTrackManager->Update(fDeltaT);

    if (g_pGame != NULL)
    {
        CrowdMood::Update(fDeltaT);
        AudioScriptEventMgr::Update();
    }

    if (::g_bWorldSFXInitialized)
    {
        gWorldSFX.UpdateAllTrackedSFX(fDeltaT);
        gPowerupSFX.UpdateAllTrackedSFX(fDeltaT);
        gStadGenSFX.UpdateAllTrackedSFX(fDeltaT);
        gCrowdSFX.UpdateAllTrackedSFX(fDeltaT);
    }

    if (g_pGame != NULL)
    {
        for (i = 0; i < 2; i++)
        {
            pTeam = g_pTeams[i];
            for (j = 0; j < 5; j++)
            {
                pPlayer = pTeam->GetPlayer(j);
                pPlayer->m_pCharacterSFX->UpdateAllTrackedSFX(fDeltaT);
            }
        }

        BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->UpdateAllTrackedSFX(fDeltaT);
    }

    if (gfSilenceTimer > 0.0f && g_fAudioTimer > gfSilenceTimer)
    {
        PlatAudio::StopAllSound();
        gfSilenceTimer = -1.0f;
    }

    if (gbGameIsPaused == false)
    {
        delayed = gDelayedSFX;
        for (i = 0; i < 15; i++, delayed++)
        {
            if (delayed->mu_Type != (unsigned long)-1)
            {
                if (delayed->mf_DelayTime >= 0.0f && delayed->mf_DelayTime - fDeltaT <= 0.0f)
                {
                    if (delayed->me_ClassType == CHAR)
                    {
                        delayed->mf_DelayTime = -1.0f;
                        delayed->mu_VoiceID = ((cCharacterSFX*)delayed->mp_OwnerSFX)->Play(*delayed);
                    }
                    else
                    {
                        delayed->mf_DelayTime = -1.0f;
                        delayed->mu_VoiceID = delayed->mp_OwnerSFX->Play(*delayed);
                    }

                    PlatAudio::SetSFXReverbVol(delayed->mu_VoiceID, delayed->mf_VolReverb);
                    delayed->Init();
                }
                else
                {
                    delayed->mf_DelayTime -= fDeltaT;
                }
            }
        }

        if (::gbListenerInit)
        {
            nlVector3 vDir;
            nlVector3 vCameraPos;
            nlVector3 vHeading;
            nlVector3 vUp;

            cBaseCamera* pCamera = nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack);
            vCameraPos = pCamera->GetCameraPosition();
            vDir = sListenerZero;
            cCameraManager::GetViewVector(vHeading);
            cCameraManager::GetUpVector(vUp);

            if (::gbTestPrintout)
            {
                nlPrintf("Listener Pos: %0.2f,%0.2f,%0.2f Heading: %0.2f,%0.2f,%0.2f Up: %0.2f,%0.2f,%0.2f\n",
                    vCameraPos.f.x,
                    vCameraPos.f.y,
                    vCameraPos.f.z,
                    vHeading.f.x,
                    vHeading.f.y,
                    vHeading.f.z,
                    vUp.f.x,
                    vUp.f.y,
                    vUp.f.z);
            }

            PlatAudio::Update3DSFXListener(&gListener, vCameraPos, vDir, vHeading, vUp, 1.0f);
        }

        Update3DSFXEmitters();
    }
}

/**
 * Offset/Address/Size: 0x2324 | 0x8013E838 | size: 0x20
 */
int GetSndIDError()
{
    return PlatAudio::GetSndIDError();
}

/**
 * Offset/Address/Size: 0x2344 | 0x8013E858 | size: 0x34
 */
bool IsSFXPlaying(unsigned long sfxID)
{
    if (g_bAudioInitialized)
    {
        return PlatAudio::IsSFXPlaying(sfxID);
    }
    return false;
}

/**
 * Offset/Address/Size: 0x2378 | 0x8013E88C | size: 0x34
 */
bool StopSFX(unsigned long sfxID)
{
    if (g_bAudioInitialized)
    {
        return PlatAudio::StopSFX(sfxID);
    }
    return false;
}

/**
 * Offset/Address/Size: 0x23AC | 0x8013E8C0 | size: 0x214
 */
unsigned long PlaySFXEventFromScript(const SoundEventData& sfxEventData, const char* szSFXType, float fVol, float fDelay)
{
    if (!g_bAudioInitialized || !g_bWorldSFXInitialized)
    {
        return -1;
    }

    eWorldSFX uSFXType = (eWorldSFX)AudioLoader::GetWorldSFXTypeFromStr(szSFXType);
    if ((unsigned long)(uSFXType + 0x10000) == 0xFFFF)
    {
        return -1;
    }

    SoundAttributes sndAtr;
    sndAtr.Init();
    sndAtr.mu_Type = uSFXType;
    sndAtr.mb_Is3D = false;
    sndAtr.mf_Volume = fVol;
    sndAtr.mf_DelayTime = fDelay;
    sndAtr.ms_EventName = sfxEventData.eventName;
    sndAtr.mi_GroupPriority = sfxEventData.eventPriority;

    char* pdest = strstr(szSFXType, "CROWDSFX");
    int loc = pdest - szSFXType;
    if (pdest != NULL && loc == 0)
    {
        return gCrowdSFX.Play(sndAtr);
    }

    pdest = strstr(szSFXType, "NISSFX");
    loc = pdest - szSFXType;
    if (pdest != NULL && loc == 0)
    {
        return gWorldSFX.Play(sndAtr);
    }

    pdest = strstr(szSFXType, "BALLSFX");
    loc = pdest - szSFXType;
    if (pdest != NULL && loc == 0)
    {
        return gStadGenSFX.Play(sndAtr);
    }

    pdest = strstr(szSFXType, "STADSFX");
    loc = pdest - szSFXType;
    if (pdest != NULL && loc == 0)
    {
        return gStadGenSFX.Play(sndAtr);
    }

    pdest = strstr(szSFXType, "WORLDSFX");
    loc = pdest - szSFXType;
    if (pdest != NULL && loc == 0)
    {
        return gWorldSFX.Play(sndAtr);
    }

    pdest = strstr(szSFXType, "PWRUPSFX");
    loc = pdest - szSFXType;
    if (pdest != NULL && loc == 0)
    {
        return gPowerupSFX.Play(sndAtr);
    }

    return gWorldSFX.Play(sndAtr);
}

/**
 * Offset/Address/Size: 0x25C0 | 0x8013EAD4 | size: 0x68
 */
void StopCharSFXbyStr(const char* szSFXType, NisCharacterClass charIdentifier)
{
    if (!g_bAudioInitialized)
        return;
    Audio::eCharSFX sfxType = (Audio::eCharSFX)AudioLoader::GetCharSFXTypeFromStr(szSFXType);
    if (g_pGame == NULL)
        return;
    cCharacter* character = Audio::cCharacterSFX::GetCharacterFromNisCharClass(charIdentifier);
    if (character == NULL)
        return;
    character->StopSFX(sfxType);
}

/**
 * Offset/Address/Size: 0x2628 | 0x8013EB3C | size: 0xB4
 */
void StopWorldSFXbyStr(const char* szSFXType)
{
    if (!g_bAudioInitialized)
        return;
    unsigned long type = AudioLoader::GetWorldSFXTypeFromStr(szSFXType);
    if (!g_bWorldSFXInitialized)
        return;

    if ((int)type < 94)
    {
        gWorldSFX.Stop((eWorldSFX)type, cGameSFX::SFX_STOP_FIRST);
    }
    else if ((int)type < 146)
    {
        gPowerupSFX.Stop((eWorldSFX)type, cGameSFX::SFX_STOP_FIRST);
    }
    else if ((int)type < 177)
    {
        gCrowdSFX.Stop((eWorldSFX)type, cGameSFX::SFX_STOP_FIRST);
    }
    else if ((int)type < 211)
    {
        gStadGenSFX.Stop((eWorldSFX)type, cGameSFX::SFX_STOP_FIRST);
    }
}

/**
 * Offset/Address/Size: 0x26DC | 0x8013EBF0 | size: 0x104
 */
int PlayCharSFXbyStr(const char* szSFXType, NisCharacterClass charIdentifier, float fVol, float fDelay, bool bIs3D, bool bKeepTrack, const nlVector3* pInitialPosVector, const nlVector3* pInitialDirVector, unsigned long* unkPtr)
{
    if (!g_bAudioInitialized)
    {
        return -1;
    }

    unsigned long sfxType = AudioLoader::GetCharSFXTypeFromStr(szSFXType);

    if (unkPtr != NULL)
    {
        *unkPtr = sfxType;
    }

    if (g_pGame == NULL)
    {
        return -1;
    }

    SoundAttributes sa;
    sa.Init();
    sa.mu_Type = sfxType;
    sa.mb_Is3D = bIs3D;
    sa.mf_Volume = fVol;
    sa.mf_DelayTime = fDelay;
    sa.mb_KeepTrack = bKeepTrack;

    if (bIs3D)
    {
        sa.mb_Is3D = bIs3D;
        sa.pos.pvPos = pInitialPosVector;
        sa.dir.pvDir = pInitialDirVector;
        sa.posUpdateMethod = PTRS_TO_VECTORS;
        sa.mb_Update3DContinuously = true;
        sa.mf_ReturnEmitterOnPlay = true;
    }

    cCharacter* character = cCharacterSFX::GetCharacterFromNisCharClass(charIdentifier);
    if (character == NULL)
    {
        return -1;
    }

    return character->m_pCharacterSFX->Play(sa);
}

/**
 * Offset/Address/Size: 0x27E0 | 0x8013ECF4 | size: 0x144
 */

unsigned long PlayWorldSFXbyStr(const char* szSFXType, float fVol, float fDelay, bool bIs3D, bool bKeepTrack, const nlVector3* pInitialPosVector, const nlVector3* pInitialDirVector, unsigned long* pType)
{
    if (!g_bAudioInitialized)
    {
        return (unsigned long)-1;
    }

    eWorldSFX type = (eWorldSFX)AudioLoader::GetWorldSFXTypeFromStr(szSFXType);

    if (pType != NULL)
    {
        *pType = type;
    }

    if (!g_bWorldSFXInitialized)
    {
        return (unsigned long)-1;
    }

    SoundAttributes attr;
    attr.Init();
    attr.mu_Type = type;
    attr.mb_Is3D = bIs3D;
    attr.mf_Volume = fVol;
    attr.mf_DelayTime = fDelay;
    attr.mb_KeepTrack = bKeepTrack;

    if (bIs3D)
    {
        attr.mb_Is3D = bIs3D;
        attr.pos.pvPos = pInitialPosVector;
        attr.dir.pvDir = pInitialDirVector;
        attr.posUpdateMethod = PTRS_TO_VECTORS;
        attr.mb_Update3DContinuously = true;
        attr.mf_ReturnEmitterOnPlay = true;
    }

    if (type < 0x5E)
    {
        return gWorldSFX.Play(attr);
    }
    else if (type < 0x92)
    {
        return gPowerupSFX.Play(attr);
    }
    else if (type < 0xB1)
    {
        return gCrowdSFX.Play(attr);
    }
    else if (type < 0xD3)
    {
        return gStadGenSFX.Play(attr);
    }

    return (unsigned long)-1;
}

/**
 * Offset/Address/Size: 0x2924 | 0x8013EE38 | size: 0x30
 */
void RemoveDelayedSFX(unsigned long index)
{
    gDelayedSFX[index].Init();
}

/**
 * Offset/Address/Size: 0x2954 | 0x8013EE68 | size: 0xE8
 */
int IsDelayedCharSFX(unsigned long sfxType, cGameSFX* pOwner)
{
    for (int i = 0; i < 15; i++)
    {
        if (gDelayedSFX[i].mu_Type == sfxType && gDelayedSFX[i].mp_OwnerSFX == pOwner)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Offset/Address/Size: 0x2A3C | 0x8013EF50 | size: 0x3E4
 */
int AddDelayedSFX(const SoundAttributes& sfxData, unsigned long uSFXID, float volume, float delay, cGameSFX* pOwnerSFX)
{
    if (!g_bAudioInitialized)
    {
        return -1;
    }

    if (uSFXID == (unsigned long)-1)
        goto error;

    {
        int slot = -1;
        for (int i = 0; i < 15; i++)
        {
            if (gDelayedSFX[i].mu_Type == (unsigned long)-1)
            {
                slot = i;
                break;
            }
        }

        if (slot == -1)
        {
            tDebugPrintManager::Print(DC_SOUND, "Too many delayed sfx - finding oldest slot and killing it\n");

            float min = gDelayedSFX[0].mf_DebugTimer;
            slot = 0;
            for (int i = 1; i < 15; i++)
            {
                if (gDelayedSFX[i].mf_DebugTimer < min)
                {
                    min = gDelayedSFX[i].mf_DebugTimer;
                    slot = i;
                }
            }
        }

        if (slot < 0)
            goto error;

        gDelayedSFX[slot].Init();
        gDelayedSFX[slot].me_ClassType = sfxData.me_ClassType;
        gDelayedSFX[slot].mu_Type = sfxData.mu_Type;
        gDelayedSFX[slot].mu_SfxID = sfxData.mu_SfxID;
        gDelayedSFX[slot].mu_VoiceID = sfxData.mu_VoiceID;
        gDelayedSFX[slot].mf_Volume = sfxData.mf_Volume;
        gDelayedSFX[slot].mf_VolReverb = sfxData.mf_VolReverb;
        gDelayedSFX[slot].mf_Attenuate = sfxData.mf_Attenuate;
        gDelayedSFX[slot].mf_VolAdjustment = sfxData.mf_VolAdjustment;
        gDelayedSFX[slot].mf_Panning = sfxData.mf_Panning;
        gDelayedSFX[slot].mf_DelayTime = sfxData.mf_DelayTime;
        gDelayedSFX[slot].mf_DebugTimer = sfxData.mf_DebugTimer;
        gDelayedSFX[slot].mb_Is3D = sfxData.mb_Is3D;
        gDelayedSFX[slot].mb_IsPlaying = sfxData.mb_IsPlaying;
        gDelayedSFX[slot].mb_KeepTrack = sfxData.mb_KeepTrack;
        gDelayedSFX[slot].mb_HasCutoff = sfxData.mb_HasCutoff;
        gDelayedSFX[slot].mb_Update3DContinuously = sfxData.mb_Update3DContinuously;
        gDelayedSFX[slot].mb_Pausable = sfxData.mb_Pausable;
        gDelayedSFX[slot].mb_Restartable = sfxData.mb_Restartable;
        gDelayedSFX[slot].mb_UseDoppler = sfxData.mb_UseDoppler;
        gDelayedSFX[slot].mf_ReturnEmitterOnPlay = sfxData.mf_ReturnEmitterOnPlay;
        gDelayedSFX[slot].mf_CutoffTime = sfxData.mf_CutoffTime;
        gDelayedSFX[slot].mp_OwnerSFX = sfxData.mp_OwnerSFX;
        gDelayedSFX[slot].mp_PhysObj = sfxData.mp_PhysObj;
        gDelayedSFX[slot].pos.vPos.as_u32[0] = sfxData.pos.vPos.as_u32[0];
        gDelayedSFX[slot].pos.vPos.as_u32[1] = sfxData.pos.vPos.as_u32[1];
        gDelayedSFX[slot].pos.vPos.as_u32[2] = sfxData.pos.vPos.as_u32[2];
        gDelayedSFX[slot].dir.vDir.as_u32[0] = sfxData.dir.vDir.as_u32[0];
        gDelayedSFX[slot].dir.vDir.as_u32[1] = sfxData.dir.vDir.as_u32[1];
        gDelayedSFX[slot].dir.vDir.as_u32[2] = sfxData.dir.vDir.as_u32[2];
        gDelayedSFX[slot].posUpdateMethod = sfxData.posUpdateMethod;
        gDelayedSFX[slot].ms_EventName = sfxData.ms_EventName;
        gDelayedSFX[slot].mi_SFXPriority = sfxData.mi_SFXPriority;
        gDelayedSFX[slot].mi_GroupPriority = sfxData.mi_GroupPriority;
        gDelayedSFX[slot].mi_VolGroup = sfxData.mi_VolGroup;
        gDelayedSFX[slot].mi_EmitterGroup = sfxData.mi_EmitterGroup;
        gDelayedSFX[slot].mb_FilterOn = sfxData.mb_FilterOn;
        gDelayedSFX[slot].mu_FilterFreq = sfxData.mu_FilterFreq;
        gDelayedSFX[slot].mu_Pitch = sfxData.mu_Pitch;
        gDelayedSFX[slot].mb_NoPhasingFilter = sfxData.mb_NoPhasingFilter;
        gDelayedSFX[slot].m_unk_0x7B = sfxData.m_unk_0x7B;
        gDelayedSFX[slot].m_unk_0x7C = sfxData.m_unk_0x7C;

        if (pOwnerSFX != NULL)
        {
            gDelayedSFX[slot].mp_OwnerSFX = pOwnerSFX;
        }

        if (sfxData.mf_CutoffTime >= 0.0f)
        {
            gDelayedSFX[slot].mb_HasCutoff = true;
        }

        if (g_pGame != NULL)
        {
            gDelayedSFX[slot].mf_DebugTimer = g_pGame->GetGameTime();
        }
        else
        {
            gDelayedSFX[slot].mf_DebugTimer = g_fAudioTimer;
        }

        return uSFXID;
    }

error:
    return -1;
}

/**
 * Offset/Address/Size: 0x2E20 | 0x8013F334 | size: 0x17C
 */
unsigned long PlaySFXbyID(const SoundAttributes& attrs, unsigned long sfxID, float fVol, float fRevVol, int volGroup)
{
    if (!g_bAudioInitialized)
    {
        return (unsigned long)-1;
    }

    if (sfxID != (unsigned long)-1)
    {
        if (attrs.mf_DelayTime <= 0.0f)
        {
            if (volGroup > -1)
            {
                if (volGroup < 0)
                {
                    PlatAudio::SetSFXVolumeGroup(sfxID, 0);
                }
                else if (volGroup > 255)
                {
                    PlatAudio::SetSFXVolumeGroup(sfxID, 255);
                }
                else
                {
                    PlatAudio::SetSFXVolumeGroup(sfxID, (unsigned char)volGroup);
                }
            }

            SFXStartInfo info;
            info.uSFXID = (unsigned long)-1;
            info.fVolume = 0.0f;
            info.fPan = 0.0f;
            info.fVolReverb = 0.0f;
            info.uSurroundPan = 0xFF;
            info.uPitchBend = 0x2000;
            info.bActivateFilter = false;
            info.filterFreq = 0;
            info.uModulation = 0;
            info.uDoppler = 0x2000;

            info.uSFXID = sfxID;
            info.fVolume = fVol;
            info.fPan = attrs.mf_Panning;
            info.fVolReverb = fRevVol;
            info.bActivateFilter = attrs.mb_FilterOn;
            info.filterFreq = attrs.mu_FilterFreq;
            info.uPitchBend = attrs.mu_Pitch;

            unsigned long voiceID = PlatAudio::PlaySFX(info);

            if (attrs.mb_KeepTrack)
            {
                attrs.mp_OwnerSFX->KeepTrack(0, attrs, voiceID);
            }

            return voiceID;
        }

        AddDelayedSFX(attrs, sfxID, fVol, fRevVol, (cGameSFX*)0);
    }

    return (unsigned long)-1;
}

/**
 * Offset/Address/Size: 0x2F9C | 0x8013F4B0 | size: 0x20
 */
unsigned long PlaySFX(const SFXStartInfo& info)
{
    return PlatAudio::PlaySFX(info);
}

/**
 * Offset/Address/Size: 0x2FBC | 0x8013F4D0 | size: 0x8
 */
float GetAudioTimer()
{
    return g_fAudioTimer;
}

/**
 * Offset/Address/Size: 0x2FC4 | 0x8013F4D8 | size: 0x44
 */
void Shutdown()
{
    if (g_bAudioInitialized)
    {
        if (PlatAudio::IsStreamingInited())
        {
            PlatAudio::ShutdownStreaming();
        }

        PlatAudio::Shutdown();
        g_bAudioInitialized = false;
    }
}

/**
 * Offset/Address/Size: 0x3008 | 0x8013F51C | size: 0x20
 */
void Silence()
{
    PlatAudio::StopAllSound();
}

/**
 * Offset/Address/Size: 0x3028 | 0x8013F53C | size: 0x48
 */
void ResetForNewGame()
{
    gbGameIsPaused = false;
    gbStartingGame = true;
    g_bHomeTeamHasJustScored = false;
    gChantDelayTimer = 0.0f;
    nlDeleteList<FadeAudioData>(&g_pFadeList);
    g_pFadeList = NULL;
}

/**
 * Offset/Address/Size: 0x3070 | 0x8013F584 | size: 0xC
 */
void ResetPauseStatus()
{
    gbGameIsPaused = false;
}

/**
 * Offset/Address/Size: 0x307C | 0x8013F590 | size: 0x64
 */
void UnloadWorldSFX()
{
    if (!g_bWorldSFXInitialized)
        return;

    gWorldSFX.ShutdownPlaySet();
    gPowerupSFX.ShutdownPlaySet();
    gStadGenSFX.ShutdownPlaySet();
    gCrowdSFX.ShutdownPlaySet();
    gbGameIsPaused = false;
    g_bWorldSFXInitialized = false;
}

/**
 * Offset/Address/Size: 0x30E0 | 0x8013F5F4 | size: 0x8
 */
bool IsWorldSFXLoaded()
{
    return g_bWorldSFXInitialized;
}

/**
 * Offset/Address/Size: 0x30E8 | 0x8013F5FC | size: 0xC0
 */
void LoadWorldSFX()
{
    if (g_bWorldSFXInitialized)
        return;

    gWorldSFX.Init();
    gPowerupSFX.Init();
    gStadGenSFX.Init();

    if (!AudioLoader::gbDisableCrowd)
        gCrowdSFX.Init();

    gWorldSFX.SetSFX(gpWORLDSoundPropAccessor);
    gPowerupSFX.SetSFX(gpPWRUPSoundPropAccessor);
    gStadGenSFX.SetSFX(gpSTADGENSoundPropAccessor);

    if (!AudioLoader::gbDisableCrowd)
        gCrowdSFX.SetSFX(gpCROWDSoundPropAccessor);

    gbGameIsPaused = false;
    g_bWorldSFXInitialized = true;
}

/**
 * Offset/Address/Size: 0x31A8 | 0x8013F6BC | size: 0xB0
 */
void UnloadInGameSFX()
{
    for (int i = 0; i < 64; i++)
    {
        PlatAudio::RemoveEmitter(i);
        PlatAudio::InitEmitter(i);
    }

    for (int i = 0; i < 15; i++)
    {
        gDelayedSFX[i].Init();
    }

    gbStartingGame = true;
    g_bHomeTeamHasJustScored = false;

    if (gbListenerInit)
    {
        PlatAudio::Remove3DSFXListener(&gListener);
        gbListenerInit = false;
    }

    g_bAudioInGameLoaded = false;
    g_fAudioTimer = 0.0f;
}

/**
 * Offset/Address/Size: 0x3258 | 0x8013F76C | size: 0x80
 */
void LoadInGameSFX()
{
    for (int i = 0; i < 64; i++)
    {
        PlatAudio::InitEmitter((unsigned long)i);
    }

    for (int j = 0; j < 15; j++)
    {
        gDelayedSFX[j].Init();
    }

    g_bHomeTeamHasJustScored = false;
    g_fAudioTimer = 0.0f;
    g_bAudioInGameLoaded = true;
}

/**
 * Offset/Address/Size: 0x32D8 | 0x8013F7EC | size: 0x8
 */
bool IsInited()
{
    return g_bAudioInitialized;
}

/**
 * Offset/Address/Size: 0x32E0 | 0x8013F7F4 | size: 0x9C
 */
bool ShutdownReverb()
{
    long long currTime = OSGetTime();
    nlPrintf("Audio::ShutdownReverb(), turning reverb off at %d\n", currTime);

    if (!AudioLoader::gReverbOn)
    {
        nlPrintf("Audio::ShutdownReverb(), gReverbOn should never be off.\n");
        return false;
    }

    nlPrintf("Audio::ShutdownReverb(), now shutting reverb down...\n");
    if (!PlatAudio::ShutdownAuxEffectA())
    {
        nlPrintf("Audio::ShutdownReverb(), PlatAudio::ShutdownAuxEffectA() returned false.\n");
        return false;
    }

    AudioLoader::gReverbOn = false;
    return true;
}

/**
 * Offset/Address/Size: 0x337C | 0x8013F890 | size: 0x734
 */
// void InitializeReverb(eStadiumID, unsigned char)
// {
// }

/**
 * Offset/Address/Size: 0x3AB0 | 0x8013FFC4 | size: 0xF60
 */
// void ReadVolGroupSettings()
// {
// }

/**
 * Offset/Address/Size: 0x4A10 | 0x80140F24 | size: 0x15C
 */
bool Initialize(bool bInit)
{
    if (!PlatAudio::Initialize(bInit))
    {
        return false;
    }

    if (g_pTrackManager == NULL)
    {
        CreateTrackMgr<3>();
    }

    for (int i = 0; i < 15; i++)
    {
        gDelayedSFX[i].Init();
    }

    static bool bAlreadySetupSoundAVLTrees = false;

    if (!bAlreadySetupSoundAVLTrees)
    {
        for (int i = 0; i < 211; i++)
        {
            gWorldSoundTypeEnumMap[i] = -1;
            gWorldSFXInfo[i].typeID = (unsigned long)-1;
            gWorldSFXInfo[i].typeStr = NULL;
            gWorldSFXInfo[i].musyxStr = NULL;
            gWorldSFXInfo[i].musyxID = (unsigned long)-1;
            gWorldSFXInfo[i].fVolume = 100.0f;
            gWorldSFXInfo[i].fDelay = -1.0f;
            gWorldSFXInfo[i].fVolReverb = 100.0f;
            gWorldSFXInfo[i].volGrp = -1;
            gWorldSFXInfo[i].sfxPriority = 0;
            gWorldSFXInfo[i].uHashVal = 0;
            gWorldSFXInfo[i].pSoundPropAccessor = NULL;
            gWorldSFXInfo[i].bSoundPropTableReloaded = 0;
            gWorldSFXInfo[i].pSoundProp = NULL;
            gWorldSFXInfo[i].pOwner = NULL;
            gWorldSFXInfo[i].lastVoiceID = (unsigned long)-1;
            gWorldSFXInfo[i].pLastEmitter = NULL;
            gWorldSFXInfo[i].m_unk_0x40 = false;
            gWorldSFXInfo[i].typeID = i;
        }

        AudioLoader::SetupSoundDefinesAVLTree();
        AudioLoader::SetupCharSoundTypesAVLTree();
        AudioLoader::SetupWorldSoundTypesAVLTree();
        AudioLoader::SetupSoundGroups();
        bAlreadySetupSoundAVLTrees = true;
    }

    ReadVolGroupSettings();
    Config::Global().LoadFromFile("audio/CrowdScript.ini");
    g_bAudioInitialized = true;
    return true;
}

/**
 * Offset/Address/Size: 0x4B6C | 0x80141080 | size: 0x2C
 */
void SoundAttributes::UseStationaryPosVector(const nlVector3& position)
{
    pos.vPos.as_u32[0] = position.as_u32[0];
    pos.vPos.as_u32[1] = position.as_u32[1];
    pos.vPos.as_u32[2] = position.as_u32[2];
    posUpdateMethod = VECTORS;
    mb_Update3DContinuously = true;
}

/**
 * Offset/Address/Size: 0x4B98 | 0x801410AC | size: 0x44
 */
void SoundAttributes::UseVectors(const nlVector3& v1, const nlVector3& v2)
{
    pos.vPos.as_u32[0] = v1.as_u32[0];
    pos.vPos.as_u32[1] = v1.as_u32[1];
    pos.vPos.as_u32[2] = v1.as_u32[2];
    dir.vDir.as_u32[0] = v2.as_u32[0];
    dir.vDir.as_u32[1] = v2.as_u32[1];
    dir.vDir.as_u32[2] = v2.as_u32[2];
    posUpdateMethod = VECTORS;
    mb_Update3DContinuously = true;
}

/**
 * Offset/Address/Size: 0x4BDC | 0x801410F0 | size: 0x1C
 */
void SoundAttributes::UseVectorPtrs(const nlVector3* v1, const nlVector3* v2)
{
    // *(const nlVector3**)&m_unk_0x44 = v1;
    // *(const nlVector3**)&m_unk_0x50 = v2;
    pos.pvPos = v1;
    dir.pvDir = v2;
    posUpdateMethod = PTRS_TO_VECTORS;
    mb_Update3DContinuously = true;
}

/**
 * Offset/Address/Size: 0x4BF8 | 0x8014110C | size: 0x14
 */
void SoundAttributes::UsePhysObj(PhysicsObject* obj)
{
    mp_PhysObj = obj;
    posUpdateMethod = PHYSOBJ;
    mb_Update3DContinuously = true;
}

/**
 * Offset/Address/Size: 0x4C0C | 0x80141120 | size: 0xC
 */
void SoundAttributes::SetSoundType(unsigned long soundType, bool bIs3D)
{
    mu_Type = soundType;
    mb_Is3D = bIs3D;
}

/**
 * Offset/Address/Size: 0x4C18 | 0x8014112C | size: 0x100
 */
void SoundAttributes::Init()
{
    me_ClassType = 0;
    mu_Type = -1;
    mu_SfxID = -1;
    mu_VoiceID = PlatAudio::GetSndIDError();

    mf_Volume = 1.0f;
    mf_VolReverb = 1.0f;
    mf_Attenuate = 0.0f;
    mf_VolAdjustment = 256.0f;
    mf_Panning = 1.0f;
    mf_DelayTime = 0.5f;
    mf_DebugTimer = 256.0f;

    mb_Is3D = false;
    mb_IsPlaying = false;
    mb_KeepTrack = true;
    mb_HasCutoff = false;
    mb_Update3DContinuously = false;
    mb_Pausable = false;
    mb_Restartable = false;
    mb_UseDoppler = false;
    mf_ReturnEmitterOnPlay = false;

    mf_CutoffTime = 0.5f;
    mp_OwnerSFX = NULL;
    mp_PhysObj = NULL;

    pos.pvPos = NULL;
    dir.pvDir = NULL;

    pos.vPos.f.x = 256.0f;
    pos.vPos.f.y = 256.0f;
    pos.vPos.f.z = 256.0f;
    dir.vDir.f.x = 256.0f;
    dir.vDir.f.y = 256.0f;
    dir.vDir.f.z = 256.0f;

    posUpdateMethod = NONE;
    ms_EventName = 0;
    mi_SFXPriority = 0;
    mi_GroupPriority = -1;
    mi_VolGroup = -1;
    mi_EmitterGroup = 0;

    mb_FilterOn = false;
    mu_FilterFreq = 0;
    mu_Pitch = 0x2000;
    mb_NoPhasingFilter = true;
    m_unk_0x7B = false;
    m_unk_0x7C = true;
}

/**
 * Offset/Address/Size: 0x0 | 0x8014122C | size: 0x8
 */
eClassType cGameSFX::GetClassType() const
{
    return meClassType;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80141234 | size: 0x8C
//  */
// void nlBSearch<nlSortedSlot<AudioStreamTrack::StreamTrack, 3>::EntryLookup<AudioStreamTrack::StreamTrack>, unsigned long>(const unsigned
// long&, nlSortedSlot<AudioStreamTrack::StreamTrack, 3>::EntryLookup<AudioStreamTrack::StreamTrack>*, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801412C0 | size: 0x54
//  */
// void nlDeleteList<FadeAudioData>(FadeAudioData**)
// {
// }

// /**
//  * Offset/Address/Size: 0x54 | 0x80141314 | size: 0x9C
//  */
// void nlListRemoveElement<FadeAudioData>(FadeAudioData**, FadeAudioData*, FadeAudioData**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801413D8 | size: 0x3C
//  */
// void nlWalkDLRing<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>>>(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*,
// DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>>::*)(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x80141414 | size: 0x3C
//  */
// void nlWalkDLRing<DLListEntry<GCAudioStreaming::StereoAudioStream*>, DLListContainerBase<GCAudioStreaming::StereoAudioStream*,
// BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>>(DLListEntry<GCAudioStreaming::StereoAudioStream*>*,
// DLListContainerBase<GCAudioStreaming::StereoAudioStream*, BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>*, void
// (DLListContainerBase<GCAudioStreaming::StereoAudioStream*,
// BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>::*)(DLListEntry<GCAudioStreaming::StereoAudioStream*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x78 | 0x80141450 | size: 0x3C
//  */
// void nlWalkDLRing<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>,
// DLListContainerBase<AudioStreamTrack::StreamTrack::QUEUED_STREAM,
// nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>,
// 4>>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*, DLListContainerBase<AudioStreamTrack::StreamTrack::QUEUED_STREAM,
// nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>, 4>>*, void
// (DLListContainerBase<AudioStreamTrack::StreamTrack::QUEUED_STREAM,
// nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>,
// 4>>::*)(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xB4 | 0x8014148C | size: 0x3C
//  */
// void nlWalkDLRing<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// WalkHelper<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// AudioStreamTrack::TrackManagerBase::FadeManager>>(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*,
// WalkHelper<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>, AudioStreamTrack::TrackManagerBase::FadeManager>*, void
// (WalkHelper<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// AudioStreamTrack::TrackManagerBase::FadeManager>::*)(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x110 | 0x801414E8 | size: 0x18
//  */
// void
// nlDLRingGetStart<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x128 | 0x80141500 | size: 0x18
//  */
// DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*
// nlDLRingGetStart<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>(
//     DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80141518 | size: 0x1AC
//  */
// void CreateTrackMgr<3>()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801416C4 | size: 0x1F4
//  */
// void AudioStreamTrack::TrackManagerBase::~TrackManagerBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x1F4 | 0x801418B8 | size: 0x204
//  */
// void AudioStreamTrack::TrackManager<3>::~TrackManager()
// {
// }

// /**
//  * Offset/Address/Size: 0x3F8 | 0x80141ABC | size: 0x1EC
//  */
// void AudioStreamTrack::TrackManager<3>::OnMasterVolumeChange(MasterVolume::VOLUME_GROUP)
// {
// }

// /**
//  * Offset/Address/Size: 0x5E4 | 0x80141CA8 | size: 0x54
//  */
// void AudioStreamTrack::TrackManager<3>::GetTrack(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x638 | 0x80141CFC | size: 0x78
//  */
// void AudioStreamTrack::TrackManager<3>::StopAllTracks(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x6B0 | 0x80141D74 | size: 0xF0
//  */
// void AudioStreamTrack::TrackManager<3>::Update(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x7A0 | 0x80141E64 | size: 0x1D0
//  */
// void AudioStreamTrack::TrackManager<3>::DestroyAllTracks()
// {
// }

// /**
//  * Offset/Address/Size: 0x970 | 0x80142034 | size: 0x1AC
//  */
// void AudioStreamTrack::TrackManager<3>::CreateTrack(const char*, MasterVolume::VOLUME_GROUP)
// {
// }

template class nlStaticSortedSlot<AudioStreamTrack::StreamTrack, 3>;

// /**
//  * Offset/Address/Size: 0x0 | 0x80142234 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<GCAudioStreaming::StereoAudioStream*>, DLListContainerBase<GCAudioStreaming::StereoAudioStream*,
// BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>>(DLListEntry<GCAudioStreaming::StereoAudioStream*>*,
// DLListContainerBase<GCAudioStreaming::StereoAudioStream*, BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>*, void
// (DLListContainerBase<GCAudioStreaming::StereoAudioStream*,
// BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>::*)(DLListEntry<GCAudioStreaming::StereoAudioStream*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x60 | 0x80142294 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>>>(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*,
// DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>>*, void
// (DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>>::*)(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xC0 | 0x801422F4 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// WalkHelper<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// AudioStreamTrack::TrackManagerBase::FadeManager>>(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*,
// WalkHelper<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>, AudioStreamTrack::TrackManagerBase::FadeManager>*, void
// (WalkHelper<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL,
// DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>,
// AudioStreamTrack::TrackManagerBase::FadeManager>::*)(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x120 | 0x80142354 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>,
// DLListContainerBase<AudioStreamTrack::StreamTrack::QUEUED_STREAM,
// nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>,
// 4>>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*, DLListContainerBase<AudioStreamTrack::StreamTrack::QUEUED_STREAM,
// nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>, 4>>*, void
// (DLListContainerBase<AudioStreamTrack::StreamTrack::QUEUED_STREAM,
// nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>,
// 4>>::*)(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*))
// {
// }

} // namespace Audio

/**
 * Offset/Address/Size: 0xF0 | 0x801414C8 | size: 0x20
 */
template bool nlDLRingIsEnd<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL> >(
    DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*,
    DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*);

/**
 * Offset/Address/Size: 0x128 | 0x80141500 | size: 0x18
 */
template DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*
nlDLRingGetStart(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*);
