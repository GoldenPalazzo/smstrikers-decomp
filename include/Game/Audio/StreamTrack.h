#ifndef _STREAMTRACK_H_
#define _STREAMTRACK_H_

#include "Game/Sys/audio.h"
#include "NL/nlFunction.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlSlotPool.h"
#include "NL/nlSortedSlot.h"
#include "NL/WalkHelper.h"

namespace GCAudioStreaming
{
class StereoAudioStream;
}

namespace Audio
{
bool TrackMgrFileNameParamLookup(const char*, char*, unsigned long);
} // namespace Audio

namespace AudioStreamTrack
{

enum TRACK_STATE
{
    TS_Idle = 0,
    TS_Playing = 1,
};

class StreamTrack;

class TrackManagerBase
{
public:
    TrackManagerBase();
    TrackManagerBase(const Function<bool(const char*, char*, unsigned long)>&);

    class StreamFileLookup
    {
    public:
        struct STREAM_FILE_LIST_LOOKUP
        {
            /* 0x00 */ unsigned long key;
            /* 0x04 */ const char* value;
            /* 0x08 */ unsigned long length;
        }; // total size: 0xC

        struct STREAM_FILE_LOOKUP
        {
            /* 0x00 */ unsigned long hash;
            /* 0x04 */ char* value;

            operator unsigned long() const { return hash; }
        }; // total size: 0x8

        StreamFileLookup(const char* name,
            const Function<bool(const char*, char*, unsigned long)>& fn);

        /* 0x00 */ Function<bool(const char*, char*, unsigned long)> m_ParamCB;
        /* 0x08 */ STREAM_FILE_LOOKUP* m_pLookup;
        /* 0x0C */ unsigned long m_StreamCount;
        /* 0x10 */ char* m_pStrings;
    }; // total size: 0x14

    typedef DLListEntry<GCAudioStreaming::StereoAudioStream*> StreamDeleteEntry;
    typedef BasicSlotPool<StreamDeleteEntry> StreamDeleteAllocator;
    typedef DLListContainerBase<GCAudioStreaming::StereoAudioStream*, StreamDeleteAllocator> StreamDeleteList;

    class FadeManager
    {
    public:
        struct STREAM_FADE_CTRL
        {
            STREAM_FADE_CTRL()
                : Callback()
            {
            }

            /* 0x00 */ Function<FnVoidVoid> Callback;
            /* 0x08 */ GCAudioStreaming::StereoAudioStream* pStream;
            /* 0x0C */ float Interp;
            /* 0x10 */ unsigned long FadeLength : 14;
            /* 0x10 */ unsigned long StartVol : 7;
            /* 0x10 */ unsigned long EndVol : 7;
            /* 0x10 */ unsigned long VolumeGroup : 3;
        }; // total size: 0x14

        typedef nlDLListSlotPool<STREAM_FADE_CTRL> FadeList;

        void Update(float dT);
        void Clear()
        {
            m_Fades.Clear();
            SlotPoolBase::BaseFreeBlocks(
                &m_Fades.m_Allocator, sizeof(DLListEntry<STREAM_FADE_CTRL>));
        }
        ~FadeManager()
        {
            Clear();
        }
        void CompleteFade(STREAM_FADE_CTRL*);
        void UpdateFade(STREAM_FADE_CTRL*);
        bool ChangeFade(GCAudioStreaming::StereoAudioStream*, unsigned long, unsigned long,
            const Function<FnVoidVoid>&);
        void AddFade(GCAudioStreaming::StereoAudioStream*, unsigned long, unsigned long,
            Audio::MasterVolume::VOLUME_GROUP, unsigned long, const Function<FnVoidVoid>&);

        /* 0x00 */ FadeList m_Fades;
        /* 0x1C */ float m_dT;
    }; // total size: 0x20

    virtual ~TrackManagerBase()
    {
        SlotPoolBase::BaseFreeBlocks(&m_StreamPool, 0x40);
    }
    /* 0x0C */ virtual void Update(float);
    /* 0x10 */ virtual StreamTrack& CreateTrack(const char*, Audio::MasterVolume::VOLUME_GROUP);
    /* 0x14 */ virtual void DestroyAllTracks();
    /* 0x18 */ virtual StreamTrack* GetTrack(unsigned long);
    /* 0x1C */ virtual void StopAllTracks(unsigned long);
    /* 0x20 */ virtual void OnMasterVolumeChange(Audio::MasterVolume::VOLUME_GROUP);

