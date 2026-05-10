#include "Game/Audio/SoundEventScript.h"

#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/AudioStream.h"
#include "Game/Audio/CrowdMood.h"
#include "Game/Audio/PriorityStream.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/Sys/audio.h"
#include "NL/nlDebug.h"

/**
 * Offset/Address/Size: 0x0 | 0x80153FD8 | size: 0x96C
 * TODO: 98.67% match - register allocation differences in cases 4, 5, 13, 14, 16
 */
void SoundEventScript::DoFunctionCall(unsigned int func)
{
    switch (func)
    {
    case 0:
    {
        m_SP--;
        unsigned long uIntensity = *m_SP;
        m_SP--;
        CrowdMood::CROWD_MOOD mood = (CrowdMood::CROWD_MOOD)*m_SP;
        CrowdMood::AdjustMood(mood, uIntensity);
        break;
    }
    case 1:
    {
        m_SP--;
        bool enabled = *m_SP != 0;
        CrowdMood::EnableCrowdDecay(enabled);
        break;
    }
    case 2:
    {
        CrowdMood::InitiateFastCrowdTransition();
        break;
    }
    case 3:
    {
        m_SP--;
        m_SP--;
        break;
    }
    case 4:
    {
        m_SP--;
        unsigned long fadeIn = *m_SP;
        m_SP--;
        unsigned long fadeOut = *m_SP;
        m_SP--;
        unsigned long loop = *m_SP;
        m_SP--;
        float vol = *(float*)m_SP;
        m_SP--;
        int streamSelect = *m_SP;

        unsigned long streamId;
        switch (streamSelect)
        {
        case 0:
            streamId = 0xE38B5407;
            break;
        case 1:
            streamId = 0x436E3953;
            break;
        case 2:
            streamId = 0x57CB5A12;
            break;
        case 3:
            streamId = 0x09451A58;
            break;
        case 4:
            streamId = 0xA207B1AE;
            break;
        default:
            streamId = 0xFFFFFFFF;
            break;
        }

        Audio::GetPriorityStream()->PlayStream(streamId, vol, loop != 0, fadeOut, fadeIn, NULL);
        break;
    }
    case 5:
    {
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        unsigned long fadeIn = *m_SP;
        m_SP--;
        unsigned long fadeOut = *m_SP;
        m_SP--;
        unsigned long loop = *m_SP;
        m_SP--;
        float vol = *(float*)m_SP;
        m_SP--;
        int streamSelect = *m_SP;

        unsigned long streamId;
        switch (streamSelect)
        {
        case 0:
            streamId = 0xE38B5407;
            break;
        case 1:
            streamId = 0x436E3953;
            break;
        case 2:
            streamId = 0x57CB5A12;
            break;
        case 3:
            streamId = 0x09451A58;
            break;
        case 4:
            streamId = 0xA207B1AE;
            break;
        default:
            streamId = 0xFFFFFFFF;
            break;
        }

        Audio::GetPriorityStream()->PlayStream(streamId, vol, loop != 0, fadeOut, fadeIn, name);
        break;
    }
    case 6:
    {
        m_SP--;
        int range = *m_SP;
        m_SP--;
        float fVol = *(float*)m_SP;
        m_SP--;
        float fDelay = *(float*)m_SP;
        if (Audio::gCrowdSFX.mbInited)
        {
            if (fVol >= 0.0f)
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_BOO_BIG, fVol, fDelay, range, 0.0f);
            }
            else
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_BOO_BIG, 100.0f, fDelay, range, 0.0f);
            }
        }
        break;
    }
    case 7:
    {
        m_SP--;
        int range = *m_SP;
        m_SP--;
        float fVol = *(float*)m_SP;
        m_SP--;
        float fDelay = *(float*)m_SP;
        if (Audio::gCrowdSFX.mbInited)
        {
            if (fVol >= 0.0f)
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_BOO_SMALL, fVol, fDelay, range, 0.0f);
            }
            else
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_BOO_SMALL, 100.0f, fDelay, range, 0.0f);
            }
        }
        break;
    }
    case 8:
    {
        m_SP--;
        int range = *m_SP;
        m_SP--;
        float fVol = *(float*)m_SP;
        m_SP--;
        float fDelay = *(float*)m_SP;
        if (Audio::gCrowdSFX.mbInited)
        {
            if (fVol >= 0.0f)
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_OH_BIG, fVol, fDelay, range, 0.0f);
            }
            else
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_OH_BIG, 100.0f, fDelay, range, 0.0f);
            }
        }
        break;
    }
    case 9:
    {
        m_SP--;
        int range = *m_SP;
        m_SP--;
        float fVol = *(float*)m_SP;
        m_SP--;
        float fDelay = *(float*)m_SP;
        if (Audio::gCrowdSFX.mbInited)
        {
            if (fVol >= 0.0f)
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_OH_SMALL, fVol, fDelay, range, 0.0f);
            }
            else
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_OH_SMALL, 100.0f, fDelay, range, 0.0f);
            }
        }
        break;
    }
    case 10:
    {
        m_SP--;
        int range = *m_SP;
        m_SP--;
        float fVol = *(float*)m_SP;
        m_SP--;
        float fDelay = *(float*)m_SP;
        if (Audio::gCrowdSFX.mbInited)
        {
            if (fVol >= 0.0f)
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_YEAH_SMALL, fVol, fDelay, range, 0.0f);
            }
            else
            {
                Audio::gCrowdSFX.PlayRandomReaction(Audio::cWorldSFX::CROWD_REACTION_YEAH_SMALL, 100.0f, fDelay, range, 0.0f);
            }
        }
        break;
    }
    case 11:
    {
        SoundEventData data;
        m_SP--;
        int eventPrio = *m_SP;
        m_SP--;
        const char* sfxType = (const char*)*m_SP;
        m_SP--;
        float fDelay = *(float*)m_SP;
        data.eventName = mCurrentFunction;
        data.eventPriority = eventPrio;
        Audio::PlaySFXEventFromScript(data, sfxType, 100.0f, fDelay);
        break;
    }
    case 12:
    {
        SoundEventData data;
        m_SP--;
        int eventPrio = *m_SP;
        m_SP--;
        float fVol = *(float*)m_SP;
        m_SP--;
        const char* sfxType = (const char*)*m_SP;
        m_SP--;
        float fDelay = *(float*)m_SP;
        data.eventName = mCurrentFunction;
        data.eventPriority = eventPrio;
        Audio::PlaySFXEventFromScript(data, sfxType, fVol, fDelay);
        break;
    }
    case 13:
    {
        m_SP--;
        unsigned long fadeIn = *m_SP;
        m_SP--;
        unsigned long fadeOut = *m_SP;
        m_SP--;
        unsigned long loop = *m_SP;
        m_SP--;
        float vol = *(float*)m_SP;
        m_SP--;
        unsigned long streamId = *m_SP;
        m_SP--;
        unsigned long trackHash = *m_SP;

        Audio::MasterVolume::VOLUME_GROUP volGroup;
        AudioStreamTrack::StreamTrack* track = g_pTrackManager->GetTrack(trackHash);

        switch (trackHash)
        {
        case 0x05A165C0:
            Audio::MasterVolume::GetVolume(Audio::MasterVolume::VG_Music);
            volGroup = Audio::MasterVolume::VG_Music;
            break;
        case 0xC25BA8E8:
            Audio::MasterVolume::GetVoiceVolume();
            volGroup = Audio::MasterVolume::VG_Voice;
            break;
        case 0x78ABFED1:
            Audio::MasterVolume::GetVolume(Audio::MasterVolume::VG_SFX);
            AudioStreamTrack::TrackManagerBase* mgr = g_pTrackManager;
            track = mgr->GetTrack(nlStringLowerHash("Music"));
            volGroup = Audio::MasterVolume::VG_SFX;
            break;
        }

        track->PlayStream(streamId, vol, loop == 1, fadeOut, fadeIn, NULL, volGroup);
        break;
    }
    case 14:
    {
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        unsigned long fadeIn = *m_SP;
        m_SP--;
        unsigned long fadeOut = *m_SP;
        m_SP--;
        unsigned long loop = *m_SP;
        m_SP--;
        float vol = *(float*)m_SP;
        m_SP--;
        unsigned long streamId = *m_SP;
        m_SP--;
        unsigned long trackHash = *m_SP;

        Audio::MasterVolume::VOLUME_GROUP volGroup;
        AudioStreamTrack::StreamTrack* track = g_pTrackManager->GetTrack(trackHash);

        switch (trackHash)
        {
        case 0x05A165C0:
            Audio::MasterVolume::GetVolume(Audio::MasterVolume::VG_Music);
            volGroup = Audio::MasterVolume::VG_Music;
            break;
        case 0xC25BA8E8:
            Audio::MasterVolume::GetVoiceVolume();
            volGroup = Audio::MasterVolume::VG_Voice;
            break;
        case 0x78ABFED1:
            Audio::MasterVolume::GetVolume(Audio::MasterVolume::VG_SFX);
            AudioStreamTrack::TrackManagerBase* mgr = g_pTrackManager;
            track = mgr->GetTrack(nlStringLowerHash("Music"));
            volGroup = Audio::MasterVolume::VG_SFX;
            break;
        }

        track->PlayStream(streamId, vol, loop == 1, fadeOut, fadeIn, name, volGroup);
        break;
    }
    case 15:
    {
        m_SP--;
        unsigned long arg2 = *m_SP;
        m_SP--;
        CrowdMood::CROWD_MOOD mood = (CrowdMood::CROWD_MOOD)*m_SP;
        CrowdMood::SetMood(mood, arg2);
        break;
    }
    case 16:
    {
        m_SP--;
        unsigned long arg2 = *m_SP;
        m_SP--;
        int streamSelect = *m_SP;

        unsigned long streamId;
        switch (streamSelect)
        {
        case 0:
            streamId = 0xE38B5407;
            break;
        case 1:
            streamId = 0x436E3953;
            break;
        case 2:
            streamId = 0x57CB5A12;
            break;
        case 3:
            streamId = 0x09451A58;
            break;
        case 4:
            streamId = 0xA207B1AE;
            break;
        default:
            streamId = 0xFFFFFFFF;
            break;
        }
        Audio::GetPriorityStream()->Stop(streamId, arg2);
        break;
    }
    case 17:
    {
        m_SP--;
        unsigned long streamId = *m_SP;
        m_SP--;
        unsigned long trackHash = *m_SP;
        g_pTrackManager->GetTrack(trackHash)->Stop(streamId);
        break;
    }
    case 18:
    {
        m_SP--;
        const char* sfxName = (const char*)*m_SP;
        m_SP--;
        Audio::StopWorldSFXbyStr(sfxName);
        break;
    }
    default:
        nlBreak();
        break;
    }
}

/**
 * Offset/Address/Size: 0x118 | 0x80153F50 | size: 0x88
 */
void SoundEventScript::CreateInstance()
{
    pInstance = new (nlMalloc(0x68, 8, 0)) SoundEventScript();
}

/**
 * Offset/Address/Size: 0xC8 | 0x80153F00 | size: 0x50
 */
void SoundEventScript::DestroyInstance()
{
    delete[] pInstance->pByteCode;
    delete pInstance;
    pInstance = NULL;
}

/**
 * Offset/Address/Size: 0xC0 | 0x80153EF8 | size: 0x8
 */
SoundEventScript& SoundEventScript::Instance()
{
    return *pInstance;
}

/**
 * Offset/Address/Size: 0x60 | 0x80153E98 | size: 0x60
 */
void SoundEventScript::Call(const char* functionName)
{
    if (AudioLoader::gbDisableAudio == false)
    {
        nlStrNCpy<char>(mCurrentFunction, functionName, 0x40);
        InterpreterCore::CallFunction(nlStringHash(functionName));
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80153E38 | size: 0x60
 */
SoundEventScript::~SoundEventScript()
{
}
