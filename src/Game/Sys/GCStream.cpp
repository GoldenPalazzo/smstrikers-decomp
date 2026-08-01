// LINKABLE NOTE (2026-07-31, notes 0045): our object for this TU weak-emits
// six functions retail's object does not have (~AudioStream,
// ~StereoAudioStream, Purge, WarmReadDone, Destructor, and a local
// nlCountBits at .text offset 0) and misses retail's GLOBAL
// __vt__Q216GCAudioStreaming15MonoAudioStream - retail defined those bodies
// in a different TU. Colour-inert for register matching, but a real
// reconstruction defect for the future /linkable task. Declaration-only
// shadow headers that reproduce retail's emission set are preserved under
// tmp/tasks/monowarm-100-20260731-1/candidates/r13*/.
#include "Game/Sys/GCStream.h"
#include "NL/nlAlgorithm.h"
#include "NL/nlFileGC.h"
#include "NL/nlMemory.h"

extern void ___blank(const char*, ...);

struct SND_ADPCMSTREAM_INFO;

extern "C"
{
    void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
    void sndStreamDeactivate(unsigned long stid);
    void sndStreamFree(unsigned long stid);
    void sndStreamARAMUpdate(unsigned long stid, unsigned long off1, unsigned long len1, unsigned long off2, unsigned long len2);
    void sndStreamFrq(unsigned long stid, unsigned long frq);
    void sndStreamADPCMParameter(unsigned long stid, SND_ADPCMSTREAM_INFO* adpcmInfo);
    unsigned long sndStreamAllocEx(unsigned char prio, void* buffer, unsigned long samples, unsigned long frq, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb, unsigned char studio, unsigned long flags, unsigned long (*updateFunction)(void*, unsigned long, void*, unsigned long, unsigned long), unsigned long user, SND_ADPCMSTREAM_INFO* adpcmInfo);
}

namespace GCAudioStreaming
{

inline AudioStreamBuffer* AudioBufferMgr::GetFreeBuffer(
    AudioStream* pStream)
{
    unsigned long buffer;
    for (buffer = 0; buffer < m_BufferCount; buffer++)
    {
        if ((int)(bool)(m_BuffersFree & (1 << buffer)) == 1)
        {
            SetBufferState(buffer, BAS_Busy);
            m_Buffers[buffer].Reset(pStream);
            ___blank(
                "After buffer alloc there are %d availible\n",
                nlCountBits(m_BuffersFree));
            return &m_Buffers[buffer];
        }
    }
    return 0;
}

inline void AudioStreamBuffer::SetVolume(
    unsigned long volume)
{
    m_Volume = (unsigned char)volume;
    sndStreamMixParameterEx(
        m_StreamId, m_Volume, m_Pan, m_SurroundPan, 0, 0);
}

} // namespace GCAudioStreaming

unsigned char ARRAY_ALLOCATOR_MEMORY_class_name_s_AllocPool[sizeof(GCAudioStreaming::AudioStream::READ_CB_INFO) * 32];

nlArrayAllocator<GCAudioStreaming::AudioStream::READ_CB_INFO> GCAudioStreaming::AudioStream::READ_CB_INFO::s_AllocPool(
    (GCAudioStreaming::AudioStream::READ_CB_INFO*)ARRAY_ALLOCATOR_MEMORY_class_name_s_AllocPool, 32);

// /**
//  * Offset/Address/Size: 0x4AC | 0x801C9630 | size: 0x140
//  */
// void 0x8028D52C..0x8028D530 | size: 0x4
// {
// }

/**
 * Offset/Address/Size: 0x49C | 0x801C9620 | size: 0x10
 */
void GCAudioStreaming::MonoAudioStream::_AsyncCancelCB(nlFile*, void*, unsigned int, unsigned long uParam, void (*)(nlFile*, void*, unsigned int, unsigned long))
{
    AudioStream::READ_CB_INFO* pCBInfo = (AudioStream::READ_CB_INFO*)uParam;
    AudioStream::READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
}

/**
 * Offset/Address/Size: 0x470 | 0x801C95F4 | size: 0x2C
 */
void GCAudioStreaming::MonoAudioStream::CancelPendingReads()
{
    nlCancelPendingAsyncReads(m_pFile, &_AsyncCancelCB);
}

/**
 * Offset/Address/Size: 0x434 | 0x801C95B8 | size: 0x3C
 */
unsigned long GCAudioStreaming::MonoAudioStream::GetUpdateReadLength()
{
    unsigned long streamPos = m_StreamPos;
    unsigned long length = m_UpdateLen;
    if (streamPos + length > m_StreamLength)
    {
        unsigned long aligned = (m_StreamLength - streamPos + 0x1f) & ~0x1f;
        if (aligned)
        {
            length = aligned;
        }
        return length;
    }
    return length;
}

/**
 * Offset/Address/Size: 0x2E4 | 0x801C9468 | size: 0x150
 */
unsigned long GCAudioStreaming::AudioStreamBuffer::_UpdateHandler(
    void* pDataA, unsigned long LengthA, void* pDataB,
    unsigned long LengthB, unsigned long user)
{
    AudioStreamBuffer* pBuffer = (AudioStreamBuffer*)user;

    return pBuffer->UpdateHandler(pDataA, LengthA, pDataB, LengthB);
}

