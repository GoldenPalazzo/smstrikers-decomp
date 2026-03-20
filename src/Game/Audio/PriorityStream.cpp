#include "Game/Audio/PriorityStream.h"
#include "Game/Audio/CrowdMood.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"

unsigned char PLAY_RECORD::s_BowserAttackNext = true;
unsigned char PLAY_RECORD::s_SuddenDeathNext = true;

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
 */
void PriorityStream::PlayStream(unsigned long, float, bool, unsigned long, unsigned long, const char*)
{
}

/**
 * Offset/Address/Size: 0x79C | 0x80158250 | size: 0x298
 * TODO: 97.29% match - switch/default block still differs in register allocation
 *   (r3/r4 swaps, literal load mr), with follow-on branch target offsets.
 */
void PriorityStream::Stop(unsigned long StreamId, unsigned long Fadeout)
{
    char StreamName[64];
    unsigned char* pCounter;
    const char* Format;

    if ((StreamId == 0xE38B5407) && m_PStream.m_OrigStreamId)
    {
        m_Track.Stop(Fadeout);
        m_PStream.m_OrigStreamId = 0;

        if (m_PStream.m_OrigStreamId)
        {
            if (m_PStream.m_OrigStreamId && m_CapChant.m_Active)
            {
                if (m_CapChant.m_Queue)
                {
                    m_CapChant.m_Queue = 0;
                    m_CapChant.m_Track.QueueStream(
                        m_PStream.m_OrigStreamId,
                        m_CapChant.m_Volume,
                        (m_CapChant.m_Looping & 1),
                        m_CapChant.m_FadeIn,
                        m_CapChant.m_StreamParam[0] ? m_CapChant.m_StreamParam : (const char*)0,
                        (Audio::MasterVolume::VOLUME_GROUP)m_CapChant.m_VolGroup);
                }
                else
                {
                    m_CapChant.m_Track.PlayStream(
                        m_PStream.m_OrigStreamId,
                        m_CapChant.m_Volume,
                        (m_CapChant.m_Looping & 1),
                        m_CapChant.m_FadeIn,
                        m_CapChant.m_ExistingFadeOut,
                        m_CapChant.m_StreamParam[0] ? m_CapChant.m_StreamParam : (const char*)0,
                        (Audio::MasterVolume::VOLUME_GROUP)m_CapChant.m_VolGroup);
                }
            }
        }
        else if (m_HasCrowdStream)
        {
            if (m_PStream.m_Active)
            {
                switch (m_PStream.m_StreamId)
                {
                case 0x436E3953:
                    pCounter = &PLAY_RECORD::s_BowserAttackNext;
                    Format = "bowser_attack_%d";
                    break;
                case 0x57CB5A12:
                    pCounter = &PLAY_RECORD::s_SuddenDeathNext;
                    Format = "sudden_death_%d";
                    break;
                default:
                    m_HasCrowdStream = m_PStream.m_StreamId;
                    goto after_switch;
                }

                nlSNPrintf(StreamName, 64, Format, *pCounter);
                *pCounter = *pCounter + 1;
                if (*pCounter == 4)
                {
                    *pCounter = 1;
                }
                m_HasCrowdStream = nlStringLowerHash(StreamName);

            after_switch:
                if (m_PStream.m_Queue)
                {
                    m_PStream.m_Queue = 0;
                    m_PStream.m_Track.QueueStream(
                        m_HasCrowdStream,
                        m_PStream.m_Volume,
                        (m_PStream.m_Looping & 1),
                        m_PStream.m_FadeIn,
                        m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0,
                        (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
                }
                else
                {
                    m_PStream.m_Track.PlayStream(
                        m_HasCrowdStream,
                        m_PStream.m_Volume,
                        (m_PStream.m_Looping & 1),
                        m_PStream.m_FadeIn,
                        m_PStream.m_ExistingFadeOut,
                        m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0,
                        (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
                }
            }
        }

        m_InPause = false;
    }
    else if ((m_PStream.m_StreamId == StreamId)
             || ((StreamId == 0x436E3953) && ((m_PStream.m_StreamId == 0x09451A58) || (m_PStream.m_StreamId == 0xA207B1AE))))
    {
        m_Track.Stop(Fadeout);
        m_HasCrowdStream = 0;
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
 * TODO: 96.6% match - register allocation diffs (r5 vs r7, r3/r4 swaps in switch),
 *   format string extra mr instructions, default case store placement.
 *   Likely -inline deferred vs -inline auto compilation difference.
 */
void PriorityStream::FakeResume(bool checkActive)
{
    char StreamName[64];
    const char* Format;
    unsigned char* pCounter;

    if (m_PStream.m_OrigStreamId)
    {
        if (m_PStream.m_OrigStreamId)
        {
            if (checkActive && !m_CapChant.m_Active)
            {
                // skip
            }
            else if (m_CapChant.m_Queue)
            {
                m_CapChant.m_Queue = 0;
                m_CapChant.m_Track.QueueStream(
                    m_PStream.m_OrigStreamId,
                    m_CapChant.m_Volume,
                    (m_CapChant.m_Looping & 1),
                    m_CapChant.m_FadeIn,
                    m_CapChant.m_StreamParam[0] ? m_CapChant.m_StreamParam : (const char*)0,
                    (Audio::MasterVolume::VOLUME_GROUP)m_CapChant.m_VolGroup);
            }
            else
            {
                m_CapChant.m_Track.PlayStream(
                    m_PStream.m_OrigStreamId,
                    m_CapChant.m_Volume,
                    (m_CapChant.m_Looping & 1),
                    m_CapChant.m_FadeIn,
                    m_CapChant.m_ExistingFadeOut,
                    m_CapChant.m_StreamParam[0] ? m_CapChant.m_StreamParam : (const char*)0,
                    (Audio::MasterVolume::VOLUME_GROUP)m_CapChant.m_VolGroup);
            }
        }
    }
    else if (m_HasCrowdStream)
    {
        if (checkActive && !m_PStream.m_Active)
        {
            // skip
        }
        else
        {
            switch (m_PStream.m_StreamId)
            {
            case 0x436E3953:
                pCounter = &PLAY_RECORD::s_BowserAttackNext;
                Format = "bowser_attack_%d";
                break;
            case 0x57CB5A12:
                pCounter = &PLAY_RECORD::s_SuddenDeathNext;
                Format = "sudden_death_%d";
                break;
            default:
                m_HasCrowdStream = m_PStream.m_StreamId;
                goto after_switch;
            }

            nlSNPrintf(StreamName, 64, Format, *pCounter);
            *pCounter = *pCounter + 1;
            if (*pCounter == 4)
            {
                *pCounter = 1;
            }
            m_HasCrowdStream = nlStringLowerHash(StreamName);

        after_switch:
            if (m_PStream.m_Queue)
            {
                m_PStream.m_Queue = 0;
                m_PStream.m_Track.QueueStream(
                    m_HasCrowdStream,
                    m_PStream.m_Volume,
                    (m_PStream.m_Looping & 1),
                    m_PStream.m_FadeIn,
                    m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0,
                    (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
            }
            else
            {
                m_PStream.m_Track.PlayStream(
                    m_HasCrowdStream,
                    m_PStream.m_Volume,
                    (m_PStream.m_Looping & 1),
                    m_PStream.m_FadeIn,
                    m_PStream.m_ExistingFadeOut,
                    m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0,
                    (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
            }
        }
    }

    m_InPause = false;
}

/**
 * Offset/Address/Size: 0x380 | 0x80157E34 | size: 0x1B0
 * TODO: 95.8% match - switch/control-flow mismatch (r3/r4 compare swap),
 *   format literal loads via addi r0 + mr r5, and default-path branch/store placement.
 */
void PriorityStream::TrackIdleCB()
{
    char StreamName[64];
    unsigned char* pCounter;
    const char* Format;

    if (m_InPause)
    {
        return;
    }

    if (m_PStream.m_OrigStreamId)
    {
        m_PStream.m_OrigStreamId = 0;

        if (m_HasCrowdStream)
        {
            if (m_HasCrowdStream && m_PStream.m_Active)
            {
                int streamId = m_PStream.m_StreamId;
                switch (streamId)
                {
                case 0x436E3953:
                    pCounter = &PLAY_RECORD::s_BowserAttackNext;
                    Format = "bowser_attack_%d";
                    break;
                case 0x57CB5A12:
                    pCounter = &PLAY_RECORD::s_SuddenDeathNext;
                    Format = "sudden_death_%d";
                    break;
                default:
                    m_HasCrowdStream = streamId;
                    goto after_switch;
                }

                nlSNPrintf(StreamName, 64, Format, *pCounter);
                *pCounter = *pCounter + 1;
                if (*pCounter == 4)
                {
                    *pCounter = 1;
                }
                m_HasCrowdStream = nlStringLowerHash(StreamName);

            after_switch:
                if (m_PStream.m_Queue)
                {
                    m_PStream.m_Queue = 0;
                    m_PStream.m_Track.QueueStream(
                        m_HasCrowdStream, m_PStream.m_Volume, (m_PStream.m_Looping & 1), m_PStream.m_FadeIn, m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0, (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
                }
                else
                {
                    m_PStream.m_Track.PlayStream(
                        m_HasCrowdStream, m_PStream.m_Volume, (m_PStream.m_Looping & 1), m_PStream.m_FadeIn, m_PStream.m_ExistingFadeOut, m_PStream.m_StreamParam[0] ? m_PStream.m_StreamParam : (const char*)0, (Audio::MasterVolume::VOLUME_GROUP)m_PStream.m_VolGroup);
                }
            }
            return;
        }
    }

    if (m_HasCrowdStream)
    {
        m_HasCrowdStream = 0;
    }
    CrowdMood::UnlockStream();
}

/**
 * Offset/Address/Size: 0x0 | 0x80157AB4 | size: 0x380
 * TODO: 88.83% match - register allocation and control-flow shape differ in
 *       prologue/state dispatch; function still compiles as void while asm
 *       carries a return register through r30/r3.
 */
namespace GCAudioStreaming
{
class AudioStreamBuffer
{
public:
    unsigned char* m_MRAMBuffer;
    unsigned long m_BufferSize;
    unsigned long m_BufferSamples;
    unsigned long m_StreamId;
    unsigned long m_UpdateOffset;
    class AudioStream* m_pStream;
    unsigned char m_Volume;
    signed char m_Pan;
    unsigned char m_SurroundPan;
    unsigned char m_bLPFOn;
    unsigned short m_LPFFreq;
};

class AudioBufferMgr
{
public:
    void FreeBuffer(GCAudioStreaming::AudioStreamBuffer*);
};

enum STREAM_STATE
{
    SS_New = 0,
    SS_Initd = 1,
    SS_Warming = 2,
    SS_Warm = 3,
    SS_Playing = 4,
};

enum STREAM_FLAG
{
    SF_Play = 0,
    SF_Loop = 1,
    SF_CoolOnStop = 2,
    SF_EndAtUpdate = 3,
    SF_SeriousStop = 4,
};

class AudioStream
{
public:
    virtual ~AudioStream();
    virtual void WarmReadDone(AudioStreamBuffer*);
    void Purge();
    void Destructor();
    virtual void Stop();
    virtual void Warm(bool);
    virtual void GetUpdateReadLength();
    virtual void DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, AudioStreamBuffer*);
    virtual void CancelPendingReads();
    virtual bool SafeToPurge();

    unsigned char m_FlagAtDelete;
    STREAM_STATE m_State;
    unsigned long m_StreamLength;
    unsigned long m_StreamPos;
    AudioStreamBuffer* m_Buffers[2];
    unsigned long m_LastPlayable;
    unsigned long m_Volume;
    unsigned char m_LPFOn;
    unsigned short m_LPFFreq;
    unsigned long m_OldLength;
    AudioBufferMgr& m_BuffMgr;
    unsigned long m_Flags;
    unsigned long m_BufferCount;
};

class StereoAudioStream : public AudioStream
{
};

} // namespace GCAudioStreaming

extern "C"
{
    void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
    void sndStreamDeactivate(unsigned long stid);
    void AttachStream__Q216AudioStreamTrack11StreamTrackFPQ216GCAudioStreaming17StereoAudioStreamQ35Audio12MasterVolume12VOLUME_GROUPUlUlbb(
        AudioStreamTrack::StreamTrack*, GCAudioStreaming::StereoAudioStream*, Audio::MasterVolume::VOLUME_GROUP, unsigned long, unsigned long, bool, bool);
    void StopHead__Q216AudioStreamTrack11StreamTrackFUl(AudioStreamTrack::StreamTrack*, unsigned long);
}

/**
 * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0x380
 * TODO: 96.32% match - ble vs beq at 0xc0, stack offset 0xc vs 0x10 at 0x180 in case 4 free loop
 */
unsigned long PriorityStream::GrabCrowdStream(unsigned long Fadeout)
{
    GCAudioStreaming::StereoAudioStream* pStream;
    unsigned long result = 0;

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
                    AttachStream__Q216AudioStreamTrack11StreamTrackFPQ216GCAudioStreaming17StereoAudioStreamQ35Audio12MasterVolume12VOLUME_GROUPUlUlbb(
                        &m_Track, pStream, (Audio::MasterVolume::VOLUME_GROUP)4, (unsigned long)-1, 0, 0, 0);
                    StopHead__Q216AudioStreamTrack11StreamTrackFUl(&m_Track, Fadeout);
                }
                else
                {
                    pStream->m_Flags &= ~(1 << GCAudioStreaming::SF_Play);

                    if (pStream->m_State == GCAudioStreaming::SS_Playing)
                    {
                        GCAudioStreaming::AudioStreamBuffer* pBuffer;
                        volatile unsigned long i = (unsigned long)(pBuffer = NULL);

                        if (pStream->m_BufferCount > 0)
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
                                unsigned long ci = i + 1;
                                i = ci;
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
                            volatile unsigned long i = (unsigned long)(pBuffer = NULL);

                            pStream->m_Flags = (pStream->m_Flags & ~(1 << GCAudioStreaming::SF_SeriousStop)) | (1 << GCAudioStreaming::SF_SeriousStop);

                            if (pStream->m_BufferCount > 0)
                            {
                                pBuffer = pStream->m_Buffers[0];
                            }

                            while (pBuffer != NULL)
                            {
                                pStream->m_BuffMgr.FreeBuffer(pBuffer);

                                {
                                    unsigned long idx = i;
                                    pStream->m_Buffers[idx] = NULL;
                                    idx = idx + 1;
                                    i = idx;
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
                    volatile unsigned long i = (unsigned long)(pBuffer = NULL);

                    if (pStream->m_BufferCount > 0)
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
                            unsigned long ci = i + 1;
                            i = ci;
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
                        volatile unsigned long i = (unsigned long)(pBuffer = NULL);

                        pStream->m_Flags = (pStream->m_Flags & ~(1 << GCAudioStreaming::SF_SeriousStop)) | (1 << GCAudioStreaming::SF_SeriousStop);

                        if (pStream->m_BufferCount > 0)
                        {
                            pBuffer = pStream->m_Buffers[0];
                        }

                        while (pBuffer != NULL)
                        {
                            pStream->m_BuffMgr.FreeBuffer(pBuffer);

                            {
                                unsigned long idx = i;
                                pStream->m_Buffers[idx] = NULL;
                                idx = idx + 1;
                                i = idx;
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
