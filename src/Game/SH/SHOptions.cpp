#include "Game/SH/SHOptions.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feMusic.h"
#include "Game/GameInfo.h"
#include "Game/SH/SHCredits.h"
#include "Game/SH/SHSaveLoad.h"
#include "NL/glx/glxSwap.h"

extern bool DidContinueWithoutOperation();
extern u8 mLastSaveLoadSuccess__13SaveLoadScene;
extern unsigned long mUserInfoCRC__12OptionsScene;

static const eMenuState MenuToMenuStateMap[] = {
    MS_AUDIO,
    MS_VISUAL,
    MS_GAMEPLAY,
    MS_CHEATS,
    MS_SAVE_LOAD,
    MS_NUMMENUSTATES,
};

// /**
//  * Offset/Address/Size: 0x0 | 0x800B5004 | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (OptionsScene::*)(eMenuState)>, OptionsScene*, eMenuState>(
//     Detail::MemFunImpl<void, void (OptionsScene::*)(eMenuState)>, OptionsScene* const&, const eMenuState&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B4FE8 | size: 0x1C
//  */
// void MemFun<OptionsScene, void, eMenuState>(void (OptionsScene::*)(eMenuState))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B4F8C | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp2<void, Detail::MemFunImpl<void, void (OptionsScene::*)(eMenuState)>, OptionsScene*, eMenuState>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800B4E30 | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                 unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800B4DAC | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                              unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800B4D74 | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800B4C18 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800B4B94 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B4B5C | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x800B4B28 | size: 0x34
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (OptionsScene::*)(eMenuState)>,
//                                                                  OptionsScene*, eMenuState>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B4AA8 | size: 0x80
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp2<void, Detail::MemFunImpl<void, void (OptionsScene::*)(eMenuState)>, OptionsScene*, eMenuState>>::Clone() const
// {
// }

s32 OptionsScene::mLastSelectedIndex;

/**
 * Offset/Address/Size: 0x1460 | 0x800B4A1C | size: 0x8C
 */
void ApplyChangesCB()
{
    OptionsScene* scene = (OptionsScene*)nlSingleton<GameSceneManager>::s_pInstance->GetScene(SCENE_OPTIONS);

    if (scene->m_curMenuState == MS_AUDIO)
    {
        OptionsAudioMenuV2* subMenuBytes = (OptionsAudioMenuV2*)scene->m_subMenu;
        if (subMenuBytes->mbUpdateMode)
        {
            FEPopupMenu* popup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);
            popup->Create(POPUP_APPLYING_AUDIO);
            scene->mPopupResult = PR_APPLY_DELAYED_AUDIO_CHANGES;
            return;
        }
    }

    scene->mPopupResult = PR_APPLY_CHANGES;
}

/**
 * Offset/Address/Size: 0x1430 | 0x800B49EC | size: 0x30
 */
void RevertChangesCB()
{
    OptionsScene* scene = (OptionsScene*)nlSingleton<GameSceneManager>::s_pInstance->GetScene(SCENE_OPTIONS);
    scene->mPopupResult = PR_REVERT_CHANGES;
}

/**
 * Offset/Address/Size: 0x1304 | 0x800B48C0 | size: 0x12C
 */
OptionsScene::OptionsScene()
    : BaseSceneHandler()
    , m_subMenu(NULL)
    , m_curMenuState(MENUSTATE_INVALID)
    , mButtons()
    , mMenuItems()
    , mPopupResult(PR_DO_NOTHING)
{
    eMenuState menuState = MenuToMenuStateMap[mLastSelectedIndex];

    if (menuState != MS_NUMMENUSTATES)
    {
        if (menuState == MS_SAVE_LOAD)
        {
            if ((mLastSaveLoadSuccess__13SaveLoadScene != 0) && (DidContinueWithoutOperation() == false))
            {
                mUserInfoCRC__12OptionsScene = nlChecksum32(&(nlSingleton<GameInfoManager>::s_pInstance->mUserInfo), 0x113C);
            }
        }
        else if (mLastSelectedIndex == 0)
        {
            mUserInfoCRC__12OptionsScene = nlChecksum32(&(nlSingleton<GameInfoManager>::s_pInstance->mUserInfo), 0x113C);
        }
    }

    mLastSaveLoadSuccess__13SaveLoadScene = 0;
}

/**
 * Offset/Address/Size: 0x123C | 0x800B47F8 | size: 0xC8
 */
OptionsScene::~OptionsScene()
{
    if (m_subMenu != NULL)
    {
        delete m_subMenu;
    }
}

