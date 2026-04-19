#include "Game/Sys/GCStream.h"
#include "NL/nlFileGC.h"
#include "NL/nlMemory.h"

extern void ___blank(const char*, ...);

struct SND_ADPCMSTREAM_INFO;

extern "C"
{
    void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
    void sndStreamLPFParameter(unsigned long stid, unsigned long lpfOn, unsigned long lpfFreq);
    void sndStreamDeactivate(unsigned long stid);
    void sndStreamFree(unsigned long stid);
    void sndStreamARAMUpdate(unsigned long stid, unsigned long off1, unsigned long len1, unsigned long off2, unsigned long len2);
    void sndStreamFrq(unsigned long stid, unsigned long frq);
    void sndStreamADPCMParameter(unsigned long stid, SND_ADPCMSTREAM_INFO* adpcmInfo);
    unsigned long sndStreamAllocEx(unsigned char prio, void* buffer, unsigned long samples, unsigned long frq, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb, unsigned char studio, unsigned long flags, unsigned long (*updateFunction)(void*, unsigned long, void*, unsigned long, unsigned long), unsigned long user, SND_ADPCMSTREAM_INFO* adpcmInfo);
}

namespace GCAudioStreaming
{

nlArrayAllocator<AudioStream::READ_CB_INFO> AudioStream::READ_CB_INFO::s_AllocPool;

}

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
    pCBInfo->m_next = AudioStream::READ_CB_INFO::s_AllocPool.m_pFree;
    AudioStream::READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
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
unsigned long GCAudioStreaming::AudioStreamBuffer::_UpdateHandler(void*, unsigned long len1, void*, unsigned long len2, unsigned long user)
{
    AudioStreamBuffer* pBuffer = (AudioStreamBuffer*)user;

    if (!len1 && !len2)
    {
        return 0;
    }

    unsigned long readLen = pBuffer->m_pStream->GetUpdateReadLength();

    if ((len1 + len2) / 14 * 8 < readLen)
    {
        return 0;
    }

    len2 = pBuffer->m_UpdateOffset;
    pBuffer->m_UpdateOffset = len2 + readLen;

    unsigned long secondLen;
    unsigned long firstLen;

    if (pBuffer->m_UpdateOffset >= pBuffer->m_BufferSize)
    {
        unsigned long wrapped = pBuffer->m_UpdateOffset - pBuffer->m_BufferSize;
        pBuffer->m_UpdateOffset = wrapped;
        firstLen = readLen - wrapped;
        secondLen = pBuffer->m_UpdateOffset & ~31;
    }
    else
    {
        firstLen = readLen;
        secondLen = 0;
    }

    ___blank(pBuffer->m_pStream->m_Buffers[0] == pBuffer ? "Left " : "Right ");
    ___blank("Asking for %d %d and %d %d\n", len2, firstLen, 0, secondLen);

    pBuffer->m_pStream->DoUpdateRead(len2, firstLen, 0, secondLen, pBuffer);

    return (firstLen + secondLen) / 8 * 14;
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
        pCBInfo->m_next = AudioStream::READ_CB_INFO::s_AllocPool.m_pFree;
        AudioStream::READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
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
 * Offset/Address/Size: 0x0 | 0x801C9184 | size: 0x20C
 * TODO: 91.83% match - FreeBuffer inline offset folding, init pattern extra li, ble/beq branch hints
 */
void GCAudioStreaming::AudioStream::Stop()
{
    m_Flags &= ~(1 << SF_Play);
    if (m_State == SS_Playing)
    {
        AudioStreamBuffer* next = 0;
        volatile unsigned long BufferIndex = (unsigned long)next;
        if (m_BufferCount > 0)
            next = m_Buffers[0];
        AudioStreamBuffer* pBuffer = next;
        while (pBuffer)
        {
            pBuffer->m_Volume = 0;
            sndStreamMixParameterEx(pBuffer->m_StreamId, pBuffer->m_Volume, pBuffer->m_Pan, pBuffer->m_SurroundPan, 0, 0);
            sndStreamDeactivate(pBuffer->m_StreamId);
            m_State = SS_Warm;
            unsigned long idx = BufferIndex + 1;
            BufferIndex = idx;
            if (idx < m_BufferCount)
                next = m_Buffers[idx];
            else
                next = 0;
            pBuffer = next;
        }
        m_StreamPos = 0;
        m_State = SS_Warm;
    }
    CancelPendingReads();
    if (m_Flags & (1 << SF_CoolOnStop))
    {
        m_Flags &= ~(1 << SF_CoolOnStop);
        if (m_State > SS_Initd)
        {
            AudioStreamBuffer* pBuffer = 0;
            volatile unsigned long BufferIndex = 0;
            m_Flags = (m_Flags & ~(1 << SF_SeriousStop)) | (1 << SF_SeriousStop);
            if (m_BufferCount > 0)
                pBuffer = m_Buffers[0];
            while (pBuffer)
            {
                m_BuffMgr.FreeBuffer(pBuffer);
                unsigned long idx = BufferIndex;
                pBuffer = 0;
                m_Buffers[idx] = pBuffer;
                idx++;
                BufferIndex = idx;
                if (idx < m_BufferCount)
                    pBuffer = m_Buffers[idx];
            }
            m_State = SS_Initd;
        }
    }
}

/**
 * Offset/Address/Size: 0x1984 | 0x801C9134 | size: 0x50
 */
void ___blank(const char*, ...)
{
}

/**
 * Offset/Address/Size: 0x187C | 0x801C902C | size: 0x108
 * TODO: 99.39% match - r3/r4 register swap in max(m_StreamLength, m_StreamPos) comparison section
 */
void GCAudioStreaming::AudioStream::_HdrReadCB(nlFile* pFile, void* pData, unsigned int Length, unsigned long User)
{
    READ_CB_INFO* pCBInfo = (READ_CB_INFO*)User;

    bool serious;
    if (((AudioStream*)pCBInfo->m_next)->m_Flags & (1 << SF_SeriousStop))
    {
        switch (((AudioStream*)pCBInfo->m_next)->m_State)
        {
        case SS_New:
        case SS_Initd:
            break;
        case SS_Warming:
            ((AudioStream*)pCBInfo->m_next)->m_State = SS_Warm;
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
        pCBInfo->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
        READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
    }
    else
    {
        AudioStreamBuffer* pBuffer;
        sDSPADPCM* pHdr = (sDSPADPCM*)((unsigned long)pData - Length);

        ((AudioStream*)pCBInfo->m_next)->m_StreamLength = (pHdr->num_samples / 14) * 8;

        unsigned long streamLength = ((AudioStream*)pCBInfo->m_next)->m_StreamLength;
        if (((AudioStream*)pCBInfo->m_next)->m_StreamPos >= streamLength)
        {
            streamLength = ((AudioStream*)pCBInfo->m_next)->m_StreamPos;
        }
        ((AudioStream*)pCBInfo->m_next)->m_StreamLength = streamLength;

        ((AudioStream*)pCBInfo->m_next)->m_OldLength = ((AudioStream*)pCBInfo->m_next)->m_StreamLength;

        pBuffer = pCBInfo->pBuffer;
        sndStreamFrq(pBuffer->m_StreamId, pHdr->sample_rate);
        sndStreamADPCMParameter(pBuffer->m_StreamId, (SND_ADPCMSTREAM_INFO*)pHdr->coef);

        pCBInfo->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
        READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
    }
}

/**
 * Offset/Address/Size: 0x17BC | 0x801C8F6C | size: 0xC0
 */
void GCAudioStreaming::AudioStream::_WarmReadCB(nlFile*, void*, unsigned int Length, unsigned long User)
{
    if (((AudioStream*)((READ_CB_INFO*)User)->m_next)->m_Flags & (1 << SF_SeriousStop))
    {
        AudioStream* pStream = (AudioStream*)((READ_CB_INFO*)User)->m_next;
        switch (pStream->m_State)
        {
        case SS_Warming:
            pStream->m_State = SS_Warm;
            break;
        }
        ((READ_CB_INFO*)User)->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
        READ_CB_INFO::s_AllocPool.m_pFree = (READ_CB_INFO*)User;
        return;
    }
    unsigned long samples = (Length >> 3);
    samples *= 0xE;
    sndStreamARAMUpdate(((READ_CB_INFO*)User)->pBuffer->m_StreamId, 0, samples, 0, 0);
    ((AudioStream*)((READ_CB_INFO*)User)->m_next)->WarmReadDone(((READ_CB_INFO*)User)->pBuffer);
    ((READ_CB_INFO*)User)->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
    READ_CB_INFO::s_AllocPool.m_pFree = (READ_CB_INFO*)User;
}

/**
 * Offset/Address/Size: 0x1704 | 0x801C8EB4 | size: 0xB8
 */
void GCAudioStreaming::AudioStream::_UpdateReadCB(nlFile*, void* pData, unsigned int Length, unsigned long User)
{
    READ_CB_INFO* pCBInfo = (READ_CB_INFO*)User;
    AudioStream* pStream = (AudioStream*)pCBInfo->m_next;

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
        pCBInfo->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
        READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
    }
    else
    {
        AudioStreamBuffer* pBuffer = pCBInfo->pBuffer;
        unsigned long MRAMStart = (unsigned long)pData - Length;
        unsigned long ARAMLength = (Length >> 3) * 0xe;
        sndStreamARAMUpdate(pBuffer->m_StreamId, (((MRAMStart - (unsigned long)pBuffer->m_MRAMBuffer) >> 3) * 0xe), ARAMLength, 0, 0);
        pCBInfo->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
        READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
    }
}

/**
 * Offset/Address/Size: 0x1364 | 0x801C8B14 | size: 0x3A0
 * TODO: 85.56% match - r31/r29 register swap for mgr/this, pool alloc branch sense
 */
void GCAudioStreaming::MonoAudioStream::Warm(bool CoolOnStop)
{
    m_State = SS_Warming;
    m_Flags &= ~(1 << SF_SeriousStop);
    m_Flags = (m_Flags & ~(1 << SF_CoolOnStop)) | ((unsigned long)CoolOnStop << SF_CoolOnStop);

    AudioBufferMgr& mgr = m_BuffMgr;
    AudioStreamBuffer* pBuf;
    unsigned long i = 0;
    unsigned long free;
    unsigned long mask;
    unsigned long test;

    for (unsigned long j = 0; j < mgr.m_BufferCount; j++)
    {
        free = mgr.m_BuffersFree;
        mask = 1 << i;
        test = free & mask;
        test = (-(long)test | test) >> 31;
        if ((int)test == 1)
        {
            mgr.m_BuffersFree = free & ~mask;
            pBuf = &mgr.m_Buffers[i];
            pBuf->m_pStream = this;
            pBuf->m_UpdateOffset = 0;
            pBuf->m_Volume = 0x7F;
            pBuf->m_Pan = 0x40;

            unsigned long remaining = mgr.m_BuffersFree;
            int count = 0;
            while (remaining)
            {
                remaining &= (remaining - 1);
                count++;
            }
            ___blank("After buffer alloc there are %d availible\n", count);
            break;
        }
        i++;
    }
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

    unsigned long alignedHdr = ((unsigned long)mgr.m_ADPCMHdrMem + 0x1F) & ~0x1F;

    bool enabled = OSDisableInterrupts();
    READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.m_pFree;
    if (pCBInfo)
    {
        READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo->m_next;
    }
    OSRestoreInterrupts(enabled);

    if (pCBInfo)
    {
        pCBInfo->m_next = (READ_CB_INFO*)this;
        pCBInfo->pBuffer = m_Buffers[0];
    }

    nlReadAsync(m_pFile, (void*)alignedHdr, sizeof(sDSPADPCM), _HdrReadCB, (unsigned long)pCBInfo);

    unsigned char* pDataBuf = m_Buffers[0]->m_MRAMBuffer;

    enabled = OSDisableInterrupts();
    READ_CB_INFO* pCBInfo2 = READ_CB_INFO::s_AllocPool.m_pFree;
    if (pCBInfo2)
    {
        READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo2->m_next;
    }
    OSRestoreInterrupts(enabled);

    if (pCBInfo2)
    {
        pCBInfo2->m_next = (READ_CB_INFO*)this;
        pCBInfo2->pBuffer = m_Buffers[0];
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
        unsigned long numSamples = ((unsigned long)secondReadLen >> 3) * 14;
        AudioStreamBuffer* pBuffer = m_Buffers[0];

        if (numSamples != 0)
        {
            unsigned long availLen = pBuffer->m_pStream->GetUpdateReadLength();
            unsigned long encSize = (numSamples / 7) & ~7;

            if (encSize >= availLen)
            {
                unsigned long oldOffset = pBuffer->m_UpdateOffset;
                pBuffer->m_UpdateOffset += availLen;

                unsigned long MRAMOffsetB;

                if (pBuffer->m_UpdateOffset >= pBuffer->m_BufferSize)
                {
                    pBuffer->m_UpdateOffset -= pBuffer->m_BufferSize;
                    ReadLen = availLen - pBuffer->m_UpdateOffset;
                    MRAMOffsetB = pBuffer->m_UpdateOffset & ~0x1F;
                }
                else
                {
                    ReadLen = availLen;
                    MRAMOffsetB = 0;
                }

                const char* side = "Right ";
                if (pBuffer->m_pStream->m_Buffers[0] == pBuffer)
                {
                    side = "Left ";
                }
                ___blank(side);
                ___blank("Asking for %d %d and %d %d\n", oldOffset, ReadLen, 0, MRAMOffsetB);
                pBuffer->m_pStream->DoUpdateRead(oldOffset, ReadLen, 0, MRAMOffsetB, pBuffer);
            }
        }
    }
    else
    {
        m_Flags |= (1 << SF_EndAtUpdate);
        m_LastPlayable = m_StreamPos;
    }
}

// /**
//  * Offset/Address/Size: 0xDCC | 0x801C857C | size: 0x598
//  */
// void GCAudioStreaming::MonoAudioStream::DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*)
// {
// }

/**
 * Offset/Address/Size: 0xA48 | 0x801C81F8 | size: 0x384
 * TODO: 79.22% match - buffer loop r-swaps, pADPCMHdr offset calc, init register allocation
 */
void GCAudioStreaming::StereoAudioStream::Warm(bool CoolOnStop)
{
    m_State = SS_Warming;
    m_Flags &= ~(1 << SF_SeriousStop);
    m_Flags = (m_Flags & ~(1 << SF_CoolOnStop)) | ((unsigned long)CoolOnStop << SF_CoolOnStop);

    AudioStreamBuffer* pBuf;
    AudioBufferMgr& mgr = m_BuffMgr;
    unsigned long i = 0;
    unsigned long buffer;
    unsigned long free;
    unsigned long mask;
    unsigned long test;

    for (unsigned long j = 0; j < mgr.m_BufferCount; j++)
    {
        free = mgr.m_BuffersFree;
        mask = 1 << i;
        test = free & mask;
        if (test)
        {
            mgr.m_BuffersFree = free & ~mask;
            pBuf = &mgr.m_Buffers[i];
            pBuf->m_pStream = this;
            pBuf->m_UpdateOffset = 0;
            pBuf->m_Volume = 0x7F;
            pBuf->m_Pan = 0x40;

            unsigned long remaining = mgr.m_BuffersFree;
            int count = 0;
            while (remaining)
            {
                remaining &= (remaining - 1);
                count++;
            }
            ___blank("After buffer alloc there are %d availible\n", count);
            break;
        }
        i++;
    }
    m_Buffers[0] = pBuf;

    i = 0;
    for (unsigned long j = 0; j < mgr.m_BufferCount; j++)
    {
        free = mgr.m_BuffersFree;
        mask = 1 << i;
        test = free & mask;
        if (test)
        {
            mgr.m_BuffersFree = free & ~mask;
            pBuf = &mgr.m_Buffers[i];
            pBuf->m_pStream = this;
            pBuf->m_UpdateOffset = 0;
            pBuf->m_Volume = 0x7F;
            pBuf->m_Pan = 0x40;

            unsigned long remaining = mgr.m_BuffersFree;
            int count = 0;
            while (remaining)
            {
                remaining &= (remaining - 1);
                count++;
            }
            ___blank("After buffer alloc there are %d availible\n", count);
            break;
        }
        i++;
    }
    m_Buffers[1] = pBuf;

    m_Buffers[0]->m_Pan = 0;
    sndStreamMixParameterEx(m_Buffers[0]->m_StreamId, m_Buffers[0]->m_Volume, m_Buffers[0]->m_Pan, m_Buffers[0]->m_SurroundPan, 0, 0);

    m_Buffers[1]->m_Pan = 0x7F;
    sndStreamMixParameterEx(m_Buffers[1]->m_StreamId, m_Buffers[1]->m_Volume, m_Buffers[1]->m_Pan, m_Buffers[1]->m_SurroundPan, 0, 0);

    AudioStreamBuffer* pBuffer = 0;
    volatile unsigned long BufferIndex = (unsigned long)pBuffer;
    if (m_BufferCount > 0)
        pBuffer = m_Buffers[0];
    while (pBuffer)
    {
        pBuffer->m_Volume = (unsigned char)m_Volume;
        sndStreamMixParameterEx(pBuffer->m_StreamId, pBuffer->m_Volume, pBuffer->m_Pan, pBuffer->m_SurroundPan, 0, 0);

        unsigned char lpfOn = m_LPFOn;
        if (lpfOn != pBuffer->m_bLPFOn)
        {
            sndStreamLPFParameter(pBuffer->m_StreamId, lpfOn, pBuffer->m_LPFFreq);
            pBuffer->m_bLPFOn = lpfOn;
        }

        unsigned short lpfFreq = m_LPFFreq;
        if (pBuffer->m_bLPFOn)
        {
            sndStreamLPFParameter(pBuffer->m_StreamId, pBuffer->m_bLPFOn, lpfFreq);
        }
        pBuffer->m_LPFFreq = lpfFreq;

        unsigned long idx = BufferIndex + 1;
        BufferIndex = idx;
        if (idx < m_BufferCount)
            pBuffer = m_Buffers[idx];
        else
            pBuffer = 0;
    }

    m_StreamLength = (unsigned long)-1;
    nlSeek(m_pFile, 0, 0);

    void* pInterlvHdr = nlMalloc(sizeof(INTERLEAVED_ADPCM_HEADER), 0x20, true);
    nlReadAsync(m_pFile, pInterlvHdr, sizeof(INTERLEAVED_ADPCM_HEADER), _InterleavedHdrReadCB, (unsigned long)this);

    unsigned long alignedHdr = ((unsigned long)mgr.m_ADPCMHdrMem + 0x1F) & ~0x1F;
    for (buffer = 0; buffer < 2; buffer++)
    {
        void* pADPCMHdr = (void*)(alignedHdr + buffer * 4);

        bool enabled = OSDisableInterrupts();
        READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.m_pFree;
        if (pCBInfo)
        {
            READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo->m_next;
        }
        OSRestoreInterrupts(enabled);

        if (pCBInfo)
        {
            pCBInfo->m_next = (READ_CB_INFO*)this;
            pCBInfo->pBuffer = m_Buffers[buffer];
        }

        nlReadAsync(m_pFile, pADPCMHdr, sizeof(sDSPADPCM), _HdrReadCB, (unsigned long)pCBInfo);
    }
}

/**
 * Offset/Address/Size: 0x7E8 | 0x801C7F98 | size: 0x260
 * TODO: 94.87% match - r27/r29 register swap for pBuffer/pCBInfo in warm buffer loop
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

    AudioStreamBuffer* pBuffer = NULL;
    volatile unsigned long BufferIndex = (unsigned long)pBuffer;
    if (m_BufferCount > 0)
        pBuffer = m_Buffers[0];

    while (pBuffer)
    {
        pBuffer->m_UpdateOffset += m_Interleave;
        unsigned char* pMRAMBuffer = pBuffer->m_MRAMBuffer;

        bool enabled = OSDisableInterrupts();
        READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.m_pFree;
        if (pCBInfo)
        {
            READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo->m_next;
        }
        OSRestoreInterrupts(enabled);

        if (pCBInfo)
        {
            pCBInfo->m_next = (READ_CB_INFO*)this;
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
    unsigned long aramLen = (readLen / 8) * 14;

    if (aramLen == 0)
        return;

    unsigned long bufReadLen = pBuf->m_pStream->GetUpdateReadLength();
    if ((aramLen / 14) * 8 < bufReadLen)
        return;

    unsigned long offset = pBuf->m_UpdateOffset;
    pBuf->m_UpdateOffset = offset + bufReadLen;

    unsigned long firstLen;
    unsigned long secondLen;

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
 * TODO: 89.84% match - r31/r30/r27 cyclic register swap for this/LengthB/MRAMOffsetB
 */
unsigned long GCAudioStreaming::StereoAudioStream::DoUpdateRead(unsigned long MRAMOffsetA, unsigned long LengthA, unsigned long MRAMOffsetB, unsigned long LengthB, GCAudioStreaming::AudioStreamBuffer* pRequestingBuffer)
{
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
        return 0;
    if (pRequestingBuffer != m_Buffers[0])
    {
        ___blank("Skiping right channel\n");
        return LengthA + LengthB;
    }
    if (m_OldLength == 0)
        m_OldLength = m_StreamLength;
    if (m_Flags & (1 << SF_EndAtUpdate))
    {
        unsigned long EndOffset = MRAMOffsetA + LengthA;
        ___blank("Lookat at stopping, last playable @ %d, currently @ %d for %d (%d) \n", m_LastPlayable, MRAMOffsetA, LengthA, EndOffset);
        if (MRAMOffsetA < m_LastPlayable && EndOffset >= m_LastPlayable)
        {
            Stop();
            return 0;
        }
        AudioStreamBuffer* pBuffer = 0;
        volatile unsigned long BufferIndex = (unsigned long)pBuffer;
        if (m_BufferCount > 0)
            pBuffer = m_Buffers[0];
        unsigned long ARAMLenA = (LengthA >> 3) * 0xe;
        unsigned long ARAMLenB = (LengthB >> 3) * 0xe;
        while (pBuffer)
        {
            unsigned char* pMRAMBuffer = pBuffer->m_MRAMBuffer;
            ___blank("Read of zereos %d %d and %d %d\n", MRAMOffsetA, LengthA, MRAMOffsetB, LengthB);
            memset(pMRAMBuffer + MRAMOffsetA, 0, LengthA);
            bool enabled = OSDisableInterrupts();
            READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.m_pFree;
            if (!pCBInfo)
            {
                pCBInfo = 0;
            }
            else
            {
                READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo->m_next;
            }
            OSRestoreInterrupts(enabled);
            if (pCBInfo)
            {
                pCBInfo->m_next = (READ_CB_INFO*)this;
                pCBInfo->pBuffer = pBuffer;
            }
            AudioStream* pStream = (AudioStream*)pCBInfo->m_next;
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
                pCBInfo->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
                READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
            }
            else
            {
                AudioStreamBuffer* pBuf = pCBInfo->pBuffer;
                unsigned long endA = (unsigned long)(pMRAMBuffer + MRAMOffsetA + LengthA);
                sndStreamARAMUpdate(pBuf->m_StreamId, (((endA - LengthA - (unsigned long)pBuf->m_MRAMBuffer) >> 3) * 0xe), ARAMLenA, 0, 0);
                pCBInfo->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
                READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo;
            }
            if (LengthB != 0)
            {
                memset(pMRAMBuffer + MRAMOffsetB, 0, LengthB);
                bool e3 = OSDisableInterrupts();
                READ_CB_INFO* pCBInfo2 = READ_CB_INFO::s_AllocPool.m_pFree;
                if (!pCBInfo2)
                {
                    pCBInfo2 = 0;
                }
                else
                {
                    READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo2->m_next;
                }
                OSRestoreInterrupts(e3);
                if (pCBInfo2)
                {
                    pCBInfo2->m_next = (READ_CB_INFO*)this;
                    pCBInfo2->pBuffer = pBuffer;
                }
                AudioStream* pStream2 = (AudioStream*)pCBInfo2->m_next;
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
                    pCBInfo2->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
                    READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo2;
                }
                else
                {
                    AudioStreamBuffer* pBuf2 = pCBInfo2->pBuffer;
                    unsigned long endB = (unsigned long)(pMRAMBuffer + MRAMOffsetB + LengthB);
                    sndStreamARAMUpdate(pBuf2->m_StreamId, (((endB - LengthB - (unsigned long)pBuf2->m_MRAMBuffer) >> 3) * 0xe), ARAMLenB, 0, 0);
                    pCBInfo2->m_next = READ_CB_INFO::s_AllocPool.m_pFree;
                    READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo2;
                }
            }
            unsigned long idx = BufferIndex + 1;
            BufferIndex = idx;
            if (idx < m_BufferCount)
                pBuffer = m_Buffers[idx];
            else
                pBuffer = 0;
        }
        return LengthA + LengthB;
    }
    unsigned long TotalReadLen = LengthA + LengthB;
    AudioStreamBuffer* pBuffer = 0;
    volatile unsigned long BufferIndex = (unsigned long)pBuffer;
    if (m_BufferCount > 0)
        pBuffer = m_Buffers[0];
    while (pBuffer)
    {
        unsigned char* pMRAMBuffer = pBuffer->m_MRAMBuffer;
        unsigned long filePos = nlGetFilePosition(m_pFile);
        ___blank("Reading into %d %d and %d %d from %d\n", MRAMOffsetA, LengthA, MRAMOffsetB, LengthB, filePos);
        bool enabled = OSDisableInterrupts();
        READ_CB_INFO* pCBInfo = READ_CB_INFO::s_AllocPool.m_pFree;
        if (!pCBInfo)
        {
            pCBInfo = 0;
        }
        else
        {
            READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo->m_next;
        }
        OSRestoreInterrupts(enabled);
        if (pCBInfo)
        {
            pCBInfo->m_next = (READ_CB_INFO*)this;
            pCBInfo->pBuffer = pBuffer;
        }
        nlReadAsync(m_pFile, pMRAMBuffer + MRAMOffsetA, LengthA, _UpdateReadCB, (unsigned long)pCBInfo);
        if (LengthB != 0)
        {
            bool e2 = OSDisableInterrupts();
            READ_CB_INFO* pCBInfo2 = READ_CB_INFO::s_AllocPool.m_pFree;
            if (!pCBInfo2)
            {
                pCBInfo2 = 0;
            }
            else
            {
                READ_CB_INFO::s_AllocPool.m_pFree = pCBInfo2->m_next;
            }
            OSRestoreInterrupts(e2);
            if (pCBInfo2)
            {
                pCBInfo2->m_next = (READ_CB_INFO*)this;
                pCBInfo2->pBuffer = pBuffer;
            }
            nlReadAsync(m_pFile, pMRAMBuffer + MRAMOffsetB, LengthB, _UpdateReadCB, (unsigned long)pCBInfo2);
        }
        unsigned long idx = BufferIndex + 1;
        BufferIndex = idx;
        if (idx < m_BufferCount)
            pBuffer = m_Buffers[idx];
        else
            pBuffer = 0;
    }
    m_StreamPos += TotalReadLen;
    if (m_StreamPos >= m_StreamLength)
    {
        ___blank("Stream wrap\n");
        m_StreamPos = 0;
        nlSeek(m_pFile, 0xCC, 0);
        if (!(m_Flags & (1 << SF_Loop)))
        {
            unsigned long lastPlayable = MRAMOffsetA + LengthA;
            if (LengthB != 0)
                lastPlayable = MRAMOffsetB + LengthB;
            m_LastPlayable = lastPlayable;
            m_Flags = (m_Flags & ~(1 << SF_EndAtUpdate)) | (1 << SF_EndAtUpdate);
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
    int buff = pBuffer - m_Buffers;

    m_Buffers[buff].m_pStream = 0;
    m_Buffers[buff].m_UpdateOffset = 0;
    m_Buffers[buff].m_Volume = 0x7F;
    m_Buffers[buff].m_Pan = 0x40;

    int mask = 1 << buff;
    unsigned long cleared = m_BuffersFree & ~mask;
    m_BuffersFree = cleared | mask;

    unsigned long free = m_BuffersFree;
    buff = 0;
    while (free)
    {
        free &= (free - 1);
        buff++;
    }

    ___blank("After buffer free there are %d availible\n", buff);
}
