#ifndef _GCSTREAM_H_
#define _GCSTREAM_H_

#include "NL/nlArrayAllocator.h"
#include "NL/nlFile.h"

namespace GCAudioStreaming
{

class AudioStreamBuffer
{
public:
    void _UpdateHandler(void*, unsigned long, void*, unsigned long, unsigned long);

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
    virtual void WarmReadDone(AudioStreamBuffer*);
    void Purge();
    void Destructor();
    virtual void Stop();
    virtual void Warm(bool) { };
    virtual void GetUpdateReadLength() { };
    virtual void DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, AudioStreamBuffer*) { };
    virtual void CancelPendingReads() { };
    virtual bool SafeToPurge() { return false; };

    void _HdrReadCB(nlFile*, void*, unsigned int, unsigned long);
    void _WarmReadCB(nlFile*, void*, unsigned int, unsigned long);
    void _UpdateReadCB(nlFile*, void*, unsigned int, unsigned long);

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
    virtual void GetUpdateReadLength();
    virtual void Warm(bool);
    virtual void DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*);
    virtual bool SafeToPurge();

    /* 0x38 */ class nlFile* m_pFile;
    /* 0x3C */ unsigned long m_UpdateLen;
}; // total size: 0x40

class StereoAudioStream : public AudioStream
{
public:
    StereoAudioStream(AudioBufferMgr& mgr);
    virtual ~StereoAudioStream();
    virtual void GetUpdateReadLength();
    void _InterleavedHdrReadCB(nlFile*, void*, unsigned int, unsigned long);
    static void _AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long));
    virtual void CancelPendingReads();
    virtual void Warm(bool);
    void InterleavedHdrReadCB(nlFile*, void*, unsigned int);
    virtual void DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*);
    virtual bool SafeToPurge();

    /* 0x38 */ nlFile* m_pFile;
    /* 0x3C */ unsigned long m_Interleave;
}; // total size: 0x40

} // namespace GCAudioStreaming

#endif // _GCSTREAM_H_