    /* 0x04 */ StreamFileLookup m_FileLookup;
    /* 0x18 */ FadeManager m_FadeMgr;
    /* 0x38 */ SlotPool<GCAudioStreaming::StereoAudioStream> m_StreamPool;
    /* 0x50 */ nlDLListSlotPool<GCAudioStreaming::StereoAudioStream*> m_StreamDeleteList;
}; // total size: 0x6C

inline void TrackManagerBase::FadeManager::Update(float dT)
{
    typedef WalkHelper<STREAM_FADE_CTRL, DLListEntry<STREAM_FADE_CTRL>, FadeManager> FadeWalkHelper;
    typedef void (FadeWalkHelper::*WalkCBType)(DLListEntry<STREAM_FADE_CTRL>*);

    m_dT = dT * 1000.0f;

    FadeWalkHelper helper;
    helper.m_CBClass = this;
    helper.m_CB = &FadeManager::UpdateFade;
    WalkCBType callback = &FadeWalkHelper::Callback;
    nlWalkDLRing(m_Fades.m_Head, &helper, callback);
}

class StreamTrack
{
public:
    struct QUEUED_STREAM
    {
        /* 0x0 */ unsigned long StreamId;
        /* 0x4 */ GCAudioStreaming::StereoAudioStream* pStream;
        /* 0x8 */ unsigned long FadeIn : 16;
        /* 0x8 */ unsigned long StartVolume : 10;
        /* 0x8 */ unsigned long VolGroup : 3;
        /* 0xB */ unsigned long Loop : 1;
        /* 0xB */ unsigned long TrackOwnsStream : 1;
    }; // total size: 0xC

    StreamTrack(TrackManagerBase& mgr, Audio::MasterVolume::VOLUME_GROUP volumeGroup);
    ~StreamTrack() { }

    void Update(float);
    void PlayStream(unsigned long, float, bool, unsigned long, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP);
    void QueueStream(unsigned long, float, bool, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP);
    void ProcessNewHeadStream();
    void SetIdleState();
    void StopHead(unsigned long);
    void Stop(unsigned long);
    void StopQStream(QUEUED_STREAM*);
    void StopStream(GCAudioStreaming::StereoAudioStream*, bool);
    void FadeOutDone(QUEUED_STREAM*);
    void FadeOutDoneStartNext(QUEUED_STREAM*);
    void StartQStreamFadeout(QUEUED_STREAM*, unsigned long, const Function<FnVoidVoid>&);
    void Pause(unsigned long, bool);
    void UpdateLPF();
    void Resume();
    void AttachStream(GCAudioStreaming::StereoAudioStream*, Audio::MasterVolume::VOLUME_GROUP, unsigned long, unsigned long, bool, bool);

    /* 0x00 */ TrackManagerBase& m_TrackMgr;
    /* 0x04 */ DLListContainerBase<QUEUED_STREAM, nlStaticArrayAllocator<DLListEntry<QUEUED_STREAM>, 4> > m_QueuedStreams;
    /* 0x5C */ unsigned long m_LPFFreq;
    /* 0x60 */ unsigned char m_LPFOn : 1;
    /* 0x60 */ unsigned char m_InFakePause : 1;
    /* 0x60 */ unsigned char m_TrackOwnsStreams : 1;
    /* 0x64 */ TRACK_STATE m_State;
    /* 0x68 */ Audio::MasterVolume::VOLUME_GROUP m_VolumeGroup;
    /* 0x6C */ Function<FnVoidVoid> m_IdleCallback;
}; // total size: 0x74

inline StreamTrack::StreamTrack(TrackManagerBase& mgr, Audio::MasterVolume::VOLUME_GROUP volumeGroup)
    : m_TrackMgr(mgr)
{
    m_LPFFreq = 32000;
    m_LPFOn = false;
    m_InFakePause = false;
    m_TrackOwnsStreams = true;
    m_State = TS_Idle;
    m_VolumeGroup = volumeGroup;
}

inline TrackManagerBase::TrackManagerBase()
    : m_FileLookup("audio/data/streams/StreamNames.txt", Audio::TrackMgrFileNameParamLookup)
    , m_StreamPool(16, 16)
{
}

inline TrackManagerBase::TrackManagerBase(const Function<bool(const char*, char*, unsigned long)>& fn)
    : m_FileLookup("audio/data/streams/StreamNames.txt", fn)
    , m_StreamPool(16, 16)
{
}

template <int N>
class TrackManager : public TrackManagerBase
{
public:
    TrackManager(const Function<bool(const char*, char*, unsigned long)>&);
    virtual ~TrackManager() { }
    virtual StreamTrack& CreateTrack(const char*, Audio::MasterVolume::VOLUME_GROUP);
    virtual void DestroyAllTracks();
    virtual void OnMasterVolumeChange(Audio::MasterVolume::VOLUME_GROUP);
    virtual StreamTrack* GetTrack(unsigned long Name);
    virtual void StopAllTracks(unsigned long);
    virtual void Update(float);

