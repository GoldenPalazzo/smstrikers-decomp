#include "Game/Audio/StreamTrack.h"
#include "Game/Sys/GCStream.h"
#include "NL/nlBSearch.h"
#include "NL/nlConfig.h"
#include "NL/nlFileGC.h"
#include "NL/nlList.h"
#include "NL/nlSlotPoolHigh.h"
#include "NL/nlString.h"

extern GCAudioStreaming::AudioBufferMgr g_BufferMgr;

namespace AudioStreamTrack
{
TrackManagerBase::StreamFileLookup::StreamFileLookup(
    const char* /*name*/,
    const Function<bool(const char*, char*, unsigned long)>& /*fn*/)
{
    typedef TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP LookupT;
    typedef BasicSlotPoolHigh<ListEntry<LookupT> > AdapterT;
    typedef ListContainerBase<LookupT, AdapterT> ContainerT;
    ContainerT container;
    nlWalkList(container.m_Head, &container, &ContainerT::DeleteEntry);
}
} // namespace AudioStreamTrack

namespace Detail
{
template <typename R, typename F>
struct MemFunImpl
{
    F mFuncPtr;
};
} // namespace Detail

typedef AudioStreamTrack::StreamTrack::QUEUED_STREAM QS_T;
typedef Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(QS_T*)> MemFunImpl_T;
typedef BindExp2<void, MemFunImpl_T, AudioStreamTrack::StreamTrack*, QS_T*> BindExp2_T;
typedef Function0<void>::FunctorImpl<BindExp2_T> FunctorImpl_T;

// /**
//  * Offset/Address/Size: 0x0 | 0x80157A98 | size: 0x1C
//  */
// void MemFun<AudioStreamTrack::StreamTrack, void, AudioStreamTrack::StreamTrack::QUEUED_STREAM*>(void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80157A58 | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*>(Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack* const&, AudioStreamTrack::StreamTrack::QUEUED_STREAM* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x238 | 0x80157A20 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM> >(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x200 | 0x801579E8 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL> >(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>**, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1C8 | 0x801579B0 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<GCAudioStreaming::StereoAudioStream*> >(DLListEntry<GCAudioStreaming::StereoAudioStream*>**, DLListEntry<GCAudioStreaming::StereoAudioStream*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x18C | 0x80157974 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<GCAudioStreaming::StereoAudioStream*> >(DLListEntry<GCAudioStreaming::StereoAudioStream*>**, DLListEntry<GCAudioStreaming::StereoAudioStream*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x150 | 0x80157938 | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL> >(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>**, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x114 | 0x801578FC | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM> >(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xD0 | 0x801578B8 | size: 0x44
//  */
// void nlDLRingRemove<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL> >(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>**, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8C | 0x80157874 | size: 0x44
//  */
// void nlDLRingRemove<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM> >(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x6C | 0x80157854 | size: 0x20
//  */
// void nlDLRingIsEnd<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM> >(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x34 | 0x8015781C | size: 0x38
//  */
// void nlDLRingRemoveStart<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM> >(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801577E8 | size: 0x34
//  */
// void nlDLRingRemoveEnd<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM> >(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**)
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x801577BC | size: 0x2C
//  */
// void nlListAddEnd<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP> >(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>**, ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>**, ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80157754 | size: 0x68
//  */
// void nlWalkList<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>, ListContainerBase<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP> > > >(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*, ListContainerBase<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP> > >*, void (ListContainerBase<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP> > >::*)(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x54 | 0x801576C8 | size: 0x8C
//  */
// void nlBSearch<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP, unsigned long>(const unsigned long&, AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, int)
// {
// }

// /**
//  * Offset/Address/Size: 0x28 | 0x8015769C | size: 0x2C
//  */
// void nlDefaultQSortComparer<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP>(const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80157674 | size: 0x28
//  */
// void nlQSort<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP>(AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, int, int (*)(const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*))
// {
// }

