#ifndef _STREAMTRACK_H_
#define _STREAMTRACK_H_

#include "NL/nlFunction.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlSlotPool.h"

namespace GCAudioStreaming
{
class StereoAudioStream;
}

namespace Audio
{
namespace MasterVolume
{
enum VOLUME_GROUP
{
    VG_Special = 0,
    VG_Music = 1,
    VG_SFX = 2,
    VG_Voice = 3,
};
}
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
    virtual ~TrackManagerBase();
    /* 0x0C */ virtual void Update(float);
    /* 0x10 */ virtual StreamTrack& CreateTrack(const char*, Audio::MasterVolume::VOLUME_GROUP);
    /* 0x14 */ virtual void DestroyAllTracks();
    /* 0x18 */ virtual StreamTrack* GetTrack(unsigned long);
    /* 0x1C */ virtual void StopAllTracks(unsigned long);
    /* 0x20 */ virtual void OnMasterVolumeChange(Audio::MasterVolume::VOLUME_GROUP);

    class FadeManager
    {
    public:
        struct STREAM_FADE_CTRL
        {
            char data[0x14];
        };
        void UpdateFade(STREAM_FADE_CTRL*);
    };

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
            /* 0x00 */ unsigned long key;
            /* 0x04 */ char* value;
        }; // total size: 0x8

        StreamFileLookup(const char* name,
            const Function<bool(const char*, char*, unsigned long)>& fn);
    }; // total size: 0x14

    typedef DLListEntry<GCAudioStreaming::StereoAudioStream*> StreamDeleteEntry;
    typedef BasicSlotPool<StreamDeleteEntry> StreamDeleteAllocator;
    typedef DLListContainerBase<GCAudioStreaming::StereoAudioStream*, StreamDeleteAllocator> StreamDeleteList;

    /* 0x04 */ char _pad_0x04[0x14]; // StreamFileLookup m_FileLookup
    /* 0x18 */ char _pad_0x18[0x20]; // FadeManager m_FadeMgr
    /* 0x38 */ SlotPool<GCAudioStreaming::StereoAudioStream> m_StreamPool;
    /* 0x50 */ StreamDeleteList m_StreamDeleteList;
}; // total size: 0x6C

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

    void Update(float);
    void PlayStream(unsigned long, float, bool, unsigned long, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP);
    void QueueStream(unsigned long, float, bool, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP);
    void ProcessNewHeadStream();
    void StopHead(unsigned long);
    void Stop(unsigned long);
    void StopQStream(QUEUED_STREAM*);
    // void StopStream(GCAudioStreaming::StereoAudioStream*, bool);
    void FadeOutDone(QUEUED_STREAM*);
    void FadeOutDoneStartNext(QUEUED_STREAM*);
    void StartQStreamFadeout(QUEUED_STREAM*, unsigned long, const Function<FnVoidVoid>&);
    void Pause(unsigned long, bool);
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

} // namespace AudioStreamTrack

// class Function0<void>
// {
// public:
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*>>::~FunctorImpl();
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*>>::operator()();
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*>>::Clone() const;
// };

// class DLListContainerBase<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::FadeManager::STREAM_FADE_CTRL, BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>>::AllocateAtEnd(unsigned long*);
// };

// class ListContainerBase<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>>>::DeleteEntry(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*);
// };

// class BasicSlotPoolHigh<ListEntry<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>>::allocFN(unsigned long);
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>>::freeFN(void*);
// };

// class nlQSort<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP>(AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, int, int (*)(const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*));
// };

// class nlDefaultQSortComparer<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP>(const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, const AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*);
// };

// class nlBSearch<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP, unsigned long>(const unsigned long&, AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LOOKUP*, int);
// };

// class nlWalkList<ListEntry<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>, ListContainerBase<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>>>>(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*, ListContainerBase<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>>>*, void (ListContainerBase<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP, BasicSlotPoolHigh<ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>>>::*)(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*));
// };

// class nlListAddEnd<ListEntry<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>>(ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>**, ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>**, ListEntry<AudioStreamTrack::TrackManagerBase::StreamFileLookup::STREAM_FILE_LIST_LOOKUP>*);
// };

// class nlDLRingRemoveEnd<DLListEntry<AudioStreamTrack
// {
// public:
//     void StreamTrack::QUEUED_STREAM>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**);
// };

// class nlDLRingRemoveStart<DLListEntry<AudioStreamTrack
// {
// public:
//     void StreamTrack::QUEUED_STREAM>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**);
// };

// class nlDLRingIsEnd<DLListEntry<AudioStreamTrack
// {
// public:
//     void StreamTrack::QUEUED_STREAM>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*);
// };

// class nlDLRingRemove<DLListEntry<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>**, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*);
//     void StreamTrack::QUEUED_STREAM>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*);
// };

// class nlDLRingAddEnd<DLListEntry<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>**, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*);
//     void StreamTrack::QUEUED_STREAM>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*);
// };

// class nlDLRingAddEnd<DLListEntry<GCAudioStreaming
// {
// public:
//     void StereoAudioStream*>>(DLListEntry<GCAudioStreaming::StereoAudioStream*>**, DLListEntry<GCAudioStreaming::StereoAudioStream*>*);
// };

// class nlDLRingAddStart<DLListEntry<GCAudioStreaming
// {
// public:
//     void StereoAudioStream*>>(DLListEntry<GCAudioStreaming::StereoAudioStream*>**, DLListEntry<GCAudioStreaming::StereoAudioStream*>*);
// };

// class nlDLRingAddStart<DLListEntry<AudioStreamTrack
// {
// public:
//     void StreamTrack::QUEUED_STREAM>>(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>**, DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*);
//     void TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>**, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*);
// };

// class Bind<void, Detail
// {
// public:
//     void MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack*, AudioStreamTrack::StreamTrack::QUEUED_STREAM*>(Detail::MemFunImpl<void, void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)>, AudioStreamTrack::StreamTrack* const&, AudioStreamTrack::StreamTrack::QUEUED_STREAM* const&);
// };

// class MemFun<AudioStreamTrack
// {
// public:
//     void StreamTrack, void, AudioStreamTrack::StreamTrack::QUEUED_STREAM*>(void (AudioStreamTrack::StreamTrack::*)(AudioStreamTrack::StreamTrack::QUEUED_STREAM*));
// };

#endif // _STREAMTRACK_H_