/**
 * Offset/Address/Size: 0xB5C | 0x800B4118 | size: 0x6E0
 */
void OptionsScene::SceneCreated()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0xAF4 | 0x800B40B0 | size: 0x68
 */
void OptionsScene::Update(float dt)
{
    BaseSceneHandler::Update(dt);
    mButtons.CentreButtons();

    if (m_curMenuState == MS_MAIN)
    {
        UpdateForMain(dt);
    }
    else
    {
        UpdateForSubOptionMenus(dt);
    }
}

/**
 * Offset/Address/Size: 0x6EC | 0x800B3CA8 | size: 0x408
 */
void OptionsScene::UpdateForMain(float)
{
    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        FEAudio::PlayAnimAudioEvent("sfx_screen_forward", false);

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

        mLastSelectedIndex = mMenuItems.mCurrentIndex;
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

        if (SaveLoadScene::IsIOEnabled())
        {
            unsigned long currentcrc = nlChecksum32(&(nlSingleton<GameInfoManager>::s_pInstance->mUserInfo), 0x113C);

            if (currentcrc != mUserInfoCRC__12OptionsScene)
            {
                SaveLoadScene* scene = (SaveLoadScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_SAVE, SCREEN_NOTHING, false);
                scene->mNextScene = SCENE_MAIN_MENU;
            }
            else
            {
                nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_BACK, false);
            }
        }
        else
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_BACK, false);
        }

        mLastSelectedIndex = 0;
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int skipDisabledFlag;
        int wrapFlag = flags & 1;
        skipDisabledFlag = flags & 2;
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

        if (skipDisabledFlag)
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
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int skipDisabledFlag;
        int wrapFlag = flags & 1;
        skipDisabledFlag = flags & 2;
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

        if (skipDisabledFlag)
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
 * Offset/Address/Size: 0x268 | 0x800B3824 | size: 0x484
 */
void OptionsScene::UpdateForSubOptionMenus(float fDeltaT)
{
    static bool gStartLoadingBar = false;

    if (mPopupResult != PR_DO_NOTHING)
    {
        if (mPopupResult == PR_APPLY_CHANGES)
        {
            if (gStartLoadingBar)
            {
                glxSwapLoading(true, false);
            }
            m_subMenu->Save();
            if (gStartLoadingBar)
            {
                glxSwapLoading(false, false);
            }
            gStartLoadingBar = false;
        }
        else if (mPopupResult == PR_REVERT_CHANGES)
        {
            m_subMenu->Revert();
        }
        else if (mPopupResult == PR_APPLY_DELAYED_AUDIO_CHANGES)
        {
            GameSceneManager* gsm = nlSingleton<GameSceneManager>::s_pInstance;
            FEPopupMenu* scene;
            if (gsm->mCurrentStackDepth != 0)
            {
                scene = (FEPopupMenu*)gsm->mBaseSceneHandlerStack[gsm->mCurrentStackDepth - 1];
            }
            else
            {
                scene = NULL;
            }

            if (!scene->m_pFEScene->m_bValid)
            {
                return;
            }

            TLSlide* slide = scene->m_pFEPresentation->m_currentSlide;
            f32 endTime = slide->m_start + slide->m_duration;
            f32 curTime = slide->m_time;

            static float HACK_DELAY_UNTIL_APPLY = 0.0f;

            if (curTime != endTime)
            {
                scene->m_pFEPresentation->m_fadeDuration = 999.9f;
                scene->m_pFEPresentation->Update(0.0f);
                HACK_DELAY_UNTIL_APPLY = 0.0f;
                return;
            }

            HACK_DELAY_UNTIL_APPLY += fDeltaT;
            if (HACK_DELAY_UNTIL_APPLY >= 0.5f)
            {
                mPopupResult = PR_APPLY_CHANGES;
                nlSingleton<GameSceneManager>::s_pInstance->Pop();
                gStartLoadingBar = true;
            }
            return;
        }

        m_subMenu->GoBack();
        m_pFEScene->m_pFEPackage->GetPresentation();

        if (m_subMenu != NULL)
        {
            delete m_subMenu;
            m_subMenu = NULL;
        }

        mMenuItems.mCurrentIndex = mLastSelectedIndex;

        int selIdx = mMenuItems.mCurrentIndex;
        Tag tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
        if (((u32)((-((s32)tag)) | tag) >> 31) != 0)
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

        m_curMenuState = MS_MAIN;

        FEAudio::PlayAnimAudioEvent((mPopupResult == PR_APPLY_CHANGES) ? "sfx_accept" : "sfx_back", false);

        mPopupResult = PR_DO_NOTHING;
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        if (m_subMenu->ChangesMade())
        {
            FEPopupMenu* popup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

            Function<FnVoidVoid> applyFn;
            applyFn.mTag = FREE_FUNCTION;
            applyFn.mFreeFunction = ApplyChangesCB;

            Function<FnVoidVoid> revertFn;
            revertFn.mTag = FREE_FUNCTION;
            revertFn.mFreeFunction = RevertChangesCB;

            popup->Create(POPUP_REVERT_OPTION_CHANGES, applyFn, revertFn);
        }
        else
        {
            m_subMenu->GoBack();
            m_pFEScene->m_pFEPackage->GetPresentation();

            if (m_subMenu != NULL)
            {
                delete m_subMenu;
                m_subMenu = NULL;
            }

            mMenuItems.mCurrentIndex = mLastSelectedIndex;

            int selIdx = mMenuItems.mCurrentIndex;
            Tag tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-((s32)tag)) | tag) >> 31) != 0)
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

            m_curMenuState = MS_MAIN;
            FEAudio::PlayAnimAudioEvent("sfx_back", false);
            FEAudio::PlayAnimAudioEvent("sfx_screen_back", false);
            return;
        }
    }

    m_subMenu->Update(fDeltaT);
}

