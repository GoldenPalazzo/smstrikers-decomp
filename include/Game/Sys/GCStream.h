#ifndef _GCSTREAM_H_
#define _GCSTREAM_H_

#include "NL/nlArrayAllocator.h"
#include "NL/nlFile.h"
#include "NL/nlFileGC.h"
#include "musyx/musyx.h"
#include "dolphin/os.h"

// Declarations needed by the inline (weak) AudioStream methods defined below,
// so this header compiles standalone in every includer.
void nlServiceFileSystem();
extern int nlPrintf(const char*, ...);
extern "C" void sndStreamLPFParameter(unsigned long, bool, unsigned long);

struct sDSPADPCM
{
    /* 0x00 */ unsigned long num_samples;
    /* 0x04 */ unsigned long num_adpcm_nibbles;
    /* 0x08 */ unsigned long sample_rate;
    /* 0x0C */ unsigned short loop_flag;
    /* 0x0E */ unsigned short format;
    /* 0x10 */ unsigned long sa;
    /* 0x14 */ unsigned long ea;
    /* 0x18 */ unsigned long ca;
    /* 0x1C */ unsigned short coef[16];
    /* 0x3C */ unsigned short gain;
    /* 0x3E */ unsigned short ps;
    /* 0x40 */ unsigned short yn1;
    /* 0x42 */ unsigned short yn2;
    /* 0x44 */ unsigned short lps;
    /* 0x46 */ unsigned short lyn1;
    /* 0x48 */ unsigned short lyn2;
    /* 0x4A */ unsigned short pad[11];
}; // total size: 0x60

struct INTERLEAVED_ADPCM_HEADER
{
    /* 0x0 */ char Thumbprint[4];
    /* 0x4 */ unsigned long Interleave;
    /* 0x8 */ unsigned long StreamLength;
}; // total size: 0xC

namespace GCAudioStreaming
{

class AudioStream;

class AudioStreamBuffer
{
public:
    AudioStreamBuffer();
    void SetVolume(unsigned long volume);
    void SetLPF(bool on)
    {
        if (on != m_bLPFOn)
        {
            sndStreamLPFParameter(m_StreamId, on, m_LPFFreq);
            m_bLPFOn = on;
        }
    }
    void SetLPF(unsigned short frequency)
    {
        if (m_bLPFOn)
        {
            sndStreamLPFParameter(m_StreamId, m_bLPFOn, frequency);
        }
        m_LPFFreq = frequency;
    }
    void Reset(AudioStream* pStream = NULL)
    {
        m_pStream = pStream;
        m_UpdateOffset = 0;
        m_Volume = 0x7F;
        m_Pan = 0x40;
    }
    void Update(unsigned long, unsigned long);
    unsigned long UpdateHandler(void*, unsigned long, void*, unsigned long);
    void DoUpdate(unsigned long, unsigned long);
    static unsigned long _UpdateHandler(void*, unsigned long, void*, unsigned long, unsigned long);

    /* 0x00 */ unsigned char* m_MRAMBuffer;   // offset 0x0, size 0x4
    /* 0x04 */ unsigned long m_BufferSize;    // offset 0x4, size 0x4
    /* 0x08 */ unsigned long m_BufferSamples; // offset 0x8, size 0x4
    /* 0x0C */ unsigned long m_StreamId;      // offset 0xC, size 0x4
    /* 0x10 */ unsigned long m_UpdateOffset;  // offset 0x10, size 0x4
    /* 0x14 */ class AudioStream* m_pStream;  // offset 0x14, size 0x4
    /* 0x18 */ unsigned char m_Volume;        // offset 0x18, size 0x1
    /* 0x19 */ signed char m_Pan;             // offset 0x19, size 0x1
    /* 0x1A */ unsigned char m_SurroundPan;   // offset 0x1A, size 0x1
    /* 0x1B */ bool m_bLPFOn;                 // offset 0x1B, size 0x1
    /* 0x1C */ unsigned short m_LPFFreq;      // offset 0x1C, size 0x2
}; // total size: 0x20

enum BUFFER_ALLOC_STATE
{
    BAS_Busy = 0,
    BAS_Free = 1,
};

class AudioBufferMgr
{
public:
    AudioBufferMgr();
    void* GetADPCMHdr();
    void SetBufferState(unsigned long index, BUFFER_ALLOC_STATE state)
    {
        m_BuffersFree =
            (m_BuffersFree & ~(1 << index))
            | (state << index);
    }
    void Init(unsigned long);
    void CreateBuffers(unsigned long);
    void DeleteBuffers();
    void FreeBuffer(GCAudioStreaming::AudioStreamBuffer*);
    AudioStreamBuffer* GetFreeBuffer(GCAudioStreaming::AudioStream*);

