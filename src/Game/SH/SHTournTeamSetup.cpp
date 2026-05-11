#include "Game/SH/SHTournTeamSetup.h"
#include "types.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feTemplates.h"
#include "Game/GameSceneManager.h"
#include "Game/SH/SHCupHub.h"

// Temporary dummy object for reference member initialization
static CustomTournament s_dummyTourn;

static unsigned long TOURN_CAPTAIN_DESCRIPTIONS[] = {
    0xFF68ABBA, 0xE2D37C19, 0x000465BA, 0x000BAD38, 0x0043DF21, 0x330C3072, 0x00C0A242, 0x00EC84AC, 0x69BFAF9D
};

// /**
//  * Offset/Address/Size: 0x0 | 0x800E7710 | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>, TournTeamSetupSceneV2*, int>(Detail::MemFunImpl<void,
// void (TournTeamSetupSceneV2::*)(int)>, TournTeamSetupSceneV2* const&, const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E76F4 | size: 0x1C
//  */
// void MemFun<TournTeamSetupSceneV2, void, int>(void (TournTeamSetupSceneV2::*)(int))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E7698 | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>,
// TournTeamSetupSceneV2*, int>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x800E7664 | size: 0x34
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>,
// TournTeamSetupSceneV2*, int>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E75E4 | size: 0x80
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)>,
// TournTeamSetupSceneV2*, int>>::Clone() const
// {
// }

/**
 * Offset/Address/Size: 0x704 | 0x800E7488 | size: 0x15C
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 4>::_Find<TLInstance>(
    TLInstance* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->pChildren, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->pChildren, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x680 | 0x800E7404 | size: 0x84
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 4>::_Find<TLSlide>(
    TLSlide* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->m_instances, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->m_instances, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

// /**
//  * Offset/Address/Size: 0x648 | 0x800E73CC | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800E7270 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800E71EC | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800E71B4 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800E7058 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800E6FD4 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800E6F9C | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800E6E40 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800E6DBC | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E6D84 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x4DBC | 0x800E6C60 | size: 0x124
 */
TournTeamSetupSceneV2::TournTeamSetupSceneV2()
    : BaseSceneHandler()
    , mMenuItems()
    , mRowOffset(0)
    , mCurrentRow(0)
    , mTournInfo(nlSingleton<GameInfoManager>::s_pInstance->mCustomTournamentInfo)
    , mCurrentState(STATE_IN)
    , mCurrentCaptain(TEAM_MARIO)
    , mCurrentSK(SK_TOAD)
    , mButtons1()
    , mButtons2()
{
    int i = 0;

    *(u8*)((u8*)this + 0x334) = false;

    for (; i < mTournInfo.m_numTeams; i++)
    {
        mTeamData[i].isEmpty = true;
        mTeamData[i].isHumanPlayer = false;
        mTeamData[i].captain = TEAM_MARIO;
        mTeamData[i].sidekick = SK_TOAD;
    }
}

/**
 * Offset/Address/Size: 0x4C18 | 0x800E6ABC | size: 0x1A4
 */
TournTeamSetupSceneV2::~TournTeamSetupSceneV2()
{
    delete mCaptainGrid;
    delete mSKGrid;

    FEScrollText* ticker = mTicker;

    if (ticker != NULL)
    {
        if (ticker != NULL)
        {
            if ((char*)ticker + 0x21C)
            {
                volatile FEScrollText* vticker = ticker;
                if ((char*)vticker + 0x21C)
                {
                    if (ticker->m_messageFinishedCB.mTag == FUNCTOR)
                    {
                        delete ticker->m_messageFinishedCB.mFunctor;
                    }
                    ticker->m_messageFinishedCB.mTag = EMPTY;
                }
            }

            if ((char*)ticker + 4)
            {
                BasicStringData<unsigned short>* data = ticker->m_message.m_data;
                if (data != NULL)
                {
                    if (--data->mRefCount == 0)
                    {
                        if (data != NULL)
                        {
                            if (data != NULL)
                            {
                                delete[] data->mData;
                            }
                            if (data != NULL)
                            {
                                nlFree(data);
                            }
                        }
                    }
                }
            }

            ::operator delete(ticker);
        }
    }
}

/**
 * Offset/Address/Size: 0x3ED4 | 0x800E5D78 | size: 0xD44
 */