// /**
//  * Offset/Address/Size: 0x20 | 0x8015764C | size: 0x28
//  */
// void BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP> >::allocFN(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8015762C | size: 0x20
//  */
// void BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP> >::freeFN(void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8015761C | size: 0x10
//  */
// void ListContainerBase<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP> > >::DeleteEntry(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8015744C | size: 0x1D0
//  */
// void DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL, BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL> > >::AllocateAtEnd(unsigned long*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801573F0 | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*> >::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x801573BC | size: 0x34
//  */
// void Function0<void>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*> >::operator()()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8015733C | size: 0x80
//  */
// void Function0<void>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*> >::Clone() const
// {
// }

/**
 * Offset/Address/Size: 0x24E4 | 0x8015723C | size: 0x100
 */
void AudioStreamTrack::TrackManagerBase::Update(float)
{
    typedef DLListEntry<GCAudioStreaming::StereoAudioStream*> Entry;

    GCAudioStreaming::StereoAudioStream* pStream;
    Entry* toFree;
    Entry* toRemove;
    Entry* head;
    Entry* entry;
    Entry** headAddr;

    Entry* tmp = nlDLRingGetStart(m_StreamDeleteList.m_Head);
    head = m_StreamDeleteList.m_Head;
    headAddr = &m_StreamDeleteList.m_Head;
    entry = tmp;

    while (entry != NULL)
    {
        if (entry->m_data->SafeToPurge())
        {
            pStream = entry->m_data;
            pStream->~StereoAudioStream();
            m_StreamPool.Free(pStream);

            toRemove = entry;
            toFree = entry;

            if (nlDLRingIsEnd(head, entry) || entry == NULL)
            {
                entry = NULL;
            }
            else
            {
                entry = entry->m_next;
            }

            nlDLRingRemove(headAddr, toRemove);
            m_StreamDeleteList.m_Allocator.Free(toFree);
        }
        else
        {
            if (nlDLRingIsEnd(head, entry) || entry == NULL)
            {
                entry = NULL;
            }
            else
            {
                entry = entry->m_next;
            }
        }
    }
}

// /**
//  * Offset/Address/Size: 0x1970 | 0x801566C8 | size: 0x3F0
//  */
// void AudioStreamTrack::TrackManagerBase::FadeManager::UpdateFade(AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL*)
// {
// }

/**
 * Offset/Address/Size: 0x18FC | 0x80156654 | size: 0x74
 */
void AudioStreamTrack::StreamTrack::Update(float)
{
    DLListEntry<QUEUED_STREAM>* head = m_QueuedStreams.m_Head;
    if (!head)
    {
        return;
    }

    struct Iter
    {
        DLListEntry<QUEUED_STREAM>* m_head;
        DLListEntry<QUEUED_STREAM>* m_current;
        ~Iter() { }
    };

    DLListEntry<QUEUED_STREAM>* entry = nlDLRingGetStart(head);
    Iter iter;
    iter.m_head = m_QueuedStreams.m_Head;
    iter.m_current = entry;
    QUEUED_STREAM* qs = &entry->m_data;

    if (m_InFakePause)
    {
        return;
    }

    if (qs->pStream->m_State == 1)
    {
        StopQStream(qs);
        ProcessNewHeadStream();
    }
}

/**
 * Offset/Address/Size: 0x14D4 | 0x8015622C | size: 0x428
 */
void AudioStreamTrack::StreamTrack::PlayStream(unsigned long, float, bool, unsigned long, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP)
{
}

inline GCAudioStreaming::AudioStream::AudioStream(GCAudioStreaming::AudioBufferMgr& mgr,
    unsigned long bufCount)
    : m_BuffMgr(mgr)
{
    m_FlagAtDelete = 0;
    m_State = GCAudioStreaming::SS_New;
    m_StreamLength = (unsigned long)-1;
    m_StreamPos = 0;
    m_OldLength = 0;
    m_Flags = 0;
    m_BufferCount = bufCount;
    memset(m_Buffers, 0, sizeof(m_Buffers));
}

inline GCAudioStreaming::StereoAudioStream::StereoAudioStream(
    GCAudioStreaming::AudioBufferMgr& mgr)
    : AudioStream(mgr, 2)
{
    m_pFile = NULL;
    m_Interleave = 0;
}

