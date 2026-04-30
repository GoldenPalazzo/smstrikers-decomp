#include "Game/Sys/PlatStream.h"
#include "Game/Sys/GCStream.h"
#include "NL/nlMemory.h"
#include "NL/nlSortedSlot.h"

extern "C"
{
    void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
    void sndStreamDeactivate(unsigned long stid);
}

extern GCAudioStreaming::AudioBufferMgr g_BufferMgr;

static nlStaticSortedSlot<GCAudioStreaming::AudioStream*, 7> g_Streams;
static bool g_StreamingInitd;

// /**
//  * Offset/Address/Size: 0x30 | 0x801C778C | size: 0x24
//  */
// void nlStaticSortedSlot<GCAudioStreaming::AudioStream*, 7>::GetNewEntry()
// {
// }

// /**
//  * Offset/Address/Size: 0x20 | 0x801C777C | size: 0x10
//  */
// void nlStaticSortedSlot<GCAudioStreaming::AudioStream*, 7>::FreeEntry(GCAudioStreaming::AudioStream**)
// {
// }

// /**
//  * Offset/Address/Size: 0x14 | 0x801C7770 | size: 0xC
//  */
// void nlStaticSortedSlot<GCAudioStreaming::AudioStream*, 7>::FreeLookup()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801C775C | size: 0x14
//  */
// void nlStaticSortedSlot<GCAudioStreaming::AudioStream*, 7>::ExpandLookup()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x801C7748 | size: 0x14
 */
GCAudioStreaming::AudioStreamBuffer::AudioStreamBuffer()
{
    m_Volume = 0x7F;
    m_Pan = 0x40;
}

// /**
//  * Offset/Address/Size: 0x5B8 | 0x801C767C | size: 0xCC
//  */
// void 0x801C7748..0x801C775C | size: 0x14
// {
// }

/**
 * Offset/Address/Size: 0x574 | 0x801C7638 | size: 0x44
 */
void PlatAudio::InitStreaming()
{
    if (!g_StreamingInitd)
    {
        g_BufferMgr.Init(0x5DD80);
        g_StreamingInitd = true;
    }
}

/**
 * Offset/Address/Size: 0x208 | 0x801C72CC | size: 0x36C
 * TODO: 92.6% match - outer loop induction registers differ in teardown pass
 * and both m_BufferCount > 0 checks still compile as beq instead of ble.
 */
void PlatAudio::ShutdownStreaming()
{
    using namespace GCAudioStreaming;

    unsigned long streamIndex = 0;
    unsigned long lookupOffset = streamIndex;
    AudioStream* stream;
    AudioStreamBuffer* buffer;

    while (streamIndex < g_Streams.m_EntryCount)
    {
        stream = *g_Streams.m_pEntryLookup[lookupOffset >> 3].pEntry;
        stream->m_Flags &= ~(1 << SF_Play);

        if (stream->m_State == SS_Playing)
        {
            volatile unsigned long i = (unsigned long)(buffer = NULL);
            if (stream->m_BufferCount > 0)
                buffer = stream->m_Buffers[0];

            while (buffer != NULL)
            {
                buffer->m_Volume = 0;
                sndStreamMixParameterEx(buffer->m_StreamId, buffer->m_Volume, buffer->m_Pan, buffer->m_SurroundPan, 0, 0);
                sndStreamDeactivate(buffer->m_StreamId);

                stream->m_State = SS_Warm;
                {
                    unsigned long next = i + 1;
                    i = next;
                    if (next < stream->m_BufferCount)
                        buffer = stream->m_Buffers[next];
                    else
                        buffer = NULL;
                }
            }

            stream->m_StreamPos = 0;
            stream->m_State = SS_Warm;
        }

        stream->CancelPendingReads();
        if (stream->m_Flags & (1 << SF_CoolOnStop))
        {
            stream->m_Flags &= ~(1 << SF_CoolOnStop);
            if (stream->m_State > SS_Initd)
            {
                unsigned long flags = stream->m_Flags;
                flags &= ~(1 << SF_SeriousStop);
                flags |= (1 << SF_SeriousStop);
                stream->m_Flags = flags;

                volatile unsigned long i = 0;
                buffer = NULL;
                if (stream->m_BufferCount > 0)
                    buffer = stream->m_Buffers[0];

                while (buffer != NULL)
                {
                    stream->m_BuffMgr.FreeBuffer(buffer);

                    {
                        unsigned long idx = i;
                        stream->m_Buffers[idx] = NULL;
                        idx = idx + 1;
                        i = idx;
                        if (idx < stream->m_BufferCount)
                            buffer = stream->m_Buffers[idx];
                        else
                            buffer = NULL;
                    }
                }

                stream->m_State = SS_Initd;
            }
        }

        lookupOffset += 8;
        streamIndex++;
    }

    streamIndex = 0;
    lookupOffset = 0;
    while (streamIndex < g_Streams.m_EntryCount)
    {
        AudioStream** pStream;
        nlSortedSlot<GCAudioStreaming::AudioStream*, 7>::EntryLookup<GCAudioStreaming::AudioStream*>* pEntryLookup;
        nlSortedSlot<GCAudioStreaming::AudioStream*, 7>::EntryLookup<GCAudioStreaming::AudioStream*>* removedLookup;
        unsigned long index;
        unsigned long count;

        stream = *g_Streams.m_pEntryLookup[lookupOffset >> 3].pEntry;
        stream->Stop();

        pStream = g_Streams.m_pEntryLookup[lookupOffset >> 3].pEntry;
        if (*pStream != NULL)
            delete *pStream;

        if (pStream != NULL)
        {
            pEntryLookup = g_Streams.m_pEntryLookup;
            removedLookup = NULL;
            index = 0;
            count = g_Streams.m_EntryCount;

            while (index < count)
            {
                if (pEntryLookup->pEntry == pStream)
                {
                    removedLookup = &g_Streams.m_pEntryLookup[index];
                    break;
                }

                pEntryLookup++;
                index++;
            }

            ((nlSortedSlot<AudioStream*, 7>*)&g_Streams)->FreeEntry(pStream);

            index = (unsigned long)(removedLookup - g_Streams.m_pEntryLookup);
            count = g_Streams.m_EntryCount;
            while (index != count)
            {
                unsigned long next = index + 1;
                nlSortedSlot<GCAudioStreaming::AudioStream*, 7>::EntryLookup<GCAudioStreaming::AudioStream*>* base = g_Streams.m_pEntryLookup;
                nlSortedSlot<GCAudioStreaming::AudioStream*, 7>::EntryLookup<GCAudioStreaming::AudioStream*>* src = &base[next];
                nlSortedSlot<GCAudioStreaming::AudioStream*, 7>::EntryLookup<GCAudioStreaming::AudioStream*>* dst = &base[index];
                unsigned long id = src->hash;
                AudioStream** entry = src->pEntry;

                dst->pEntry = entry;
                dst->hash = id;
                index = next;
            }
            g_Streams.m_EntryCount--;
        }

        lookupOffset += 8;
        streamIndex++;
    }

    streamIndex = 0;
    lookupOffset = 0;
    while (streamIndex < g_Streams.m_EntryCount)
    {
        ((nlSortedSlot<AudioStream*, 7>*)&g_Streams)->FreeEntry(g_Streams.m_pEntryLookup[lookupOffset >> 3].pEntry);
        lookupOffset += 8;
        streamIndex++;
    }

    ((nlSortedSlot<AudioStream*, 7>*)&g_Streams)->FreeLookup();
    g_Streams.m_EntryCount = 0;

    nlFree(g_BufferMgr.m_MRAMBuffer);
    g_BufferMgr.m_MRAMBuffer = NULL;
    g_BufferMgr.m_PoolSize = 0;
    g_StreamingInitd = false;
}

