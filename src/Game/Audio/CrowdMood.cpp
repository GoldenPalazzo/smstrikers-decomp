#include "Game/Audio/CrowdMood.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/AI/AiUtil.h"
#include "Game/Game.h"
#include "Game/Sys/GCStream.h"

#include "NL/nlConfig.h"
#include "NL/nlMath.h"
#include "NL/nlString.h"

extern void ___blank(const char*, ...);
extern "C" void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
extern "C" void sndStreamLPFParameter(unsigned long, unsigned long, unsigned long);
extern "C" void sndStreamDeactivate(unsigned long stid);

struct CROWD_SETTINGS
{
    /* 0x000 */ float MoodDecayDelay;
    /* 0x004 */ float MoodDecayRate;
    /* 0x008 */ float BlendSpeedNormal;
    /* 0x00C */ float BlendSpeedFast;
    /* 0x010 */ float BlendStrictness;
    /* 0x014 */ float CrowdMasterVolume;
    /* 0x018 */ char NeutralSampleName[256];
    /* 0x118 */ char NegativeSampleName[256];
    /* 0x218 */ char PositiveSampleName[256];
    /* 0x318 */ char SaturationSampleNames[5][256];
    /* 0x818 */ unsigned char NoStreaming : 1;
}; // total size: 0x81C

static bool g_Initd;

template <typename T>
void Increment(T& Value);

static bool g_DoDecay = true;
static bool g_CrowdSFXStopped;
static CROWD_SETTINGS g_Settings;

CROWD_AUDIO_INIT g_CrowdAudio;
CROWD_STATE g_CrowdState;

// /**
//  * Offset/Address/Size: 0x134 | 0x801514D0 | size: 0x2C
//  */
// void GCAudioStreaming::MonoAudioStream::Purge()
// {
// }

// /**
//  * Offset/Address/Size: 0x108 | 0x801514A4 | size: 0x2C
//  */
// void GCAudioStreaming::StereoAudioStream::Purge()
// {
// }

// /**
//  * Offset/Address/Size: 0x98 | 0x80151434 | size: 0x70
//  */
// void GCAudioStreaming::MonoAudioStream::~MonoAudioStream()
// {
// }

// /**
//  * Offset/Address/Size: 0x4C | 0x801513E8 | size: 0x4C
//  */
// void GCAudioStreaming::StereoAudioStream::SafeToPurge()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8015139C | size: 0x4C
//  */
// void GCAudioStreaming::MonoAudioStream::SafeToPurge()
// {
// }

// /**
//  * Offset/Address/Size: 0x3C78 | 0x8015138C | size: 0x10
//  */
// void Increment<CrowdMood::CROWD_MOOD>(CrowdMood::CROWD_MOOD&)
// {
// }

// /**
//  * Offset/Address/Size: 0x3C10 | 0x80151324 | size: 0x68
//  */
// void NDimDistance<4>(float*, float*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3ADC | 0x801511F0 | size: 0x134
//  */
// void WarmRandomStream<GCAudioStreaming::StereoAudioStream>(const RANDOM_STREAMS&, GCAudioStreaming::StereoAudioStream*)
// {
// }

// /**
//  * Offset/Address/Size: 0x39A8 | 0x801510BC | size: 0x134
//  */
// void WarmRandomStream<GCAudioStreaming::MonoAudioStream>(const RANDOM_STREAMS&, GCAudioStreaming::MonoAudioStream*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3958 | 0x8015106C | size: 0x50
//  */
// void ___blank(const char*, ...)
// {
// }

/**
 * Offset/Address/Size: 0x3948 | 0x8015105C | size: 0x10
 */
unsigned char CrowdMood::IsStreamLocked()
{
    return g_CrowdState.StreamLocked;
}

/**
 * Offset/Address/Size: 0x3780 | 0x80150E94 | size: 0x1C8
 * TODO: 96.49% match - two branch-opcode diffs remain in stream setup:
 *       compiler emits `beq` after `cmplwi m_BufferCount, 0`, target emits `ble`.
 */
void ChangeCrowdVolume(float NewVolume)
{
    MOOD_DEFINITION MoodDef;
    g_CrowdState.CrowdVolume = NewVolume * g_Settings.CrowdMasterVolume;
    MoodDefFromBlend(g_CrowdState.CurrentMoodBlend, MoodDef);
    PlayMoodDef(MoodDef);

    float zero = 0.0f;
    if (fabsf(NewVolume - zero) <= 0.01f)
    {
        GCAudioStreaming::StereoAudioStream* pChant = g_CrowdAudio.pChantStream;
        if (pChant != NULL)
        {
            if (pChant->m_State >= GCAudioStreaming::SS_Warming)
            {
                GCAudioStreaming::AudioStreamBuffer* buf;
                volatile unsigned long i = (unsigned long)(buf = NULL);

                if (pChant->m_BufferCount > 0)
                {
                    buf = pChant->m_Buffers[0];
                }

                while (buf != NULL)
                {
                    buf->m_Volume = 0;
                    sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);

                    unsigned long ci = i + 1;
                    i = ci;
                    if (ci < pChant->m_BufferCount)
                    {
                        buf = pChant->m_Buffers[ci];
                    }
                    else
                    {
                        buf = NULL;
                    }
                }
            }
            pChant->m_Volume = 0;
        }

        GCAudioStreaming::MonoAudioStream* pHeckle = g_CrowdAudio.pHeckleStream;
        if (pHeckle != NULL)
        {
            if (pHeckle->m_State >= GCAudioStreaming::SS_Warming)
            {
                GCAudioStreaming::AudioStreamBuffer* buf;
                volatile unsigned long i = (unsigned long)(buf = NULL);

                if (pHeckle->m_BufferCount > 0)
                {
                    buf = pHeckle->m_Buffers[0];
                }

                while (buf != NULL)
                {
                    buf->m_Volume = 0;
                    sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);

                    unsigned long ci = i + 1;
                    i = ci;
                    if (ci < pHeckle->m_BufferCount)
                    {
                        buf = pHeckle->m_Buffers[ci];
                    }
                    else
                    {
                        buf = NULL;
                    }
                }
            }
            pHeckle->m_Volume = 0;
        }
    }
}

