#ifndef _SHMAINMENU_H_
#define _SHMAINMENU_H_

#include "types.h"

#include "Game/BaseSceneHandler.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feMenu.h"
#include "Game/FE/feScrollText.h"
#include "Game/FE/tlComponentInstance.h"
#include "NL/nlColour.h"

class SHMainMenu : public BaseSceneHandler
{
public:
    SHMainMenu();
    virtual ~SHMainMenu();
    virtual void SceneCreated();
    virtual void Update(float);
    void OpenItem(TLComponentInstance*);
    void CloseItem(TLComponentInstance*);

    static bool mSnapMenuIntoPosition;
    static int mLastMenuItem;

    /* 0x01C */ FEScrollText* m_itemDescriptions;
    /* 0x020 */ nlColour mHighlightColour;
    /* 0x024 */ MenuList<TLComponentInstance> mMenuItems;
    /* 0x238 */ ButtonComponent mButtons;
}; // total size: 0x25C

// class MenuItem<TLComponentInstance>
// {
// public:
//     void MenuItem();
//     void ~MenuItem();
// };

// class FEFinder<TLImageInstance, 2>
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

// class Function1<void, TLComponentInstance*>
// {
// public:
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu*,
//     Placeholder<0>>>::~FunctorImpl(); void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void
//     (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu*, Placeholder<0>>>::operator()(TLComponentInstance*); void
//     FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu*,
//     Placeholder<0>>>::Clone() const; void FunctorBase::~FunctorBase();
// };

// class MenuList<TLComponentInstance>
// {
// public:
//     void ~MenuList();
// };

// class FEPopupMenu
// {
// public:
//     void Create(ePopupMenu);
//     void Nothing();
// };

// class MemFun<SHMainMenu, void, TLComponentInstance*>(void (SHMainMenu
// {
// public:
//     void *)(TLComponentInstance*));
// };

// class Bind<void, Detail
// {
// public:
//     void MemFunImpl<void, void (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu*, Placeholder<0>>(Detail::MemFunImpl<void, void
//     (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu* const&, const Placeholder<0>&);
// };

#endif // _SHMAINMENU_H_
