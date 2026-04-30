#ifndef _FEAUDIO_H_
#define _FEAUDIO_H_

#include "Game/Sys/audio.h"

class Event;
void FEAudioEventHandler(Event*, void*);
// void nlQSort<AnimAudioEventLookup>(AnimAudioEventLookup*, int, int (*)(const AnimAudioEventLookup*, const AnimAudioEventLookup*));
// void nlDefaultQSortComparer<AnimAudioEventLookup>(const AnimAudioEventLookup*, const AnimAudioEventLookup*);
// void nlBSearch<AnimAudioEventLookup, unsigned long>(const unsigned long&, AnimAudioEventLookup*, int);
// void nlListAddEnd<ListEntry<AnimAudioEventLookup>>(ListEntry<AnimAudioEventLookup>**, ListEntry<AnimAudioEventLookup>**,
// ListEntry<AnimAudioEventLookup>*); void nlStrICmp<char>(const char*, const char*); void nlStrCmp<char>(const char*, const char*); void
// nlToUpper<char>(char);

struct AnimAudioEventLookup
{
    /* 0x00 */ unsigned long hash; // eventNameHash
    /* 0x04 */ char szSFXType[50];
}; // total size: 0x38

extern AnimAudioEventLookup* gp_AnimAudioEventTable;
extern unsigned long gNumAnimAudioEvents;

template <typename T, typename U>
T* nlBSearch(const U& key, T* pBase, int count);

template <typename T>
int nlStrCmp(const T* a, const T* b);

class FEAudio
{
public:
    static void EnableSounds(bool);
    static void ResetRandomVoiceToggleSFX();
    void PlayRandomVoiceToggleSFX();
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
