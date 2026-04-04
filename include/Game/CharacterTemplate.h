#ifndef _CHARACTERTEMPLATE_H_
#define _CHARACTERTEMPLATE_H_

#include "Game/Character.h"
#include "Game/Inventory.h"
#include "Game/SHierarchy.h"

class cAnimInventory;
class CharacterPhysicsData;
class AnimRetargetList;

struct tCharacterTemplate
{
    /* 0x00 */ int nCharacterModelID[2];
    /* 0x08 */ unsigned long uAnimInventoryHashID;
    /* 0x0C */ unsigned char bAnimInventoryCopy;
    /* 0x0D */ char pad_0xD[3];
    /* 0x10 */ cInventory<cSHierarchy>* pHierarchyInventory;
    /* 0x14 */ cAnimInventory* pAnimInventory;
    /* 0x18 */ const CharacterPhysicsData* pPhysicsData;
    /* 0x1C */ cInventory<AnimRetargetList>* pAnimRetargetListInventory;
}; // total size: 0x20

struct tGoalieTemplateInfo
{
    const char* szCharName;        // offset 0x0, size 0x4
    const char* szTextureFilename; // offset 0x4, size 0x4
    unsigned char bLoaded;         // offset 0x8, size 0x1
}; // total size: 0xC

struct tCharacterTemplateInfo
{
    const char* szCharName;                       // offset 0x0, size 0x4
    eClassTypes classType;                        // offset 0x4, size 0x4
    const char* szModelFilename;                  // offset 0x8, size 0x4
    const char* szBlendedModelFilename;           // offset 0xC, size 0x4
    const char* szTextureFilename;                // offset 0x10, size 0x4
    const char* szTriggerFilename;                // offset 0x14, size 0x4
    void (*pTriggerCallback)(unsigned int);       // offset 0x18, size 0x4
    const char* szHierarchyFilename;              // offset 0x1C, size 0x4
    const char* szHierarchy;                      // offset 0x20, size 0x4
    const struct AnimProperties* pAnimProperties; // offset 0x24, size 0x4
    int nNumAnimProperties;                       // offset 0x28, size 0x4
    const char* szAnimFilename;                   // offset 0x2C, size 0x4
    const char* szEffectsName;                    // offset 0x30, size 0x4
    SoundPropAccessor* pSFXPropAccessor;          // offset 0x34, size 0x4
    const char* szPhysicsFilename;                // offset 0x38, size 0x4
    const char* szTweaksFilename;                 // offset 0x3C, size 0x4
    const char* szAnimRetargetFilename;           // offset 0x40, size 0x4
}; // total size: 0x44

class tCharacterTemplateInfo;

s32 GetGoalieIndex(int);
s32 GetCharacterIndex(const cCharacter*);
void DestroyCharacters();
void CreateCharacters();
cPlayer* CreateGoalie(eCharacterClass, bool);
cPlayer* CreateSidekick(int, int, eCharacterClass, eCharacterClass, bool);
s32 SidekickTexture_cb(unsigned long);
cPlayer* CreateCharacter(int, int, eCharacterClass, bool);
void CharacterLoadingGuts(tCharacterTemplate*, const tCharacterTemplateInfo&, eCharacterClass, bool);
bool IsCaptain(eCharacterClass);
char* GetCharacterName(eCharacterClass cc);

extern cCharacter* g_pCharacters[10];

// void LexicalCast<bool, const char*>(const char* const&);
// void LexicalCast<bool, float>(const float&);
// void LexicalCast<bool, int>(const int&);
// void LexicalCast<bool, bool>(const bool&);
// void 0x800148D0..0x800149F0 | size: 0x120;
// void nlListAddStart<ListEntry<AnimRetargetList*>>(ListEntry<AnimRetargetList*>**, ListEntry<AnimRetargetList*>*, ListEntry<AnimRetargetList*>**);
// void nlListAddStart<ListEntry<cSHierarchy*>>(ListEntry<cSHierarchy*>**, ListEntry<cSHierarchy*>*, ListEntry<cSHierarchy*>**);

// class DLListContainerBase<GCAudioStreaming
// {
// public:
//     void StereoAudioStream*, BasicSlotPool<DLListEntry<GCAudioStreaming::StereoAudioStream*>>>::DeleteEntry(DLListEntry<GCAudioStreaming::StereoAudioStream*>*);
// };

// class DLListContainerBase<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::FadeManager::STREAM_FADE_CTRL, BasicSlotPool<DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>>>::DeleteEntry(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*);
//     void StreamTrack::QUEUED_STREAM, nlStaticArrayAllocator<DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>, 4>>::DeleteEntry(DLListEntry<AudioStreamTrack::StreamTrack::QUEUED_STREAM>*);
// };

// class WalkHelper<AudioStreamTrack
// {
// public:
//     void TrackManagerBase::FadeManager::STREAM_FADE_CTRL, DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>, AudioStreamTrack::TrackManagerBase::FadeManager>::Callback(DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL>*);
// };

// class ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>
// {
// public:
//     void DeleteEntry(ListEntry<cSHierarchy*>*);
// };

// class ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>
// {
// public:
//     void DeleteEntry(ListEntry<AnimRetargetList*>*);
// };

// class Function0<void>
// {
// public:
//     void FunctorBase::~FunctorBase();
// };

// class CharacterPhysicsData
// {
// public:
//     ~CharacterPhysicsData();
// };

// class SebringAnimTagScriptInterpreter
// {
// public:
//     ~SebringAnimTagScriptInterpreter();
// };

// class AnimTagScriptInterpreter
// {
// public:
//     ~AnimTagScriptInterpreter();
//     void TriggerFired(unsigned long);
// };

// class nlWalkList<ListEntry<AnimRetargetList*>, ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>>(ListEntry<AnimRetargetList*>*, ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>*, void (ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*>>>
// {
// public:
//     void *)(ListEntry<AnimRetargetList*>*));
// };

// class nlWalkList<ListEntry<cSHierarchy*>, ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>>(ListEntry<cSHierarchy*>*, ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>*, void (ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*>>>
// {
// public:
//     void *)(ListEntry<cSHierarchy*>*));
// };

#endif // _CHARACTERTEMPLATE_H_
