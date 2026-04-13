#ifndef _REPLAYCHOREO_H_
#define _REPLAYCHOREO_H_

#include "types.h"
#include "Game/InterpreterCore.h"
#include "Game/Replay.h"
#include "Game/ReplayManager.h"
#include "Game/Camera/ReplayCamera.h"
#include "Game/Goalie.h"
#include "Game/Sys/eventman.h"

class ReplayChoreo : public InterpreterCore
{
public:
    enum HighlightQuality
    {
        HIGHLIGHT_QUALITY_EMPTY = 0,
        HIGHLIGHT_QUALITY_SAVE = 1,
        HIGHLIGHT_QUALITY_GOAL_DECREASE_DIFF = 2,
        HIGHLIGHT_QUALITY_GOAL_EQUALIZER = 3,
        HIGHLIGHT_QUALITY_GOAL_INCREASE_DIFF = 4,
        NUM_QUALITY_LEVELS = 5,
    };

    struct Highlight
    {
        Highlight()
            : mSideOfInterest(0)
            , mTime(0.0f)
            , mReplayPad(-1)
        {
        }

        int mSideOfInterest;            // offset 0x0, size 0x4
        float mTime;                    // offset 0x4, size 0x4
        int mReplayPad;                 // offset 0x8, size 0x4
        GoalScoredData mGoalScoredData; // offset 0xC, size 0x24
        u8 pad[0x4];                    // padding to 0x34
    }; // total size: 0x34

    /**
     * Offset/Address/Size: 0xFDC | 0x80128648 | size: 0xE8
     */
    ReplayChoreo()
        : InterpreterCore(10)
        , mReplayManager(NULL)
        , mRunForTimeLeft(0.0f)
        , mRunningFor(false)
        , mByteCode(NULL)
        , mHighlightIndex(-1)
        , mNumHighlights(0)
    {
        LoadScript();
    }

    void DoFunctionCall(unsigned int);
    static ReplayChoreo& Instance();
    void LoadScript();
    void EventHandler(Event*);
    static void EventHandler(Event* event, void* userData) { ((ReplayChoreo*)userData)->EventHandler(event); }
    void Reset();
    void CalcAutoReplayScriptName(ReplayType) const;
    void StartAutoReplay(ReplayType);
    void FlushHighlights();
    void Update(float);
    bool Done() const;
    void SaveHighlight(ReplayChoreo::HighlightQuality);
    int NumHighlights() const;
    ~ReplayChoreo();

    int mNumScripts[3][3][9];       // offset 0x24, size 0x144
    ReplayManager* mReplayManager;  // offset 0x168, size 0x4
    Replay* mReplay;                // offset 0x16C, size 0x4
    ReplayCamera mCamera;           // offset 0x170, size 0x8C
    float mRunForTimeLeft;          // offset 0x1FC, size 0x4
    bool mRunningFor;               // offset 0x200, size 0x1
    void* mByteCode;                // offset 0x204, size 0x4
    GoalScoredData mGoalScoredData; // offset 0x208, size 0x24
    int mReplayPad;                 // offset 0x22C, size 0x4
    Highlight mHighlights[3];       // offset 0x230, size 0x9C
    int mHighlightIndex;            // offset 0x2CC, size 0x4
    int mNumHighlights;             // offset 0x2D0, size 0x4
}; // total size: 0x2D4

#endif // _REPLAYCHOREO_H_