inline unsigned long GCAudioStreaming::AudioStreamBuffer::UpdateHandler(
    void*, unsigned long LengthA, void*, unsigned long LengthB)
{
    if (!LengthA && !LengthB)
    {
        return 0;
    }

    unsigned long StreamChunkSize = m_pStream->GetUpdateReadLength();

    if ((LengthA + LengthB) / 14 * 8 < StreamChunkSize)
    {
        return 0;
    }

    unsigned long ChunkAOffset = m_UpdateOffset;
    m_UpdateOffset = ChunkAOffset + StreamChunkSize;
    unsigned long ChunkASize;
    unsigned long ChunkBSize;

    if (m_UpdateOffset >= m_BufferSize)
    {
        unsigned long wrapped = m_UpdateOffset - m_BufferSize;
        m_UpdateOffset = wrapped;
        ChunkASize = StreamChunkSize - wrapped;
        ChunkBSize = m_UpdateOffset & ~31;
    }
    else
    {
        ChunkASize = StreamChunkSize;
        ChunkBSize = 0;
    }

    ___blank(m_pStream->m_Buffers[0] == this ? "Left " : "Right ");
    ___blank("Asking for %d %d and %d %d\n", ChunkAOffset, ChunkASize, 0, ChunkBSize);

    m_pStream->DoUpdateRead(
        ChunkAOffset, ChunkASize, 0, ChunkBSize, this);

    return (ChunkASize + ChunkBSize) / 8 * 14;
}

inline void GCAudioStreaming::AudioStreamBuffer::DoUpdate(
    unsigned long LengthA, unsigned long LengthB)
{
    UpdateHandler(
        0, ((LengthA + LengthB) >> 3) * 14, 0, 0);
}

/**
 * Offset/Address/Size: 0x2B8 | 0x801C943C | size: 0x2C
 */
unsigned long GCAudioStreaming::StereoAudioStream::GetUpdateReadLength()
{
    unsigned long len = m_Interleave;
    if (m_StreamPos + len > m_StreamLength)
    {
        len = (m_StreamLength - m_StreamPos + 0x1f) & ~0x1f;
    }
    return len;
}

/**
 * Offset/Address/Size: 0x27C | 0x801C9400 | size: 0x3C
 */
void GCAudioStreaming::StereoAudioStream::_InterleavedHdrReadCB(nlFile* pFile, void* pData, unsigned int Length, unsigned long User)
{
    ((StereoAudioStream*)User)->InterleavedHdrReadCB(pFile, pData, Length);
}

/**
 * Offset/Address/Size: 0x238 | 0x801C93BC | size: 0x44
 */
void GCAudioStreaming::StereoAudioStream::_AsyncCancelCB(nlFile*, void* buffer, unsigned int, unsigned long uParam, void (*Callback)(nlFile*, void*, unsigned int, unsigned long))
{
    if (Callback == &_InterleavedHdrReadCB)
    {
        nlFree(buffer);
    }
    else
    {
        AudioStream::READ_CB_INFO* pCBInfo = (AudioStream::READ_CB_INFO*)uParam;
        AudioStream::READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
    }
}

/**
 * Offset/Address/Size: 0x20C | 0x801C9390 | size: 0x2C
 */
void GCAudioStreaming::StereoAudioStream::CancelPendingReads()
{
    nlCancelPendingAsyncReads(m_pFile, &_AsyncCancelCB);
}

/**
 * Offset/Address/Size: 0x1984 | 0x801C9134 | size: 0x50
 */
void ___blank(const char*, ...)
{
}

/**
 * Offset/Address/Size: 0x187C | 0x801C902C | size: 0x108
 */
void GCAudioStreaming::AudioStream::_HdrReadCB(nlFile* pFile, void* pData, unsigned int Length, unsigned long User)
{
    READ_CB_INFO* pCBInfo = (READ_CB_INFO*)User;

    bool serious;
    if (pCBInfo->pStream->m_Flags & (1 << SF_SeriousStop))
    {
        switch (pCBInfo->pStream->m_State)
        {
        case SS_New:
        case SS_Initd:
            break;
        case SS_Warming:
            pCBInfo->pStream->m_State = SS_Warm;
            break;
        case SS_Warm:
        case SS_Playing:
            break;
        }
        serious = true;
    }
    else
    {
        serious = false;
    }

    if (serious)
    {
        READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
    }
    else
    {
        AudioStreamBuffer* pBuffer;
        unsigned long streamLength;
        AudioStream* pStream;
        sDSPADPCM* pHdr = (sDSPADPCM*)((unsigned long)pData - Length);

        pCBInfo->pStream->m_StreamLength = (pHdr->num_samples / 14) * 8;

        pStream = pCBInfo->pStream;
        streamLength = pStream->m_StreamLength;
        if (pStream->m_StreamPos >= streamLength)
        {
            streamLength = pStream->m_StreamPos;
        }
        pStream->m_StreamLength = streamLength;

        pCBInfo->pStream->m_OldLength = pCBInfo->pStream->m_StreamLength;

        pBuffer = pCBInfo->pBuffer;
        sndStreamFrq(pBuffer->m_StreamId, pHdr->sample_rate);
        sndStreamADPCMParameter(pBuffer->m_StreamId, (SND_ADPCMSTREAM_INFO*)pHdr->coef);

        READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
    }
}

