#ifndef _SHTOURNSETPARAMS_H_
#define _SHTOURNSETPARAMS_H_

#include "types.h"
#include "Game/BaseSceneHandler.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/FE/feButtonComponent.h"
#include "Game/FE/feMenu.h"
#include "Game/FE/feSlideMenu.h"

class TournSetParamsScene : public BaseSceneHandler
{
public:
    TournSetParamsScene();
    virtual ~TournSetParamsScene();
    virtual void Update(float);
    virtual void SceneCreated();

    void BuildSubMenuList(int, TLComponentInstance*, bool, int);
    void SetInitialParams(bool, int, int);
    void ApplyMenuDefaults();
    void InitializeMenu();

    /* 0x01C */ MenuList<TLComponentInstance> mMenuItems; // size 0x214
    /* 0x230 */ SlideMenuList* mSlideMenuLists[3];
    /* 0x23C */ bool m_isLeagueMode;
    /* 0x23D */ char pad23D[3];
    /* 0x240 */ s32 m_numTeams;
    /* 0x244 */ s32 m_numGames;
    /* 0x248 */ ButtonComponent mButtons;
}; // total size: 0x26C

// class FEFinder<TLComponentInstance, 4>
// {
// public:
//     void _Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void _Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
//     void Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
// };

#endif // _SHTOURNSETPARAMS_H_
