#ifndef _FESLIDEMENU_H_
#define _FESLIDEMENU_H_

#include "Game/FE/tlComponentInstance.h"
#include "NL/nlFunction.h"
#include "Game/FE/feMenu.h"

class SlideMenuItem
{
public:
    SlideMenuItem();
    virtual ~SlideMenuItem()
    {
        delete mComponentInstance;
    };

    /* 0x4 */ unsigned long mSlideMenuHash;
    /* 0x8 */ TLComponentInstance* mComponentInstance;
    /* 0xC */ int mUserEnumType;
}; // total size: 0x10

class SlideMenuList : public MenuList<SlideMenuItem>
{
public:
    SlideMenuList()
    {
        mInputLocked = 0;
    }
    SlideMenuList(TLComponentInstance* comp)
    {
        mInputLocked = 0;
        mComponentInstance = comp;
    }
    virtual ~SlideMenuList();
    virtual void Update(float dt);

    void SetSlide()
    {
        MenuItem<SlideMenuItem>& mi = mMenuItems[mCurrentIndex];
        mi.mType->mComponentInstance->SetActiveSlide(mi.mType->mSlideMenuHash);
    }

    /* 0x214 */ unsigned char mInputLocked;
    /* 0x218 */ TLComponentInstance* mComponentInstance;
}; // total size: 0x21C

class FESlideMenu
{
public:
    struct MenuItem
    {
        MenuItem() { };
        ~MenuItem() { };

        /* 0x0 */ unsigned long ItemSlide;
        /* 0x4 */ Function<FnVoidVoid> ItemCBFuncs[2];
    }; // total size: 0x14

    void UpdatePresentation();
    bool PrevItem();
    bool NextItem();
    void SetSlideByIndex(unsigned char);
    bool ApplyFunction();
    MenuItem* AddMenuItem(const char*);
    MenuItem* AddMenuItem(const char*, const Function<FnVoidVoid>&);
    ~FESlideMenu();
    FESlideMenu(TLComponentInstance*);

    /* 0x0,  */ MenuItem m_menuItems[16];
    /* 0x140 */ unsigned char m_size;
    /* 0x141 */ unsigned char m_currentSlide;
    /* 0x142 */ unsigned char m_doWrapAround;
    /* 0x144 */ TLComponentInstance* m_pMenuComp;
    /* 0x148 */ unsigned char m_lockInput;
    /* 0x14C */ void* m_callbackParam;
    /* 0x150 */ long mLastChosenSlide;
    /* 0x154 */ long mLastRandomSlide;
    /* 0x158 */ long mNumCyclesRemaining;
    /* 0x15C */ float mRandDeltaTime;
}; // total size: 0x160

#endif // _FESLIDEMENU_H_