/**
 * Offset/Address/Size: 0x31F8 | 0x8015090C | size: 0x588
 */
void MoodDefFromBlend(float*, MOOD_DEFINITION&)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x2EA0 | 0x801505B4 | size: 0x358
 * TODO: 96.89% match - remaining diffs are m_BufferCount gate branch opcodes
 *       (`beq` vs `ble`) and LPF loop r28/r29 register allocation swap.
 *       Both appear to be MWCC compiler behavior differences.
 */
bool PlayVocal(const CROWD_VOCAL_DEFINITION& VocalDef, CROWD_STATE::VOCALIZATION_STATE& VocalState, GCAudioStreaming::AudioStream* pStream)
{
    if (g_Settings.NoStreaming)
    {
        return false;
    }

    float zero = 0.0f;
    if (fabsf(VocalDef.Volume - zero) <= 0.01f)
    {
        return false;
    }

    if (VocalState.SinceLast > VocalDef.Delay)
    {
        bool playNow;
        float delayRange = VocalDef.DelayRange;
        if (delayRange != zero)
        {
            playNow = ((VocalState.SinceLast - VocalDef.Delay) / delayRange) > VocalState.NextAt;
        }
        else
        {
            playNow = VocalState.SinceLast > VocalDef.Delay;
        }

        if (playNow)
        {
            ___blank("Crowd vocal\n");

            float randVol = nlRandomf(VocalDef.VolumeRange, &nlDefaultSeed);
            float scalar = 127.0f;
            float vocalVol = VocalDef.Volume;
            vocalVol = vocalVol + randVol;
            float crowdVol = g_CrowdState.CrowdVolume;
            crowdVol = crowdVol * vocalVol;
            scalar = scalar * crowdVol;
            int tmpVol = (int)scalar;
            unsigned long streamVol = 0x7F;

            if ((unsigned char)tmpVol <= 0x7F)
            {
                streamVol = tmpVol;
            }

            if (pStream->m_State >= GCAudioStreaming::SS_Warming)
            {
                GCAudioStreaming::AudioStreamBuffer* buf;
                volatile unsigned long i = (unsigned long)(buf = NULL);

                if (pStream->m_BufferCount > 0)
                {
                    buf = pStream->m_Buffers[0];
                }

                while (buf != NULL)
                {
                    buf->m_Volume = streamVol;
                    sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);

                    unsigned long ci = i + 1;
                    i = ci;
                    if (ci < pStream->m_BufferCount)
                    {
                        buf = pStream->m_Buffers[ci];
                    }
                    else
                    {
                        buf = NULL;
                    }
                }
            }

            pStream->m_Volume = (unsigned char)streamVol;

            unsigned short lpfFreq = (unsigned short)g_CrowdState.LPFFreq;
            if (pStream->m_State >= GCAudioStreaming::SS_Warming)
            {
                volatile unsigned long i;
                GCAudioStreaming::AudioStreamBuffer* pBuf;
                i = (unsigned long)(pBuf = NULL);

                if (pStream->m_BufferCount > 0)
                {
                    pBuf = pStream->m_Buffers[0];
                }

                while (pBuf != NULL)
                {
                    unsigned char lpfOn = pBuf->m_bLPFOn;
                    if (lpfOn != 0)
                    {
                        sndStreamLPFParameter(pBuf->m_StreamId, lpfOn, lpfFreq);
                    }

                    pBuf->m_LPFFreq = lpfFreq;

                    unsigned long ci = i + 1;
                    i = ci;
                    if (ci < pStream->m_BufferCount)
                    {
                        pBuf = pStream->m_Buffers[ci];
                    }
                    else
                    {
                        pBuf = NULL;
                    }
                }
            }

            pStream->m_LPFFreq = lpfFreq;

            pStream->m_Flags &= ~(1 << GCAudioStreaming::SF_Loop);
            pStream->m_Flags = (pStream->m_Flags & ~(1 << GCAudioStreaming::SF_CoolOnStop)) | (1 << GCAudioStreaming::SF_CoolOnStop);

            switch (pStream->m_State)
            {
            case GCAudioStreaming::SS_Initd:
                pStream->m_Flags = (pStream->m_Flags & ~(1 << GCAudioStreaming::SF_Play)) | (1 << GCAudioStreaming::SF_Play);
                pStream->WarmReadDone((GCAudioStreaming::AudioStreamBuffer*)1);
                break;
            case GCAudioStreaming::SS_Warming:
                pStream->m_Flags = (pStream->m_Flags & ~(1 << GCAudioStreaming::SF_Play)) | (1 << GCAudioStreaming::SF_Play);
                break;
            case GCAudioStreaming::SS_Warm:
            {
                GCAudioStreaming::AudioStreamBuffer* buf;
                volatile unsigned long i = (unsigned long)(buf = NULL);

                if (pStream->m_BufferCount > 0)
                {
                    buf = pStream->m_Buffers[0];
                }

                while (buf != NULL)
                {
                    sndStreamActivate(buf->m_StreamId);

                    unsigned long ci = i + 1;
                    i = ci;
                    if (ci < pStream->m_BufferCount)
                    {
                        buf = pStream->m_Buffers[ci];
                    }
                    else
                    {
                        buf = NULL;
                    }
                }

                pStream->m_State = GCAudioStreaming::SS_Playing;
                break;
            }
            }

            VocalState.Ready = false;
            VocalState.SinceLast = 0.0f;
            VocalState.NextAt = nlRandomf(1.0f, &nlDefaultSeed);
            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x2C7C | 0x80150390 | size: 0x224
 */
void PlayMoodDef(MOOD_DEFINITION& MoodDef)
{
    Audio::SetSFXVolume(g_CrowdAudio.NeutralVoiceId, MoodDef.NeutralVol * g_CrowdState.CrowdVolume);
    Audio::SetSFXVolume(g_CrowdAudio.PositiveVoiceId, MoodDef.PositiveVol * g_CrowdState.CrowdVolume);
    Audio::SetSFXVolume(g_CrowdAudio.NegativeVoiceId, MoodDef.NegativeVol * g_CrowdState.CrowdVolume);

    if (MoodDef.SaturationVolume > 0.0f)
    {
        if (MoodDef.SaturationSFXId == g_CrowdAudio.CurrentSaturationSFXId)
        {
            Audio::SetSFXVolume(g_CrowdAudio.SaturationVoiceId, MoodDef.SaturationVolume * g_CrowdState.CrowdVolume);
        }
        else
        {
            Audio::SetSFXVolumeGroup(MoodDef.SaturationSFXId, 2);
            float fVolReverb = Audio::gCrowdSFX.GetSFXVolReverb(0x99);

            SFXStartInfo info;
            info.uSFXID = (unsigned long)-1;
            info.fVolume = 0.0f;
            info.fPan = 0.0f;
            info.fVolReverb = 0.0f;
            info.uSurroundPan = 0xFF;
            info.uPitchBend = 0x2000;
            info.uModulation = 0;
            info.uDoppler = 0x2000;
            info.bActivateFilter = 0;
            info.filterFreq = 0;

            info.uSFXID = MoodDef.SaturationSFXId;
            info.fVolume = 1.0f;
            info.fVolReverb = fVolReverb;

            g_CrowdAudio.SaturationVoiceId = Audio::PlaySFX(info);
            Audio::SetSFXVolume(g_CrowdAudio.SaturationVoiceId, MoodDef.SaturationVolume);
        }
    }
    else
    {
        Audio::StopSFX(g_CrowdAudio.CurrentSaturationSFXId);
        g_CrowdAudio.CurrentSaturationSFXId = (unsigned long)-1;
    }

    if (!g_CrowdState.StreamLocked)
    {
        PlayVocal(MoodDef.Chant, g_CrowdState.ChantState, g_CrowdAudio.pChantStream);

        if (PlayVocal(MoodDef.Heckle, g_CrowdState.HeckleState, g_CrowdAudio.pHeckleStream))
        {
            unsigned char pan;
            GCAudioStreaming::AudioStreamBuffer* pBuf;

            pan = (unsigned char)nlRandom(0x7F, &nlDefaultSeed);
            pBuf = g_CrowdAudio.pHeckleStream->m_Buffers[0];
            pBuf->m_Pan = pan;
            sndStreamMixParameterEx(pBuf->m_StreamId, pBuf->m_Volume, pBuf->m_Pan, pBuf->m_SurroundPan, 0, 0);

            pan = (unsigned char)nlRandom(0x7F, &nlDefaultSeed);
            pBuf = g_CrowdAudio.pHeckleStream->m_Buffers[0];
            pBuf->m_SurroundPan = pan;
            sndStreamMixParameterEx(pBuf->m_StreamId, pBuf->m_Volume, pBuf->m_Pan, pBuf->m_SurroundPan, 0, 0);
        }
    }
}

/**
 * Offset/Address/Size: 0x2A0C | 0x80150120 | size: 0x270
 * TODO: 92.37% match - remaining f1/f2 register allocation and clamp control-flow
 *       differences in the dead decay block and volume-fade clamp path.
 */
void UpdateTiming(float dtArg)
{
    float dt = g_pGame->GetGameTime() - g_CrowdState.LastGameTime;

    if (!g_DoDecay)
    {
        dt = dtArg;
    }

    g_CrowdState.LastGameTime = g_pGame->GetGameTime();

    do
    {
        if (g_DoDecay)
        {
            break;
        }

        if (!g_CrowdState.AtDestination)
        {
            float speed = g_CrowdState.BlendFast ? g_Settings.BlendSpeedFast : g_Settings.BlendSpeedNormal;
            g_CrowdState.Interpolant += dt / speed;
        }

        if (g_CrowdState.ChantState.Ready)
        {
            g_CrowdState.ChantState.SinceLast += dt;
        }

        if (g_CrowdState.HeckleState.Ready)
        {
            g_CrowdState.HeckleState.SinceLast += dt;
        }

        if (!g_DoDecay)
        {
            break;
        }

        g_CrowdState._unk78 += dt;

        if (g_CrowdState.AtDestination)
        {
            if (g_CrowdState._unk78 > g_Settings.MoodDecayDelay)
            {
                float f2 = g_CrowdState.SinceMoodDest;
                if (f2 != 0.0f)
                {
                    if (g_DoDecay)
                    {
                        float f0 = g_Settings.MoodDecayRate;
                        if (f0 <= f2)
                        {
                            f2 = f0;
                        }

                        f2 = g_CrowdState.SinceMoodDest - f2;
                        g_CrowdState.SinceMoodDest = f2;
                        f2 = (f2 >= 0.0f) ? f2 : 0.0f;

                        {
                            float f1 = (float)f2;
                            g_CrowdState.SinceMoodDest = f2;
                            g_CrowdState.DestMoodLevel = (unsigned char)(255.0f * f1);
                            g_CrowdState.CurrentMoodBlend[(s8)g_CrowdState.CurrentMood] = f1;
                            g_CrowdState.SkipBlend = true;
                        }
                    }
                }
            }
        }
    } while (0);

    if (g_CrowdState.VolumeFade.Time > 0.0f)
    {
        float f3 = g_CrowdState.VolumeFade.Interp + (dtArg / g_CrowdState.VolumeFade.Time);
        g_CrowdState.VolumeFade.Interp = f3;
        f3 = (f3 <= 1.0f) ? f3 : 1.0f;

        {
            float f2 = (float)f3;
            g_CrowdState.VolumeFade.Interp = f3;

            if (fabsf(f2 - 1.0f) <= 0.01f)
            {
                g_CrowdState.VolumeFade.Time = 0.0f;
            }
        }

        ChangeCrowdVolume(Interpolate(g_CrowdState.VolumeFade.StartVol,
            g_CrowdState.VolumeFade.EndVol,
            g_CrowdState.VolumeFade.Interp));
    }
}

/**
 * Offset/Address/Size: 0x16E4 | 0x8014EDF8 | size: 0x1328
 */
void CrowdMood::ReadConfig()
{
}

/**
 * Offset/Address/Size: 0x1300 | 0x8014EA14 | size: 0x3E4
 */
void CrowdMood::Init()
{
}

/**
 * Offset/Address/Size: 0xF4C | 0x8014E660 | size: 0x3B4
 * TODO: 94.79% match - register allocation diffs (r29/r31, r30/r28 swapped),
 *       ble vs beq for m_BufferCount<=0 checks, r3/r4 swap in free-buffer loops,
 *       stack offset diffs (0x0C vs 0x10). Same known MWCC quirks as UnlockStream.
 */
void CrowdMood::Purge(bool bJustStopSFX)
{
    g_CrowdSFXStopped = true;

    Audio::StopSFX(g_CrowdAudio.NeutralVoiceId);
    Audio::StopSFX(g_CrowdAudio.PositiveVoiceId);
    Audio::StopSFX(g_CrowdAudio.NegativeVoiceId);

    GCAudioStreaming::StereoAudioStream* pChant = g_CrowdAudio.pChantStream;
    if (pChant != NULL)
    {
        pChant->m_Flags &= ~(1 << GCAudioStreaming::SF_Play);

        if (pChant->m_State == GCAudioStreaming::SS_Playing)
        {
            GCAudioStreaming::AudioStreamBuffer* buf;
            volatile unsigned long i = (unsigned long)(buf = NULL);

            if (pChant->m_BufferCount <= 0)
            {
            }
            else
            {
                buf = pChant->m_Buffers[0];
            }

            while (buf != NULL)
            {
                buf->m_Volume = 0;
                sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);
                sndStreamDeactivate(buf->m_StreamId);
                pChant->m_State = GCAudioStreaming::SS_Warm;

                unsigned long ci = i + 1;
                i = ci;
                if (ci < pChant->m_BufferCount)
                {
                    buf = pChant->m_Buffers[ci];
                }
                else
                {
                    buf = NULL;
                }
            }

            pChant->m_StreamPos = 0;
            pChant->m_State = GCAudioStreaming::SS_Warm;
        }

        pChant->CancelPendingReads();

        if (pChant->m_Flags & (1 << GCAudioStreaming::SF_CoolOnStop))
        {
            pChant->m_Flags &= ~(1 << GCAudioStreaming::SF_CoolOnStop);

            if (pChant->m_State > GCAudioStreaming::SS_Initd)
            {
                GCAudioStreaming::AudioStreamBuffer* buf;
                volatile unsigned long i = (unsigned long)(buf = NULL);

                pChant->m_Flags = (pChant->m_Flags & ~(1 << GCAudioStreaming::SF_SeriousStop)) | (1 << GCAudioStreaming::SF_SeriousStop);

                if (pChant->m_BufferCount <= 0)
                {
                }
                else
                {
                    buf = pChant->m_Buffers[0];
                }

                while (buf != NULL)
                {
                    pChant->m_BuffMgr.FreeBuffer(buf);

                    unsigned long bi = i;
                    unsigned long ci = bi + 1;
                    i = ci;
                    pChant->m_Buffers[bi] = NULL;

                    if (ci < pChant->m_BufferCount)
                    {
                        buf = pChant->m_Buffers[ci];
                    }
                    else
                    {
                        buf = NULL;
                    }
                }

                pChant->m_State = GCAudioStreaming::SS_Initd;
            }
        }
    }

    GCAudioStreaming::MonoAudioStream* pHeckle = g_CrowdAudio.pHeckleStream;
    if (pHeckle != NULL)
    {
        pHeckle->m_Flags &= ~(1 << GCAudioStreaming::SF_Play);

        if (pHeckle->m_State == GCAudioStreaming::SS_Playing)
        {
            GCAudioStreaming::AudioStreamBuffer* buf;
            volatile unsigned long i = (unsigned long)(buf = NULL);

            if (pHeckle->m_BufferCount <= 0)
            {
            }
            else
            {
                buf = pHeckle->m_Buffers[0];
            }

            while (buf != NULL)
            {
                buf->m_Volume = 0;
                sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);
                sndStreamDeactivate(buf->m_StreamId);
                pHeckle->m_State = GCAudioStreaming::SS_Warm;

                unsigned long ci = i + 1;
                i = ci;
                if (ci < pHeckle->m_BufferCount)
                {
                    buf = pHeckle->m_Buffers[ci];
                }
                else
                {
                    buf = NULL;
                }
            }

            pHeckle->m_StreamPos = 0;
            pHeckle->m_State = GCAudioStreaming::SS_Warm;
        }

        pHeckle->CancelPendingReads();

        if (pHeckle->m_Flags & (1 << GCAudioStreaming::SF_CoolOnStop))
        {
            pHeckle->m_Flags &= ~(1 << GCAudioStreaming::SF_CoolOnStop);

            if (pHeckle->m_State > GCAudioStreaming::SS_Initd)
            {
                GCAudioStreaming::AudioStreamBuffer* buf;
                volatile unsigned long i = (unsigned long)(buf = NULL);

                pHeckle->m_Flags = (pHeckle->m_Flags & ~(1 << GCAudioStreaming::SF_SeriousStop)) | (1 << GCAudioStreaming::SF_SeriousStop);

                if (pHeckle->m_BufferCount <= 0)
                {
                }
                else
                {
                    buf = pHeckle->m_Buffers[0];
                }

                while (buf != NULL)
                {
                    pHeckle->m_BuffMgr.FreeBuffer(buf);

                    unsigned long bi = i;
                    unsigned long ci = bi + 1;
                    i = ci;
                    pHeckle->m_Buffers[bi] = NULL;

                    if (ci < pHeckle->m_BufferCount)
                    {
                        buf = pHeckle->m_Buffers[ci];
                    }
                    else
                    {
                        buf = NULL;
                    }
                }

                pHeckle->m_State = GCAudioStreaming::SS_Initd;
            }
        }
    }

    if (!bJustStopSFX)
    {
        if (g_CrowdAudio.pChantStream != NULL)
        {
            g_CrowdAudio.pChantStream->WarmReadDone((GCAudioStreaming::AudioStreamBuffer*)1);
        }
        if (g_CrowdAudio.pHeckleStream != NULL)
        {
            g_CrowdAudio.pHeckleStream->WarmReadDone((GCAudioStreaming::AudioStreamBuffer*)1);
        }
        g_CrowdAudio.pChantStream = NULL;
        g_CrowdAudio.pHeckleStream = NULL;
        memset(&g_CrowdState, 0, 0x8C);
        g_Initd = false;
    }
}

