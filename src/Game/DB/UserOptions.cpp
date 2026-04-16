#include "types.h"
#pragma pool_data off

#include "Game/DB/UserOptions.h"

#include "dolphin/os.h"
#include "NL/nlMemory.h"
#include "NL/nlBasicString.h"

#include "Game/AI/AiUtil.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/GameInfo.h"

extern bool g_e3_Build;

static const char* POWERUPS_DEFAULT_CONFIG_FILE = "DefaultPowerupOptions.ini";
static const char* AUDIO_DEFAULT_CONFIG_FILE = "DefaultAudioOptions.ini";

// /**
//  * Offset/Address/Size: 0xA8 | 0x801907F8 | size: 0x84
//  */
// void Config::TagValuePair::Get<BasicString<char, Detail::TempStringAllocator> >() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80190750 | size: 0xA8
//  */
// void Config::Get<BasicString<char, Detail::TempStringAllocator> >(const char*, BasicString<char, Detail::TempStringAllocator>)
// {
// }

/**
 * Offset/Address/Size: 0xFE8 | 0x801906F4 | size: 0x5C
 */
AudioSettings::AudioSettings()
{
    memset(this, 0, 0x20);
    MusicVolume = 0xA;
    SFXVolume = 0xA;
    VoiceVolume = 0xA;
    DefaultMusicVolume = 0xA;
    DefaultSFXVolume = 0xA;
    DefaultVoiceVolume = 0xA;
    DefaultMode = STEREO;
}

/**
 * Offset/Address/Size: 0xBB0 | 0x801902BC | size: 0x438
 * TODO: 98.37% match - r29/r30 register-allocation swap in the "undefined"
 * BasicString data construction/destruction path (data->r30/str->r29 vs
 * target data->r29/str->r30); extra mr instruction and copy-ctor reload diff.
 */
void AudioSettings::InitializeDefaults()
{
    Config cfg(Config::ALLOCATE_HIGH);
    cfg.LoadFromFile(AUDIO_DEFAULT_CONFIG_FILE);

    MusicVolume = GetConfigInt(cfg, "Music Volume", 0xA);
    SFXVolume = GetConfigInt(cfg, "SFX Volume", 0xA);
    VoiceVolume = GetConfigInt(cfg, "Voice Volume", 0xA);

    BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        const char* str = "undefined";
        const char* p = str;
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        while ((signed char)*p++ != 0)
        {
            data->mSize = data->mSize + 1;
        }

        data->mSize = data->mSize + 1;
        data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);

        s32 i = 0;
        data->mCapacity = data->mSize;
        while (i < data->mSize)
        {
            data->mData[i] = *str;
            str++;
            i++;
        }

        data->mRefCount = 1;
    }

    BasicString<char, Detail::TempStringAllocator> mode(
        cfg.Get<BasicString<char, Detail::TempStringAllocator> >(
            "Mode", BasicString<char, Detail::TempStringAllocator>(data)));

    if (mode == "STEREO")
    {
        Mode = STEREO;
    }
    else if (mode == "MONO")
    {
        Mode = MONO;
    }
    else if (mode == "DOLBY")
    {
        Mode = DOLBY;
    }
    else
    {
        Mode = OSGetSoundMode() == 0 ? MONO : STEREO;
    }
}

/**
 * Offset/Address/Size: 0xB20 | 0x8019022C | size: 0x90
 */
void AudioSettings::ForceApplySettings(bool bUpdateMode)
{
    if (AudioLoader::gbDisableAudio == false)
    {
        u32 curr_mode = OSGetSoundMode();
        u32 our_mode = Mode;

        if (curr_mode == 0)
        {
            Mode = MONO;
        }
        else
        {
            if (our_mode == (u32)MONO)
            {
                Mode = STEREO;
            }
        }

        DefaultMusicVolume = -1;
        DefaultSFXVolume = -1;
        DefaultVoiceVolume = -1;
        DefaultMode = INVALID;

        ApplySettings(true, bUpdateMode);
    }
}

/**
 * Offset/Address/Size: 0x7D0 | 0x8018FEDC | size: 0x350
 */
