#include "Game/Sys/GCStream.h"
#include "Game/Audio/PriorityStream.h"
#include "Game/Audio/CrowdMood.h"
#include "Game/Game.h"
#include "NL/nlConfig.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"

unsigned char PriorityStream::PLAY_RECORD::s_BowserAttackNext = true;
unsigned char PriorityStream::PLAY_RECORD::s_SuddenDeathNext = true;

unsigned long PriorityStream::PLAY_RECORD::GetNextStreamId(unsigned long SimpleStreamId)
{
    char StreamName[64];
    char* Format;
    unsigned char* pCounter;

    switch (SimpleStreamId)
    {
    case 0x436E3953:
        pCounter = &PLAY_RECORD::s_BowserAttackNext;
        Format = "STAD_Bowser_Attack_%02d";
        break;
    case 0x57CB5A12:
        pCounter = &PLAY_RECORD::s_SuddenDeathNext;
        Format = "STAD_Sudden_Death_%02d";
        break;
    default:
        return SimpleStreamId;
    }

    nlSNPrintf(StreamName, 64, Format, *pCounter);
    *pCounter = *pCounter + 1;
    if (*pCounter == 4)
    {
        *pCounter = 1;
    }
    return nlStringLowerHash(StreamName);
}

void PriorityStream::PLAY_RECORD::Set(
    unsigned long StreamId,
    float Volume,
    bool Looping,
    unsigned long FadeIn,
    unsigned long ExistingFadeOut,
    const char* StreamParam,
    Audio::MasterVolume::VOLUME_GROUP VolGroup,
    bool Queue,
    bool Active)
{
    m_StreamId = StreamId;
    m_OrigStreamId = StreamId;
    m_Volume = Volume;
    m_Fades.b.looping = Looping;
    m_Fades.b.fadeIn = FadeIn;
    m_Fades.b.fadeOut = ExistingFadeOut;
    m_VolGroup = VolGroup;
    m_Fades.b.queue = Queue;
    m_Fades.b.active = Active;

    if (StreamParam)
    {
        nlStrNCpy<char>(m_StreamParam, StreamParam, 32);
    }
    else
    {
        m_StreamParam[0] = '\0';
    }
}

void PriorityStream::PLAY_RECORD::Play(bool CheckActive, bool GetNextId)
{
    if (!m_StreamId)
    {
        return;
    }

    if (CheckActive && !m_Fades.b.active)
    {
        return;
    }

    if (GetNextId)
    {
        m_StreamId = GetNextStreamId(m_OrigStreamId);
    }

    if (m_Fades.b.queue)
    {
        m_Fades.b.queue = 0;
        m_Track.QueueStream(
            m_StreamId,
            m_Volume,
            (m_Fades.b.looping & 1),
            m_Fades.b.fadeIn,
            m_StreamParam[0] ? m_StreamParam : (const char*)0,
            (Audio::MasterVolume::VOLUME_GROUP)m_VolGroup);
    }
    else
    {
        m_Track.PlayStream(
            m_StreamId,
            m_Volume,
            (m_Fades.b.looping & 1),
            m_Fades.b.fadeIn,
            m_Fades.b.fadeOut,
            m_StreamParam[0] ? m_StreamParam : (const char*)0,
            (Audio::MasterVolume::VOLUME_GROUP)m_VolGroup);
    }
}

/**
 * Offset/Address/Size: 0xEA8 | 0x8015895C | size: 0x10
 */
void PriorityStream::Reset()
{
    PLAY_RECORD::s_BowserAttackNext = true;
    PLAY_RECORD::s_SuddenDeathNext = true;
}

/**
 * Offset/Address/Size: 0xA34 | 0x801584E8 | size: 0x474
 * TODO: 99.21% match - selected PLAY_RECORD base uses r9 instead of r5,
 *   volume group uses r8 instead of r6, two looping-bit reloads use r3
 *   instead of r4, and one track load follows the FadeIn extract.
 */