/**
 * Offset/Address/Size: 0x17BC | 0x801C8F6C | size: 0xC0
 */
void GCAudioStreaming::AudioStream::_WarmReadCB(nlFile*, void*, unsigned int Length, unsigned long User)
{
    READ_CB_INFO* pCBInfo = (READ_CB_INFO*)User;
    AudioStream* pStream = pCBInfo->pStream;

    bool serious;
    if (pStream->m_Flags & (1 << SF_SeriousStop))
    {
        switch (pStream->m_State)
        {
        case SS_New:
        case SS_Initd:
            break;
        case SS_Warming:
            pStream->m_State = SS_Warm;
            break;
        case SS_Warm:
        case SS_Playing:
            break;
        }
        serious = true;
    }
    else
    {
        serious = false;
    }

    if (serious)
    {
        READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
    }
    else
    {
        unsigned long samples = (Length >> 3) * 0xE;
        sndStreamARAMUpdate(pCBInfo->pBuffer->m_StreamId, 0, samples, 0, 0);
        pCBInfo->pStream->WarmReadDone(pCBInfo->pBuffer);
        READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
    }
}

inline void GCAudioStreaming::AudioStreamBuffer::Update(
    unsigned long Offset, unsigned long Length)
{
    sndStreamARAMUpdate(
        m_StreamId,
        (Offset >> 3) * 0xe,
        (Length >> 3) * 0xe,
        0,
        0);
}

/**
 * Offset/Address/Size: 0x1704 | 0x801C8EB4 | size: 0xB8
 */
void GCAudioStreaming::AudioStream::_UpdateReadCB(nlFile*, void* pData, unsigned int Length, unsigned long User)
{
    READ_CB_INFO* pCBInfo = (READ_CB_INFO*)User;
    AudioStream* pStream = pCBInfo->pStream;

    bool serious;
    if (pStream->m_Flags & (1 << SF_SeriousStop))
    {
        switch (pStream->m_State)
        {
        case SS_New:
        case SS_Initd:
            break;
        case SS_Warming:
            pStream->m_State = SS_Warm;
            break;
        case SS_Warm:
        case SS_Playing:
            break;
        }
        serious = true;
    }
    else
    {
        serious = false;
    }

    if (serious)
    {
        READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
    }
    else
    {
        AudioStreamBuffer* pBuffer = pCBInfo->pBuffer;
        pBuffer->Update(
            (unsigned long)pData - Length - (unsigned long)pBuffer->m_MRAMBuffer,
            Length);
        READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
    }
}

inline void* GCAudioStreaming::AudioBufferMgr::GetADPCMHdr()
{
    return (void*)(((unsigned long)m_ADPCMHdrMem + 0x1F) & ~0x1F);
}

/**
 * Offset/Address/Size: 0x1364 | 0x801C8B14 | size: 0x3A0
 * TODO: 95.88% match - object pointer and allocation-loop registers still differ
 */
void GCAudioStreaming::MonoAudioStream::Warm(bool CoolOnStop)
{
    m_State = SS_Warming;
    m_Flags &= ~(1 << SF_SeriousStop);
    m_Flags = (m_Flags & ~(1 << SF_CoolOnStop)) | ((unsigned long)CoolOnStop << SF_CoolOnStop);

    AudioStreamBuffer* pBuf = m_BuffMgr.GetFreeBuffer(this);
    m_Buffers[0] = pBuf;

    m_UpdateLen = m_Buffers[0]->m_BufferSize >> 1;
    m_StreamLength = (unsigned long)-1;

    unsigned int AllocSize;
    nlFileSize(m_pFile, &AllocSize);

    long ReadLen = m_UpdateLen;
    if (AllocSize - 0x60 <= (unsigned long)ReadLen)
    {
        ReadLen = AllocSize - 0x60;
    }
    m_UpdateLen = ReadLen;

    nlSeek(m_pFile, 0, 0);

    void* pADPCMHdr = m_BuffMgr.GetADPCMHdr();

    bool enabled = OSDisableInterrupts();
    READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.Allocate();
    OSRestoreInterrupts(enabled);

    if (pCBInfo)
    {
        AudioStreamBuffer* pBuffer = m_Buffers[0];
        pCBInfo->pStream = this;
        pCBInfo->pBuffer = pBuffer;
    }

    nlReadAsync(m_pFile, pADPCMHdr, sizeof(sDSPADPCM), _HdrReadCB, (unsigned long)pCBInfo);

    unsigned char* pDataBuf = m_Buffers[0]->m_MRAMBuffer;

    enabled = OSDisableInterrupts();
    READ_CB_INFO* pCBInfo2 = READ_CB_INFO::s_AllocPool.Allocate();
    OSRestoreInterrupts(enabled);

    if (pCBInfo2)
    {
        AudioStreamBuffer* pBuffer = m_Buffers[0];
        pCBInfo2->pStream = this;
        pCBInfo2->pBuffer = pBuffer;
    }

    nlReadAsync(m_pFile, pDataBuf, ReadLen, _WarmReadCB, (unsigned long)pCBInfo2);

    m_StreamPos = ReadLen;
    m_Buffers[0]->m_UpdateOffset += ReadLen;

    if ((unsigned long)ReadLen < m_Buffers[0]->m_BufferSize >> 1)
    {
        memset(pDataBuf + ReadLen, 0, (m_Buffers[0]->m_BufferSize >> 1) - ReadLen);
    }

    long secondReadLen = m_UpdateLen;
    unsigned long dataRemaining = AllocSize - 0x60 - ReadLen;
    if (dataRemaining <= (unsigned long)secondReadLen)
    {
        secondReadLen = dataRemaining;
    }

    if (secondReadLen > 0)
    {
        m_Buffers[0]->DoUpdate(secondReadLen, 0);
    }
    else
    {
        m_Flags = (m_Flags & ~(1 << SF_EndAtUpdate)) | (1 << SF_EndAtUpdate);
        m_LastPlayable = m_StreamPos;
    }
}

