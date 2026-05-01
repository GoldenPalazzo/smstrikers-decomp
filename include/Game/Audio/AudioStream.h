#ifndef _AUDIOSTREAM_H_
#define _AUDIOSTREAM_H_

#include "Game/Audio/PriorityStream.h"

namespace Audio
{

void InitStreaming();
void StopStreaming();
bool TrackMgrFileNameParamLookup(const char*, char*, unsigned long);
void CreatePriorityStreams();
void DestroyPriorityStreams();
PriorityStream* GetPriorityStream();
void ConfigureStreamBuffers(unsigned long);

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
