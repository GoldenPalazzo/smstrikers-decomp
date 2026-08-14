#include "Game/Sys/GCStream.h"
#include "Game/Audio/StreamTrack.h"
#include "NL/nlAlgorithm.h"
#include "NL/nlConfig.h"
#include "NL/nlFileGC.h"
#include "NL/nlList.h"
#include "NL/nlString.h"

extern GCAudioStreaming::AudioBufferMgr g_BufferMgr;

#include "NL/nlBind.h"

namespace AudioStreamTrack
{
TrackManagerBase::StreamFileLookup::StreamFileLookup(
    const char* /*name*/,
    const Function<bool(const char*, char*, unsigned long)>& ParamCB)
    : m_ParamCB(ParamCB)
    , m_pLookup(NULL)
    , m_StreamCount(0)
{
    typedef STREAM_FILE_LIST_LOOKUP LookupT;
    typedef ListEntry<LookupT> ListEntryT;

    unsigned long TotalFileNameMem = 0;
    nlListSlotPoolHigh<LookupT> LookupList(16, 16);
    Config config(Config::ALLOCATE_HIGH);
    config.LoadFromFile("audio/data/streams/StreamNames.txt");

    for (Config::Iterator<const char*> iter(config); iter.IsValid(); iter.Next())
    {
        ListEntryT* entry = LookupList.Allocate(LookupT());
        nlListAddEnd(&LookupList.m_Head, &LookupList.m_Tail, entry);

        entry->entry.NameHash = nlStringLowerHash(iter.Tag());
        entry->entry.FileName = iter.Current();

        entry->entry.FileNameLen = nlStrLen(entry->entry.FileName) + 1;

        TotalFileNameMem += entry->entry.FileNameLen;
        ++m_StreamCount;
    }

    m_pLookup = (STREAM_FILE_LOOKUP*)nlMalloc(
        m_StreamCount * sizeof(STREAM_FILE_LOOKUP), 8, false);
    m_pStrings = (char*)nlMalloc(TotalFileNameMem, 8, false);

    int i = 0;
    char* pLastString = m_pStrings;
    nlListIterator<LookupT> listIter = LookupList.Begin();
    while (listIter.IsValid())
    {
        m_pLookup[i].NameHash = listIter.Current().NameHash;
        m_pLookup[i].FileName = pLastString;
        memcpy(pLastString, listIter.Current().FileName, listIter.Current().FileNameLen);

        i++;
        pLastString += listIter.Current().FileNameLen;
        listIter.Next();
    }

    nlQSort<STREAM_FILE_LOOKUP>(
        m_pLookup, (int)m_StreamCount,
        &nlDefaultQSortComparer<STREAM_FILE_LOOKUP>);

    LookupList.Clear();
    SlotPoolBase::BaseFreeBlocks(&LookupList.m_Allocator, sizeof(ListEntryT));
}

} // namespace AudioStreamTrack

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

/**
 * Offset/Address/Size: 0x2038 | 0x80156D90 | size: 0x104
 */