/**
 * Offset/Address/Size: 0x10BC | 0x80155E14 | size: 0x418
 * TODO: 86.8% match - double QUEUED_STREAM copy elided by MWCC, constructor register cascade,
 * placement new duplicate beq, memset arg swap
 */
void AudioStreamTrack::StreamTrack::QueueStream(
    unsigned long StreamId, float Volume, bool Looping,
    unsigned long FadeIn, const char* StreamParam,
    Audio::MasterVolume::VOLUME_GROUP OverrideVolGroup)
{
    char FileName[256];

    if (GetConfigBool(Config::Global(), "no_stream", false) == true)
    {
        return;
    }

    QUEUED_STREAM qs = { };
    DLListEntry<QUEUED_STREAM>* entry = m_QueuedStreams.Allocate(qs);
    nlDLRingAddEnd(&m_QueuedStreams.m_Head, entry);

    entry->m_data.StreamId = StreamId;

    GCAudioStreaming::StereoAudioStream* pStream = NULL;
    TrackManagerBase& mgr = m_TrackMgr;
    mgr.m_StreamPool.Allocate(pStream);
    if (pStream != NULL)
    {
        new (pStream) GCAudioStreaming::StereoAudioStream(g_BufferMgr);
    }

    entry->m_data.pStream = pStream;
    entry->m_data.FadeIn = FadeIn;
    entry->m_data.StartVolume = (int)(127.0f * Volume);

    Audio::MasterVolume::VOLUME_GROUP volGroup;
    if (OverrideVolGroup == 0)
    {
        volGroup = m_VolumeGroup;
    }
    else
    {
        volGroup = OverrideVolGroup;
    }
    entry->m_data.VolGroup = volGroup;
    entry->m_data.Loop = Looping;
    entry->m_data.TrackOwnsStream = m_TrackOwnsStreams;

    nlStrNCpy<char>(FileName, "audio/data/streams/", 0x100);
    TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP* lookup = nlBSearch<TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP, unsigned long>(
        StreamId, mgr.m_FileLookup.m_pLookup, mgr.m_FileLookup.m_StreamCount);

    char* percentPos = strchr(lookup->value, '%');
    if (percentPos != NULL)
    {
        unsigned long prefixLen = (unsigned long)(percentPos - lookup->value);
        char* dest = &FileName[19];
        nlStrNCpy<char>(dest, lookup->value, prefixLen + 1);
        unsigned long remainLen = 0xed - prefixLen;
        dest += prefixLen;
        Tag tag = mgr.m_FileLookup.m_ParamCBTag;
        if (tag == FREE_FUNCTION)
        {
            mgr.m_FileLookup.m_ParamCBFunc(StreamParam, dest, remainLen);
        }
        else
        {
            (*mgr.m_FileLookup.m_ParamCBFunctor)(StreamParam, dest, remainLen);
        }
        unsigned long cbLen = nlStrLen(dest);
        nlStrNCpy<char>(dest + cbLen, percentPos + 3, remainLen - cbLen);
    }
    else
    {
        nlStrNCpy<char>(&FileName[19], lookup->value, 0xed);
    }

    GCAudioStreaming::StereoAudioStream* stream = entry->m_data.pStream;
    stream->m_StreamLength = 0;
    stream->m_OldLength = 0;
    stream->m_StreamPos = 0;

    volatile unsigned long bufIdx = 0;
    GCAudioStreaming::AudioStreamBuffer* buf = NULL;
    if (stream->m_BufferCount > 0)
    {
        buf = stream->m_Buffers[0];
    }
    while (buf != NULL)
    {
        unsigned long i = bufIdx;
        unsigned long next = i + 1;
        bufIdx = next;
        stream->m_Buffers[i] = NULL;
        if (next < stream->m_BufferCount)
        {
            buf = stream->m_Buffers[next];
        }
        else
        {
            buf = NULL;
        }
    }

    stream->m_LastPlayable = 0;
    stream->m_Flags = 0;
    stream->m_Volume = 64;
    stream->m_LPFOn = 0;
    stream->m_LPFFreq = 0x3FFF;
    nlFile* file = nlOpen(FileName);
    stream->m_pFile = file;
    stream->m_State = GCAudioStreaming::SS_Initd;
    if (m_State == TS_Idle)
    {
        ProcessNewHeadStream();
    }
}