void AudioSettings::ApplySettings(bool bApplyMode, bool bUpdateMode)
{
    if (AudioLoader::gbDisableAudio == false)
    {
        float musicVolume = (float)MusicVolume / 10.0f;
        if (musicVolume != ((float)DefaultMusicVolume / 10.0f))
        {
            float adjustedMusicVolume = 0.0f;
            if (musicVolume != adjustedMusicVolume)
            {
                adjustedMusicVolume = Interpolate(0.2f, 1.0f, musicVolume);
            }

            musicVolume = adjustedMusicVolume;
            Audio::MasterVolume::SetVolume(Audio::MasterVolume::VG_Music, adjustedMusicVolume);
            DefaultMusicVolume = MusicVolume;
            g_pTrackManager->OnMasterVolumeChange(Audio::MasterVolume::VG_Music);
        }

        float sfxVolume = (float)SFXVolume / 10.0f;
        if (sfxVolume != ((float)DefaultSFXVolume / 10.0f))
        {
            const float sfxScale = 0.8f;
            sfxVolume *= sfxScale;
            Audio::SetVolGroupVolume(30, sfxVolume, 0);
            Audio::MasterVolume::SetVolume(Audio::MasterVolume::VG_SFX, sfxVolume);
            DefaultSFXVolume = SFXVolume;
            g_pTrackManager->OnMasterVolumeChange(Audio::MasterVolume::VG_SFX);
        }

        float voiceVolume = (float)VoiceVolume / 10.0f;
        if (voiceVolume != ((float)DefaultVoiceVolume / 10.0f))
        {
            Audio::MasterVolume::SetVoiceVolume(voiceVolume, 0);
            DefaultVoiceVolume = VoiceVolume;
            g_pTrackManager->OnMasterVolumeChange(Audio::MasterVolume::VG_Voice);
        }

        if ((Mode != DefaultMode) && bApplyMode)
        {
            switch (Mode)
            {
            case MONO:
                AudioLoader::ActivateDPL2(false, bUpdateMode);
                Audio::SetOutputMode((MusyXOutputType)0);
                break;
            case STEREO:
                AudioLoader::ActivateDPL2(false, bUpdateMode);
                Audio::SetOutputMode((MusyXOutputType)1);
                break;
            case DOLBY:
                AudioLoader::ActivateDPL2(true, bUpdateMode);
                break;
            default:
                break;
            }

            DefaultMode = Mode;
            OSSetSoundMode(Mode != MONO);

            Audio::MasterVolume::SetVolume(Audio::MasterVolume::VG_Music, musicVolume);
            g_pTrackManager->OnMasterVolumeChange(Audio::MasterVolume::VG_Music);

            Audio::SetVolGroupVolume(30, sfxVolume, 0);
            Audio::MasterVolume::SetVolume(Audio::MasterVolume::VG_SFX, sfxVolume);
            g_pTrackManager->OnMasterVolumeChange(Audio::MasterVolume::VG_SFX);

            Audio::MasterVolume::SetVoiceVolume(voiceVolume, 0);
            g_pTrackManager->OnMasterVolumeChange(Audio::MasterVolume::VG_Voice);
        }

        GameInfoManager* pGameInfo = GameInfoManager::s_pInstance;
        pGameInfo->mCurGameAudioSettings.MusicVolume = MusicVolume;
        pGameInfo->mCurGameAudioSettings.SFXVolume = SFXVolume;
        pGameInfo->mCurGameAudioSettings.VoiceVolume = VoiceVolume;
        pGameInfo->mCurGameAudioSettings.Mode = Mode;
    }
}

/**
 * Offset/Address/Size: 0x764 | 0x8018FE70 | size: 0x6C
 */
GameplaySettings::GameplaySettings()
{
    memset(this, 0, 0xC);

    SkillLevel = ROOKIE;
    GameTime = 0x12C;
    PowerUps = true;
    Shoot2Score = true;
    RumbleEnabled = true;
    BowserAttackEnabled = true;
    if (g_e3_Build != 0)
    {
        BowserAttackEnabled = false;
    }
}

/**
 * Offset/Address/Size: 0x760 | 0x8018FE6C | size: 0x4
 */
void GameplaySettings::InitializeDefaults()
{
}

/**
 * Offset/Address/Size: 0x75C | 0x8018FE68 | size: 0x4
 */
void GameplaySettings::OnSettingsUpdated() const
{
}

/**
 * Offset/Address/Size: 0x700 | 0x8018FE0C | size: 0x5C
 */
PowerupSettings::PowerupSettings()
{
    memset(this, 0, 8);
    RedShells = true;
    GreenShells = true;
    BlueShells = true;
    SpinyShells = true;
    Starman = true;
    Twister = true;
    Bobombs = true;
    Bananas = true;
}

/**
 * Offset/Address/Size: 0x234 | 0x8018F940 | size: 0x4CC
 */
void PowerupSettings::InitializeDefaults()
{
    Config cfg(Config::ALLOCATE_HIGH);
    cfg.LoadFromFile(POWERUPS_DEFAULT_CONFIG_FILE);

    RedShells = GetConfigBool(cfg, "Red Shells", true);
    GreenShells = GetConfigBool(cfg, "Green Shells", true);
    BlueShells = GetConfigBool(cfg, "Blue Shells", true);
    SpinyShells = GetConfigBool(cfg, "Spiny Shells", true);
    Starman = GetConfigBool(cfg, "Starman", true);
    Twister = GetConfigBool(cfg, "Twister", true);
    Bobombs = GetConfigBool(cfg, "Bobombs", true);
    Bananas = GetConfigBool(cfg, "Bananas", true);
}

/**
 * Offset/Address/Size: 0x230 | 0x8018F93C | size: 0x4
 */
void PowerupSettings::OnSettingsUpdated() const
{
}

/**
 * Offset/Address/Size: 0x1E0 | 0x8018F8EC | size: 0x50
 */
CheatSettings::CheatSettings()
{
    memset(this, 0, 8);
    mCustomPowerups = CP_OFF;
    mStunnedGoalies = false;
    mInfinitePowerups = false;
    mCheatTBD1Enabled = false;
    mCheatTBD2Enabled = false;
}

/**
 * Offset/Address/Size: 0x1C4 | 0x8018F8D0 | size: 0x1C
 */
void CheatSettings::InitializeDefaults()
{
    mCustomPowerups = CP_OFF;
    mStunnedGoalies = false;
    mInfinitePowerups = false;
    mCheatTBD1Enabled = false;
    mCheatTBD2Enabled = false;
}

/**
 * Offset/Address/Size: 0x1C0 | 0x8018F8CC | size: 0x4
 */
void CheatSettings::OnSettingsUpdated() const
{
}

/**
 * Offset/Address/Size: 0xD8 | 0x8018F7E4 | size: 0xE8
 */
VisualSettings::VisualSettings()
{
    memset(this, 0, 0x0C);
    mIsAutoZoomCamera = true;
    mCameraZoomLevel = 0.5f;
    mIsWidescreen = GetConfigBool(Config::Global(), "widescreen", false);
}

/**
 * Offset/Address/Size: 0x0 | 0x8018F70C | size: 0xD8
 */
void VisualSettings::InitializeDefaults()
{
    mIsAutoZoomCamera = true;
    mCameraZoomLevel = 0.5f;
    mIsWidescreen = GetConfigBool(Config::Global(), "widescreen", false);
}