/**
 * Offset/Address/Size: 0x9F8 | 0x8014E10C | size: 0x554
 */
void CrowdMood::Update(float)
{
}

/**
 * Offset/Address/Size: 0x92C | 0x8014E040 | size: 0xCC
 */
void CrowdMood::AdjustMood(CrowdMood::CROWD_MOOD Towards, unsigned long Amount)
{
    if (!g_Initd)
        return;

    if (Towards == (s8)g_CrowdState.DestMood)
    {
        g_CrowdState.DestMoodLevel += Amount;
    }
    else
    {
        u8 level = g_CrowdState.DestMoodLevel;
        if (level > Amount)
        {
            g_CrowdState.DestMoodLevel -= Amount;
        }
        else
        {
            g_CrowdState.DestMood = Towards;
            g_CrowdState.DestMoodLevel = (Towards == CM_Neutral) ? 0 : (Amount - level);
        }
    }

    g_CrowdState.HasChanged = true;
    g_CrowdState.AtDestination = false;

    ___blank("Crowd mood adjusted to %d %d\n", *(volatile s8*)&g_CrowdState.DestMood, g_CrowdState.DestMoodLevel);
}

/**
 * Offset/Address/Size: 0x804 | 0x8014DF18 | size: 0x128
 */
void CrowdMood::SetMood(CrowdMood::CROWD_MOOD Mood, unsigned long Amount)
{
    g_CrowdState.AtDestination = false;
    g_CrowdState.DestMoodLevel = Amount;
    g_CrowdState.DestMood = Mood;
    g_CrowdState.SinceMoodDest = (f32)Amount;
    g_CrowdState.CurrentMood = Mood;
    g_CrowdState.Interpolant = 0.0f;
    g_CrowdState.SkipBlend = true;

    for (CrowdMood::CROWD_MOOD mood = CM_Positive; mood < CM_Neutral; Increment<CrowdMood::CROWD_MOOD>(mood))
    {
        f32 blend;
        if (mood == Mood)
        {
            blend = (f32)Amount / (f32)CM_END;
        }
        else
        {
            blend = 0.0f;
        }
        g_CrowdState.CurrentMoodBlend[mood] = blend;
    }

    ___blank("Crowd mood set to %d %d\n", *(volatile s8*)&g_CrowdState.DestMood, g_CrowdState.DestMoodLevel);
}

