#include "Game/Sys/PlatStream.h"
#include "Game/Sys/GCStream.h"
#include "NL/nlMemory.h"
#include "NL/nlSortedSlot.h"

extern "C"
{
    void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
    void sndStreamDeactivate(unsigned long stid);
}

GCAudioStreaming::AudioBufferMgr g_BufferMgr;

nlStaticSortedSlot<GCAudioStreaming::AudioStream*, 7> g_Streams;
static bool g_StreamingInitd;

GCAudioStreaming::AudioBufferMgr::AudioBufferMgr()
    : m_MRAMBuffer(NULL)
{
    m_BuffersFree = 0;
    m_BufferCount = 0;
}

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
 * TODO: 98.5% match - initial zero setup is scheduled differently; the pass-two
 * lookup offset and stream-slot pointer are register-swapped; shifted pEntry uses
 * r0 instead of r5.
 */
void PlatAudio::ShutdownStreaming()
{
    using namespace GCAudioStreaming;
    typedef nlSortedSlot<AudioStream*, 7>::EntryLookup<AudioStream*> EL;

    AudioStream* stream;
    unsigned long streamIndex = 0;
    unsigned long lookupOffset = 0;
    AudioStreamBuffer* buffer;
    unsigned long zero = 0;

    while (streamIndex < g_Streams.m_EntryCount)
    {
        stream = *((EL*)((char*)g_Streams.m_pEntryLookup + lookupOffset))->pEntry;
        stream->m_Flags &= ~(1 << SF_Play);

        if (stream->m_State == SS_Playing)
        {
            volatile unsigned long i = (unsigned long)(buffer = NULL);
            if (zero < stream->m_BufferCount)
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
                if (zero < stream->m_BufferCount)
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
        nlSortedSlot<GCAudioStreaming::AudioStream*, 7>::EntryLookup<GCAudioStreaming::AudioStream*>* removedLookup;
        AudioStream** pStream;

        stream = *((EL*)((char*)g_Streams.m_pEntryLookup + lookupOffset))->pEntry;
        stream->SafeToPurge();

        pStream = ((EL*)((char*)g_Streams.m_pEntryLookup + lookupOffset))->pEntry;
        delete *pStream;

        if (pStream != NULL)
        {
            struct LookupFinder
            {
                static EL* Find(nlStaticSortedSlot<AudioStream*, 7>* self, AudioStream** pStream)
                {
                    EL* pEntryLookup = self->m_pEntryLookup;
                    unsigned long index = 0;
                    unsigned long count = self->m_EntryCount;
                    while (index < count)
                    {
                        if (pEntryLookup->pEntry == pStream)
                            return &self->m_pEntryLookup[index];
                        pEntryLookup++;
                        index++;
                    }
                    return NULL;
                }
            };
            removedLookup = LookupFinder::Find(&g_Streams, pStream);
            ((nlSortedSlot<AudioStream*, 7>*)&g_Streams)->FreeEntry(pStream);

            struct LookupShifter
            {
                static void Shift(nlStaticSortedSlot<AudioStream*, 7>* self, EL* removedLookup)
                {
                    unsigned long entryCount = self->m_EntryCount;
                    long idx = removedLookup - self->m_pEntryLookup;
                    while ((unsigned long)idx != entryCount)
                    {
                        long next = idx + 1;
                        EL* src = &self->m_pEntryLookup[next];
                        register unsigned long hash = src->hash;
                        EL* dst = &self->m_pEntryLookup[idx];
                        idx = next;
                        AudioStream** pEntry = src->pEntry;
                        dst->pEntry = pEntry;
                        dst->hash = hash;
                    }
                }
            };
            LookupShifter::Shift(&g_Streams, removedLookup);
            g_Streams.m_EntryCount--;
        }

        lookupOffset += 8;
        streamIndex++;
    }

    streamIndex = 0;
    lookupOffset = 0;
    while (streamIndex < g_Streams.m_EntryCount)
    {
        ((nlSortedSlot<AudioStream*, 7>*)&g_Streams)->FreeEntry(((EL*)((char*)g_Streams.m_pEntryLookup + lookupOffset))->pEntry);
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
 */
void PlatAudio::StopAllStreams()
{
    using namespace GCAudioStreaming;
    typedef nlSortedSlot<AudioStream*, 7>::EntryLookup<AudioStream*> EL;

    AudioStream* stream;
    AudioStreamBuffer* buffer;
    int streamOffset;
    unsigned long streamIndex;
    unsigned long zero = 0;

    for (streamIndex = 0, streamOffset = 0; streamIndex < g_Streams.m_EntryCount; streamOffset += 8, streamIndex++)
    {
        stream = *((EL*)((char*)g_Streams.m_pEntryLookup + streamOffset))->pEntry;
        stream->m_Flags &= ~(1 << SF_Play);
        if (stream->m_State == SS_Playing)
        {
            volatile unsigned long j = (unsigned long)(buffer = NULL);
            if (zero < stream->m_BufferCount)
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
                if (zero < stream->m_BufferCount)
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
    }
}
