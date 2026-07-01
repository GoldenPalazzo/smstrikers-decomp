#ifndef _FRAMECOUNTER_H_
#define _FRAMECOUNTER_H_

#include "NL/nlMath.h"
#include "NL/nlColour.h"

void DrawBrow(const nlVector3&, const nlVector3&, float, float, float);
void DrawCircle(nlVector3, float, float, nlColour);

class FrameCounter
{
public:
    static int NUM_FRAMES_TO_AVERAGE_OVER;

    FrameCounter(const char*, const char*);
    void StartTimer(int);
    void FinishTiming();
    void WriteFrameRateStatsToFile(const char*);
    void DisplayFrameSmiler();

    /* 0x000 */ const char* m_FirstName;
    /* 0x004 */ const char* m_SecondName;
    /* 0x008 */ unsigned int m_StartTick;
    /* 0x00C */ int m_CurrTimerNum;
    /* 0x010 */ float m_CurrTimer[2];
    /* 0x018 */ float m_CurrFrame[2];
    /* 0x020 */ float m_LastFrame[2];
    // /* 0x028 */ unsigned int m_Counter;
    // /* 0x02C */ unsigned int m_NextHistoryPos;

    /* 0x028 */ s32 unk28;
    /* 0x02C */ f32 unk2C;
    /* 0x030 */ u32 unk30;
    /* 0x034 */ s32 unk34;

    /* 0x038 */ float m_FrameHistory[640];
    /* 0xA38 */ int m_ContinuousFrameHistoryIndex;
    /* 0xA3C */ float m_ContinuousFrameHistory[2][200];
}; // total size: 0x107C

extern FrameCounter g_FrameCounter;

// class ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*>>>
// {
// public:
//     void DeleteEntry(ListEntry<TimeRegion*>*);
// };

// class nlListContainer<TimeRegion*>
// {
// public:
//     void ~nlListContainer();
// };

// class nlWalkList<ListEntry<TimeRegion*>, ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*>>>>(ListEntry<TimeRegion*>*, ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*>>>*, void (ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*>>>
// {
// public:
//     void *)(ListEntry<TimeRegion*>*));
// };

#endif // _FRAMECOUNTER_H_