/**
 * Offset/Address/Size: 0xDCC | 0x801C857C | size: 0x598
 */
unsigned long GCAudioStreaming::MonoAudioStream::DoUpdateRead(unsigned long MRAMOffsetA, unsigned long LengthA, unsigned long MRAMOffsetB, unsigned long LengthB, GCAudioStreaming::AudioStreamBuffer* pRequestingBuffer)
{
    unsigned long OffsetChunk;
    unsigned long ReadASize;
    unsigned long ReadBSize;
    unsigned long AlignOff;
    unsigned char* pMRAMBuffer = m_Buffers[0]->m_MRAMBuffer;
    if (m_Flags & (1 << SF_EndAtUpdate))
    {
        if (MRAMOffsetA <= m_LastPlayable && MRAMOffsetA + LengthA >= m_LastPlayable)
        {
            Stop();
            return 0;
        }
        ___blank("Read of zereos %d %d and %d %d\n", MRAMOffsetA, LengthA, MRAMOffsetB, LengthB);
        memset(pMRAMBuffer + MRAMOffsetA, 0, LengthA);
        bool enabled = OSDisableInterrupts();
        READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.Allocate();
        OSRestoreInterrupts(enabled);
        if (pCBInfo)
        {
            AudioStreamBuffer* pB = m_Buffers[0];
            pCBInfo->pStream = this;
            pCBInfo->pBuffer = pB;
        }
        _UpdateReadCB(m_pFile, pMRAMBuffer + MRAMOffsetA + LengthA, LengthA, (unsigned long)pCBInfo);
        if (LengthB != 0)
        {
            memset(pMRAMBuffer + MRAMOffsetB, 0, LengthB);
            bool e3 = OSDisableInterrupts();
            READ_CB_INFO* pCBInfo2 = READ_CB_INFO::s_AllocPool.Allocate();
            OSRestoreInterrupts(e3);
            if (pCBInfo2)
            {
                AudioStreamBuffer* pB = m_Buffers[0];
                pCBInfo2->pStream = this;
                pCBInfo2->pBuffer = pB;
            }
            _UpdateReadCB(m_pFile, pMRAMBuffer + MRAMOffsetB + LengthB, LengthB, (unsigned long)pCBInfo2);
        }
        return LengthA + LengthB;
    }
    for (OffsetChunk = 0; OffsetChunk < 2; OffsetChunk++)
    {
        unsigned long ReadLen = LengthA;
        if (OffsetChunk != 0)
            ReadLen = LengthB;
        unsigned long MRAMOffset;
        if (ReadLen == 0)
            continue;
        ReadASize = ReadLen;
        MRAMOffset = MRAMOffsetA;
        ReadBSize = 0;
        if (OffsetChunk != 0)
            MRAMOffset = MRAMOffsetB;
        ___blank("Len: %d Pos: %d ReadLen: %d\n", m_StreamLength, m_StreamPos, ReadLen);
        m_StreamPos += ReadLen;
        if (m_StreamPos >= m_StreamLength)
        {
            ___blank("Stream wrap\n");
            ReadASize = ReadLen - (m_StreamPos = m_StreamPos - m_StreamLength);
            AlignOff = 32 - (ReadASize & 31);
            AlignOff = (AlignOff == 32) ? 0 : AlignOff;
            ReadASize += AlignOff;
            ReadBSize = m_StreamPos - AlignOff;
            if (!(m_Flags & (1 << SF_Loop)))
            {
                unsigned long eosOff = MRAMOffset + ReadASize;
                ___blank("EOS zeros %d %d\n", eosOff, ReadBSize);
                memset(pMRAMBuffer + ReadASize, 0, ReadBSize);
                bool e4 = OSDisableInterrupts();
                READ_CB_INFO* pCBInfo3 = READ_CB_INFO::s_AllocPool.Allocate();
                OSRestoreInterrupts(e4);
                if (pCBInfo3)
                {
                    AudioStreamBuffer* pB = m_Buffers[0];
                    pCBInfo3->pStream = this;
                    pCBInfo3->pBuffer = pB;
                }
                _UpdateReadCB(m_pFile, pMRAMBuffer + MRAMOffset + ReadASize + ReadBSize, ReadBSize, (unsigned long)pCBInfo3);
                m_LastPlayable = eosOff;
                ReadBSize = 0;
                m_Flags = (m_Flags & ~(1 << SF_EndAtUpdate)) | (1 << SF_EndAtUpdate);
            }
        }
        ___blank("Reading into %d %d from %d\n", MRAMOffset, ReadASize, nlGetFilePosition(m_pFile));
        bool e5 = OSDisableInterrupts();
        READ_CB_INFO* pCBInfo4 = READ_CB_INFO::s_AllocPool.Allocate();
        OSRestoreInterrupts(e5);
        if (pCBInfo4)
        {
            AudioStreamBuffer* pB = m_Buffers[0];
            pCBInfo4->pStream = this;
            pCBInfo4->pBuffer = pB;
        }
        nlReadAsync(m_pFile, pMRAMBuffer + MRAMOffset, ReadASize, _UpdateReadCB, (unsigned long)pCBInfo4);
        if (ReadBSize != 0)
        {
            nlSeek(m_pFile, 0x60, 0);
            unsigned long filePos2 = nlGetFilePosition(m_pFile);
            ___blank("Also reading into %d %d from %d\n", MRAMOffset + ReadASize, ReadBSize, filePos2);
            OSGetConsoleType();
            bool e6 = OSDisableInterrupts();
            READ_CB_INFO* pCBInfo5 = READ_CB_INFO::s_AllocPool.Allocate();
            OSRestoreInterrupts(e6);
            if (pCBInfo5)
            {
                AudioStreamBuffer* pB = m_Buffers[0];
                pCBInfo5->pStream = this;
                pCBInfo5->pBuffer = pB;
            }
            nlReadAsync(m_pFile, pMRAMBuffer + MRAMOffset + ReadASize, ReadBSize, _UpdateReadCB, (unsigned long)pCBInfo5);
        }
    }
    return 0;
}