/**
 * Offset/Address/Size: 0x1C4 | 0x801C7288 | size: 0x44
 */
void PlatAudio::ConfigureStreamBuffers(unsigned long count)
{
    g_BufferMgr.DeleteBuffers();
    g_BufferMgr.CreateBuffers(count);
}

/**
 * Offset/Address/Size: 0x1BC | 0x801C7280 | size: 0x8
 */
bool PlatAudio::IsStreamingInited()
{
    return g_StreamingInitd;
}

/**
 * Offset/Address/Size: 0x0 | 0x801C70C4 | size: 0x1BC
 * TODO: 96.1% match - r28/r29 outer induction-variable roles remain swapped
 * and both m_BufferCount > 0 checks still compile as beq instead of ble.
 */
void PlatAudio::StopAllStreams()
{
    using namespace GCAudioStreaming;

    AudioStream* stream;
    AudioStreamBuffer* buffer;
    unsigned long streamIndex = 0;
    unsigned long lookupOffset = 0;

    while (streamIndex < g_Streams.m_EntryCount)
    {
        stream = *g_Streams.m_pEntryLookup[lookupOffset >> 3].pEntry;
        stream->m_Flags &= ~(1 << SF_Play);
        if (stream->m_State == SS_Playing)
        {
            volatile unsigned long j = (unsigned long)(buffer = NULL);
            if (stream->m_BufferCount > 0)
                buffer = stream->m_Buffers[0];

            while (buffer != NULL)
            {
                buffer->m_Volume = 0;
                sndStreamMixParameterEx(buffer->m_StreamId, buffer->m_Volume, buffer->m_Pan, buffer->m_SurroundPan, 0, 0);
                sndStreamDeactivate(buffer->m_StreamId);

                stream->m_State = SS_Warm;
                {
                    unsigned long next = j + 1;
                    j = next;
                    if (next < stream->m_BufferCount)
                        buffer = stream->m_Buffers[next];
                    else
                        buffer = NULL;
                }
            }

            stream->m_StreamPos = 0;
            stream->m_State = SS_Warm;
        }

        stream->CancelPendingReads();
        if (stream->m_Flags & (1 << SF_CoolOnStop))
        {
            stream->m_Flags &= ~(1 << SF_CoolOnStop);
            if (stream->m_State > SS_Initd)
            {
                unsigned long flags = stream->m_Flags;
                flags &= ~(1 << SF_SeriousStop);
                flags |= (1 << SF_SeriousStop);
                stream->m_Flags = flags;

                volatile unsigned long k = 0;
                buffer = NULL;
                if (stream->m_BufferCount > 0)
                    buffer = stream->m_Buffers[0];

                while (buffer != NULL)
                {
                    stream->m_BuffMgr.FreeBuffer(buffer);

                    {
                        unsigned long idx = k;
                        stream->m_Buffers[idx] = NULL;
                        idx = idx + 1;
                        k = idx;
                        if (idx < stream->m_BufferCount)
                            buffer = stream->m_Buffers[idx];
                        else
                            buffer = NULL;
                    }
                }

                stream->m_State = SS_Initd;
            }
        }

        streamIndex++;
        lookupOffset += 8;
    }
}