extern "C" void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan,
    unsigned char span, unsigned char auxa, unsigned char auxb);
extern "C" void sndStreamActivate(unsigned long stid);
extern "C" void sndStreamDeactivate(unsigned long stid);

/**
 * Offset/Address/Size: 0xE20 | 0x80155B78 | size: 0x29C
 * TODO: 96.2% match - volatile counter causes extra li per loop init, ble vs beq, r3/r4 register diffs
 */
void AudioStreamTrack::StreamTrack::ProcessNewHeadStream()
{
    if (m_QueuedStreams.m_Head == NULL)
    {
        if (m_State != TS_Idle)
        {
            m_State = TS_Idle;
            if ((bool)m_IdleCallback.mTag)
            {
                if (m_IdleCallback.mTag == FREE_FUNCTION)
                {
                    m_IdleCallback.mFreeFunction();
                }
                else
                {
                    (*m_IdleCallback.mFunctor)();
                }
            }
        }
        return;
    }

    DLListEntry<QUEUED_STREAM>* pEntry = nlDLRingGetStart(m_QueuedStreams.m_Head);

    struct Iter
    {
        DLListEntry<QUEUED_STREAM>* m_head;
        DLListEntry<QUEUED_STREAM>* m_current;
        ~Iter() { }
    };
    Iter iter;
    iter.m_current = pEntry;
    iter.m_head = m_QueuedStreams.m_Head;

    if (pEntry->m_data.StartVolume != 0)
    {
        Function<FnVoidVoid> callback;
        callback.mTag = EMPTY;

        m_TrackMgr.m_FadeMgr.AddFade(
            pEntry->m_data.pStream,
            0,
            pEntry->m_data.StartVolume,
            (Audio::MasterVolume::VOLUME_GROUP)pEntry->m_data.VolGroup,
            pEntry->m_data.FadeIn,
            callback);
    }

    GCAudioStreaming::StereoAudioStream* pStream = pEntry->m_data.pStream;

    if (pStream->m_State >= GCAudioStreaming::SS_Warming)
    {
        GCAudioStreaming::AudioStreamBuffer* buf = NULL;
        volatile unsigned long bufCounter = (unsigned long)buf;
        if (pStream->m_BufferCount > 0)
        {
            buf = pStream->m_Buffers[0];
        }

        while (buf != NULL)
        {
            buf->m_Volume = 0;
            sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);
            unsigned long ci = bufCounter + 1;
            bufCounter = ci;
            if (ci < pStream->m_BufferCount)
            {
                buf = pStream->m_Buffers[ci];
            }
            else
            {
                buf = NULL;
            }
        }
    }

    pStream->m_Volume = 0;
    GCAudioStreaming::StereoAudioStream* pStreamActive = pEntry->m_data.pStream;

    {
        unsigned long flags = pStreamActive->m_Flags;
        unsigned long loopBit = (pEntry->m_data.Loop);
        flags = (flags & ~(1 << 1)) | (loopBit << 1);
        pStreamActive->m_Flags = flags;
    }

    {
        unsigned long flags = pStreamActive->m_Flags;
        flags = (flags & ~(1 << 2)) | (1 << 2);
        pStreamActive->m_Flags = flags;
    }

    switch (pStreamActive->m_State)
    {
    case GCAudioStreaming::SS_Initd:
    {
        unsigned long flags = pStreamActive->m_Flags;
        flags = (flags & ~1) | 1;
        pStreamActive->m_Flags = flags;
        pStreamActive->Warm(true);
        break;
    }
    case GCAudioStreaming::SS_Warming:
    {
        unsigned long flags = pStreamActive->m_Flags;
        flags = (flags & ~1) | 1;
        pStreamActive->m_Flags = flags;
        break;
    }
    case GCAudioStreaming::SS_Warm:
    {
        GCAudioStreaming::AudioStreamBuffer* buf = NULL;
        volatile unsigned long bufCounter2 = (unsigned long)buf;
        if (pStreamActive->m_BufferCount > 0)
        {
            buf = pStreamActive->m_Buffers[0];
        }

        while (buf != NULL)
        {
            sndStreamActivate(buf->m_StreamId);
            unsigned long cj = bufCounter2 + 1;
            bufCounter2 = cj;
            if (cj < pStreamActive->m_BufferCount)
            {
                buf = pStreamActive->m_Buffers[cj];
            }
            else
            {
                buf = NULL;
            }
        }
        pStreamActive->m_State = GCAudioStreaming::SS_Playing;
        break;
    }
    default:
        break;
    }

    m_State = TS_Playing;
}

