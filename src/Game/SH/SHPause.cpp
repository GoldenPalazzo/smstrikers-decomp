#include "Game/SH/SHPause.h"
#include "Game/OverlayManager.h"

#include "Game/FE/FEAudio.h"
#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feManager.h"
#include "Game/FE/feSceneManager.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/GameInfo.h"
#include "Game/Game.h"
#include "NL/glx/glxSwap.h"
#include "NL/nlPrint.h"

extern FEInput* g_pFEInput;
extern nlColour MenuHighliteColour;
extern float mDelayBeforeUnpause__14PauseMenuScene;

namespace DoubleHighlite
{
void OpenItem(TLComponentInstance*);
}

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)())
{
    return Detail::MemFunImpl<R, void (T::*)()>(fn);
}

template <typename T, typename R, typename A>
Detail::MemFunImpl<R, void (T::*)(A)> MemFun(void (T::*fn)(A))
{
    return Detail::MemFunImpl<R, void (T::*)(A)>(fn);
}

// /**
//  * Offset/Address/Size: 0x38 | 0x800B01B4 | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>(
//     Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene* const&, const Placeholder<0>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B017C | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(Detail::MemFunImpl<void, void
// (PauseMenuScene::*)()>,
//                                                                                        PauseMenuScene* const&)
// {
// }

/**
 * Offset/Address/Size: 0x1C | 0x800B0160 | size: 0x1C
 * TODO: 69.29% match - r0/r5 register allocation swap and load/store interleaving
 */
template Detail::MemFunImpl<void, void (PauseMenuScene::*)()>
MemFun<PauseMenuScene, void>(void (PauseMenuScene::*)());

/**
 * Offset/Address/Size: 0x0 | 0x800B0144 | size: 0x1C
 * TODO: 69.29% match - r0/r5 register allocation swap and load/store interleaving
 */
template Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>
MemFun<PauseMenuScene, void, TLComponentInstance*>(void (PauseMenuScene::*)(TLComponentInstance*));

// /**
//  * Offset/Address/Size: 0x5C | 0x800B00E8 | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<
//     void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B008C | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x128 | 0x800B005C | size: 0x30
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::operator()()
// {
// }

// /**
//  * Offset/Address/Size: 0xB0 | 0x800AFFE4 | size: 0x78
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x800AFFB4 | size: 0x30
//  */
// void Function1<void,
//                TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void
//                (PauseMenuScene::*)(TLComponentInstance*)>,
//                                                            PauseMenuScene*, Placeholder<0>>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AFF34 | size: 0x80
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp2<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>>::Clone()
//     const
// {
// }

// /**
//  * Offset/Address/Size: 0x6CC | 0x800AFEFC | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x570 | 0x800AFDA0 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800AFD1C | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800AFC98 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                              unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800AFC60 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                             InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800AFB04 | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                 unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800AFA80 | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                              unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800AFA48 | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800AF8EC | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800AF868 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AF830 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x225C | 0x800AF754 | size: 0xDC
 */
PauseMenuScene::PauseMenuScene(PauseMenuScene::ScreenContext context)
    : BaseSceneHandler()
    , mContext(context)
    , mGameIsOver(false)
    , mQuitDelay(0.0f)
    , mQuittingController(FE_ALL_PADS)
    , mMenuItems()
    , mTransitionTo(TT_IN)
    , mIsInTransition(false)
    , mStartAnimAtEnd(false)
    , mButtons()
    , mButtons2()
{
    mDelayBeforeUnpause__14PauseMenuScene = 0.1f;
}

/**
 * Offset/Address/Size: 0x21AC | 0x800AF6A4 | size: 0xB0
 */
PauseMenuScene::~PauseMenuScene()
{
}

/**
 * Offset/Address/Size: 0x2158 | 0x800AF650 | size: 0x54
 */