void PriorityStream::PlayStream(unsigned long StreamId, float Volume, bool Looping, unsigned long FadeIn, unsigned long ExistingFadeOut, const char* StreamParam)
{
    if (GetConfigBool(Config::Global(), "no_stream", false) == true)
    {
        return;
    }

    if (g_pGame->mInSuddenDeath)
    {
        switch (StreamId)
        {
        case 0x09451A58:
        case 0xA207B1AE:
        case 0x436E3953:
            return;
        }
    }

    bool active = GrabCrowdStream(ExistingFadeOut);
    bool queue = true;
    Audio::MasterVolume::VOLUME_GROUP volGroup = Audio::MasterVolume::VG_Special;

    switch (StreamId)
    {
    case 0xE38B5407:
        volGroup = Audio::MasterVolume::VG_Voice;
        break;
    case 0x436E3953:
        volGroup = Audio::MasterVolume::VG_Music;
        break;
    case 0x09451A58:
        volGroup = Audio::MasterVolume::VG_Music;
        queue = 0;
        break;
    case 0xA207B1AE:
        volGroup = Audio::MasterVolume::VG_Music;
        queue = 0;
        break;
    case 0x57CB5A12:
        volGroup = Audio::MasterVolume::VG_Music;
        break;
    }

    PLAY_RECORD* pRecord;
    if (StreamId == 0xE38B5407)
    {
        pRecord = &m_CapChant;
    }
    else
    {
        pRecord = &m_PStream;
    }

    if ((StreamId == 0xA207B1AE) && m_CapChant.m_StreamId)
    {
        pRecord->m_StreamId = 0;
    }
    else
    {
        pRecord->Set(
            StreamId,
            Volume,
            Looping,
            FadeIn,
            ExistingFadeOut,
            StreamParam,
            volGroup,
            active,
            queue);
    }

    FakeResume(false);
}

/**
 * Offset/Address/Size: 0x79C | 0x80158250 | size: 0x298
 * TODO: 99.52% match - r3/r4 register swap in dead CapChant QueueStream path
 */
void PriorityStream::Stop(unsigned long StreamId, unsigned long Fadeout)
{
    if ((StreamId == 0xE38B5407) && m_CapChant.m_StreamId)
    {
        m_Track.Stop(Fadeout);
        m_CapChant.m_StreamId = 0;

        FakeResume(true);
    }
    else if ((m_PStream.m_OrigStreamId == StreamId)
             || ((StreamId == 0x436E3953) && ((m_PStream.m_OrigStreamId == 0x09451A58) || (m_PStream.m_OrigStreamId == 0xA207B1AE))))
    {
        m_Track.Stop(Fadeout);
        m_PStream.m_StreamId = 0;
    }
}

/**
 * Offset/Address/Size: 0x770 | 0x80158224 | size: 0x2C
 */
void PriorityStream::FakePause(unsigned long Fadeout)
{
    // Assembly stores byte at offset 0x00 (pause flag)
    // Header shows m_InPause at 0x02, but assembly uses 0x00
    // reinterpret_cast<unsigned char*>(this)[0] = 1;
    m_InPause = true;
    m_Track.Stop(Fadeout);
}

/**
 * Offset/Address/Size: 0x530 | 0x80157FE4 | size: 0x240
 * TODO: 99.34% match - remaining diffs are register selection in OrigStream/
 * PStream queue-play dispatch and one instruction ordering swap in PStream
 * play path setup.
 */
void PriorityStream::FakeResume(bool checkActive)
{
    if (m_CapChant.m_StreamId)
    {
        m_CapChant.Play(checkActive, false);
    }
    else
    {
        m_PStream.Play(checkActive, true);
    }

    m_InPause = false;
}

/**
 * Offset/Address/Size: 0x380 | 0x80157E34 | size: 0x1B0
 * TODO: 99.72% match - play-path FadeIn halfword uses r6 instead of r4.
 */
