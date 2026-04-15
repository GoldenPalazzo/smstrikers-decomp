#include "Game/Audio/StreamTrack.h"
#include "NL/nlList.h"
#include "NL/nlSlotPoolHigh.h"

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

// /**
//  * Offset/Address/Size: 0x24E4 | 0x8015723C | size: 0x100
//  */
// void AudioStreamTrack::TrackManagerBase::Update(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x1970 | 0x801566C8 | size: 0x3F0
//  */
// void AudioStreamTrack::TrackManagerBase::FadeManager::UpdateFade(AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL*)
// {
// }

// /**
//  * Offset/Address/Size: 0x18FC | 0x80156654 | size: 0x74
//  */
// void AudioStreamTrack::StreamTrack::Update(float)
// {
// }

// /**
//  * Offset/Address/Size: 0x14D4 | 0x8015622C | size: 0x428
//  */
// void AudioStreamTrack::StreamTrack::PlayStream(unsigned long, float, bool, unsigned long, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP)
// {
// }

// /**
//  * Offset/Address/Size: 0x10BC | 0x80155E14 | size: 0x418
//  */
// void AudioStreamTrack::StreamTrack::QueueStream(unsigned long, float, bool, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP)
// {
// }

// /**
//  * Offset/Address/Size: 0xE20 | 0x80155B78 | size: 0x29C
//  */
// void AudioStreamTrack::StreamTrack::ProcessNewHeadStream()
// {
// }

// /**
//  * Offset/Address/Size: 0xC90 | 0x801559E8 | size: 0x190
//  */
// void AudioStreamTrack::StreamTrack::StopHead(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xA28 | 0x80155780 | size: 0x268
//  */
// void AudioStreamTrack::StreamTrack::Stop(unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x888 | 0x801555E0 | size: 0xC4
//  */
// void AudioStreamTrack::StreamTrack::StopQStream(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)
// {
// }

// /**
//  * Offset/Address/Size: 0x5B0 | 0x80155308 | size: 0x2D8
//  */
// void AudioStreamTrack::StreamTrack::StopStream(GCAudioStreaming::StereoAudioStream*, bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x590 | 0x801552E8 | size: 0x20
//  */
// void AudioStreamTrack::StreamTrack::FadeOutDone(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)
// {
// }

// /**
//  * Offset/Address/Size: 0x55C | 0x801552B4 | size: 0x34
//  */
// void AudioStreamTrack::StreamTrack::FadeOutDoneStartNext(AudioStreamTrack::StreamTrack::QUEUED_STREAM*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1E8 | 0x80154F40 | size: 0x374
//  */
// void AudioStreamTrack::StreamTrack::Pause(unsigned long, bool)
// {
// }

// /**
//  * Offset/Address/Size: 0x1B8 | 0x80154F10 | size: 0x30
//  */
// void AudioStreamTrack::StreamTrack::Resume()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80154D58 | size: 0x1B8
//  */
// void AudioStreamTrack::StreamTrack::AttachStream(GCAudioStreaming::StereoAudioStream*, Audio::MasterVolume::VOLUME_GROUP, unsigned long, unsigned long, bool, bool)
// {
// }