/**
 * Offset/Address/Size: 0x0 | 0x800B35BC | size: 0x268
 */
void OptionsScene::ChangeMenuState(eMenuState newState)
{
    FEPresentation* pres = m_pFEScene->m_pFEPackage->GetPresentation();

    if (m_subMenu != NULL)
    {
        delete m_subMenu;
        m_subMenu = NULL;
    }

    switch (newState)
    {
    case MS_MAIN:
    {
        mMenuItems.mCurrentIndex = mLastSelectedIndex;

        int idx = mMenuItems.mCurrentIndex;
        Tag tag = mMenuItems.mMenuItems[idx].mCallbacks[1].mTag;

        if (((u32)((-((s32)tag)) | tag) >> 31) != 0)
        {
            TLComponentInstance* item = mMenuItems.mMenuItems[idx].mType;

            if (tag == FREE_FUNCTION)
            {
                mMenuItems.mMenuItems[idx].mCallbacks[1].mFreeFunction(item);
            }
            else
            {
                (*mMenuItems.mMenuItems[idx].mCallbacks[1].mFunctor)(item);
            }
        }
        break;
    }
    case MS_AUDIO:
        m_subMenu = new ((OptionsAudioMenuV2*)nlMalloc(sizeof(OptionsAudioMenuV2), 8, false))
            OptionsAudioMenuV2(pres, ButtonComponent::BS_B_ONLY, nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mAudioOptions);
        break;
    case MS_VISUAL:
        m_subMenu = new ((OptionsVisualMenuV2*)nlMalloc(sizeof(OptionsVisualMenuV2), 8, false))
            OptionsVisualMenuV2(pres, ButtonComponent::BS_B_ONLY, nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mVisualOptions);
        break;
    case MS_GAMEPLAY:
    {
        bool showLegend = nlSingleton<GameInfoManager>::s_pInstance->IsLegendSkillUnlocked();
        m_subMenu = new ((OptionsGameplayMenuV2*)nlMalloc(sizeof(OptionsGameplayMenuV2), 8, false))
            OptionsGameplayMenuV2(pres,
                ButtonComponent::BS_B_ONLY,
                nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mGameplayOptions,
                !showLegend ? 4 : -1);
        break;
    }
    case MS_CHEATS:
        m_subMenu = new ((OptionsCheatsMenu*)nlMalloc(sizeof(OptionsCheatsMenu), 8, false))
            OptionsCheatsMenu(pres, ButtonComponent::BS_B_ONLY, nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mCheatOptions);
        break;
    case MS_SAVE_LOAD:
        m_subMenu = new ((OptionsSaveLoad*)nlMalloc(sizeof(OptionsSaveLoad), 8, false))
            OptionsSaveLoad(pres, ButtonComponent::BS_A_AND_B);
        break;
    case MS_NUMMENUSTATES:
        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CREDITS, SCREEN_NOTHING, false);
        CreditScene::mNextScene = SCENE_OPTIONS;
        FEMusic::StopStream();
        break;
    default:
        break;
    }

    m_curMenuState = newState;
}
