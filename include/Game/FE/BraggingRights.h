#ifndef _BRAGGINGRIGHTS_H_
#define _BRAGGINGRIGHTS_H_

#include "Game/BaseSceneHandler.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feMenu.h"

class FEScrollText;

class BraggingRightsScene : public BaseSceneHandler
{
public:
    BraggingRightsScene();
    ~BraggingRightsScene();
    void SceneCreated();
    void Update(float);

    /* 0x001C */ u16 mBuffer[5][256];
    /* 0x0A1C */ u16 mThisCupBuffer[5][256];
    /* 0x141C */ u16 mRatioBuffer[256];
    /* 0x161C */ s8 mUserPlace;
    /* 0x1620 */ ButtonComponent mButtons;
};

class BraggingRightsOverlay : public BaseSceneHandler
{
public:
    BraggingRightsOverlay();
    ~BraggingRightsOverlay();
    void SceneCreated();
    void IngameSceneCreated();
    void TournamentSceneCreated();
    void Update(float);
    void ChangeTicker(int);

    /* 0x001C */ u16 mBuffer[256];
    /* 0x021C */ MenuList<TLComponentInstance> mMenuItems;
    /* 0x0430 */ FEScrollText* mTicker;
    /* 0x0434 */ int mHighestStats[5];
    /* 0x0448 */ int mAwardWinners[5];
    /* 0x045C */ u8 mIsTournamentScene;
    /* 0x0460 */ ButtonComponent mButtons;
};

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class Format<BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>, unsigned short[32], unsigned short[32]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[32], const unsigned short(&)[32]);
//     void TempStringAllocator>, const unsigned short*, unsigned short[16]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short(&)[16]);
// };

// class FormatImpl<BasicString<unsigned short, Detail
// {
// public:
//     void TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&);
// };

#endif // _BRAGGINGRIGHTS_H_
