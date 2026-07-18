#ifndef _AUDIOSTREAMAPI_H_
#define _AUDIOSTREAMAPI_H_

class PriorityStream;

namespace Audio
{

void InitStreaming();
void StopStreaming();
bool TrackMgrFileNameParamLookup(const char*, char*, unsigned long);
void DestroyTrackMgr();
void CreatePriorityStreams();
void DestroyPriorityStreams();
PriorityStream* GetPriorityStream();
void ConfigureStreamBuffers(unsigned long);

} // namespace Audio

#endif // _AUDIOSTREAMAPI_H_