/**
 * Offset/Address/Size: 0xC90 | 0x801559E8 | size: 0x190
 * TODO: 97.0% match - lwz r0,8(r1) scheduled before bne in target but after in current
 */
void AudioStreamTrack::StreamTrack::StopHead(unsigned long Fadeout)
{
    FORCE_DONT_INLINE;
    DLListEntry<QUEUED_STREAM>* entry = nlDLRingGetStart(m_QueuedStreams.m_Head);

    if (Fadeout == 0)
    {
        StopQStream(&entry->m_data);
    }
    else
    {
        QUEUED_STREAM* qs = &entry->m_data;
        BindExp2_T bind = Bind<void>(
            MemFun<StreamTrack, void, QUEUED_STREAM*>(&StreamTrack::FadeOutDoneStartNext), this, qs);

        Function<FnVoidVoid> callback;
        callback.mTag = FUNCTOR;
        FunctorImpl_T* functor = new ((FunctorImpl_T*)nlMalloc(sizeof(FunctorImpl_T), 8, false))
            FunctorImpl_T(bind);
        callback.mFunctor = functor;

        StartQStreamFadeout(&entry->m_data, Fadeout, callback);
    }
}

/**
 * Offset/Address/Size: 0xA28 | 0x80155780 | size: 0x268
 * TODO: 90.8% match - r-diffs: qs in r8 vs r30, Fadeout in r30 vs r29; compiler uses 3 callee-saved registers instead of 4
 */
void AudioStreamTrack::StreamTrack::Stop(unsigned long Fadeout)
{
    DLListEntry<QUEUED_STREAM>* entry;
    DLListEntry<QUEUED_STREAM>* head;

    if (m_InFakePause)
        return;

    if (Fadeout == 0)
    {
        while (m_QueuedStreams.m_Head != NULL)
        {
            entry = nlDLRingGetStart(m_QueuedStreams.m_Head);
            head = m_QueuedStreams.m_Head;
            StopQStream(&entry->m_data);
        }
        return;
    }

    if (m_QueuedStreams.m_Head == NULL)
        return;

    entry = nlDLRingGetStart(m_QueuedStreams.m_Head);
    QUEUED_STREAM* qs = &entry->m_data;

    {
        BindExp2_T bind = Bind<void>(
            MemFun<StreamTrack, void, QUEUED_STREAM*>(&StreamTrack::FadeOutDone), this, qs);

        Function<FnVoidVoid> callback;
        callback.mTag = FUNCTOR;
        FunctorImpl_T* functor = new ((FunctorImpl_T*)nlMalloc(sizeof(FunctorImpl_T), 8, false))
            FunctorImpl_T(bind);
        callback.mFunctor = functor;

        StartQStreamFadeout(&entry->m_data, Fadeout, callback);
    }

    head = m_QueuedStreams.m_Head;
    DLListEntry<QUEUED_STREAM>* iter = nlDLRingGetStart(head);
    head = m_QueuedStreams.m_Head;

    if (&iter->m_data == qs)
    {
        if (nlDLRingIsEnd(head, iter) || iter == NULL)
        {
            iter = NULL;
        }
        else
        {
            iter = iter->m_next;
        }
    }

    while (iter != NULL)
    {
        QUEUED_STREAM* curQs = &iter->m_data;

        if (nlDLRingIsEnd(head, iter) || iter == NULL)
        {
            iter = NULL;
        }
        else
        {
            iter = iter->m_next;
        }

        StopQStream(curQs);
    }
}

/**
 * Offset/Address/Size: 0x888 | 0x801555E0 | size: 0xC4
 */
