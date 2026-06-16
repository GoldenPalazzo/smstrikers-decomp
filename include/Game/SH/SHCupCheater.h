#ifndef _SHCUPCHEATER_H_
#define _SHCUPCHEATER_H_

#include "Game/BaseSceneHandler.h"
#include "Game/FE/feSlideMenu.h"
#include "Game/GameSceneManager.h"

class CupCheaterScene : public BaseSceneHandler
{
public:
    CupCheaterScene();
    ~CupCheaterScene();
    void SceneCreated();
    void Update(float);
    void OnSelectGameplay();
    void OnSelectHomeWin();
    void OnSelectAwayWin();
    void OnSelectHomeOTWin();
    void OnSelectAwayOTWin();
    void AddMilestoneStats();
    void UpdateSlides();

    /* 0x1C */ FESlideMenu* m_SlideMenu;            // size 0x4
    /* 0x20 */ int mSniper;                         // size 0x4
    /* 0x24 */ int mStriker;                        // size 0x4
    /* 0x28 */ int mTactician;                      // size 0x4
    /* 0x2C */ int mParamedic;                      // size 0x4
    /* 0x30 */ int mVeteran;                        // size 0x4
    /* 0x34 */ unsigned short mSniperBuffer[10];    // size 0x14
    /* 0x48 */ unsigned short mStrikerBuffer[10];   // size 0x14
    /* 0x5C */ unsigned short mTacticianBuffer[10]; // size 0x14
    /* 0x70 */ unsigned short mParamedicBuffer[10]; // size 0x14
    /* 0x84 */ unsigned short mVeteranBuffer[10];   // size 0x14
}; // total size: 0x98

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
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher);
// };

// class Function0<void>
// {
// public:
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupCheaterScene::*)()>, CupCheaterScene*>>::~FunctorImpl();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupCheaterScene::*)()>, CupCheaterScene*>>::operator()();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupCheaterScene::*)()>, CupCheaterScene*>>::Clone() const;
// };

// class MemFun<CupCheaterScene, void>(void (CupCheaterScene
// {
// public:
//     void *)());
// };

// class Bind<void, Detail
// {
// public:
//     void MemFunImpl<void, void (CupCheaterScene::*)()>, CupCheaterScene*>(Detail::MemFunImpl<void, void (CupCheaterScene::*)()>,
//     CupCheaterScene* const&);
// };

#endif // _SHCUPCHEATER_H_
