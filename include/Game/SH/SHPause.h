#ifndef _SHPAUSE_H_
#define _SHPAUSE_H_

#include "types.h"

#include "Game/FE/feInput.h"
#include "Game/FE/feMenu.h"
#include "Game/BaseSceneHandler.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/FE/feButtonComponent.h"

class PauseMenuScene : public BaseSceneHandler
{
public:
    enum ScreenContext
    {
        SC_UNKNOWN = -1,
        SC_REGULAR_PAUSE = 0,
        SC_101_PAUSE = 1,
        SC_LAST = 2,
    };

    enum TransitionType
    {
        TT_INVALID = -1,
        TT_IN = 0,
        TT_OUT = 1,
    };

    PauseMenuScene(PauseMenuScene::ScreenContext);
    ~PauseMenuScene();
    void OnSelectRESUME(TLComponentInstance*);
    void OnSelectQUIT(TLComponentInstance*);
    void OnSelectCHOOSESIDES(TLComponentInstance*);
    void OnSelectAUDIOOPTIONS(TLComponentInstance*);
    void OnSelectVISUALOPTIONS(TLComponentInstance*);
    void OnSelectSTATISTICS(TLComponentInstance*);
    void OnSelectPopupNOFORFEIT();
    void OnSelectPopupYESFORFEIT();
    void OnSelectLESSONS(TLComponentInstance*);
    void SceneCreated();
    void Update(float);
    void TransitionOut(PauseMenuScene::TransitionType);
    void OpenItem(TLComponentInstance*);

    /* 0x01C */ ScreenContext mContext;
    /* 0x020 */ bool mGameIsOver;
    /* 0x024 */ float mQuitDelay;
    /* 0x028 */ eFEINPUT_PAD mQuittingController;
    /* 0x02C */ MenuList<TLComponentInstance> mMenuItems;
    /* 0x240 */ TransitionType mTransitionTo;
    /* 0x244 */ bool mIsInTransition;
    /* 0x245 */ bool mStartAnimAtEnd;
    /* 0x248 */ ButtonComponent mButtons;
    /* 0x26C */ ButtonComponent mButtons2;

    static eFEINPUT_PAD mControllingInput;
    static s32 mLastSelectedIndex;
}; // total size: 0x290

// class FEFinder<TLTextInstance, 3>
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

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
//     long); void Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//     InlineHasher);
// };

// class Function1<void, TLComponentInstance*>
// {
// public:
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*,
//     Placeholder<0>>>::~FunctorImpl(); void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void
//     (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>>::operator()(TLComponentInstance*); void
//     FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*,
//     Placeholder<0>>>::Clone() const;
// };

// class Function0<void>
// {
// public:
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::~FunctorImpl();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::operator()();
//     void FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::Clone() const;
// };

// class MemFun<PauseMenuScene, void, TLComponentInstance*>(void (PauseMenuScene
// {
// public:
//     void *)(TLComponentInstance*));
// };

// class MemFun<PauseMenuScene, void>(void (PauseMenuScene
// {
// public:
//     void *)());
// };

// class Bind<void, Detail
// {
// public:
//     void MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>(Detail::MemFunImpl<void, void
//     (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene* const&, const Placeholder<0>&); void MemFunImpl<void, void
//     (PauseMenuScene::*)()>, PauseMenuScene*>(Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene* const&);
// };

#endif // _SHPAUSE_H_
