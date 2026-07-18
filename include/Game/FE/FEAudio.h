#ifndef _FEAUDIO_H_
#define _FEAUDIO_H_

#include "Game/Sys/audio.h"

class Event;
void FEAudioEventHandler(Event*, void*);

struct AnimAudioEventLookup
{
    /* 0x00 */ unsigned long eventNameHash;
    /* 0x04 */ char szSFXType[50];

    operator unsigned long() const { return eventNameHash; }
}; // total size: 0x38

extern AnimAudioEventLookup* gp_AnimAudioEventTable;
extern unsigned long gNumAnimAudioEvents;

class FEAudio
{
public:
    static void EnableSounds(bool);
    static void ResetRandomVoiceToggleSFX();
    static void PlayRandomVoiceToggleSFX();
    static long PlayAnimAudioEvent(unsigned long, bool);
    static void StopAnimAudioEvent(const char*);
    static long PlayAnimAudioEvent(const char*, bool);
    static void BuildAnimAudioEventLookup();
};

// class ListContainerBase<AnimAudioEventLookup, BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>
// {
// public:
//     void DeleteEntry(ListEntry<AnimAudioEventLookup>*);
// };

// class BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>
// {
// public:
//     void freeFN(void*);
//     void allocFN(unsigned long);
// };

// class nlWalkList<ListEntry<AnimAudioEventLookup>, ListContainerBase<AnimAudioEventLookup,
// BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>>(ListEntry<AnimAudioEventLookup>*, ListContainerBase<AnimAudioEventLookup,
// BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>*, void (ListContainerBase<AnimAudioEventLookup,
// BasicSlotPoolHigh<ListEntry<AnimAudioEventLookup>>>
// {
// public:
//     void *)(ListEntry<AnimAudioEventLookup>*));
// };

#endif // _FEAUDIO_H_