void AudioStreamTrack::StreamTrack::StopQStream(QUEUED_STREAM* pQueuedStream)
{
    FORCE_DONT_INLINE;
    unsigned char flags = *((unsigned char*)pQueuedStream + 0xB);
    StopStream(pQueuedStream->pStream, (flags >> 1) & 1);

    DLListEntry<QUEUED_STREAM>* entry = (DLListEntry<QUEUED_STREAM>*)((char*)pQueuedStream - 8);

    nlDLRingIsEnd(m_QueuedStreams.m_Head, entry);
    nlDLRingRemove(&m_QueuedStreams.m_Head, entry);
    entry->m_next = m_QueuedStreams.m_Allocator.m_pFree;
    m_QueuedStreams.m_Allocator.m_pFree = entry;

    if (m_QueuedStreams.m_Head == NULL)
    {
        if (m_State != TS_Idle)
        {
            m_State = TS_Idle;
            if ((bool)m_IdleCallback.mTag)
            {
                if (m_IdleCallback.mTag == FREE_FUNCTION)
                {
                    m_IdleCallback.mFreeFunction();
                }
                else
                {
                    (*m_IdleCallback.mFunctor)();
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x5B0 | 0x80155308 | size: 0x2D8
 * TODO: 94.08% match - fadeCtrl in callee-saved r25 vs volatile r3, volatile counter init reuse, ble vs beq, second loop r3/r4 swap
 */
void AudioStreamTrack::StreamTrack::StopStream(GCAudioStreaming::StereoAudioStream* pStream, bool TrackOwns)
{
    pStream->m_Flags = pStream->m_Flags & ~1;

    if (pStream->m_State == GCAudioStreaming::SS_Playing)
    {
        GCAudioStreaming::AudioStreamBuffer* buf = NULL;
        volatile unsigned long bufCounter = (unsigned long)buf;
        if (pStream->m_BufferCount > 0)
            buf = pStream->m_Buffers[0];

        while (buf != NULL)
        {
            buf->m_Volume = 0;
            sndStreamMixParameterEx(buf->m_StreamId, buf->m_Volume, buf->m_Pan, buf->m_SurroundPan, 0, 0);
            sndStreamDeactivate(buf->m_StreamId);
            pStream->m_State = GCAudioStreaming::SS_Warm;
            unsigned long ci = bufCounter + 1;
            bufCounter = ci;
            if (ci < pStream->m_BufferCount)
                buf = pStream->m_Buffers[ci];
            else
                buf = NULL;
        }

        pStream->m_StreamPos = 0;
        pStream->m_State = GCAudioStreaming::SS_Warm;
    }

    pStream->CancelPendingReads();

    unsigned long flags = pStream->m_Flags;
    if (flags & 4)
    {
        pStream->m_Flags = flags & ~4;

        if (pStream->m_State > GCAudioStreaming::SS_Initd)
        {
            unsigned long fl = pStream->m_Flags;
            GCAudioStreaming::AudioStreamBuffer* buf = NULL;
            volatile unsigned long bufCounter = (unsigned long)buf;
            pStream->m_Flags = (fl & ~0x10) | 0x10;
            if (pStream->m_BufferCount > 0)
                buf = pStream->m_Buffers[0];

            while (buf != NULL)
            {
                pStream->m_BuffMgr.FreeBuffer(buf);
                unsigned long ci = bufCounter;
                unsigned long nextCI = ci + 1;
                bufCounter = nextCI;
                pStream->m_Buffers[ci] = NULL;
                if (nextCI < pStream->m_BufferCount)
                    buf = pStream->m_Buffers[nextCI];
                else
                    buf = NULL;
            }

            pStream->m_State = GCAudioStreaming::SS_Initd;
        }
    }

    typedef TrackManagerBase::FadeManager::STREAM_FADE_CTRL FadeCtrl;
    typedef DLListEntry<FadeCtrl> FadeEntry;

    TrackManagerBase& mgr = m_TrackMgr;
    FadeEntry* fadeIter = nlDLRingGetStart(mgr.m_FadeMgr.m_Fades.m_Head);
    FadeEntry* fadeHead = mgr.m_FadeMgr.m_Fades.m_Head;
    FadeCtrl* fadeCtrl = NULL;

    while (fadeIter != NULL)
    {
        if (fadeIter->m_data.pStream == pStream)
        {
            fadeCtrl = &fadeIter->m_data;
            break;
        }

        if (nlDLRingIsEnd(fadeHead, fadeIter) || fadeIter == NULL)
            fadeIter = NULL;
        else
            fadeIter = fadeIter->m_next;
    }

    if (fadeCtrl != NULL)
    {
        FadeEntry* fadeEntry = (FadeEntry*)((char*)fadeCtrl - 8);
        nlDLRingIsEnd(mgr.m_FadeMgr.m_Fades.m_Head, fadeEntry);
        nlDLRingRemove(&mgr.m_FadeMgr.m_Fades.m_Head, fadeEntry);

        if (fadeEntry != NULL)
        {
            fadeEntry->m_data.~FadeCtrl();
        }

        mgr.m_FadeMgr.m_Fades.m_Allocator.Free(fadeEntry);
    }

    if (TrackOwns)
    {
        typedef DLListEntry<GCAudioStreaming::StereoAudioStream*> StreamEntry;
        TrackManagerBase& delMgr = m_TrackMgr;
        StreamEntry* entry = NULL;
        delMgr.m_StreamDeleteList.m_Allocator.Allocate(entry);
        if (entry != NULL)
        {
            entry->m_next = NULL;
            entry->m_prev = NULL;
            entry->m_data = pStream;
        }
        nlDLRingAddEnd(&delMgr.m_StreamDeleteList.m_Head, entry);
    }
}

/**
 * Offset/Address/Size: 0x590 | 0x801552E8 | size: 0x20
 */
void AudioStreamTrack::StreamTrack::FadeOutDone(AudioStreamTrack::StreamTrack::QUEUED_STREAM* qs)
{
    FORCE_DONT_INLINE;
    StopQStream(qs);
}

/**
 * Offset/Address/Size: 0x55C | 0x801552B4 | size: 0x34
 */
void AudioStreamTrack::StreamTrack::FadeOutDoneStartNext(AudioStreamTrack::StreamTrack::QUEUED_STREAM* qs)
{
    FadeOutDone(qs);
    ProcessNewHeadStream();
}

// /**
//  * Offset/Address/Size: 0x1E8 | 0x80154F40 | size: 0x374
//  */
// void AudioStreamTrack::StreamTrack::Pause(unsigned long, bool)
// {
// }

/**
 * Offset/Address/Size: 0x1B8 | 0x80154F10 | size: 0x30
 */
void AudioStreamTrack::StreamTrack::Resume()
{
    m_InFakePause = 0;
    ProcessNewHeadStream();
}

/**
 * Offset/Address/Size: 0x0 | 0x80154D58 | size: 0x1B8
 * TODO: 91.3% match - missing double QUEUED_STREAM stack copy, register allocation shift, null check pattern
 */
void AudioStreamTrack::StreamTrack::AttachStream(
    GCAudioStreaming::StereoAudioStream* pStream,
    Audio::MasterVolume::VOLUME_GROUP VolGroup,
    unsigned long StreamId,
    unsigned long FadeIn,
    bool Looping,
    bool TrackOwnsStream)
{
    if (m_State != TS_Idle)
    {
        return;
    }

    if (GetConfigBool(Config::Global(), "no_stream", false) == true)
    {
        return;
    }

    QUEUED_STREAM qs = { };
    DLListEntry<QUEUED_STREAM>* entry = m_QueuedStreams.Allocate(qs);
    nlDLRingAddEnd(&m_QueuedStreams.m_Head, entry);

    entry->m_data.StreamId = StreamId;
    entry->m_data.pStream = pStream;
    entry->m_data.FadeIn = FadeIn;
    entry->m_data.StartVolume = (u8)pStream->m_Volume;
    entry->m_data.Loop = Looping;
    entry->m_data.VolGroup = VolGroup;
    entry->m_data.TrackOwnsStream = TrackOwnsStream;

    m_State = TS_Playing;
}