/**
 * Offset/Address/Size: 0x7E8 | 0x8014DEFC | size: 0x1C
 */
void CrowdMood::InitiateFastCrowdTransition()
{
    g_CrowdState.BlendFast = true;
}

/**
 * Offset/Address/Size: 0x65C | 0x8014DD70 | size: 0x18C
 */
void CrowdMood::SetCrowdVolume(unsigned long Volume, unsigned long FadeTime)
{
    MOOD_DEFINITION MoodDef;
    unsigned char crowdOff = GetConfigBool(Config::Global(), "CrowdOff", false);

    if (crowdOff == 1)
    {
        g_CrowdState.CrowdVolume = 0.0f;
        MoodDefFromBlend(g_CrowdState.CurrentMoodBlend, MoodDef);
        PlayMoodDef(MoodDef);
        return;
    }

    if (FadeTime == 0)
    {
        ChangeCrowdVolume((float)Volume / 100.0f);
    }
    else
    {
        g_CrowdState.VolumeFade.StartVol = g_CrowdState.CrowdVolume;
        g_CrowdState.VolumeFade.EndVol = (float)Volume / 100.0f;
        g_CrowdState.VolumeFade.Time = (float)FadeTime / 1000.0f;
        g_CrowdState.VolumeFade.Interp = 0.0f;
    }
}

