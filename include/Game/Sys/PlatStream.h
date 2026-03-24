#ifndef _PLATSTREAM_H_
#define _PLATSTREAM_H_

namespace PlatAudio
{

static void InitStreaming();
static void ShutdownStreaming();
void ConfigureStreamBuffers(unsigned long);
static bool IsStreamingInited();
static void StopAllStreams();

}; // namespace PlatAudio

// class GCAudioStreaming
// {
// public:
//     void AudioStreamBuffer::AudioStreamBuffer();
// };

// class nlStaticSortedSlot<GCAudioStreaming
// {
// public:
//     void AudioStream*, 7>::GetNewEntry();
//     void AudioStream*, 7>::FreeEntry(GCAudioStreaming::AudioStream**);
//     void AudioStream*, 7>::FreeLookup();
//     void AudioStream*, 7>::ExpandLookup();
// };

#endif // _PLATSTREAM_H_
