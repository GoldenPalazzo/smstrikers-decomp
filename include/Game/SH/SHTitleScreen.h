#ifndef _SHTITLESCREEN_H_
#define _SHTITLESCREEN_H_

#include "Game/BaseSceneHandler.h"
#include "Game/FE/tlTextInstance.h"
#include "NL/nlConfig.h"

void DoNothingCallback();
void StartMovieCB();
// void 0x800AD0C8..0x800AD4F8 | size: 0x430;

class TitleScene : public BaseSceneHandler
{
public:
    TitleScene();
    virtual ~TitleScene();
    virtual void Update(float);
    virtual void SceneCreated();

    static void StartIntroMovie();

    // /* 0x04 */ char pad4[4];
    // /* 0x08 */ s8 m_unk_0x08;      /* inferred */
    //                                // /* 0x09 */ char pad9[3];       /* maybe part of unk8[4]? */
    // /* 0x0C */ s32 m_unk_0x0C;     /* inferred */
    // /* 0x10 */ s32 m_unk_0x10;     /* inferred */
    // /* 0x14 */ void* m_unk_0x14;   /* inferred */
    // /* 0x18 */ s32 m_unk_0x18;     /* inferred */
    // /* 0x1C */ f32 m_unk_0x1C;     /* inferred */
    // /* 0x20 */ Config* m_unk_0x20; /* inferred */
    // /* 0x24 */ u8 m_unk_0x24;      /* inferred */
    // /* 0x25 */ s8 m_unk_0x25;      /* inferred */

    /* 0x1C */ float m_fTimeElapsed;
    /* 0x20 */ TLTextInstance* mTextPressStart;
    /* 0x24 */ bool mStartedDemo;
    /* 0x25 */ bool mStartedMovie;
}; // total size: 0x28

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

#endif // _SHTITLESCREEN_H_