    /* 0x6C */ nlStaticSortedSlot<StreamTrack, N> m_Tracks;
};

template <int N>
inline TrackManager<N>::TrackManager(const Function<bool(const char*, char*, unsigned long)>& fn)
    : TrackManagerBase(fn)
    , m_Tracks()
{
}

template <int N>
StreamTrack* TrackManager<N>::GetTrack(unsigned long Name)
{
    typedef typename nlSortedSlot<StreamTrack, N>::template EntryLookup<StreamTrack> EntryLookup;

    EntryLookup* result;
    if (m_Tracks.m_EntryCount != 0)
    {
        result = nlBSearch(Name, m_Tracks.m_pEntryLookup, m_Tracks.m_EntryCount);
    }
    else
    {
        result = NULL;
    }
    if (result != NULL)
    {
        return result->pEntry;
    }
    return NULL;
}

template <int N>
void TrackManager<N>::StopAllTracks(unsigned long FadeOut)
{
    for (unsigned long track = 0; track < m_Tracks.m_EntryCount; track++)
    {
        m_Tracks.m_pEntryLookup[track].pEntry->Stop(FadeOut);
    }
}

/**
 * Offset/Address/Size: 0x3E4 | 0x800C6728 | size: 0xF0
 */
template <int N>
void TrackManager<N>::Update(float dT)
{
    int trackOffset;
    unsigned long track;

    m_FadeMgr.Update(dT);

    for (track = 0, trackOffset = 0; track < m_Tracks.m_EntryCount; track++, trackOffset += 8)
    {
        ((typename nlSortedSlot<StreamTrack, N>::template EntryLookup<StreamTrack>*)((char*)m_Tracks.m_pEntryLookup + trackOffset))->pEntry->Update(dT);
    }

    TrackManagerBase::Update(dT);
}

/**
 * Offset/Address/Size: 0x390 | 0x80141E64 | size: 0x1D0
 * TODO: 99.40% match - search-loop zero init and lookup pEntry load registers differ
 */
template <int N>
void TrackManager<N>::DestroyAllTracks()
{
    typedef typename nlSortedSlot<StreamTrack, N>::template EntryLookup<StreamTrack> EL;

    unsigned long i;
    unsigned long trackOffset;
    EL* entryLookup;
    EL* foundSlot;
    StreamTrack* track;

    StopAllTracks(0);

    while (m_Tracks.m_EntryCount != 0)
    {
        track = ((EL*)m_Tracks.m_pEntryLookup)->pEntry;
        if (track == NULL)
            continue;

        if (track)
        {
            track->m_InFakePause = 0;
            track->Stop(0);
            track->~StreamTrack();
        }

        if (track == NULL)
            continue;

        for (i = 0, trackOffset = i; i < m_Tracks.m_EntryCount; trackOffset += 8, i++)
        {
            entryLookup = m_Tracks.m_pEntryLookup;
            if (((EL*)((char*)entryLookup + trackOffset))->pEntry == track)
            {
                foundSlot = (EL*)((char*)entryLookup + i * 8);
                goto found_slot;
            }
        }
        foundSlot = 0;
    found_slot:

        m_Tracks.FreeEntry(track);

        struct LookupShifter
        {
            static void Shift(TrackManager<N>* self, EL* foundSlot)
            {
                unsigned long entryCount = self->m_Tracks.m_EntryCount;
                long idx = foundSlot - self->m_Tracks.m_pEntryLookup;
                while ((unsigned long)idx != entryCount)
                {
                    long next = idx + 1;
                    EL* src = &self->m_Tracks.m_pEntryLookup[next];
                    register unsigned long hash = src->hash;
                    EL* dst = &self->m_Tracks.m_pEntryLookup[idx];
                    idx = next;
                    StreamTrack* pEntry = src->pEntry;
                    dst->pEntry = pEntry;
                    dst->hash = hash;
                }
            }
        };
        LookupShifter::Shift(this, foundSlot);

        m_Tracks.m_EntryCount--;
    }
}

template <>
void TrackManager<3>::DestroyAllTracks();

} // namespace AudioStreamTrack

#endif // _STREAMTRACK_H_
