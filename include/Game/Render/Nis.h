#ifndef _NIS_H_
#define _NIS_H_

#include "NL/nlMath.h"
#include "Game/SAnim/pnSAnimController.h"

// #include "Game/Camera/animcam.h"
class cAnimCamera;
class SFXEmitter;

// void Bind<void, void (*)(EmissionController&, int), Placeholder<0>, int>(void (*)(EmissionController&, int), const Placeholder<0>&, const int&);

enum NisTriggerType
{
    NIS_TRIGGER_TYPE_EFFECT = 0,
    NIS_TRIGGER_TYPE_TIME_DILATION = 1,
    NIS_TRIGGER_TYPE_PLAY_SOUND = 2,
    NIS_TRIGGER_TYPE_PLAY_RANDOM_DIALOGUE = 3,
    NIS_TRIGGER_TYPE_STOP_SOUND = 4,
    NIS_TRIGGER_TYPE_PLAY_STREAM = 5,
    NIS_TRIGGER_TYPE_STOP_STREAM = 6,
    NIS_TRIGGER_TYPE_STOP_ALL_STREAMS = 7,
    NIS_TRIGGER_TYPE_SET_ACTIVE_STREAM_LOOPING = 8,
    NIS_TRIGGER_TYPE_REGISTER_GOAL_AUDIO = 9,
    NIS_TRIGGER_TYPE_RAISE_EVENT = 10,
};

enum NisCharacterClass
{
    NIS_CHAR_CLASS_INVALID = -1,
    NIS_CHAR_CLASS_BIRDO = 0,
    NIS_CHAR_CLASS_DAISY = 1,
    NIS_CHAR_CLASS_DONKEYKONG = 2,
    NIS_CHAR_CLASS_HAMMERBROS = 3,
    NIS_CHAR_CLASS_KOOPA = 4,
    NIS_CHAR_CLASS_LUIGI = 5,
    NIS_CHAR_CLASS_MARIO = 6,
    NIS_CHAR_CLASS_PEACH = 7,
    NIS_CHAR_CLASS_TOAD = 8,
    NIS_CHAR_CLASS_WALUIGI = 9,
    NIS_CHAR_CLASS_WARIO = 10,
    NIS_CHAR_CLASS_YOSHI = 11,
    NIS_CHAR_CLASS_MYSTERY = 12,
    NIS_CHAR_CLASS_HOME_GOALIE = 13,
    NIS_CHAR_CLASS_AWAY_GOALIE = 14,
    NIS_CHAR_CLASS_NUM_CLASSES = 15,
};

enum NisTarget
{
    NIS_TARGET_NONE = 0,
    NIS_TARGET_STADIUM = 1,
    NIS_TARGET_HOME_CAPTAIN = 2,
    NIS_TARGET_AWAY_CAPTAIN = 3,
    NIS_TARGET_HOME_SIDEKICK = 4,
    NIS_TARGET_AWAY_SIDEKICK = 5,
    NIS_TARGET_HOME_GOALIE = 6,
    NIS_TARGET_AWAY_GOALIE = 7,
    NIS_TARGET_LOSER_CAPTAIN = 8,
    NIS_TARGET_WINNER_CAPTAIN = 9,
    NIS_TARGET_LOSER_SIDEKICK = 10,
    NIS_TARGET_WINNER_SIDEKICK = 11,
    NIS_TARGET_LOSER_GOALIE = 12,
    NIS_TARGET_WINNER_GOALIE = 13,
    NIS_NUM_TARGETS = 14,
};

enum NisWinnerType
{
    NIS_GAME_WINNER = 0,
    NIS_GOAL_WINNER = 1,
    NIS_NUM_WINNER_TYPES = 2,
    NIS_DO_NOT_CARE = 3,
};

struct NisHeader
{
    /* 0x00 */ char name[64];
    /* 0x40 */ int size;
    /* 0x44 */ int numBalls;
    /* 0x48 */ int numAnimations;
    /* 0x4C */ int numCameras;
    /* 0x50 */ nlVector3 center;
    /* 0x5C */ nlVector3 minBounds;
    /* 0x68 */ nlVector3 maxBounds;
    /* 0x74 */ nlVector3 beginPositions[4];
    /* 0xA4 */ NisTarget target;
    /* 0xA8 */ NisWinnerType winnerType;
    /* 0xAC */ nlVector3 stadiumOffset;
    /* 0xB8 */ float mTime;
}; // total size: 0xBC

enum NisAudioType
{
    NIS_AUDIO_TYPE_NONE = 0,
    NIS_AUDIO_TYPE_STREAM = 1,
    NIS_AUDIO_TYPE_SFX = 2,
};