inline void GCAudioStreaming::StereoAudioStream::ReadHeader(
    unsigned long buffer)
{
    void* pADPCMHdr = m_BuffMgr.GetADPCMHdr();
    bool enabled = OSDisableInterrupts();
    READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.Allocate();
    OSRestoreInterrupts(enabled);
    if (pCBInfo)
    {
        AudioStreamBuffer* pBuffer = m_Buffers[buffer];
        pCBInfo->pStream = this;
        pCBInfo->pBuffer = pBuffer;
    }
    nlReadAsync(
        m_pFile,
        pADPCMHdr,
        sizeof(sDSPADPCM),
        _HdrReadCB,
        (unsigned long)pCBInfo);
}

/**
 * Offset/Address/Size: 0xA48 | 0x801C81F8 | size: 0x384
 */
void GCAudioStreaming::StereoAudioStream::Warm(bool CoolOnStop)
{
    m_State = SS_Warming;
    m_Flags &= ~(1 << SF_SeriousStop);
    m_Flags = (m_Flags & ~(1 << SF_CoolOnStop)) | ((unsigned long)CoolOnStop << SF_CoolOnStop);

    AudioStreamBuffer* pBuf = m_BuffMgr.GetFreeBuffer(this);
    m_Buffers[0] = pBuf;

    pBuf = m_BuffMgr.GetFreeBuffer(this);
    m_Buffers[1] = pBuf;

    {
        AudioStreamBuffer* pBuf0 = m_Buffers[0];
        pBuf0->m_Pan = 0;
        sndStreamMixParameterEx(pBuf0->m_StreamId, pBuf0->m_Volume, pBuf0->m_Pan, pBuf0->m_SurroundPan, 0, 0);
    }
    {
        AudioStreamBuffer* pBuf1 = m_Buffers[1];
        pBuf1->m_Pan = 0x7F;
        sndStreamMixParameterEx(pBuf1->m_StreamId, pBuf1->m_Volume, pBuf1->m_Pan, pBuf1->m_SurroundPan, 0, 0);
    }

    AudioStreamBuffer* pBuffer;
    AudioStreamBuffer* init;
    unsigned long Zero = 0;
    volatile unsigned long BufferIndex = (unsigned long)(init = 0);
    if (m_BufferCount > Zero)
    {
        init = m_Buffers[0];
    }
    pBuffer = init;
    while (pBuffer)
    {
        pBuffer->SetVolume(m_Volume);
        pBuffer->SetLPF(m_LPFOn);
        pBuffer->SetLPF(m_LPFFreq);

        unsigned long idx = BufferIndex + 1;
        BufferIndex = idx;
        AudioStreamBuffer* pNext;
        if (idx < m_BufferCount)
            pNext = m_Buffers[idx];
        else
            pNext = 0;
        pBuffer = pNext;
    }

    m_StreamLength = (unsigned long)-1;
    nlSeek(m_pFile, 0, 0);

    void* pInterlvHdr = nlMalloc(sizeof(INTERLEAVED_ADPCM_HEADER), 0x20, true);
    nlReadAsync(m_pFile, pInterlvHdr, sizeof(INTERLEAVED_ADPCM_HEADER), _InterleavedHdrReadCB, (unsigned long)this);

    for (unsigned long buffer = 0; buffer < 2; buffer++)
    {
        ReadHeader(buffer);
    }
}

