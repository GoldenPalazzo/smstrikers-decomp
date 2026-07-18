#include "Game/SH/SHOptions.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feMusic.h"
#include "Game/FE/feScene.h"
#include "Game/GameInfo.h"
#include "Game/SH/SHCredits.h"
#include "Game/SH/SHSaveLoad.h"
#include "Game/TrophyInfo.h"
#include "NL/glx/glxSwap.h"
#include "NL/nlPrint.h"

#include "NL/nlBind.h"

extern bool DidContinueWithoutOperation();
extern TLInstance* FindComponent(TLSlide*, const char*);

namespace DoubleHighlite
{
static const char* SLIDE_IN = "in";
static const char* SLIDE_OUT = "out";
} // namespace DoubleHighlite

static const eMenuState MenuToMenuStateMap[] = {
    MS_AUDIO,
    MS_VISUAL,
    MS_GAMEPLAY,
    MS_CHEATS,
    MS_SAVE_LOAD,
    MS_NUMMENUSTATES,
};

s32 OptionsScene::mLastSelectedIndex;
u32 OptionsScene::mUserInfoCRC;

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
            if ((SaveLoadScene::mLastSaveLoadSuccess != 0) && (DidContinueWithoutOperation() == false))
            {
                OptionsScene::mUserInfoCRC = nlChecksum32(&(nlSingleton<GameInfoManager>::s_pInstance->mUserInfo), 0x113C);
            }
        }
        else if (mLastSelectedIndex == 0)
        {
            OptionsScene::mUserInfoCRC = nlChecksum32(&(nlSingleton<GameInfoManager>::s_pInstance->mUserInfo), 0x113C);
        }
    }

    SaveLoadScene::mLastSaveLoadSuccess = 0;
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

typedef Detail::MemFunImpl<void, void (OptionsScene::*)(eMenuState)> MemFunImpl_Options_t;
typedef BindExp2<void, MemFunImpl_Options_t, OptionsScene*, eMenuState> BindExp2_Options_t;
typedef Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2_Options_t> FunctorImpl_Options_t;

/**
 * Offset/Address/Size: 0xB5C | 0x800B4118 | size: 0x6E0
 */
void OptionsScene::SceneCreated()
{
    FEAudio::EnableSounds(false);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    MenuItem<TLComponentInstance>* item;

    for (int i = 0; i < 6; i++)
    {
        char menuname[64];
        nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

        TLInstance* foundInstance = FEFinder<TLInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)));
        TLComponentInstance* instance = (TLComponentInstance*)foundInstance;

        if (MenuToMenuStateMap[i] == MS_NUMMENUSTATES)
        {
            if (!nlSingleton<GameInfoManager>::s_pInstance->HasTrophy(TROPHY_BOWSER_CUP))
            {
                instance->m_bVisible = false;
                continue;
            }
        }

        instance->SetActiveSlide(i == mLastSelectedIndex ? DoubleHighlite::SLIDE_IN : DoubleHighlite::SLIDE_OUT);

        item = mMenuItems.AddItem(instance);

        {
            MenuItem<TLComponentInstance>::Callback openFunc(DoubleHighlite::OpenItem);
            item->SetCallback(ON_HIGHLIGHT, openFunc);
        }

        {
            MenuItem<TLComponentInstance>::Callback closeFunc(DoubleHighlite::CloseItem);
            item->SetCallback(ON_UNHIGHLIGHT, closeFunc);
        }

        {
            MenuItem<TLComponentInstance>::Callback callback(Bind<void, MemFunImpl_Options_t, OptionsScene*, eMenuState>(
                MemFun<OptionsScene, void, eMenuState>(&OptionsScene::ChangeMenuState),
                this,
                MenuToMenuStateMap[i]));
            item->SetCallback(ON_APPLY, callback);
        }

        FindComponent(instance->GetActiveSlide(), "highlite");

        if (i == mLastSelectedIndex)
        {
            item->RunCallback(ON_HIGHLIGHT);
        }
        else
        {
            item->RunCallback(ON_UNHIGHLIGHT);

            TLSlide* slide = instance->GetActiveSlide();
            instance->Update(1.0f + (slide->m_start + slide->m_duration));
        }
    }

    mMenuItems.SetFlag(1);
    mMenuItems.SetActiveItemIndex(mLastSelectedIndex);
    mMenuItems.RunCallbackOnCurrent(ON_HIGHLIGHT);

    m_pFEScene->m_pFEPackage->GetPresentation();

    if (m_subMenu != NULL)
    {
        delete m_subMenu;
        m_subMenu = NULL;
    }

    mMenuItems.SetActiveItemIndex(mLastSelectedIndex);
    mMenuItems.RunCallbackOnCurrent(ON_HIGHLIGHT);

    {
        m_curMenuState = MS_MAIN;

        mButtons.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("buttons")));
    }

    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    FEAudio::EnableSounds(true);
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

        mMenuItems.RunCallbackOnCurrent(ON_APPLY);
        mLastSelectedIndex = mMenuItems.GetActiveItemIndex();
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

        if (SaveLoadScene::IsIOEnabled())
        {
            unsigned long currentcrc = nlChecksum32(&(nlSingleton<GameInfoManager>::s_pInstance->mUserInfo), 0x113C);

            if (currentcrc != OptionsScene::mUserInfoCRC)
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
        mMenuItems.PreviousItem();
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        mMenuItems.NextItem();
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

        mMenuItems.SetActiveItemIndex(mLastSelectedIndex);
        mMenuItems.RunCallbackOnCurrent(ON_HIGHLIGHT);

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
            popup->Create(POPUP_REVERT_OPTION_CHANGES, ApplyChangesCB, RevertChangesCB);
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

            mMenuItems.SetActiveItemIndex(mLastSelectedIndex);
            mMenuItems.RunCallbackOnCurrent(ON_HIGHLIGHT);

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
        mMenuItems.SetActiveItemIndex(mLastSelectedIndex);
        mMenuItems.RunCallbackOnCurrent(ON_HIGHLIGHT);
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