void TournTeamSetupSceneV2::SceneCreated()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    FEAudio::EnableSounds(false);

    typedef Detail::MemFunImpl<void, void (TournTeamSetupSceneV2::*)(int)> MemFunImpl_Tourn_t;
    typedef BindExp2<void, MemFunImpl_Tourn_t, TournTeamSetupSceneV2*, int> BindExp2_Tourn_t;
    typedef Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2_Tourn_t> FunctorImpl_Tourn_t;

    for (int i = 0; i < 4; i++)
    {
        char menuname[64];
        nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

        TLComponentInstance* compinstance = (TLComponentInstance*)FEFinder<TLInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        if (i < mTournInfo.m_numTeams)
        {
            compinstance->SetActiveSlide((i == 0) ? "in" : "out");
            UpdateRow(i);

            if (mCurrentState == STATE_SCROLLING)
            {
                MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
                menuItem->mType = compinstance;
                mMenuItems.mNumItemsAdded++;

                {
                    Function<TLComponentInstance*> openFunction;
                    openFunction.mTag = FREE_FUNCTION;
                    openFunction.mFreeFunction = DoubleHighlite::OpenItem;
                    menuItem->mCallbacks[ON_HIGHLIGHT] = openFunction;
                }

                {
                    Function<TLComponentInstance*> closeFunction;
                    closeFunction.mTag = FREE_FUNCTION;
                    closeFunction.mFreeFunction = DoubleHighlite::CloseItem;
                    menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunction;
                }

                {
                    BindExp2_Tourn_t bind = Bind<void, MemFunImpl_Tourn_t, TournTeamSetupSceneV2*, int>(
                        MemFun<TournTeamSetupSceneV2, void, int>(&TournTeamSetupSceneV2::StartChooseCaptain),
                        this,
                        i);

                    FunctorImpl_Tourn_t* impl
                        = new ((FunctorImpl_Tourn_t*)nlMalloc(sizeof(FunctorImpl_Tourn_t), 8, false)) FunctorImpl_Tourn_t(bind);

                    Function<TLComponentInstance*> applyFunction;
                    applyFunction.mTag = FUNCTOR;
                    applyFunction.mFunctor = impl;
                    menuItem->mCallbacks[ON_APPLY] = applyFunction;
                }

                menuItem->ApplyAction((i == 0) ? ON_HIGHLIGHT : ON_UNHIGHLIGHT);
            }
            else
            {
                if (i == 0)
                {
                    DoubleHighlite::OpenItem(compinstance);
                }
                else
                {
                    DoubleHighlite::CloseItem(compinstance);
                }
            }

            TLSlide* slide = compinstance->GetActiveSlide();
            compinstance->Update(slide->m_start + slide->m_duration);
        }
        else
        {
            DoubleHighlite::CloseItem(compinstance);

            TLSlide* slide = compinstance->GetActiveSlide();
            compinstance->Update(slide->m_start + slide->m_duration);
            compinstance->m_bVisible = false;
        }
    }

    if (mCurrentState == STATE_SCROLLING)
    {
        mComponents[0] = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("CAPTAIN_NAME_RIGHT")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        mComponents[0]->m_bVisible = false;

        mComponents[1] = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("CAPTAIN_CHOOSER_LEFT")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        mComponents[1]->m_bVisible = false;

        mComponents[2] = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("CHOOSE_SIDEKICKS_LEFT")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        mComponents[2]->m_bVisible = false;

        TLComponentInstance* chooserComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            mComponents[1]->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("HIGHLIGHT")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        chooserComp->m_bVisible = false;

        chooserComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            mComponents[2]->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("HIGHLIGHT")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        chooserComp->m_bVisible = false;

        UpdateCaptainName();

        mCaptainGrid = new (nlMalloc(0x1C, 8, false)) ICaptainGridComponent(mComponents[1], false);
        mCaptainGrid->BuildMapMenu();

        mSKGrid = new (nlMalloc(0x1C, 8, false)) ISidekickGridComponent(mComponents[2], false);
        mSKGrid->BuildMapMenu();

        TLComponentInstance* tempComponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("Component")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        TLTextInstance* scrollText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            tempComponent->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("Group")),
            InlineHasher(nlStringLowerHash("TickerText")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        extern void* glGetScreenInfo();
        int screenWidth = *(int*)glGetScreenInfo();
        mTicker = new (nlMalloc(0x22C, 0x20, true)) FEScrollText(scrollText, 0, screenWidth + 0x32);
        mTicker->SetDisplayMessage("CHOOSE_CAPTAIN_CUSTOM_TOURNAMENT");

        mPressStartComponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEScene->m_pFEPackage->GetPresentation()->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("START TO CONTINUE")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        mPressStartComponent->m_bVisible = false;
    }
    else
    {
        presentation->SetActiveSlide("CHANGER");

        mButtons1.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("buttons")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        mButtons1.SetState((ButtonComponent::ButtonState)3);

        presentation->SetActiveSlide("MENU IN");

        mButtons2.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("buttons")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        mButtons2.SetState((ButtonComponent::ButtonState)3);
    }

    TLComponentInstance* tempComponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ARROWS")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (mTournInfo.m_numTeams < 5)
    {
        tempComponent->m_bVisible = false;
    }

    mUpArrow = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        tempComponent->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("arrow2")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mDownArrow = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        tempComponent->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("arrow")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (mCurrentRow == 0)
    {
        mUpArrow->m_bVisible = false;
        mDownArrow->m_bVisible = true;
    }
    else if (mCurrentRow == (int)mTournInfo.m_numTeams - 1)
    {
        mUpArrow->m_bVisible = true;
        mDownArrow->m_bVisible = false;
    }
    else
    {
        mUpArrow->m_bVisible = true;
        mDownArrow->m_bVisible = true;
    }

    FEAudio::EnableSounds(true);
}

/**
 * Offset/Address/Size: 0x32F4 | 0x800E5198 | size: 0xBE0
 */