/**
 * Offset/Address/Size: 0x7E8 | 0x801C7F98 | size: 0x260
 */
void GCAudioStreaming::StereoAudioStream::InterleavedHdrReadCB(nlFile* pFile, void* pData, unsigned int Length)
{
    INTERLEAVED_ADPCM_HEADER* pHdr = (INTERLEAVED_ADPCM_HEADER*)((unsigned long)pData - Length);

    bool serious;
    if (m_Flags & (1 << SF_SeriousStop))
    {
        switch (m_State)
        {
        case SS_New:
        case SS_Initd:
            break;
        case SS_Warming:
            m_State = SS_Warm;
            break;
        case SS_Warm:
        case SS_Playing:
            break;
        }
        serious = true;
    }
    else
    {
        serious = false;
    }

    if (serious)
    {
        nlFree(pHdr);
        return;
    }

    m_Interleave = pHdr->Interleave;
    m_StreamLength = pHdr->StreamLength;
    nlFree(pHdr);

    unsigned char* pMRAMBuffer;
    unsigned long aramLen;
    unsigned long offset;
    unsigned long firstLen;
    unsigned long secondLen;
    AudioStreamBuffer* pBuffer;
    AudioStreamBuffer* init;
    unsigned long Zero = 0;
    volatile unsigned long BufferIndex = (unsigned long)(init = 0);
    if (m_BufferCount > Zero)
    {
        init = m_Buffers[0];
    }
    pBuffer = init;

    while (pBuffer)
    {
        pBuffer->m_UpdateOffset += m_Interleave;
        pMRAMBuffer = pBuffer->m_MRAMBuffer;

        bool enabled = OSDisableInterrupts();
        register READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.Allocate();
        OSRestoreInterrupts(enabled);

        if (pCBInfo)
        {
            pCBInfo->pStream = this;
            pCBInfo->pBuffer = pBuffer;
        }

        nlReadAsync(m_pFile, pMRAMBuffer, m_Interleave, _WarmReadCB, (unsigned long)pCBInfo);

        unsigned long idx = BufferIndex + 1;
        BufferIndex = idx;
        AudioStreamBuffer* pNext;
        if (idx < m_BufferCount)
            pNext = m_Buffers[idx];
        else
            pNext = 0;
        pBuffer = pNext;
    }

    m_StreamPos = m_Interleave;
    unsigned long readLen = GetUpdateReadLength();
    AudioStreamBuffer* pBuf = m_Buffers[0];
    aramLen = (readLen / 8) * 14;

    if (aramLen == 0)
        return;

    unsigned long bufReadLen = pBuf->m_pStream->GetUpdateReadLength();
    if ((aramLen / 14) * 8 < bufReadLen)
        return;

    offset = pBuf->m_UpdateOffset;
    pBuf->m_UpdateOffset = offset + bufReadLen;

    if (pBuf->m_UpdateOffset >= pBuf->m_BufferSize)
    {
        unsigned long wrapped = pBuf->m_UpdateOffset - pBuf->m_BufferSize;
        pBuf->m_UpdateOffset = wrapped;
        firstLen = bufReadLen - wrapped;
        secondLen = pBuf->m_UpdateOffset & ~31;
    }
    else
    {
        firstLen = bufReadLen;
        secondLen = 0;
    }

    ___blank(pBuf->m_pStream->m_Buffers[0] == pBuf ? "Left " : "Right ");
    ___blank("Asking for %d %d and %d %d\n", offset, firstLen, 0, secondLen);

    pBuf->m_pStream->DoUpdateRead(offset, firstLen, 0, secondLen, pBuf);
}

/**
 * Offset/Address/Size: 0x2A8 | 0x801C7A58 | size: 0x540
 * TODO: 98.79% match - receiver and MRAMOffsetB register coloring across member accesses and buffer/read calls
 */