class Nis
{
public:
    struct NisAudioData
    {
        /* 0x00 */ NisAudioType audioType;
        union
        {
            SFXEmitter* pEmitter;
            unsigned long index;
        }
        /* 0x04 */ identifier;
        /* 0x08 */ unsigned long soundType;
        /* 0x0C */ char str[128];
        /* 0x8C */ unsigned char isEmitter;
        /* 0x8D */ unsigned char stopAtNisEnd;
        /* 0x8E */ unsigned char pad[2];
        /* 0x90 */ NisAudioData* next;
    }; // total size: 0x94

    struct TriggerParams
    {
        /* 0x00 */ float float1;
        /* 0x04 */ unsigned long param1;
        /* 0x08 */ unsigned long param2;
        /* 0x0C */ unsigned long param3;
        /* 0x10 */ unsigned long param4;
    }; // total size: 0x14

    struct Trigger
    {
        /* 0x00 */ NisTriggerType type;
        /* 0x04 */ float frameNumber;
        /* 0x08 */ const char* name;
        /* 0x0C */ const char* target;
        /* 0x10 */ TriggerParams params;

        void FireEffect(const Nis&) const;
        void Fire(Nis&) const;
    }; // total size: 0x24

    Nis(NisHeader&, char*, int);
    ~Nis();
    char* Name() const;
    void Update(float dt);
    void UpdateTriggers(float, float, float);
    void SelectCamera(cAnimCamera&, int);
    bool SelectRandomCamera(cAnimCamera&);
    void Render();
    nlVector3 Offset() const;
    void AddTrigger(NisTriggerType, float, const char*, const char*, Nis::TriggerParams*);
    void StopAllOutstandingNisAudio();

    /* 0x000 */ NisHeader* mHeader;                             // offset 0x0, size 0x4
    /* 0x004 */ NisTarget mTarget;                              // offset 0x4, size 0x4
    /* 0x008 */ NisWinnerType mWinnerType;                      // offset 0x8, size 0x4
    /* 0x00C */ char* mData;                                    // offset 0xC, size 0x4
    /* 0x010 */ int mSize;                                      // offset 0x10, size 0x4
    /* 0x014 */ int mBallId[10];                                // offset 0x14, size 0x28
    /* 0x03C */ cPN_SAnimController* mCharacterControllers[10]; // offset 0x3C, size 0x28
    /* 0x064 */ bool mMirrored;                                 // offset 0x64, size 0x1
    /* 0x068 */ cAnimCamera* mCamera;                           // offset 0x68, size 0x4
    /* 0x06C */ int mNumCameras;                                // offset 0x6C, size 0x4
    /* 0x070 */ int mNumTriggers;                               // offset 0x70, size 0x4
    /* 0x074 */ Trigger mTriggers[48];                          // offset 0x74, size 0x6C0
    /* 0x734 */ int mMainCharacterIndex;                        // offset 0x734, size 0x4
    /* 0x738 */ int mUnk_0x738;                                 // offset 0x738, size 0x4
    /* 0x73C */ NisAudioData* mNisAudioDataList;                // offset 0x73C, size 0x4
}; // total size: 0x740

// class Function1<void, EmissionController&>
// {
// public:
//     void FunctorImpl<BindExp2<void, void (*)(EmissionController&, int), Placeholder<0>, int>>::~FunctorImpl();
//     void FunctorImpl<BindExp2<void, void (*)(EmissionController&, int), Placeholder<0>, int>>::operator()(EmissionController&);
//     void FunctorImpl<BindExp2<void, void (*)(EmissionController&, int), Placeholder<0>, int>>::Clone() const;
// };

// class nlDeleteList<Nis
// {
// public:
//     void NisAudioData>(Nis::NisAudioData**);
// };

// class nlListRemoveElement<Nis
// {
// public:
//     void NisAudioData>(Nis::NisAudioData**, Nis::NisAudioData*, Nis::NisAudioData**);
// };

// class nlListAddStart<Nis
// {
// public:
//     void NisAudioData>(Nis::NisAudioData**, Nis::NisAudioData*, Nis::NisAudioData**);
// };

// class Format<BasicString<char, Detail
// {
// public:
//     void TempStringAllocator>, char[64], int>(const BasicString<char, Detail::TempStringAllocator>&, const char(&)[64], const int&);
// };

// class FormatImpl<BasicString<char, Detail
// {
// public:
//     void TempStringAllocator>>::operator%<const char*>(const char* const&);
//     void TempStringAllocator>>::operator%<int>(const int&);
// };

#endif // _NIS_H_
