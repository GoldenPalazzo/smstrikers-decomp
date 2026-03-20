#include "Game/SH/SHSpoils.h"
#include "Game/GameSceneManager.h"
#include "Game/FE/feInput.h"
#include "Game/SH/SHCupTrophy.h"
#include "Game/SH/SHMilestoneTrophy.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x800D1FBC | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene*, SpoilsScene::eSpoils>(
//     Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene* const&, const SpoilsScene::eSpoils&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D1FA0 | size: 0x1C
//  */
// void MemFun<SpoilsScene, void, SpoilsScene::eSpoils>(void (SpoilsScene::*)(SpoilsScene::eSpoils))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D1F44 | size: 0x5C
//  */
// void
//     Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void
//     (SpoilsScene::*)(SpoilsScene::eSpoils)>,
//                                                                 SpoilsScene*, SpoilsScene::eSpoils>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x800D1F10 | size: 0x34
//  */
// void Function1<void,
//                TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>,
//                                                            SpoilsScene*, SpoilsScene::eSpoils>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D1E90 | size: 0x80
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp2<void, Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)>, SpoilsScene*, SpoilsScene::eSpoils>>::Clone()
//     const
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800D1D34 | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                 unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800D1CB0 | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                              unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800D1C78 | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800D1B1C | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800D1A98 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D1A60 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0xACC | 0x800D19BC | size: 0xA4
 */
SpoilsScene::SpoilsScene()
{
}

/**
 * Offset/Address/Size: 0xA28 | 0x800D1918 | size: 0xA4
 */
SpoilsScene::~SpoilsScene()
{
}

/**
 * Offset/Address/Size: 0x6B4 | 0x800D15A4 | size: 0x374
 */
void SpoilsScene::Update(float dt)
{
    extern s32 mLastSelectedIndex__11SpoilsScene;

    BaseSceneHandler::Update(dt);
    mButtons.CentreButtons();

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        int currentIndex = mMenuItems.mCurrentIndex;
        int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mTag;

        if (((u32)((-tag) | tag) >> 31) > 0)
        {
            if (mMenuItems.mMenuItems[currentIndex].mDisabled == 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;

                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mFunctor)(type);
                }
            }
        }

        mLastSelectedIndex__11SpoilsScene = mMenuItems.mCurrentIndex;
        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_BACK, true);
        mLastSelectedIndex__11SpoilsScene = 0;
        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int wrapFlag = flags & 1;
        int currentIndex = mMenuItems.mCurrentIndex;
        int newIndex = currentIndex - 1;

    loop_up:
        if (wrapFlag)
        {
            if (newIndex < 0)
            {
                newIndex = mMenuItems.mNumItemsAdded - 1;
            }
        }
        else
        {
            if (newIndex < 0)
            {
                return;
            }
        }

        if (flags & 2)
        {
            if (mMenuItems.mMenuItems[newIndex].mDisabled)
            {
                newIndex--;
                goto loop_up;
            }
        }

        {
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
                }
            }
        }

        mMenuItems.mCurrentIndex = newIndex;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
                }
            }
        }

        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int wrapFlag = flags & 1;
        int currentIndex = mMenuItems.mCurrentIndex;
        int newIndex = currentIndex + 1;

    loop_down:
        if (wrapFlag)
        {
            newIndex = newIndex % mMenuItems.mNumItemsAdded;
        }
        else
        {
            if (newIndex >= mMenuItems.mNumItemsAdded)
            {
                return;
            }
        }

        if (flags & 2)
        {
            if (mMenuItems.mMenuItems[newIndex].mDisabled)
            {
                newIndex++;
                goto loop_down;
            }
        }

        {
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
                }
            }
        }

        mMenuItems.mCurrentIndex = newIndex;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xC4 | 0x800D0FB4 | size: 0x5F0
 */
void SpoilsScene::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800D0EF0 | size: 0xC4
 */
void SpoilsScene::ShowSpoils(SpoilsScene::eSpoils spoils)
{
    if (spoils == SPOILS_CUP)
    {
        CupTrophyScene* pScene = (CupTrophyScene*)GameSceneManager::GetInstance()->Push(SCENE_CUP_TROPHY, SCREEN_FORWARD, true);
        pScene->CreateTrophyScene(TROPHY_MUSHROOM_CUP, ButtonComponent::BS_B_ONLY, false);
    }
    else if (spoils == SPOILS_SUPER_CUP)
    {
        CupTrophyScene* pScene = (CupTrophyScene*)GameSceneManager::GetInstance()->Push(SCENE_CUP_TROPHY, SCREEN_FORWARD, true);
        pScene->CreateTrophyScene(TROPHY_SUPER_MUSHROOM_CUP, ButtonComponent::BS_B_ONLY, false);
    }
    else
    {
        MilestoneTrophyScene* pScene = (MilestoneTrophyScene*)GameSceneManager::GetInstance()->Push(SCENE_MILESTONE_TROPHY, SCREEN_FORWARD, true);
        pScene->CreateTrophyScene(NUM_BATTLE_TROPHIES, ButtonComponent::BS_B_ONLY, false);
    }
}
