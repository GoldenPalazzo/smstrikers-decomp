#ifndef _PRESENTATION_H_
#define _PRESENTATION_H_

#include "types.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/InterpreterCore.h"
#include "Game/Sys/eventman.h"
#include "Game/OverlayManager.h"

void CupWinStingerDone();
void ReadTrophyModel(void*, unsigned long, void*);
void ReadTrophyTexture(void*, unsigned long, void*);
// void 0x801267BC..0x801272A0 | size: 0xAE4;
// void LexicalCast<const char*, float>(const float&);
// void LexicalCast<const char*, int>(const int&);
// void LexicalCast<const char*, bool>(const bool&);
// void LexicalCast<const char*, const char*>(const char* const&);

enum HighlightQuality
{
    HIGHLIGHT_QUALITY_EMPTY = 0,
    HIGHLIGHT_QUALITY_SAVE = 1,
    HIGHLIGHT_QUALITY_GOAL_DECREASE_DIFF = 2,
    HIGHLIGHT_QUALITY_GOAL_EQUALIZER = 3,
    HIGHLIGHT_QUALITY_GOAL_INCREASE_DIFF = 4,
    NUM_QUALITY_LEVELS = 5,
};

class Presentation : public InterpreterCore
{
public:
    Presentation();
    virtual ~Presentation() { };
    virtual void DoFunctionCall(unsigned int);

    static Presentation& Instance();

    void LoadTrophyModel();
    void Finish();
    void Update(float);
    bool DuringEndOfGamePresentation() const;
    void Call(const char*, const char*);
    void EventHandler(Event*);
    static void EventHandler(Event* event, void* userData) { ((Presentation*)userData)->EventHandler(event); }
    void PlayOverlay(const char*, float, float);
    void StopOverlay();
    void Reset();

    /* 0x24 */ char mCurrentFunction[64];
    /* 0x64 */ bool mByPassWasSkipped;
    /* 0x65 */ bool mSkipPressed;
    /* 0x66 */ bool mInsideByPass;
    /* 0x67 */ bool mByPassing;
    /* 0x68 */ float mWaitingForCharacterDirectionSince;
    /* 0x6C */ float mTimeInFunction;
    /* 0x70 */ bool mTrophyTextureLoaded;
    /* 0x74 */ float mDisplayLetterBox;
    /* 0x78 */ float mLetterBoxDuration;
    /* 0x7C */ bool mLetterBoxEnabled;
    /* 0x80 */ float mOverlayDelay;
    /* 0x84 */ float mOverlayDisplayLength;
    /* 0x88 */ bool mOverlayDisplayed;
    /* 0x8C */ SceneList mOverlayToDisplay;
    /* 0x90 */ const char* mInterruptWipe;
    /* 0x94 */ const char* mUseInterruptWipe;
    /* 0x98 */ const char* mQueuedFunction;
    /* 0x9C */ bool mIsAllowedToSkip[4];
    /* 0xA0 */ HighlightQuality mGoalQuality;
}; // total size: 0xA4

// class NISData
// {
// public:
//     void GetID();
// };

#endif // _PRESENTATION_H_