/**
 * Offset/Address/Size: 0x490 | 0x8014DBA4 | size: 0x1CC
 * TODO: 94.48% match (decomp.me) / 97.43% match (build) - register allocation
 *       differs for LPF flag/state/audio stream pointers (decomp.me compiler
 *       version issue), and both buffer-count checks emit `beq` (target `ble`).
 */
void CrowdMood::ActivateLPF(bool Activate)
{
    GCAudioStreaming::StereoAudioStream* pChant;
    GCAudioStreaming::MonoAudioStream* pHeckle;

    if (Activate == g_CrowdState.LPFOn)
    {
        return;
    }

    Audio::ActivateFilterOnSFX(g_CrowdAudio.NeutralVoiceId, Activate);
    Audio::ActivateFilterOnSFX(g_CrowdAudio.PositiveVoiceId, Activate);
    Audio::ActivateFilterOnSFX(g_CrowdAudio.NegativeVoiceId, Activate);

    pChant = g_CrowdAudio.pChantStream;
    if (pChant != NULL && g_CrowdAudio.pHeckleStream != NULL && !g_CrowdState.StreamLocked)
    {
        if (pChant->m_State >= GCAudioStreaming::SS_Warming)
        {
            GCAudioStreaming::AudioStreamBuffer* buf;
            volatile unsigned long i = (unsigned long)(buf = NULL);

            if (pChant->m_BufferCount > 0)
            {
                buf = pChant->m_Buffers[0];
            }

            unsigned char act = (unsigned char)Activate;
            while (buf != NULL)
            {
                if (act != buf->m_bLPFOn)
                {
                    sndStreamLPFParameter(buf->m_StreamId, Activate, buf->m_LPFFreq);
                    buf->m_bLPFOn = Activate;
                }

                unsigned long ci = i + 1;
                i = ci;
                if (ci < pChant->m_BufferCount)
                {
                    buf = pChant->m_Buffers[ci];
                }
                else
                {
                    buf = NULL;
                }
            }
        }

        pChant->m_LPFOn = Activate;

        pHeckle = g_CrowdAudio.pHeckleStream;
        if (pHeckle->m_State >= GCAudioStreaming::SS_Warming)
        {
            GCAudioStreaming::AudioStreamBuffer* buf;
            volatile unsigned long i = (unsigned long)(buf = NULL);

            if (pHeckle->m_BufferCount > 0)
            {
                buf = pHeckle->m_Buffers[0];
            }

            while (buf != NULL)
            {
                if (Activate != buf->m_bLPFOn)
                {
                    sndStreamLPFParameter(buf->m_StreamId, Activate, buf->m_LPFFreq);
                    buf->m_bLPFOn = Activate;
                }

                unsigned long ci = i + 1;
                i = ci;
                if (ci < pHeckle->m_BufferCount)
                {
                    buf = pHeckle->m_Buffers[ci];
                }
                else
                {
                    buf = NULL;
                }
            }
        }

        pHeckle->m_LPFOn = Activate;
    }

    g_CrowdState.LPFOn = Activate;
}

