#ifndef _SHCHOOSESIDES_H_
#define _SHCHOOSESIDES_H_

#include "Game/BaseSceneHandler.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/FE/feAsyncImage.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feChooseSideComponent.h"
#include "Game/FE/feScrollText.h"

class SHChooseSides2 : public BaseSceneHandler
{
public:
    enum eCSContext
    {
        FRIENDLY = 0,
        CUP = 1,
        SUPERCUP = 2,
        TOURNAMENT = 3,
        PAUSE = 4,
    };

    SHChooseSides2(SHChooseSides2::eCSContext context);
    ~SHChooseSides2();
    void SceneCreated();
    void UpdateChooseSideComponent(float fDeltaT);
    void BindChooseSideInstances();
    void Update(float fDeltaT);

    /* 0x1C */ eCSContext mContext;                              // size 0x4
    /* 0x20 */ FEScrollText* m_pTicker;                          // size 0x4
    /* 0x24 */ float mSoundDelay;                                // size 0x4
    /* 0x28 */ SceneList mNextScene;                             // size 0x4
    /* 0x2C */ SceneList mBackScene;                             // size 0x4
    /* 0x30 */ AsyncImage* mAsyncImage[2][3];                    // size 0x18
    /* 0x48 */ IChooseSide mChooseSide;                          // size 0xA0 (G4QE01), 0xB0 (G4QJ01)
    /* 0xE8 (G4QE01), 0xF8 (G4QJ01) */ int mProceedDelay;        // size 0x4
    /* 0xEC (G4QE01), 0xFC (G4QJ01) */ ButtonComponent mButtons; // size 0x24
}; // total size: 0x110 (G4QE01), 0x120 (G4QJ01)

// class FEFinder<TLInstance, 2>
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

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLInstance, 3>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLInstance>(TLInstance*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLInstance, 5>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

// class FEFinder<TLInstance, 4>
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

// class BasicString<char, Detail
// {
// public:
//     void TempStringAllocator>::BasicString(const char*);
// };

#endif // _SHCHOOSESIDES_H_
