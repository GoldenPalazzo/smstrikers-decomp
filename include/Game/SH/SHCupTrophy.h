#ifndef _SHCUPTROPHY_H_
#define _SHCUPTROPHY_H_

#include "types.h"

#include "Game/BaseSceneHandler.h"
#include "Game/TrophyInfo.h"
#include "Game/FE/feAsyncImage.h"
#include "Game/FE/feButtonComponent.h"

struct Spoil;

class CupTrophyScene : public BaseSceneHandler
{
public:
    CupTrophyScene();
    ~CupTrophyScene();
    void SceneCreated();
    void HandleUnlockedTriggers();
    void Update(float);
    void CreateTrophyScene(eTrophyType, ButtonComponent::ButtonState, bool);
    void SetWinRecord(Spoil&);
    void SetLossRecord(Spoil&);
    void SetHistory(Spoil&);
    void ChangeSlides();

    /* 0x1C, */ eTrophyType mTrophy;                       // size 0x4
    /* 0x20, */ eTrophyType mFirstTrophy;                  // size 0x4
    /* 0x24, */ eTrophyType mLastTrophy;                   // size 0x4
    /* 0x28, */ bool mCreated;                             // size 0x1
    /* 0x29, */ bool mIsNew;                               // size 0x1
    /* 0x2A, */ bool mFirstSlideChange;                    // size 0x1
    /* 0x02C */ unsigned short mFirstWinBuffer[128];       // size 0x100
    /* 0x12C */ unsigned short mHistoryBuffer[128];        // size 0x100
    /* 0x22C */ unsigned short mUnknownBuffer[128];        // size 0x100
    /* 0x32C */ unsigned short mRecordBuffer[3][128];      // size 0x300
    /* 0x62C */ unsigned short mWonBuffer[128];            // size 0x100
    /* 0x72C */ int mScrollOffset;                         // size 0x4
    /* 0x730 */ int mRow;                                  // size 0x4
    /* 0x734 */ AsyncImage* mAsyncTrophy;                  // size 0x4
    /* 0x738 */ bool mDoBlockLoad;                         // size 0x1
    /* 0x73C */ ButtonComponent::ButtonState mButtonState; // size 0x4
    /* 0x740 */ ButtonComponent mButtons;                  // size 0x24
    /* 0x764 */ ButtonComponent mButtons2;                 // size 0x24
}; // total size: 0x788

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher); void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//     unsigned long); void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class BasicString<char, Detail
// {
// public:
//     void TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&);
//     void TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&) const;
// };

// class Function0<void>
// {
// public:
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTrophyScene::*)()>, CupTrophyScene*>>::~FunctorImpl();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTrophyScene::*)()>, CupTrophyScene*>>::operator()();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTrophyScene::*)()>, CupTrophyScene*>>::Clone() const;
// };

// class Format<BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>, unsigned short[32]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned
//     short(&)[32]); void TempStringAllocator>, unsigned short[16], unsigned short[16], unsigned short[16], const unsigned short*, const
//     unsigned short*>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[16], const unsigned
//     short(&)[16], const unsigned short(&)[16], const unsigned short* const&, const unsigned short* const&); void TempStringAllocator>,
//     unsigned short[16], unsigned short[16], unsigned short[16], const unsigned short*, const unsigned short*, const unsigned
//     short*>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[16], const unsigned short(&)[16],
//     const unsigned short(&)[16], const unsigned short* const&, const unsigned short* const&, const unsigned short* const&);
// };

// class FormatImpl<BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&);
// };

// class MemFun<CupTrophyScene, void>(void (CupTrophyScene
// {
// public:
//     void *)());
// };

// class Bind<void, Detail
// {
// public:
//     void MemFunImpl<void, void (CupTrophyScene::*)()>, CupTrophyScene*>(Detail::MemFunImpl<void, void (CupTrophyScene::*)()>,
//     CupTrophyScene* const&);
// };

#endif // _SHCUPTROPHY_H_