unsigned long GCAudioStreaming::StereoAudioStream::DoUpdateRead(unsigned long MRAMOffsetA, unsigned long LengthA, unsigned long LengthB, unsigned long MRAMOffsetB, GCAudioStreaming::AudioStreamBuffer* pRequestingBuffer)
{
    bool serious;
    if (this->m_Flags & (1 << SF_SeriousStop))
    {
        switch (this->m_State)
        {
        case SS_New:
        case SS_Initd:
            break;
        case SS_Warming:
            this->m_State = SS_Warm;
            break;
        case SS_Warm:
        case SS_Playing:
            break;
        }
        serious = true;
    }
    else
    {
        serious = false;
    }
    if (serious)
        return 0;
    if (pRequestingBuffer != this->m_Buffers[0])
    {
        ___blank("Skiping right channel\n");
        return LengthA + LengthB;
    }
    if (this->m_OldLength == 0)
        this->m_OldLength = this->m_StreamLength;
    if (this->m_Flags & (1 << SF_EndAtUpdate))
    {
        AudioStream* pStream = this;
        unsigned long EndOffset = MRAMOffsetA + LengthA;
        ___blank("Lookat at stopping, last playable @ %d, currently @ %d for %d (%d) \n", this->m_LastPlayable, MRAMOffsetA, LengthA, EndOffset);
        if (MRAMOffsetA < this->m_LastPlayable && EndOffset >= this->m_LastPlayable)
        {
            this->Stop();
            return 0;
        }
        AudioStreamBuffer* pBuffer;
        AudioStreamBuffer* init;
        unsigned long Zero = 0;
        volatile unsigned long BufferIndex = (unsigned long)(init = 0);
        if (this->m_BufferCount > Zero)
        {
            init = this->m_Buffers[0];
        }
        pBuffer = init;
        unsigned long ARAMLenA = (LengthA >> 3) * 0xe;
        unsigned long ARAMLenB = (LengthB >> 3) * 0xe;
        while (pBuffer)
        {
            unsigned char* pMRAMBuffer = pBuffer->m_MRAMBuffer;
            ___blank("Read of zereos %d %d and %d %d\n", MRAMOffsetA, LengthA, MRAMOffsetB, LengthB);
            memset(pMRAMBuffer + MRAMOffsetA, 0, LengthA);
            bool enabled = OSDisableInterrupts();
            READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.Allocate();
            OSRestoreInterrupts(enabled);
            if (pCBInfo)
            {
                pCBInfo->pStream = pStream;
                pCBInfo->pBuffer = pBuffer;
            }
            AudioStream* pStream = pCBInfo->pStream;
            bool serious2;
            if (pStream->m_Flags & (1 << SF_SeriousStop))
            {
                switch (pStream->m_State)
                {
                case SS_New:
                case SS_Initd:
                    break;
                case SS_Warming:
                    pStream->m_State = SS_Warm;
                    break;
                case SS_Warm:
                case SS_Playing:
                    break;
                }
                serious2 = true;
            }
            else
            {
                serious2 = false;
            }
            if (serious2)
            {
                READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
            }
            else
            {
                AudioStreamBuffer* pBuf = pCBInfo->pBuffer;
                unsigned long endA = (unsigned long)(pMRAMBuffer + MRAMOffsetA + LengthA);
                sndStreamARAMUpdate(pBuf->m_StreamId, (((endA - LengthA - (unsigned long)pBuf->m_MRAMBuffer) >> 3) * 0xe), ARAMLenA, 0, 0);
                READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo);
            }
            if (LengthB != 0)
            {
                memset(pMRAMBuffer + MRAMOffsetB, 0, LengthB);
                bool e3 = OSDisableInterrupts();
                READ_CB_INFO* pCBInfo2 = READ_CB_INFO::s_AllocPool.Allocate();
                OSRestoreInterrupts(e3);
                if (pCBInfo2)
                {
                    pCBInfo2->pStream = pStream;
                    pCBInfo2->pBuffer = pBuffer;
                }
                AudioStream* pStream2 = pCBInfo2->pStream;
                bool serious3;
                if (pStream2->m_Flags & (1 << SF_SeriousStop))
                {
                    switch (pStream2->m_State)
                    {
                    case SS_New:
                    case SS_Initd:
                        break;
                    case SS_Warming:
                        pStream2->m_State = SS_Warm;
                        break;
                    case SS_Warm:
                    case SS_Playing:
                        break;
                    }
                    serious3 = true;
                }
                else
                {
                    serious3 = false;
                }
                if (serious3)
                {
                    READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo2);
                }
                else
                {
                    AudioStreamBuffer* pBuf2 = pCBInfo2->pBuffer;
                    unsigned long endB = (unsigned long)(pMRAMBuffer + MRAMOffsetB + LengthB);
                    sndStreamARAMUpdate(pBuf2->m_StreamId, (((endB - LengthB - (unsigned long)pBuf2->m_MRAMBuffer) >> 3) * 0xe), ARAMLenB, 0, 0);
                    READ_CB_INFO::s_AllocPool.DeleteEntry(pCBInfo2);
                }
            }
            unsigned long idx = BufferIndex + 1;
            BufferIndex = idx;
            AudioStreamBuffer* next;
            if (idx < this->m_BufferCount)
                next = this->m_Buffers[idx];
            else
                next = 0;
            pBuffer = next;
        }
        return LengthA + LengthB;
    }
    unsigned long TotalReadLen = LengthA + LengthB;
    AudioStreamBuffer* pBuffer;
    AudioStreamBuffer* init;
    unsigned long Zero = 0;
    volatile unsigned long BufferIndex = (unsigned long)(init = 0);
    if (this->m_BufferCount > Zero)
    {
        init = this->m_Buffers[0];
    }
    pBuffer = init;
    while (pBuffer)
    {
        unsigned char* pMRAMBuffer = pBuffer->m_MRAMBuffer;
        ___blank("Reading into %d %d and %d %d from %d\n", MRAMOffsetA, LengthA, MRAMOffsetB, LengthB, nlGetFilePosition(this->m_pFile));
        bool enabled = OSDisableInterrupts();
        READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.Allocate();
        OSRestoreInterrupts(enabled);
        if (pCBInfo)
        {
            pCBInfo->pStream = this;
            pCBInfo->pBuffer = pBuffer;
        }
        nlReadAsync(this->m_pFile, pMRAMBuffer + MRAMOffsetA, LengthA, _UpdateReadCB, (unsigned long)pCBInfo);
        if (LengthB != 0)
        {
            bool e2 = OSDisableInterrupts();
            READ_CB_INFO* pCBInfo2 = READ_CB_INFO::s_AllocPool.Allocate();
            OSRestoreInterrupts(e2);
            if (pCBInfo2)
            {
                pCBInfo2->pStream = this;
                pCBInfo2->pBuffer = pBuffer;
            }
            nlReadAsync(this->m_pFile, pMRAMBuffer + MRAMOffsetB, LengthB, _UpdateReadCB, (unsigned long)pCBInfo2);
        }
        unsigned long idx = BufferIndex + 1;
        BufferIndex = idx;
        AudioStreamBuffer* next;
        if (idx < this->m_BufferCount)
            next = this->m_Buffers[idx];
        else
            next = 0;
        pBuffer = next;
    }
    this->m_StreamPos += TotalReadLen;
    if (this->m_StreamPos >= this->m_StreamLength)
    {
        ___blank("Stream wrap\n");
        this->m_StreamPos = 0;
        nlSeek(this->m_pFile, 0xCC, 0);
        if (!(this->m_Flags & (1 << SF_Loop)))
        {
            unsigned long lastPlayable = MRAMOffsetA + LengthA;
            if (LengthB != 0)
                lastPlayable = MRAMOffsetB + LengthB;
            this->m_LastPlayable = lastPlayable;
            this->m_Flags = (this->m_Flags & ~(1 << SF_EndAtUpdate)) | (1 << SF_EndAtUpdate);
        }
    }
    return TotalReadLen;
}