    static const unsigned long MAX_BUFFERS = 8;

    /* 0x000 */ unsigned long m_PoolSize;         // offset 0x0, size 0x4
    /* 0x004 */ unsigned char* m_MRAMBuffer;      // offset 0x4, size 0x4
    /* 0x008 */ AudioStreamBuffer m_Buffers[MAX_BUFFERS]; // offset 0x8, size 0x100
    /* 0x108 */ unsigned char m_ADPCMHdrMem[128]; // offset 0x108, size 0x80
    /* 0x188 */ unsigned long m_BuffersFree;      // offset 0x188, size 0x4
    /* 0x18C */ unsigned long m_BufferCount;      // offset 0x18C, size 0x4
    /* 0x190 */ unsigned long m_BufferSize;       // offset 0x190, size 0x4
}; // total size: 0x194

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
    AudioStreamBuffer* GetBuffer(unsigned long index)
    {
        AudioStreamBuffer* buffer;
        if (index < m_BufferCount)
        {
            buffer = m_Buffers[index];
        }
        else
        {
            buffer = NULL;
        }
        return buffer;
    }
    void SetVolume(unsigned long volume)
    {
        int clampedVolume = ((u32)(u8)volume <= 0x7Fu) ? volume : 0x7F;

        if (m_State >= SS_Warming)
        {
            AudioStreamBuffer* bufferIndex = NULL;
            AudioStreamBuffer* buffer = GetBuffer((unsigned long)bufferIndex);

            while (buffer != NULL)
            {
                buffer->m_Volume = (u8)clampedVolume;
                sndStreamMixParameterEx(buffer->m_StreamId, buffer->m_Volume, buffer->m_Pan, buffer->m_SurroundPan, 0, 0);
                ((unsigned long&)bufferIndex)++;
                buffer = GetBuffer((unsigned long)bufferIndex);
            }
        }

        m_Volume = (u8)clampedVolume;
    }
    void SetLPF(bool on)
    {
        if (m_State >= SS_Warming)
        {
            AudioStreamBuffer* bufferIndex = NULL;
            AudioStreamBuffer* buffer = GetBuffer((unsigned long)bufferIndex);

            while (buffer != NULL)
            {
                buffer->SetLPF(on);
                ((unsigned long&)bufferIndex)++;
                buffer = GetBuffer((unsigned long)bufferIndex);
            }
        }

        m_LPFOn = on;
    }
    void SetLPF(unsigned short frequency)
    {
        if (m_State >= SS_Warming)
        {
            AudioStreamBuffer* bufferIndex = NULL;
            AudioStreamBuffer* buffer = GetBuffer((unsigned long)bufferIndex);

            while (buffer != NULL)
            {
                buffer->SetLPF(frequency);
                ((unsigned long&)bufferIndex)++;
                buffer = GetBuffer((unsigned long)bufferIndex);
            }
        }

        m_LPFFreq = frequency;
    }
    void SetLoop(bool loop)
    {
        m_Flags = (m_Flags & ~(1 << SF_Loop)) | ((unsigned long)loop << SF_Loop);
    }
    void Play(bool coolOnStop)
    {
        m_Flags = (m_Flags & ~(1 << SF_CoolOnStop)) | ((unsigned long)coolOnStop << SF_CoolOnStop);

        switch (m_State)
        {
        case SS_Initd:
            m_Flags = (m_Flags & ~(1 << SF_Play)) | (1 << SF_Play);
            Warm(coolOnStop);
            break;
        case SS_Warming:
            m_Flags = (m_Flags & ~(1 << SF_Play)) | (1 << SF_Play);
            break;
        case SS_Warm:
        {
            AudioStreamBuffer* bufferIndex = NULL;
            AudioStreamBuffer* buffer = GetBuffer((unsigned long)bufferIndex);

            while (buffer != NULL)
            {
                sndStreamActivate(buffer->m_StreamId);
                ((unsigned long&)bufferIndex)++;
                buffer = GetBuffer((unsigned long)bufferIndex);
            }

            m_State = SS_Playing;
            break;
        }
        }
    }
    void ResetBuffers()
    {
        AudioStreamBuffer* buf;
        AudioStreamBuffer* BufferIndex = NULL;

        buf = GetBuffer(0);

        while (buf != NULL)
        {
            m_Buffers[(unsigned long)BufferIndex] = NULL;
            ((unsigned long&)BufferIndex)++;
            buf = GetBuffer((unsigned long)BufferIndex);
        }
    }
    AudioStream(AudioBufferMgr& mgr, unsigned long bufCount);
    virtual ~AudioStream() { };
    virtual void Warm(bool) = 0;
    virtual bool SafeToPurge() = 0;
    virtual void Purge()
    {
        FORCE_DONT_INLINE;
        m_State = SS_New;
    }
    virtual unsigned long DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, AudioStreamBuffer*) = 0;
    virtual unsigned long GetUpdateReadLength() = 0;
    virtual void CancelPendingReads() = 0;
    virtual void WarmReadDone(AudioStreamBuffer* pBuffer)
    {
        FORCE_DONT_INLINE;
        if (m_Buffers[m_BufferCount - 1] != pBuffer)
        {
            return;
        }

        m_State = SS_Warm;

        if (!(m_Flags & (1 << SF_Play)))
        {
            return;
        }

        if (pBuffer != m_Buffers[m_BufferCount - 1])
        {
            return;
        }

        m_Flags &= ~(1 << SF_Play);

        unsigned long start = 0;
        AudioStreamBuffer* buf;
        volatile unsigned long i = (unsigned long)(buf = NULL);
        if (start < m_BufferCount)
        {
            buf = m_Buffers[0];
        }
        while (buf != NULL)
        {
            sndStreamActivate(buf->m_StreamId);
            unsigned long ci = i + 1;
            i = ci;
            if (ci < m_BufferCount)
            {
                buf = m_Buffers[ci];
            }
            else
            {
                buf = NULL;
            }
        }

        m_State = SS_Playing;
    }
    void Cool()
    {
        if (m_Flags & (1 << SF_CoolOnStop))
        {
            m_Flags &= ~(1 << SF_CoolOnStop);
            if (m_State > SS_Initd)
            {
                AudioStreamBuffer* buffer = NULL;
                AudioStreamBuffer* bufferIndex = NULL;
                m_Flags =
                    (m_Flags & ~(1 << SF_SeriousStop))
                    | (1 << SF_SeriousStop);

                if (m_BufferCount > (unsigned long)buffer)
                {
                    buffer = m_Buffers[0];
                }
                else
                {
                    buffer = NULL;
                }
                while (buffer != NULL)
                {
                    m_BuffMgr.FreeBuffer(buffer);
                    m_Buffers[(unsigned long)bufferIndex] = NULL;
                    ((unsigned long&)bufferIndex)++;
                    buffer = GetBuffer((unsigned long)bufferIndex);
                }

                m_State = SS_Initd;
            }
        }
    }
    void Stop();
    void StopPlaying()
    {
        m_Flags &= ~(1 << SF_Play);
        if (m_State == SS_Playing)
        {
            AudioStreamBuffer* pBuffer;
            AudioStreamBuffer* bufferIndex = NULL;
            pBuffer = GetBuffer((unsigned long)bufferIndex);
            while (pBuffer != NULL)
            {
                pBuffer->m_Volume = 0;
                sndStreamMixParameterEx(
                    pBuffer->m_StreamId,
                    pBuffer->m_Volume,
                    pBuffer->m_Pan,
                    pBuffer->m_SurroundPan,
                    0,
                    0);
                sndStreamDeactivate(pBuffer->m_StreamId);
                m_State = SS_Warm;

                ((unsigned long&)bufferIndex)++;
                pBuffer = GetBuffer((unsigned long)bufferIndex);
            }
            m_StreamPos = 0;
            m_State = SS_Warm;
        }
    }
    void Destructor()
    {
        FORCE_DONT_INLINE;
        m_Flags = (m_Flags & ~(1 << SF_SeriousStop)) | (1 << SF_SeriousStop);
        m_Flags &= ~(1 << SF_Play);

        if (m_State == SS_Playing)
        {
            AudioStreamBuffer* pBuffer;
            volatile unsigned long i = (unsigned long)(pBuffer = NULL);
            unsigned long start = 0;

            if (start < m_BufferCount)
            {
                pBuffer = m_Buffers[0];
            }

            while (pBuffer != NULL)
            {
                pBuffer->m_Volume = 0;
                sndStreamMixParameterEx(pBuffer->m_StreamId, pBuffer->m_Volume, pBuffer->m_Pan, pBuffer->m_SurroundPan, 0, 0);
                sndStreamDeactivate(pBuffer->m_StreamId);
                m_State = SS_Warm;

                {
                    unsigned long ci = i + 1;
                    i = ci;
                    if (ci < m_BufferCount)
                    {
                        pBuffer = m_Buffers[ci];
                    }
                    else
                    {
                        pBuffer = NULL;
                    }
                }
            }

            m_StreamPos = 0;
            m_State = SS_Warm;
        }

        CancelPendingReads();

        {
            volatile unsigned long main_i;
            volatile unsigned long cool_i;

            if (m_Flags & (1 << SF_CoolOnStop))
            {
                m_Flags &= ~(1 << SF_CoolOnStop);

                if (m_State > SS_Initd)
                {
                    AudioStreamBuffer* pBuffer;
                    cool_i = (unsigned long)(pBuffer = NULL);

                    m_Flags = (m_Flags & ~(1 << SF_SeriousStop)) | (1 << SF_SeriousStop);
                    unsigned long start = 0;

                    if (start < m_BufferCount)
                    {
                        pBuffer = m_Buffers[0];
                    }

                    while (pBuffer != NULL)
                    {
                        m_BuffMgr.FreeBuffer(pBuffer);

                        {
                            unsigned long idx = cool_i;
                            m_Buffers[idx] = NULL;
                            idx = idx + 1;
                            cool_i = idx;
                            if (idx < m_BufferCount)
                            {
                                pBuffer = m_Buffers[idx];
                            }
                            else
                            {
                                pBuffer = NULL;
                            }
                        }
                    }

                    m_State = SS_Initd;
                }
            }

            if (m_State > SS_Initd)
            {
                AudioStreamBuffer* pBuffer;
                main_i = (unsigned long)(pBuffer = NULL);

                m_Flags = (m_Flags & ~(1 << SF_SeriousStop)) | (1 << SF_SeriousStop);
                unsigned long start = 0;

                if (start < m_BufferCount)
                {
                    pBuffer = m_Buffers[0];
                }

                while (pBuffer != NULL)
                {
                    m_BuffMgr.FreeBuffer(pBuffer);

                    {
                        unsigned long idx = main_i;
                        m_Buffers[idx] = NULL;
                        idx = idx + 1;
                        main_i = idx;
                        if (idx < m_BufferCount)
                        {
                            pBuffer = m_Buffers[idx];
                        }
                        else
                        {
                            pBuffer = NULL;
                        }
                    }
                }

                m_State = SS_Initd;
            }
        }

        long long startTime = OSGetTime();

        while (!SafeToPurge())
        {
            nlServiceFileSystem();
            OSYieldThread();
            long long elapsed = OSGetTime() - startTime;
            if (OSTicksToMilliseconds(elapsed) > 250)
            {
                nlPrintf("WARNING! Breaking out of audio stream d'tor early!\n");
                break;
            }
        }

        Purge();
    }

    static void _HdrReadCB(nlFile*, void*, unsigned int, unsigned long);
    static void _WarmReadCB(nlFile*, void*, unsigned int, unsigned long);
    static void _UpdateReadCB(nlFile*, void*, unsigned int, unsigned long);

    class READ_CB_INFO
    {
    public:
        /* 0x0 */ AudioStream* pStream;
        /* 0x4 */ class AudioStreamBuffer* pBuffer;

        static nlArrayAllocator<READ_CB_INFO> s_AllocPool;
    };

    static const unsigned long MAX_BUFFERS = 2;

    /* 0x04 */ unsigned char m_FlagAtDelete;
    /* 0x08 */ STREAM_STATE m_State;
    /* 0x0C */ unsigned long m_StreamLength;
    /* 0x10 */ unsigned long m_StreamPos;
    /* 0x14 */ AudioStreamBuffer* m_Buffers[MAX_BUFFERS];
    /* 0x1C */ unsigned long m_LastPlayable;
    /* 0x20 */ unsigned long m_Volume;
    /* 0x24 */ bool m_LPFOn;
    /* 0x26 */ unsigned short m_LPFFreq;
    /* 0x28 */ unsigned long m_OldLength;
    /* 0x2C */ AudioBufferMgr& m_BuffMgr;
    /* 0x30 */ unsigned long m_Flags;
    /* 0x34 */ unsigned long m_BufferCount;
}; // total size: 0x38