void AudioStreamTrack::TrackManagerBase::FadeManager::AddFade(
    GCAudioStreaming::StereoAudioStream* pStream, unsigned long startVol,
    unsigned long endVol, Audio::MasterVolume::VOLUME_GROUP volGroup,
    unsigned long fadeLength, const Function<FnVoidVoid>& callback)
{
    FORCE_DONT_INLINE;
    typedef STREAM_FADE_CTRL FadeCtrl;

    FadeCtrl* fadeCtrl = m_Fades.AllocateAtEnd(NULL);

    new (&fadeCtrl->Callback) Function<FnVoidVoid>();

    fadeCtrl->pStream = pStream;
    fadeCtrl->StartVol = startVol;
    fadeCtrl->EndVol = endVol;
    fadeCtrl->VolumeGroup = volGroup;
    fadeCtrl->FadeLength = fadeLength;
    fadeCtrl->Interp = 0.0f;

    fadeCtrl->Callback = callback;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x801573F0 | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*> >::~FunctorImpl()
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
        if (entry->entry->SafeToPurge())
        {
            pStream = entry->entry;
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

extern "C" void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan,
    unsigned char span, unsigned char auxa, unsigned char auxb);

void AudioStreamTrack::TrackManagerBase::FadeManager::CompleteFade(
    STREAM_FADE_CTRL* fadeCtrl)
{
    typedef DLListEntry<STREAM_FADE_CTRL> FadeEntry;

    Function<FnVoidVoid> callback(fadeCtrl->Callback);

    FadeEntry* entry = (FadeEntry*)((char*)fadeCtrl - 8);
    nlDLRingIsEnd(m_Fades.m_Head, entry);
    nlDLRingRemove(&m_Fades.m_Head, entry);

    m_Fades.DeleteEntry(entry);

    if (callback)
    {
        callback();
    }
}

/**
 * Offset/Address/Size: 0x1D60 | 0x80156AB8 | size: 0x2D8
 */
bool AudioStreamTrack::TrackManagerBase::FadeManager::ChangeFade(
    GCAudioStreaming::StereoAudioStream* pStream, unsigned long endVol,
    unsigned long fadeLength, const Function<FnVoidVoid>& callback)
{
    typedef STREAM_FADE_CTRL FadeCtrl;

    FadeCtrl* fadeCtrl = FindFade(pStream);

    if (fadeCtrl == NULL)
    {
        return false;
    }

    fadeCtrl->Callback = callback;

    unsigned long startVol = fadeCtrl->StartVol;
    unsigned long curEndVol = fadeCtrl->EndVol;
    int diff = (int)curEndVol - (int)startVol;
    float curVol =
        fadeCtrl->Interp * (float)diff + (float)startVol;

    if (endVol == (unsigned char)curVol)
    {
        CompleteFade(fadeCtrl);
    }
    else
    {
        fadeCtrl->StartVol = (int)curVol;
        fadeCtrl->Interp = 0.0f;
        fadeCtrl->EndVol = endVol;
        fadeCtrl->FadeLength = fadeLength;
    }

    return true;
}

static inline float ClampFadeInterp(float x)
{
    if (x <= 1.0f)
    {
        return x;
    }
    else
    {
        return 1.0f;
    }
}

/**
 * Offset/Address/Size: 0x1970 | 0x801566C8 | size: 0x3F0
 */
void AudioStreamTrack::TrackManagerBase::FadeManager::UpdateFade(STREAM_FADE_CTRL* pFade)
{
    unsigned long fadeLength = pFade->FadeLength;
    float totalTime;
    if (fadeLength != 0)
    {
        totalTime = (float)fadeLength;
    }
    else
    {
        totalTime = m_dT;
    }

    float& interp = pFade->Interp;
    interp = interp + (m_dT / totalTime);

    float newVal = ClampFadeInterp(pFade->Interp);
    pFade->Interp = newVal;

    float absDiff = (float)__fabs(pFade->Interp - 1.0f);
    if (absDiff <= 0.0001f)
    {
        float masterVol = Audio::MasterVolume::GetVolume(
            (Audio::MasterVolume::VOLUME_GROUP)pFade->VolumeGroup);

        unsigned long endVol = pFade->EndVol;

        pFade->pStream->SetVolume(
            (int)((float)endVol * masterVol));

        CompleteFade(pFade);
    }
    else
    {
        unsigned long startVol = pFade->StartVol;
        unsigned long endVol = pFade->EndVol;
        int diff = (int)endVol - (int)startVol;
        Audio::MasterVolume::VOLUME_GROUP vg = (Audio::MasterVolume::VOLUME_GROUP)pFade->VolumeGroup;

        float interpVol = pFade->Interp * (float)diff + (float)startVol;
        int interpVolInt = (int)interpVol;

        float masterVol = Audio::MasterVolume::GetVolume(vg);

        pFade->pStream->SetVolume(
            (int)((float)(u8)interpVolInt * masterVol));
    }
}

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
    QUEUED_STREAM* qs = &entry->entry;

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
void AudioStreamTrack::StreamTrack::PlayStream(
    unsigned long StreamId, float Volume, bool Looping,
    unsigned long FadeIn, unsigned long ExistingFadeOut,
    const char* StreamParam, Audio::MasterVolume::VOLUME_GROUP OverrideVolGroup)
{
    if (GetConfigBool(Config::Global(), "no_stream", false) == true)
    {
        return;
    }

    TRACK_STATE StateAtStart = m_State;
    QueueStream(StreamId, Volume, Looping, FadeIn, StreamParam, OverrideVolGroup);

    if (StateAtStart != TS_Playing)
    {
        return;
    }

    QUEUED_STREAM Head;
    m_QueuedStreams.RemoveStart(&Head);

    QUEUED_STREAM Tail;
    m_QueuedStreams.Deallocate(nlDLRingRemoveEnd(&m_QueuedStreams.m_Head), &Tail);

    while (m_QueuedStreams.m_Head != NULL)
    {
        QUEUED_STREAM QStream;
        m_QueuedStreams.RemoveStart(&QStream);
        StopStream(QStream.pStream, QStream.TrackOwnsStream & 1);
    }

    m_QueuedStreams.AddStart(Head);
    m_QueuedStreams.AddEnd(Tail);

    DLListEntry<QUEUED_STREAM>* startEntry = nlDLRingGetStart(m_QueuedStreams.m_Head);
    struct Iter
    {
        DLListEntry<QUEUED_STREAM>* m_head;
        DLListEntry<QUEUED_STREAM>* m_current;
        ~Iter() { }
    };
    Iter iter;
    iter.m_head = m_QueuedStreams.m_Head;
    iter.m_current = startEntry;
    QUEUED_STREAM* pHead = &startEntry->entry;

    Function<FnVoidVoid> callback(Bind<void>(
        MemFun<StreamTrack, void, QUEUED_STREAM*>(&StreamTrack::FadeOutDoneStartNext), this, pHead));
    StartQStreamFadeout(pHead, ExistingFadeOut, callback);
}

inline GCAudioStreaming::AudioStream::AudioStream(GCAudioStreaming::AudioBufferMgr& mgr,
    unsigned long bufCount)
    : m_FlagAtDelete(0)
    , m_State(GCAudioStreaming::SS_New)
    , m_StreamLength((unsigned long)-1)
    , m_StreamPos(0)
    , m_OldLength(0)
    , m_BuffMgr(mgr)
    , m_Flags(0)
    , m_BufferCount(bufCount)
{
    memset(m_Buffers, sizeof(m_Buffers), 0);
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

    QUEUED_STREAM* queuedStream = m_QueuedStreams.AllocateAtEnd(NULL);
    queuedStream->StreamId = StreamId;

    TrackManagerBase& mgr = m_TrackMgr;
    GCAudioStreaming::StereoAudioStream* pStream =
        mgr.m_StreamPool.Allocate();
    new (pStream) GCAudioStreaming::StereoAudioStream(g_BufferMgr);

    queuedStream->pStream = pStream;
    queuedStream->FadeIn = FadeIn;
    queuedStream->StartVolume = (int)(127.0f * Volume);

    queuedStream->VolGroup =
        OverrideVolGroup == 0 ? m_VolumeGroup : OverrideVolGroup;
    queuedStream->Loop = Looping;
    queuedStream->TrackOwnsStream = m_TrackOwnsStreams;

    nlStrNCpy<char>(FileName, "audio/data/streams/", 0x100);
    unsigned long lookupKey = StreamId;
    unsigned long prefixLen;
    TrackManagerBase& lookupMgr = m_TrackMgr;
    TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP* lookup = nlBSearch<TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP, unsigned long>(
        lookupKey, lookupMgr.m_FileLookup.m_pLookup, lookupMgr.m_FileLookup.m_StreamCount);

    const char* percentPos = strchr(lookup->FileName, '%');
    if (percentPos != NULL)
    {
        prefixLen = (unsigned long)(percentPos - lookup->FileName);
        char* dest = &FileName[19];
        nlStrNCpy<char>(dest, lookup->FileName, prefixLen + 1);
        unsigned long remainLen = 0xed - prefixLen;
        dest += prefixLen;
        lookupMgr.m_FileLookup.m_ParamCB(StreamParam, dest, remainLen);
        unsigned long cbLen = nlStrLen(dest);
        nlStrNCpy<char>(dest + cbLen, percentPos + 3, remainLen - cbLen);
    }
    else
    {
        nlStrNCpy<char>(&FileName[19], lookup->FileName, 0xed);
    }

    queuedStream->pStream->Open(FileName);
    if (m_State == TS_Idle)
    {
        ProcessNewHeadStream();
    }
}

extern "C" bool sndStreamActivate(unsigned long stid);
extern "C" void sndStreamDeactivate(unsigned long stid);

/**
 * Offset/Address/Size: 0xE20 | 0x80155B78 | size: 0x29C
 */
void AudioStreamTrack::StreamTrack::ProcessNewHeadStream()
{
    if (m_QueuedStreams.m_Head == NULL)
    {
        SetIdleState();
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

    if (pEntry->entry.StartVolume != 0)
    {
        Function<FnVoidVoid> callback;

        m_TrackMgr.m_FadeMgr.AddFade(
            pEntry->entry.pStream,
            0,
            pEntry->entry.StartVolume,
            (Audio::MasterVolume::VOLUME_GROUP)pEntry->entry.VolGroup,
            pEntry->entry.FadeIn,
            callback);
    }

    GCAudioStreaming::StereoAudioStream* pStream = pEntry->entry.pStream;
    unsigned long zero = 0;
    GCAudioStreaming::AudioStreamBuffer* buf;

    if (pStream->m_State >= GCAudioStreaming::SS_Warming)
    {
        volatile unsigned long bufCounter = (unsigned long)(buf = NULL);
        if (pStream->m_BufferCount > zero)
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

    pStream->m_Volume = zero;
    GCAudioStreaming::StereoAudioStream* pStreamActive = pEntry->entry.pStream;

    pStreamActive->m_Flags = (pStreamActive->m_Flags & ~(1 << 1)) | (pEntry->entry.Loop << 1);
    pStreamActive->m_Flags = (pStreamActive->m_Flags & ~(1 << 2)) | (1 << 2);

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
        volatile unsigned long bufCounter2 = (unsigned long)(buf = NULL);
        if (pStreamActive->m_BufferCount > zero)
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

void AudioStreamTrack::StreamTrack::SetIdleState()
{
    if (m_State != TS_Idle)
    {
        m_State = TS_Idle;
        if (m_IdleCallback)
        {
            m_IdleCallback();
        }
    }
}

/**
 * Offset/Address/Size: 0xC90 | 0x801559E8 | size: 0x190
 */
void AudioStreamTrack::StreamTrack::StopHead(unsigned long Fadeout)
{
    FORCE_DONT_INLINE;
    DLListEntry<QUEUED_STREAM>* entry = nlDLRingGetStart(m_QueuedStreams.m_Head);
    StreamTrack* pTrack = this;

    if (Fadeout == 0)
    {
        pTrack->StopQStream(&entry->entry);
    }
    else
    {
        QUEUED_STREAM* qs = &entry->entry;
        Function<FnVoidVoid> callback(Bind<void>(
            MemFun<StreamTrack, void, QUEUED_STREAM*>(&StreamTrack::FadeOutDoneStartNext), this, qs));
        StartQStreamFadeout(&entry->entry, Fadeout, callback);
    }
}

/**
 * Offset/Address/Size: 0xA28 | 0x80155780 | size: 0x268
 * TODO: 98.50% match - post-fade list head uses r29 vs r28 and curQs uses r30 vs r29
 */
void AudioStreamTrack::StreamTrack::Stop(unsigned long Fadeout)
{
    DLListEntry<QUEUED_STREAM>* entry;

    if (m_InFakePause)
        return;

    if (Fadeout == 0)
    {
        struct Iter
        {
            DLListEntry<QUEUED_STREAM>* m_head;
            DLListEntry<QUEUED_STREAM>* m_current;
            ~Iter() { }
        } iter;

        while (m_QueuedStreams.m_Head != NULL)
        {
            iter.m_current = nlDLRingGetStart(m_QueuedStreams.m_Head);
            iter.m_head = m_QueuedStreams.m_Head;
            StopQStream(&iter.m_current->entry);
        }
        return;
    }

    if (m_QueuedStreams.m_Head == NULL)
        return;

    entry = nlDLRingGetStart(m_QueuedStreams.m_Head);
    QUEUED_STREAM* qs = &entry->entry;

    {
        Function<FnVoidVoid> callback(Bind<void>(
            MemFun<StreamTrack, void, QUEUED_STREAM*>(&StreamTrack::FadeOutDone), this, &entry->entry));

        StartQStreamFadeout(&entry->entry, Fadeout, callback);
    }

    QUEUED_STREAM* curQs;
    DLListEntry<QUEUED_STREAM>* iter = nlDLRingGetStart(m_QueuedStreams.m_Head);
    DLListEntry<QUEUED_STREAM>* head = m_QueuedStreams.m_Head;

    if (&iter->entry == qs)
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
        curQs = &iter->entry;

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
 * Offset/Address/Size: 0x9C4 | 0x801556A4 | size: 0xDC
 */
void AudioStreamTrack::StreamTrack::StartQStreamFadeout(
    QUEUED_STREAM* pQS, unsigned long Fadeout, const Function<FnVoidVoid>& callback)
{
    FORCE_DONT_INLINE;
    if (!m_TrackMgr.m_FadeMgr.ChangeFade(pQS->pStream, 0, Fadeout, callback))
    {
        GCAudioStreaming::StereoAudioStream* pStream = pQS->pStream;
        unsigned char vol = (unsigned char)pStream->m_Volume;
        if (vol != 0)
        {
            m_TrackMgr.m_FadeMgr.AddFade(pStream, vol, 0, (Audio::MasterVolume::VOLUME_GROUP)pQS->VolGroup, Fadeout, callback);
        }
        else
        {
            if (callback)
            {
                callback();
            }
        }
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
        SetIdleState();
    }
}

/**
 * Offset/Address/Size: 0x5B0 | 0x80155308 | size: 0x2D8
 */
void AudioStreamTrack::StreamTrack::StopStream(GCAudioStreaming::StereoAudioStream* pStream, bool TrackOwns)
{
    pStream->m_Flags = pStream->m_Flags & ~1;

    if (pStream->m_State == GCAudioStreaming::SS_Playing)
    {
        unsigned long zero = 0;
        GCAudioStreaming::AudioStreamBuffer* buf;
        volatile unsigned long bufCounter = (unsigned long)(buf = NULL);
        if (pStream->m_BufferCount > zero)
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

        pStream->m_StreamPos = zero;
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
            unsigned long zero2 = 0;
            GCAudioStreaming::AudioStreamBuffer* buf;
            volatile unsigned long bufCounter = (unsigned long)(buf = NULL);
            pStream->m_Flags = (fl & ~0x10) | 0x10;
            if (pStream->m_BufferCount > zero2)
                buf = pStream->m_Buffers[0];

            while (buf != NULL)
            {
                pStream->m_BuffMgr.FreeBuffer(buf);
                unsigned long ci = bufCounter;
                pStream->m_Buffers[ci] = NULL;
                ci++;
                bufCounter = ci;
                if (ci < pStream->m_BufferCount)
                    buf = pStream->m_Buffers[ci];
                else
                    buf = NULL;
            }

            pStream->m_State = GCAudioStreaming::SS_Initd;
        }
    }

    typedef TrackManagerBase::FadeManager::STREAM_FADE_CTRL FadeCtrl;
    typedef DLListEntry<FadeCtrl> FadeEntry;
    typedef DLListEntry<GCAudioStreaming::StereoAudioStream*> StreamEntry;

    TrackManagerBase* delMgr;
    StreamEntry* entry;
    TrackManagerBase& mgr = m_TrackMgr;
    FadeEntry* fadeEntry;
    FadeEntry* fadeHead;
    FadeEntry* fadeIter = nlDLRingGetStart(mgr.m_FadeMgr.m_Fades.m_Head);
    fadeHead = mgr.m_FadeMgr.m_Fades.m_Head;
    FadeCtrl* fadeCtrl;

    while (fadeIter != NULL)
    {
        if (fadeIter->entry.pStream == pStream)
        {
            fadeCtrl = &fadeIter->entry;
            goto fade_found;
        }

        if (nlDLRingIsEnd(fadeHead, fadeIter) || fadeIter == NULL)
            fadeIter = NULL;
        else
            fadeIter = fadeIter->m_next;
    }
    fadeCtrl = NULL;
fade_found:

    if (fadeCtrl != NULL)
    {
        fadeEntry = (FadeEntry*)((char*)fadeCtrl - 8);
        nlDLRingIsEnd(mgr.m_FadeMgr.m_Fades.m_Head, fadeEntry);
        nlDLRingRemove(&mgr.m_FadeMgr.m_Fades.m_Head, fadeEntry);

        mgr.m_FadeMgr.m_Fades.DeleteEntry(fadeEntry);
    }

    if (TrackOwns)
    {
        delMgr = &m_TrackMgr;
        entry = NULL;
        delMgr->m_StreamDeleteList.m_Allocator.Allocate(entry);
        if (entry != NULL)
        {
            entry->m_next = NULL;
            entry->m_prev = NULL;
            entry->entry = pStream;
        }
        nlDLRingAddEnd(&delMgr->m_StreamDeleteList.m_Head, entry);
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

void AudioStreamTrack::StreamTrack::UpdateLPF()
{
}

/**
 * Offset/Address/Size: 0x1E8 | 0x80154F40 | size: 0x374
 */
void AudioStreamTrack::StreamTrack::Pause(unsigned long Fadeout, bool bPause)
{
    m_InFakePause = 1;

    QUEUED_STREAM* qs = m_QueuedStreams.GetHead();

    if (qs == NULL)
    {
        return;
    }

    unsigned long endVol;
    if (m_TrackMgr.m_FadeMgr.IsFading(qs->pStream, &endVol) && endVol == 0)
    {
        StopHead(Fadeout);
        return;
    }

    m_TrackMgr.m_FadeMgr.RemoveFade(qs->pStream);

    if (bPause)
    {
        return;
    }

    GCAudioStreaming::StereoAudioStream* stream = qs->pStream;

    stream->StopPlaying();

    stream->CancelPendingReads();

    stream->Cool();
}

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

    QUEUED_STREAM* queuedStream = m_QueuedStreams.AllocateAtEnd(NULL);

    queuedStream->StreamId = StreamId;
    queuedStream->pStream = pStream;
    queuedStream->FadeIn = FadeIn;
    Looping = Looping & 1;
    TrackOwnsStream = TrackOwnsStream & 1;
    queuedStream->StartVolume = (u8)pStream->m_Volume;
    queuedStream->Loop = Looping;
    queuedStream->VolGroup = VolGroup;
    queuedStream->TrackOwnsStream = TrackOwnsStream;

    m_State = TS_Playing;
}