void PriorityStream::TrackIdleCB()
{
    if (m_InPause)
    {
        return;
    }

    if (m_CapChant.m_StreamId)
    {
        m_CapChant.m_StreamId = 0;

        if (m_PStream.m_StreamId)
        {
            if (!m_PStream.m_StreamId)
            {
                return;
            }

            if (!m_PStream.m_Fades.b.active)
            {
                return;
            }

            m_PStream.m_StreamId = m_PStream.GetNextStreamId(m_PStream.m_OrigStreamId);

            if (m_PStream.m_Fades.b.queue)
            {
                m_PStream.m_Fades.b.queue = 0;
                bool looping;
                unsigned long dummy;
                unsigned long fadeIn;
                AudioStreamTrack::StreamTrack& track = m_PStream.m_Track;
                looping = (m_PStream.m_Fades.b.looping & 1);
                dummy = m_PStream.m_StreamId;
                fadeIn = m_PStream.m_Fades.b.fadeIn;
                track.QueueStream(
                    dummy,
                    m_PStream.m_Volume,
                    looping,
                    fadeIn,
                    m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0,
                    (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
            }
            else
            {
                m_PStream.m_Track.PlayStream(
                    m_PStream.m_StreamId,
                    m_PStream.m_Volume,
                    (m_PStream.m_Fades.b.looping & 1),
                    (unsigned long)((m_PStream.m_Fades.half & 0xfffe) >> 1),
                    m_PStream.m_Fades.b.fadeOut,
                    m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0,
                    (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
            }
            return;
        }
    }

    if (m_PStream.m_StreamId)
    {
        m_PStream.m_StreamId = 0;
    }
    CrowdMood::UnlockStream();
}

/**
 * Offset/Address/Size: 0x0 | 0x80157AB4 | size: 0x380
 * TODO: 88.83% match - register allocation and control-flow shape differ in
 *       prologue/state dispatch; function still compiles as void while asm
 *       carries a return register through r30/r3.
 */
extern "C"
{
    void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
    void sndStreamDeactivate(unsigned long stid);
}

/**
 * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0x380
 */
bool PriorityStream::GrabCrowdStream(unsigned long Fadeout)
{
    GCAudioStreaming::StereoAudioStream* pStream;
    bool result = false;
    unsigned long zero = 0;
    volatile unsigned long iPlayStop;
    volatile unsigned long iWarmStop;
    volatile unsigned long iPlayFree;
    volatile unsigned long iWarmFree;

    if (!CrowdMood::IsStreamLocked())
    {
        pStream = CrowdMood::LockStream();
        if (pStream != NULL)
        {
            switch (pStream->m_State)
            {
            case GCAudioStreaming::SS_Playing:
            {
                if (Fadeout != 0)
                {
                    result = 1;
                    m_Track.AttachStream(
                        pStream, (Audio::MasterVolume::VOLUME_GROUP)4, (unsigned long)-1, 0, 0, 0);
                    m_Track.StopHead(Fadeout);
                }
                else
                {
                    pStream->m_Flags &= ~(1 << GCAudioStreaming::SF_Play);

                    if (pStream->m_State == GCAudioStreaming::SS_Playing)
                    {
                        GCAudioStreaming::AudioStreamBuffer* pBuffer;
                        iPlayStop = (unsigned long)(pBuffer = NULL);

                        if (pStream->m_BufferCount > zero)
                        {
                            pBuffer = pStream->m_Buffers[0];
                        }

                        while (pBuffer != NULL)
                        {
                            pBuffer->m_Volume = 0;
                            sndStreamMixParameterEx(pBuffer->m_StreamId, pBuffer->m_Volume, pBuffer->m_Pan, pBuffer->m_SurroundPan, 0, 0);
                            sndStreamDeactivate(pBuffer->m_StreamId);
                            pStream->m_State = GCAudioStreaming::SS_Warm;

                            {
                                unsigned long ci = iPlayStop + 1;
                                iPlayStop = ci;
                                if (ci < pStream->m_BufferCount)
                                {
                                    pBuffer = pStream->m_Buffers[ci];
                                }
                                else
                                {
                                    pBuffer = NULL;
                                }
                            }
                        }

                        pStream->m_StreamPos = 0;
                        pStream->m_State = GCAudioStreaming::SS_Warm;
                    }

                    pStream->CancelPendingReads();

                    if (pStream->m_Flags & (1 << GCAudioStreaming::SF_CoolOnStop))
                    {
                        pStream->m_Flags &= ~(1 << GCAudioStreaming::SF_CoolOnStop);

                        if (pStream->m_State > GCAudioStreaming::SS_Initd)
                        {
                            GCAudioStreaming::AudioStreamBuffer* pBuffer;
                            iPlayFree = (unsigned long)(pBuffer = NULL);

                            pStream->m_Flags = (pStream->m_Flags & ~(1 << GCAudioStreaming::SF_SeriousStop)) | (1 << GCAudioStreaming::SF_SeriousStop);

                            if (pStream->m_BufferCount > zero)
                            {
                                pBuffer = pStream->m_Buffers[0];
                            }

                            while (pBuffer != NULL)
                            {
                                pStream->m_BuffMgr.FreeBuffer(pBuffer);

                                {
                                    unsigned long idx = iPlayFree;
                                    pStream->m_Buffers[idx] = NULL;
                                    idx = idx + 1;
                                    iPlayFree = idx;
                                    if (idx < pStream->m_BufferCount)
                                    {
                                        pBuffer = pStream->m_Buffers[idx];
                                    }
                                    else
                                    {
                                        pBuffer = NULL;
                                    }
                                }
                            }

                            pStream->m_State = GCAudioStreaming::SS_Initd;
                        }
                    }
                }
                break;
            }
            case GCAudioStreaming::SS_Warming:
            case GCAudioStreaming::SS_Warm:
            {
                pStream->m_Flags &= ~(1 << GCAudioStreaming::SF_Play);

                if (pStream->m_State == GCAudioStreaming::SS_Playing)
                {
                    GCAudioStreaming::AudioStreamBuffer* pBuffer;
                    iWarmStop = (unsigned long)(pBuffer = NULL);

                    if (pStream->m_BufferCount > zero)
                    {
                        pBuffer = pStream->m_Buffers[0];
                    }

                    while (pBuffer != NULL)
                    {
                        pBuffer->m_Volume = 0;
                        sndStreamMixParameterEx(pBuffer->m_StreamId, pBuffer->m_Volume, pBuffer->m_Pan, pBuffer->m_SurroundPan, 0, 0);
                        sndStreamDeactivate(pBuffer->m_StreamId);
                        pStream->m_State = GCAudioStreaming::SS_Warm;

                        {
                            unsigned long ci = iWarmStop + 1;
                            iWarmStop = ci;
                            if (ci < pStream->m_BufferCount)
                            {
                                pBuffer = pStream->m_Buffers[ci];
                            }
                            else
                            {
                                pBuffer = NULL;
                            }
                        }
                    }

                    pStream->m_StreamPos = 0;
                    pStream->m_State = GCAudioStreaming::SS_Warm;
                }

                pStream->CancelPendingReads();

                if (pStream->m_Flags & (1 << GCAudioStreaming::SF_CoolOnStop))
                {
                    pStream->m_Flags &= ~(1 << GCAudioStreaming::SF_CoolOnStop);

                    if (pStream->m_State > GCAudioStreaming::SS_Initd)
                    {
                        GCAudioStreaming::AudioStreamBuffer* pBuffer;
                        iWarmFree = (unsigned long)(pBuffer = NULL);

                        pStream->m_Flags = (pStream->m_Flags & ~(1 << GCAudioStreaming::SF_SeriousStop)) | (1 << GCAudioStreaming::SF_SeriousStop);

                        if (pStream->m_BufferCount > zero)
                        {
                            pBuffer = pStream->m_Buffers[0];
                        }

                        while (pBuffer != NULL)
                        {
                            pStream->m_BuffMgr.FreeBuffer(pBuffer);

                            {
                                unsigned long idx = iWarmFree;
                                pStream->m_Buffers[idx] = NULL;
                                idx = idx + 1;
                                iWarmFree = idx;
                                if (idx < pStream->m_BufferCount)
                                {
                                    pBuffer = pStream->m_Buffers[idx];
                                }
                                else
                                {
                                    pBuffer = NULL;
                                }
                            }
                        }

                        pStream->m_State = GCAudioStreaming::SS_Initd;
                    }
                }
                break;
            }
            }
        }
    }

    return result;
}
