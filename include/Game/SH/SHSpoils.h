#ifndef _SHSPOILS_H_
#define _SHSPOILS_H_

#include "Game/BaseSceneHandler.h"
#include "Game/FE/feMenu.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/FE/feButtonComponent.h"

class SpoilsScene : public BaseSceneHandler
{
public:
    enum eSpoils
    {
        SPOILS_INVALID = -1,
        SPOILS_CUP = 0,
        SPOILS_SUPER_CUP = 1,
        SPOILS_MILESTONE = 2,
        SPOILS_NUM_CHOICES = 3,
    };

    SpoilsScene();
    virtual ~SpoilsScene();
    virtual void Update(float);
    virtual void SceneCreated();
    void ShowSpoils(SpoilsScene::eSpoils);

    /* 0x01C */ MenuList<TLComponentInstance> mMenuItems; // size 0x214
    /* 0x230 */ ButtonComponent mButtons;                 // size 0x24

    static s32 mLastSelectedIndex;
}; // total size: 0x254

// class FEFinder<TLComponentInstance, 4>
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
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene*,
//                               SpoilsScene::eSpoils>>::~FunctorImpl();
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene*,
//                               SpoilsScene::eSpoils>>::operator()(TLComponentInstance*);
//     void FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene*,
//                               SpoilsScene::eSpoils>>::Clone() const;
// };

// class MemFun < SpoilsScene, void, SpoilsScene
// {
// public:
//     void eSpoils > (void (SpoilsScene::*)(SpoilsScene::eSpoils));
// };

// class Bind < void, Detail
// {
// public:
//     void MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene *,
//         SpoilsScene::eSpoils
//             > (Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene* const&, const SpoilsScene::eSpoils&);
// };

#endif // _SHSPOILS_H_
