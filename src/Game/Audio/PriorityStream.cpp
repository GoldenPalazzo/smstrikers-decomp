#include "Game/Audio/PriorityStream.h"
#include "Game/Audio/CrowdMood.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"

unsigned char PriorityStream::PLAY_RECORD::s_BowserAttackNext = true;
unsigned char PriorityStream::PLAY_RECORD::s_SuddenDeathNext = true;

/**
 * Offset/Address/Size: 0xEA8 | 0x8015895C | size: 0x10
 */
void PriorityStream::Reset()
{
    PLAY_RECORD::s_BowserAttackNext = true;
    PLAY_RECORD::s_SuddenDeathNext = true;
}

inline unsigned long PriorityStream::GetNextStreamId(unsigned long SimpleStreamId)
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

/**
 * Offset/Address/Size: 0xA34 | 0x801584E8 | size: 0x474
 * TODO: 99.09% match - register allocation drift remains in PLAY_RECORD setup
 *   (pSlot r5->r9, volGroup r6->r8) and inlined crowd-id path
 *   (counter pointer and looping-byte loads use shifted registers).
 */
enum Type
{
    _BOOL = 0,
    _INT = 1,
    _FLOAT = 2,
    _STRING = 3,
};

union Value
{
    const char* s;
    int i;
    bool b;
    float f;
};

class Config
{
public:
    struct TagValuePair
    {
        const char* tag;
        Type type;
        Value value;
    };

    static Config& Global();
    TagValuePair& FindTvp(const char*);
    void Set(const char*, bool);
};

template <typename To, typename From>
To LexicalCast(const From&);

class cGame
{
public:
    unsigned char _pad[0x42];
    bool mInSuddenDeath;
};

extern cGame* g_pGame;

void PriorityStream::PlayStream(unsigned long StreamId, float Volume, bool Looping, unsigned long FadeIn, unsigned long ExistingFadeOut, const char* StreamParam)
{
    Config& cfg = Config::Global();
    Config::TagValuePair& tvp = cfg.FindTvp("no_stream");
    bool noStream;

    if (tvp.tag == NULL)
    {
        cfg.Set("no_stream", false);
        noStream = false;
    }
    else if (tvp.type == _BOOL)
    {
        noStream = LexicalCast<bool, bool>(tvp.value.b);
    }
    else if (tvp.type == _INT)
    {
        noStream = LexicalCast<bool, int>(tvp.value.i);
    }
    else if (tvp.type == _FLOAT)
    {
        noStream = LexicalCast<bool, float>(tvp.value.f);
    }
    else if (tvp.type == _STRING)
    {
        noStream = LexicalCast<bool, const char*>(tvp.value.s);
    }
    else
    {
        noStream = false;
    }

    if (noStream == true)
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

    unsigned long active = GrabCrowdStream(ExistingFadeOut);
    unsigned long queue = 1;
    unsigned long volGroup = 0;

    switch (StreamId)
    {
    case 0xE38B5407:
        volGroup = 3;
        break;
    case 0x436E3953:
        volGroup = 1;
        break;
    case 0x09451A58:
        volGroup = 1;
        queue = 0;
        break;
    case 0xA207B1AE:
        volGroup = 1;
        queue = 0;
        break;
    case 0x57CB5A12:
        volGroup = 1;
        break;
    }

    unsigned long* pSlot;
    if (StreamId == 0xE38B5407)
    {
        pSlot = &m_PStream.m_OrigStreamId;
    }
    else
    {
        pSlot = &m_HasCrowdStream;
    }

    if ((StreamId == 0xA207B1AE) && m_PStream.m_OrigStreamId)
    {
        *pSlot = 0;
    }
    else
    {
        *pSlot = StreamId;
        ((PLAY_RECORD*)(pSlot + 1))->m_StreamId = StreamId;
        ((PLAY_RECORD*)(pSlot + 1))->m_Volume = Volume;
        ((PLAY_RECORD*)(pSlot + 1))->m_Looping = Looping;
        ((PLAY_RECORD*)(pSlot + 1))->m_FadeIn = FadeIn;
        ((PLAY_RECORD*)(pSlot + 1))->m_ExistingFadeOut = ExistingFadeOut;
        ((PLAY_RECORD*)(pSlot + 1))->m_VolGroup = (VOLUME_GROUP)volGroup;
        ((PLAY_RECORD*)(pSlot + 1))->m_Queue = active;
        ((PLAY_RECORD*)(pSlot + 1))->m_Active = queue;

        if (StreamParam)
        {
            nlStrNCpy<char>(((PLAY_RECORD*)(pSlot + 1))->m_StreamParam, StreamParam, 32);
        }
        else
        {
            ((PLAY_RECORD*)(pSlot + 1))->m_StreamParam[0] = '\0';
        }
    }

    if (m_PStream.m_OrigStreamId)
    {
        if (!m_PStream.m_OrigStreamId)
        {
            goto end_playstream;
        }
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
    else if (m_HasCrowdStream)
    {
        m_HasCrowdStream = GetNextStreamId(m_PStream.m_StreamId);

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

end_playstream:
    m_InPause = false;
}

/**
 * Offset/Address/Size: 0x79C | 0x80158250 | size: 0x298
 * TODO: 99.52% match - r3/r4 register swap in dead CapChant QueueStream path
 */
void PriorityStream::Stop(unsigned long StreamId, unsigned long Fadeout)
{
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
                m_HasCrowdStream = GetNextStreamId(m_PStream.m_StreamId);

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
 * TODO: 99.34% match - remaining diffs are register selection in OrigStream/
 * PStream queue-play dispatch and one instruction ordering swap in PStream
 * play path setup.
 */
void PriorityStream::FakeResume(bool checkActive)
{
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
            m_HasCrowdStream = GetNextStreamId(m_PStream.m_StreamId);
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
 * TODO: 99.35% match - queued path loads looping flag through r3 instead of r4;
 *   play path extracts FadeIn before loading the track pointer.
 */
void PriorityStream::TrackIdleCB()
{
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
                m_HasCrowdStream = GetNextStreamId(m_PStream.m_StreamId);

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
}

/**
 * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0x380
 * TODO: 99.97% match - stack offsets swap between first free loop and second volume loop
 */
unsigned long PriorityStream::GrabCrowdStream(unsigned long Fadeout)
{
    GCAudioStreaming::StereoAudioStream* pStream;
    unsigned long result = 0;
    unsigned long zero = 0;

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
                        volatile unsigned long i = (unsigned long)(pBuffer = NULL);

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

                            if (pStream->m_BufferCount > zero)
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

                        if (pStream->m_BufferCount > zero)
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
