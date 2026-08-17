#ifndef _SHCHOOSECUP_H_
#define _SHCHOOSECUP_H_

#include "Game/BaseSceneHandler.h"
#include "Game/FE/TLComponentInstance.h"
#include "Game/FE/feAsyncImage.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feScrollText.h"
#include "Game/Team.h"
#include "Game/TrophyTextures.h"

void continueCup(bool);
void startNewCup(bool);
void confirmedNewCup(bool);

class ChooseCupSceneV2 : public BaseSceneHandler
{
public:
    ChooseCupSceneV2(bool);
    ~ChooseCupSceneV2();
    void SceneCreated();
    void Update(float);
    void DisplayCup();
    void SetCurrentChamp(eTeamID, bool, TLComponentInstance*);
    void Proceed();

    /* 0x1C, */ bool mIsSuperCup;                      // size 0x1
    /* 0x20, */ AsyncImage* mCupImage;                 // size 0x4
    /* 0x24, */ eTrophyType mCupToDisplay;             // size 0x4
    /* 0x28, */ eTrophyType mMinCup;                   // size 0x4
    /* 0x2C, */ eTrophyType mMaxCup;                   // size 0x4
    /* 0x30, */ bool mUpdateSlide;                     // size 0x1
    /* 0x31, */ bool mIsFirstSlide;                    // size 0x1
    /* 0x32, */ unsigned short mDescriptorBuffer[256]; // size 0x200
    /* 0x232 */ unsigned short mChampBuffer[256];      // size 0x200
    /* 0x434 */ FEScrollText* mTicker;                 // size 0x4
    /* 0x438 */ ButtonComponent mButtons;              // size 0x24
    /* 0x45C */ ButtonComponent mButtons2;             // size 0x24
    /* 0x480 */ bool mCupInProgressVisible;            // size 0x1
    /* 0x481 */ bool mCupInProgressDirty;              // size 0x1
}; // total size: 0x484

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher); void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//     unsigned long); void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLImageInstance, 2>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>::Append(const unsigned short*) const;
//     void TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<unsigned short,
//     Detail::TempStringAllocator>&); void TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<unsigned short,
//     Detail::TempStringAllocator>&) const;
// };

#endif // _SHCHOOSECUP_H_
