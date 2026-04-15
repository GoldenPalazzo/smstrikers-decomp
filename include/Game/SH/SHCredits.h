#ifndef _SHCREDITS_H_
#define _SHCREDITS_H_

#include "Game/BaseSceneHandler.h"
#include "Game/Sys/simpleparser.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/FE/feSceneManager.h"
#include "Game/FE/feFinder.h"
#include "Game/GameSceneManager.h"
#include "Game/FE/feMusic.h"

class TLComponentInstance;
class TLImageInstance;

struct CreditParser
{
    CreditParser()
        : mFileData(NULL)
        , mActualSize(0)
        , mActualData(NULL)
    {
    }
    /* 0x00 */ unsigned long mFileSize;   // size 0x4
    /* 0x04 */ char* mFileData;           // size 0x4
    /* 0x08 */ unsigned long mActualSize; // size 0x4
    /* 0x0C */ char* mActualData;         // size 0x4
    /* 0x10 */ SimpleParser mParser;      // size 0x514
}; // total size: 0x524

class CreditScene : public BaseSceneHandler
{
public:
    CreditScene();
    ~CreditScene();
    void SceneCreated();
    void Update(float);
    void SetupForPhase();
    void GotoNextPhase();
    void SetupForCredits();
    void SetupForNLGMovie();
    void UpdateForCopyrightMessage(float);
    void UpdateForCredits(float);
    void UpdateForNintendoLogo(float);
    void UpdateForNLGMovie(float);
    TLComponentInstance* GetWhiteFadeComponent();

    static SceneList mNextScene;

    /*  0x01C */ TLTextInstance* m_pTextLines[10]; // offset 0x1C, size 0x28
    /*  0x044 */ bool mLineOnScreen[10];           // offset 0x44, size 0xA
    /*  0x04E */ bool mAreCreditsOver;             // offset 0x4E, size 0x1
    /*  0x04F */ bool mFinalMessageDisplayed;      // offset 0x4F, size 0x1
    /*  0x050 */ bool mTimeElapsed;                // offset 0x50, size 0x1
    /*  0x054 */ CreditParser mCreditParser;       // offset 0x54, size 0x524
    /*  0x578 */ unsigned short mStrings[10][64];  // offset 0x578, size 0x500
}; // total size: 0xA78

// class FEFinder<TLComponentInstance, 4>
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

// class FEFinder<TLTextInstance, 3>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher);
// };

#endif // _SHCREDITS_H_