void PauseMenuScene::OnSelectRESUME(TLComponentInstance*)
{
    TransitionOut(TT_OUT);
    g_pFEInput->Reset();
    FEAudio::PlayAnimAudioEvent("sfx_screen_back", false);
    FEAudio::PlayAnimAudioEvent("sfx_pause_resume", false);
    mLastSelectedIndex = 0;
}

/**
 * Offset/Address/Size: 0x18CC | 0x800AEDC4 | size: 0x88C
 */
void PauseMenuScene::OnSelectQUIT(TLComponentInstance*)
{
    FEPopupMenu* popup;

    if (FrontEnd::m_bGameOver)
    {
        OverlayManager::s_pInstance->Pop();
        OverlayManager::s_pInstance->Pop();
        OverlayManager::s_pInstance->Push(OVERLAY_BRAG, SCREEN_FORWARD, false);
    }
    else
    {
        popup = (FEPopupMenu*)OverlayManager::s_pInstance->Push(OVERLAY_POPUP, SCREEN_NOTHING, false);
        popup->mControlInput = mQuittingController;

        if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
        {
            {
                Function<FnVoidVoid> yes(Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(
                    MemFun<PauseMenuScene, void>(&PauseMenuScene::OnSelectPopupYESFORFEIT), this));
                Function<FnVoidVoid> no(Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(
                    MemFun<PauseMenuScene, void>(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
                popup->Create(POPUP_INGAME_QUIT_STRIKERS_101, yes, no);
            }
        }
        else if (nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode == GameInfoManager::GM_FRIENDLY || g_pGame->m_eGameState == GS_END_GAME)
        {
            {
                Function<FnVoidVoid> yes(Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(
                    MemFun<PauseMenuScene, void>(&PauseMenuScene::OnSelectPopupYESFORFEIT), this));
                Function<FnVoidVoid> no(Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(
                    MemFun<PauseMenuScene, void>(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
                popup->Create(POPUP_INGAME_QUIT_MATCH, yes, no);
            }
        }
        else if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode()
                 || (nlSingleton<GameInfoManager>::s_pInstance->IsInTournamentMode()
                     && nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)mQuittingController) != -1))
        {
            {
                Function<FnVoidVoid> yes(Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(
                    MemFun<PauseMenuScene, void>(&PauseMenuScene::OnSelectPopupYESFORFEIT), this));
                Function<FnVoidVoid> no(Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(
                    MemFun<PauseMenuScene, void>(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
                popup->Create(POPUP_INGAME_FORFEIT_MATCH, yes, no);
            }
        }
        else
        {
            {
                Function<FnVoidVoid> no(Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(
                    MemFun<PauseMenuScene, void>(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
                popup->Create(POPUP_NO_FORFEIT, no);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1890 | 0x800AED88 | size: 0x3C
 */
void PauseMenuScene::OnSelectCHOOSESIDES(TLComponentInstance*)
{
    OverlayManager::s_pInstance->Push(IGSCENE_CHOOSE_SIDES, SCREEN_FORWARD, true);
}

/**
 * Offset/Address/Size: 0x1854 | 0x800AED4C | size: 0x3C
 */
void PauseMenuScene::OnSelectAUDIOOPTIONS(TLComponentInstance*)
{
    OverlayManager::s_pInstance->Push(IGSCENE_PAUSE_AUDIO, SCREEN_FORWARD, true);
}

/**
 * Offset/Address/Size: 0x1818 | 0x800AED10 | size: 0x3C
 */
void PauseMenuScene::OnSelectVISUALOPTIONS(TLComponentInstance*)
{
    OverlayManager::s_pInstance->Push(IGSCENE_PAUSE_VISUAL, SCREEN_FORWARD, true);
}

/**
 * Offset/Address/Size: 0x17CC | 0x800AECC4 | size: 0x4C
 */
void PauseMenuScene::OnSelectSTATISTICS(TLComponentInstance*)
{
    SummaryOverlay* scene = (SummaryOverlay*)OverlayManager::s_pInstance->Push(OVERLAY_SUMMARY_PAUSE, SCREEN_FORWARD, true);
    scene->m_controllingInput = mControllingInput;
    scene->mButtonState = ButtonComponent::BS_B_ONLY;
}

/**
 * Offset/Address/Size: 0x17C8 | 0x800AECC0 | size: 0x4
 */
void PauseMenuScene::OnSelectPopupNOFORFEIT()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x1684 | 0x800AEB7C | size: 0x144
 */
void PauseMenuScene::OnSelectPopupYESFORFEIT()
{
    GameInfoManager* gim;
    s32 quittingSide;

    gim = nlSingleton<GameInfoManager>::s_pInstance;

    if (gim->mIsInStrikers101Mode)
    {
        mQuitDelay = 1.0f;
        return;
    }

    if (g_pGame->m_eGameState != GS_END_GAME)
    {
        gim = nlSingleton<GameInfoManager>::s_pInstance;
        quittingSide = -1;

        if (gim->IsInCupMode())
        {
            eTeamID userTeam = gim->GetUserSelectedCupTeam();
            if (userTeam == gim->GetTeam(0))
            {
                quittingSide = 0;
            }
            else if (userTeam == gim->GetTeam(1))
            {
                quittingSide = 1;
            }
        }
        else if (gim->IsInTournamentMode())
        {
            quittingSide = gim->GetPlayingSide(mQuittingController);
        }

        if (gim->IsInCupOrTournamentMode())
        {
            if (quittingSide == 0)
            {
                nlSingleton<StatsTracker>::s_pInstance->TrackWinner(0);
                gim->SetResultsOfLastUserGame((eUserGameResult)0xD);
            }
            else if (quittingSide == 1)
            {
                nlSingleton<StatsTracker>::s_pInstance->TrackWinner(1);
                gim->SetResultsOfLastUserGame((eUserGameResult)0xE);
            }
        }
    }

    mQuitDelay = 1.0f;
}

/**
 * Offset/Address/Size: 0x1640 | 0x800AEB38 | size: 0x44
 * Todo: Figure out how to what scene this is connecting...
 */
void PauseMenuScene::OnSelectLESSONS(TLComponentInstance*)
{
    BaseSceneHandler* scene = OverlayManager::s_pInstance->Push(IGSCENE_LESSON_SELECT, SCREEN_FORWARD, true);
    ((u8*)scene)[0x2b2] = 1;
}

/**
 * Offset/Address/Size: 0x84C | 0x800ADD44 | size: 0xDF4
 */
void PauseMenuScene::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x268 | 0x800AD760 | size: 0x5E4
 * TODO: 83.81% match - r27/r28/r29 register allocation swap for i/goToChooseSides/connState, overlay pop loop structure, ExitMenuState branch pattern
 */
void PauseMenuScene::Update(float fDeltaT)
{
    if (mQuitDelay > 0.0f)
    {
        mQuitDelay = mQuitDelay - fDeltaT;
        if (!nlSingleton<OverlayManager>::s_pInstance->IsOnStack(OVERLAY_POPUP))
        {
            glxSwapSetBlack(true);
        }
        if (mQuitDelay <= 0.0f)
        {
            mQuitDelay = 0.0f;
            FrontEnd::ReturnToFE();
        }
        return;
    }

    if (mStartAnimAtEnd)
    {
        if (m_pFEPresentation->m_currentSlide != NULL)
        {
            m_pFEPresentation->m_fadeDuration = 999.9f;
            mStartAnimAtEnd = false;
        }
    }

    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();
    mButtons2.CentreButtons();

    if (mIsInTransition)
    {
        TLSlide* slide = m_pFEPresentation->m_currentSlide;
        f32 currentTime = slide->m_time;
        f32 endTime = slide->m_start + slide->m_duration;
        if (!(currentTime >= endTime))
            return;

        if (mTransitionTo == TT_OUT)
        {
            FrontEnd::ExitMenuState();
        }
        mIsInTransition = false;
        mTransitionTo = (TransitionType)0;
        return;
    }

    u8 goToChooseSides = 0;
    int i = 0;
    u8* connState = &FrontEnd::m_ctrlConnectedState[0];

    for (; i < 4; i++)
    {
        bool isConn = g_pFEInput->IsConnected((eFEINPUT_PAD)i);

        if (!g_pFEInput->IsConnected((eFEINPUT_PAD)i))
        {
            if (nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)i) != -1)
            {
                if (!goToChooseSides)
                {
                    OverlayManager* om = nlSingleton<OverlayManager>::s_pInstance;
                    BaseSceneHandler* top;
                    if (om->mCurrentStackDepth != 0)
                    {
                        top = om->mBaseSceneHandlerStack[om->mCurrentStackDepth - 1];
                    }
                    else
                    {
                        top = NULL;
                    }
                    while (top != (BaseSceneHandler*)this)
                    {
                        om->Pop();
                        nlSingleton<FESceneManager>::s_pInstance->ForceImmediateStackProcessing();
                        om = nlSingleton<OverlayManager>::s_pInstance;
                        if (om->mCurrentStackDepth != 0)
                        {
                            top = om->mBaseSceneHandlerStack[om->mCurrentStackDepth - 1];
                        }
                        else
                        {
                            top = NULL;
                        }
                    }
                    om->Push(IGSCENE_CHOOSE_SIDES, SCREEN_FORWARD, true);
                }
                goToChooseSides = 1;
            }
        }

        *connState = isConn;
        connState++;
    }

    if (goToChooseSides)
        return;

    mDelayBeforeUnpause__14PauseMenuScene = mDelayBeforeUnpause__14PauseMenuScene - fDeltaT;
    if (mDelayBeforeUnpause__14PauseMenuScene > 0.0f)
        return;

    mDelayBeforeUnpause__14PauseMenuScene = 0.0f;

    if (m_pFEPresentation->m_currentSlide == NULL)
        return;

    if (g_pFEInput->IsAutoPressed(mControllingInput, 0xd, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int currentIndex = mMenuItems.mCurrentIndex;
        u8 wrapBit = flags & 1;
        int skipDisabled = flags & 2;
        int newIndex = currentIndex - 1;

        while (true)
        {
            if (wrapBit)
            {
                if (newIndex < 0)
                    newIndex = mMenuItems.mNumItemsAdded - 1;
            }
            else
            {
                if (newIndex < 0)
                    return;
            }

            if (!skipDisabled)
                break;

            if (!mMenuItems.mMenuItems[newIndex].mLocked)
                break;

            newIndex = newIndex - 1;
        }

        mMenuItems.mMenuItems[currentIndex].ApplyAction(ON_UNHIGHLIGHT);

        mMenuItems.mCurrentIndex = newIndex;
        int idx = mMenuItems.mCurrentIndex;
        mMenuItems.mMenuItems[idx].ApplyAction(ON_HIGHLIGHT);
        return;
    }

    if (g_pFEInput->IsAutoPressed(mControllingInput, 0xe, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int currentIndex = mMenuItems.mCurrentIndex;
        u8 wrapBit = flags & 1;
        int skipDisabled = flags & 2;
        int newIndex = currentIndex + 1;

        while (true)
        {
            if (wrapBit)
            {
                int numItems = mMenuItems.mNumItemsAdded;
                newIndex = newIndex % numItems;
            }
            else
            {
                if (newIndex >= mMenuItems.mNumItemsAdded)
                    return;
            }

            if (!skipDisabled)
                break;

            if (!mMenuItems.mMenuItems[newIndex].mLocked)
                break;

            newIndex = newIndex + 1;
        }

        mMenuItems.mMenuItems[currentIndex].ApplyAction(ON_UNHIGHLIGHT);

        mMenuItems.mCurrentIndex = newIndex;
        int idx = mMenuItems.mCurrentIndex;
        mMenuItems.mMenuItems[idx].ApplyAction(ON_HIGHLIGHT);
        return;
    }

    if (g_pFEInput->JustPressed(mControllingInput, 0x100, false, &mQuittingController))
    {
        int selectedIndex = mMenuItems.mCurrentIndex;
        MenuItem<TLComponentInstance>* item = &mMenuItems.mMenuItems[selectedIndex];
        MenuResult result;

        if (item->mCallbacks[ON_APPLY].mTag != EMPTY)
        {
            if (item->mDisabled)
            {
                result = RES_ITEM_DISABLED;
            }
            else
            {
                item->mCallbacks[ON_APPLY](item->mType);
                result = RES_OK;
            }
        }
        else
        {
            result = RES_NO_CALLBACK_FUNC;
        }

        switch (result)
        {
        case RES_OK:
            mLastSelectedIndex = mMenuItems.mCurrentIndex;
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
            break;
        case RES_ITEM_DISABLED:
            FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            break;
        default:
            break;
        }
        return;
    }

    if (!g_pFEInput->JustPressed(mControllingInput, 0x200, false, NULL))
    {
        if (!g_pFEInput->JustPressed(mControllingInput, 0x1000, false, NULL))
            return;
        if (FrontEnd::m_bGameOver)
            return;
    }

    if (!FrontEnd::m_bGameOver)
    {
        OnSelectRESUME(NULL);
    }
    else
    {
        FrontEnd::ExitMenuState();
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
    }
}

/**
 * Offset/Address/Size: 0xD0 | 0x800AD5C8 | size: 0x198
 */
void PauseMenuScene::TransitionOut(PauseMenuScene::TransitionType newtype)
{
    mIsInTransition = true;
    mTransitionTo = newtype;

    if (mTransitionTo == TT_OUT)
    {
        FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
        presentation->SetActiveSlide("menu in2");
        presentation->Update(0.0f);

        int i;
        for (i = 0; i < mMenuItems.mNumItemsAdded; i++)
        {
            typedef TLInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
            typedef TLInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
            union
            {
                FindByValue byValue;
                FindByRef byRef;
            } find;
            find.byValue = FEFinder<TLInstance, 4>::Find<TLSlide>;

            char menuname[64];
            nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

            volatile InlineHasher hB, hA;
            volatile InlineHasher h9, h8;
            volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

            h0.m_Hash = 0;
            h1.m_Hash = 0;
            h2.m_Hash = 0;
            h3.m_Hash = 0;
            h4.m_Hash = 0;
            h5.m_Hash = 0;
            h6.m_Hash = 0;
            h7.m_Hash = 0;

            unsigned long hash = nlStringLowerHash(menuname);
            h8.m_Hash = hash;
            h9.m_Hash = hash;

            hash = nlStringLowerHash("Layer");
            hB.m_Hash = hash;
            hA.m_Hash = hash;

            TLInstance* instance = find.byRef(
                presentation->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

            TLComponentInstance* compinstance = (TLComponentInstance*)instance;

            if (i == mMenuItems.mCurrentIndex)
            {
                compinstance->SetActiveSlide("out");
                compinstance->Update(0.0f);

                TLComponentInstance* highlite = (TLComponentInstance*)FindComponent(compinstance->GetActiveSlide(), "highlite");
                highlite->SetActiveSlide("out");
                highlite->Update(0.0f);
                highlite->SetAssetColour(MenuHighliteColour);
            }
            else
            {
                compinstance->SetActiveSlide("out");
                compinstance->Update(0.0f);

                TLComponentInstance* highlite = (TLComponentInstance*)FindComponent(compinstance->GetActiveSlide(), "highlite");
                highlite->m_bVisible = false;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800AD4F8 | size: 0xD0
 */
void PauseMenuScene::OpenItem(TLComponentInstance* instance)
{
    DoubleHighlite::OpenItem(instance);

    if (mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mDisabled)
    {
        TLTextInstance* text = FEFinder<TLTextInstance, 3>::Find(
            instance->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("disabled_text")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        text->m_LocStrId = 0x38202C30;
        text->m_OverloadFlags |= 0x8;
    }
}
