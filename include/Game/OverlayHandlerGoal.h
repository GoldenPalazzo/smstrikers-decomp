#ifndef _OVERLAYHANDLERGOAL_H_
#define _OVERLAYHANDLERGOAL_H_

#include "Game/FE/BaseOverlayHandler.h"
#include "Game/Sys/eventman.h"

class GoalOverlay : public BaseOverlayHandler
{
public:
    GoalOverlay();
    virtual ~GoalOverlay();
    virtual void SceneCreated();
    virtual void Update(float);
    static void eventHandler(Event*, void*);
    void UpdateGoalInfo(int, int, bool, int);
    void SetHighlightNumber(int);
    void DoMatchEndOverlay();
    void SetWinnerTitle();
    void DoCupWinOverlay();
    void Restart();
    int mCaptainGoals[2];        // offset 0x28, size 0x8
    int mSidekickGoals[2];       // offset 0x30, size 0x8
    EventHandler* mEventHandler; // offset 0x38, size 0x4
    u16 mClockBuffer[32];        // offset 0x3C, size 0x40
    u16 mDescriptionBuffer[128]; // offset 0x7C, size 0x100
    u16 mScoresBuffer[128];      // offset 0x17C, size 0x100
    bool mHasSniperCup;          // offset 0x27C, size 0x1
    bool mIsCreated;             // offset 0x27D, size 0x1
    bool mIsInOvertime;          // offset 0x27E, size 0x1
};

/*
class BasicString<char, Detail
{
public:
    void TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&);
    void TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&) const;
};


class FEFinder<TLTextInstance, 3>
{
public:
    void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
    void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
    void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
    void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
};


class Format<BasicString<unsigned short, Detail
{
public:
    void TempStringAllocator>, unsigned short[32], BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[32], const BasicString<unsigned short, Detail::TempStringAllocator>&);
    void TempStringAllocator>, unsigned short[16], unsigned short[16]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[16], const unsigned short(&)[16]);
    void TempStringAllocator>, const unsigned short*, unsigned short[32], unsigned short[32]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short(&)[32], const unsigned short(&)[32]);
};


class FormatImpl<BasicString<unsigned short, Detail
{
public:
    void TempStringAllocator>>::operator%<BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&);
    void TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&);
};
*/

#endif // _OVERLAYHANDLERGOAL_H_