/**
 * Offset/Address/Size: 0x268 | 0x801C7A18 | size: 0x40
 */
void GCAudioStreaming::AudioBufferMgr::Init(unsigned long BufferPoolSize)
{
    m_PoolSize = BufferPoolSize;
    m_MRAMBuffer = (unsigned char*)nlMalloc(BufferPoolSize, 0x20, false);
}

/**
 * Offset/Address/Size: 0x134 | 0x801C78E4 | size: 0x134
 */
void GCAudioStreaming::AudioBufferMgr::CreateBuffers(unsigned long Count)
{
    unsigned long buffer;

    m_BufferCount = Count;
    m_BufferSize = (m_PoolSize / Count) & ~31u;

    for (buffer = 0; buffer < m_BufferCount; buffer++)
    {
        unsigned long tmp = m_BuffersFree & ~(1u << buffer);
        m_BuffersFree = tmp | (1u << buffer);

        unsigned char* bufAddr = m_MRAMBuffer + m_BufferSize * buffer;
        m_Buffers[buffer].m_BufferSize = m_BufferSize;
        m_Buffers[buffer].m_BufferSamples = (m_Buffers[buffer].m_BufferSize / 8) * 14;
        m_Buffers[buffer].m_MRAMBuffer = bufAddr;

        m_Buffers[buffer].m_Volume = 0x7F;
        m_Buffers[buffer].m_Pan = 0x40;
        m_Buffers[buffer].m_bLPFOn = 0;
        m_Buffers[buffer].m_LPFFreq = 0x3FFF;
        m_Buffers[buffer].m_UpdateOffset = 0;
        m_Buffers[buffer].m_pStream = 0;

        m_Buffers[buffer].m_StreamId = sndStreamAllocEx(
            0xFF,
            m_Buffers[buffer].m_MRAMBuffer,
            m_Buffers[buffer].m_BufferSamples,
            0x7D00,
            0,
            0x40,
            0x40,
            0,
            0,
            0,
            0x30001,
            AudioStreamBuffer::_UpdateHandler,
            (unsigned long)&m_Buffers[buffer],
            NULL);

        sndStreamMixParameterEx(
            m_Buffers[buffer].m_StreamId,
            m_Buffers[buffer].m_Volume,
            m_Buffers[buffer].m_Pan,
            m_Buffers[buffer].m_SurroundPan,
            0,
            0);
    }
}

/**
 * Offset/Address/Size: 0x98 | 0x801C7848 | size: 0x9C
 */
void GCAudioStreaming::AudioBufferMgr::DeleteBuffers()
{
    unsigned long buffer;
    for (buffer = 0; buffer < m_BufferCount; buffer++)
    {
        m_Buffers[buffer].m_Volume = 0;
        sndStreamMixParameterEx(m_Buffers[buffer].m_StreamId, m_Buffers[buffer].m_Volume, m_Buffers[buffer].m_Pan, m_Buffers[buffer].m_SurroundPan, 0, 0);
        sndStreamDeactivate(m_Buffers[buffer].m_StreamId);
        sndStreamFree(m_Buffers[buffer].m_StreamId);
    }
    m_BufferCount = 0;
}

/**
 * Offset/Address/Size: 0x0 | 0x801C77B0 | size: 0x98
 */
void GCAudioStreaming::AudioBufferMgr::FreeBuffer(GCAudioStreaming::AudioStreamBuffer* pBuffer)
{
    unsigned long buff = pBuffer - m_Buffers;

    m_Buffers[buff].Reset();

    SetBufferState(buff, BAS_Free);

    ___blank("After buffer free there are %d availible\n", nlCountBits(m_BuffersFree));
}
