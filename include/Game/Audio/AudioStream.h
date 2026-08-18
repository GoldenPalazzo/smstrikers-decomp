#ifndef _AUDIOSTREAM_H_
#define _AUDIOSTREAM_H_

#include "Game/Sys/GCStream.h"
#include "Game/Audio/AudioStreamAPI.h"
#include "Game/Audio/AudioStreamVirtuals.h"
#include "Game/Audio/PriorityStream.h"

namespace Audio
{

/**
 * Offset/Address/Size: 0x0 | 0x80141518 | size: 0x1AC
 * TODO: 99.35% match - vtable and free-list setup use different temporary registers.
 */
template <int N>
void CreateTrackMgr()
{
    g_pTrackManager = new (8, false) AudioStreamTrack::TrackManager<N>(TrackMgrFileNameParamLookup);
}

}; // namespace Audio

// class Function0<void>
// {
// public:
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*>>::~FunctorImpl();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*>>::operator()();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*>>::Clone() const;
// };

// class PriorityStream
// {
// public:
//     void PriorityStream(AudioStreamTrack::StreamTrack&);
// };

// class Bind<void, Detail
// {
// public:
//     void MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream*>(Detail::MemFunImpl<void, void (PriorityStream::*)()>, PriorityStream* const&);
// };

// class MemFun<PriorityStream, void>(void (PriorityStream
// {
// public:
//     void *)());
// };

#endif // _AUDIOSTREAM_H_
