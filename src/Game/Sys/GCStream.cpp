#include "Game/Sys/GCStream.h"
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

// /**
//  * Offset/Address/Size: 0x17BC | 0x801C8F6C | size: 0xC0
//  */
// void GCAudioStreaming::AudioStream::_WarmReadCB(nlFile*, void*, unsigned int, unsigned long)
// {
// }

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

// /**
//  * Offset/Address/Size: 0x1364 | 0x801C8B14 | size: 0x3A0
//  */
// void GCAudioStreaming::MonoAudioStream::Warm(bool)
// {
// }

// /**
//  * Offset/Address/Size: 0xDCC | 0x801C857C | size: 0x598
//  */
// void GCAudioStreaming::MonoAudioStream::DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*)
// {
// }

// /**
//  * Offset/Address/Size: 0xA48 | 0x801C81F8 | size: 0x384
//  */
// void GCAudioStreaming::StereoAudioStream::Warm(bool)
// {
// }

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

// /**
//  * Offset/Address/Size: 0x2A8 | 0x801C7A58 | size: 0x540
//  */
// void GCAudioStreaming::StereoAudioStream::DoUpdateRead(unsigned long, unsigned long, unsigned long, unsigned long, GCAudioStreaming::AudioStreamBuffer*)
// {
// }

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