class MonoAudioStream : public AudioStream
{
public:
    MonoAudioStream(AudioBufferMgr& mgr);
    void Open(const char* filename)
    {
        m_StreamLength = 0;
        m_OldLength = 0;
        m_StreamPos = 0;

        ResetBuffers();

        m_LastPlayable = 0;
        m_Flags = 0;
        m_Volume = 64;
        m_LPFOn = 0;
        m_LPFFreq = 0x3FFF;
        m_pFile = nlOpen(filename);
        m_State = SS_Initd;
    }
    virtual ~MonoAudioStream();
    static void _AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long));
    virtual void CancelPendingReads();
    virtual unsigned long GetUpdateReadLength();
    virtual void Warm(bool);
    virtual unsigned long DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*);
    virtual bool SafeToPurge();
    virtual void Purge();

    /* 0x38 */ class nlFile* m_pFile;
    /* 0x3C */ unsigned long m_UpdateLen;
}; // total size: 0x40

class StereoAudioStream : public AudioStream
{
public:
    StereoAudioStream(AudioBufferMgr& mgr);
    void ReadHeader(unsigned long buffer);
    void Open(const char* filename)
    {
        m_StreamLength = 0;
        m_OldLength = 0;
        m_StreamPos = 0;

        ResetBuffers();

        m_LastPlayable = 0;
        m_Flags = 0;
        m_Volume = 64;
        m_LPFOn = 0;
        m_LPFFreq = 0x3FFF;
        m_pFile = nlOpen(filename);
        m_State = SS_Initd;
    }
    virtual ~StereoAudioStream()
    {
        FORCE_DONT_INLINE;
        Destructor();
    }
    virtual unsigned long GetUpdateReadLength();
    static void _InterleavedHdrReadCB(nlFile*, void*, unsigned int, unsigned long);
    static void _AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long));
    virtual void CancelPendingReads();
    virtual void Warm(bool);
    void InterleavedHdrReadCB(nlFile*, void*, unsigned int);
    virtual unsigned long DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*);
    virtual bool SafeToPurge();
    virtual void Purge();

    /* 0x38 */ nlFile* m_pFile;
    /* 0x3C */ unsigned long m_Interleave;
}; // total size: 0x40

} // namespace GCAudioStreaming

#endif // _GCSTREAM_H_