/**
 * Offset/Address/Size: 0x2D4 | 0x8014D9E8 | size: 0x1BC
 * TODO: 97.9% match (build) - `cmplwi m_BufferCount, 0` emits `beq` (target `ble`) in both
 *       buffer-start checks; likely GC/2.0 vs GC/1.3.2 compiler version difference.
 */
void CrowdMood::SetLPF(unsigned short Frequency)
{
    GCAudioStreaming::StereoAudioStream* pChant;
    GCAudioStreaming::MonoAudioStream* pHeckle;
    GCAudioStreaming::AudioStreamBuffer* buf;
    unsigned long maskedFreq;

    if (g_CrowdState.LPFFreq == Frequency)
        return;

    Audio::SetFilterFreqOnSFX(g_CrowdAudio.NeutralVoiceId, Frequency);
    Audio::SetFilterFreqOnSFX(g_CrowdAudio.PositiveVoiceId, Frequency);
    Audio::SetFilterFreqOnSFX(g_CrowdAudio.NegativeVoiceId, Frequency);

    pChant = g_CrowdAudio.pChantStream;
    if (pChant != NULL && g_CrowdAudio.pHeckleStream != NULL && !g_CrowdState.StreamLocked)
    {
        if (pChant->m_State >= GCAudioStreaming::SS_Warming)
        {
            volatile unsigned long i = (unsigned long)(buf = NULL);
            if (pChant->m_BufferCount > 0)
            {
                buf = pChant->m_Buffers[0];
            }
            maskedFreq = Frequency;
            while (buf != NULL)
            {
                if (buf->m_bLPFOn)
                {
                    sndStreamLPFParameter(buf->m_StreamId, buf->m_bLPFOn, maskedFreq);
                }
                buf->m_LPFFreq = Frequency;
                unsigned long ci = i + 1;
                i = ci;
                if (ci < pChant->m_BufferCount)
                {
                    buf = pChant->m_Buffers[ci];
                }
                else
                {
                    buf = NULL;
                }
            }
        }
        pChant->m_LPFFreq = Frequency;

        pHeckle = g_CrowdAudio.pHeckleStream;
        if (pHeckle->m_State >= GCAudioStreaming::SS_Warming)
        {
            volatile unsigned long i = (unsigned long)(buf = NULL);
            if (pHeckle->m_BufferCount > 0)
            {
                buf = pHeckle->m_Buffers[0];
            }
            maskedFreq = Frequency;
            while (buf != NULL)
            {
                if (buf->m_bLPFOn)
                {
                    sndStreamLPFParameter(buf->m_StreamId, buf->m_bLPFOn, maskedFreq);
                }
                buf->m_LPFFreq = Frequency;
                unsigned long ci = i + 1;
                i = ci;
                if (ci < pHeckle->m_BufferCount)
                {
                    buf = pHeckle->m_Buffers[ci];
                }
                else
                {
                    buf = NULL;
                }
            }
        }
        pHeckle->m_LPFFreq = Frequency;
    }

    g_CrowdState.LPFFreq = Frequency;
}

