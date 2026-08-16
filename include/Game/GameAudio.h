#ifndef _GAMEAUDIO_H_
#define _GAMEAUDIO_H_

#include "types.h"
#include "Game/Sys/audio.h"
#include "NL/plat/plataudio.h"

#include "NL/nlMath.h"
#include "NL/nlDLRing.h"
#include "NL/nlDLListContainer.h"
#include "NL/nlSlotPool.h"

enum eClassType
{
    GAME = 0,
    CHAR = 1,
    WORLD = 2,
};

// Forward declarations
namespace Audio
{
struct SoundAttributes;
}

// void TrackedSFXPriorityCallback(SFXPlaySet*, unsigned long, cGameSFX*);
// void TrackedSFXFilterFreqTypeCheckCallback(unsigned long, cGameSFX*);
// void TrackedSFXPitchFreqTypeCheckCallback(unsigned long, cGameSFX*);
// void nlDLRingIsEnd<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>*, DLListEntry<SFXPlaySet*>*);
// void nlDLRingGetStart<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>*);
// void nlDLRingRemove<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>**, DLListEntry<SFXPlaySet*>*);
// void nlDLRingAddStart<DLListEntry<SFXPlaySet*>>(DLListEntry<SFXPlaySet*>**, DLListEntry<SFXPlaySet*>*);

struct SFXPlaySet
{
    /* 0x00 */ unsigned long type;
    /* 0x04 */ unsigned long voiceID;
    /* 0x08 */ unsigned char bIs3D;
    /* 0x0C */ SFXEmitter* emitter;
    /* 0x10 */ float delay;
    /* 0x14 */ float timeStamp;
    /* 0x18 */ int sfxPriority;
    /* 0x1C */ int groupPriority;
    /* 0x20 */ unsigned short filterFreq;
    /* 0x22 */ unsigned short pitch;

    static SlotPool<SFXPlaySet> m_TrackedSFXSlotPool;
}; // total size: 0x24

class SoundPropAccessor
{
public:
    ~SoundPropAccessor();

    virtual SoundProperties* GetSoundProperty(unsigned int index) const;
    virtual SoundProperties* GetSoundPropTable();
    virtual int GetNumSFX();
    virtual char* GetSoundPropTableName();
    virtual char* GetHTMLFileName();
    virtual unsigned char IsUsingOrigTable() const;
    virtual void SetSoundPropTable(SoundProperties* pNewTable);
    virtual void ResetSoundPropTable();

    /* 0x4 */ SoundProperties* mpSoundProp;
    /* 0x8 */ unsigned char mbIsReloaded;
}; // total size: 0xC

class cGameSFX
{
public:
    enum StopFlag
    {
        SFX_STOP_ALL = 0,
        SFX_STOP_FIRST = 1,
        SFX_STOP_OLDEST = 2,
    };

    cGameSFX();
    virtual ~cGameSFX();

    virtual void Init();
    virtual void DeInit();
    void SetupPlaySet();
    void ShutdownPlaySet();
    virtual void SetSFX(SoundPropAccessor*);
    void CheckTypeMap(SoundPropAccessor*) const;
    void ResetSFX();
    float GetSFXVol(const Audio::SoundAttributes&) const;
    float GetSFXVol(unsigned long) const;
    float GetSFXVolReverb(const Audio::SoundAttributes&) const;
    float GetSFXVolReverb(unsigned long) const;
    int GetVolGroup(unsigned long) const;
    int GetSFXPriority(unsigned long) const;
    bool IsKeepingTrackOf(unsigned long, SFXPlaySet**);
    bool InitiateCallbackOnAllTrackedSFX(
        bool (*)(SFXPlaySet*, unsigned long, cGameSFX*),
        unsigned long,
        bool (*)(unsigned long, cGameSFX*));
    bool ActivateFilterOnAllTrackedSFX(bool);
    bool SetFilterFreqOnAllTrackedSFX(unsigned short);
    bool SetPitchBendOnAllDialogueSFX(unsigned short);
    bool CheckForHigherPrioritySFX(int);
    bool KillLowerPrioritySFX(int);
    virtual unsigned long Play(Audio::SoundAttributes&);
    virtual eClassType GetClassType() const { return meClassType; }
    SFXPlaySet* KeepTrack(SFXEmitter*, const Audio::SoundAttributes&, unsigned long);
    void Stop(unsigned long, cGameSFX::StopFlag);
    void StopEmitter(SFXEmitter*, unsigned long);
    bool StopTrackedSFX(SFXPlaySet*);
    bool StopTrackedSFX(nlDLListIterator<SFXPlaySet*>*);
    void StopPlayingAllTrackedSFX();
    void UpdateGroupFilterStatusOnSFX(SFXPlaySet*);
    void UpdateGroupPitchStatusOnSFX(SFXPlaySet*);
    void UpdateAllTrackedSFX(float);
    SFXPlaySet* RemoveTrackedSFX(unsigned long);
    SFXPlaySet* RemoveTrackedSFX(nlDLListIterator<SFXPlaySet*>*);
    unsigned long GetSFXID(unsigned long) const;
    void SetSFXInfo(unsigned long, unsigned long, float, float, int, int);
    SoundPropAccessor* GetSoundPropAccessor(unsigned long);

    /* 0x04 */ bool mbInited;
    /* 0x08 */ unsigned long mNumSFX;
    /* 0x0C */ unsigned long mNumSFXTypes;
    /* 0x10 */ SoundStrToIDNode* mpSFX;
    /* 0x14 */ nlDLListContainer<SFXPlaySet*> mpCurPlaySet;
    /* 0x1C */ bool mbCurPlaySetIsValid;
    /* 0x20 */ float mfTrackedSFXCheckInterval;
    /* 0x24 */ const char** mpSoundStrTable;
    /* 0x28 */ eClassType meClassType;
    /* 0x2C */ bool mbGroupFilterOn;
    /* 0x2E */ unsigned short muGroupFilterFreq;
    /* 0x30 */ unsigned short muGroupPitch;
}; // total size: 0x34

// class DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<SFXPlaySet*>*);
// };

// class nlWalkDLRing<DLListEntry<SFXPlaySet*>, DLListContainerBase<SFXPlaySet*,
// NewAdapter<DLListEntry<SFXPlaySet*>>>>(DLListEntry<SFXPlaySet*>*, DLListContainerBase<SFXPlaySet*,
// NewAdapter<DLListEntry<SFXPlaySet*>>>*, void (DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*>>>
// {
// public:
//     void *)(DLListEntry<SFXPlaySet*>*));
// };

// class nlWalkRing<DLListEntry<SFXPlaySet*>, DLListContainerBase<SFXPlaySet*,
// NewAdapter<DLListEntry<SFXPlaySet*>>>>(DLListEntry<SFXPlaySet*>*, DLListContainerBase<SFXPlaySet*,
// NewAdapter<DLListEntry<SFXPlaySet*>>>*, void (DLListContainerBase<SFXPlaySet*, NewAdapter<DLListEntry<SFXPlaySet*>>>
// {
// public:
//     void *)(DLListEntry<SFXPlaySet*>*));
// };

// class SlotPool<SFXPlaySet>
// {
// public:
//     void ~SlotPool();
// };

#endif // _GAMEAUDIO_H_