void TournTeamSetupSceneV2::Update(float fDeltaT)
{
    static const char* lastCaptainSelectSoundStrPlayed;
    static signed char init;

    BaseSceneHandler::Update(fDeltaT);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    if (!init)
    {
        lastCaptainSelectSoundStrPlayed = NULL;
        init = 1;
    }

    switch (mCurrentState)
    {
    case STATE_IN:
        mButtons1.SetState(ButtonComponent::BS_A_AND_B_AND_Y);
        mButtons1.CentreButtons();
        mButtons2.SetState(ButtonComponent::BS_A_AND_B_AND_Y);
        mButtons2.CentreButtons();
        break;

    case STATE_SCROLLING:
    {
        bool hasEmpty = false;
        for (int i = 0; i < mTournInfo.m_numTeams; i++)
        {
            if (mTeamData[i].isEmpty)
            {
                hasEmpty = true;
                break;
            }
        }

        if (hasEmpty)
        {
            mButtons1.SetState(ButtonComponent::BS_A_AND_B_AND_Y);
            mButtons2.SetState(ButtonComponent::BS_A_AND_B_AND_Y);
        }
        else
        {
            int numHumans = 0;
            int result = 0;

            for (int i = 0; i < mTournInfo.m_numTeams; i++)
            {
                if (mTeamData[i].isEmpty)
                {
                    result = -1;
                    break;
                }
                if (mTeamData[i].isHumanPlayer)
                {
                    numHumans++;
                }
            }

            if (result != -1)
            {
                result = (numHumans < 1) ? -2 : 1;
            }

            if (result == 1)
            {
                mButtons1.SetState((ButtonComponent::ButtonState)4);
                mButtons2.SetState((ButtonComponent::ButtonState)4);
            }
            else
            {
                mButtons1.SetState(ButtonComponent::BS_A_AND_B);
                mButtons2.SetState(ButtonComponent::BS_A_AND_B);
            }
        }

        mButtons1.CentreButtons();
        mButtons2.CentreButtons();
        break;
    }

    case STATE_CAPTAIN:
    case STATE_SIDEKICK:
        mButtons1.SetState(ButtonComponent::BS_A_AND_B);
        mButtons2.SetState(ButtonComponent::BS_A_AND_B);
        mButtons1.CentreButtons();
        mButtons2.CentreButtons();
        break;

    default:
        break;
    }

    if (mCurrentState == STATE_IN)
    {
        TLSlide* slide = presentation->m_currentSlide;
        if (presentation->m_fadeDuration >= (slide->m_start + slide->m_duration))
        {
            mCurrentState = STATE_SCROLLING;
            OnActivate();
        }
        else
        {
            return;
        }
    }

    TLComponentInstance* pTickerComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Component")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    TLSlide* tickerSlide = pTickerComp->GetActiveSlide();
    float endTime = tickerSlide->m_start + tickerSlide->m_duration;
    if (tickerSlide->m_time >= endTime)
    {
        mTicker->Update(fDeltaT);
    }

    if (mCurrentState == STATE_SCROLLING)
    {
        presentation->SetActiveSlide(presentation->m_currentSlide);
        presentation->Update(0.0f);

        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
        {
            MenuItem<TLComponentInstance>& item = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex];
            if (item.mCallbacks[ON_APPLY].mTag != EMPTY && !item.mDisabled)
            {
                item.mCallbacks[ON_APPLY](item.mType);
            }
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        }
        else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_OPTIONS_INITIAL_TOURN, SCREEN_BACK, true);
            FEAudio::PlayAnimAudioEvent("sfx_back", false);
        }
        else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x800, false, NULL))
        {
            AutoFill();

            int result = 0;
            int numHumans = 0;
            for (int i = 0; i < mTournInfo.m_numTeams; i++)
            {
                if (mTeamData[i].isEmpty)
                {
                    result = -1;
                    break;
                }
                if (mTeamData[i].isHumanPlayer)
                {
                    numHumans++;
                }
            }

            if (result != -1)
            {
                result = (numHumans < 1) ? -2 : 1;
            }

            mPressStartComponent->m_bVisible = (result == 1);
        }
        else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
        {
            ScrollUp(true);
        }
        else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
        {
            ScrollDown(true);
        }
        else
        {
            bool left = g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xB, true, NULL);
            bool right = false;

            if (!left)
            {
                right = g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xC, true, NULL);
            }

            if (left || right)
            {
                TeamData* data = &mTeamData[mCurrentRow];
                if (!data->isEmpty)
                {
                    data->isHumanPlayer = !data->isHumanPlayer;
                    UpdateControllerIcon(mCurrentRow);

                    int result = 0;
                    int numHumans = 0;
                    for (int i = 0; i < mTournInfo.m_numTeams; i++)
                    {
                        if (mTeamData[i].isEmpty)
                        {
                            result = -1;
                            break;
                        }
                        if (mTeamData[i].isHumanPlayer)
                        {
                            numHumans++;
                        }
                    }

                    if (result != -1)
                    {
                        result = (numHumans < 1) ? -2 : 1;
                    }
                    mPressStartComponent->m_bVisible = (result == 1);

                    if (left)
                    {
                        FEAudio::PlayAnimAudioEvent("sfx_option_scroll_left", false);
                    }
                    else
                    {
                        FEAudio::PlayAnimAudioEvent("sfx_option_scroll_right", false);
                    }
                }
            }
            else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x1000, false, NULL))
            {
                int result = 0;
                int numHumans = 0;
                for (int i = 0; i < mTournInfo.m_numTeams; i++)
                {
                    if (mTeamData[i].isEmpty)
                    {
                        result = -1;
                        break;
                    }
                    if (mTeamData[i].isHumanPlayer)
                    {
                        numHumans++;
                    }
                }

                if (result != -1)
                {
                    result = (numHumans < 1) ? -2 : 1;
                }

                if (result == 1)
                {
                    Proceed();
                    FEAudio::PlayAnimAudioEvent("sfx_accept", false);
                }
                else if (result == -1)
                {
                    nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);
                }
                else if (result == -2)
                {
                    nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);
                }
            }
        }

        if (mCurrentRow == 0)
        {
            mUpArrow->m_bVisible = false;
            mDownArrow->m_bVisible = true;
        }
        else if (mCurrentRow == (int)mTournInfo.m_numTeams - 1)
        {
            mUpArrow->m_bVisible = true;
            mDownArrow->m_bVisible = false;
        }
        else
        {
            mUpArrow->m_bVisible = true;
            mDownArrow->m_bVisible = true;
        }
    }
    else if (mCurrentState == STATE_CAPTAIN)
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
        {
            if (mCurrentCaptain == TEAM_MYSTERY)
            {
                mTeamData[mCurrentRow].captain = mCurrentCaptain;
                mTeamData[mCurrentRow].sidekick = mCurrentSK;
                mTeamData[mCurrentRow].isEmpty = false;
                mCaptainGrid->SetValid(mCurrentCaptain, false);
                mTeamData[mCurrentRow].isHumanPlayer = true;
                UpdateRow(mCurrentRow);
                ChangeState(mCurrentState, STATE_SCROLLING);
                lastCaptainSelectSoundStrPlayed = FECharacterSound::PlayCaptainName(mCurrentCaptain);
                ScrollDown(false);
            }
            else
            {
                ChangeState(mCurrentState, STATE_SIDEKICK);
                lastCaptainSelectSoundStrPlayed = FECharacterSound::PlayCaptainName(mCurrentCaptain);
            }
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        }
        else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            if (!mTeamData[mCurrentRow].isEmpty)
            {
                mCaptainGrid->SetValid(mTeamData[mCurrentRow].captain, false);
            }
            ChangeState(mCurrentState, STATE_SCROLLING);
            mCurrentCaptain = mTeamData[mCurrentRow].captain;
            mCaptainGrid->MoveHighlightToTarget(mCurrentCaptain);
            UpdateCaptainName();
            FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
        }
        else
        {
            mCaptainGrid->Update(FE_ALL_PADS);
            if (mCaptainGrid->mHasChangedSinceLastUpdate)
            {
                mCurrentCaptain = mCaptainGrid->GetSelectedItem();
                mComponents[0]->SetActiveSlide("Slide1");
                UpdateCaptainName();
                mComponents[0]->Update(0.0f);
            }
        }
    }
    else if (mCurrentState == STATE_SIDEKICK)
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
        {
            mTeamData[mCurrentRow].captain = mCurrentCaptain;
            mTeamData[mCurrentRow].sidekick = mCurrentSK;
            mTeamData[mCurrentRow].isEmpty = false;
            mCaptainGrid->SetValid(mCurrentCaptain, false);
            mTeamData[mCurrentRow].isHumanPlayer = true;
            UpdateRow(mCurrentRow);
            ChangeState(mCurrentState, STATE_SCROLLING);
            FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
            FEAudio::PlayAnimAudioEvent("sfx_character_group_left_exit", false);
            if (lastCaptainSelectSoundStrPlayed != NULL)
            {
                FEAudio::StopAnimAudioEvent(lastCaptainSelectSoundStrPlayed);
                lastCaptainSelectSoundStrPlayed = NULL;
            }
            FECharacterSound::PlaySidekickName(mCurrentSK);
            ScrollDown(false);
        }
        else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            ChangeState(mCurrentState, STATE_CAPTAIN);
            mCurrentSK = mTeamData[mCurrentRow].sidekick;
            mSKGrid->MoveHighlightToTarget(mCurrentSK);
            FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
        }
        else
        {
            mSKGrid->Update(FE_ALL_PADS);
            if (mSKGrid->mHasChangedSinceLastUpdate)
            {
                mCurrentSK = mSKGrid->GetSelectedItem();
                mComponents[0]->SetActiveSlide("Slide2");
                UpdateSKName();
                mComponents[0]->Update(0.0f);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x2FB4 | 0x800E4E58 | size: 0x340
 */
void TournTeamSetupSceneV2::UpdateControllerIcon(int arg)
{
    typedef TLComponentInstance* (*FindCompByValue)(
        TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(
        TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    int nlSNPrintf(char*, unsigned long, const char*, ...);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    int row = arg + mRowOffset;

    volatile InlineHasher hA, hB;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher innerHash;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    volatile InlineHasher eB, eA;
    volatile InlineHasher e4, e3, e2, e1, e0;

    volatile InlineHasher dB, dA;
    volatile InlineHasher d4, d3, d2, d1, d0;

    char menuName[68];

    nlSNPrintf(menuName, 64, "MENU ITEM%d", arg + 1);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    unsigned long hash = nlStringLowerHash(menuName);
    h8.m_Hash = hash;
    h9.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* item = findComp.byRef(
        presentation->m_currentSlide,
        (InlineHasher&)hA,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    item->SetActiveSlide("IN");
    TLSlide* active = item->GetActiveSlide();
    item->Update(active->m_start + active->m_duration);

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("CONTROLLER");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* controllerA = findComp.byRef(
        item->GetActiveSlide(),
        (InlineHasher&)gB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    item->SetActiveSlide("OUT");
    active = item->GetActiveSlide();
    item->Update(active->m_start + active->m_duration);

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("CONTROLLER");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* controllerB = findComp.byRef(
        item->GetActiveSlide(),
        (InlineHasher&)fB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    item->SetActiveSlide("IN");
    active = item->GetActiveSlide();
    item->Update(active->m_start + active->m_duration);

    if (mTeamData[row].isHumanPlayer)
    {
        controllerA->SetActiveSlide("CONTROLLER");
        controllerB->SetActiveSlide("CONTROLLER");
    }
    else
    {
        controllerA->SetActiveSlide("CPU");
        controllerB->SetActiveSlide("CPU");
    }

    e0.m_Hash = 0;
    h1.m_Hash = 0;
    e1.m_Hash = 0;
    h3.m_Hash = 0;
    e2.m_Hash = 0;
    h5.m_Hash = 0;
    e3.m_Hash = 0;
    h7.m_Hash = 0;
    e4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("arrows");
    eA.m_Hash = hash;
    eB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* arrowsA = findComp.byRef(
        controllerA->GetActiveSlide(),
        (InlineHasher&)eB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    d0.m_Hash = 0;
    h1.m_Hash = 0;
    d1.m_Hash = 0;
    h3.m_Hash = 0;
    d2.m_Hash = 0;
    h5.m_Hash = 0;
    d3.m_Hash = 0;
    h7.m_Hash = 0;
    d4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("arrows");
    dA.m_Hash = hash;
    dB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* arrowsB = findComp.byRef(
        controllerB->GetActiveSlide(),
        (InlineHasher&)dB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    arrowsA->m_bVisible = true;
    arrowsB->m_bVisible = true;
}

/**
 * Offset/Address/Size: 0x2500 | 0x800E43A4 | size: 0xAB4
 * TODO: 91.76% match - remaining diffs are in InlineHasher stack-slot ordering
 * and literal symbol placement across repeated finder setup blocks.
 */
void TournTeamSetupSceneV2::UpdateRow(int onScreenRow)
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(
        TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    typedef TLComponentInstance* (*FindCompByValue)(
        TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(
        TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    int nlSNPrintf(char*, unsigned long, const char*, ...);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    int currentRow = onScreenRow + mRowOffset;

    char menuName[64];

    TLComponentInstance* pComp;
    TLTextInstance* pText1;
    TLComponentInstance* pCaptainComp1;
    TLComponentInstance* pSidekickComp1;
    TLComponentInstance* controller1;
    TLTextInstance* pNameText1;

    TLTextInstance* pText2;
    TLComponentInstance* pCaptainComp2;
    TLComponentInstance* pSidekickComp2;
    TLComponentInstance* controller2;
    TLTextInstance* pNameText2;

    TLComponentInstance* arrow1;
    TLComponentInstance* arrow2;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher g4, g3, g2, g1, g0;
    volatile InlineHasher innerHash;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    volatile InlineHasher eB, eA;
    volatile InlineHasher e4, e3, e2, e1, e0;

    volatile InlineHasher dB, dA;
    volatile InlineHasher d4, d3, d2, d1, d0;

    volatile InlineHasher cB, cA;
    volatile InlineHasher c4, c3, c2, c1, c0;

    volatile InlineHasher bB, bA;
    volatile InlineHasher b4, b3, b2, b1, b0;

    volatile InlineHasher aB, aA;
    volatile InlineHasher a4, a3, a2, a1, a0;

    volatile InlineHasher zB, zA;
    volatile InlineHasher z4, z3, z2, z1, z0;

    volatile InlineHasher yB, yA;
    volatile InlineHasher y4, y3, y2, y1, y0;

    volatile InlineHasher xB, xA;
    volatile InlineHasher x4, x3, x2, x1, x0;

    volatile InlineHasher wB, wA;
    volatile InlineHasher w4, w3, w2, w1, w0;

    volatile InlineHasher vB, vA;
    volatile InlineHasher v4, v3, v2, v1, v0;

    nlSNPrintf(menuName, 64, "MENU ITEM%d", onScreenRow + 1);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    unsigned long hash = nlStringLowerHash(menuName);
    h8.m_Hash = hash;
    h9.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    pComp = findComp.byRef(
        presentation->m_currentSlide,
        (InlineHasher&)hA,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    pComp->SetActiveSlide("IN");
    TLSlide* active = pComp->GetActiveSlide();
    pComp->Update(active->m_start + active->m_duration);

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("NUMBER");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    pText1 = findText.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)gB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("captain heads");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    pCaptainComp1 = findComp.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)fB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    e0.m_Hash = 0;
    h1.m_Hash = 0;
    e1.m_Hash = 0;
    h3.m_Hash = 0;
    e2.m_Hash = 0;
    h5.m_Hash = 0;
    e3.m_Hash = 0;
    h7.m_Hash = 0;
    e4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("sidekick heads");
    eA.m_Hash = hash;
    eB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    pSidekickComp1 = findComp.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)eB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    d0.m_Hash = 0;
    h1.m_Hash = 0;
    d1.m_Hash = 0;
    h3.m_Hash = 0;
    d2.m_Hash = 0;
    h5.m_Hash = 0;
    d3.m_Hash = 0;
    h7.m_Hash = 0;
    d4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("CONTROLLER");
    dA.m_Hash = hash;
    dB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    controller1 = findComp.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)dB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    c0.m_Hash = 0;
    h1.m_Hash = 0;
    c1.m_Hash = 0;
    h3.m_Hash = 0;
    c2.m_Hash = 0;
    h5.m_Hash = 0;
    c3.m_Hash = 0;
    h7.m_Hash = 0;
    c4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("pauseresume");
    cA.m_Hash = hash;
    cB.m_Hash = hash;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    pNameText1 = findText.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)cB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    pComp->SetActiveSlide("OUT");
    active = pComp->GetActiveSlide();
    pComp->Update(active->m_start + active->m_duration);

    b0.m_Hash = 0;
    h1.m_Hash = 0;
    b1.m_Hash = 0;
    h3.m_Hash = 0;
    b2.m_Hash = 0;
    h5.m_Hash = 0;
    b3.m_Hash = 0;
    h7.m_Hash = 0;
    b4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("NUMBER");
    bA.m_Hash = hash;
    bB.m_Hash = hash;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    pText2 = findText.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)bB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    a0.m_Hash = 0;
    h1.m_Hash = 0;
    a1.m_Hash = 0;
    h3.m_Hash = 0;
    a2.m_Hash = 0;
    h5.m_Hash = 0;
    a3.m_Hash = 0;
    h7.m_Hash = 0;
    a4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("captain heads");
    aA.m_Hash = hash;
    aB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    pCaptainComp2 = findComp.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)aB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    z0.m_Hash = 0;
    h1.m_Hash = 0;
    z1.m_Hash = 0;
    h3.m_Hash = 0;
    z2.m_Hash = 0;
    h5.m_Hash = 0;
    z3.m_Hash = 0;
    h7.m_Hash = 0;
    z4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("sidekick heads");
    zA.m_Hash = hash;
    zB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    pSidekickComp2 = findComp.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)zB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    y0.m_Hash = 0;
    h1.m_Hash = 0;
    y1.m_Hash = 0;
    h3.m_Hash = 0;
    y2.m_Hash = 0;
    h5.m_Hash = 0;
    y3.m_Hash = 0;
    h7.m_Hash = 0;
    y4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("CONTROLLER");
    yA.m_Hash = hash;
    yB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    controller2 = findComp.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)yB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    x0.m_Hash = 0;
    h1.m_Hash = 0;
    x1.m_Hash = 0;
    h3.m_Hash = 0;
    x2.m_Hash = 0;
    h5.m_Hash = 0;
    x3.m_Hash = 0;
    h7.m_Hash = 0;
    x4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("pauseresume");
    xA.m_Hash = hash;
    xB.m_Hash = hash;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    pNameText2 = findText.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)xB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mTeamData[currentRow].isHumanPlayer)
    {
        controller1->SetActiveSlide("CONTROLLER");
        controller2->SetActiveSlide("CONTROLLER");
    }
    else
    {
        controller1->SetActiveSlide("CPU");
        controller2->SetActiveSlide("CPU");
    }

    w0.m_Hash = 0;
    h1.m_Hash = 0;
    w1.m_Hash = 0;
    h3.m_Hash = 0;
    w2.m_Hash = 0;
    h5.m_Hash = 0;
    w3.m_Hash = 0;
    h7.m_Hash = 0;
    w4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("arrows");
    wA.m_Hash = hash;
    wB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    arrow1 = findComp.byRef(
        controller1->GetActiveSlide(),
        (InlineHasher&)wB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    v0.m_Hash = 0;
    h1.m_Hash = 0;
    v1.m_Hash = 0;
    h3.m_Hash = 0;
    v2.m_Hash = 0;
    h5.m_Hash = 0;
    v3.m_Hash = 0;
    h7.m_Hash = 0;
    v4.m_Hash = 0;
    innerHash.m_Hash = 0;

    hash = nlStringLowerHash("arrows");
    vA.m_Hash = hash;
    vB.m_Hash = hash;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    arrow2 = findComp.byRef(
        controller2->GetActiveSlide(),
        (InlineHasher&)vB,
        (InlineHasher&)innerHash,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (onScreenRow == (mCurrentRow - mRowOffset))
    {
        pComp->SetActiveSlide("IN");
        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].ApplyAction(ON_HIGHLIGHT);
        arrow1->m_bVisible = true;
        arrow2->m_bVisible = true;
    }
    else
    {
        arrow1->m_bVisible = false;
        arrow2->m_bVisible = false;
    }

    char numIDName[13] = "TOURNAMENT_?";
    numIDName[11] = (char)(currentRow + '1');

    pText1->SetStringId(numIDName);
    pText2->SetStringId(numIDName);

    if (mTeamData[currentRow].isEmpty)
    {
        pCaptainComp1->m_bVisible = false;
        pCaptainComp2->m_bVisible = false;
        pSidekickComp1->m_bVisible = false;
        pSidekickComp2->m_bVisible = false;

        pNameText1->SetStringId("FIELD_EMPTY");
        pNameText2->SetStringId("FIELD_EMPTY");

        controller1->m_bVisible = false;
        controller2->m_bVisible = false;
    }
    else
    {
        pCaptainComp1->m_bVisible = true;
        pCaptainComp2->m_bVisible = true;

        BasicString<char, Detail::TempStringAllocator> captainSlide = FindCaptainSlideName(mTeamData[currentRow].captain);
        pCaptainComp1->SetActiveSlide(captainSlide.c_str());
        pCaptainComp2->SetActiveSlide(captainSlide.c_str());

        controller1->m_bVisible = true;
        controller2->m_bVisible = true;

        if (mTeamData[currentRow].captain != TEAM_MYSTERY)
        {
            pSidekickComp1->m_bVisible = true;
            pSidekickComp2->m_bVisible = true;

            BasicString<char, Detail::TempStringAllocator> sidekickSlide = FindSidekickSlideName(mTeamData[currentRow].sidekick);
            pSidekickComp1->SetActiveSlide(sidekickSlide.c_str());
            pSidekickComp2->SetActiveSlide(sidekickSlide.c_str());
        }
        else
        {
            pSidekickComp1->m_bVisible = false;
            pSidekickComp2->m_bVisible = false;
        }

        pNameText1->m_LocStrId = GetLOCCharacterName(mTeamData[currentRow].captain, false, false);
        pNameText1->m_OverloadFlags |= 0x8;

        pNameText2->m_LocStrId = GetLOCCharacterName(mTeamData[currentRow].captain, false, false);
        pNameText2->m_OverloadFlags |= 0x8;
    }

    if (mTeamData[currentRow].isHumanPlayer)
    {
        controller1->SetActiveSlide("CONTROLLER");
        controller2->SetActiveSlide("CONTROLLER");
    }
    else
    {
        controller1->SetActiveSlide("CPU");
        controller2->SetActiveSlide("CPU");
    }
}

/**
 * Offset/Address/Size: 0x1F30 | 0x800E3DD4 | size: 0x5D0
 */
void TournTeamSetupSceneV2::ChangeState(TournTeamSetupSceneV2::eTeamChooserState, TournTeamSetupSceneV2::eTeamChooserState)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x1EC4 | 0x800E3D68 | size: 0x6C
 */
void TournTeamSetupSceneV2::StartChooseCaptain(int arg)
{
    mCurrentRow = arg + mRowOffset;

    if (!mTeamData[mCurrentRow].isEmpty)
    {
        mCaptainGrid->SetValid(mTeamData[mCurrentRow].captain, true);
    }

    ChangeState(STATE_SCROLLING, STATE_CAPTAIN);
}

/**
 * Offset/Address/Size: 0x1CC4 | 0x800E3B68 | size: 0x200
 * TODO: 87.3% match - stack frame size 0xB0 vs target 0x80, all diffs are stack
 * offset (s/i) only. -inline deferred file causes different InlineHasher temporary
 * stack allocation on decomp.me vs original build.
 */
void TournTeamSetupSceneV2::UpdateCaptainName()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    TLSlide* slide = mComponents[0]->GetActiveSlide();

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash("CAPTAIN_NAME");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLTextInstance* captainNameText = findText.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    captainNameText->m_LocStrId = GetLOCCharacterName(mCurrentCaptain, false, true);
    captainNameText->m_OverloadFlags |= 0x8;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("CAPTAIN_NAME2");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    TLTextInstance* captainDesc = findText.byRef(
        slide,
        (InlineHasher&)gB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mCurrentCaptain == TEAM_MYSTERY && !nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
    {
        captainDesc->SetStringId("CUP_ATTR_MYSTERY_LOCKED");
    }
    else
    {
        captainDesc->m_LocStrId = TOURN_CAPTAIN_DESCRIPTIONS[mCurrentCaptain];
        captainDesc->m_OverloadFlags |= 0x8;
    }

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("COMPONENT");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    TLComponentInstance* comp = findComp.byRef(
        slide,
        (InlineHasher&)fB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    comp->SetActiveSlide(GetTeamName(mCurrentCaptain));
}

/**
 * Offset/Address/Size: 0x1A48 | 0x800E38EC | size: 0x27C
 */
void TournTeamSetupSceneV2::UpdateSKName()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher iB, iA;
    volatile InlineHasher i4, i3, i2, i1, i0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    TLSlide* slide = mComponents[0]->GetActiveSlide();

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash("CAPTAIN_NAME");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLTextInstance* captainNameText = findText.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    captainNameText->m_LocStrId = GetLOCCharacterName(mCurrentCaptain, false, false);
    captainNameText->m_OverloadFlags |= 0x8;

    i0.m_Hash = 0;
    h1.m_Hash = 0;
    i1.m_Hash = 0;
    h3.m_Hash = 0;
    i2.m_Hash = 0;
    h5.m_Hash = 0;
    i3.m_Hash = 0;
    h7.m_Hash = 0;
    i4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("SIDEKICK_NAME");
    iA.m_Hash = hash;
    iB.m_Hash = hash;

    TLTextInstance* sidekickNameText = findText.byRef(
        slide,
        (InlineHasher&)iB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    sidekickNameText->m_LocStrId = GetLOCSidekickName(mCurrentSK);
    sidekickNameText->m_OverloadFlags |= 0x8;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("CAPTAIN_NAME2");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    TLTextInstance* captainDescText = findText.byRef(
        slide,
        (InlineHasher&)gB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mCurrentCaptain == TEAM_MYSTERY && !nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
    {
        captainDescText->SetStringId("CUP_ATTR_MYSTERY_LOCKED");
    }
    else
    {
        captainDescText->m_LocStrId = TOURN_CAPTAIN_DESCRIPTIONS[mCurrentCaptain];
        captainDescText->m_OverloadFlags |= 0x8;
    }

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("COMPONENT");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    TLComponentInstance* teamComp = findComp.byRef(
        slide,
        (InlineHasher&)fB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    teamComp->SetActiveSlide(GetTeamName(mCurrentCaptain));
}

/**
 * Offset/Address/Size: 0x15F4 | 0x800E3498 | size: 0x454
 */
void TournTeamSetupSceneV2::Proceed()
{
    GameInfoManager* pGameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    int i = 0;

    mTournInfo.m_cup->mHumanTeams = 0;

    for (; i < mTournInfo.m_numTeams; i++)
    {
        if (mTeamData[i].isHumanPlayer)
        {
            mTournInfo.m_cup->mHumanTeams = (u16)(mTournInfo.m_cup->mHumanTeams | (1 << mTeamData[i].captain));
        }
    }

    pGameInfo->SetMode(GameInfoManager::GM_TOURNAMENT);
    nlSingleton<GameInfoManager>::s_pInstance->GetCurrentRoundNumber();

    if (mTournInfo.m_tournMode == TM_LEAGUE)
    {
        GameInfoManager* const pTournamentInfo = nlSingleton<GameInfoManager>::s_pInstance;
        u16 numPlayingTeams = pTournamentInfo->GetNumPlayingTeams();

        eSidekickID sklineup[8];
        eTeamID lineup[8];

        int idx = 0;
        for (; idx < numPlayingTeams; idx++)
        {
            lineup[idx] = mTeamData[idx].captain;
            sklineup[idx] = mTeamData[idx].sidekick;
        }

        pTournamentInfo->SetupRoundRobinSchedule(lineup, sklineup);
        pGameInfo->SetPreviousTeamStats();
        pGameInfo->IncreaseRoundNumber();
    }
    else
    {
        BaseCup* pCup = pGameInfo->mCurrentCup;

        pCup->mRoundNumber = -4;

        if (pCup->GetNumRounds() == 2)
        {
            pCup->mRoundNumber = -3;
        }

        GameInfoManager* const pTournamentInfo = nlSingleton<GameInfoManager>::s_pInstance;
        u16 numPlayingTeams = pTournamentInfo->GetNumPlayingTeams();

        eSidekickID sklineup[8];
        eTeamID lineup[8];

        int idx = 0;
        for (; idx < numPlayingTeams; idx++)
        {
            lineup[idx] = mTeamData[idx].captain;
            sklineup[idx] = mTeamData[idx].sidekick;
        }

        pTournamentInfo->SetupTournamentKnockout(lineup, sklineup);
    }

    while (pGameInfo->GetCurrentRoundNumber() != -5)
    {
        if (pGameInfo->DetermineNextMatchups(0x1B))
        {
            break;
        }

        pGameInfo->IncreaseRoundNumber();
    }

    pGameInfo->SetResultsOfLastUserGame(RESULT_CUP_START);

    nlSingleton<GameSceneManager>::s_pInstance->Pop();

    if (pGameInfo->GetNumHumanTeams() == 1)
    {
        int j = 0;

        for (; j < mTournInfo.m_numTeams; j++)
        {
            if (mTeamData[j].isHumanPlayer)
            {
                pGameInfo->SetUserSelectedCupTeam(mTeamData[j].captain);
                break;
            }
        }
    }

    GameSceneManager* pSceneMgr = nlSingleton<GameSceneManager>::s_pInstance;
    SceneList nextScene = SCENE_TOURNAMENT_STANDINGS;

    if (mTournInfo.m_tournMode == TM_LEAGUE)
    {
        nextScene = SCENE_TOURNAMENT_STANDINGS_ANIM;
    }

    CupHubScene* pHubScene = (CupHubScene*)pSceneMgr->Push(nextScene, SCREEN_FORWARD, false);
    pHubScene->mDoAutoSave = true;
}

/**
 * Offset/Address/Size: 0xC34 | 0x800E2AD8 | size: 0x9C0
 * TODO: 96.09% match - string literal hoisted before nlMalloc in inlined BasicString constructor
 */
#pragma optimization_level 2
BasicString<char, Detail::TempStringAllocator> TournTeamSetupSceneV2::FindCaptainSlideName(eTeamID captain)
{
    BasicString<char, Detail::TempStringAllocator> returnValue;

    switch (captain)
    {
    case TEAM_DAISY:
        returnValue = "daisy";
        break;
    case TEAM_DONKEYKONG:
        returnValue = "dk";
        break;
    case TEAM_LUIGI:
        returnValue = "luigi";
        break;
    case TEAM_MARIO:
        returnValue = "mario";
        break;
    case TEAM_PEACH:
        returnValue = "peach";
        break;
    case TEAM_WALUIGI:
        returnValue = "waluigi";
        break;
    case TEAM_WARIO:
        returnValue = "wario";
        break;
    case TEAM_YOSHI:
        returnValue = "yoshi";
        break;
    case TEAM_MYSTERY:
        returnValue = "super";
        break;
    }

    return returnValue;
}
#pragma optimization_level 4

/**
 * Offset/Address/Size: 0x76C | 0x800E2610 | size: 0x4C8
 * TODO: 93.1% match - r29/r30 register swap: MWCC hoists string literal address
 * before nlMalloc in inlined BasicString constructor, swapping data/str registers
 */
BasicString<char, Detail::TempStringAllocator> TournTeamSetupSceneV2::FindSidekickSlideName(eSidekickID sidekick)
{
    BasicString<char, Detail::TempStringAllocator> returnValue;

    switch (sidekick)
    {
    case SK_TOAD:
        returnValue = BasicString<char, Detail::TempStringAllocator>("toad");
        break;
    case SK_KOOPA:
        returnValue = BasicString<char, Detail::TempStringAllocator>("koopa");
        break;
    case SK_HAMMERBROS:
        returnValue = BasicString<char, Detail::TempStringAllocator>("hammer");
        break;
    case SK_BIRDO:
        returnValue = BasicString<char, Detail::TempStringAllocator>("birdo");
        break;
    }

    return returnValue;
}

/**
 * Offset/Address/Size: 0x4B0 | 0x800E2354 | size: 0x2BC
 */
void TournTeamSetupSceneV2::AutoFill()
{
    bool changed = false;
    int i = 0;

    for (; i < mTournInfo.m_numTeams; i++)
    {
        if (mTeamData[i].isEmpty)
        {
            eTeamID randCapt;
            u8 alreadySelected;

            do
            {
                if (nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
                {
                    randCapt = (eTeamID)nlRandom(9, &nlDefaultSeed);
                }
                else
                {
                    randCapt = (eTeamID)nlRandom(8, &nlDefaultSeed);
                }

                for (int k = 0; k < mTournInfo.m_numTeams; k++)
                {
                    if (!mTeamData[k].isEmpty && mTeamData[k].captain == randCapt)
                    {
                        alreadySelected = 1;
                        goto done_select_check;
                    }
                }

                alreadySelected = 0;
            done_select_check:;
            } while (alreadySelected);

            if (!(*(u8*)((u8*)this + 0x334)))
            {
                mTeamData[mCurrentRow].isHumanPlayer = true;
                *(u8*)((u8*)this + 0x334) = true;
            }

            mTeamData[i].isEmpty = false;
            mTeamData[i].sidekick = (eSidekickID)nlRandom(4, &nlDefaultSeed);
            mTeamData[i].captain = randCapt;
            mCaptainGrid->SetValid(randCapt, false);
            changed = true;
        }
    }

    if (changed)
    {
        int numRows = mTournInfo.m_numTeams == 3 ? 3 : 4;
        mRowOffset = mTournInfo.m_numTeams - numRows;

        {
            int currentIndex = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) != 0)
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

        mMenuItems.mCurrentIndex = numRows - 1;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) != 0)
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

        mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
        mCurrentCaptain = mTeamData[mCurrentRow].captain;
        mCurrentSK = mTeamData[mCurrentRow].sidekick;

        mCaptainGrid->MoveHighlightToTarget(mCurrentCaptain);
        ((IGridComponent<eTeamID>*)mSKGrid)->MoveHighlightToTarget((eTeamID)mCurrentSK);
        UpdateCaptainName();

        int j = 0;
        int numRows2 = ((u32)(3 - (u32)mTournInfo.m_numTeams) >> 31) + 3;
        for (; j < numRows2; j++)
        {
            UpdateRow(j);
        }
    }
}

/**
 * Offset/Address/Size: 0x3E4 | 0x800E2288 | size: 0xCC
 */
void TournTeamSetupSceneV2::UpdateForCurrentRow()
{
    FORCE_DONT_INLINE;
    int numRows;
    int i = 0;
    numRows = ((u32)(3 - (u32)mTournInfo.m_numTeams) >> 31) + 3;

    for (; i < numRows; i++)
    {
        UpdateRow(i);
    }

    mCurrentCaptain = mTeamData[mCurrentRow].captain;
    mCurrentSK = mTeamData[mCurrentRow].sidekick;

    mCaptainGrid->MoveHighlightToTarget(mCurrentCaptain);
    ((IGridComponent<eTeamID>*)mSKGrid)->MoveHighlightToTarget((eTeamID)mCurrentSK);
    UpdateCaptainName();
}

/**
 * Offset/Address/Size: 0x1FC | 0x800E20A0 | size: 0x1E8
 */
void TournTeamSetupSceneV2::ScrollUp(bool bPlaySound)
{
    int newIndex;
    bool doUpdate;

    if (!bPlaySound)
    {
        FEAudio::EnableSounds(false);
    }

    int flags = mMenuItems.mFlags;
    int skipFlag;
    int wrapFlag;
    int currentIndex;
    wrapFlag = flags & 1;
    skipFlag = flags & 2;
    currentIndex = mMenuItems.mCurrentIndex;
    newIndex = currentIndex - 1;

loop:
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
            newIndex = 2;
            goto end_section;
        }
    }

    if (skipFlag)
    {
        if (mMenuItems.mMenuItems[newIndex].mDisabled)
        {
            newIndex--;
            goto loop;
        }
    }

    // Deselect old item (callback 2)
    {
        int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
        if (((u32)((-tag) | tag) >> 31) != 0)
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

    int selIdx = mMenuItems.mCurrentIndex;
    int tag2 = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
    if (((u32)((-tag2) | tag2) >> 31) != 0)
    {
        TLComponentInstance* type2 = mMenuItems.mMenuItems[selIdx].mType;
        if (tag2 == FREE_FUNCTION)
        {
            mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type2);
        }
        else
        {
            (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type2);
        }
    }

    newIndex = 1;

end_section:
    mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
    FEAudio::EnableSounds(false);

    doUpdate = true;
    if (newIndex == 2)
    {
        if (mRowOffset > 0)
        {
            mRowOffset--;
            mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
            goto check_update;
        }
    }
    if (newIndex == 2)
    {
        doUpdate = false;
        FEAudio::PlayAnimAudioEvent("sfx_deny", false);
    }

check_update:
    if (doUpdate)
    {
        if (newIndex == 2)
        {
            FEAudio::EnableSounds(true);
        }
        UpdateForCurrentRow();
    }

    FEAudio::EnableSounds(true);
}

/**
 * Offset/Address/Size: 0x0 | 0x800E1EA4 | size: 0x1FC
 */
void TournTeamSetupSceneV2::ScrollDown(bool bPlaySound)
{
    int newIndex;
    bool doUpdate;

    if (!bPlaySound)
    {
        FEAudio::EnableSounds(false);
    }

    int flags = mMenuItems.mFlags;
    int skipFlag;
    int wrapFlag;
    int currentIndex;
    wrapFlag = flags & 1;
    skipFlag = flags & 2;
    currentIndex = mMenuItems.mCurrentIndex;
    newIndex = currentIndex + 1;

loop:
    if (wrapFlag)
    {
        newIndex = newIndex % mMenuItems.mNumItemsAdded;
    }
    else
    {
        if (newIndex >= mMenuItems.mNumItemsAdded)
        {
            newIndex = 2;
            goto end_section;
        }
    }

    if (skipFlag)
    {
        if (mMenuItems.mMenuItems[newIndex].mDisabled)
        {
            newIndex++;
            goto loop;
        }
    }

    // Deselect old item (callback 2)
    {
        int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
        if (((u32)((-tag) | tag) >> 31) != 0)
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

    // Select new item (callback 1)
    {
        int selIdx = mMenuItems.mCurrentIndex;
        int tag2 = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
        if (((u32)((-tag2) | tag2) >> 31) != 0)
        {
            TLComponentInstance* type2 = mMenuItems.mMenuItems[selIdx].mType;
            if (tag2 == FREE_FUNCTION)
            {
                mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type2);
            }
            else
            {
                (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type2);
            }
        }
    }

    newIndex = 1;

end_section:
    mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
    FEAudio::EnableSounds(false);

    doUpdate = true;
    if (newIndex == 2)
    {
        if (mRowOffset + 3 < (int)mTournInfo.m_numTeams - 1)
        {
            mRowOffset++;
            mCurrentRow = mRowOffset + mMenuItems.mCurrentIndex;
            goto check_update;
        }
    }
    if (newIndex == 2)
    {
        doUpdate = false;
        FEAudio::PlayAnimAudioEvent("sfx_deny", false);
    }

check_update:
    if (doUpdate)
    {
        if (newIndex == 2)
        {
            FEAudio::EnableSounds(true);
        }
        UpdateForCurrentRow();
    }

    FEAudio::EnableSounds(true);
}

void SHTournTeamSetup_stub()
{
    TLInstance* (*volatile forceFind)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher) = &FEFinder<TLInstance, 4>::Find<TLSlide>;
    (void)forceFind;
}
