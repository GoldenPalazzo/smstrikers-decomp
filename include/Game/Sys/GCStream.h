#ifndef _GCSTREAM_H_
#define _GCSTREAM_H_

#include "NL/nlArrayAllocator.h"
#include "NL/nlFile.h"

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

class AudioStreamBuffer
{
public:
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
    /* 0x1B */ unsigned char m_bLPFOn;        // offset 0x1B, size 0x1
    /* 0x1C */ unsigned short m_LPFFreq;      // offset 0x1C, size 0x2
}; // total size: 0x20

class AudioBufferMgr
{
public:
    void Init(unsigned long);
    void CreateBuffers(unsigned long);
    void DeleteBuffers();
    void FreeBuffer(GCAudioStreaming::AudioStreamBuffer*);

    /* 0x000 */ unsigned long m_PoolSize;         // offset 0x0, size 0x4
    /* 0x004 */ unsigned char* m_MRAMBuffer;      // offset 0x4, size 0x4
    /* 0x008 */ AudioStreamBuffer m_Buffers[8];   // offset 0x8, size 0x100
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
    AudioStream(AudioBufferMgr& mgr, unsigned long bufCount);
    virtual ~AudioStream() { };
    virtual void Warm(bool) { };
    virtual bool SafeToPurge() { return false; };
    virtual void Purge();
    virtual void DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, AudioStreamBuffer*) { };
    virtual unsigned long GetUpdateReadLength() { return 0; };
    virtual void CancelPendingReads() { };
    virtual void WarmReadDone(AudioStreamBuffer*);
    void Stop();
    void Destructor();

    static void _HdrReadCB(nlFile*, void*, unsigned int, unsigned long);
    static void _WarmReadCB(nlFile*, void*, unsigned int, unsigned long);
    static void _UpdateReadCB(nlFile*, void*, unsigned int, unsigned long);

    class READ_CB_INFO
    {
    public:
        /* 0x0 */ READ_CB_INFO* m_next;
        /* 0x4 */ class AudioStreamBuffer* pBuffer;

        static nlArrayAllocator<READ_CB_INFO> s_AllocPool;
    };

    /* 0x04 */ unsigned char m_FlagAtDelete;
    /* 0x08 */ STREAM_STATE m_State;
    /* 0x0C */ unsigned long m_StreamLength;
    /* 0x10 */ unsigned long m_StreamPos;
    /* 0x14 */ AudioStreamBuffer* m_Buffers[2];
    /* 0x1C */ unsigned long m_LastPlayable;
    /* 0x20 */ unsigned long m_Volume;
    /* 0x24 */ unsigned char m_LPFOn;
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
    static void _AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long));
    virtual void CancelPendingReads();
    virtual unsigned long GetUpdateReadLength();
    virtual void Warm(bool);
    virtual void DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*);
    virtual bool SafeToPurge();
    virtual void Purge();

    /* 0x38 */ class nlFile* m_pFile;
    /* 0x3C */ unsigned long m_UpdateLen;
}; // total size: 0x40

class StereoAudioStream : public AudioStream
{
public:
    StereoAudioStream(AudioBufferMgr& mgr);
    virtual ~StereoAudioStream();
    virtual unsigned long GetUpdateReadLength();
    static void _InterleavedHdrReadCB(nlFile*, void*, unsigned int, unsigned long);
    static void _AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long));
    virtual void CancelPendingReads();
    virtual void Warm(bool);
    void InterleavedHdrReadCB(nlFile*, void*, unsigned int);
    virtual void DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*);
    virtual bool SafeToPurge();
    virtual void Purge();

    /* 0x38 */ nlFile* m_pFile;
    /* 0x3C */ unsigned long m_Interleave;
}; // total size: 0x40

} // namespace GCAudioStreaming

#endif // _GCSTREAM_H_