/**
 * Offset/Address/Size: 0x2B4 | 0x8014D9C8 | size: 0x20
 */
GCAudioStreaming::StereoAudioStream* CrowdMood::LockStream()
{
    g_CrowdState.StreamLocked = true;
    return g_CrowdAudio.pChantStream;
}

/**
 * Offset/Address/Size: 0x104 | 0x8014D818 | size: 0x1B0
 * TODO: 95.88% match - `cmplwi m_BufferCount, 0` still emits `beq` (target `ble`)
 *       in both buffer-start checks; second free-buffer loop still has r3/r4 swap.
 */
void CrowdMood::UnlockStream()
{
    GCAudioStreaming::StereoAudioStream* pChant = g_CrowdAudio.pChantStream;

    if (pChant != NULL)
    {
        pChant->m_Flags &= ~(1 << GCAudioStreaming::SF_Play);

        if (pChant->m_State == GCAudioStreaming::SS_Playing)
        {
            GCAudioStreaming::AudioStreamBuffer* buf;
            volatile unsigned long i = (unsigned long)(buf = NULL);

            if (pChant->m_BufferCount <= 0)
            {
            }
            else
            {
                buf = pChant->m_Buffers[0];
            }

            while (buf != NULL)
            {
                buf->m_Volume = 0;
                sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);
                sndStreamDeactivate(buf->m_StreamId);
                pChant->m_State = GCAudioStreaming::SS_Warm;

                unsigned long ci = i + 1;
                i = ci;
                if (ci < pChant->m_BufferCount)
                {
                    buf = pChant->m_Buffers[ci];
                }
                else
                {
                    buf = NULL;
                }
            }

            pChant->m_StreamPos = 0;
            pChant->m_State = GCAudioStreaming::SS_Warm;
        }

        pChant->CancelPendingReads();

        if (pChant->m_Flags & (1 << GCAudioStreaming::SF_CoolOnStop))
        {
            pChant->m_Flags &= ~(1 << GCAudioStreaming::SF_CoolOnStop);

            if (pChant->m_State > GCAudioStreaming::SS_Initd)
            {
                GCAudioStreaming::AudioStreamBuffer* buf;
                volatile unsigned long i = (unsigned long)(buf = NULL);

                pChant->m_Flags = (pChant->m_Flags & ~(1 << GCAudioStreaming::SF_SeriousStop)) | (1 << GCAudioStreaming::SF_SeriousStop);

                if (pChant->m_BufferCount <= 0)
                {
                }
                else
                {
                    buf = pChant->m_Buffers[0];
                }

                while (buf != NULL)
                {
                    pChant->m_BuffMgr.FreeBuffer(buf);

                    unsigned long bi = i;
                    unsigned long ci = bi + 1;
                    i = ci;
                    pChant->m_Buffers[bi] = NULL;

                    if (ci < pChant->m_BufferCount)
                    {
                        buf = pChant->m_Buffers[ci];
                    }
                    else
                    {
                        buf = NULL;
                    }
                }

                pChant->m_State = GCAudioStreaming::SS_Initd;
            }
        }
    }

    g_CrowdState.StreamLocked = false;
}

/**
 * Offset/Address/Size: 0xFC | 0x8014D810 | size: 0x8
 */
void CrowdMood::EnableCrowdDecay(bool enable)
{
    g_DoDecay = enable;
}

/**
 * Offset/Address/Size: 0x0 | 0x8014D714 | size: 0xFC
 */
void CrowdMood::RestartLoops()
{
    struct LOOP_LOAD
    {
        const char* SampleName;
        unsigned long AudioId;
        unsigned long* pVoiceId;
    };

    LOOP_LOAD LoadData[3] = {
        { 0, 0, &g_CrowdAudio.NeutralVoiceId },
        { 0, 0, &g_CrowdAudio.PositiveVoiceId },
        { 0, 0, &g_CrowdAudio.NegativeVoiceId },
    };

    u32 i;
    for (i = 0; i < 3; i++)
    {
        Audio::SoundAttributes sndAtr;
        sndAtr.Init();
        sndAtr.SetSoundType(LoadData[i].AudioId, false);
        sndAtr.mf_Volume = 0.0f;
        *LoadData[i].pVoiceId = Audio::gCrowdSFX.Play(sndAtr);
        PlatAudio::SetSFXVolume(*LoadData[i].pVoiceId, 0.0f);
    }
    g_CrowdSFXStopped = false;
}

// #include "Game/Audio/CrowdMood.h"

// static bool g_DoDecay;

// /**
//  * Offset/Address/Size: 0x0 | 0x8014D714 | size: 0xFC
//  */
// void CrowdMood::RestartLoops()
// {
// }

// /**
//  * Offset/Address/Size: 0xFC | 0x8014D810 | size: 0x8
//  * TODO: 97.5% match - register mismatch (r3 vs r4) due to unused 'this'
//  */
// void CrowdMood::EnableCrowdDecay(bool enable)
// {
//     g_DoDecay = enable;
// }

// /**
//  * Offset/Address/Size: 0x104 | 0x8014D818 | size: 0x1B0
//  */
// void CrowdMood::UnlockStream()
// {
// }

// /**
//  * Offset/Address/Size: 0x2B4 | 0x8014D9C8 | size: 0x20
//  */
// void CrowdMood::LockStream()
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x8014D9E8 | size: 0x1BC
//  */
// void CrowdMood::SetLPF(unsigned short)
// {
// }

// /**
//  * Offset/Address/Size: 0x490 | 0x8014DBA4 | size: 0x1CC
//  */
// void CrowdMood::ActivateLPF(bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x65C | 0x8014DD70 | size: 0x18C
//  */
// void CrowdMood::SetCrowdVolume(unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x7E8 | 0x8014DEFC | size: 0x1C
//  */
// void CrowdMood::InitiateFastCrowdTransition()
// {
//     g_CrowdState.BlendFast = true;
// }

// /**
//  * Offset/Address/Size: 0x804 | 0x8014DF18 | size: 0x128
//  */
// void CrowdMood::SetMood(CrowdMood::CROWD_MOOD, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x92C | 0x8014E040 | size: 0xCC
//  */
// void CrowdMood::AdjustMood(CrowdMood::CROWD_MOOD, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x9F8 | 0x8014E10C | size: 0x554
//  */
// void CrowdMood::Update(float)
// {
// }

// /**
//  * Offset/Address/Size: 0xF4C | 0x8014E660 | size: 0x3B4
//  */
// void CrowdMood::Purge(bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x1300 | 0x8014EA14 | size: 0x3E4
//  */
// void CrowdMood::Init()
// {
// }

// /**
//  * Offset/Address/Size: 0x16E4 | 0x8014EDF8 | size: 0x1328
//  */
// void CrowdMood::ReadConfig()
// {
// }

// /**
//  * Offset/Address/Size: 0x2A0C | 0x80150120 | size: 0x270
//  */
// void UpdateTiming(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x2C7C | 0x80150390 | size: 0x224
//  */
// void PlayMoodDef(MOOD_DEFINITION&)
// {
// }

// /**
//  * Offset/Address/Size: 0x2EA0 | 0x801505B4 | size: 0x358
//  */
// // void PlayVocal(const CROWD_VOCAL_DEFINITION&, CROWD_STATE::VOCALIZATION_STATE&, GCAudioStreaming::AudioStream*)
// // {
// // }

// /**
//  * Offset/Address/Size: 0x31F8 | 0x8015090C | size: 0x588
//  */
// void MoodDefFromBlend(float*, MOOD_DEFINITION&)
// {
// }

// /**
//  * Offset/Address/Size: 0x3780 | 0x80150E94 | size: 0x1C8
//  */
// void ChangeCrowdVolume(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x3948 | 0x8015105C | size: 0x10
//  */
// void CrowdMood::IsStreamLocked()
// {
// }

// /**
//  * Offset/Address/Size: 0x3958 | 0x8015106C | size: 0x50
//  */
// void ___blank(const char*, ...)
// {
// }

// // /**
// //  * Offset/Address/Size: 0x39A8 | 0x801510BC | size: 0x134
// //  */
// // void WarmRandomStream<GCAudioStreaming::MonoAudioStream>(const RANDOM_STREAMS&, GCAudioStreaming::MonoAudioStream*)
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x3ADC | 0x801511F0 | size: 0x134
// //  */
// // void WarmRandomStream<GCAudioStreaming::StereoAudioStream>(const RANDOM_STREAMS&, GCAudioStreaming::StereoAudioStream*)
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x3C10 | 0x80151324 | size: 0x68
// //  */
// // void NDimDistance<4>(float*, float*)
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x3C78 | 0x8015138C | size: 0x10
// //  */
// // void Increment<CrowdMood::CROWD_MOOD>(CrowdMood::CROWD_MOOD&)
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x0 | 0x8015139C | size: 0x4C
// //  */
// // void GCAudioStreaming::MonoAudioStream::SafeToPurge()
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x4C | 0x801513E8 | size: 0x4C
// //  */
// // void GCAudioStreaming::StereoAudioStream::SafeToPurge()
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x98 | 0x80151434 | size: 0x70
// //  */
// // void GCAudioStreaming::MonoAudioStream::~MonoAudioStream()
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x108 | 0x801514A4 | size: 0x2C
// //  */
// // void GCAudioStreaming::StereoAudioStream::Purge()
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x134 | 0x801514D0 | size: 0x2C
// //  */
// // void GCAudioStreaming::MonoAudioStream::Purge()
// // {
// // }

// // /**
// //  * Offset/Address/Size: 0x0 | 0x801514FC | size: 0x48
// //  */
// // void nlStrNCmp<char>(const char*, const char*, unsigned long)
// // {
// // }
